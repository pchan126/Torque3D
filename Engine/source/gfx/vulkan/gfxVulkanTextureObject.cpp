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

#include "console/console.h"
#include "vulkan/vulkan.h"
#include "math/mRect.h"
#include "gfx/vulkan/gfxVulkanTextureObject.h"
#include "gfx/gfxDevice.h"
#include "gfx/vulkan/gfxVulkanEnumTranslate.h"
#include "gfx/vulkan/gfxVulkanTextureManager.h"
#include "gfx/vulkan/gfxVulkanUtils.h"
#include "gfx/gfxCardProfile.h"


GFXVulkanTextureObject::GFXVulkanTextureObject(GFXDevice * aDevice, GFXTextureProfile *profile) :
   GFXTextureObject(aDevice, profile),
   mBytesPerTexel(4),
   mLockedRectRect(0, 0, 0, 0),
   mVulkanDevice(static_cast<GFXVulkanDevice*>(mDevice)),
   mZombieCache(NULL),
   mFrameAllocatorMark(0),
   mFrameAllocatorPtr(NULL)
{
   AssertFatal(dynamic_cast<GFXVulkanDevice*>(mDevice), "GFXVulkanTextureObject::GFXVulkanTextureObject - Invalid device type, expected GFXVulkanDevice!");
	//createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
}

GFXVulkanTextureObject::~GFXVulkanTextureObject() 
{ 
   //vulkanDeleteTextures(1, &mHandle);
   //vulkanDeleteBuffers(1, &mBuffer);
   delete[] mZombieCache;
   kill();
}

GFXLockedRect* GFXVulkanTextureObject::lock(U32 mipLevel, RectI *inRect)
{
   //AssertFatal(mBinding != Vulkan_TEXTURE_3D, "GFXVulkanTextureObject::lock - We don't support locking 3D textures yet");
   U32 width = mTextureSize.x >> mipLevel;
   U32 height = mTextureSize.y >> mipLevel;

   if(inRect)
   {
      if((inRect->point.x + inRect->extent.x > width) || (inRect->point.y + inRect->extent.y > height))
         AssertFatal(false, "GFXVulkanTextureObject::lock - Rectanvulkane too big!");

      mLockedRectRect = *inRect;
   }
   else
   {
      mLockedRectRect = RectI(0, 0, width, height);
   }
   
   mLockedRect.pitch = mLockedRectRect.extent.x * mBytesPerTexel;

   // CodeReview [ags 12/19/07] This one texel boundary is necessary to keep the clipmap code from crashing.  Figure out why.
   U32 size = (mLockedRectRect.extent.x + 1) * (mLockedRectRect.extent.y + 1) * getDepth() * mBytesPerTexel;
   AssertFatal(!mFrameAllocatorMark && !mFrameAllocatorPtr, "");
   mFrameAllocatorMark = FrameAllocator::getWaterMark();
   mFrameAllocatorPtr = (U8*)FrameAllocator::alloc( size );
   mLockedRect.bits = mFrameAllocatorPtr;
#if TORQUE_DEBUG
   mFrameAllocatorMarkGuard = FrameAllocator::getWaterMark();
#endif
   
   if( !mLockedRect.bits )
      return NULL;

   return &mLockedRect;
}

