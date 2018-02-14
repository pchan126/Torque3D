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

#ifndef TORQUE_GFX_Vulkan_GFXVulkanUTILS_H_
#define TORQUE_GFX_Vulkan_GFXVulkanUTILS_H_

#include "core/util/preprocessorHelpers.h"
#include "gfx/vulkan/gfxVulkanEnumTranslate.h"
//#include "gfx/vulkan/gfxVulkanStateCache.h"
#include "gfx/bitmap/imageUtils.h"



//inline U32 getMaxMipmaps(U32 width, U32 height, U32 depth)
//{
//   return getMax( getBinLog2(depth), getMax(getBinLog2(width), getBinLog2(height)));
//}
//
//inline Vulkanenum minificationFilter(U32 minFilter, U32 mipFilter, U32 /*mipLevels*/)
//{
//   // the compiler should interpret this as array lookups
//   switch( minFilter ) 
//   {
//      case GFXTextureFilterLinear:
//         switch( mipFilter ) 
//         {
//         case GFXTextureFilterLinear:
//            return Vulkan_LINEAR_MIPMAP_LINEAR;
//         case GFXTextureFilterPoint:
//            return Vulkan_LINEAR_MIPMAP_NEAREST;
//         default: 
//            return Vulkan_LINEAR;
//         }
//      default:
//         switch( mipFilter ) {
//      case GFXTextureFilterLinear:
//         return Vulkan_NEAREST_MIPMAP_LINEAR;
//      case GFXTextureFilterPoint:
//         return Vulkan_NEAREST_MIPMAP_NEAREST;
//      default:
//         return Vulkan_NEAREST;
//         }
//   }
//}
//
////Get the surface size of a compressed mip map level - see ddsLoader.cpp
//inline U32 getCompressedSurfaceSize(GFXFormat format,U32 width, U32 height, U32 mipLevel=0 )
//{
//   if(!ImageUtil::isCompressedFormat(format))
//      return 0;
//
//   // Bump by the mip level.
//   height = getMax(U32(1), height >> mipLevel);
//   width = getMax(U32(1), width >> mipLevel);
//
//   U32 sizeMultiple = 0;
//   if(format == GFXFormatBC1 || format == GFXFormatBC1_SRGB)
//      sizeMultiple = 8;
//   else
//      sizeMultiple = 16;
//
//   return getMax(U32(1), width/4) * getMax(U32(1), height/4) * sizeMultiple;
//}
//
///// Simple class which preserves a given Vulkan integer.
///// This class determines the integer to preserve on construction and restores 
///// it on destruction.
//class GFXVulkanPreserveInteger
//{
//public:
//   typedef void(STDCALL *BindFn)(Vulkanenum, Vulkanuint);
//
//   /// Preserve the integer.
//   /// @param binding The binding which should be set on destruction.
//   /// @param getBinding The parameter to be passed to VulkanGetIntegerv to determine
//   /// the integer to be preserved.
//   /// @param binder The Vulkan function to call to restore the integer.
//   GFXVulkanPreserveInteger(Vulkanenum binding, Vulkanint getBinding, BindFn binder) :
//      mBinding(binding), mPreserved(0), mBinder(binder)
//   {
//      AssertFatal(mBinder, "GFXVulkanPreserveInteger - Need a valid binder function");
//      mPreserved = GFXVulkan->getOpenVulkanCache()->getCacheBinded(mBinding);
//#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
//      Vulkanint bindedOnOpenVulkanDriver;
//      VulkanGetIntegerv(getBinding, &bindedOnOpenVulkanDriver);
//      AssertFatal( mPreserved == bindedOnOpenVulkanDriver, "GFXVulkanPreserveInteger - GFXVulkanDevice/OpenVulkan mismatch on cache binded resource.");
//#endif
//   }
//   
//   /// Restores the integer.
//   ~GFXVulkanPreserveInteger()
//   {
//      mBinder(mBinding, mPreserved);
//   }
//
//private:
//   Vulkanenum mBinding;
//   Vulkanint mPreserved;
//   BindFn mBinder;
//};
//
//class GFXVulkanPreserveTexture
//{
//public:
//   typedef void(STDCALL *BindFn)(Vulkanenum, Vulkanuint);
//   
//   GFXVulkanPreserveTexture(Vulkanenum binding, Vulkanint getBinding, BindFn binder) :
//      mBinding(binding), mPreserved(0), mBinder(binder)
//   {
//      AssertFatal(mBinder, "GFXVulkanPreserveTexture - Need a valid binder function");
//      GFXVulkanDevice *gfx = GFXVulkan;
//      mPreserved = gfx->getOpenVulkanCache()->getCacheBinded(mBinding);
//      mActiveTexture = gfx->getOpenVulkanCache()->getCacheActiveTexture();
//#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
//      Vulkanint activeTextureOnOpenVulkanDriver, bindedTextureOnOpenVulkanDriver;
//      VulkanGetIntegerv(getBinding, &bindedTextureOnOpenVulkanDriver);
//      VulkanGetIntegerv(Vulkan_ACTIVE_TEXTURE, &activeTextureOnOpenVulkanDriver);
//      activeTextureOnOpenVulkanDriver -= Vulkan_TEXTURE0;
//      AssertFatal( mPreserved == bindedTextureOnOpenVulkanDriver, "GFXVulkanPreserveTexture - GFXVulkanDevice/OpenVulkan mismatch on cache binded resource.");
//      AssertFatal( activeTextureOnOpenVulkanDriver == mActiveTexture, "GFXVulkanPreserveTexture - GFXVulkanDevice/OpenVulkan mismatch on cache binded resource.");
//#endif
//   }
//   
//   /// Restores the texture.
//   ~GFXVulkanPreserveTexture()
//   {
//#if defined(TORQUE_DEBUG) && defined(TORQUE_DEBUG_GFX)
//      Vulkanint activeTextureOnOpenVulkanDriver;
//      VulkanGetIntegerv(Vulkan_ACTIVE_TEXTURE, &activeTextureOnOpenVulkanDriver);
//      activeTextureOnOpenVulkanDriver -= Vulkan_TEXTURE0;
//      Vulkanint cacheActiveTexture = GFXVulkan->getOpenVulkanCache()->getCacheActiveTexture();
//      AssertFatal( cacheActiveTexture == activeTextureOnOpenVulkanDriver, "GFXVulkanPreserveTexture - GFXVulkanDevice/OpenVulkan mismatch on cache ActiveTexture.");
//#endif
//      mBinder(mBinding, mPreserved);
//   }
//
//private:
//   Vulkanenum mBinding;
//   Vulkanint mPreserved;
//   BindFn mBinder;
//   S16 mActiveTexture;
//};
//
///// Helper macro to preserve the current VBO binding.
//#define PRESERVE_VERTEX_BUFFER() \
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_ARRAY_BUFFER, Vulkan_ARRAY_BUFFER_BINDING, (GFXVulkanPreserveInteger::BindFn)VulkanBindBuffer)
//
///// Helper macro to preserve the current element array binding.
//#define PRESERVE_INDEX_BUFFER() \
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_ELEMENT_ARRAY_BUFFER, Vulkan_ELEMENT_ARRAY_BUFFER_BINDING, (GFXVulkanPreserveInteger::BindFn)VulkanBindBuffer)
//
//#define _GET_BUFFER_BINDING( BINDING ) \
//BINDING == Vulkan_ARRAY_BUFFER ? Vulkan_ARRAY_BUFFER_BINDING : ( BINDING == Vulkan_ELEMENT_ARRAY_BUFFER ?  Vulkan_ELEMENT_ARRAY_BUFFER_BINDING : 0 )
//
///// Helper macro to preserve the current element array binding.
//#define PRESERVE_BUFFER( BINDING ) \
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (BINDING, _GET_BUFFER_BINDING(BINDING), (GFXVulkanPreserveInteger::BindFn)VulkanBindBuffer)
//
///// ASSERT: Never call VulkanActiveTexture for a "bind to modify" or in a PRESERVER_TEXTURE MACRO scope.
//
///// Helper macro to preserve the current 1D texture binding.
//#define PRESERVE_1D_TEXTURE() \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_TEXTURE_1D, Vulkan_TEXTURE_BINDING_1D, (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
///// Helper macro to preserve the current 2D texture binding.
//#define PRESERVE_2D_TEXTURE() \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_TEXTURE_2D, Vulkan_TEXTURE_BINDING_2D, (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
///// Helper macro to preserve the current 3D texture binding.
//#define PRESERVE_3D_TEXTURE() \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_TEXTURE_3D, Vulkan_TEXTURE_BINDING_3D, (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
///// Helper macro to preserve the current 3D texture binding.
//#define PRESERVE_CUBEMAP_TEXTURE() \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_TEXTURE_CUBE_MAP, Vulkan_TEXTURE_BINDING_CUBE_MAP, (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
//#define _GET_TEXTURE_BINDING(binding) \
//binding == Vulkan_TEXTURE_2D ? Vulkan_TEXTURE_BINDING_2D : (binding == Vulkan_TEXTURE_3D ?  Vulkan_TEXTURE_BINDING_3D : Vulkan_TEXTURE_BINDING_1D )
//
//#define PRESERVE_TEXTURE(binding) \
//GFXVulkanPreserveTexture TORQUE_CONCAT(preserve_, __LINE__) (binding, _GET_TEXTURE_BINDING(binding), (GFXVulkanPreserveInteger::BindFn)VulkanBindTexture)
//
//#define PRESERVE_FRAMEBUFFER() \
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve_, __LINE__) (Vulkan_READ_FRAMEBUFFER, Vulkan_READ_FRAMEBUFFER_BINDING, (GFXVulkanPreserveInteger::BindFn)VulkanBindFramebuffer);\
//GFXVulkanPreserveInteger TORQUE_CONCAT(preserve2_, __LINE__) (Vulkan_DRAW_FRAMEBUFFER, Vulkan_DRAW_FRAMEBUFFER_BINDING, (GFXVulkanPreserveInteger::BindFn)VulkanBindFramebuffer)
//
//
//#if TORQUE_DEBUG
//
//    // Handy macro for checking the status of a framebuffer.  Framebuffers can fail in 
//    // all sorts of interesting ways, these are just the most common.  Further, no existing Vulkan profiling 
//    // tool catches framebuffer errors when the framebuffer is created, so we actually need this.
//    #define CHECK_FRAMEBUFFER_STATUS()\
//    {\
//    Vulkanenum status;\
//    status = VulkanCheckFramebufferStatus(Vulkan_FRAMEBUFFER);\
//    switch(status) {\
//    case Vulkan_FRAMEBUFFER_COMPLETE:\
//    break;\
//    case Vulkan_FRAMEBUFFER_UNSUPPORTED:\
//    AssertFatal(false, "Unsupported FBO");\
//    break;\
//    case Vulkan_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:\
//    AssertFatal(false, "Incomplete FBO Attachment");\
//    break;\
//    case Vulkan_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:\
//    AssertFatal(false, "Incomplete FBO Missing Attachment");\
//    break;\
//    case Vulkan_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:\
//    AssertFatal(false, "Incomplete FBO Draw buffer");\
//    break;\
//    case Vulkan_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:\
//    AssertFatal(false, "Incomplete FBO Read buffer");\
//    break;\
//    default:\
//    /* programming error; will fail on all hardware */\
//    AssertFatal(false, "Something really bad happened with an FBO");\
//    }\
//    }
//#else
//    #define CHECK_FRAMEBUFFER_STATUS()
//#endif //TORQUE_DEBUG
//
#endif
