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

#ifndef _GFXVulkanENUMTRANSLATE_H_
#define _GFXVulkanENUMTRANSLATE_H_

#include "gfx/gfxEnums.h"
#include "gfx/vulkan/gfxVulkanDevice.h"

namespace GFXVulkanEnumTranslate
{
   void init();
};

extern VkPrimitiveTopology GFXVulkanPrimType[GFXPT_COUNT];
extern VkBlendFactor GFXVulkanBlend[GFXBlend_COUNT];
extern VkBlendOp GFXVulkanBlendOp[GFXBlendOp_COUNT];
extern VkSamplerAddressMode GFXVulkanSamplerState[GFXSAMP_COUNT];
extern VkFilter GFXVulkanTextureFilter[GFXTextureFilter_COUNT];
extern VkSamplerMipmapMode GFXVulkanMipmapFilter[GFXTextureFilter_COUNT];
extern VkSamplerAddressMode GFXVulkanTextureAddress[GFXAddress_COUNT];
extern VkCompareOp GFXVulkanCmpFunc[GFXCmp_COUNT];
extern VkStencilOp GFXVulkanStencilOp[GFXStencilOp_COUNT];

extern VkFormat GFXVulkanTextureInternalFormat[GFXFormat_COUNT];
extern VkFormat  GFXVulkanTextureFormat[GFXFormat_COUNT];
extern VkFormat GFXVulkanTextureType[GFXFormat_COUNT];
extern VkComponentSwizzle GFXVulkanTextureSwizzle[GFXFormat_COUNT];
extern Map<String, GFXShaderConstDesc> GFXVulkanShaderConstDesc;

//extern Vulkanenum GFXVulkanBufferType[GFXBufferType_COUNT];
//extern Vulkanenum GFXVulkanCullMode[GFXCull_COUNT];
//
//extern Vulkanenum GFXVulkanFillMode[GFXFill_COUNT];

#endif
