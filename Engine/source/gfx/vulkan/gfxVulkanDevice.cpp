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

#include <platform/platform.h>

#include <gfx/gfxCubemap.h>
#include <gfx/screenshot.h>
#include "gfx/gfxDrawUtil.h"

#include <gfx/vulkan/gfxVulkanEnumTranslate.h>
#include <gfx/vulkan/gfxVulkanVertexBuffer.h>
//#include "gfx/gl/gfxVulkanPrimitiveBuffer.h"
#include <gfx/vulkan/gfxVulkanTextureTarget.h>
#include <gfx/vulkan/gfxVulkanTextureManager.h>
#include <gfx/vulkan/gfxVulkanTextureObject.h>
#include <gfx/vulkan/gfxVulkanDevice.h>
//#include "gfx/gl/gfxVulkanCardProfiler.h"
#include <gfx/vulkan/gfxVulkanWindowTarget.h>
#include <platform/platformDlibrary.h>
//#include "gfx/gl/gfxVulkanShader.h"
#include <gfx/primBuilder.h>
#include <console/console.h>
//#include "gfx/gl/gfxVulkanOcclusionQuery.h"
#include <materials/shaderData.h>
//#include "gfx/gl/gfxVulkanStateCache.h"
//#include "gfx/gl/gfxVulkanVertexAttribLocation.h"
#include "gfx/Vulkan/gfxVulkanVertexDecl.h"
#include <shaderGen/shaderGen.h>
#include <gfx/vulkan/gfxVulkanCubemap.h>
#include <gfx/vulkan/gfxVulkanCardProfiler.h>
#include <valarray>
#include "gfxVulkanShader.h"
#include <numeric>

GFXAdapter::CreateDeviceInstanceDelegate GFXVulkanDevice::mCreateDeviceInstance(createInstance);


GFXDevice* GFXVulkanDevice::createInstance(U32 adapterIndex)
{
	return new GFXVulkanDevice(adapterIndex);
}

//namespace Vulkan
//{
//   extern void gglPerformBinds();
//   extern void gglPerformExtensionBinds(void *context);
//}

//void loadVulkanCore()
//{
//   static bool coreLoaded = false; // Guess what this is for.
//   if(coreLoaded)
//      return;
//   coreLoaded = true;
//   
//   // Make sure we've got our Vulkan bindings.
//   Vulkan::gglPerformBinds();
//}

//void loadVulkanExtensions(void *context)
//{
//   static bool extensionsLoaded = false;
//   if(extensionsLoaded)
//      return;
//   extensionsLoaded = true;
//   
//   Vulkan::gglPerformExtensionBinds(context);
//}

//void STDCALL glDebugCallback(Vulkanenum source, Vulkanenum type, Vulkanuint id, Vulkanenum severity, Vulkansizei length, 
//	const Vulkanchar *message, const void *userParam)
//{
//    // JTH [11/24/2016]: This is a temporary fix so that we do not get spammed for redundant fbo changes.
//    // This only happens on Intel cards. This should be looked into sometime in the near future.
//    if (dStrStartsWith(message, "API_ID_REDUNDANT_FBO"))
//        return;
//    if (severity == Vulkan_DEBUG_SEVERITY_HIGH)
//        Con::errorf("OPENVulkan: %s", message);
//    else if (severity == Vulkan_DEBUG_SEVERITY_MEDIUM)
//        Con::warnf("OPENVulkan: %s", message);
//    else if (severity == Vulkan_DEBUG_SEVERITY_LOW)
//        Con::printf("OPENVulkan: %s", message);
//}
//
//void STDCALL glAmdDebugCallback(Vulkanuint id, Vulkanenum category, Vulkanenum severity, Vulkansizei length,
//    const Vulkanchar* message, Vulkanvoid* userParam)
//{
//    if (severity == Vulkan_DEBUG_SEVERITY_HIGH)
//        Con::errorf("AMDOPENVulkan: %s", message);
//    else if (severity == Vulkan_DEBUG_SEVERITY_MEDIUM)
//        Con::warnf("AMDOPENVulkan: %s", message);
//    else if (severity == Vulkan_DEBUG_SEVERITY_LOW)
//        Con::printf("AMDOPENVulkan: %s", message);
//}

void GFXVulkanDevice::initVulkanState()
{
	VkResult err;

	// Deal with the card profiler here when we know we have a valid context.
	mCardProfiler = new GFXVulkanCardProfiler();
	mCardProfiler->init();
	physicalDevice = dynamic_cast<GFXVulkanCardProfiler*>(mCardProfiler)->getPhysicalDevice();

	// Check queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	if (queueFamilyCount == 0)
	{
		Con::errorf("physical device has no queue families!");
		exit(1);
	}

	queueFamilies.resize(queueFamilyCount);

	// Find queue family with graphics support
	// Note: is a transfer queue necessary to copy vertices to the gpu or can a graphics queue handle that?
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	Con::printf("physical device has %d queue families", queueFamilyCount);

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueFamily = i;
		}
		i++;
	}

	std::vector<VkExtensionProperties> available_extensions = extensions;

	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

	U32 dev_prop_count;
	err = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &dev_prop_count, nullptr);

	std::vector<VkExtensionProperties> device_extension_properties;
	device_extension_properties.resize(dev_prop_count);

	err = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &dev_prop_count,
	                                           device_extension_properties.data());

	desired_extensions.clear();
	for (auto& ext2 : device_extension_properties)
	{
		if (strcmp(ext2.extensionName, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME) != 0 &&
			strcmp(ext2.extensionName, "VK_AMD_wave_limits") != 0 &&
			strcmp(ext2.extensionName, "VK_AMD_gpa_interface") != 0)
			desired_extensions.push_back(ext2.extensionName);
	}

	VkPhysicalDeviceFeatures desired_features;
	vkGetPhysicalDeviceFeatures(physicalDevice, &desired_features);

	std::vector<float> priorities = {1.0f};

	VkDeviceQueueCreateInfo temp{};
	temp.queueFamilyIndex = graphicsQueueFamily;
	temp.queueCount = 1;
	temp.pQueuePriorities = priorities.data();
	temp.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueFamilies[0].queueCount = 1;
	queue_create_infos.push_back(temp);
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<U32>(queue_create_infos.size());
	device_create_info.pQueueCreateInfos = queue_create_infos.empty() ? nullptr : queue_create_infos.data();
	device_create_info.enabledExtensionCount = static_cast<U32>(desired_extensions.size());
	device_create_info.ppEnabledExtensionNames = desired_extensions.empty() ? nullptr : desired_extensions.data();
	device_create_info.pEnabledFeatures = &desired_features;

	err = vkCreateDevice(physicalDevice, &device_create_info, nullptr, &logical_device);
	if (err)
		Con::errorf("Could not create logical device.");

	VmaAllocatorCreateInfo allocatorInfo= {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = logical_device;

	vmaCreateAllocator(&allocatorInfo, &allocator);

	//std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings
	//{
	//	VulkanUtils::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
	//	VulkanUtils::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1),
	//	VulkanUtils::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2)
	//};

	//CreatePipelineLayout(setLayoutBindings);

	vkGetDeviceQueue(logical_device, graphicsQueueFamily, 0, &queue);
	createCommandPool(graphicsQueueFamily, commandPool);
	createSemaphore(semaphores.overlayComplete);
	createSemaphore(semaphores.presentComplete);
	createSemaphore(semaphores.renderComplete);

	mPipelineManager->init();
}

void GFXVulkanDevice::vsyncCallback()
{
	//PlatformVulkan::setVSync(smDisableVSync ? 0 : 1);
}