void GFXVulkanTextureObject::unlock(U32 mipLevel)
{
   if(!mLockedRect.bits)
      return;

   // I know this is in unlock, but in Vulkan we actually do our submission in unlock.
   PROFILE_SCOPE(GFXVulkanTextureObject_lockRT);

  // PRESERVE_TEXTURE(mBinding);
  // vulkanBindTexture(mBinding, mHandle);
  // vulkanBindBuffer(Vulkan_PIXEL_UNPACK_BUFFER, mBuffer);
  // vulkanBufferData(Vulkan_PIXEL_UNPACK_BUFFER, (mLockedRectRect.extent.x + 1) * (mLockedRectRect.extent.y + 1) * mBytesPerTexel, mFrameAllocatorPtr, Vulkan_STREAM_DRAW);
  // S32 z = getDepth();
  // if (mBinding == Vulkan_TEXTURE_3D)
  //    vulkanTexSubImage3D(mBinding, mipLevel, mLockedRectRect.point.x, mLockedRectRect.point.y, z,
  //    mLockedRectRect.extent.x, mLockedRectRect.extent.y, z, GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], NULL);
  // else if(mBinding == Vulkan_TEXTURE_2D)
	 //  vulkanTexSubImage2D(mBinding, mipLevel, mLockedRectRect.point.x, mLockedRectRect.point.y, 
		//  mLockedRectRect.extent.x, mLockedRectRect.extent.y, GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], NULL);
  // else if(mBinding == Vulkan_TEXTURE_1D)
		//vulkanTexSubImage1D(mBinding, mipLevel, (mLockedRectRect.point.x > 1 ? mLockedRectRect.point.x : mLockedRectRect.point.y), 
		//  (mLockedRectRect.extent.x > 1 ? mLockedRectRect.extent.x : mLockedRectRect.extent.y), GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], NULL);
  // 
  // vulkanBindBuffer(Vulkan_PIXEL_UNPACK_BUFFER, 0);

   mLockedRect.bits = NULL;
#if TORQUE_DEBUG
   AssertFatal(mFrameAllocatorMarkGuard == FrameAllocator::getWaterMark(), "");
#endif
   FrameAllocator::setWaterMark(mFrameAllocatorMark);
   mFrameAllocatorMark = 0;
   mFrameAllocatorPtr = NULL;
}

void GFXVulkanTextureObject::release()
{
   //vulkanDeleteTextures(1, &mHandle);
   //vulkanDeleteBuffers(1, &mBuffer);
   //
   //mHandle = 0;
   //mBuffer = 0;
}

void GFXVulkanTextureObject::reInit()
{
   //AssertFatal(!mHandle && !mBuffer,"Must release before reInit");
   //vulkanGenTextures(1, &mHandle);
   //vulkanGenBuffers(1, &mBuffer);
}

bool GFXVulkanTextureObject::copyToBmp(GBitmap * bmp)
{
   if (!bmp)
      return false;

   // check format limitations
   // at the moment we only support RGBA for the source (other 4 byte formats should
   // be easy to add though)
   AssertFatal(mFormat == GFXFormatR8G8B8A8 || mFormat == GFXFormatR8G8B8A8_SRGB , "GFXVulkanTextureObject::copyToBmp - invalid format");
   AssertFatal(bmp->getFormat() == GFXFormatR8G8B8A8 || bmp->getFormat() == GFXFormatR8G8B8 || bmp->getFormat() == GFXFormatR8G8B8A8_SRGB, "GFXVulkanTextureObject::copyToBmp - invalid format");
   
   if(mFormat != GFXFormatR8G8B8A8 && mFormat != GFXFormatR8G8B8A8_SRGB)
      return false;

   if(bmp->getFormat() != GFXFormatR8G8B8A8 && bmp->getFormat() != GFXFormatR8G8B8 && bmp->getFormat() != GFXFormatR8G8B8A8_SRGB )
      return false;

   AssertFatal(bmp->getWidth() == getWidth(), "GFXVulkanTextureObject::copyToBmp - invalid size");
   AssertFatal(bmp->getHeight() == getHeight(), "GFXVulkanTextureObject::copyToBmp - invalid size");

   PROFILE_SCOPE(GFXVulkanTextureObject_copyToBmp);

   //PRESERVE_TEXTURE(mBinding);
   //vulkanBindTexture(mBinding, mHandle);

   U8 dstBytesPerPixel = GFXFormat_getByteSize( bmp->getFormat() );
   U8 srcBytesPerPixel = GFXFormat_getByteSize( mFormat );
   if(dstBytesPerPixel == srcBytesPerPixel)
   {
      //vulkanGetTexImage(mBinding, 0, GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], bmp->getWritableBits());
      return true;
   }

   FrameAllocatorMarker mem;
   
   U32 srcPixelCount = mTextureSize.x * mTextureSize.y;
   U8 *dest = bmp->getWritableBits();
   U8 *orig = (U8*)mem.alloc(srcPixelCount * srcBytesPerPixel);

   //vulkanGetTexImage(mBinding, 0, GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], orig);
   
   PROFILE_START(GFXVulkanTextureObject_copyToBmp_pixCopy);
   for(int i = 0; i < srcPixelCount; ++i)
   {
      dest[0] = orig[0];
      dest[1] = orig[1];
      dest[2] = orig[2];
      if(dstBytesPerPixel == 4)
         dest[3] = orig[3];

      orig += srcBytesPerPixel;
      dest += dstBytesPerPixel;
   }
   PROFILE_END();

   return true;
}

