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

#ifndef _GFXVulkanDevice_H_
#define _GFXVulkanDevice_H_

#include "platform/platform.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxInit.h"

#include <vulkan/vulkan.h>

#include "windowManager/platformWindow.h"
#include "gfx/gfxFence.h"
#include "gfx/gfxResource.h"
#include "gfx/vulkan/gfxVulkanStateBlock.h"
#include "gfx/vulkan/gfxVulkanUtils.h"
#include "gfx/vulkan/gfxVulkanRenderPass.h"
#include "vk_mem_alloc.h"
#include "gfxPipelineManager.h"

class GFXVulkanWindowTarget;
//class GFXVulkanVertexBuffer;
//class GFXVulkanPrimitiveBuffer;
class GFXVulkanTextureTarget;
class GFXVulkanCubemap;
//class GFXVulkanStateCache;
//class GFXVulkanVertexDecl;

class GFXVulkanDevice : public GFXDevice
{
public:
   //struct VulkanCapabilities
   //{
   //   bool anisotropicFiltering;
   //   bool bufferStorage;
   //   bool shaderModel5;
   //   bool textureStorage;
   //   bool samplerObjects;
   //   bool copyImage;
   //   bool vertexAttributeBinding;
   //};
   //VulkanCapabilities mCapabilities;

   void zombify();
   void resurrect();
   GFXVulkanDevice(U32 adapterIndex);
   virtual ~GFXVulkanDevice();

   static void enumerateAdapters( Vector<GFXAdapter*> &adapterList );
   static GFXDevice *createInstance( U32 adapterIndex );

	void init( const GFXVideoMode &mode, PlatformWindow *window = NULL ) override;

   virtual void activate() { }
   virtual void deactivate() { }
   virtual GFXAdapterType getAdapterType() { return Vulkan; }

   virtual void enterDebugEvent(ColorI color, const char *name);
   virtual void leaveDebugEvent();
   virtual void setDebugMarker(ColorI color, const char *name);

   virtual void enumerateVideoModes();
   static void _EnumerateVideoModes(Vector<GFXVideoMode>& outModes);

   //virtual U32 getTotalVideoMemory_Vulkan_EXT();
   virtual U32 getTotalVideoMemory();

   virtual GFXCubemap * createCubemap();

   virtual F32 getFillConventionOffset() const { return 0.0f; }


   ///@}

   /// @name Render Target functions
   /// @{

   ///
   virtual GFXTextureTarget *allocRenderToTextureTarget();
	GFXWindowTarget *allocWindowTarget(PlatformWindow *window) override;
   virtual void _updateRenderTargets();

   ///@}

   /// @name Shader functions
   /// @{
   virtual F32 getPixelShaderVersion() const { return mPixelShaderVersion; }
   virtual void  setPixelShaderVersion( F32 version ) { mPixelShaderVersion = version; }
   
   virtual void setShader(GFXShader *shader, bool force = false);
   
   /// @attention Vulkan cannot check if the given format supports blending or filtering!
   virtual GFXFormat selectSupportedFormat(GFXTextureProfile *profile,
	   const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter);
      
   /// Returns the number of texture samplers that can be used in a shader rendering pass
   virtual U32 getNumSamplers() const;

   /// Returns the number of simultaneous render targets supported by the device.
   virtual U32 getNumRenderTargets() const;

   virtual GFXShader* createShader();
      
   virtual void clear( U32 flags, const LinearColorF& color, F32 z, U32 stencil );
   virtual bool beginSceneInternal();
   virtual void endSceneInternal();

