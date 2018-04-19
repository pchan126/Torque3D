#ifndef GFX_Vulkan_STATE_CACHE
#define GFX_Vulkan_STATE_CACHE


/// GFXVulkanStateCache store OpenVulkan state to avoid performance penalities of VulkanGet* calls
/// Vulkan_TEXTURE_1D/2D/3D, Vulkan_FRAMEBUFFER, Vulkan_ARRAY_BUFFER, Vulkan_ELEMENT_ARRAY_BUFFER
class GFXVulkanStateCache
{
public:
   GFXVulkanStateCache()
   {      
      //mActiveTexture = 0;      
      //mBindedVBO = 0;
      //mBindedIBO = 0;
      //mBindedFBO_W = 0;
      //mBindedFBO_R = 0;
      mVertexAttribActive = 0;
   }

   class TextureUnit
   {
   public:
      TextureUnit() //:  mTexture1D(0), mTexture2D(0), mTexture3D(0), mTextureCube(0)
      {

      }
      //Vulkanuint mTexture1D, mTexture2D, mTexture3D, mTextureCube;
   };

   ///// after VulkanBindTexture
   //void setCacheBindedTex(U32 texUnit, Vulkanenum biding, Vulkanuint handle)
   //{ 
   //   mActiveTexture = texUnit;
   //   switch (biding)
   //   {
   //   case Vulkan_TEXTURE_2D:
   //      mTextureUnits[mActiveTexture].mTexture2D = handle;
   //      break;
   //   case Vulkan_TEXTURE_3D:
   //      mTextureUnits[mActiveTexture].mTexture3D = handle;
   //      break;
   //   case Vulkan_TEXTURE_1D:
   //      mTextureUnits[mActiveTexture].mTexture1D = handle;
   //      break;
   //   case Vulkan_TEXTURE_CUBE_MAP:
   //      mTextureUnits[mActiveTexture].mTextureCube = handle;
   //      break;
   //   default:
   //      AssertFatal(0, avar("GFXVulkanStateCache::setCacheBindedTex - binding (%x) not supported.", biding) );
   //      return;
   //   }
   //}

   ///// after openVulkan object binded
   //void setCacheBinded(Vulkanenum biding, Vulkanuint handle) 
   //{ 
   //   switch (biding)
   //   {
   //   case Vulkan_TEXTURE_2D:
   //      mTextureUnits[mActiveTexture].mTexture2D = handle;
   //      break;
   //   case Vulkan_TEXTURE_3D:
   //      mTextureUnits[mActiveTexture].mTexture3D = handle;
   //      break;
   //   case Vulkan_TEXTURE_1D:
   //      mTextureUnits[mActiveTexture].mTexture1D = handle;
   //      break;
   //   case Vulkan_TEXTURE_CUBE_MAP:
   //      mTextureUnits[mActiveTexture].mTextureCube = handle;
   //      break;
   //   case Vulkan_FRAMEBUFFER:
   //      mBindedFBO_W = mBindedFBO_R = handle;
   //      break;
   //   case Vulkan_DRAW_FRAMEBUFFER:
   //      mBindedFBO_W = handle;
   //      break;
   //   case Vulkan_READ_FRAMEBUFFER:
   //      mBindedFBO_R = handle;
   //      break;
   //   case Vulkan_ARRAY_BUFFER:
   //      mBindedVBO = handle;
   //      break;
   //   case Vulkan_ELEMENT_ARRAY_BUFFER:
   //      mBindedIBO = handle;
   //      break;
   //   default:
   //      AssertFatal(0, avar("GFXVulkanStateCache::setCacheBinded - binding (%x) not supported.", biding) );
   //      break;
   //   }
   //}

   //Vulkanuint getCacheBinded(Vulkanenum biding) const
   //{
   //   switch (biding)
   //   {
   //   case Vulkan_TEXTURE_2D:
   //      return mTextureUnits[mActiveTexture].mTexture2D;
   //   case Vulkan_TEXTURE_3D:
   //      return mTextureUnits[mActiveTexture].mTexture3D;
   //   case Vulkan_TEXTURE_1D:
   //      return mTextureUnits[mActiveTexture].mTexture1D;
   //   case Vulkan_TEXTURE_CUBE_MAP:
   //      return mTextureUnits[mActiveTexture].mTextureCube;
   //   case Vulkan_DRAW_FRAMEBUFFER:
   //      return mBindedFBO_W;
   //   case Vulkan_READ_FRAMEBUFFER:
   //      return mBindedFBO_R;
   //   case Vulkan_ARRAY_BUFFER:
   //      return mBindedVBO;
   //   case Vulkan_ELEMENT_ARRAY_BUFFER:
   //      return mBindedIBO;
   //   default:
   //      AssertFatal(0, avar("GFXVulkanStateCache::getCacheBinded - binding (%x) not supported.", biding) );
   //      return 0;
   //   }
   //}

   /// after VulkanActiveTexture
   //void setCacheActiveTexture(U32 unit) { mActiveTexture = unit; }
   //U32 getCacheActiveTexture() const { return mActiveTexture;  }

   /// for cache VulkanEnableVertexAttribArray / VulkanDisableVertexAttribArray
   void setCacheVertexAttribActive(U32 activeMask) { mVertexAttribActive = activeMask; }
   U32 getCacheVertexAttribActive() const { return mVertexAttribActive;  }

protected:   
   //Vulkanuint mActiveTexture, mBindedVBO, mBindedIBO, mBindedFBO_W, mBindedFBO_R;
   TextureUnit mTextureUnits[TEXTURE_STAGE_COUNT];
   U32 mVertexAttribActive;
};


#endif