void GFXVulkanTextureObject::initSamplerState(const GFXSamplerStateDesc &ssd)
{
   //vulkanTexParameteri(mBinding, Vulkan_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, mMipLevels));
   //vulkanTexParameteri(mBinding, Vulkan_TEXTURE_MAG_FILTER, GFXVulkanTextureFilter[ssd.magFilter]);
   //vulkanTexParameteri(mBinding, Vulkan_TEXTURE_WRAP_S, !mIsNPoT2 ? GFXVulkanTextureAddress[ssd.addressModeU] : Vulkan_CLAMP_TO_EDGE);
   //vulkanTexParameteri(mBinding, Vulkan_TEXTURE_WRAP_T, !mIsNPoT2 ? GFXVulkanTextureAddress[ssd.addressModeV] : Vulkan_CLAMP_TO_EDGE);
   //if(mBinding == Vulkan_TEXTURE_3D)
   //   vulkanTexParameteri(mBinding, Vulkan_TEXTURE_WRAP_R, GFXVulkanTextureAddress[ssd.addressModeW]);
   //if(static_cast< GFXVulkanDevice* >( GFX )->supportsAnisotropic() )
   //   vulkanTexParameterf(mBinding, Vulkan_TEXTURE_MAX_ANISOTROPY_EXT, ssd.maxAnisotropy);

   //mNeedInitSamplerState = false;
   //mSampler = ssd;
}

void GFXVulkanTextureObject::bind(U32 textureUnit)
{
   //vulkanActiveTexture(Vulkan_TEXTURE0 + textureUnit);
   //vulkanBindTexture(mBinding, mHandle);
   //GFXVulkan->getOpenvulkanCache()->setCacheBindedTex(textureUnit, mBinding, mHandle);

   //if(GFXVulkan->mCapabilities.samplerObjects)
	  // return;
  
   //GFXVulkanStateBlockRef sb = mVulkanDevice->getCurrentStateBlock();
   //AssertFatal(sb, "GFXVulkanTextureObject::bind - No active stateblock!");
   //if (!sb)
   //   return;
   //      
   //const GFXSamplerStateDesc ssd = sb->getDesc().samplers[textureUnit];

   //if(mNeedInitSamplerState)
   //{
   //   initSamplerState(ssd);
   //   return;
   //}

   //if(mSampler.minFilter != ssd.minFilter || mSampler.mipFilter != ssd.mipFilter)
   //   vulkanTexParameteri(mBinding, Vulkan_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, mMipLevels));
   //if(mSampler.magFilter != ssd.magFilter)
   //   vulkanTexParameteri(mBinding, Vulkan_TEXTURE_MAG_FILTER, GFXVulkanTextureFilter[ssd.magFilter]);
   //if(mSampler.addressModeU != ssd.addressModeU)
   //   vulkanTexParameteri(mBinding, Vulkan_TEXTURE_WRAP_S, !mIsNPoT2 ? GFXVulkanTextureAddress[ssd.addressModeU] : Vulkan_CLAMP_TO_EDGE);
   //if(mSampler.addressModeV != ssd.addressModeV)
   //   vulkanTexParameteri(mBinding, Vulkan_TEXTURE_WRAP_T, !mIsNPoT2 ? GFXVulkanTextureAddress[ssd.addressModeV] : Vulkan_CLAMP_TO_EDGE);
   //if(mBinding == Vulkan_TEXTURE_3D && mSampler.addressModeW != ssd.addressModeW )
   //   vulkanTexParameteri(mBinding, Vulkan_TEXTURE_WRAP_R, GFXVulkanTextureAddress[ssd.addressModeW]);
   //if(mSampler.maxAnisotropy != ssd.maxAnisotropy  && static_cast< GFXVulkanDevice* >( GFX )->supportsAnisotropic() )
   //   vulkanTexParameterf(mBinding, Vulkan_TEXTURE_MAX_ANISOTROPY_EXT, ssd.maxAnisotropy);

   //mSampler = ssd;
}