   virtual void drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount );

   virtual void drawIndexedPrimitive(  GFXPrimitiveType primType, 
                                       U32 startVertex, 
                                       U32 minIndex, 
                                       U32 numVerts, 
                                       U32 startIndex, 
                                       U32 primitiveCount );

   virtual void setClipRect( const RectI &rect );
   virtual const RectI &getClipRect() const { return mClip; }

   virtual void preDestroy() { Parent::preDestroy(); }

   virtual U32 getMaxDynamicVerts() { return MAX_DYNAMIC_VERTS; }
   virtual U32 getMaxDynamicIndices() { return MAX_DYNAMIC_INDICES; }
   

   // Fences
   std::vector<VkFence> mFences;
	bool createFence(VkFence &fence, bool signaled = false );
	virtual void createFences(U32 mNumFences) override;
	virtual void waitForFences() override;
	bool resetFences();
	void destroyFence(VkFence &fence);

   GFXOcclusionQuery* createOcclusionQuery();

   GFXVulkanStateBlockRef getCurrentStateBlock() { return mCurrentVulkanStateBlock; }
   
   virtual void setupGenericShaders( GenericShaderType type = GSColor );
   
   ///
   bool supportsAnisotropic() const { return mSupportsAnisotropic; }

   //GFXVulkanStateCache* getOpenglCache() { return mOpenglStateCache; }

   GFXTextureObject* getDefaultDepthTex() const;
   GFXVulkanWindowTarget* getWindowTarget() const;
	VkRenderPass getRenderPass() const;



   /// Returns the number of vertex streams supported by the device.	
   const U32 getNumVertexStreams() const { return mNumVertexStream; }

   //bool glUseMap() const { return mUseGlMap; }   

	VkInstance	getInstance() const { return instance; }
	VkDevice	getLogicalDevice() const { return logical_device; }
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

	bool createCommandPool( U32 queue_family, VkCommandPool &command_pool, VkCommandPoolCreateFlags parameters = 0 );

	bool allocateCommandBuffer(VkCommandPool& command_pool, VkCommandBufferLevel level, U32 command_buffer_count,
	                           std::vector<VkCommandBuffer>& command_buffers);

	bool beginCommandBuffer(VkCommandBuffer &command_buffer, VkCommandBufferUsageFlags usage);
	bool endCommandBuffer(VkCommandBuffer &command_buffer);
	bool resetCommandBuffer(VkCommandBuffer& command_buffer, bool release);
	bool resetCommandPool(VkCommandPool& command_pool, bool release);

	bool sendCommandBuffersToQueue( VkQueue queue, 
		std::vector<VkSemaphore> wait_semaphore_handles, 
		std::vector<VkPipelineStageFlags> wait_semaphore_stages,
		std::vector<VkCommandBuffer> command_buffers,
		std::vector<VkSemaphore> signal_semaphores,
		VkFence fence = VK_NULL_HANDLE);

	void freeCommandBuffers( VkCommandPool &command_pool, std::vector<VkCommandBuffer> command_buffers );
	void destroyCommandPool( VkCommandPool &command_pool );

	bool createSemaphore( VkSemaphore &semaphore);
	void destroySemaphore( VkSemaphore &semaphore );

	bool createBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer &buffer);
	bool allocateAndBindMemoryObjectForBuffer(VkBuffer& buffer, VkDeviceMemory& memory_object,
	                                          VkMemoryPropertyFlagBits memory_property) const;

	void setBufferMemoryBarrier(std::vector<VkBuffer>& buffers,
	                            std::vector<VkAccessFlags>& current_access,
	                            std::vector<VkAccessFlags>& newaccess,
	                            std::vector<U32>& currentQueueFamily,
	                            std::vector<U32>& NewQueueFamily, VkCommandBuffer& command_buffer,
	                            VkPipelineStageFlags generating_stages, VkPipelineStageFlags consuming_stages);
	
	bool createBufferView(VkBuffer& buffer, VkFormat format, VkDeviceSize memory_offset, VkDeviceSize memory_range,
	                      VkBufferView& buffer_view);

	bool createImage(VkImageType imageType, 
								VkFormat format, 
								VkExtent3D size, 
								U32 num_mipmaps, 
								U32 num_layers, 
								VkSampleCountFlagBits samples, 
								VkImageUsageFlags usage_scenarios,
								bool isCubemap,
								VkImage &image);
	void destroyImage( VkImage &image );

	bool allocateAndBindMemoryObjectToImage(VkImage image, VkMemoryPropertyFlagBits memory_properties,
	                                        VkDeviceMemory& memory_object);

	void setImageMemoryBarrier(std::vector<VkImage>& Image,
	                           std::vector<VkAccessFlags>& CurrentAccess,
	                           std::vector<VkAccessFlags>& NewAccess,
	                           std::vector<VkImageLayout>& CurrentLayout,
	                           std::vector<VkImageLayout>& NewLayout,
	                           std::vector<U32>& CurrentQueueFamily,
	                           std::vector<U32>& NewQueueFamily,
	                           std::vector<VkImageAspectFlags> Aspect,
	                           VkCommandBuffer& command_buffer,
	                           VkPipelineStageFlags generating_stages, VkPipelineStageFlags consuming_stages);

	void freeMemoryObject( VkDeviceMemory &memory_object);

	bool createImageView(VkImage image, VkImageViewType viewtype, VkFormat format, VkImageAspectFlags aspectmask,
	                     VkImageView& imageView);
	void destroyImageView( VkImageView& image_view );

	bool mapHostVisibleMemory(VkDeviceMemory memory_object, VkDeviceSize offset, VkDeviceSize data_size,
	                          void* local_pointer, bool unmap);
	void copyDataBetweenBuffers(VkCommandBuffer command_buffer, VkBuffer source_buffer, VkBuffer destination_buffer,
	                            std::vector<VkBufferCopy> regions);
	void copyDataFromBufferToImage(VkCommandBuffer command_buffer, VkBuffer source_buffer, VkImage destination_image,
	                               VkImageLayout image_layout, std::vector<VkBufferImageCopy> regions);
	void copyDataFromImageToBuffer(VkCommandBuffer command_buffer, VkImage source_image, VkImageLayout image_layout,
	                               VkBuffer destination_buffer, std::vector<VkBufferImageCopy> regions);

	//U32 findMemoryType(U32 typeFilter)
	//VkSwapchainKHR getSwapChain() { return mSwapChain; }
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	
	protected:   
   /// Called by GFXDevice to create a device specific stateblock
   virtual GFXStateBlockRef createStateBlockInternal(const GFXStateBlockDesc& desc);
   /// Called by GFXDevice to actually set a stateblock.
   virtual void setStateBlockInternal(GFXStateBlock* block, bool force);   

   /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer);

   virtual void setTextureInternal(U32 textureUnit, const GFXTextureObject*texture);
   virtual void setCubemapInternal(U32 cubemap, const GFXVulkanCubemap* texture);

   virtual void setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable);
   virtual void setLightMaterialInternal(const GFXLightMaterial mat);
   virtual void setGlobalAmbientInternal(LinearColorF color);

   /// @name State Initalization.
   /// @{

   /// State initalization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
   virtual void initStates() { }

   virtual void setMatrix( GFXMatrixType mtype, const MatrixF &mat );

   virtual GFXVertexBuffer *allocVertexBuffer(  U32 numVerts, 
                                                const GFXVertexFormat *vertexFormat,
                                                U32 vertSize, 
                                                GFXBufferType bufferType,
                                                void* data = nullptr);

   virtual GFXPrimitiveBuffer* allocPrimitiveBuffer(U32 numIndices, U32 numPrimitives, GFXBufferType bufferType,
                                                    void* data = nullptr);
   
   // NOTE: The Vulkan device doesn't need a vertex declaration at
   // this time, but we need to return something to keep the system
   // from retrying to allocate one on every call.
   virtual GFXVertexDecl* allocVertexDecl( const GFXVertexFormat *vertexFormat );

   virtual void setVertexDecl( const GFXVertexDecl *decl );

   virtual void setVertexStream( U32 stream, GFXVertexBuffer *buffer );
   virtual void setVertexStreamFrequency( U32 stream, U32 frequency );

	void assignQueue(VkSurfaceKHR surface);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	
