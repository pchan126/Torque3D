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

#include "windowManager/platformWindow.h"
#include <gfx/vulkan/GFXVulkanDevice.h>
#include <gfx/vulkan/GFXVulkanWindowTarget.h>
#include "gfx/vulkan/GFXVulkanTextureObject.h"
#include "gfx/vulkan/GFXVulkanUtils.h"
#include "postFx/postEffect.h"

GFXVulkanWindowTarget::GFXVulkanWindowTarget(PlatformWindow *win, GFXDevice *d)
      : GFXWindowTarget(win), mDevice(d), mContext(NULL), mFullscreenContext(NULL)
      //, mCopyFBO(0), mBackBufferFBO(0)
{      
	surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	image_usage = 0;
   win->appEvent.notify(this, &GFXVulkanWindowTarget::_onAppSignal);
}

GFXVulkanWindowTarget::~GFXVulkanWindowTarget()
{
    vkDestroyPipeline(GFXVulkan->getLogicalDevice(), graphicsPipeline, nullptr);
//    vkDestroyPipelineLayout(GFXVulkan->getLogicalDevice(), pipelineLayout, nullptr);
	//if(glIsFramebuffer(mCopyFBO))
   //{
   //   glDeleteFramebuffers(1, &mCopyFBO);
   //}
}

void GFXVulkanWindowTarget::resetMode()
{
   //if(mWindow->getVideoMode().fullScreen != mWindow->isFullscreen())
   //{
   //   _teardownCurrentMode();
   //   _setupNewMode();
   //}
   GFX->beginReset();
}

void GFXVulkanWindowTarget::_onAppSignal(WindowId wnd, S32 event)
{
   if(event != WindowHidden)
      return;
      
   // TODO: Investigate this further.
   // Opening and then closing the console results in framerate dropping at an alarming rate down to 3-4 FPS and then
   // rebounding to it's usual level.  Clearing all the volatile VBs prevents this behavior, but I can't explain why.
   // My fear is there is something fundamentally wrong with how we share objects between contexts and this is simply 
   // masking the issue for the most common case.
   //static_cast<GFXVulkanDevice*>(mDevice)->mVolatileVBs.clear();
}

