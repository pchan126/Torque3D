//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "gfx/Vulkan/gfxPipelineManager.h"

#include "core/strings/stringFunctions.h"
#include "core/util/safeDelete.h"
#include "core/util/dxt5nmSwizzle.h"
#include "console/consoleTypes.h"
#include "gfx/Vulkan/gfxPipelineObject.h"
#include "gfxVulkanShader.h"

using namespace Torque;

//#define DEBUG_SPEW


//S32 GFXPipelineManager::smPipelineReductionLevel = 0;
//
//String GFXPipelineManager::smMissingPipelinePath(Con::getVariable("$Core::MissingPipelinePath"));
//String GFXPipelineManager::smUnavailablePipelinePath(Con::getVariable("$Core::UnAvailablePipelinePath"));
//String GFXPipelineManager::smWarningPipelinePath(Con::getVariable("$Core::WarningPipelinePath"));

GFXPipelineManager::EventSignal GFXPipelineManager::smEventSignal;


void GFXPipelineManager::init()
{
  	CreatePipelineCacheObject();

	//Con::addVariable( "$pref::Video::PipelineReductionLevel", TypeS32, &smPipelineReductionLevel,
   //   "The number of mipmap levels to drop on loaded Pipelines to reduce "
   //   "video memory usage.  It will skip any Pipelines that have been defined "
   //   "as not allowing down scaling.\n"
   //   "@ingroup GFX\n" );

   //Con::addVariable( "$pref::Video::missingPipelinePath", TypeRealString, &smMissingPipelinePath,
   //   "The file path of the Pipeline to display when the requested Pipeline is missing.\n"
   //   "@ingroup GFX\n" );

   //Con::addVariable( "$pref::Video::unavailablePipelinePath", TypeRealString, &smUnavailablePipelinePath,
   //   "@brief The file path of the Pipeline to display when the requested Pipeline is unavailable.\n\n"
   //   "Often this Pipeline is used by GUI controls to indicate that the request image is unavailable.\n"
   //   "@ingroup GFX\n" );

   //Con::addVariable( "$pref::Video::warningPipelinePath", TypeRealString, &smWarningPipelinePath,
   //   "The file path of the Pipeline used to warn the developer.\n"
   //   "@ingroup GFX\n" );
}

void GFXPipelineManager::deletePipeline(GFXPipelineObject* Pipeline)
{
}

GFXPipelineManager::GFXPipelineManager()
{
   mListHead = mListTail = NULL;
   mPipelineManagerState = GFXPipelineManager::Living;

   // Set up the hash table
   mHashCount = 1023;
   mHashTable = new GFXPipelineObject *[mHashCount];
   for(U32 i = 0; i < mHashCount; i++)
      mHashTable[i] = NULL;

}

GFXPipelineManager::~GFXPipelineManager()
{
   if( mHashTable )
      SAFE_DELETE_ARRAY( mHashTable );

   //mCubemapTable.clear();
}

//U32 GFXPipelineManager::getPipelineDownscalePower( GFXPipelineProfile *profile )
//{
//   if ( !profile || profile->canDownscale() )
//      return smPipelineReductionLevel;
//
//   return 0;
//}

//bool GFXPipelineManager::validatePipelineQuality( GFXPipelineProfile *profile, U32 &width, U32 &height )
//{
//   U32 scaleFactor = getPipelineDownscalePower( profile );
//   if ( scaleFactor == 0 )
//      return true;
//
//   // Otherwise apply the appropriate scale...
//   width  >>= scaleFactor;
//   height >>= scaleFactor;
//
//   return true;
//}

void GFXPipelineManager::kill()
{
   AssertFatal( mPipelineManagerState != GFXPipelineManager::Dead, "Pipeline Manager already killed!" );

   // Release everything in the cache we can
   // so we don't leak any Pipelines.
   cleanupCache();

   GFXPipelineObject *curr = mListHead;
   GFXPipelineObject *temp;

   // Actually delete all the Pipelines we know about.
   //while( curr != NULL ) 
   //{
   //   temp = curr->mNext;
   //   curr->kill();
   //   curr = temp;
   //}

   mPipelineManagerState = GFXPipelineManager::Dead;
}

