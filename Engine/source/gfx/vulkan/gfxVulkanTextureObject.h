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

#ifndef _GFXVulkanTEXTUREOBJECT_H
#define _GFXVulkanTEXTUREOBJECT_H


#include "gfx/gfxTextureObject.h"
#include "vulkan/vulkan.h"
#include "gfx/gfxStateBlock.h"

class GFXVulkanDevice;

class GFXVulkanTextureObject : public GFXTextureObject 
{
public:
   GFXVulkanTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile); 
   ~GFXVulkanTextureObject();
   
   void release();
   void reInit();
   
   inline VkSampler getHandle() const { return sampler; }
   inline VkImageType getBinding() const { return mBinding; }
   inline VkDeviceMemory getBuffer() const { return deviceMemory; }
   
   inline bool isZombie() const { return mIsZombie; }

   /// Binds the texture to the given texture unit
   /// and applies the current sampler state because Vulkan tracks
   /// filtering and wrapper per object, while GFX tracks per sampler.
   void bind(U32 textureUnit);
   
   /// @return An array containing the texture data
   /// @note You are responsible for deleting the returned data! (Use delete[])
   U8* getTextureData( U32 mip = 0);

   virtual F32 getMaxUCoord() const;
   virtual F32 getMaxVCoord() const;
   
   void reloadFromCache(); ///< Reloads texture from zombie cache, used by GFXVulkanTextureManager to resurrect the texture.
   
#ifdef TORQUE_DEBUG
   virtual void pureVirtualCrash() {}
#endif

   /// Get/set data from texture (for dynamic textures and render targets)
   /// @attention DO NOT READ FROM THE RETURNED RECT! It is not guaranteed to work and may incur significant performance penalties.
   virtual GFXLockedRect* lock(U32 mipLevel = 0, RectI *inRect = NULL);
   virtual void unlock(U32 mipLevel = 0 );

   virtual bool copyToBmp(GBitmap *); ///< Not implemented
   
   bool mIsNPoT2;

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   virtual const String describeSelf() const;

   bool initSamplerState(const GFXSamplerStateDesc &ssd);

	//bool CreateSampledImage(GFXFormat gfx_format);
	
   void transitionImageLayout( VkFormat format, VkImageLayout newLayout) const;

private:
   friend class GFXVulkanTextureManager;
   typedef GFXTextureObject Parent;

   bool mNeedInitSamplerState;

	/// Internal Vulkan object
		VkSampler sampler;
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView image_view;
	
   U32 mBytesPerTexel;
   GFXLockedRect mLockedRect;
   RectI mLockedRectRect;

   /// Pointer to owner device
   GFXVulkanDevice* mVulkanDevice;

	VkImageType mBinding;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
   bool mIsZombie;
   U8* mZombieCache;
   
   void copyIntoCache();

   //FrameAllocator
   U32 mFrameAllocatorMark;
#if TORQUE_DEBUG
   U32 mFrameAllocatorMarkGuard;
#endif
   U8 *mFrameAllocatorPtr;
};

#endif