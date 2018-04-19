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

#ifndef _GFXVulkanWINDOWTARGET_H_
#define _GFXVulkanWINDOWTARGET_H_

#include "gfx/gfxTarget.h"
#include "windowManager/platformWindow.h"

class GFXVulkanWindowTarget : public GFXWindowTarget
{
public:

   GFXVulkanWindowTarget(PlatformWindow *win, GFXDevice *d);
   ~GFXVulkanWindowTarget();

   const Point2I getSize() 
   { 
      return mWindow->getClientExtent();
   }
   virtual GFXFormat getFormat()
   {
      // TODO: Fix me!
      return GFXFormatR8G8B8A8_SRGB;
   }
   void makeActive();
   virtual bool present();
   virtual void resetMode();
   virtual void zombify() { }
   virtual void resurrect() { }

	void resolveTo(GFXTextureObject* obj) override;
   
   void _onAppSignal(WindowId wnd, S32 event);

	void init(VkSurfaceKHR &in_surface);	

	VkSurfaceFormatKHR getSwapChainImageFormat() { return surface_format; };

	VkRenderPass getRenderPass() { return renderPass; };
	U32 getSwapchainImageCount() const { return swapchain_images.size(); };
private:
   friend class GFXVulkanDevice;

   VkFramebuffer mCopyFBO, mBackBufferFBO;
   //GFXTexHandle mBackBufferColorTex, mBackBufferDepthTex;
   Point2I size;   
   GFXDevice* mDevice;

   VkSurfaceKHR mSurface;
   VkSurfaceFormatKHR surface_format;
   std::vector<VkImageView> swapchain_images;

	VkSwapchainKHR mSwapChain;

	VkQueue queue;
	VkExtent2D size_of_images;
	U32 number_of_images;
	VkImageUsageFlags image_usage;
	VkSurfaceTransformFlagBitsKHR surface_transform;
	VkFormat image_format;
	VkColorSpaceKHR image_color_space;
	VkPresentModeKHR present_mode;

   void* mContext;
   void* mFullscreenContext;
   void _teardownCurrentMode();
//   void _setupNewMode();
   void _setupAttachments();
   //void _WindowPresent();
   ////set this windows context to be current
   //void _makeContextCurrent();

	VkSurfaceCapabilitiesKHR surface_capabilities;
	bool getSurfaceCapabilities();

	void chooseSwapchainSize();
	void chooseMinSwapchainImages();
	bool chooseUsages(VkImageUsageFlags desired_usages);
	bool selectSwapchainImagesTransformation(VkSurfaceTransformFlagBitsKHR desired_transform);
	bool selectSwapchainImagesFormat(VkSurfaceFormatKHR desired_surface_format);
	bool chooseSwapPresentMode(VkPresentModeKHR _present_mode = VK_PRESENT_MODE_MAILBOX_KHR);

	std::vector<VkSubpassDependency> dependencies;

	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;

	bool createSwapchain();
	bool initSwapchainImages();
	bool createRenderPass();
	bool createPipeline();
};

#endif