void GFXPipelineManager::zombify()
{
   AssertFatal( mPipelineManagerState != GFXPipelineManager::Zombie, "Pipeline Manager already a zombie!" );

   // Notify everyone that cares about the zombification!
   smEventSignal.trigger( GFXZombify );

   // Release unused pool Pipelines.
   cleanupPool();

   // Release everything in the cache we can.
   cleanupCache();

   // Free all the device copies of the Pipelines.
   GFXPipelineObject *temp = mListHead;
   //while( temp != NULL ) 
   //{
   //   freePipeline( temp, true );
   //   temp = temp->mNext;
   //}

   // Finally, note our state.
   mPipelineManagerState = GFXPipelineManager::Zombie;
}

void GFXPipelineManager::resurrect()
{
   // Reupload all the device copies of the Pipelines.
   GFXPipelineObject *temp = mListHead;

   //while( temp != NULL ) 
   //{
   //   refreshPipeline( temp );
   //   temp = temp->mNext;
   //}

   // Notify callback registries.
   smEventSignal.trigger( GFXResurrect );
   
   // Update our state.
   mPipelineManagerState = GFXPipelineManager::Living;
}

void GFXPipelineManager::cleanupPool()
{
   PROFILE_SCOPE( GFXPipelineManager_CleanupPool );

   PipelinePoolMap::Iterator iter = mPipelinePool.begin();
   //for ( ; iter != mPipelinePool.end(); )
   //{
   //   if ( iter->value->getRefCount() == 1 )
   //   {
   //      // This Pipeline is unreferenced, so take the time
   //      // now to completely remove it from the pool.
   //      PipelinePoolMap::Iterator unref = iter;
   //      ++iter;
   //      unref->value = NULL;
   //      mPipelinePool.erase( unref );
   //      continue;
   //   }

   //   ++iter;
   //}
}

void GFXPipelineManager::requestDeletePipeline( GFXPipelineObject *Pipeline )
{
   // If this is a non-cached Pipeline then just really delete it.
   //if ( Pipeline->mPipelineLookupName.isEmpty() )
   //{
   //   delete Pipeline;
   //   return;
   //}

   //// Set the time and store it.
   //Pipeline->mDeleteTime = Platform::getTime();
   //mToDelete.push_back_unique( Pipeline );
}

void GFXPipelineManager::cleanupCache( U32 secondsToLive )
{
   PROFILE_SCOPE( GFXPipelineManager_CleanupCache );

   //U32 killTime = Platform::getTime() - secondsToLive;

   //for ( U32 i=0; i < mToDelete.size(); )
   //{
   //   GFXPipelineObject *tex = mToDelete[i];

   //   // If the Pipeline was picked back up by a user
   //   // then just remove it from the list.
   //   if ( tex->getRefCount() != 0 )
   //   {
   //      mToDelete.erase_fast( i );
   //      continue;
   //   }

   //   // If its time has expired delete it for real.
   //   if ( tex->mDeleteTime <= killTime )
   //   {
   //      //Con::errorf( "Killed Pipeline: %s", tex->mPipelineLookupName.c_str() );
   //      delete tex;
   //      mToDelete.erase_fast( i );
   //      continue;
   //   }

   //   i++;
   //}
}

bool GFXPipelineManager::CreatePipelineCacheObject(char const *cache_data)
{
	VkPipelineCacheCreateInfo pipeline_cache_create_info =
	{
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		nullptr,
		0,
		cache_data == nullptr ? 0 : strlen(cache_data),
		cache_data
	};

	if (vkCreatePipelineCache(GFXVulkan->getLogicalDevice(), &pipeline_cache_create_info, nullptr, &pipeline_cache) != VK_SUCCESS)
	{
		Con::printf ("Could not create pipeline cache");
		return false;
	}
	return true;
}

GFXPipelineObject* GFXPipelineManager::hashFind(const String& name)
{
	return nullptr;
}

void GFXPipelineManager::hashInsert(GFXPipelineObject* object)
{
}

void GFXPipelineManager::hashRemove(GFXPipelineObject* object)
{
}

GFXPipelineObject * GFXPipelineManager::_findPooledPipeline(U32 width, U32 height, GFXFormat format, GFXPipelineProfile * profile, U32 numMipLevels, S32 antialiasLevel)
{
	return nullptr;
}

