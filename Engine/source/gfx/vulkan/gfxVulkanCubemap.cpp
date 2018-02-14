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

#include "gfx/vulkan/gfxVulkanDevice.h"
#include "gfx/vulkan/gfxVulkanTextureObject.h"
#include "gfx/vulkan/gfxVulkanEnumTranslate.h"
//#include "gfx/Vulkan/gfxVulkanUtils.h"
#include "gfx/vulkan/gfxVulkanCubemap.h"
#include "gfx/gfxTextureManager.h"
#include "gfx/gfxCardProfile.h"
#include "gfx/bitmap/ddsFile.h"
#include "gfx/bitmap/imageUtils.h"


GFXVulkanCubemap::GFXVulkanCubemap() :
      mCubemap(0), 
      mDynamicTexSize(0),
      mFaceFormat( GFXFormatR8G8B8A8 )
{
   for(U32 i = 0; i < 6; i++)
      mTextures[i] = NULL;
   
   GFXTextureManager::addEventDelegate( this, &GFXVulkanCubemap::_onTextureEvent );
}

GFXVulkanCubemap::~GFXVulkanCubemap()
{
   //VulkanDeleteTextures(1, &mCubemap);
   GFXTextureManager::removeEventDelegate( this, &GFXVulkanCubemap::_onTextureEvent );
}

void GFXVulkanCubemap::fillCubeTextures(GFXTexHandle* faces)
{
   AssertFatal( faces, "");
   AssertFatal( faces[0]->mMipLevels > 0, "");

   //PRESERVE_CUBEMAP_TEXTURE();
   //VulkanBindTexture(Vulkan_TEXTURE_CUBE_MAP, mCubemap);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MAX_LEVEL, faces[0]->mMipLevels - 1 );
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MIN_FILTER, Vulkan_NEAREST);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MAG_FILTER, Vulkan_NEAREST);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_S, Vulkan_CLAMP_TO_EDGE);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_T, Vulkan_CLAMP_TO_EDGE);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_R, Vulkan_CLAMP_TO_EDGE);
   //
   //U32 reqWidth = faces[0]->getWidth();
   //U32 reqHeight = faces[0]->getHeight();
   //GFXFormat regFaceFormat = faces[0]->getFormat();
   //const bool isCompressed = ImageUtil::isCompressedFormat(regFaceFormat);
   //mWidth = reqWidth;
   //mHeight = reqHeight;
   //mFaceFormat = regFaceFormat;
   //mMipMapLevels = getMax( (U32)1, faces[0]->mMipLevels);
   //AssertFatal(reqWidth == reqHeight, "GFXVulkanCubemap::fillCubeTextures - Width and height must be equal!");
   //
   //for(U32 i = 0; i < 6; i++)
   //{
   //   AssertFatal(faces[i], avar("GFXVulkanCubemap::fillCubeFaces - texture %i is NULL!", i));
   //   AssertFatal((faces[i]->getWidth() == reqWidth) && (faces[i]->getHeight() == reqHeight), "GFXVulkanCubemap::fillCubeFaces - All textures must have identical dimensions!");
   //   AssertFatal(faces[i]->getFormat() == regFaceFormat, "GFXVulkanCubemap::fillCubeFaces - All textures must have identical formats!");
   //   
   //   mTextures[i] = faces[i];
   //   GFXFormat faceFormat = faces[i]->getFormat();

   //     GFXVulkanTextureObject* VulkanTex = static_cast<GFXVulkanTextureObject*>(faces[i].getPointer());
   //     if( isCompressed )
   //     {
   //         for( U32 mip = 0; mip < mMipMapLevels; ++mip )
   //         {
   //             const U32 mipWidth  = getMax( U32(1), faces[i]->getWidth() >> mip );
   //             const U32 mipHeight = getMax( U32(1), faces[i]->getHeight() >> mip );
   //             const U32 mipDataSize = getCompressedSurfaceSize( mFaceFormat, mWidth, mHeight, mip );

   //             U8* buf = VulkanTex->getTextureData( mip );
   //             VulkanCompressedTexImage2D(faceList[i], mip, GFXVulkanTextureInternalFormat[mFaceFormat], mipWidth, mipHeight, 0, mipDataSize, buf);
   //             delete[] buf;
   //         }
   //     }
   //     else
   //     {
   //         U8* buf = VulkanTex->getTextureData();
   //         VulkanTexImage2D(faceList[i], 0, GFXVulkanTextureInternalFormat[faceFormat], mWidth, mHeight, 
   //             0, GFXVulkanTextureFormat[faceFormat], GFXVulkanTextureType[faceFormat], buf);
   //         delete[] buf;
   //     }
   //}
   //
   // if( !isCompressed )
   //     VulkanGenerateMipmap(Vulkan_TEXTURE_CUBE_MAP);
}

