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

#ifndef TORQUE_GFX_Vulkan_GFXVulkanUTILS_H_
#define TORQUE_GFX_Vulkan_GFXVulkanUTILS_H_

#include "core/util/preprocessorHelpers.h"
#include "gfx/vulkan/gfxVulkanEnumTranslate.h"
//#include "gfx/vulkan/gfxVulkanStateCache.h"
#include "gfx/bitmap/imageUtils.h"


namespace VulkanUtils {
	inline U32 getMaxMipmaps(U32 width, U32 height, U32 depth)
	{
	   return getMax( getBinLog2(depth), getMax(getBinLog2(width), getBinLog2(height)));
	}

	struct BufferTransition
	{
		VkBuffer Buffer;
		VkAccessFlags CurrentAccess;
		VkAccessFlags NewAccess;
		U32 CurrentQueueFamily;
		U32 NewQueueFamily;
	};

	struct ImageTransition
	{
		VkImage Image;
		VkAccessFlags CurrentAccess;
		VkAccessFlags NewAccess;
		VkImageLayout CurrentLayout;
		VkImageLayout NewLayout;
		U32 CurrentQueueFamily;
		U32 NewQueueFamily;
		VkImageAspectFlags Aspect;
	};

	struct ShaderStageParameters
	{
		VkShaderStageFlagBits      ShaderStage;
		VkShaderModule	           ShaderModule;
		char const                 *EntryPointName;
		VkSpecializationInfo const *SpecializationInfo;
	};

	struct ViewportInfo
	{
		std::vector<VkViewport> Viewports;
		std::vector<VkRect2D>	Scissors;
	};

		inline VkMemoryAllocateInfo memoryAllocateInfo()
		{
			VkMemoryAllocateInfo memAllocInfo {};
			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			return memAllocInfo;
		}

		inline VkMappedMemoryRange mappedMemoryRange()
		{
			VkMappedMemoryRange mappedMemoryRange {};
			mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			return mappedMemoryRange;
		}

		inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
			VkCommandPool commandPool, 
			VkCommandBufferLevel level, 
			uint32_t bufferCount)
		{
			VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
			commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferAllocateInfo.commandPool = commandPool;
			commandBufferAllocateInfo.level = level;
			commandBufferAllocateInfo.commandBufferCount = bufferCount;
			return commandBufferAllocateInfo;
		}

		inline VkCommandPoolCreateInfo commandPoolCreateInfo()
		{
			VkCommandPoolCreateInfo cmdPoolCreateInfo {};
			cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			return cmdPoolCreateInfo;
		}

		inline VkCommandBufferBeginInfo commandBufferBeginInfo()
		{
			VkCommandBufferBeginInfo cmdBufferBeginInfo {};
			cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			return cmdBufferBeginInfo;
		}

		inline VkCommandBufferInheritanceInfo commandBufferInheritanceInfo()
		{
			VkCommandBufferInheritanceInfo cmdBufferInheritanceInfo {};
			cmdBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			return cmdBufferInheritanceInfo;
		}

		inline VkRenderPassBeginInfo renderPassBeginInfo()
		{
			VkRenderPassBeginInfo renderPassBeginInfo {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			return renderPassBeginInfo;
		}

		inline VkRenderPassCreateInfo renderPassCreateInfo()
		{
			VkRenderPassCreateInfo renderPassCreateInfo {};
			renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			return renderPassCreateInfo;
		}

		/** @brief Initialize an image memory barrier with no image transfer ownership */
		inline VkImageMemoryBarrier imageMemoryBarrier()
		{
			VkImageMemoryBarrier imageMemoryBarrier {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			return imageMemoryBarrier;
		}

		/** @brief Initialize a buffer memory barrier with no image transfer ownership */
		inline VkBufferMemoryBarrier bufferMemoryBarrier()
		{
			VkBufferMemoryBarrier bufferMemoryBarrier {};
			bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			return bufferMemoryBarrier;
		}

		inline VkMemoryBarrier memoryBarrier()
		{
			VkMemoryBarrier memoryBarrier {};
			memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			return memoryBarrier;
		}

		inline VkImageCreateInfo imageCreateInfo()
		{
			VkImageCreateInfo imageCreateInfo {};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			return imageCreateInfo;
		}

		inline VkSamplerCreateInfo samplerCreateInfo()
		{
			VkSamplerCreateInfo samplerCreateInfo {};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.maxAnisotropy = 1.0f;
			return samplerCreateInfo;
		}

		inline VkImageViewCreateInfo imageViewCreateInfo()
		{
			VkImageViewCreateInfo imageViewCreateInfo {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			return imageViewCreateInfo;
		}

		inline VkFramebufferCreateInfo framebufferCreateInfo()
		{
			VkFramebufferCreateInfo framebufferCreateInfo {};
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			return framebufferCreateInfo;
		}

		inline VkSemaphoreCreateInfo semaphoreCreateInfo()
		{
			VkSemaphoreCreateInfo semaphoreCreateInfo {};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			return semaphoreCreateInfo;
		}

		inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0)
		{
			VkFenceCreateInfo fenceCreateInfo {};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.flags = flags;
			return fenceCreateInfo;
		}

		inline VkEventCreateInfo eventCreateInfo()
		{
			VkEventCreateInfo eventCreateInfo {};
			eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
			return eventCreateInfo;
		}

		inline VkSubmitInfo submitInfo()
		{
			VkSubmitInfo submitInfo {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			return submitInfo;
		}

		inline VkViewport viewport(
			float width,
			float height,
			float minDepth,
			float maxDepth)
		{
			VkViewport viewport {};
			viewport.width = width;
			viewport.height = height;
			viewport.minDepth = minDepth;
			viewport.maxDepth = maxDepth;
			return viewport;
		}

		inline VkRect2D rect2D(
			int32_t width,
			int32_t height,
			int32_t offsetX,
			int32_t offsetY)
		{
			VkRect2D rect2D {};
			rect2D.extent.width = width;
			rect2D.extent.height = height;
			rect2D.offset.x = offsetX;
			rect2D.offset.y = offsetY;
			return rect2D;
		}

		inline VkBufferCreateInfo bufferCreateInfo()
		{
			VkBufferCreateInfo bufCreateInfo {};
			bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			return bufCreateInfo;
		}

		inline VkBufferCreateInfo bufferCreateInfo(
			VkBufferUsageFlags usage,
			VkDeviceSize size)
		{
			VkBufferCreateInfo bufCreateInfo {};
			bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufCreateInfo.usage = usage;
			bufCreateInfo.size = size;
			return bufCreateInfo;
		}

		inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
			uint32_t poolSizeCount,
			VkDescriptorPoolSize* pPoolSizes,
			uint32_t maxSets)
		{
			VkDescriptorPoolCreateInfo descriptorPoolInfo {};
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.poolSizeCount = poolSizeCount;
			descriptorPoolInfo.pPoolSizes = pPoolSizes;
			descriptorPoolInfo.maxSets = maxSets;
			return descriptorPoolInfo;
		}

		inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
			const std::vector<VkDescriptorPoolSize>& poolSizes,
			uint32_t maxSets)
		{
			VkDescriptorPoolCreateInfo descriptorPoolInfo{};
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			descriptorPoolInfo.pPoolSizes = poolSizes.data();
			descriptorPoolInfo.maxSets = maxSets;
			return descriptorPoolInfo;
		}

		inline VkDescriptorPoolSize descriptorPoolSize(
			VkDescriptorType type,
			uint32_t descriptorCount)
		{
			VkDescriptorPoolSize descriptorPoolSize {};
			descriptorPoolSize.type = type;
			descriptorPoolSize.descriptorCount = descriptorCount;
			return descriptorPoolSize;
		}

		inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
			VkDescriptorType type,
			VkShaderStageFlags stageFlags,
			uint32_t binding,
			uint32_t descriptorCount = 1)
		{
			VkDescriptorSetLayoutBinding setLayoutBinding {};
			setLayoutBinding.descriptorType = type;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorCount = descriptorCount;
			return setLayoutBinding;
		}

		inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const VkDescriptorSetLayoutBinding* pBindings,
			uint32_t bindingCount)
		{
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {};
			descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCreateInfo.pBindings = pBindings;
			descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
			return descriptorSetLayoutCreateInfo;
		}

		inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const std::vector<VkDescriptorSetLayoutBinding>& bindings)
		{
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
			descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCreateInfo.pBindings = bindings.data();
			descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			return descriptorSetLayoutCreateInfo;
		}

		inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			const VkDescriptorSetLayout* pSetLayouts,
			uint32_t setLayoutCount = 1)
		{
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
			pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
			return pipelineLayoutCreateInfo;
		}

		inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
			VkDescriptorPool descriptorPool,
			const VkDescriptorSetLayout* pSetLayouts,
			uint32_t descriptorSetCount)
		{
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.descriptorPool = descriptorPool;
			descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
			descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
			return descriptorSetAllocateInfo;
		}

		inline VkDescriptorImageInfo descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
		{
			VkDescriptorImageInfo descriptorImageInfo {};
			descriptorImageInfo.sampler = sampler;
			descriptorImageInfo.imageView = imageView;
			descriptorImageInfo.imageLayout = imageLayout;
			return descriptorImageInfo;
		}

		inline VkWriteDescriptorSet writeDescriptorSet(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorBufferInfo* bufferInfo,
			uint32_t descriptorCount = 1)
		{
			VkWriteDescriptorSet writeDescriptorSet {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pBufferInfo = bufferInfo;
			writeDescriptorSet.descriptorCount = descriptorCount;
			return writeDescriptorSet;
		}

		inline VkWriteDescriptorSet writeDescriptorSet(
			VkDescriptorSet dstSet,
			VkDescriptorType type,
			uint32_t binding,
			VkDescriptorImageInfo *imageInfo,
			uint32_t descriptorCount = 1)
		{
			VkWriteDescriptorSet writeDescriptorSet {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pImageInfo = imageInfo;
			writeDescriptorSet.descriptorCount = descriptorCount;
			return writeDescriptorSet;
		}

		inline VkVertexInputBindingDescription vertexInputBindingDescription(
			uint32_t binding,
			uint32_t stride,
			VkVertexInputRate inputRate)
		{
			VkVertexInputBindingDescription vInputBindDescription {};
			vInputBindDescription.binding = binding;
			vInputBindDescription.stride = stride;
			vInputBindDescription.inputRate = inputRate;
			return vInputBindDescription;
		}

		inline VkVertexInputAttributeDescription vertexInputAttributeDescription(
			uint32_t binding,
			uint32_t location,
			VkFormat format,
			uint32_t offset)
		{
			VkVertexInputAttributeDescription vInputAttribDescription {};
			vInputAttribDescription.location = location;
			vInputAttribDescription.binding = binding;
			vInputAttribDescription.format = format;
			vInputAttribDescription.offset = offset;
			return vInputAttribDescription;
		}

		inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo()
		{
			VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo {};
			pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			return pipelineVertexInputStateCreateInfo;
		}

		inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
			VkPrimitiveTopology topology,
			VkPipelineInputAssemblyStateCreateFlags flags,
			VkBool32 primitiveRestartEnable)
		{
			VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo {};
			pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			pipelineInputAssemblyStateCreateInfo.topology = topology;
			pipelineInputAssemblyStateCreateInfo.flags = flags;
			pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
			return pipelineInputAssemblyStateCreateInfo;
		}

		inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
			VkPolygonMode polygonMode,
			VkCullModeFlags cullMode,
			VkFrontFace frontFace,
			VkPipelineRasterizationStateCreateFlags flags = 0)
		{
			VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo {};
			pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
			pipelineRasterizationStateCreateInfo.cullMode = cullMode;
			pipelineRasterizationStateCreateInfo.frontFace = frontFace;
			pipelineRasterizationStateCreateInfo.flags = flags;
			pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
			pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
			return pipelineRasterizationStateCreateInfo;
		}

		inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
			VkColorComponentFlags colorWriteMask,
			VkBool32 blendEnable)
		{
			VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState {};
			pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
			pipelineColorBlendAttachmentState.blendEnable = blendEnable;
			return pipelineColorBlendAttachmentState;
		}

		inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
			uint32_t attachmentCount,
			const VkPipelineColorBlendAttachmentState * pAttachments)
		{
			VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo {};
			pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
			pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
			return pipelineColorBlendStateCreateInfo;
		}

		inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
			VkBool32 depthTestEnable,
			VkBool32 depthWriteEnable,
			VkCompareOp depthCompareOp)
		{
			VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo {};
			pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
			pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
			pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;
			pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;
			pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
			return pipelineDepthStencilStateCreateInfo;
		}

		inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
			uint32_t viewportCount,
			uint32_t scissorCount,
			VkPipelineViewportStateCreateFlags flags = 0)
		{
			VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo {};
			pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			pipelineViewportStateCreateInfo.viewportCount = viewportCount;
			pipelineViewportStateCreateInfo.scissorCount = scissorCount;
			pipelineViewportStateCreateInfo.flags = flags;
			return pipelineViewportStateCreateInfo;
		}

		inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
			VkSampleCountFlagBits rasterizationSamples,
			VkPipelineMultisampleStateCreateFlags flags = 0)
		{
			VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo {};
			pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
			pipelineMultisampleStateCreateInfo.flags = flags;
			return pipelineMultisampleStateCreateInfo;
		}

		inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			const VkDynamicState * pDynamicStates,
			uint32_t dynamicStateCount,
			VkPipelineDynamicStateCreateFlags flags = 0)
		{
			VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo {};
			pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates;
			pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
			pipelineDynamicStateCreateInfo.flags = flags;
			return pipelineDynamicStateCreateInfo;
		}

		inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			const std::vector<VkDynamicState>& pDynamicStates,
			VkPipelineDynamicStateCreateFlags flags = 0)
		{
			VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
			pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates.data();
			pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(pDynamicStates.size());
			pipelineDynamicStateCreateInfo.flags = flags;
			return pipelineDynamicStateCreateInfo;
		}

		inline VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints)
		{
			VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo {};
			pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
			return pipelineTessellationStateCreateInfo;
		}

		inline VkGraphicsPipelineCreateInfo pipelineCreateInfo(
			VkPipelineLayout layout,
			VkRenderPass renderPass,
			VkPipelineCreateFlags flags = 0)
		{
			VkGraphicsPipelineCreateInfo pipelineCreateInfo {};
			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCreateInfo.layout = layout;
			pipelineCreateInfo.renderPass = renderPass;
			pipelineCreateInfo.flags = flags;
			pipelineCreateInfo.basePipelineIndex = -1;
			pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
			return pipelineCreateInfo;
		}

		inline VkComputePipelineCreateInfo computePipelineCreateInfo(
			VkPipelineLayout layout, 
			VkPipelineCreateFlags flags = 0)
		{
			VkComputePipelineCreateInfo computePipelineCreateInfo {};
			computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			computePipelineCreateInfo.layout = layout;
			computePipelineCreateInfo.flags = flags;
			return computePipelineCreateInfo;
		}

		inline VkPushConstantRange pushConstantRange(
			VkShaderStageFlags stageFlags,
			uint32_t size,
			uint32_t offset)
		{
			VkPushConstantRange pushConstantRange {};
			pushConstantRange.stageFlags = stageFlags;
			pushConstantRange.offset = offset;
			pushConstantRange.size = size;
			return pushConstantRange;
		}

		inline VkBindSparseInfo bindSparseInfo()
		{
			VkBindSparseInfo bindSparseInfo{};
			bindSparseInfo.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
			return bindSparseInfo;
		}

		/** @brief Initialize a map entry for a shader specialization constant */
		inline VkSpecializationMapEntry specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size)
		{
			VkSpecializationMapEntry specializationMapEntry{};
			specializationMapEntry.constantID = constantID;
			specializationMapEntry.offset = offset;
			specializationMapEntry.size = size;
			return specializationMapEntry;
		}

		/** @brief Initialize a specialization constant info structure to pass to a shader stage */
		inline VkSpecializationInfo specializationInfo(uint32_t mapEntryCount, const VkSpecializationMapEntry* mapEntries, size_t dataSize, const void* data)
		{
			VkSpecializationInfo specializationInfo{};
			specializationInfo.mapEntryCount = mapEntryCount;
			specializationInfo.pMapEntries = mapEntries;
			specializationInfo.dataSize = dataSize;
			specializationInfo.pData = data;
			return specializationInfo;
		}
};