GFXVulkanDevice::GFXVulkanDevice(U32 adapterIndex) :
	mAdapterIndex(adapterIndex),
	mNeedUpdateVertexAttrib(false),
	//mCurrentPB(NULL),
	mDrawInstancesCount(0),
	mCurrentShader(nullptr),
	m_mCurrentWorld(true),
	m_mCurrentView(true),
	mContext(nullptr),
	mPixelFormat(nullptr),
	mPixelShaderVersion(4.5f),
	mMaxShaderTextures(2),
	mMaxFFTextures(2),
	mMaxTRColors(1),
	mClip(0, 0, 0, 0),
	mWindowRT(nullptr),
	descriptorPool(VK_NULL_HANDLE)
{
	for(U32 i = 0; i < VERTEX_STREAM_COUNT; ++i)
	{
	   mCurrentVB[i] = nullptr;
	   mCurrentVB_Divisor[i] = 0;
	}

	//// Initiailize capabilities to false.
	//memset(&mCapabilities, 0, sizeof(VulkanCapabilities));

	//loadVulkanCore();

	GFXVulkanEnumTranslate::init();

	//GFXVertexColor::setSwizzle( &Swizzles::rgba );

	//// OpenVulkan have native RGB, no need swizzle
	//mDeviceSwizzle32 = &Swizzles::rgba;
	//mDeviceSwizzle24 = &Swizzles::rgb;

	mTextureManager = new GFXVulkanTextureManager();
	gScreenShot = new ScreenShot();

	mPipelineManager = new GFXPipelineManager();

	//for(U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
	//   mActiveTextureType[i] = Vulkan_ZERO;

	mNumVertexStream = 2;

	//for(int i = 0; i < GS_COUNT; ++i)
	//   mModelViewProjSC[i] = NULL;

	//mOpenglStateCache = new GFXVulkanStateCache;
}

GFXVulkanDevice::~GFXVulkanDevice()
{
	mCurrentStateBlock = NULL;

	for(int i = 0; i < VERTEX_STREAM_COUNT; ++i)      
	   mCurrentVB[i] = NULL;

	//mCurrentPB = NULL;
	//
	//for(U32 i = 0; i < mVolatileVBs.size(); i++)
	//   mVolatileVBs[i] = NULL;
	//for(U32 i = 0; i < mVolatilePBs.size(); i++)
	//   mVolatilePBs[i] = NULL;

	//// Clear out our current texture references
	//for (U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
	//{
	//   mCurrentTexture[i] = NULL;
	//   mNewTexture[i] = NULL;
	//   mCurrentCubemap[i] = NULL;
	//   mNewCubemap[i] = NULL;
	//}

	//mRTStack.clear();
	//mCurrentRT = NULL;

	if (mTextureManager)
	{
		mTextureManager->zombify();
		mTextureManager->kill();
	}

	//GFXResource* walk = mResourceListHead;
	//while(walk)
	//{
	//   walk->zombify();
	//   walk = walk->getNextResource();
	//}
	//   
	//if( mCardProfiler )
	//   SAFE_DELETE( mCardProfiler );

	//SAFE_DELETE( gScreenShot );

	//SAFE_DELETE( mOpenglStateCache );
	if (descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(logical_device, descriptorPool, nullptr);
	}

	vkDestroyInstance(instance, nullptr);
	vkDestroyCommandPool(logical_device, commandPool, nullptr);


}

void GFXVulkanDevice::zombify()
{
	//mTextureManager->zombify();

	//for(int i = 0; i < VERTEX_STREAM_COUNT; ++i)   
	//   if(mCurrentVB[i])
	//      mCurrentVB[i]->finish();
	//if(mCurrentPB)
	//      mCurrentPB->finish();

	//mVolatileVBs.clear();
	//mVolatilePBs.clear();
	GFXResource* walk = mResourceListHead;
	while (walk)
	{
		walk->zombify();
		walk = walk->getNextResource();
	}
}

void GFXVulkanDevice::resurrect()
{
	GFXResource* walk = mResourceListHead;
	while (walk)
	{
		walk->resurrect();
		walk = walk->getNextResource();
	}
	for (int i = 0; i < VERTEX_STREAM_COUNT; ++i)
		if (mCurrentVB[i])
			mCurrentVB[i]->prepare();
	//if(mCurrentPB)
	//   mCurrentPB->prepare();

	//mTextureManager->resurrect();
}

GFXVertexBuffer* GFXVulkanDevice::findVolatileVBO(U32 numVerts, const GFXVertexFormat* vertexFormat, U32 vertSize)
{
	PROFILE_SCOPE(GFXVulkanDevice_findVBPool);
	for (auto itr : mVolatileVBs)
		if (itr->mNumVerts >= numVerts &&
			itr->mVertexFormat.isEqual(*vertexFormat) &&
			itr->mVertexSize == vertSize &&
			itr->getRefCount() == 1)
			return itr;

	// No existing VB, so create one
	PROFILE_SCOPE(GFXVulkanDevice_createVBPool);
	StrongRefPtr<GFXVulkanVertexBuffer> buf(
		new GFXVulkanVertexBuffer(GFX, numVerts, vertexFormat, vertSize, GFXBufferTypeVolatile));
	buf->registerResourceWithDevice(this);
	mVolatileVBs.push_back(buf);
	return buf.getPointer();
}

GFXPrimitiveBuffer* GFXVulkanDevice::findVolatilePBO(U32 numIndices, U32 numPrimitives)
{
	//for(U32 i = 0; i < mVolatilePBs.size(); i++)
	//   if((mVolatilePBs[i]->mIndexCount >= numIndices) && (mVolatilePBs[i]->getRefCount() == 1))
	//      return mVolatilePBs[i];
	//
	//// No existing PB, so create one
	//StrongRefPtr<GFXVulkanPrimitiveBuffer> buf(new GFXVulkanPrimitiveBuffer(GFX, numIndices, numPrimitives, GFXBufferTypeVolatile));
	//buf->registerResourceWithDevice(this);
	//mVolatilePBs.push_back(buf);
	//return buf.getPointer();
	return nullptr;
}

GFXVertexBuffer* GFXVulkanDevice::allocVertexBuffer(U32 numVerts,
                                                    const GFXVertexFormat* vertexFormat,
                                                    U32 vertSize,
                                                    GFXBufferType bufferType,
                                                    void* data)
{
	PROFILE_SCOPE(GFXVulkanDevice_allocVertexBuffer);
	if (bufferType == GFXBufferTypeVolatile)
		return findVolatileVBO(numVerts, vertexFormat, vertSize);

	GFXVulkanVertexBuffer* buf = new GFXVulkanVertexBuffer(GFX, numVerts, vertexFormat, vertSize, bufferType);
	buf->registerResourceWithDevice(this);

	if (data)
	{
		void* dest;
		buf->lock(0, numVerts, &dest);
		dMemcpy(dest, data, vertSize * numVerts);
		buf->unlock();
	}

	return buf;
}

GFXPrimitiveBuffer* GFXVulkanDevice::allocPrimitiveBuffer(U32 numIndices, U32 numPrimitives, GFXBufferType bufferType,
                                                          void* data)
{
	//GFXPrimitiveBuffer* buf;
	//
	//if(bufferType == GFXBufferTypeVolatile)
	//{
	//   buf = findVolatilePBO(numIndices, numPrimitives);
	//}
	//else
	//{
	//   buf = new GFXVulkanPrimitiveBuffer(GFX, numIndices, numPrimitives, bufferType);
	//   buf->registerResourceWithDevice(this);
	//}
	//
	//if(data)
	//{
	//   void* dest;
	//   buf->lock(0, numIndices, &dest);
	//   dMemcpy(dest, data, sizeof(U16) * numIndices);
	//   buf->unlock();
	//}
	//return buf;
	return nullptr;
}