U8* GFXVulkanTextureObject::getTextureData( U32 mip )
{
   //AssertFatal( mMipLevels, "");
   //mip = (mip < mMipLevels) ? mip : 0;

   //const U32 dataSize = ImageUtil::isCompressedFormat(mFormat) 
   //    ? getCompressedSurfaceSize( mFormat, mTextureSize.x, mTextureSize.y, mip ) 
   //    : (mTextureSize.x >> mip) * (mTextureSize.y >> mip) * mBytesPerTexel;

   //U8* data = new U8[dataSize];
   //PRESERVE_TEXTURE(mBinding);
   //vulkanBindTexture(mBinding, mHandle);

   //if( ImageUtil::isCompressedFormat(mFormat) )
   //   vulkanGetCompressedTexImage( mBinding, mip, data );
   //else
   //   vulkanGetTexImage(mBinding, mip, GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], data);
   //return data;
	return nullptr;
}

void GFXVulkanTextureObject::copyIntoCache()
{
   //PRESERVE_TEXTURE(mBinding);
   //vulkanBindTexture(mBinding, mHandle);
   //U32 cacheSize = mTextureSize.x * mTextureSize.y;
   //if(mBinding == Vulkan_TEXTURE_3D)
   //   cacheSize *= mTextureSize.z;
   //   
   //cacheSize *= mBytesPerTexel;
   //mZombieCache = new U8[cacheSize];
   //
   //vulkanGetTexImage(mBinding, 0, GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], mZombieCache);
}

void GFXVulkanTextureObject::reloadFromCache()
{
   if(!mZombieCache)
      return;
      
  // if(mBinding == Vulkan_TEXTURE_3D)
  // {
  //    static_cast<GFXVulkanTextureManager*>(TEXMGR)->_loadTexture(this, mZombieCache);
  //    delete[] mZombieCache;
  //    mZombieCache = NULL;
  //    return;
  // }
  // 
  // PRESERVE_TEXTURE(mBinding);
  // vulkanBindTexture(mBinding, mHandle);

  // if(mBinding == Vulkan_TEXTURE_2D)
		//vulkanTexSubImage2D(mBinding, 0, 0, 0, mTextureSize.x, mTextureSize.y, GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], mZombieCache);
  // else if(mBinding == Vulkan_TEXTURE_1D)
		//vulkanTexSubImage1D(mBinding, 0, 0, (mTextureSize.x > 1 ? mTextureSize.x : mTextureSize.y), GFXVulkanTextureFormat[mFormat], GFXVulkanTextureType[mFormat], mZombieCache);
  // 
  // if(mMipLevels != 1)
  //    vulkanGenerateMipmap(mBinding);
  //    
  // delete[] mZombieCache;
  // mZombieCache = NULL;
  // mIsZombie = false;
}

void GFXVulkanTextureObject::zombify()
{
   if(mIsZombie)
      return;
      
   mIsZombie = true;
   if(!mProfile->doStoreBitmap() && !mProfile->isRenderTarget() && !mProfile->isDynamic() && !mProfile->isZTarget())
      copyIntoCache();
      
   release();
}

void GFXVulkanTextureObject::resurrect()
{
   if(!mIsZombie)
      return;
      
   //vulkanGenTextures(1, &mHandle);
   //vulkanGenBuffers(1, &mBuffer);
}

F32 GFXVulkanTextureObject::getMaxUCoord() const
{
   //return mBinding == Vulkan_TEXTURE_2D ? 1.0f : (F32)getWidth();
	return 1.0f;
}

F32 GFXVulkanTextureObject::getMaxVCoord() const
{
   //return mBinding == Vulkan_TEXTURE_2D ? 1.0f : (F32)getHeight();
	return 1.0f;
}

const String GFXVulkanTextureObject::describeSelf() const
{
   String ret = Parent::describeSelf();
   //ret += String::ToString("   Vulkan Handle: %i", mHandle);
   
   return ret;
}
