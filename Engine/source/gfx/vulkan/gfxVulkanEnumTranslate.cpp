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

#include "platform/platform.h"
#include "gfx/vulkan/gfxVulkanEnumTranslate.h"
 
VkPrimitiveTopology GFXVulkanPrimType[GFXPT_COUNT];
VkBlendFactor GFXVulkanBlend[GFXBlend_COUNT];
VkBlendOp GFXVulkanBlendOp[GFXBlendOp_COUNT];
VkSamplerAddressMode GFXVulkanSamplerState[GFXSAMP_COUNT];
VkFilter GFXVulkanTextureFilter[GFXTextureFilter_COUNT];
VkSamplerMipmapMode GFXVulkanMipmapFilter[GFXTextureFilter_COUNT];
VkSamplerAddressMode GFXVulkanTextureAddress[GFXAddress_COUNT];
VkCompareOp GFXVulkanCmpFunc[GFXCmp_COUNT];
VkStencilOp GFXVulkanStencilOp[GFXStencilOp_COUNT];
VkFormat GFXVulkanTextureInternalFormat[GFXFormat_COUNT];
VkFormat  GFXVulkanTextureFormat[GFXFormat_COUNT];
VkFormat GFXVulkanTextureType[GFXFormat_COUNT];
VkComponentSwizzle GFXVulkanTextureSwizzle[GFXFormat_COUNT];
Map<String, GFXShaderConstDesc> GFXVulkanShaderConstDesc;
//Vulkanenum GFXVulkanBufferType[GFXBufferType_COUNT];
//Vulkanenum GFXVulkanCullMode[GFXCull_COUNT];
//Vulkanenum GFXVulkanFillMode[GFXFill_COUNT];