void GFXVulkanDevice::setVertexStream(U32 stream, GFXVertexBuffer* buffer)
{
//	AssertFatal(stream <= 1, "GFXVulkanDevice::setVertexStream only support 2 stream (0: data, 1: instancing)");
	if (buffer)
	{
   		GFXVulkanVertexDecl* temp = dynamic_cast<GFXVulkanVertexDecl*>(buffer->mVertexFormat.getDecl());
		auto x = temp->vertices;
	}
	//for (auto itr: )
	//{
	//	int i = 48;
	//}
	if(mCurrentVB[stream] != buffer)
	{
	   // Reset the state the old VB required, then set the state the new VB requires.
	   if( mCurrentVB[stream] )
	   {     
	      //mCurrentVB[stream]->finish();
	   }

	   mCurrentVB[stream] = static_cast<GFXVulkanVertexBuffer*>( buffer );

	   mNeedUpdateVertexAttrib = true;
	}
}

void GFXVulkanDevice::setVertexStreamFrequency(U32 stream, U32 frequency)
{
	//if( stream == 0 )
	//{
	//   mCurrentVB_Divisor[stream] = 0; // non instanced, is vertex buffer
	//   mDrawInstancesCount = frequency; // instances count
	//}
	//else
	//{
	//   AssertFatal(frequency <= 1, "GFXVulkanDevice::setVertexStreamFrequency only support 0/1 for this stream" );
	//   if( stream == 1 && frequency == 1 )
	//      mCurrentVB_Divisor[stream] = 1; // instances data need a frequency of 1
	//   else
	//      mCurrentVB_Divisor[stream] = 0;
	//}

	//mNeedUpdateVertexAttrib = true;
}

GFXCubemap* GFXVulkanDevice::createCubemap()
{
	GFXVulkanCubemap* cube = new GFXVulkanCubemap();
	cube->registerResourceWithDevice(this);
	return cube;
};

void GFXVulkanDevice::endSceneInternal()
{
	// nothing to do for opengl
	mCanCurrentlyRender = false;
}

void GFXVulkanDevice::clear(U32 flags, const LinearColorF& color, F32 z, U32 stencil)
{
	//// Make sure we have flushed our render target state.
	//_updateRenderTargets();
	//
	bool writeAllColors = true;
	bool zwrite = true;   
	bool writeAllStencil = true;
	const GFXStateBlockDesc *desc = NULL;
	if (mCurrentVulkanStateBlock)
	{
	   desc = &mCurrentVulkanStateBlock->getDesc();
	   zwrite = desc->zWriteEnable;
	   writeAllColors = desc->colorWriteRed && desc->colorWriteGreen && desc->colorWriteBlue && desc->colorWriteAlpha;
	   writeAllStencil = desc->stencilWriteMask == 0xFFFFFFFF;
	}
	
	//glColorMask(true, true, true, true);
	//glDepthMask(true);
	//glStencilMask(0xFFFFFFFF);
	//glClearColor(color.red, color.green, color.blue, color.alpha);
	//glClearDepth(z);
	//glClearStencil(stencil);

	//Vulkanbitfield clearflags = 0;
	//clearflags |= (flags & GFXClearTarget)   ? Vulkan_COLOR_BUFFER_BIT : 0;
	//clearflags |= (flags & GFXClearZBuffer)  ? Vulkan_DEPTH_BUFFER_BIT : 0;
	//clearflags |= (flags & GFXClearStencil)  ? Vulkan_STENCIL_BUFFER_BIT : 0;

	//glClear(clearflags);

	//if(!writeAllColors)
	//   glColorMask(desc->colorWriteRed, desc->colorWriteGreen, desc->colorWriteBlue, desc->colorWriteAlpha);
	//
	//if(!zwrite)
	//   glDepthMask(false);

	//if(!writeAllStencil)
	//   glStencilMask(desc->stencilWriteMask);
}

// Given a primitive type and a number of primitives, return the number of indexes/vertexes used.
inline U32 GFXVulkanDevice::primCountToIndexCount(GFXPrimitiveType primType, U32 primitiveCount)
{
switch (primType)
{
   case GFXPointList :
      return primitiveCount;
      break;
   case GFXLineList :
      return primitiveCount * 2;
      break;
   case GFXLineStrip :
      return primitiveCount + 1;
      break;
   case GFXTriangleList :
      return primitiveCount * 3;
      break;
   case GFXTriangleStrip :
      return 2 + primitiveCount;
      break;
   default:
      AssertFatal(false, "GFXVulkanDevice::primCountToIndexCount - unrecognized prim type");
      break;
}

   return 0;
}

GFXVertexDecl* GFXVulkanDevice::allocVertexDecl(const GFXVertexFormat* vertexFormat)
{
	//PROFILE_SCOPE(GFXVulkanDevice_allocVertexDecl);
	typedef Map<void*, GFXVulkanVertexDecl> GFXVulkanVertexDeclMap;
	static GFXVulkanVertexDeclMap declMap;   
	GFXVulkanVertexDeclMap::Iterator itr = declMap.find( (void*)vertexFormat->getDescription().c_str() ); // description string are interned, safe to use c_str()
	if(itr != declMap.end())
	   return &itr->value;

	auto temp = vertexFormat->getDescription().c_str();
	GFXVulkanVertexDecl &decl = declMap[(void*)vertexFormat->getDescription().c_str()];   
	decl.init(vertexFormat);
	return &decl;
	//return nullptr;
}

void GFXVulkanDevice::setVertexDecl(const GFXVertexDecl* decl)
{
	static_cast<const GFXVulkanVertexDecl*>(decl)->prepareVertexFormat();
}

inline void GFXVulkanDevice::preDrawPrimitive()
{
	if (mStateDirty)
	{
		updateStates();
	}
	
	if(mCurrentShaderConstBuffer)
	   setShaderConstBufferInternal(mCurrentShaderConstBuffer);

//	if (mNeedUpdateVertexAttrib)
//	{
//		AssertFatal(mCurrVertexDecl, "");
//		const GFXVulkanVertexDecl* decl = static_cast<const GFXVulkanVertexDecl*>(mCurrVertexDecl);
//
//		for (int i = 0; i < getNumVertexStreams(); ++i)
//		{
//			if (mCurrentVB[i])
//			{
//				mCurrentVB[i]->prepare();    // Vulkan_ARB_vertex_attrib_binding  
//				            //decl->prepareBuffer_old( i, mCurrentVB[i]->mBuffer, mCurrentVB_Divisor[i] ); // old vertex buffer/format
//			}
//		}
//
////		      decl->updateActiveVertexAttrib( GFXVulkan->getOpenglCache()->getCacheVertexAttribActive() );         
//	}

	mNeedUpdateVertexAttrib = false;
}

inline void GFXVulkanDevice::postDrawPrimitive(U32 primitiveCount)
{
	mDeviceStatistics.mDrawCalls++;
	mDeviceStatistics.mPolyCount += primitiveCount;
}

void GFXVulkanDevice::drawPrimitive(GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount)
{
	Vector<U32> indexBuffer;
	indexBuffer.setSize(primCountToIndexCount(primType,primitiveCount));
	std::iota(indexBuffer.begin(), indexBuffer.end(), vertexStart);


	preDrawPrimitive();
	auto pipeline = mPipelineManager->createPipeline(primType, mCurrentShader);
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	//if(mCurrentVB[0])
	//   vertexStart += mCurrentVB[0]->mBufferVertexOffset;

	//if(mDrawInstancesCount)
	//   glDrawArraysInstanced(GFXVulkanPrimType[primType], vertexStart, primCountToIndexCount(primType, primitiveCount), mDrawInstancesCount);
	//else
	//   glDrawArrays(GFXVulkanPrimType[primType], vertexStart, primCountToIndexCount(primType, primitiveCount));   

	postDrawPrimitive(primitiveCount);
}

