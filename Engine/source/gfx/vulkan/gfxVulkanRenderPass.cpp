#include "gfx/vulkan/gfxVulkanRenderPass.h"
#include "gfx/vulkan/gfxVulkanWindowTarget.h"
#include "gfxVulkanDevice.h"

GFXVulkanRenderPassImpl::GFXVulkanRenderPassImpl()
{

}

GFXVulkanRenderBinImpl::GFXVulkanRenderBinImpl(VkPipelineBindPoint pipelineType,
                                               std::vector<VkAttachmentReference> inputAttachments,
                                               std::vector<VkAttachmentReference> colorAttachments,
                                               std::vector<VkAttachmentReference> resolveAttachments,
                                               VkAttachmentReference const* depthStencilAttachments,
                                               std::vector<U32> preserveAtttachments)
{
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = pipelineType; //VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = static_cast<U32>(colorAttachments.size());
	subpass.pColorAttachments = colorAttachments.empty() ? nullptr : colorAttachments.data();
	subpass.inputAttachmentCount = static_cast<U32>(inputAttachments.size());
	subpass.pInputAttachments = inputAttachments.empty() ? nullptr :inputAttachments.data();
	subpass.pResolveAttachments = resolveAttachments.empty() ? nullptr :resolveAttachments.data();
	subpass.pDepthStencilAttachment = depthStencilAttachments;
	subpass.preserveAttachmentCount = static_cast<U32>(preserveAtttachments.size());
	subpass.pPreserveAttachments =  preserveAtttachments.empty() ? nullptr :preserveAtttachments.data();
}
