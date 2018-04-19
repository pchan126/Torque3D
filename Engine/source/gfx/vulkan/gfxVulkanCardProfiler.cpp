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

#include "gfx/Vulkan/gfxVulkanCardProfiler.h"
#include "gfx/Vulkan/gfxVulkanDevice.h"
#include "gfx/Vulkan/gfxVulkanEnumTranslate.h"

void GFXVulkanCardProfiler::init()
{
	U32 count;
	auto res = vkEnumeratePhysicalDevices(dynamic_cast<GFXVulkanDevice*>(GFX)->getInstance(), &count, NULL);
	assert(res == VK_SUCCESS);
	assert(count >= 1);
	std::vector<VkPhysicalDevice> physicalDevices(count);
	res = vkEnumeratePhysicalDevices(dynamic_cast<GFXVulkanDevice*>(GFX)->getInstance(), &count, physicalDevices.data());
	assert(res == VK_SUCCESS);

	physicalProperties = {};

	physicalDevice = physicalDevices[0];
	checkSwapChainSupport();

	vkGetPhysicalDeviceProperties(physicalDevice, &physicalProperties);
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(
	    physicalDevice,
		&memoryProperties);
	
	StringBuilder in;
      in.format("API Version:    %d.%d.%d\n",
		VK_VERSION_MAJOR(physicalProperties.apiVersion),
        VK_VERSION_MINOR(physicalProperties.apiVersion),
        VK_VERSION_PATCH(physicalProperties.apiVersion));

	mChipSet = reinterpret_cast<const char*>(physicalProperties.deviceName);
    mRendererString = "Vulkan";
    mCardDescription = reinterpret_cast<const char*>(physicalProperties.deviceName);
    mVersionString = reinterpret_cast<const char*>(in.data());   
    mVideoMemory = static_cast<GFXVulkanDevice*>(GFX)->getTotalVideoMemory();

	Parent::init();
}

void GFXVulkanCardProfiler::setupCardCapabilities()
{
   setCapability("maxTextureWidth", physicalProperties.limits.maxImageDimension2D);
   setCapability("maxTextureHeight", physicalProperties.limits.maxImageDimension2D);
   setCapability("maxTextureSize", physicalProperties.limits.maxImageDimension2D);
}

bool GFXVulkanCardProfiler::_queryCardCap(const String& query, U32& foundResult)
{
   // Just doing what the D3D11 layer does
   return false;
}

bool GFXVulkanCardProfiler::_queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips)
{
   return GFXVulkanTextureInternalFormat[fmt] != VK_FORMAT_UNDEFINED;
}


bool GFXVulkanCardProfiler::checkSwapChainSupport()
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	if (extensionCount == 0)
	{
		Con::errorf("physical device doesn't support any extensions");
		//exit(1);
		return false;
	}

	std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, deviceExtensions.data());

	for (const auto& extension : deviceExtensions)
	{
		if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			Con::printf("physical device supports swap chains");
			//std::cout << "physical device supports swap chains" << std::endl;
			return true;
		}
	}

	Con::errorf("physical device doesn't supports swap chains");
	return false;
}

 