void GFXVulkanDevice::drawIndexedPrimitive(GFXPrimitiveType primType,
                                           U32 startVertex,
                                           U32 minIndex,
                                           U32 numVerts,
                                           U32 startIndex,
                                           U32 primitiveCount)
{
	preDrawPrimitive();

	//U16* buf = (U16*)static_cast<GFXVulkanPrimitiveBuffer*>(mCurrentPrimitiveBuffer.getPointer())->getBuffer() + startIndex + mCurrentPrimitiveBuffer->mVolatileStart;

	//const U32 baseVertex = mCurrentVB[0]->mBufferVertexOffset + startVertex;

	//if(mDrawInstancesCount)
	//   glDrawElementsInstancedBaseVertex(GFXVulkanPrimType[primType], primCountToIndexCount(primType, primitiveCount), Vulkan_UNSIGNED_SHORT, buf, mDrawInstancesCount, baseVertex);
	//else
	//   glDrawElementsBaseVertex(GFXVulkanPrimType[primType], primCountToIndexCount(primType, primitiveCount), Vulkan_UNSIGNED_SHORT, buf, baseVertex);

	postDrawPrimitive(primitiveCount);
}

//void GFXVulkanDevice::setPB(GFXVulkanPrimitiveBuffer* pb)
//{
//   if(mCurrentPB)
//      mCurrentPB->finish();
//   mCurrentPB = pb;
//}

void GFXVulkanDevice::setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable)
{
	// ONLY NEEDED ON FFP
}

void GFXVulkanDevice::setLightMaterialInternal(const GFXLightMaterial mat)
{
	// ONLY NEEDED ON FFP
}

void GFXVulkanDevice::setGlobalAmbientInternal(LinearColorF color)
{
	// ONLY NEEDED ON FFP
}

void GFXVulkanDevice::setTextureInternal(U32 textureUnit, const GFXTextureObject* texture)
{
	GFXVulkanTextureObject* tex = static_cast<GFXVulkanTextureObject*>(const_cast<GFXTextureObject*>(texture));
}

void GFXVulkanDevice::setCubemapInternal(U32 textureUnit, const GFXVulkanCubemap* texture)
{
	if (texture)
	{
	}
}

void GFXVulkanDevice::setMatrix(GFXMatrixType mtype, const MatrixF& mat)
{
	// ONLY NEEDED ON FFP
}

void GFXVulkanDevice::setClipRect(const RectI& inRect)
{
	AssertFatal(mCurrentRT.isValid(),
		"GFXVulkanDevice::setClipRect - must have a render target set to do any rendering operations!");

	//// Clip the rect against the renderable size.
	//Point2I size = mCurrentRT->getSize();
	//RectI maxRect(Point2I(0,0), size);
	//mClip = inRect;
	//mClip.intersect(maxRect);

	//// Create projection matrix.  See http://www.opengl.org/documentation/specs/man_pages/hardcopy/Vulkan/html/gl/ortho.html
	//const F32 left = mClip.point.x;
	//const F32 right = mClip.point.x + mClip.extent.x;
	//const F32 bottom = mClip.extent.y;
	//const F32 top = 0.0f;
	//const F32 nearPlane = 0.0f;
	//const F32 farPlane = 1.0f;
	//
	//const F32 tx = -(right + left)/(right - left);
	//const F32 ty = -(top + bottom)/(top - bottom);
	//const F32 tz = -(farPlane + nearPlane)/(farPlane - nearPlane);
	//
	//static Point4F pt;
	//pt.set(2.0f / (right - left), 0.0f, 0.0f, 0.0f);
	//mProjectionMatrix.setColumn(0, pt);
	//
	//pt.set(0.0f, 2.0f/(top - bottom), 0.0f, 0.0f);
	//mProjectionMatrix.setColumn(1, pt);
	//
	//pt.set(0.0f, 0.0f, -2.0f/(farPlane - nearPlane), 0.0f);
	//mProjectionMatrix.setColumn(2, pt);
	//
	//pt.set(tx, ty, tz, 1.0f);
	//mProjectionMatrix.setColumn(3, pt);
	//
	//// Translate projection matrix.
	//static MatrixF translate(true);
	//pt.set(0.0f, -mClip.point.y, 0.0f, 1.0f);
	//translate.setColumn(3, pt);
	//
	//mProjectionMatrix *= translate;
	//
	//setMatrix(GFXMatrixProjection, mProjectionMatrix);
	//
	//MatrixF mTempMatrix(true);
	//setViewMatrix( mTempMatrix );
	//setWorldMatrix( mTempMatrix );

	//// Set the viewport to the clip rect
	//RectI viewport(mClip.point.x, mClip.point.y, mClip.extent.x, mClip.extent.y);
	//setViewport(viewport);
}

/// Creates a state block object based on the desc passed in.  This object
/// represents an immutable state.
GFXStateBlockRef GFXVulkanDevice::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
	return GFXStateBlockRef(new GFXVulkanStateBlock(desc));
	//   return GFXStateBlockRef();
}

/// Activates a stateblock
void GFXVulkanDevice::setStateBlockInternal(GFXStateBlock* block, bool force)
{
	AssertFatal(dynamic_cast<GFXVulkanStateBlock*>(block), "GFXVulkanDevice::setStateBlockInternal - Incorrect stateblock type for this device!");
	GFXVulkanStateBlock* vkBlock = static_cast<GFXVulkanStateBlock*>(block);
	GFXVulkanStateBlock* vkCurrent = static_cast<GFXVulkanStateBlock*>(mCurrentStateBlock.getPointer());
	if (force)
	   vkCurrent = NULL;
	   
	vkBlock->activate(vkCurrent); // Doesn't use current yet.
	mCurrentVulkanStateBlock = vkBlock;
}

//------------------------------------------------------------------------------

GFXTextureTarget* GFXVulkanDevice::allocRenderToTextureTarget()
{
	GFXVulkanTextureTarget* targ = new GFXVulkanTextureTarget();
	targ->registerResourceWithDevice(this);
	return targ;
}

bool GFXVulkanDevice::createFence(VkFence& fence, bool signaled)
{
	VkFenceCreateInfo fence_create_info;
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext = nullptr;
	fence_create_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	if (VK_SUCCESS != vkCreateFence(logical_device, &fence_create_info, nullptr, &fence))
	{
		Con::errorf("Could not create a fence");
		return false;
	}
	return true;
}


void GFXVulkanDevice::createFences(U32 mNumFences)
{
	mFences.clear();
	mFences.resize(mNumFences);
	// Allocate the new fences
	for (U32 i = 0; i < mNumFences; i++)
	{
		VkFence fence;
		createFence(fence);
		mFences.push_back(fence);
	}
}

void GFXVulkanDevice::waitForFences()
{
	VkBool32 wait_for_all = VK_TRUE;
	U64 timeout = 500;
	if (mFences.empty())
		return;

	if (VK_SUCCESS != vkWaitForFences(logical_device, static_cast<U32>(mFences.size()), mFences.data(), wait_for_all,
	                                  timeout))
	{
		Con::errorf("Waiting on fence failed");
	}
}

bool GFXVulkanDevice::resetFences()
{
	if (VK_SUCCESS != vkResetFences(logical_device, static_cast<U32>(mFences.size()), mFences.data()))
	{
		Con::errorf("Error occurred when trying to reset fences.");
		return false;
	}
	return true;
}

void GFXVulkanDevice::destroyFence(VkFence& fence)
{
	if (VK_NULL_HANDLE != fence)
	{
		vkDestroyFence(logical_device, fence, nullptr);
		fence = VK_NULL_HANDLE;
	}
}

GFXOcclusionQuery* GFXVulkanDevice::createOcclusionQuery()
{
	//GFXOcclusionQuery *query = new GFXVulkanOcclusionQuery( this );
	//query->registerResourceWithDevice(this);
	//return query;
	return nullptr;
}

