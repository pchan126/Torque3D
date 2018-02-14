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
#include "gfx/vulkan/gfxVulkanDevice.h"
#include "gfx/vulkan/gfxVulkanTextureTarget.h"
#include "gfx/vulkan/gfxVulkanTextureObject.h"
#include "gfx/vulkan/gfxVulkanCubemap.h"
#include "gfx/gfxTextureManager.h"
//#include "gfx/vulkan/gfxVulkanUtils.h"

/// Internal struct used to track texture information for FBO attachments
/// This serves as an abstract base so we can deal with cubemaps and standard 
/// 2D/Rect textures through the same interface
class _GFXVulkanTargetDesc
{
public:
   _GFXVulkanTargetDesc(U32 _mipLevel, U32 _zOffset) :
      mipLevel(_mipLevel), zOffset(_zOffset)
   {
   }
   
   virtual ~_GFXVulkanTargetDesc() {}
   
   virtual U32 getHandle() = 0;
   virtual U32 getWidth() = 0;
   virtual U32 getHeight() = 0;
   virtual U32 getDepth() = 0;
   virtual bool hasMips() = 0;
   //virtual Vulkanenum getBinding() = 0;
   virtual GFXFormat getFormat() = 0;
   virtual bool isCompatible(const GFXVulkanTextureObject* tex) = 0;
   
   U32 getMipLevel() { return mipLevel; }
   U32 getZOffset() { return zOffset; }
   
private:
   U32 mipLevel;
   U32 zOffset;
};

/// Internal struct used to track 2D/Rect texture information for FBO attachment
class _GFXVulkanTextureTargetDesc : public _GFXVulkanTargetDesc
{
public:
   _GFXVulkanTextureTargetDesc(GFXVulkanTextureObject* tex, U32 _mipLevel, U32 _zOffset) 
      : _GFXVulkanTargetDesc(_mipLevel, _zOffset), mTex(tex)
   {
   }
   
   virtual ~_GFXVulkanTextureTargetDesc() {}
   
   virtual U32 getHandle() { return 0; }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return mTex->getDepth(); }
   virtual bool hasMips() { return mTex->mMipLevels != 1; }
   virtual VkSampler  getBinding() { return mTex->getBinding(); }
   virtual GFXFormat getFormat() { return mTex->getFormat(); }
   virtual bool isCompatible(const GFXVulkanTextureObject* tex)
   {
      return mTex->getFormat() == tex->getFormat()
         && mTex->getWidth() == tex->getWidth()
         && mTex->getHeight() == tex->getHeight();
   }
   GFXVulkanTextureObject* getTextureObject() const {return mTex; }
   
private:
   StrongRefPtr<GFXVulkanTextureObject> mTex;
};

/// Internal struct used to track Cubemap texture information for FBO attachment
class _GFXVulkanCubemapTargetDesc : public _GFXVulkanTargetDesc
{
public:
   _GFXVulkanCubemapTargetDesc(GFXVulkanCubemap* tex, U32 _face, U32 _mipLevel, U32 _zOffset) 
      : _GFXVulkanTargetDesc(_mipLevel, _zOffset), mTex(tex), mFace(_face)
   {
   }
   
   virtual ~_GFXVulkanCubemapTargetDesc() {}
   
   virtual U32 getHandle() { return mTex->getHandle(); }
   virtual U32 getWidth() { return mTex->getWidth(); }
   virtual U32 getHeight() { return mTex->getHeight(); }
   virtual U32 getDepth() { return 0; }
   virtual bool hasMips() { return mTex->getMipMapLevels() != 1; }
   //virtual  getBinding() { return GFXVulkanCubemap::getEnumForFaceNumber(mFace); }
   virtual GFXFormat getFormat() { return mTex->getFormat(); }
   virtual bool isCompatible(const GFXVulkanTextureObject* tex)
   {
	   return true;
      //return mTex->getFormat() == tex->getFormat()
      //   && mTex->getWidth() == tex->getWidth()
      //   && mTex->getHeight() == tex->getHeight();
   }
   
private:
   StrongRefPtr<GFXVulkanCubemap> mTex;
   U32 mFace;
};

// Internal implementations
class _GFXVulkanTextureTargetImpl // TODO OPENVulkan remove and implement on GFXVulkanTextureTarget
{
public:
   GFXVulkanTextureTarget* mTarget;
   
   virtual ~_GFXVulkanTextureTargetImpl() {}
   
   virtual void applyState() = 0;
   virtual void makeActive() = 0;
   virtual void finish() = 0;
};

// Use FBOs to render to texture.  This is the preferred implementation and is almost always used.
class _GFXVulkanTextureTargetFBOImpl : public _GFXVulkanTextureTargetImpl
{
public:
   //Vulkanuint mFramebuffer;
   