void GFXVulkanWindowTarget::init(VkSurfaceKHR &in_surface)
{
	mSurface = in_surface;
	getSurfaceCapabilities();
	chooseMinSwapchainImages();
	selectSwapchainImagesFormat({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
	selectSwapchainImagesTransformation(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
	chooseSwapPresentMode();
	chooseSwapchainSize();
	createSwapchain();
	initSwapchainImages();
	createRenderPass();
}


void GFXVulkanWindowTarget::resolveTo(GFXTextureObject* obj)
{
   AssertFatal(dynamic_cast<GFXVulkanTextureObject*>(obj), "GFXVulkanTextureTarget::resolveTo - Incorrect type of texture, expected a GFXVulkanTextureObject");
   GFXVulkanTextureObject* glTexture = static_cast<GFXVulkanTextureObject*>(obj);

   //if( GFXVulkan->mCapabilities.copyImage )
   //{
   //   if(mBackBufferColorTex.getWidth() == glTexture->getWidth()
   //      && mBackBufferColorTex.getHeight() == glTexture->getHeight()
   //      && mBackBufferColorTex.getFormat() == glTexture->getFormat())
   //   {
   //      glCopyImageSubData(
   //        static_cast<GFXVulkanTextureObject*>(mBackBufferColorTex.getPointer())->getHandle(), GL_TEXTURE_2D, 0, 0, 0, 0,
   //        glTexture->getHandle(), GL_TEXTURE_2D, 0, 0, 0, 0,
   //        getSize().x, getSize().y, 1);
   //      return;
   //   }
   //}

   //PRESERVE_FRAMEBUFFER();

   if(!mCopyFBO)
   {
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	    framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = swapchain_images.size();
	    framebufferInfo.pAttachments = swapchain_images.data();
		framebufferInfo.width = getSize().x;
		framebufferInfo.height = getSize().y;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(GFXVulkan->getLogicalDevice(), &framebufferInfo, nullptr, &mCopyFBO) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
   }
   //
   //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mCopyFBO);
   //glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTexture->getHandle(), 0);
   //
   //glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackBufferFBO);
   //
   //glBlitFramebuffer(0, 0, getSize().x, getSize().y,
   //   0, 0, glTexture->getWidth(), glTexture->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

inline void GFXVulkanWindowTarget::_setupAttachments()
{
   //glBindFramebuffer( GL_FRAMEBUFFER, mBackBufferFBO);
   //glEnable(GL_FRAMEBUFFER_SRGB);
   //GFXVulkan->getOpenglCache()->setCacheBinded(GL_FRAMEBUFFER, mBackBufferFBO);
   //const Point2I dstSize = getSize();
   //mBackBufferColorTex.set(dstSize.x, dstSize.y, getFormat(), &GFXRenderTargetSRGBProfile, "backBuffer");
   //GFXVulkanTextureObject *color = static_cast<GFXVulkanTextureObject*>(mBackBufferColorTex.getPointer());
   //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->getHandle(), 0);
   //mBackBufferDepthTex.set(dstSize.x, dstSize.y, GFXFormatD24S8, &BackBufferDepthProfile, "backBuffer");
   //GFXVulkanTextureObject *depth = static_cast<GFXVulkanTextureObject*>(mBackBufferDepthTex.getPointer());
   //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth->getHandle(), 0);
}

bool GFXVulkanWindowTarget::getSurfaceCapabilities()
{
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GFXVulkan->physicalDevice, mSurface, &surface_capabilities);
	if ( VK_SUCCESS != result )
	{
		Con::errorf("Could not get the capabilities of a presentation surface");
		return false;
	}
	return true;
}

void GFXVulkanWindowTarget::chooseSwapchainSize()
{
	if (0xFFFFFFFF == surface_capabilities.currentExtent.width)
	{
		size_of_images = { 640, 480 };

		if (size_of_images.width < surface_capabilities.minImageExtent.width)
		{
			size_of_images.width = surface_capabilities.minImageExtent.width;
		}
		else if ( size_of_images.width > surface_capabilities.maxImageExtent.width)
		{
			size_of_images.width = surface_capabilities.maxImageExtent.width;
		}

		if (size_of_images.height < surface_capabilities.minImageExtent.height)
		{
			size_of_images.height = surface_capabilities.minImageExtent.height;
		}
		else if ( size_of_images.height > surface_capabilities.maxImageExtent.height)
		{
			size_of_images.height = surface_capabilities.maxImageExtent.height;
		}
	}
	else
	{
		size_of_images = surface_capabilities.currentExtent;
	}
}

void GFXVulkanWindowTarget::chooseMinSwapchainImages()
{
	number_of_images = surface_capabilities.minImageCount + 1;
	if ((surface_capabilities.maxImageCount > 0 ) && 
		(number_of_images > surface_capabilities.maxImageCount ))
	{
		number_of_images = surface_capabilities.maxImageCount;
	}
}

bool GFXVulkanWindowTarget::chooseUsages(VkImageUsageFlags desired_usages)
{
	image_usage = desired_usages & surface_capabilities.supportedUsageFlags;

	return desired_usages == image_usage;
}

bool GFXVulkanWindowTarget::selectSwapchainImagesTransformation(VkSurfaceTransformFlagBitsKHR desired_transform)
{
	if (surface_capabilities.supportedTransforms & desired_transform)
	{
		surface_transform = desired_transform;
	}
	else
	{
		surface_transform = surface_capabilities.currentTransform;
	}
	return false;
}

bool GFXVulkanWindowTarget::selectSwapchainImagesFormat(VkSurfaceFormatKHR desired_surface_format)
{
	U32 formats_count = 0;
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(GFXVulkan->physicalDevice, mSurface, &formats_count, nullptr);
	if ((VK_SUCCESS != result) ||
		(0 == formats_count )) 
	{
		Con::errorf("Could not get the number of supported surface formats.");
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surface_formats(formats_count);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(GFXVulkan->physicalDevice, mSurface, &formats_count, surface_formats.data());
	if ((VK_SUCCESS != result) ||
		(0 == formats_count )) 
	{
		Con::errorf("Could not enumerate supported surface formats.");
		return false;
	}
	if ((1 == surface_formats.size() ) && 
		(VK_FORMAT_UNDEFINED == surface_formats[0].format) ) 
	{
		image_format = desired_surface_format.format;
		image_color_space = desired_surface_format.colorSpace;
		surface_format = desired_surface_format;
		return true;
	}
	for (auto & surface_format: surface_formats )
	{
		if ((desired_surface_format.format == surface_format.format) &&
			(desired_surface_format.colorSpace == surface_format.colorSpace)) 
		{
			image_format = desired_surface_format.format;
			image_color_space = desired_surface_format.colorSpace;
			surface_format = {image_format, image_color_space};
			return true;
		}
	}

	for (auto & surface_format: surface_formats )
	{
		if ((desired_surface_format.format == surface_format.format))
		{
			image_format = desired_surface_format.format;
			image_color_space = surface_format.colorSpace;
			Con::warnf("Desired combination of format and colorspace is not supported. Selected other colorspace");
			surface_format = {image_format, image_color_space};
			return true;
		}
	}

	image_format = surface_formats[0].format;
	image_color_space = surface_formats[0].colorSpace;
	Con::warnf("Desired format is not supported Selecting available format - colorspace combination");
	surface_format = {image_format, image_color_space};
	return true;
}

bool GFXVulkanWindowTarget::chooseSwapPresentMode(VkPresentModeKHR _present_mode)
{
	U32 count;
	auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(GFXVulkan->physicalDevice, mSurface, &count, nullptr);
	if ((VK_SUCCESS != result) ||
		(0 == count )) 
	{
		Con::errorf("Could not get the number of supported present modes.");
		return false;
	}
	std::vector<VkPresentModeKHR> availablePresentModes(count);
	result  = vkGetPhysicalDeviceSurfacePresentModesKHR(GFXVulkan->physicalDevice, mSurface, &count, availablePresentModes.data());
	if ((VK_SUCCESS != result) ||
		(0 == count )) 
	{
		Con::errorf("Could not enumerate supported present modes.");
		return false;
	}

	for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == _present_mode) {
            present_mode = _present_mode;
			return true;
        }
    }
	present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	Con::warnf("Could support desired present mode.");
	return true;
}


bool GFXVulkanWindowTarget::createSwapchain()
{
	VkSwapchainCreateInfoKHR swapchain_create_info = {
	VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		mSurface,
		number_of_images,
		image_format,
		image_color_space,
		size_of_images,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		surface_transform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		present_mode,
		VK_TRUE,
		mSwapChain
		};

	VkSwapchainKHR newSwapChain;

	VkResult result = vkCreateSwapchainKHR( GFXVulkan->getLogicalDevice(), &swapchain_create_info, nullptr, &newSwapChain);
	if (result == VK_SUCCESS)
	{
		vkDestroySwapchainKHR(GFXVulkan->getLogicalDevice(), mSwapChain, nullptr);
	}
	else
	{
		Con::errorf("Error creating swapchain");
		return false;
	}
	mSwapChain = newSwapChain;
	return true;
}

bool GFXVulkanWindowTarget::initSwapchainImages()
{
	U32 images_count = 0;
	VkResult result = VK_SUCCESS;
	result = vkGetSwapchainImagesKHR(GFXVulkan->getLogicalDevice(), mSwapChain, &images_count, nullptr);
	if ( (VK_SUCCESS != result) || ( 0 == images_count))
	{
		Con::errorf("Could not get the number of swapchain images.");
		return false;
	}
	swapchain_images.resize( images_count );
	result = vkGetSwapchainImagesKHR(GFXVulkan->getLogicalDevice(), mSwapChain, &images_count, swapchain_images.data());
	if ( (VK_SUCCESS != result) || ( 0 == images_count))
	{
		Con::errorf("Could not enumerate swapchain images.");
		return false;
	}

	return true;
}


bool GFXVulkanWindowTarget::createRenderPass()
{
	std::vector<VkAttachmentDescription> colorAttachments;
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = getSwapChainImageFormat().format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	colorAttachments.push_back(colorAttachment);

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription>subpasses;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	subpasses.push_back(subpass);

	//VkSubpassDependency subpass_dependency = {};
 //   subpass_dependency.srcSubpass;
 //   subpass_dependency.dstSubpass;
 //   subpass_dependency.srcStageMask;
 //   subpass_dependency.dstStageMask;
 //   subpass_dependency.srcAccessMask;
 //   subpass_dependency.dstAccessMask;
 //   subpass_dependency.dependencyFlags;

	VkPipelineLayout pipelineLayout;
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<U32>(colorAttachments.size());
	renderPassInfo.pAttachments = colorAttachments.data();
	renderPassInfo.subpassCount = static_cast<U32>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = static_cast<U32>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(GFXVulkan->getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		Con::errorf("Failed To create render pass");
		return false;
	}
	return true;
}

bool GFXVulkanWindowTarget::createPipeline()
{
	//VkGraphicsPipelineCreateInfo pipelineInfo = {};
	//pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	//pipelineInfo.stageCount = 2;
	//pipelineInfo.pStages = shaderStages;
	//pipelineInfo.pVertexInputState = &vertexInputInfo;
	//pipelineInfo.pInputAssemblyState = &inputAssembly;
	//pipelineInfo.pViewportState = &viewportState;
	//pipelineInfo.pRasterizationState = &rasterizer;
	//pipelineInfo.pMultisampleState = &multisampling;
	//pipelineInfo.pDepthStencilState = nullptr; // Optional
	//pipelineInfo.pColorBlendState = &colorBlending;
	//pipelineInfo.pDynamicState = nullptr; // Optional
	//pipelineInfo.layout = pipelineLayout;
	//pipelineInfo.renderPass = renderPass;
	//pipelineInfo.subpass = 0;
	//pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	//pipelineInfo.basePipelineIndex = -1; // Optional

	//if (vkCreateGraphicsPipelines(GFXVulkan->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) 
	//{
	//	Con::errorf("failed to create graphics pipeline!");
	//}

	return false;
}


void GFXVulkanWindowTarget::makeActive()
{
   ////make the rendering context active on this window
   //_makeContextCurrent();

   //if(mBackBufferFBO)
   //{
   //   glBindFramebuffer( GL_FRAMEBUFFER, mBackBufferFBO);
   //   GFXVulkan->getOpenglCache()->setCacheBinded(GL_FRAMEBUFFER, mBackBufferFBO);
   //}
   //else
   //{
   //   glGenFramebuffers(1, &mBackBufferFBO);
   //   _setupAttachments();
   //   CHECK_FRAMEBUFFER_STATUS();
   //}
}

bool GFXVulkanWindowTarget::present()
{
   // PRESERVE_FRAMEBUFFER();

   //const Point2I srcSize = mBackBufferColorTex.getWidthHeight();
   //const Point2I dstSize = getSize();

   //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
   //glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackBufferFBO);

   //// OpenGL render upside down for make render more similar to DX.
   //// Final screen are corrected here
   //glBlitFramebuffer(
   //   0, 0, srcSize.x, srcSize.y,
   //   0, dstSize.y, dstSize.x, 0, // Y inverted
   //   GL_COLOR_BUFFER_BIT, GL_NEAREST);

   //_WindowPresent();

   //if(srcSize != dstSize || mBackBufferDepthTex.getWidthHeight() != dstSize)
   //   _setupAttachments();

   return true;
}