void GFXVulkanDevice::setupGenericShaders(GenericShaderType type)
{
	AssertFatal(type != GSTargetRestore, "");

	if (mGenericShader[GSColor] == nullptr)
	{
		ShaderData* shaderData;

		shaderData = new ShaderData();
		shaderData->setField("SpirvVertexShaderFile",
		                     ShaderGen::smCommonShaderPath + String("/fixedFunction/spirv/colorV.spv"));
		shaderData->setField("SpirvPixelShaderFile",
		                     ShaderGen::smCommonShaderPath + String("/fixedFunction/spirv/colorP.spv"));
		shaderData->setField("pixVersion", "2.0");
		shaderData->registerObject();
		mGenericShader[GSColor] = shaderData->getShader();
		static_cast<GFXVulkanShader*>(mGenericShader[GSColor].getPointer())->setVertexFormat(getGFXVertexFormat<GFXVertexPC>());
		mGenericShaderBuffer[GSColor] = mGenericShader[GSColor]->allocConstBuffer();
		mModelViewProjSC[GSColor] = mGenericShader[GSColor]->getShaderConstHandle("$modelView");
		Sim::getRootGroup()->addObject(shaderData);

		shaderData = new ShaderData();
		shaderData->setField("SpirvVertexShaderFile",
		                     ShaderGen::smCommonShaderPath + String("/fixedFunction/spirv/modColorTextureV.spv"));
		shaderData->setField("SpirvPixelShaderFile",
		                     ShaderGen::smCommonShaderPath + String("/fixedFunction/spirv/modColorTextureP.spv"));
		shaderData->setSamplerName("$diffuseMap", 0);
		shaderData->setField("pixVersion", "2.0");
		shaderData->registerObject();
		mGenericShader[GSModColorTexture] = shaderData->getShader();
		static_cast<GFXVulkanShader*>(mGenericShader[GSModColorTexture].getPointer())->setVertexFormat(getGFXVertexFormat<GFXVertexPCT>());
		mGenericShaderBuffer[GSModColorTexture] = mGenericShader[GSModColorTexture]->allocConstBuffer();
		mModelViewProjSC[GSModColorTexture] = mGenericShader[GSModColorTexture]->getShaderConstHandle("$modelView");
		Sim::getRootGroup()->addObject(shaderData);

		shaderData = new ShaderData();
		shaderData->setField("SpirvVertexShaderFile",
		                     ShaderGen::smCommonShaderPath + String("/fixedFunction/spirv/addColorTextureV.spv"));
		shaderData->setField("SpirvPixelShaderFile",
		                     ShaderGen::smCommonShaderPath + String("/fixedFunction/spirv/addColorTextureP.spv"));
		shaderData->setSamplerName("$diffuseMap", 0);
		shaderData->setField("pixVersion", "2.0");
		shaderData->registerObject();
		mGenericShader[GSAddColorTexture] = shaderData->getShader();
		static_cast<GFXVulkanShader*>(mGenericShader[GSAddColorTexture].getPointer())->setVertexFormat(getGFXVertexFormat<GFXVertexPCT>());
		mGenericShaderBuffer[GSAddColorTexture] = mGenericShader[GSAddColorTexture]->allocConstBuffer();
		mModelViewProjSC[GSAddColorTexture] = mGenericShader[GSAddColorTexture]->getShaderConstHandle("$modelView");
		Sim::getRootGroup()->addObject(shaderData);

		shaderData = new ShaderData();
		shaderData->setField("SpirvVertexShaderFile",
		                     ShaderGen::smCommonShaderPath + String("/fixedFunction/spirv/textureV.spv"));
		shaderData->setField("SpirvPixelShaderFile",
		                     ShaderGen::smCommonShaderPath + String("/fixedFunction/spirv/textureP.spv"));
		shaderData->setSamplerName("$diffuseMap", 0);
		shaderData->setField("pixVersion", "2.0");
		shaderData->registerObject();
		mGenericShader[GSTexture] = shaderData->getShader();
		static_cast<GFXVulkanShader*>(mGenericShader[GSTexture].getPointer())->setVertexFormat(getGFXVertexFormat<GFXVertexPT>());
		mGenericShaderBuffer[GSTexture] = mGenericShader[GSTexture]->allocConstBuffer();
		mModelViewProjSC[GSTexture] = mGenericShader[GSTexture]->getShaderConstHandle("$modelView");
		Sim::getRootGroup()->addObject(shaderData);
	}

	MatrixF tempMatrix = mProjectionMatrix * mViewMatrix * mWorldMatrix[mWorldStackSize];
	mGenericShaderBuffer[type]->setSafe(mModelViewProjSC[type], tempMatrix);

	setShader(mGenericShader[type]);
	setShaderConstBuffer(mGenericShaderBuffer[type]);
}

GFXShader* GFXVulkanDevice::createShader()
{
	GFXVulkanShader* shader = new GFXVulkanShader();
	shader->registerResourceWithDevice( this );
	return shader;
}

void GFXVulkanDevice::setShader(GFXShader* shader, bool force)
{
	if (mCurrentShader == shader && !force)
		return;

	if ( shader )
	{
	//   GFXVulkanShader *glShader = static_cast<GFXVulkanShader*>( shader );
	//   glShader->useProgram();
	   mCurrentShader = shader;
	}
	else
	{
		setupGenericShaders();
	}
}

void GFXVulkanDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
	PROFILE_SCOPE(GFXVulkanDevice_setShaderConstBufferInternal);
	static_cast<GFXVulkanShaderConstBuffer*>(buffer)->activate();
}

U32 GFXVulkanDevice::getNumSamplers() const
{
	return getMin((U32)TEXTURE_STAGE_COUNT, mPixelShaderVersion > 0.001f ? mMaxShaderTextures : mMaxFFTextures);
}

GFXTextureObject* GFXVulkanDevice::getDefaultDepthTex() const
{
	//if(mWindowRT && mWindowRT->getPointer())
	//   return static_cast<GFXVulkanWindowTarget*>( mWindowRT->getPointer() )->mBackBufferDepthTex.getPointer();

	return nullptr;
}

GFXVulkanWindowTarget* GFXVulkanDevice::getWindowTarget() const
{
	if(mWindowRT && mWindowRT->getPointer())
	   return static_cast<GFXVulkanWindowTarget*>( mWindowRT->getPointer());

	return nullptr;
}

VkRenderPass GFXVulkanDevice::getRenderPass() const
{
	auto wintarget = getWindowTarget();
   return wintarget->getRenderPass();
}


U32 GFXVulkanDevice::getNumRenderTargets() const
{
	return mMaxTRColors;
}


void GFXVulkanDevice::_updateRenderTargets()
{
	if (mRTDirty || mCurrentRT->isPendingState())
	{
		if (mRTDeactivate)
		{
			mRTDeactivate->deactivate();
			mRTDeactivate = nullptr;
		}

		// NOTE: The render target changes is not really accurate
		// as the GFXTextureTarget supports MRT internally.  So when
		// we activate a GFXTarget it could result in multiple calls
		// to SetRenderTarget on the actual device.
		mDeviceStatistics.mRenderTargetChanges++;

		GFXVulkanTextureTarget *tex = dynamic_cast<GFXVulkanTextureTarget*>( mCurrentRT.getPointer() );
		if ( tex )
		{
		   tex->applyState();
		   tex->makeActive();
		}
		else
		{
		   GFXVulkanWindowTarget *win = dynamic_cast<GFXVulkanWindowTarget*>( mCurrentRT.getPointer() );
		   AssertFatal( win != NULL, 
		               "GFXVulkanDevice::_updateRenderTargets() - invalid target subclass passed!" );
		   
		   win->makeActive();
		   
		   if( win->mContext != static_cast<GFXVulkanDevice*>(GFX)->mContext )
		   {
		      mRTDirty = false;
		      GFX->updateStates(true);
		   }
		}

		mRTDirty = false;
	}

	if (mViewportDirty)
	{
		//glViewport( mViewport.point.x, mViewport.point.y, mViewport.extent.x, mViewport.extent.y ); 
		//mViewportDirty = false;
	}
}