void GFXVulkanCubemap::initStatic(GFXTexHandle* faces)
{
   if(mCubemap)
      return;
      
   //if(faces)
   //{
   //   AssertFatal(faces[0], "GFXVulkanCubemap::initStatic - empty texture passed");
   //   VulkanGenTextures(1, &mCubemap);
   //   fillCubeTextures(faces);
   //}
}

void GFXVulkanCubemap::initStatic( DDSFile *dds )
{
   if(mCubemap)
      return;
      
   //AssertFatal( dds, "GFXVulkanCubemap::initStatic - Got null DDS file!" );
   //AssertFatal( dds->isCubemap(), "GFXVulkanCubemap::initStatic - Got non-cubemap DDS file!" );
   //AssertFatal( dds->mSurfaces.size() == 6, "GFXVulkanCubemap::initStatic - DDS has less than 6 surfaces!" );

   //mWidth = dds->getWidth();
   //mHeight = dds->getHeight();
   //mFaceFormat = dds->getFormat();
   //mMipMapLevels = dds->getMipLevels();
   //const bool isCompressed = ImageUtil::isCompressedFormat(mFaceFormat);
   //VulkanGenTextures(1, &mCubemap);

   //PRESERVE_CUBEMAP_TEXTURE();
   //VulkanBindTexture(Vulkan_TEXTURE_CUBE_MAP, mCubemap);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MAX_LEVEL, mMipMapLevels - 1);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MIN_FILTER, Vulkan_NEAREST);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MAG_FILTER, Vulkan_NEAREST);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_S, Vulkan_CLAMP_TO_EDGE);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_T, Vulkan_CLAMP_TO_EDGE);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_R, Vulkan_CLAMP_TO_EDGE);

   //AssertFatal(mWidth == mHeight, "GFXVulkanCubemap::initStatic - Width and height must be equal!");
   //
   //for(U32 i = 0; i < 6; i++)
   //{
   //   if ( !dds->mSurfaces[i] )
   //   {
   //      // TODO: The DDS can skip surfaces, but i'm unsure what i should
   //      // do here when creating the cubemap.  Ignore it for now.
   //      continue;
   //   }

   //   // convert to Z up
   //   const U32 faceIndex = _zUpFaceIndex(i);

   //   // Now loop thru the mip levels!
   //   for (U32 mip = 0; mip < mMipMapLevels; ++mip)
   //   {
   //      const U32 mipWidth  = getMax( U32(1), mWidth >> mip );
   //      const U32 mipHeight = getMax( U32(1), mHeight >> mip );
   //      if (isCompressed)
   //         VulkanCompressedTexImage2D(faceList[faceIndex], mip, GFXVulkanTextureInternalFormat[mFaceFormat], mipWidth, mipHeight, 0, dds->getSurfaceSize(mip), dds->mSurfaces[i]->mMips[mip]);
   //      else
   //         VulkanTexImage2D(faceList[faceIndex], mip, GFXVulkanTextureInternalFormat[mFaceFormat], mipWidth, mipHeight, 0,
   //            GFXVulkanTextureFormat[mFaceFormat], GFXVulkanTextureType[mFaceFormat], dds->mSurfaces[i]->mMips[mip]);
   //   }
   //}
}