   _GFXVulkanTextureTargetFBOImpl(GFXVulkanTextureTarget* target);
   virtual ~_GFXVulkanTextureTargetFBOImpl();
   
   virtual void applyState();
   virtual void makeActive();
   virtual void finish();
};

_GFXVulkanTextureTargetFBOImpl::_GFXVulkanTextureTargetFBOImpl(GFXVulkanTextureTarget* target)
{
   mTarget = target;
   //vulkanGenFramebuffers(1, &mFramebuffer);
}

_GFXVulkanTextureTargetFBOImpl::~_GFXVulkanTextureTargetFBOImpl()
{
   //vulkanDeleteFramebuffers(1, &mFramebuffer);
}

void _GFXVulkanTextureTargetFBOImpl::applyState()
{   
   // REMINDER: When we implement MRT support, check against GFXVulkanDevice::getNumRenderTargets()
   
   //PRESERVE_FRAMEBUFFER();
   //vulkanBindFramebuffer(Vulkan_FRAMEBUFFER, mFramebuffer);
   //vulkanEnable(Vulkan_FRAMEBUFFER_SRGB);
   //bool drawbufs[16];
   //int bufsize = 0;
   //for (int i = 0; i < 16; i++)
   //        drawbufs[i] = false;
   //bool hasColor = false;
   //for(int i = 0; i < GFXVulkan->getNumRenderTargets(); ++i)
   //{   
   //   _GFXVulkanTargetDesc* color = mTarget->getTargetDesc( static_cast<GFXTextureTarget::RenderSlot>(GFXTextureTarget::Color0+i ));
   //   if(color)
   //   {
   //      hasColor = true;
   //      const Vulkanenum binding = color->getBinding();
   //      if( binding == Vulkan_TEXTURE_2D || (binding >= Vulkan_TEXTURE_CUBE_MAP_POSITIVE_X && binding <= Vulkan_TEXTURE_CUBE_MAP_NEGATIVE_Z) )
   //         vulkanFramebufferTexture2D( Vulkan_FRAMEBUFFER, Vulkan_COLOR_ATTACHMENT0 + i, color->getBinding( ), color->getHandle( ), color->getMipLevel( ) );
   //      else if( binding == Vulkan_TEXTURE_1D )
   //         vulkanFramebufferTexture1D( Vulkan_FRAMEBUFFER, Vulkan_COLOR_ATTACHMENT0 + i, color->getBinding( ), color->getHandle( ), color->getMipLevel( ) );
   //      else if( binding == Vulkan_TEXTURE_3D )
   //         vulkanFramebufferTexture3D( Vulkan_FRAMEBUFFER, Vulkan_COLOR_ATTACHMENT0 + i, color->getBinding( ), color->getHandle( ), color->getMipLevel( ), color->getZOffset( ) );
   //      else
   //          Con::errorf("_GFXVulkanTextureTargetFBOImpl::applyState - Bad binding");
   //   }
   //   else
   //   {
   //      // Clears the texture (note that the binding is irrelevent)
   //      vulkanFramebufferTexture2D(Vulkan_FRAMEBUFFER, Vulkan_COLOR_ATTACHMENT0+i, Vulkan_TEXTURE_2D, 0, 0);
   //   }
   //}
   //
   //_GFXVulkanTargetDesc* depthStecil = mTarget->getTargetDesc(GFXTextureTarget::DepthStencil);
   //if(depthStecil)
   //{
   //   // Certain drivers have issues with depth only FBOs.  That and the next two asserts assume we have a color target.
   //   AssertFatal(hasColor, "GFXVulkanTextureTarget::applyState() - Cannot set DepthStencil target without Color0 target!");
   //   vulkanFramebufferTexture2D(Vulkan_FRAMEBUFFER, Vulkan_DEPTH_STENCIL_ATTACHMENT, depthStecil->getBinding(), depthStecil->getHandle(), depthStecil->getMipLevel());
   //}
   //else
   //{
   //   // Clears the texture (note that the binding is irrelevent)
   //   vulkanFramebufferTexture2D(Vulkan_FRAMEBUFFER, Vulkan_DEPTH_ATTACHMENT, Vulkan_TEXTURE_2D, 0, 0);
   //}

   //Vulkanenum *buf = new Vulkanenum[bufsize];
   //int count = 0;
   //for (int i = 0; i < bufsize; i++)
   //{
   //        if (drawbufs[i])
   //        {
   //                buf[count] = Vulkan_COLOR_ATTACHMENT0 + i;
   //                count++;
   //        }
   //}
 
   //vulkanDrawBuffers(bufsize, buf);
 
   //delete[] buf;
   //CHECK_FRAMEBUFFER_STATUS();
}