//void GFXVulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
//	VkBufferCreateInfo bufferInfo = {};
//	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	bufferInfo.size = size;
//	bufferInfo.usage = usage;
//	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create buffer!");
//	}

//	VkMemoryRequirements memRequirements;
//	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

//	VkMemoryAllocateInfo allocInfo = {};
//	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	allocInfo.allocationSize = memRequirements.size;
//	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

//	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
//		throw std::runtime_error("failed to allocate buffer memory!");
//	}

//	vkBindBufferMemory(device, buffer, bufferMemory, 0);
//}


GFXFormat GFXVulkanDevice::selectSupportedFormat(GFXTextureProfile* profile,
                                                 const Vector<GFXFormat>& formats,
                                                 bool texture,
                                                 bool mustblend,
                                                 bool mustfilter)
{
	//for(U32 i = 0; i < formats.size(); i++)
	//{
	//   // Single channel textures are not supported by FBOs.
	//   if(profile->testFlag(GFXTextureProfile::RenderTarget) && (formats[i] == GFXFormatA8 || formats[i] == GFXFormatL8 || formats[i] == GFXFormatL16))
	//      continue;
	//   if(GFXVulkanTextureInternalFormat[formats[i]] == Vulkan_ZERO)
	//      continue;
	//   
	//   return formats[i];
	//}

	return GFXFormatR8G8B8A8;
}

//U32 GFXVulkanDevice::getTotalVideoMemory_Vulkan_EXT()
//{
//   return 0;
//}

void GFXVulkanDevice::assignQueue(VkSurfaceKHR surface)
{
	bool foundGraphicsQueueFamily = false;
	bool foundPresentQueueFamily = false;

	// Among all queues, select a queue that supports presentation
	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		VkBool32 presentSupport = false;
		VkResult err = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
		if (err)
		{
			Con::errorf("err");
		}


		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueueFamily = i;
			foundGraphicsQueueFamily = true;

			// Store this queues index
			if (presentSupport)
			{
				presentQueueFamily = i;
				foundPresentQueueFamily = true;
				break;
			}
		}

		if (!foundPresentQueueFamily && presentSupport)
		{
			presentQueueFamily = i;
			foundPresentQueueFamily = true;
		}
	}


	if (foundGraphicsQueueFamily)
	{
		Con::printf("queue family #%d supports graphics", graphicsQueueFamily);

		if (foundPresentQueueFamily)
		{
			Con::printf("queue family #%d supports presentation", presentQueueFamily);
		}
		else
		{
			Con::errorf("could not find a valid queue family with present support");
			exit(1);
		}
	}
	else
	{
		Con::errorf("could not find a valid queue family with graphics support");
		exit(1);
	}

	vkGetDeviceQueue(logical_device, graphicsQueueFamily, 0, &queue);
}

VkCommandBuffer GFXVulkanDevice::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logical_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}


void GFXVulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(logical_device, commandPool, 1, &commandBuffer);
}

GFXRenderPassImpl* GFXVulkanDevice::makeRenderPassImpl(RenderPassManager* main)
{
	auto temp = new GFXVulkanRenderPassImpl();
	return temp;
}

GFXRenderBinImpl* GFXVulkanDevice::makeRenderBinImpl(RenderBinManager* main)
{
	std::vector<VkAttachmentReference> inputAttachments;
                                               std::vector<VkAttachmentReference> colorAttachments;
                                               std::vector<VkAttachmentReference> resolveAttachments;
                                               VkAttachmentReference const* depthStencilAttachments = NULL;
                                               std::vector<U32> preserveAttachments;

    auto temp = new GFXVulkanRenderBinImpl(VK_PIPELINE_BIND_POINT_GRAPHICS, inputAttachments, colorAttachments, resolveAttachments, depthStencilAttachments, preserveAttachments);
	return temp;
}


bool GFXVulkanDevice::createCommandPool(U32 queue_family, VkCommandPool& command_pool,
                                        VkCommandPoolCreateFlags parameters)
{
	VkCommandPoolCreateInfo command_pool_create_info = {};
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.pNext = nullptr;
	command_pool_create_info.flags = parameters;
	command_pool_create_info.queueFamilyIndex = queue_family;
	if (VK_SUCCESS != vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr, &command_pool))
	{
		Con::errorf("Could not create command pool.");
		return false;
	}
	return true;
}

bool GFXVulkanDevice::allocateCommandBuffer(VkCommandPool& command_pool, VkCommandBufferLevel level,
                                            U32 command_buffer_count, std::vector<VkCommandBuffer>& command_buffers)
{
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.pNext = nullptr;
	command_buffer_allocate_info.commandPool = command_pool;
	command_buffer_allocate_info.level = level;
	command_buffer_allocate_info.commandBufferCount = command_buffer_count;

	command_buffers.resize(command_buffer_count);

	if (VK_SUCCESS != vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, &command_buffers[0]))
	{
		Con::errorf("Could not allocate command buffers.");
		return false;
	}
	return true;
}

bool GFXVulkanDevice::beginCommandBuffer(VkCommandBuffer& command_buffer, VkCommandBufferUsageFlags usage)
{
	VkCommandBufferInheritanceInfo* secondary_command_buffer_info = nullptr;

	VkCommandBufferBeginInfo command_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		usage,
		secondary_command_buffer_info
	};

	if (VK_SUCCESS != vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info))
	{
		Con::errorf("Could not begin command buffer recording operation.");
		return false;
	}

	return true;
}

bool GFXVulkanDevice::endCommandBuffer(VkCommandBuffer& command_buffer)
{
	if (VK_SUCCESS != vkEndCommandBuffer(command_buffer))
	{
		Con::errorf("Error occurred during command buffer recording");
		return false;
	}
	return true;
}

bool GFXVulkanDevice::resetCommandBuffer(VkCommandBuffer& command_buffer, bool release)
{
	if (VK_SUCCESS != vkResetCommandBuffer(command_buffer, release ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0))
	{
		Con::errorf("Error occurred during command buffer reset.");
		return false;
	}
	return true;
}

bool GFXVulkanDevice::resetCommandPool(VkCommandPool& command_pool, bool release)
{
	if (VK_SUCCESS != vkResetCommandPool(logical_device, command_pool,
	                                     release ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0))
	{
		Con::errorf("Error occurred during command pool reset.");
		return false;
	}
	return true;
}

bool GFXVulkanDevice::sendCommandBuffersToQueue(VkQueue queue, std::vector<VkSemaphore> wait_semaphore_handles,
                                                std::vector<VkPipelineStageFlags> wait_semaphore_stages,
                                                std::vector<VkCommandBuffer> command_buffers,
                                                std::vector<VkSemaphore> signal_semaphores, VkFence fence)
{
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.waitSemaphoreCount = wait_semaphore_handles.size();
	submit_info.pWaitSemaphores = wait_semaphore_handles.empty() ? nullptr : wait_semaphore_handles.data();
	submit_info.pWaitDstStageMask = wait_semaphore_stages.empty() ? nullptr : wait_semaphore_stages.data();
	submit_info.commandBufferCount = command_buffers.size();
	submit_info.pCommandBuffers = command_buffers.empty() ? nullptr : command_buffers.data();
	submit_info.signalSemaphoreCount = signal_semaphores.size();
	submit_info.pSignalSemaphores = signal_semaphores.empty() ? nullptr : signal_semaphores.data();

	if (VK_SUCCESS != vkQueueSubmit(queue, 1, &submit_info, fence))
	{
		Con::errorf("Error occurred during command buffer submission.");
		return false;
	}
	return true;
}

