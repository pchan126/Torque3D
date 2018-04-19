#ifndef GFX_Vulkan_VERTEX_DECL
#define GFX_Vulkan_VERTEX_DECL

class GFXVertexFormat;
class GFXVulkanDevice;

class GFXVulkanVertexDecl : public GFXVertexDecl
{
public:
   GFXVulkanVertexDecl() : mFormat(NULL), mVertexAttribActiveMask(0) {}
   void init(const GFXVertexFormat *format);

   void prepareVertexFormat() const;
   //void prepareBuffer_old(U32 stream, Vulkanint mBuffer, Vulkanint mDivisor) const;
   void updateActiveVertexAttrib(U32 lastActiveMask) const;

   //struct VulkanVertexAttribData
   //{
   //   U32 stream;
	  // VkVertexInputBindingDescription bindingDescription = {};
	  // VkVertexInputAttributeDescription attributeDescription = {};
   //};

	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		Vector<VkVertexInputBindingDescription> bindingDescriptions;
		Vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;
	
	//VkVertexInputBindingDescription bindingDescription = {};
	//Vector<VkVertexInputAttributeDescription> attributeDescription = {};

	//Vector<VulkanVertexAttribData> VulkanVerticesFormat;
   
protected:
   friend class GFXVulkanDevice;
   const GFXVertexFormat *mFormat;
   //Vulkanuint mVertexSize[4];
   U32 mVertexAttribActiveMask;

   void _initVerticesFormat(U32 stream);
   void _initVerticesFormat2();
};

#endif //GFX_Vulkan_VERTEX_DECL