void _GFXVulkanTextureTargetFBOImpl::makeActive()
{
    //vulkanBindFramebuffer(Vulkan_FRAMEBUFFER, mFramebuffer);
    //GFXVulkan->getOpenvulkanCache()->setCacheBinded(Vulkan_FRAMEBUFFER, mFramebuffer);

    //int i = 0;
    //Vulkanenum draws[16];
    //for( i = 0; i < GFXVulkan->getNumRenderTargets(); ++i)
    //{
    //    _GFXVulkanTargetDesc* color = mTarget->getTargetDesc( static_cast<GFXTextureTarget::RenderSlot>(GFXTextureTarget::Color0+i ));
    //    if(color)
    //        draws[i] = Vulkan_COLOR_ATTACHMENT0 + i;
    //    else
    //        break;
    //}

    //vulkanDrawBuffers( i, draws );
}

void _GFXVulkanTextureTargetFBOImpl::finish()
{
   //vulkanBindFramebuffer(Vulkan_FRAMEBUFFER, 0);
   //GFXVulkan->getOpenvulkanCache()->setCacheBinded(Vulkan_FRAMEBUFFER, 0);

   //for(int i = 0; i < GFXVulkan->getNumRenderTargets(); ++i)
   //{   
   //   _GFXVulkanTargetDesc* color = mTarget->getTargetDesc( static_cast<GFXTextureTarget::RenderSlot>(GFXTextureTarget::Color0+i ) );
   //   if(!color || !(color->hasMips()))
   //      continue;
   //
   //   // Generate mips if necessary
   //   // Assumes a 2D texture.
   //   Vulkanenum binding = color->getBinding();
   //   binding = (binding >= Vulkan_TEXTURE_CUBE_MAP_POSITIVE_X && binding <= Vulkan_TEXTURE_CUBE_MAP_NEGATIVE_Z) ? Vulkan_TEXTURE_CUBE_MAP : binding;

   //   PRESERVE_TEXTURE( binding );
   //   vulkanBindTexture( binding, color->getHandle() );
   //   vulkanGenerateMipmap( binding );
   //}
}

// Actual GFXVulkanTextureTarget interface
GFXVulkanTextureTarget::GFXVulkanTextureTarget() // : mCopyFboSrc(0), mCopyFboDst(0)
{
   //for(U32 i=0; i<MaxRenderSlotId; i++)
   //   mTargets[i] = NULL;
   //
   //GFXTextureManager::addEventDelegate( this, &GFXVulkanTextureTarget::_onTextureEvent );

   //_impl = new _GFXVulkanTextureTargetFBOImpl(this);
   // 
   //vulkanGenFramebuffers(1, &mCopyFboSrc);
   //vulkanGenFramebuffers(1, &mCopyFboDst);
}

GFXVulkanTextureTarget::~GFXVulkanTextureTarget()
{
   //GFXTextureManager::removeEventDelegate(this, &GFXVulkanTextureTarget::_onTextureEvent);

   //vulkanDeleteFramebuffers(1, &mCopyFboSrc);
   //vulkanDeleteFramebuffers(1, &mCopyFboDst);
}

const Point2I GFXVulkanTextureTarget::getSize()
{
   if(mTargets[Color0].isValid())
      return Point2I(mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight());

   return Point2I(0, 0);
}

GFXFormat GFXVulkanTextureTarget::getFormat()
{
   if(mTargets[Color0].isValid())
      return mTargets[Color0]->getFormat();

   return GFXFormatR8G8B8A8;
}

void GFXVulkanTextureTarget::attachTexture( RenderSlot slot, GFXTextureObject *tex, U32 mipLevel/*=0*/, U32 zOffset /*= 0*/ )
{
   if( tex == GFXTextureTarget::sDefaultDepthStencil )
      tex = GFXVulkan->getDefaultDepthTex();

   _GFXVulkanTextureTargetDesc* mTex = static_cast<_GFXVulkanTextureTargetDesc*>(mTargets[slot].ptr());
   if( (!tex && !mTex) || (mTex && mTex->getTextureObject() == tex) )
      return;
   
   // Triggers an update when we next render
   invalidateState();

   // We stash the texture and info into an internal struct.
   GFXVulkanTextureObject* vulkanTexture = static_cast<GFXVulkanTextureObject*>(tex);
   if(tex && tex != GFXTextureTarget::sDefaultDepthStencil)
      mTargets[slot] = new _GFXVulkanTextureTargetDesc(vulkanTexture, mipLevel, zOffset);
   else
      mTargets[slot] = NULL;
}