void GFXVulkanDevice::freeCommandBuffers(VkCommandPool& command_pool, std::vector<VkCommandBuffer> command_buffers)
{
	if (!command_buffers.empty())
	{
		vkFreeCommandBuffers(logical_device, command_pool, static_cast<U32>(command_buffers.size()), command_buffers.data());
		command_buffers.clear();
	}
}

void GFXVulkanDevice::destroyCommandPool(VkCommandPool& command_pool)
{
	if (VK_NULL_HANDLE != command_pool)
	{
		vkDestroyCommandPool(logical_device, command_pool, nullptr);
		command_pool = VK_NULL_HANDLE;
	}
}

bool GFXVulkanDevice::createSemaphore(VkSemaphore& semaphore)
{
	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (VK_SUCCESS != vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &semaphore))
	{
		Con::errorf("Could not create a semaphore");
		return false;
	}
	return true;
}

void GFXVulkanDevice::destroySemaphore(VkSemaphore& semaphore)
{
	if (VK_NULL_HANDLE != semaphore)
	{
		vkDestroySemaphore(logical_device, semaphore, nullptr);
		semaphore = VK_NULL_HANDLE;
	}
}


bool GFXVulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer)
{
	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.pNext = nullptr;
	buffer_create_info.flags = 0;
	buffer_create_info.size = size;
	buffer_create_info.usage = usage;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.queueFamilyIndexCount = 0;
	buffer_create_info.pQueueFamilyIndices = nullptr;

	const auto result = vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &buffer);
	if (result != VK_SUCCESS )
	{
		Con::errorf("Could not create a buffer");
		return false;
	}
	return true;
}

bool GFXVulkanDevice::allocateAndBindMemoryObjectForBuffer(VkBuffer& buffer, VkDeviceMemory& memory_object,
                                                           VkMemoryPropertyFlagBits memory_properties) const
{
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physical_device_memory_properties);
	VkMemoryRequirements memory_requirements = {};
	vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);

	for (U32 type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type)
	{
		if ((memory_requirements.memoryTypeBits & (1 << type)) &&
			((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_properties) == memory_properties))
		{
			VkMemoryAllocateInfo buffer_memory_allocate_info = {
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				memory_requirements.size,
				type
			};

			const auto result = vkAllocateMemory(logical_device, &buffer_memory_allocate_info, nullptr, &memory_object);
			if ( result == VK_SUCCESS)
				break;
		}
	}

	if (VK_NULL_HANDLE == memory_object)
	{
		Con::errorf("Could not allocate memory for a buffer.");
		return false;
	}

	const auto result = vkBindBufferMemory(logical_device, buffer, memory_object, 0);
	if (result != VK_SUCCESS )
	{
		Con::errorf("Could not bind memory object to a buffer");
		return false;
	}
	return true;
}

void GFXVulkanDevice::setBufferMemoryBarrier(	std::vector<VkBuffer>& buffers,
	                            std::vector<VkAccessFlags>& current_access,
	                            std::vector<VkAccessFlags>& newaccess,
	                            std::vector<U32>& currentQueueFamily,
	                            std::vector<U32>& NewQueueFamily, VkCommandBuffer& command_buffer, VkPipelineStageFlags generating_stages,
                                             VkPipelineStageFlags consuming_stages)
{
	std::vector<VkBufferMemoryBarrier> buffer_memory_barriers;
	for (auto i = 0; i < buffers.size(); ++i)
	{
		VkBufferMemoryBarrier memory_barrier = {};
		memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		memory_barrier.pNext = nullptr;
		memory_barrier.srcAccessMask = current_access[i];
		memory_barrier.dstAccessMask = newaccess[i];
		memory_barrier.srcQueueFamilyIndex = currentQueueFamily[i];
		memory_barrier.dstQueueFamilyIndex = NewQueueFamily[i];
		memory_barrier.buffer = buffers[i];
		memory_barrier.offset = 0;
		memory_barrier.size = VK_WHOLE_SIZE;
		buffer_memory_barriers.push_back(memory_barrier);
	}
	if (!buffer_memory_barriers.empty())
		vkCmdPipelineBarrier(command_buffer, generating_stages, consuming_stages, 0, 0, nullptr,
		                     static_cast<U32>(buffer_memory_barriers.size()), buffer_memory_barriers.data(), 0, nullptr);
}

bool GFXVulkanDevice::createBufferView(VkBuffer& buffer, VkFormat format, VkDeviceSize memory_offset,
                                       VkDeviceSize memory_range, VkBufferView& buffer_view)
{
	VkBufferViewCreateInfo buffer_view_create_info = {};
	buffer_view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_view_create_info.pNext = nullptr;
	buffer_view_create_info.flags = 0;
	buffer_view_create_info.buffer = buffer;
	buffer_view_create_info.format = format;
	buffer_view_create_info.offset = memory_offset;
	buffer_view_create_info.range = memory_range;

	if (VK_SUCCESS != vkCreateBufferView(logical_device, &buffer_view_create_info, nullptr, &buffer_view))
	{
		Con::errorf("Could not create buffer view.");
		return false;
	}
	return true;
}

bool GFXVulkanDevice::createImage(VkImageType imageType,
                                  VkFormat format,
                                  VkExtent3D size,
                                  U32 num_mipmaps,
                                  U32 num_layers,
                                  VkSampleCountFlagBits samples,
                                  VkImageUsageFlags usage_scenarios,
                                  bool isCubemap,
                                  VkImage& image)
{
	VkImageCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.pNext = nullptr;
	create_info.flags = isCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.format = format;
	create_info.extent = size;
	create_info.mipLevels = num_mipmaps;
	create_info.arrayLayers = isCubemap ? 6 * num_layers : num_layers;
	create_info.samples = samples;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.usage = usage_scenarios;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = nullptr;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;


	if (VK_SUCCESS != vkCreateImage(logical_device, &create_info, nullptr, &image))
	{
		Con::errorf("GFXVulkanDevice::createImage Could not create an image.");
		return false;
	}
	return true;
}

void GFXVulkanDevice::destroyImage(VkImage& image)
{
	if (VK_NULL_HANDLE != image)
	{
		vkDestroyImage(logical_device, image, nullptr);
		image = VK_NULL_HANDLE;
	}
}

bool GFXVulkanDevice::allocateAndBindMemoryObjectToImage(VkImage image, VkMemoryPropertyFlagBits memory_properties,
                                                         VkDeviceMemory& memory_object)
{
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties = {};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physical_device_memory_properties);
	VkMemoryRequirements memory_requirements = {};
	vkGetImageMemoryRequirements(logical_device, image, &memory_requirements);

	for (U32 type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type)
		if ((memory_requirements.memoryTypeBits & (1 << type)) &&
			((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_properties) == memory_properties))
		{
			VkMemoryAllocateInfo image_memory_allocate_info = {
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				memory_requirements.size,
				type
			};

			if (VK_SUCCESS == vkAllocateMemory(logical_device, &image_memory_allocate_info, nullptr, &memory_object))
			{
				break;
			}
		}

	if (VK_NULL_HANDLE == memory_object)
	{
		Con::errorf("Could not allocate memory for an image.");
		return false;
	}

	if (VK_SUCCESS != vkBindImageMemory(logical_device, image, memory_object, 0))
	{
		Con::errorf("Could not bind memory object to an image");
		return false;
	}

	return true;
}

