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

#ifndef _GFXVulkanCUBEMAP_H_
#define _GFXVulkanCUBEMAP_H_

#ifndef _GFXCUBEMAP_H_
#include "gfx/gfxCubemap.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif


class GFXVulkanCubemap : public GFXCubemap
{
public:
   GFXVulkanCubemap();
   virtual ~GFXVulkanCubemap();

   virtual void initStatic( GFXTexHandle *faces );
   virtual void initStatic( DDSFile *dds );
   virtual void initDynamic( U32 texSize, GFXFormat faceFormat = GFXFormatR8G8B8A8 );
   virtual U32 getSize() const { return size; }
   virtual GFXFormat getFormat() const { return mFaceFormat; }

   // Convenience methods for GFXVulkanTextureTarget
   U32 getWidth() { return size; }
   U32 getHeight() { return size; }
   U32 getHandle() { return mCubemap; }
   
   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   
   /// Called by texCB; this is to ensure that all textures have been resurrected before we attempt to res the cubemap.
   void tmResurrect();
   
   //static Vulkanenum getEnumForFaceNumber(U32 face) { return faceList[face]; } ///< Performs lookup to get a Vulkanenum for the given face number

protected:

   friend class GFXDevice;
   friend class GFXVulkanDevice;

   /// The callback used to get texture events.
   /// @see GFXTextureManager::addEventDelegate
   void _onTextureEvent( GFXTexCallbackCode code );
   
   VkImage mCubemap; ///< Internal Vulkan handle
   U32 mDynamicTexSize; ///< Size of faces for a dynamic texture (used in resurrect)
   
   // Self explanatory
   U32 size; // must be square

   VkImageUsageFlags usage;

   GFXFormat mFaceFormat;
      
   GFXTexHandle mTexture; ///< single 6 layer texture
   
   /// The backing DDSFile uses to restore the faces
   /// when the surface is lost.
   Resource<DDSFile> mDDSFile;

   // should only be called by GFXDevice
   virtual void setToTexUnit( U32 tuNum ); ///< Binds the cubemap to the given texture unit
   virtual void bind(U32 textureUnit) const; ///< Notifies our owning device that we want to be set to the given texture unit (used for Vulkan internal state tracking)
   void fillCubeTextures(GFXTexHandle faces); ///< Copies the textures in faces into the cubemap
   
   //static Vulkanenum faceList[6]; ///< Lookup table
};

#endif