GFXPipelineObject* GFXPipelineManager::_createPipeline(const String& resourceName, GFXPipelineProfile* profile,
	GFXPipelineObject* inObj)
{
	return nullptr;
}

GFXPipelineObject* GFXPipelineManager::_createPipeline(GFXPrimitiveType primType, GFXShader* _shader)
{
	GFXVulkanShader* shader = dynamic_cast<GFXVulkanShader*>(_shader);
	GFXPipelineObject* retval = new GFXPipelineObject(GFXVulkan);

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(GFX->getViewport().extent.x);
	viewport.height = static_cast<float>(GFX->getViewport().extent.y);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { GFX->getViewport().point.x, GFX->getViewport().point.y };
	scissor.extent = { static_cast<U32>(GFX->getViewport().extent.x), static_cast<U32>(GFX->getViewport().extent.y) };

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
	{
		VulkanUtils::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		VulkanUtils::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1),
		VulkanUtils::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2)
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout =
		VulkanUtils::descriptorSetLayoutCreateInfo(
			setLayoutBindings.data(),
			static_cast<uint32_t>(setLayoutBindings.size()));

	VkDescriptorSetLayout descriptorSetLayout;
	VkResult result1 = vkCreateDescriptorSetLayout(GFXVulkan->getLogicalDevice(), &descriptorLayout, nullptr, &descriptorSetLayout);

	std::vector<VkDescriptorSetLayout> temp;
	temp.push_back(descriptorSetLayout);
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = VulkanUtils::pipelineLayoutCreateInfo(&descriptorSetLayout);	

	VkPipelineLayout pipelineLayout;

	if (vkCreatePipelineLayout(GFXVulkan->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		VulkanUtils::pipelineInputAssemblyStateCreateInfo(
			GFXVulkanPrimType[primType],
			0,
			VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		VulkanUtils::pipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			0);

	VkPipelineColorBlendAttachmentState blendAttachmentState =
		VulkanUtils::pipelineColorBlendAttachmentState(
			0xf,
			VK_FALSE);

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		VulkanUtils::pipelineColorBlendStateCreateInfo(
			1,
			&blendAttachmentState);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		VulkanUtils::pipelineDepthStencilStateCreateInfo(
			VK_TRUE,
			VK_TRUE,
			VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		VulkanUtils::pipelineMultisampleStateCreateInfo(
			VK_SAMPLE_COUNT_1_BIT,
			0);

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo _dynamicState =
		VulkanUtils::pipelineDynamicStateCreateInfo(
			dynamicStateEnables.data(),
			static_cast<uint32_t>(dynamicStateEnables.size()),
			0);

	//// Load shaders
	//std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	//shaderStages[0] = loadShader(getAssetPath() + "shaders/dynamicuniformbuffer/base.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	//shaderStages[1] = loadShader(getAssetPath() + "shaders/dynamicuniformbuffer/base.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		VulkanUtils::pipelineCreateInfo(
			pipelineLayout,
			GFXVulkan->getRenderPass(),
			0);

	pipelineCreateInfo.pVertexInputState = shader->getVertexInputInfo();
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = shader->getShaderStageCount();
	pipelineCreateInfo.pStages = shader->getShaderData();

	vkCreateGraphicsPipelines(GFXVulkan->getLogicalDevice(), pipeline_cache, 1, &pipelineCreateInfo, nullptr, &(retval->pipeline));

	return retval;
}

void GFXPipelineManager::freePipeline(GFXPipelineObject* Pipeline, bool zombify)
{
}

void GFXPipelineManager::refreshPipeline(GFXPipelineObject* Pipeline)
{
}

GFXPipelineObject *GFXPipelineManager::_lookupPipeline( const char *hashName, const GFXPipelineProfile *profile  )
{
   GFXPipelineObject *ret = hashFind( hashName );

   ////compare just the profile flags and not the entire profile, names could be different but otherwise identical flags
   //if (ret && (ret->mProfile->compareFlags(*profile)))
   //   return ret;
   //else if (ret)
   //   Con::warnf("GFXPipelineManager::_lookupPipeline: Cached Pipeline %s has a different profile flag", hashName);

   return NULL;
}

