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

#include "gfx/vulkan/gfxVulkanStateBlock.h"
#include "gfx/vulkan/gfxVulkanDevice.h"
#include "gfx/vulkan/gfxVulkanEnumTranslate.h"
#include "gfx/vulkan/gfxVulkanUtils.h"
#include "gfx/vulkan/gfxVulkanTextureObject.h"
#include "core/crc.h"

namespace DictHash
{
   inline U32 hash(const GFXSamplerStateDesc &data)
   {
      return CRC::calculateCRC(&data, sizeof(GFXSamplerStateDesc));;
   }
}

GFXVulkanStateBlock::GFXVulkanStateBlock(const GFXStateBlockDesc& desc) :
   mDesc(desc),
   mCachedHashValue(desc.getHashValue())
{
   // if( !GFXVulkan->mCapabilities.samplerObjects )
   //   return;

   static Map<GFXSamplerStateDesc, U32> mSamplersMap;

   for(int i = 0; i < TEXTURE_STAGE_COUNT; ++i)
   {
      U32 &id = mSamplerObjects[i];
      GFXSamplerStateDesc &ssd = mDesc.samplers[i];
      Map<GFXSamplerStateDesc, U32>::Iterator itr =  mSamplersMap.find(ssd);
      if(itr == mSamplersMap.end())
      {
   //      VulkanGenSamplers(1, &id);

   //      VulkanSamplerParameteri(id, Vulkan_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, 1) );
   //      VulkanSamplerParameteri(id, Vulkan_TEXTURE_MAG_FILTER, GFXVulkanTextureFilter[ssd.magFilter]);
   //      VulkanSamplerParameteri(id, Vulkan_TEXTURE_WRAP_S, GFXVulkanTextureAddress[ssd.addressModeU]);
   //      VulkanSamplerParameteri(id, Vulkan_TEXTURE_WRAP_T, GFXVulkanTextureAddress[ssd.addressModeV]);
   //      VulkanSamplerParameteri(id, Vulkan_TEXTURE_WRAP_R, GFXVulkanTextureAddress[ssd.addressModeW]);
   //      
   //      //compare modes
   //      const bool comparison = ssd.samplerFunc != GFXCmpNever;
   //      VulkanSamplerParameteri(id, Vulkan_TEXTURE_COMPARE_MODE, comparison ? Vulkan_COMPARE_R_TO_TEXTURE_ARB : Vulkan_NONE );
   //      VulkanSamplerParameteri(id, Vulkan_TEXTURE_COMPARE_FUNC, GFXVulkanCmpFunc[ssd.samplerFunc]);

   //      if (static_cast< GFXVulkanDevice* >(GFX)->supportsAnisotropic())
   //         VulkanSamplerParameterf(id, Vulkan_TEXTURE_MAX_ANISOTROPY_EXT, ssd.maxAnisotropy);

         mSamplersMap[ssd] = id;
      }
      else
         id = itr->value;
   }
}

GFXVulkanStateBlock::~GFXVulkanStateBlock()
{
}

/// Returns the hash value of the desc that created this block
U32 GFXVulkanStateBlock::getHashValue() const
{
   return mCachedHashValue;
}

/// Returns a GFXStateBlockDesc that this block represents
const GFXStateBlockDesc& GFXVulkanStateBlock::getDesc() const
{
   return mDesc;   
}