//inline Vulkanenum minificationFilter(U32 minFilter, U32 mipFilter, U32 /*mipLevels*/)
//{
//   // the compiler should interpret this as array lookups
//   switch( minFilter ) 
//   {
//      case GFXTextureFilterLinear:
//         switch( mipFilter ) 
//         {
//         case GFXTextureFilterLinear:
//            return Vulkan_LINEAR_MIPMAP_LINEAR;
//         case GFXTextureFilterPoint:
//            return Vulkan_LINEAR_MIPMAP_NEAREST;
//         default: 
//            return Vulkan_LINEAR;
//         }
//      default:
//         switch( mipFilter ) {
//      case GFXTextureFilterLinear:
//         return Vulkan_NEAREST_MIPMAP_LINEAR;
//      case GFXTextureFilterPoint:
//         return Vulkan_NEAREST_MIPMAP_NEAREST;
//      default:
//         return Vulkan_NEAREST;
//         }
//   }
//}
//
////Get the surface size of a compressed mip map level - see ddsLoader.cpp
//inline U32 getCompressedSurfaceSize(GFXFormat format,U32 width, U32 height, U32 mipLevel=0 )
//{
//   if(!ImageUtil::isCompressedFormat(format))
//      return 0;
//
//   // Bump by the mip level.
//   height = getMax(U32(1), height >> mipLevel);
//   width = getMax(U32(1), width >> mipLevel);
//
//   U32 sizeMultiple = 0;
//   if(format == GFXFormatBC1 || format == GFXFormatBC1_SRGB)
//      sizeMultiple = 8;
//   else
//      sizeMultiple = 16;
//
//   return getMax(U32(1), width/4) * getMax(U32(1), height/4) * sizeMultiple;
//}
//
///// Simple class which preserves a given Vulkan integer.
///// This class determines the integer to preserve on construction and restores 
///// it on destruction.
//class GFXVulkanPreserveInteger
//{
//public:
//   typedef void(STDCALL *BindFn)(Vulkanenum, Vulkanuint);
//
//   /// Preserve the integer.
//   /// @param binding The binding which should be set on destruction.
//   /// @param getBinding The parameter to be passed to VulkanGetIntegerv to determine
//   /// the integer to be preserved.
//   /// @param binder The Vulkan function to call to restore the integer.
//   GFXVulkanPreserveInteger(Vulkanenum binding, Vulkanint getBinding, BindFn binder) :
//      mBinding(binding), mPreserved(0), mBinder(binder)
//   {
//      AssertFatal(mBinder, "GFXVulkanPreserveInteger - Need a valid binder function");
//      mPreserved = GFXVulkan->getOpenVulkanCache()->getCacheBinded(mBinding);
//#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
//      Vulkanint bindedOnOpenVulkanDriver;
//      VulkanGetIntegerv(getBinding, &bindedOnOpenVulkanDriver);
//      AssertFatal( mPreserved == bindedOnOpenVulkanDriver, "GFXVulkanPreserveInteger - GFXVulkanDevice/OpenVulkan mismatch on cache binded resource.");
//#endif
//   }
//   
//   /// Restores the integer.
//   ~GFXVulkanPreserveInteger()
//   {
//      mBinder(mBinding, mPreserved);
//   }
//
//private:
//   Vulkanenum mBinding;
//   Vulkanint mPreserved;
//   BindFn mBinder;
//};
//
//class GFXVulkanPreserveTexture
//{
//public:
//   typedef void(STDCALL *BindFn)(Vulkanenum, Vulkanuint);
//   
//   GFXVulkanPreserveTexture(Vulkanenum binding, Vulkanint getBinding, BindFn binder) :
//      mBinding(binding), mPreserved(0), mBinder(binder)
//   {
//      AssertFatal(mBinder, "GFXVulkanPreserveTexture - Need a valid binder function");
//      GFXVulkanDevice *gfx = GFXVulkan;
//      mPreserved = gfx->getOpenVulkanCache()->getCacheBinded(mBinding);
//      mActiveTexture = gfx->getOpenVulkanCache()->getCacheActiveTexture();
//#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
//      Vulkanint activeTextureOnOpenVulkanDriver, bindedTextureOnOpenVulkanDriver;
//      VulkanGetIntegerv(getBinding, &bindedTextureOnOpenVulkanDriver);
//      VulkanGetIntegerv(Vulkan_ACTIVE_TEXTURE, &activeTextureOnOpenVulkanDriver);
//      activeTextureOnOpenVulkanDriver -= Vulkan_TEXTURE0;
//      AssertFatal( mPreserved == bindedTextureOnOpenVulkanDriver, "GFXVulkanPreserveTexture - GFXVulkanDevice/OpenVulkan mismatch on cache binded resource.");
//      AssertFatal( activeTextureOnOpenVulkanDriver == mActiveTexture, "GFXVulkanPreserveTexture - GFXVulkanDevice/OpenVulkan mismatch on cache binded resource.");
//#endif
//   }
//   
//   /// Restores the texture.
//   ~GFXVulkanPreserveTexture()
//   {
//#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
//      Vulkanint activeTextureOnOpenVulkanDriver;
//      VulkanGetIntegerv(Vulkan_ACTIVE_TEXTURE, &activeTextureOnOpenVulkanDriver);
//      activeTextureOnOpenVulkanDriver -= Vulkan_TEXTURE0;
//      Vulkanint cacheActiveTexture = GFXVulkan->getOpenVulkanCache()->getCacheActiveTexture();
//      AssertFatal( cacheActiveTexture == activeTextureOnOpenVulkanDriver, "GFXVulkanPreserveTexture - GFXVulkanDevice/OpenVulkan mismatch on cache ActiveTexture.");
//#endif
//      mBinder(mBinding, mPreserved);
//   }
//
//private:
//   Vulkanenum mBinding;
//   Vulkanint mPreserved;
//   BindFn mBinder;
//   S16 mActiveTexture;
//};
//
///// Helper macro to preserve the current VBO binding.
//#define PRESERVE_VERTEX_BUFFER() \
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_ARRAY_BUFFER, Vulkan_ARRAY_BUFFER_BINDING, (GFXVulkanPreserveInteger::BindFn)VulkanBindBuffer)
//
///// Helper macro to preserve the current element array binding.
//#define PRESERVE_INDEX_BUFFER() \
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_ELEMENT_ARRAY_BUFFER, Vulkan_ELEMENT_ARRAY_BUFFER_BINDING, (GFXVulkanPreserveInteger::BindFn)VulkanBindBuffer)
//
//#define _GET_BUFFER_BINDING( BINDING ) \
//BINDING == Vulkan_ARRAY_BUFFER ? Vulkan_ARRAY_BUFFER_BINDING : ( BINDING == Vulkan_ELEMENT_ARRAY_BUFFER ?  Vulkan_ELEMENT_ARRAY_BUFFER_BINDING : 0 )
//
///// Helper macro to preserve the current element array binding.
//#define PRESERVE_BUFFER( BINDING ) \
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (BINDING, _GET_BUFFER_BINDING(BINDING), (GFXVulkanPreserveInteger::BindFn)VulkanBindBuffer)
//
///// ASSERT: Never call VulkanActiveTexture for a "bind to modify" or in a PRESERVER_TEXTURE MACRO scope.
//
///// Helper macro to preserve the current 1D texture binding.
//#define PRESERVE_1D_TEXTURE() \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_TEXTURE_1D, Vulkan_TEXTURE_BINDING_1D, (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
///// Helper macro to preserve the current 2D texture binding.
//#define PRESERVE_2D_TEXTURE() \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_TEXTURE_2D, Vulkan_TEXTURE_BINDING_2D, (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
///// Helper macro to preserve the current 3D texture binding.
//#define PRESERVE_3D_TEXTURE() \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_TEXTURE_3D, Vulkan_TEXTURE_BINDING_3D, (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
///// Helper macro to preserve the current 3D texture binding.
//#define PRESERVE_CUBEMAP_TEXTURE() \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_BINDING_CUBE_MAP, (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
//#define _GET_TEXTURE_BINDING(binding) \
//binding == Vulkan_TEXTURE_2D ? Vulkan_TEXTURE_BINDING_2D : (binding == Vulkan_TEXTURE_3D ?  Vulkan_TEXTURE_BINDING_3D : Vulkan_TEXTURE_BINDING_1D )
//
//#define PRESERVE_TEXTURE(binding) \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (binding, _GET_TEXTURE_BINDING(binding), (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
//#define PRESERVE_FRAMEBUFFER() \
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_READ_FRAMEBUFFER, Vulkan_READ_FRAMEBUFFER_BINDING, (GFXVulkanPreserveInteger::BindFn)VulkanBindFramebuffer);\
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve2_, __LINE__) (Vulkan_DRAW_FRAMEBUFFER, Vulkan_DRAW_FRAMEBUFFER_BINDING, (GFXVulkanPreserveInteger::BindFn)VulkanBindFramebuffer)
//
//
//#if TORQUE_DEBUG
//
//    // Handy macro for checking the status of a framebuffer.  Framebuffers can fail in 
//    // all sorts of interesting ways, these are just the most common.  Further, no existing Vulkan profiling 
//    // tool catches framebuffer errors when the framebuffer is created, so we actually need this.
//    #define CHECK_FRAMEBUFFER_STATUS()\
//    {\
//    Vulkanenum status;\
//    status = VulkanCheckFramebufferStatus(Vulkan_FRAMEBUFFER);\
//    switch(status) {\
//    case Vulkan_FRAMEBUFFER_COMPLETE:\
//    break;\
//    case Vulkan_FRAMEBUFFER_UNSUPPORTED:\
//    AssertFatal(false, "Unsupported FBO");\
//    break;\
//    case Vulkan_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:\
//    AssertFatal(false, "Incomplete FBO Attachment");\
//    break;\
//    case Vulkan_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:\
//    AssertFatal(false, "Incomplete FBO Missing Attachment");\
//    break;\
//    case Vulkan_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:\
//    AssertFatal(false, "Incomplete FBO Draw buffer");\
//    break;\
//    case Vulkan_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:\
//    AssertFatal(false, "Incomplete FBO Read buffer");\
//    break;\
//    default:\
//    /* programming error; will fail on all hardware */\
//    AssertFatal(false, "Something really bad happened with an FBO");\
//    }\
//    }
//#else
//    #define CHECK_FRAMEBUFFER_STATUS()
//#endif //TORQUE_DEBUG
//
#endif
