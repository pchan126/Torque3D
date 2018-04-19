#pragma once
#include "gfx/gfxRenderPass.h"
#include <vulkan/vulkan.h>
#include <vector>
#include "platform/platform.h"

class GFXVulkanRenderPassImpl :public GFXRenderPassImpl
{
public:
	GFXVulkanRenderPassImpl(); 
	virtual ~GFXVulkanRenderPassImpl() {};
};

class GFXVulkanRenderBinImpl: public GFXRenderBinImpl
{
public:
	GFXVulkanRenderBinImpl(VkPipelineBindPoint pipelineType,
                                               std::vector<VkAttachmentReference> inputAttachments,
                                               std::vector<VkAttachmentReference> colorAttachments,
                                               std::vector<VkAttachmentReference> resolveAttachments,
                                               VkAttachmentReference const* depthStencilAttachments,
                                               std::vector<U32> preserveAtttachments);
	virtual ~GFXVulkanRenderBinImpl() {};
};