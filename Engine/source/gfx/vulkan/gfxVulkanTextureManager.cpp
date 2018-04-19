//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//Vulkan
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
#include "gfx/vulkan/gfxVulkanTextureManager.h"
#include "gfx/vulkan/gfxVulkanEnumTranslate.h"
#include "gfx/gfxCardProfile.h"
#include "core/util/safeDelete.h"
//#include "gfx/vulkan/gfxVulkanUtils.h"

#include <squish.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
GFXVulkanTextureManager::GFXVulkanTextureManager()
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXVulkanTextureManager::~GFXVulkanTextureManager()
{
   SAFE_DELETE_ARRAY( mHashTable );
}

//-----------------------------------------------------------------------------
// createTexture
//-----------------------------------------------------------------------------
GFXTextureObject *GFXVulkanTextureManager::_createTextureObject(   U32 height, 
                                                               U32 width,
                                                               U32 depth,
                                                               GFXFormat format, 
                                                               GFXTextureProfile *profile, 
                                                               U32 numMipLevels,
                                                               bool forceMips,
                                                               S32 antialiasLevel,
                                                               GFXTextureObject *inTex )
{
   AssertFatal(format >= 0 && format < GFXFormat_COUNT, "GFXVulkanTextureManager::_createTexture - invalid format!");
	if (numMipLevels == 0) numMipLevels++;
   
	GFXVulkanTextureObject *retTex;
   if ( inTex )
   {
      AssertFatal( dynamic_cast<GFXVulkanTextureObject*>( inTex ), "GFXVulkanTextureManager::_createTexture() - Bad inTex type!" );
      retTex = static_cast<GFXVulkanTextureObject*>( inTex );
      retTex->release();
      retTex->reInit();
   }      
   else
   {
      retTex = new GFXVulkanTextureObject( GFX, profile );
      retTex->registerResourceWithDevice( GFX );
   }

   innerCreateTexture(retTex, height, width, depth, format, profile, numMipLevels, forceMips);

   return retTex;
}

//-----------------------------------------------------------------------------
// innerCreateTexture
//-----------------------------------------------------------------------------
// This just creates the texture, no info is actually loaded to it.  We do that later.
void GFXVulkanTextureManager::innerCreateTexture( GFXVulkanTextureObject *retTex, 
                                               U32 height, 
                                               U32 width, 
                                               U32 depth,
                                               GFXFormat format, 
                                               GFXTextureProfile *profile, 
                                               U32 numMipLevels,
                                               bool forceMips)
{
   // No 24 bit formats.  They trigger various oddities because hardware (and Apple's drivers apparently...) don't natively support them.
   if (format == GFXFormatR8G8B8)
      format = GFXFormatR8G8B8A8;
   else if (format == GFXFormatR8G8B8_SRGB)
      format = GFXFormatR8G8B8A8_SRGB;
      
   retTex->mFormat = format;
   retTex->mIsZombie = false;
   retTex->mIsNPoT2 = false;
   
   VkImageType binding = ( (height == 1 || width == 1) && ( height != width ) ) ? VK_IMAGE_TYPE_1D : ( (depth == 0) ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D );
   if((profile->testFlag(GFXTextureProfile::RenderTarget) || profile->testFlag(GFXTextureProfile::ZTarget)) && (!isPow2(width) || !isPow2(height)) && !depth)
      retTex->mIsNPoT2 = true;
   retTex->mBinding = binding;
   
//	VkImage image;
//	VkSampleCountFlagBits samples = static_cast<VkSampleCountFlagBits>(pDL->getBytesPerPixel()*8);
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	if (!GFXVulkan->createImage(VK_IMAGE_TYPE_2D, GFXVulkanTextureFormat[format], {width, height, 1}, numMipLevels, 1, samples, usage, false, retTex->image))
		return; 

    if(retTex->getBinding() == VK_IMAGE_TYPE_3D)
      return;
	
	VkDeviceMemory memory_object;
	if (!GFXVulkan->allocateAndBindMemoryObjectToImage(retTex->image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object))
		return;

	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	//VkImageView image_view;
	if (!GFXVulkan->createImageView(retTex->image, VK_IMAGE_VIEW_TYPE_2D, GFXVulkanTextureFormat[format], aspect, retTex->image_view))
		return;

	//retTex->image = image;
	//retTex->image_view = image_view;
}

//-----------------------------------------------------------------------------
// loadTexture - GBitmap
//-----------------------------------------------------------------------------