public: 
	// 
	GFXRenderPassImpl* makeRenderPassImpl(RenderPassManager* main) override;
	GFXRenderBinImpl* makeRenderBinImpl(RenderBinManager* main) override;

private:
   typedef GFXDevice Parent;
   
   friend class GFXVulkanTextureObject;
   friend class GFXVulkanCubemap;
   friend class GFXVulkanWindowTarget;
   //friend class GFXVulkanPrimitiveBuffer;
   friend class GFXVulkanVertexBuffer;

   static GFXAdapter::CreateDeviceInstanceDelegate mCreateDeviceInstance; 

   U32 mAdapterIndex;
   
   StrongRefPtr<GFXVulkanVertexBuffer> mCurrentVB[VERTEX_STREAM_COUNT];
   U32 mCurrentVB_Divisor[VERTEX_STREAM_COUNT];
   bool mNeedUpdateVertexAttrib;
   //StrongRefPtr<GFXVulkanPrimitiveBuffer> mCurrentPB;
   U32 mDrawInstancesCount;
   
   GFXShader* mCurrentShader;
   GFXShaderRef mGenericShader[GS_COUNT];
   GFXShaderConstBufferRef mGenericShaderBuffer[GS_COUNT];
   GFXShaderConstHandle *mModelViewProjSC[GS_COUNT];
   
   /// Since Vulkan does not have separate world and view matrices we need to track them
   MatrixF m_mCurrentWorld;
   MatrixF m_mCurrentView;

   void* mContext;
   void* mPixelFormat;


	std::vector<VkExtensionProperties> extensions;

	VkInstance instance;
	VkDevice logical_device;
	VmaAllocator allocator;
	VkQueue queue;

	VkPhysicalDevice physicalDevice;

	std::vector<char const*> desired_extensions;
	std::vector<VkQueueFamilyProperties> queueFamilies;

	U32 graphicsQueueFamily;
	U32 presentQueueFamily;
	VkCommandPool commandPool;
	// Command buffers used for rendering
	std::vector<VkCommandBuffer> drawCmdBuffers;

	struct 
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	} depthStencil;

	struct {
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
		// UI overlay submission and execution
		VkSemaphore overlayComplete;
	} semaphores;
	
	F32 mPixelShaderVersion;
   
   bool mSupportsAnisotropic;   

   U32 mNumVertexStream;
   
   U32 mMaxShaderTextures;
   U32 mMaxFFTextures;

   U32 mMaxTRColors;

   RectI mClip;
   
   GFXVulkanStateBlockRef mCurrentVulkanStateBlock;
   
   std::vector<StrongRefPtr<GFXVulkanVertexBuffer>> mVolatileVBs;
	///< Pool of existing volatile VBs so we can reuse previously created ones
   //Vector< StrongRefPtr<GFXVulkanPrimitiveBuffer> > mVolatilePBs; ///< Pool of existing volatile PBs so we can reuse previously created ones

   U32 primCountToIndexCount(GFXPrimitiveType primType, U32 primitiveCount);
   void preDrawPrimitive();
   void postDrawPrimitive(U32 primitiveCount);  
   
   GFXVertexBuffer* findVolatileVBO(U32 numVerts, const GFXVertexFormat* vertexFormat, U32 vertSize);
	///< Returns an existing volatile VB which has >= numVerts and the same vert flags/size, or creates a new VB if necessary

   GFXPrimitiveBuffer* findVolatilePBO(U32 numIndices, U32 numPrimitives);
	///< Returns an existing volatile PB which has >= numIndices, or creates a new PB if necessary

   void vsyncCallback(); ///< Vsync callback
   
   void initVulkanState(); ///< Guaranteed to be called after all extensions have been loaded, use to init card profiler, shader version, max samplers, etc.
   
   GFXFence* _createPlatformSpecificFence();
	///< If our platform (e.g. OS X) supports a fence extenstion (e.g. Vulkan_APPLE_fence) this will create one, otherwise returns NULL
   
   //void setPB(GFXVulkanPrimitiveBuffer* pb); ///< Sets mCurrentPB

   //GFXVulkanStateCache *mOpenglStateCache;

   GFXWindowTargetRef *mWindowRT;

	bool IsExtensionSupported( std::vector<char const *> desired_extensions);

	VkDescriptorPool descriptorPool;
	bool setupDescriptorPool(std::vector<VkDescriptorPoolSize> poolSizes, bool free_individual_sets, U32 max_sets_count);


	VkDescriptorSet descriptorSet;

	VkDescriptorSetLayout descriptorSetLayout;
	bool CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> bindings);

	VkPipelineLayout pipelineLayout;
	bool CreatePipelineLayout(std::vector<VkDescriptorSetLayoutBinding> bindings);

	friend GFXPipelineObject;
protected:
   GFXPipelineManager *mPipelineManager;

public:

   inline GFXPipelineManager *getPipelineManager()
   {
      return mPipelineManager;
   }
};

#define GFXVulkan (dynamic_cast<GFXVulkanDevice*>(GFXDevice::get()))
#endif
