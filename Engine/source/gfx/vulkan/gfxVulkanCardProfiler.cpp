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
	VkPhysicalDevice* physical_device;
	U32 count;
	auto res = vkEnumeratePhysicalDevices(dynamic_cast<GFXVulkanDevice*>(GFX)->getInstance(), &count, NULL);
	assert(res == VK_SUCCESS);
	assert(count >= 1);
	std::vector<VkPhysicalDevice> physicalDevices(count);
	res = vkEnumeratePhysicalDevices(dynamic_cast<GFXVulkanDevice*>(GFX)->getInstance(), &count, physicalDevices.data());
	assert(res == VK_SUCCESS);

	physicalProperties = {};

	vkGetPhysicalDeviceProperties(physicalDevices[0], &physicalProperties);
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(
	    physicalDevices[0],
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
//   Vulkanint maxTexSize;
//   VulkanGetIntegerv(Vulkan_MAX_TEXTURE_SIZE, &maxTexSize);
//   
   // OpenVulkan doesn't have separate maximum width/height.
   setCapability("maxTextureWidth", 1);
   setCapability("maxTextureHeight", 1);
   setCapability("maxTextureSize", 1);
//
//   // Check for anisotropic filtering support.
//   setCapability("Vulkan_EXT_texture_filter_anisotropic", gVulkanHasExtension(EXT_texture_filter_anisotropic));
//
//   // Check for buffer storage
//#ifdef TORQUE_NSIGHT_WORKAROUND
//   setCapability("Vulkan_ARB_buffer_storage", false);
//#else
//   setCapability("Vulkan_ARB_buffer_storage", gVulkanHasExtension(ARB_buffer_storage));
//#endif
//
//   // Check for shader model 5.0
//   setCapability("Vulkan_ARB_gpu_shader5", gVulkanHasExtension(ARB_gpu_shader5));
//
//   // Check for texture storage
//   setCapability("Vulkan_ARB_texture_storage", gVulkanHasExtension(ARB_texture_storage));
//
//   // Check for sampler objects
//   setCapability("Vulkan_ARB_sampler_objects", gVulkanHasExtension(ARB_sampler_objects));
//
//   // Check for copy image support
//   setCapability("Vulkan_ARB_copy_image", gVulkanHasExtension(ARB_copy_image));
//
//   // Check for vertex attrib binding
//   setCapability("Vulkan_ARB_vertex_attrib_binding", gVulkanHasExtension(ARB_vertex_attrib_binding));    

}

bool GFXVulkanCardProfiler::_queryCardCap(const String& query, U32& foundResult)
{
   // Just doing what the D3D11 layer does
   return 0;
}

bool GFXVulkanCardProfiler::_queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips)
{
	// We assume if the format is valid that we can use it for any purpose.
   // This may not be the case, but we have no way to check short of in depth 
   // testing of every format for every purpose.  And by testing, I mean sitting
   // down and doing it by hand, because there is no OpenVulkan API to check these
   // things.
   //return GFXVulkanTextureInternalFormat[fmt] != Vulkan_ZERO;
	return true;
}