void GFXVulkanEnumTranslate::init()
{
   // Buffer types
  /* GFXVulkanBufferType[GFXBufferTypeStatic] = Vulkan_STATIC_DRAW;
   GFXVulkanBufferType[GFXBufferTypeDynamic] = Vulkan_DYNAMIC_DRAW;
   GFXVulkanBufferType[GFXBufferTypeVolatile] = Vulkan_STREAM_DRAW;
   GFXVulkanBufferType[GFXBufferTypeImmutable] = Vulkan_STATIC_DRAW;
*/
   // Primitives
   GFXVulkanPrimType[GFXPointList] = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
   GFXVulkanPrimType[GFXLineList] = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
   GFXVulkanPrimType[GFXLineStrip] = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
   GFXVulkanPrimType[GFXTriangleList] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   GFXVulkanPrimType[GFXTriangleStrip] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

   // Blend
   GFXVulkanBlend[GFXBlendZero] = VK_BLEND_FACTOR_ZERO;
   GFXVulkanBlend[GFXBlendOne] = VK_BLEND_FACTOR_ONE;
   GFXVulkanBlend[GFXBlendSrcColor] = VK_BLEND_FACTOR_SRC_COLOR;
   GFXVulkanBlend[GFXBlendInvSrcColor] = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
   GFXVulkanBlend[GFXBlendSrcAlpha] = VK_BLEND_FACTOR_SRC_ALPHA;
   GFXVulkanBlend[GFXBlendInvSrcAlpha] = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   GFXVulkanBlend[GFXBlendDestAlpha] = VK_BLEND_FACTOR_DST_ALPHA;
   GFXVulkanBlend[GFXBlendInvDestAlpha] = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
   GFXVulkanBlend[GFXBlendDestColor] = VK_BLEND_FACTOR_DST_COLOR;
   GFXVulkanBlend[GFXBlendInvDestColor] = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
   GFXVulkanBlend[GFXBlendSrcAlphaSat] = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
   
   // Blend op
   GFXVulkanBlendOp[GFXBlendOpAdd] = VK_BLEND_OP_ADD;
   GFXVulkanBlendOp[GFXBlendOpSubtract] = VK_BLEND_OP_SUBTRACT;
   GFXVulkanBlendOp[GFXBlendOpRevSubtract] = VK_BLEND_OP_REVERSE_SUBTRACT;
   GFXVulkanBlendOp[GFXBlendOpMin] = VK_BLEND_OP_MIN;
   GFXVulkanBlendOp[GFXBlendOpMax] = VK_BLEND_OP_MAX;

   // Sampler
   GFXVulkanSamplerState[GFXSAMPMagFilter] = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   GFXVulkanSamplerState[GFXSAMPMinFilter] = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   GFXVulkanSamplerState[GFXSAMPAddressU] = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   GFXVulkanSamplerState[GFXSAMPAddressV] = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   GFXVulkanSamplerState[GFXSAMPAddressW] = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   GFXVulkanSamplerState[GFXSAMPMipMapLODBias] = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   
   // Comparison
   GFXVulkanCmpFunc[GFXCmpNever] = VK_COMPARE_OP_NEVER;
   GFXVulkanCmpFunc[GFXCmpLess] = VK_COMPARE_OP_LESS;
   GFXVulkanCmpFunc[GFXCmpEqual] = VK_COMPARE_OP_EQUAL;
   GFXVulkanCmpFunc[GFXCmpLessEqual] = VK_COMPARE_OP_LESS_OR_EQUAL;
   GFXVulkanCmpFunc[GFXCmpGreater] = VK_COMPARE_OP_GREATER;
   GFXVulkanCmpFunc[GFXCmpNotEqual] = VK_COMPARE_OP_NOT_EQUAL;
   GFXVulkanCmpFunc[GFXCmpGreaterEqual] = VK_COMPARE_OP_GREATER_OR_EQUAL;
   GFXVulkanCmpFunc[GFXCmpAlways] = VK_COMPARE_OP_ALWAYS;

   GFXVulkanTextureFilter[GFXTextureFilterNone] = VK_FILTER_NEAREST;
   GFXVulkanTextureFilter[GFXTextureFilterPoint] = VK_FILTER_NEAREST;
   GFXVulkanTextureFilter[GFXTextureFilterLinear] = VK_FILTER_LINEAR;

   GFXVulkanTextureFilter[GFXTextureFilterAnisotropic] = VK_FILTER_LINEAR;
   GFXVulkanTextureFilter[GFXTextureFilterPyramidalQuad] = VK_FILTER_LINEAR; 
   GFXVulkanTextureFilter[GFXTextureFilterGaussianQuad] = VK_FILTER_LINEAR;

   GFXVulkanMipmapFilter[GFXTextureFilterNone] = VK_SAMPLER_MIPMAP_MODE_NEAREST;
   GFXVulkanMipmapFilter[GFXTextureFilterPoint] = VK_SAMPLER_MIPMAP_MODE_NEAREST;
   GFXVulkanMipmapFilter[GFXTextureFilterLinear] = VK_SAMPLER_MIPMAP_MODE_LINEAR;

   GFXVulkanTextureAddress[GFXAddressWrap] = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   GFXVulkanTextureAddress[GFXAddressMirror] = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
   GFXVulkanTextureAddress[GFXAddressClamp] = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   GFXVulkanTextureAddress[GFXAddressBorder] = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
   GFXVulkanTextureAddress[GFXAddressMirrorOnce] = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
   
   // Stencil ops
   GFXVulkanStencilOp[GFXStencilOpKeep] = VK_STENCIL_OP_KEEP;
   GFXVulkanStencilOp[GFXStencilOpZero] = VK_STENCIL_OP_ZERO;
   GFXVulkanStencilOp[GFXStencilOpReplace] = VK_STENCIL_OP_REPLACE;
   GFXVulkanStencilOp[GFXStencilOpIncrSat] = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
   GFXVulkanStencilOp[GFXStencilOpDecrSat] = VK_STENCIL_OP_DECREMENT_AND_CLAMP;
   GFXVulkanStencilOp[GFXStencilOpInvert] = VK_STENCIL_OP_INVERT;
   
   GFXVulkanStencilOp[GFXStencilOpIncr] = VK_STENCIL_OP_INCREMENT_AND_WRAP;
   GFXVulkanStencilOp[GFXStencilOpDecr] = VK_STENCIL_OP_DECREMENT_AND_WRAP;
   
   
   // Texture formats
   for(int i = 0; i < GFXFormat_COUNT; ++i)
   {
      GFXVulkanTextureInternalFormat[i] = VK_FORMAT_UNDEFINED;
      GFXVulkanTextureFormat[i] = VK_FORMAT_UNDEFINED;
      GFXVulkanTextureType[i] = VK_FORMAT_UNDEFINED;
      GFXVulkanTextureSwizzle[i] = VK_COMPONENT_SWIZZLE_IDENTITY;
   }

   GFXVulkanTextureInternalFormat[GFXFormatA8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatL8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatR5G5B5A1] = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
   GFXVulkanTextureInternalFormat[GFXFormatR5G5B5X1] = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
   GFXVulkanTextureInternalFormat[GFXFormatL16] = VK_FORMAT_R16_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatD16] = VK_FORMAT_D16_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatR8G8B8] = VK_FORMAT_R8G8B8_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatR8G8B8A8] = VK_FORMAT_R8G8B8A8_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatR8G8B8X8] = VK_FORMAT_R8G8B8A8_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatB8G8R8A8] = VK_FORMAT_R8G8B8A8_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatR10G10B10A2] = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
   GFXVulkanTextureInternalFormat[GFXFormatD32] = VK_FORMAT_D32_SFLOAT;
   GFXVulkanTextureInternalFormat[GFXFormatD24X8] = VK_FORMAT_D24_UNORM_S8_UINT;
   GFXVulkanTextureInternalFormat[GFXFormatD24S8] = VK_FORMAT_D24_UNORM_S8_UINT;
   GFXVulkanTextureInternalFormat[GFXFormatR16G16B16A16] = VK_FORMAT_R16G16B16A16_UNORM;
   GFXVulkanTextureInternalFormat[GFXFormatBC1] = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
   GFXVulkanTextureInternalFormat[GFXFormatBC2] = VK_FORMAT_BC2_UNORM_BLOCK;
   GFXVulkanTextureInternalFormat[GFXFormatBC3] = VK_FORMAT_BC3_UNORM_BLOCK;
   GFXVulkanTextureInternalFormat[GFXFormatBC4] = VK_FORMAT_BC4_UNORM_BLOCK;
   GFXVulkanTextureInternalFormat[GFXFormatBC5] = VK_FORMAT_BC5_UNORM_BLOCK;
   //sRGB
   GFXVulkanTextureInternalFormat[GFXFormatR8G8B8_SRGB] = VK_FORMAT_R8G8B8_SRGB;
   GFXVulkanTextureInternalFormat[GFXFormatR8G8B8A8_SRGB] = VK_FORMAT_R8G8B8A8_SRGB;
   GFXVulkanTextureInternalFormat[GFXFormatBC1_SRGB] = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
   GFXVulkanTextureInternalFormat[GFXFormatBC2_SRGB] = VK_FORMAT_BC2_SRGB_BLOCK;
   GFXVulkanTextureInternalFormat[GFXFormatBC3_SRGB] = VK_FORMAT_BC3_SRGB_BLOCK;
   
   GFXVulkanTextureFormat[GFXFormatA8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureFormat[GFXFormatL8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureFormat[GFXFormatR5G5B5A1] = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
   GFXVulkanTextureFormat[GFXFormatR5G5B5X1] = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
   GFXVulkanTextureFormat[GFXFormatL16] = VK_FORMAT_R16_UNORM;
   GFXVulkanTextureFormat[GFXFormatD16] = VK_FORMAT_D16_UNORM;
   GFXVulkanTextureFormat[GFXFormatR8G8B8] = VK_FORMAT_R8G8B8_UNORM;
   GFXVulkanTextureFormat[GFXFormatR8G8B8A8] = VK_FORMAT_R8G8B8A8_UNORM;
   GFXVulkanTextureFormat[GFXFormatR8G8B8X8] = VK_FORMAT_R8G8B8A8_UNORM;
   GFXVulkanTextureFormat[GFXFormatB8G8R8A8] = VK_FORMAT_B8G8R8A8_UNORM;
   GFXVulkanTextureFormat[GFXFormatR10G10B10A2] = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
   GFXVulkanTextureFormat[GFXFormatD32] = VK_FORMAT_D32_SFLOAT;
   GFXVulkanTextureFormat[GFXFormatD24X8] = VK_FORMAT_D24_UNORM_S8_UINT;
   GFXVulkanTextureFormat[GFXFormatD24S8] = VK_FORMAT_D24_UNORM_S8_UINT;
   GFXVulkanTextureFormat[GFXFormatR16G16B16A16] = VK_FORMAT_R16G16B16A16_UNORM;
   GFXVulkanTextureFormat[GFXFormatBC1] = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
   GFXVulkanTextureFormat[GFXFormatBC2] = VK_FORMAT_BC2_UNORM_BLOCK;
   GFXVulkanTextureFormat[GFXFormatBC3] = VK_FORMAT_BC3_UNORM_BLOCK;
   GFXVulkanTextureFormat[GFXFormatBC4] = VK_FORMAT_BC4_UNORM_BLOCK;
   GFXVulkanTextureFormat[GFXFormatBC5] = VK_FORMAT_BC5_UNORM_BLOCK;
   //sRGB
   GFXVulkanTextureFormat[GFXFormatR8G8B8_SRGB] = VK_FORMAT_R8G8B8_SNORM;
   GFXVulkanTextureFormat[GFXFormatR8G8B8A8_SRGB] = VK_FORMAT_R8G8B8A8_SRGB;
   GFXVulkanTextureFormat[GFXFormatBC1_SRGB] = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
   GFXVulkanTextureFormat[GFXFormatBC2_SRGB] = VK_FORMAT_BC2_SRGB_BLOCK;
   GFXVulkanTextureFormat[GFXFormatBC3_SRGB] = VK_FORMAT_BC3_SRGB_BLOCK;
   
   GFXVulkanTextureType[GFXFormatA8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatL8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatR5G5B5A1] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatR5G5B5X1] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatL16] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatD16] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatR8G8B8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatR8G8B8A8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatR8G8B8X8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatB8G8R8A8] = VK_FORMAT_R8_UNORM;;
   GFXVulkanTextureType[GFXFormatR10G10B10A2] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatD32] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatD24X8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatD24S8] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatR16G16B16A16] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatBC1] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatBC2] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatBC3] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatBC4] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatBC5] = VK_FORMAT_R8_UNORM;
   // sRGB
   GFXVulkanTextureType[GFXFormatR8G8B8_SRGB] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatR8G8B8A8_SRGB] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatBC1_SRGB] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatBC2_SRGB] = VK_FORMAT_R8_UNORM;
   GFXVulkanTextureType[GFXFormatBC3_SRGB] = VK_FORMAT_R8_UNORM;


   //static Vulkanint Swizzle_GFXFormatA8[] = { Vulkan_NONE, Vulkan_NONE, Vulkan_NONE, Vulkan_RED };
   //static Vulkanint Swizzle_GFXFormatL[] = { Vulkan_RED, Vulkan_RED, Vulkan_RED, Vulkan_ALPHA };
   //GFXVulkanTextureSwizzle[GFXFormatA8] = Swizzle_GFXFormatA8; // old Vulkan_ALPHA8   
   //GFXVulkanTextureSwizzle[GFXFormatL8] = Swizzle_GFXFormatL; // old Vulkan_LUMINANCE8
   //GFXVulkanTextureSwizzle[GFXFormatL16] = Swizzle_GFXFormatL; // old Vulkan_LUMINANCE16

   GFXVulkanTextureInternalFormat[GFXFormatR32F] = VK_FORMAT_R32_SFLOAT;
   GFXVulkanTextureFormat[GFXFormatR32F] = VK_FORMAT_R32_SFLOAT;
   GFXVulkanTextureType[GFXFormatR32F] = VK_FORMAT_R32_SFLOAT;

   GFXVulkanTextureInternalFormat[GFXFormatR32G32B32A32F] = VK_FORMAT_R32G32B32A32_SFLOAT;
   GFXVulkanTextureFormat[GFXFormatR32G32B32A32F] = VK_FORMAT_R32G32B32A32_SFLOAT;
   GFXVulkanTextureType[GFXFormatR32G32B32A32F] = VK_FORMAT_R32G32B32A32_SFLOAT;

   GFXVulkanTextureInternalFormat[GFXFormatR16F] = VK_FORMAT_R16_SFLOAT;
   GFXVulkanTextureFormat[GFXFormatR16F] = VK_FORMAT_R16_SFLOAT;
   GFXVulkanTextureType[GFXFormatR16F] = VK_FORMAT_R16_SFLOAT;

   GFXVulkanTextureInternalFormat[GFXFormatR16G16F] = VK_FORMAT_R16G16_SFLOAT;
   GFXVulkanTextureFormat[GFXFormatR16G16F] = VK_FORMAT_R16G16_SFLOAT;
   GFXVulkanTextureType[GFXFormatR16G16F] = VK_FORMAT_R16G16_SFLOAT;

   GFXVulkanTextureInternalFormat[GFXFormatR16G16B16A16F] = VK_FORMAT_R16G16B16A16_SFLOAT;
   GFXVulkanTextureFormat[GFXFormatR16G16B16A16F] = VK_FORMAT_R16G16B16A16_SFLOAT;
   GFXVulkanTextureType[GFXFormatR16G16B16A16F] = VK_FORMAT_R16G16B16A16_SFLOAT;

   GFXVulkanTextureInternalFormat[GFXFormatR5G6B5] = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
   GFXVulkanTextureFormat[GFXFormatR5G6B5] = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
   GFXVulkanTextureType[GFXFormatR5G6B5] = VK_FORMAT_R5G5B5A1_UNORM_PACK16;

   GFXVulkanTextureInternalFormat[GFXFormatR16G16] = VK_FORMAT_R16G16_UINT;
   GFXVulkanTextureFormat[GFXFormatR16G16] = VK_FORMAT_R16G16_UINT;
   GFXVulkanTextureType[GFXFormatR16G16] = VK_FORMAT_R16G16_UINT;

   //// Cull - Openvulkan render upside down need to invert cull
   //GFXVulkanCullMode[GFXCullNone] = VK_CULL_MODE_FRONT_BIT;
   //GFXVulkanCullMode[GFXCullCW] = VK_CULL_MODE_FRONT_BIT;
   //GFXVulkanCullMode[GFXCullCCW] = VK_CULL_MODE_BACK_BIT;

   //// Fill
   //GFXVulkanFillMode[GFXFillPoint] = Vulkan_POINT;
   //GFXVulkanFillMode[GFXFillWireframe] = Vulkan_LINE;
   //GFXVulkanFillMode[GFXFillSolid] = Vulkan_FILL;

	GFXVulkanShaderConstDesc["$modelViewProj"] = GFXShaderConstDesc("$modelViewProj", GFXSCT_Float4x4, 1);
	GFXVulkanShaderConstDesc["$fsModelViewProj"] = GFXShaderConstDesc("$fsModelViewProj", GFXSCT_Float4x4, 1);
	GFXVulkanShaderConstDesc["$modelview"] = GFXShaderConstDesc("$modelview", GFXSCT_Float4x4, 1);
	GFXVulkanShaderConstDesc["$oneOverFar"] = GFXShaderConstDesc("$oneOverFar", GFXSCT_Float, 1);
	GFXVulkanShaderConstDesc["$oneOverSoftness"] = GFXShaderConstDesc("$oneOverSoftness", GFXSCT_Float, 1);
	GFXVulkanShaderConstDesc["$deferredTargetParams"] = GFXShaderConstDesc("$deferredTargetParams", GFXSCT_Float4, 1);
	GFXVulkanShaderConstDesc["$alphaFactor"] = GFXShaderConstDesc("$alphaFactor", GFXSCT_Float, 1);
	GFXVulkanShaderConstDesc["$alphaScale"] = GFXShaderConstDesc("$alphaScale", GFXSCT_Float, 1);

	GFXVulkanShaderConstDesc["$screenRect"] = GFXShaderConstDesc("$screenRect", GFXSCT_Float4, 1);
	GFXVulkanShaderConstDesc["$edgeTargetParams"] = GFXShaderConstDesc("$edgeTargetParams", GFXSCT_Float4, 1);
	GFXVulkanShaderConstDesc["$edgeSource"] = GFXShaderConstDesc("$edgeSource", GFXSCT_Sampler, 1);
	GFXVulkanShaderConstDesc["$colorSource"] = GFXShaderConstDesc("$colorSource", GFXSCT_Sampler, 1);
	GFXVulkanShaderConstDesc["$offscreenTargetParams"] = GFXShaderConstDesc("$offscreenTargetParams", GFXSCT_Float4, 1);
	
	GFXVulkanShaderConstDesc["$diffuseMap"] = GFXShaderConstDesc("$diffuseMap", GFXSCT_Sampler, 1);
	GFXVulkanShaderConstDesc["$deferredTex"] = GFXShaderConstDesc("$deferredTex", GFXSCT_Sampler, 1);
	GFXVulkanShaderConstDesc["$paraboloidLightMap"] = GFXShaderConstDesc("$paraboloidLightMap", GFXSCT_Sampler, 1);
	
}