/// Called by OpenVulkan device to active this state block.
/// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
void GFXVulkanStateBlock::activate(const GFXVulkanStateBlock* oldState)
{
   PROFILE_SCOPE(GFXVulkanStateBlock_Activate);
   // Big scary warning copied from Apple docs 
   // http://developer.apple.com/documentation/GraphicsImaging/Conceptual/OpenVulkan-MacProgGuide/openVulkan_performance/chapter_13_section_2.html#//apple_ref/doc/uid/TP40001987-CH213-SW12
   // Don't set a state that's already set. Once a feature is enabled, it does not need to be enabled again.
   // Calling an enable function more than once does nothing except waste time because OpenVulkan does not check 
   // the state of a feature when you call VulkanEnable or VulkanDisable. For instance, if you call VulkanEnable(Vulkan_LIGHTING) 
   // more than once, OpenVulkan does not check to see if the lighting state is already enabled. It simply updates 
   // the state value even if that value is identical to the current value.

#define STATE_CHANGE(state) (!oldState || oldState->mDesc.state != mDesc.state)
#define TOGVulkanE_STATE(state, enum) if(mDesc.state) VulkanEnable(enum); else VulkanDisable(enum)
#define CHECK_TOGVulkanE_STATE(state, enum) if(!oldState || oldState->mDesc.state != mDesc.state) if(mDesc.state) VulkanEnable(enum); else VulkanDisable(enum)

//   // Blending
//   CHECK_TOGVulkanE_STATE(blendEnable, Vulkan_BLEND);
//   if(STATE_CHANGE(blendSrc) || STATE_CHANGE(blendDest))
//      VulkanBlendFunc(GFXVulkanBlend[mDesc.blendSrc], GFXVulkanBlend[mDesc.blendDest]);
//   if(STATE_CHANGE(blendOp))
//      VulkanBlendEquation(GFXVulkanBlendOp[mDesc.blendOp]);
//
//   if (mDesc.separateAlphaBlendEnable == true)
//   {
//       if (STATE_CHANGE(separateAlphaBlendSrc) || STATE_CHANGE(separateAlphaBlendDest))
//           VulkanBlendFuncSeparate(GFXVulkanBlend[mDesc.blendSrc], GFXVulkanBlend[mDesc.blendDest], GFXVulkanBlend[mDesc.separateAlphaBlendSrc], GFXVulkanBlend[mDesc.separateAlphaBlendDest]);
//       if (STATE_CHANGE(separateAlphaBlendOp))
//           VulkanBlendEquationSeparate(GFXVulkanBlendOp[mDesc.blendOp], GFXVulkanBlendOp[mDesc.separateAlphaBlendOp]);
//   }
//
//   // Color write masks
//   if(STATE_CHANGE(colorWriteRed) || STATE_CHANGE(colorWriteBlue) || STATE_CHANGE(colorWriteGreen) || STATE_CHANGE(colorWriteAlpha))
//      VulkanColorMask(mDesc.colorWriteRed, mDesc.colorWriteBlue, mDesc.colorWriteGreen, mDesc.colorWriteAlpha);
//   
//   // Culling
//   if(STATE_CHANGE(cullMode))
//   {
//      TOGVulkanE_STATE(cullMode, Vulkan_CULL_FACE);
//      VulkanCullFace(GFXVulkanCullMode[mDesc.cullMode]);
//   }
//
//   // Depth
//   CHECK_TOGVulkanE_STATE(zEnable, Vulkan_DEPTH_TEST);
//   
//   if(STATE_CHANGE(zFunc))
//      VulkanDepthFunc(GFXVulkanCmpFunc[mDesc.zFunc]);
//   
//   if (STATE_CHANGE(zBias))
//   {
//      if (mDesc.zBias == 0)
//      {
//         VulkanDisable(Vulkan_POLYGON_OFFSET_FILL);
//      }
//      else 
//      {
//         //this assumes 24bit depth
//         const F32 depthMul = F32((1 << 24) - 1);
//         VulkanEnable(Vulkan_POLYGON_OFFSET_FILL);
//         VulkanPolygonOffset(mDesc.zSlopeBias, mDesc.zBias * depthMul);
//      }
//   }
//   
//   if(STATE_CHANGE(zWriteEnable))
//      VulkanDepthMask(mDesc.zWriteEnable);
//
//   // Stencil
//   CHECK_TOGVulkanE_STATE(stencilEnable, Vulkan_STENCIL_TEST);
//   if(STATE_CHANGE(stencilFunc) || STATE_CHANGE(stencilRef) || STATE_CHANGE(stencilMask))
//      VulkanStencilFunc(GFXVulkanCmpFunc[mDesc.stencilFunc], mDesc.stencilRef, mDesc.stencilMask);
//   if(STATE_CHANGE(stencilFailOp) || STATE_CHANGE(stencilZFailOp) || STATE_CHANGE(stencilPassOp))
//      VulkanStencilOp(GFXVulkanStencilOp[mDesc.stencilFailOp], GFXVulkanStencilOp[mDesc.stencilZFailOp], GFXVulkanStencilOp[mDesc.stencilPassOp]);
//   if(STATE_CHANGE(stencilWriteMask))
//      VulkanStencilMask(mDesc.stencilWriteMask);
//   
//
//   if(STATE_CHANGE(fillMode))
//      VulkanPolygonMode(Vulkan_FRONT_AND_BACK, GFXVulkanFillMode[mDesc.fillMode]);
//
//#undef CHECK_STATE
//#undef TOGVulkanE_STATE
//#undef CHECK_TOGVulkanE_STATE
//
//   //sampler objects
//   if( GFXVulkan->mCapabilities.samplerObjects )
//   {
//      for (U32 i = 0; i < getMin(getOwningDevice()->getNumSamplers(), (U32) TEXTURE_STAGE_COUNT); i++)
//      {
//         if(!oldState || oldState->mSamplerObjects[i] != mSamplerObjects[i])
//            VulkanBindSampler(i, mSamplerObjects[i] );
//      }
//   }    
//
   // TODO: states added for detail blend   
}