void GFXVulkanTextureManager::_textureUpload(const U32 width, const U32 height, const U32 bytesPerPixel,
                           const GFXVulkanTextureObject* texture, const GFXFormat fmt, const U8* pixels,
                           const U32 mip, Swizzle<U8, 4>* pSwizzle)
{
	auto logical_device = GFXVulkan->getLogicalDevice();
	U32 bufSize = width * height * bytesPerPixel;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

	GFXVulkan->createBuffer(bufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
	GFXVulkan->allocateAndBindMemoryObjectForBuffer(stagingBuffer, stagingBufferMemory, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
	
	void* data;
    vkMapMemory(GFXVulkan->getLogicalDevice(), stagingBufferMemory, 0, bufSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(bufSize));
    vkUnmapMemory(GFXVulkan->getLogicalDevice(), stagingBufferMemory);

	texture->transitionImageLayout(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GFXVulkan->copyBufferToImage(stagingBuffer, texture->image, width, height);

	//createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
   //vulkanBindBuffer(Vulkan_PIXEL_UNPACK_BUFFER, texture->getBuffer());
   //U32 bufSize = width * height * bytesPerPixel;
   //vulkanBufferData(Vulkan_PIXEL_UNPACK_BUFFER, bufSize, NULL, Vulkan_STREAM_DRAW);

   //if(pSwizzle)
   //{
   //   PROFILE_SCOPE(Swizzle32_Upload);
   //   U8* pboMemory = (U8*)dMalloc(bufSize);
   //   pSwizzle->ToBuffer(pboMemory, data, bufSize);
   //   vulkanBufferSubData(Vulkan_PIXEL_UNPACK_BUFFER, 0, bufSize, pboMemory);
   //   dFree(pboMemory);
   //}
   //else
   //{
   //   PROFILE_SCOPE(SwizzleNull_Upload);
   //   vulkanBufferSubData(Vulkan_PIXEL_UNPACK_BUFFER, 0, bufSize, data);
   //}

   //if (texture->getBinding() == VK_IMAGE_TYPE_2D)
   //   vulkanTexSubImage2D(texture->getBinding(), mip, 0, 0, width, height, GFXVulkanTextureFormat[fmt], GFXVulkanTextureType[fmt], NULL);
   //else
   //   vulkanTexSubImage1D(texture->getBinding(), mip, 0, (width > 1 ? width : height), GFXVulkanTextureFormat[fmt], GFXVulkanTextureType[fmt], NULL);

   //vulkanBindBuffer(Vulkan_PIXEL_UNPACK_BUFFER, 0);
}

bool GFXVulkanTextureManager::_loadTexture(GFXTextureObject *aTexture, GBitmap *pDL)
{
   PROFILE_SCOPE(GFXVulkanTextureManager_loadTexture);
	auto logical_device = dynamic_cast<GFXVulkanDevice*>(GFXDevice::get())->getLogicalDevice();
    GFXVulkanTextureObject *texture = dynamic_cast<GFXVulkanTextureObject*>(aTexture);

	// No 24bit formats.
   if(pDL->getFormat() == GFXFormatR8G8B8)
      pDL->setFormat(GFXFormatR8G8B8A8);
   else if (pDL->getFormat() == GFXFormatR8G8B8_SRGB)
      pDL->setFormat(GFXFormatR8G8B8A8_SRGB);
	
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(GFXVulkan->getPhysicalDevice(), GFXVulkanTextureFormat[pDL->getFormat()], &format_properties);
	if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
	{
		Con::errorf("Provided format is not supported for a sampled image.");
		return false;
	}

	_textureUpload(pDL->getWidth(),pDL->getHeight(),pDL->getBytesPerPixel(),texture,pDL->getFormat(), pDL->getBits());
	
	// Bind to edit
  // PRESERVE_TEXTURE(texture->getBinding());
  // vulkanBindTexture(texture->getBinding(), texture->getHandle());

//  _textureUpload(pDL->getWidth(),pDL->getHeight(),pDL->getBytesPerPixel(),texture,pDL->getFormat(), pDL->getBits(), 0);

  //if(!ImageUtil::isCompressedFormat(pDL->getFormat()))
  // vulkanGenerateMipmap(texture->getBinding());
   
   return true;
}

bool GFXVulkanTextureManager::_loadTexture(GFXTextureObject *aTexture, DDSFile *dds)
{
   //GFXVulkanTextureObject* texture = static_cast<GFXVulkanTextureObject*>(aTexture);
   //
   //AssertFatal(texture->getBinding() == Vulkan_TEXTURE_2D, 
   //   "GFXVulkanTextureManager::_loadTexture(DDSFile) - This method can only be used with 2D textures");
   //   
   //if(texture->getBinding() != Vulkan_TEXTURE_2D)
   //   return false;
   //
   //PRESERVE_TEXTURE(texture->getBinding());
   //vulkanBindTexture(texture->getBinding(), texture->getHandle());
   //U32 numMips = dds->mSurfaces[0]->mMips.size();
   //const GFXFormat fmt = texture->mFormat;

   //for(U32 i = 0; i < numMips; i++)
   //{
   //   PROFILE_SCOPE(GFXVulkanTexMan_loadSurface);

   //   if(ImageUtil::isCompressedFormat(texture->mFormat))
   //   {
   //      if((!isPow2(dds->getWidth()) || !isPow2(dds->getHeight())) && GFX->getCardProfiler()->queryProfile("Vulkan::Workaround::noCompressedNPoTTextures"))
   //      {
   //         U8* uncompressedTex = new U8[dds->getWidth(i) * dds->getHeight(i) * 4];
   //         ImageUtil::decompress(dds->mSurfaces[0]->mMips[i],uncompressedTex, dds->getWidth(i), dds->getHeight(i), fmt);
   //         vulkanTexSubImage2D(texture->getBinding(), i, 0, 0, dds->getWidth(i), dds->getHeight(i), Vulkan_RGBA, Vulkan_UNSIGNED_BYTE, uncompressedTex);
   //         delete[] uncompressedTex;
   //      }
   //      else
   //         vulkanCompressedTexSubImage2D(texture->getBinding(), i, 0, 0, dds->getWidth(i), dds->getHeight(i), GFXVulkanTextureInternalFormat[fmt], dds->getSurfaceSize(dds->getHeight(), dds->getWidth(), i), dds->mSurfaces[0]->mMips[i]);
   //   }
   //   else
   //   {
   //      Swizzle<U8, 4> *pSwizzle = NULL;
   //      if (fmt == GFXFormatR8G8B8A8 || fmt == GFXFormatR8G8B8X8 || fmt == GFXFormatR8G8B8A8_SRGB || fmt == GFXFormatR8G8B8A8_LINEAR_FORCE || fmt == GFXFormatB8G8R8A8)
   //         pSwizzle = &Swizzles::bgra;

   //      _textureUpload(dds->getWidth(i), dds->getHeight(i),dds->mBytesPerPixel, texture, fmt, dds->mSurfaces[0]->mMips[i],i, pSwizzle);
   //   }
   //}

   //if(numMips !=1 && !ImageUtil::isCompressedFormat(texture->mFormat))
   //   vulkanGenerateMipmap(texture->getBinding());
   //
   return true;
}

bool GFXVulkanTextureManager::_loadTexture(GFXTextureObject *aTexture, void *raw)
{
   //PROFILE_SCOPE(GFXVulkanTextureManager_loadTextureRaw);
   //if(aTexture->getDepth() < 1)
   //   return false;
   //
   //GFXVulkanTextureObject* texture = static_cast<GFXVulkanTextureObject*>(aTexture);
   //
   //PRESERVE_3D_TEXTURE();
   //vulkanBindTexture(texture->getBinding(), texture->getHandle());
   //vulkanTexSubImage3D(Vulkan_TEXTURE_3D, 0, 0, 0, 0, texture->getWidth(), texture->getHeight(), texture->getDepth(), GFXVulkanTextureFormat[texture->mFormat], GFXVulkanTextureType[texture->mFormat], raw);
   
   return true;
}

bool GFXVulkanTextureManager::_freeTexture(GFXTextureObject *texture, bool zombify /*= false*/)
{
   if(zombify)
      static_cast<GFXVulkanTextureObject*>(texture)->zombify();
   else
      static_cast<GFXVulkanTextureObject*>(texture)->release();
      
   return true;
}

bool GFXVulkanTextureManager::_refreshTexture(GFXTextureObject *texture)
{
   U32 usedStrategies = 0;
   GFXVulkanTextureObject* realTex = static_cast<GFXVulkanTextureObject*>(texture);
      
   if(texture->mProfile->doStoreBitmap())
   {
      if(realTex->isZombie())
      {
         realTex->resurrect();
         innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, texture->mProfile, texture->mMipLevels);
      }
      if(texture->mBitmap)
         _loadTexture(texture, texture->mBitmap);
      
      if(texture->mDDS)
         return false;
      
      usedStrategies++;
   }
   
   if(texture->mProfile->isRenderTarget() || texture->mProfile->isDynamic() || texture->mProfile->isZTarget() || !usedStrategies)
   {
      realTex->release();
      realTex->resurrect();
      innerCreateTexture(realTex, texture->getHeight(), texture->getWidth(), texture->getDepth(), texture->mFormat, texture->mProfile, texture->mMipLevels);
      realTex->reloadFromCache();
      usedStrategies++;
   }
   
   AssertFatal(usedStrategies < 2, "GFXVulkanTextureManager::_refreshTexture - Inconsistent profile flags (store bitmap and dynamic/target");
   
   return true;
}
