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

#include "gfx/vulkan/gfxVulkanFence.h"

GFXVulkanFence::GFXVulkanFence(GFXDevice* device) : GFXFence(device), mIssued(false)
{
	VkFenceCreateInfo fence_create_info {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.pNext = nullptr;
	fence_create_info.flags = 0;

	const VkResult result = vkCreateFence( dynamic_cast<GFXVulkanDevice*>(device)->getDevice(), &fence_create_info, nullptr, &fence );
	if (result != VK_SUCCESS)
	{
		Con::errorf("err");
	}
}

GFXVulkanFence::~GFXVulkanFence()
{
   //glDeleteFencesAPPLE(1, &mHandle);
}

void GFXVulkanFence::issue()
{
   //glSetFenceAPPLE(mHandle);
   mIssued = true;
}

GFXFence::FenceStatus GFXVulkanFence::getStatus() const
{
   if(!mIssued)
      return GFXFence::Unset;
      
   //GLboolean res = glTestFenceAPPLE(mHandle);
   return true ? GFXFence::Processed : GFXFence::Pending;
}

void GFXVulkanFence::block()
{
   if(!mIssued)
      return;
      
   //glFinishFenceAPPLE(mHandle);
}

void GFXVulkanFence::zombify()
{
	auto logical_device = GFXVulkan->getDevice();
	vkDestroyFence(logical_device, fence, nullptr);
}

void GFXVulkanFence::resurrect()
{
   //glGenFencesAPPLE(1, &mHandle);
}

const String GFXVulkanFence::describeSelf() const
{
   return String::ToString("   Handle");
}