void GFXVulkanDevice::setImageMemoryBarrier(std::vector<VkImage>& Image,
	                           std::vector<VkAccessFlags>& CurrentAccess,
	                           std::vector<VkAccessFlags>& NewAccess,
	                           std::vector<VkImageLayout>& CurrentLayout,
	                           std::vector<VkImageLayout>& NewLayout,
	                           std::vector<U32>& CurrentQueueFamily,
	                           std::vector<U32>& NewQueueFamily,
	                           std::vector<VkImageAspectFlags> Aspect,
                                            VkCommandBuffer& command_buffer, VkPipelineStageFlags generating_stages,
                                            VkPipelineStageFlags consuming_stages)
{
	std::vector<VkImageMemoryBarrier> image_memory_barriers;
	for (auto i = 0; i < Image.size(); ++i)
	{
		VkImageMemoryBarrier temp = {};
		temp.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		temp.pNext = nullptr;
		temp.srcAccessMask = CurrentAccess[i];
		temp.dstAccessMask = NewAccess[i];
		temp.oldLayout = CurrentLayout[i];
		temp.newLayout = NewLayout[i];
		temp.srcQueueFamilyIndex = CurrentQueueFamily[i];
		temp.dstQueueFamilyIndex = NewQueueFamily[i];
		temp.image = Image[i];
		temp.subresourceRange = {
			Aspect[i],
			0,
			VK_REMAINING_MIP_LEVELS,
			0,
			VK_REMAINING_ARRAY_LAYERS
		};

		image_memory_barriers.push_back(temp);
	}
	if (!image_memory_barriers.empty())
		vkCmdPipelineBarrier(command_buffer, generating_stages, consuming_stages, 0, 0, nullptr, 0, nullptr,
		                     static_cast<U32>(image_memory_barriers.size()), image_memory_barriers.data());
}

void GFXVulkanDevice::freeMemoryObject(VkDeviceMemory& memory_object)
{
	if (VK_NULL_HANDLE != memory_object)
	{
		vkFreeMemory(logical_device, memory_object, nullptr);
		memory_object = VK_NULL_HANDLE;
	}
}

bool GFXVulkanDevice::createImageView(VkImage image, VkImageViewType viewtype, VkFormat format,
                                      VkImageAspectFlags aspectmask, VkImageView& imageView)
{
	VkImageViewCreateInfo image_view_create_info = {};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.pNext = nullptr;
	image_view_create_info.flags = 0;
	image_view_create_info.image = image;
	image_view_create_info.viewType = viewtype;
	image_view_create_info.format = format;
	image_view_create_info.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	image_view_create_info.subresourceRange.aspectMask = aspectmask;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	if (VK_SUCCESS != vkCreateImageView(logical_device, &image_view_create_info, nullptr, &imageView))
	{
		Con::errorf("Could not create an image view.");
		return false;
	}
	return true;
}

void GFXVulkanDevice::destroyImageView(VkImageView& image_view)
{
	if (VK_NULL_HANDLE != image_view)
	{
		vkDestroyImageView(logical_device, image_view, nullptr);
		image_view = VK_NULL_HANDLE;
	}
}

bool GFXVulkanDevice::mapHostVisibleMemory(VkDeviceMemory memory_object, VkDeviceSize offset, VkDeviceSize data_size,
                                           void* local_pointer, bool unmap)
{
	void* data;
	if (VK_SUCCESS != vkMapMemory(logical_device, memory_object, offset, data_size, 0, &local_pointer))
	{
		Con::errorf("Could not map memory object.");
		return false;
	}
	std::memcpy(local_pointer, data, data_size);
	std::vector<VkMappedMemoryRange> memory_ranges = {
		{
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			nullptr,
			memory_object,
			offset,
			data_size
		}
	};
	if (VK_SUCCESS != vkFlushMappedMemoryRanges(logical_device, static_cast<U32>(memory_ranges.size()),
	                                            memory_ranges.data()))
	{
		Con::errorf("Could not flush mapped memory.");
		return false;
	}

	//if (unmap)
	//{
	//	vkUnmapMemory(logical_device, memory_object);
	//}
	//else if (nullptr != pointer)
	//{
	//	*pointer = local_pointer;
	//}

	return true;
}

void GFXVulkanDevice::copyDataBetweenBuffers(VkCommandBuffer command_buffer, VkBuffer source_buffer,
                                             VkBuffer destination_buffer, std::vector<VkBufferCopy> regions)
{
	if (!regions.empty())
		vkCmdCopyBuffer(command_buffer, source_buffer, destination_buffer, static_cast<U32>(regions.size()), regions.data());
}

void GFXVulkanDevice::copyDataFromBufferToImage(VkCommandBuffer command_buffer, VkBuffer source_buffer,
                                                VkImage destination_image, VkImageLayout image_layout,
                                                std::vector<VkBufferImageCopy> regions)
{
	if (!regions.empty())
		vkCmdCopyBufferToImage(command_buffer, source_buffer, destination_image, image_layout,
		                       static_cast<U32>(regions.size()), regions.data());
}

void GFXVulkanDevice::copyDataFromImageToBuffer(VkCommandBuffer command_buffer, VkImage source_image,
                                                VkImageLayout image_layout, VkBuffer destination_buffer,
                                                std::vector<VkBufferImageCopy> regions)
{
	if (!regions.empty())
		vkCmdCopyImageToBuffer(command_buffer, source_image, image_layout, destination_buffer,
		                       static_cast<U32>(regions.size()), regions.data());
}

void GFXVulkanDevice::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	
	endSingleTimeCommands(commandBuffer);
}

bool GFXVulkanDevice::IsExtensionSupported(std::vector<char const*> in_desired_extensions)
{
	for (auto& ext : in_desired_extensions)
	{
		bool found = false;
		for (auto& ext2 : extensions)
		{
			if (strcmp(ext2.extensionName, ext) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}
	return true;
}


bool GFXVulkanDevice::setupDescriptorPool(std::vector<VkDescriptorPoolSize> poolSizes, bool free_individual_sets,
                                          U32 max_sets_count)
{
	// Example uses one ubo and one image sampler
		//VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		//vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1),
		//vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		nullptr,
		free_individual_sets ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0,
		max_sets_count,
		static_cast<uint32_t>(poolSizes.size()),
		poolSizes.data()
	};


	const auto result = vkCreateDescriptorPool(logical_device, &descriptorPoolInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS )
	{
		Con::errorf("Could not create a buffer");
		return false;
	}
	return true;
}


bool GFXVulkanDevice::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> bindings )
{
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
	VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	nullptr,
	0, 
	static_cast<U32>(bindings.size()),
	bindings.data()
	};

	VkResult result = vkCreateDescriptorSetLayout( GFXVulkan->getLogicalDevice(), &descriptor_set_layout_create_info, nullptr, &descriptorSetLayout);
	if (VK_SUCCESS != result)
	{
		Con::errorf("Could not create a layout for secriptor sets.");
		return false;
	}
	return true;
}

bool GFXVulkanDevice::CreatePipelineLayout(std::vector<VkDescriptorSetLayoutBinding> bindings)
{
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

	VkResult result1 = vkCreateDescriptorSetLayout(GFXVulkan->getLogicalDevice(), &descriptorLayout, nullptr, &descriptorSetLayout);

	std::vector<VkDescriptorSetLayout> temp;
	temp.push_back(descriptorSetLayout);
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = VulkanUtils::pipelineLayoutCreateInfo(&descriptorSetLayout);	

	VkResult result = vkCreatePipelineLayout( GFXVulkan->getLogicalDevice(), &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if (VK_SUCCESS != result)
	{
		Con::errorf("Could not create a layout for desecriptor sets.");
		return false;
	}
	return true;

}


//
// Register this device with GFXInit
//
class GFXVulkanRegisterDevice
{
public:
	GFXVulkanRegisterDevice()
	{
		GFXInit::getRegisterDeviceSignal().notify(&GFXVulkanDevice::enumerateAdapters);
	}
};

static GFXVulkanRegisterDevice pVulkanRegisterDevice;

//ConsoleFunction(cycleVulkanResources, void, 1, 1, "")
//{
//   static_cast<GFXVulkanDevice*>(GFX)->zombify();
//   static_cast<GFXVulkanDevice*>(GFX)->resurrect();
//}