void GFXVulkanTextureTarget::attachTexture( RenderSlot slot, GFXCubemap *tex, U32 face, U32 mipLevel/*=0*/ )
{
   // No depth cubemaps, sorry
   AssertFatal(slot != DepthStencil, "GFXVulkanTextureTarget::attachTexture (cube) - Cube depth textures not supported!");
   if(slot == DepthStencil)
      return;
    
   // Triggers an update when we next render
   invalidateState();
   
   //// We stash the texture and info into an internal struct.
   //GFXVulkanCubemap* vulkanTexture = static_cast<GFXVulkanCubemap*>(tex);
   //if(tex)
   //   mTargets[slot] = new _GFXVulkanCubemapTargetDesc(vulkanTexture, face, mipLevel, 0);
   //else
      mTargets[slot] = NULL;
}

void GFXVulkanTextureTarget::clearAttachments()
{
   deactivate();
   for(S32 i=1; i<MaxRenderSlotId; i++)
      attachTexture((RenderSlot)i, NULL);
}

void GFXVulkanTextureTarget::zombify()
{
   invalidateState();
   
   // Will be recreated in applyState
   _impl = NULL;
}

void GFXVulkanTextureTarget::resurrect()
{
   // Dealt with when the target is next bound
}

void GFXVulkanTextureTarget::makeActive()
{
   _impl->makeActive();
}

void GFXVulkanTextureTarget::deactivate()
{
   _impl->finish();
}

void GFXVulkanTextureTarget::applyState()
{
   if(!isPendingState())
      return;

   // So we don't do this over and over again
   stateApplied();
   
   if(_impl.isNull())
      _impl = new _GFXVulkanTextureTargetFBOImpl(this);
           
   _impl->applyState();
}

_GFXVulkanTargetDesc* GFXVulkanTextureTarget::getTargetDesc(RenderSlot slot) const
{
   // This can only be called by our implementations, and then will not actually store the pointer so this is (almost) safe
   return mTargets[slot].ptr();
}

void GFXVulkanTextureTarget::_onTextureEvent( GFXTexCallbackCode code )
{
   invalidateState();
}

const String GFXVulkanTextureTarget::describeSelf() const
{
   String ret = String::ToString("   Color0 Attachment: %i", mTargets[Color0].isValid() ? mTargets[Color0]->getHandle() : 0);
   ret += String::ToString("   Depth Attachment: %i", mTargets[DepthStencil].isValid() ? mTargets[DepthStencil]->getHandle() : 0);
   
   return ret;
}

void GFXVulkanTextureTarget::resolve()
{
}

void GFXVulkanTextureTarget::resolveTo(GFXTextureObject* obj)
{
   AssertFatal(dynamic_cast<GFXVulkanTextureObject*>(obj), "GFXVulkanTextureTarget::resolveTo - Incorrect type of texture, expected a GFXVulkanTextureObject");
   GFXVulkanTextureObject* vulkanTexture = static_cast<GFXVulkanTextureObject*>(obj);

   //if( GFXVulkan->mCapabilities.copyImage && mTargets[Color0]->isCompatible(vulkanTexture) )
   //{
   //   Vulkanenum binding = mTargets[Color0]->getBinding();      
   //   binding = (binding >= Vulkan_TEXTURE_CUBE_MAP_POSITIVE_X && binding <= Vulkan_TEXTURE_CUBE_MAP_NEGATIVE_Z) ? Vulkan_TEXTURE_CUBE_MAP : binding;
   //   U32 srcStartDepth = binding == Vulkan_TEXTURE_CUBE_MAP ? mTargets[Color0]->getBinding() - Vulkan_TEXTURE_CUBE_MAP_POSITIVE_X : 0;
   //   vulkanCopyImageSubData(
   //     mTargets[Color0]->getHandle(), binding, 0, 0, 0, srcStartDepth,
   //     vulkanTexture->getHandle(), vulkanTexture->getBinding(), 0, 0, 0, 0,
   //     mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight(), 1);

   //   return;
   //}

   //PRESERVE_FRAMEBUFFER();
   //
   //vulkanBindFramebuffer(Vulkan_DRAW_FRAMEBUFFER, mCopyFboDst);
   //vulkanFramebufferTexture2D(Vulkan_DRAW_FRAMEBUFFER, Vulkan_COLOR_ATTACHMENT0, vulkanTexture->getBinding(), vulkanTexture->getHandle(), 0);
   //
   //vulkanBindFramebuffer(Vulkan_READ_FRAMEBUFFER, mCopyFboSrc);
   //vulkanFramebufferTexture2D(Vulkan_READ_FRAMEBUFFER, Vulkan_COLOR_ATTACHMENT0, mTargets[Color0]->getBinding(), mTargets[Color0]->getHandle(), 0);
   //
   //vulkanBlitFramebuffer(0, 0, mTargets[Color0]->getWidth(), mTargets[Color0]->getHeight(),
   //   0, 0, vulkanTexture->getWidth(), vulkanTexture->getHeight(), Vulkan_COLOR_BUFFER_BIT, Vulkan_NEAREST);
}