void GFXVulkanCubemap::initDynamic(U32 texSize, GFXFormat faceFormat)
{
   //mDynamicTexSize = texSize;
   //mFaceFormat = faceFormat;
   //const bool isCompressed = ImageUtil::isCompressedFormat(faceFormat);
   //mMipMapLevels = getMax( (U32)1, getMaxMipmaps( texSize, texSize, 1 ) );

   //VulkanGenTextures(1, &mCubemap);
   //PRESERVE_CUBEMAP_TEXTURE();
   //VulkanBindTexture(Vulkan_TEXTURE_CUBE_MAP, mCubemap);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MAX_LEVEL, mMipMapLevels - 1);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MIN_FILTER, Vulkan_NEAREST);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MAG_FILTER, Vulkan_NEAREST);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_S, Vulkan_CLAMP_TO_EDGE);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_T, Vulkan_CLAMP_TO_EDGE);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_R, Vulkan_CLAMP_TO_EDGE);
   //mWidth = texSize;
   //mHeight = texSize;

   // for(U32 i = 0; i < 6; i++)
   // {
   //     if( ImageUtil::isCompressedFormat(faceFormat) )
   //     {
   //         for( U32 mip = 0; mip < mMipMapLevels; ++mip )
   //         {
   //             const U32 mipSize = getMax( U32(1), texSize >> mip );
   //             const U32 mipDataSize = getCompressedSurfaceSize( mFaceFormat, texSize, texSize, mip );
   //             VulkanCompressedTexImage2D(faceList[i], mip, GFXVulkanTextureInternalFormat[mFaceFormat], mipSize, mipSize, 0, mipDataSize, NULL);
   //         }
   //     }
   //     else
   //     {
   //         VulkanTexImage2D( faceList[i], 0, GFXVulkanTextureInternalFormat[faceFormat], texSize, texSize, 
   //             0, GFXVulkanTextureFormat[faceFormat], GFXVulkanTextureType[faceFormat], NULL);
   //     }
   // }

   // if( !isCompressed )
   //     VulkanGenerateMipmap(Vulkan_TEXTURE_CUBE_MAP);
}

void GFXVulkanCubemap::zombify()
{
   //VulkanDeleteTextures(1, &mCubemap);
   mCubemap = 0;
}

void GFXVulkanCubemap::resurrect()
{
   // Handled in tmResurrect
}

void GFXVulkanCubemap::tmResurrect()
{
   if(mDynamicTexSize)
      initDynamic(mDynamicTexSize,mFaceFormat);
   else
   {
      if ( mDDSFile )
         initStatic( mDDSFile );
      else
         initStatic( mTextures );
   }
}

void GFXVulkanCubemap::setToTexUnit(U32 tuNum)
{
   //static_cast<GFXVulkanDevice*>(getOwningDevice())->setCubemapInternal(tuNum, this);
}

void GFXVulkanCubemap::bind(U32 textureUnit) const
{
   //VulkanActiveTexture(Vulkan_TEXTURE0 + textureUnit);
   //VulkanBindTexture(Vulkan_TEXTURE_CUBE_MAP, mCubemap);
   //static_cast<GFXVulkanDevice*>(getOwningDevice())->getOpenVulkanCache()->setCacheBindedTex(textureUnit, Vulkan_TEXTURE_CUBE_MAP, mCubemap);
   //
   //GFXVulkanStateBlockRef sb = static_cast<GFXVulkanDevice*>(GFX)->getCurrentStateBlock();
   //AssertFatal(sb, "GFXVulkanCubemap::bind - No active stateblock!");
   //if (!sb)
   //   return;   
   //   
   //const GFXSamplerStateDesc& ssd = sb->getDesc().samplers[textureUnit];
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, 0));   
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_MAG_FILTER, GFXVulkanTextureFilter[ssd.magFilter]);   
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_S, GFXVulkanTextureAddress[ssd.addressModeU]);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_T, GFXVulkanTextureAddress[ssd.addressModeV]);
   //VulkanTexParameteri(Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_WRAP_R, GFXVulkanTextureAddress[ssd.addressModeW]);
}

void GFXVulkanCubemap::_onTextureEvent( GFXTexCallbackCode code )
{
   if ( code == GFXZombify )
      zombify();
   else
      tmResurrect();
}
