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

#ifndef _GFXVulkanSHADER_H_
#define _GFXVulkanSHADER_H_

#include "core/util/refBase.h"
#include "gfx/gfxShader.h"
#include <vulkan/vulkan.h>
#include "core/util/tSignal.h"
#include "core/util/tDictionary.h"
#include "gfx/vulkan/gfxVulkanUtils.h"
#include <array>

class GFXVulkanShaderConstHandle;
class FileStream;
class GFXVulkanShaderConstBuffer;

using namespace VulkanUtils;

class GFXVulkanShader : public GFXShader
{
   typedef Map<String, GFXVulkanShaderConstHandle*> HandleMap;
public:
   GFXVulkanShader();
   virtual ~GFXVulkanShader();
   
   /// @name GFXShader interface
   /// @{
   virtual GFXShaderConstHandle* getShaderConstHandle(const String& name);
   virtual GFXShaderConstHandle* findShaderConstHandle(const String& name);

   /// Returns our list of shader constants, the material can get this and just set the constants it knows about
   virtual const Vector<GFXShaderConstDesc>& getShaderConstDesc() const;

   /// Returns the alignment value for constType
   virtual U32 getAlignmentValue(const GFXShaderConstType constType) const; 

   virtual GFXShaderConstBufferRef allocConstBuffer();

   /// @}
   
   /// @name GFXResource interface
   /// @{
   virtual void zombify();
   virtual void resurrect() { reload(); }
   virtual const String describeSelf() const;
   /// @}      

   /// Activates this shader in the Vulkan context.
   void useProgram();
   void setupPipeline();

	void setVertexFormat(const GFXVertexFormat* format);
//	getGFXVertexFormat<T>()

	VkPipelineVertexInputStateCreateInfo const* getVertexInputInfo() const	{ return &vertexInputInfo; 	};

	U32 getShaderStageCount() const { return shaderStages.size(); };
	VkPipelineShaderStageCreateInfo const* getShaderData() { return shaderStages.data(); };

protected:

   friend class GFXVulkanShaderConstBuffer;
   friend class GFXVulkanShaderConstHandle;
   
	virtual bool _init();   

   bool initShader(  const Torque::Path &file, 
                     bool isVertex, 
                     const Vector<GFXShaderMacro> &macros );

   void clearShaders();
   void initConstantDescs();
   void initHandles();
   void setConstantsFromBuffer(GFXVulkanShaderConstBuffer* buffer);

   static bool _loadShaderFromStream(  VkShaderModule &shader, 
                                          FileStream *s);

   void AddPipelineShaderStage(ShaderStageParameters shader_stage);
   void SetPipelineShaderStages(std::vector<ShaderStageParameters> const shader_stages );

 //  std::vector<VkVertexInputBindingDescription> binding_descriptions_;
	//std::vector<VkVertexInputAttributeDescription> attribute_descriptions_;

	//void setPipelineVertexBindingDesc();
	//void setPipelineInputAssembState(GFXPrimitiveType primType, VkBool32 primitive_restart_enable);

	//void SpecPipelineTessellationState(U32 patch_control_points_count);
	//void SpecPipelineViewportAndScissor(ViewportInfo viewport_infos);
	//void SpecPipelineRasterizationState(VkBool32 depth_clamp_enable, VkBool32 rasterization_discard_enable,
	//                                    VkPolygonMode polygon_mode, VkCullModeFlags culling_mode, VkFrontFace front_face,
	//                                    VkBool32 depth_bias_enable, F32 depth_bias_constant_factor, F32 depth_bias_clamp,
	//                                    F32 depth_bias_slope_factor, F32 line_width);
	//void SpecPipelineMultisampleState(VkSampleCountFlagBits sample_count,
	//                                  VkBool32 per_sample_shading_enable,
	//                                  float min_sample_shading,
	//                                  const VkSampleMask* sample_masks,
	//                                  VkBool32 alpha_to_coverage_enable,
	//                                  VkBool32 alpha_to_one_enable);
	//void SpecPipelineDepthandStencilState(VkBool32 depthTestEnable,
	//                                      VkBool32 depthWriteEnable,
	//                                      VkCompareOp depthCompareOp,
	//                                      VkBool32 depthBoundsTestEnable,
	//                                      VkBool32 stencilTestEnable,
	//                                      VkStencilOpState front,
	//                                      VkStencilOpState back,
	//                                      float minDepthBounds,
	//                                      float maxDepthBounds);
	//void SpecPipelineBlendState(VkBool32 logic_op_enable,
 //                                            VkLogicOp logic_op,
 //                                            std::vector<VkPipelineColorBlendAttachmentState> attachment_blend_states,
 //                                            float blend_constants[4]);
	//void SpecPipelineDynamicStates(std::vector<VkDynamicState> dynamic_states);

	//void SpecPipelineCreationParams(VkPipelineCreateFlags& additional_options, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, U32 subpass, VkPipeline
	//                                basePipelineHandle = VK_NULL_HANDLE, S32 basePipelineIndex = -1);

	//bool CreatePipelineCacheObject();
	//bool GetDataFromPipelineCache(VkPipelineCache& in_pipeline_cache, std::vector<unsigned char>& out_pipeline_cache_data);

	//bool MergePipelineCacheObjects(VkPipelineCache& target_pipeline_cache, std::vector<VkPipelineCache>& source_pipeline_caches);


   /// @name Internal Vulkan handles
   /// @{
   VkShaderModule mVertexShader;
   VkShaderModule mPixelShader;
   /// @}
    
	// Load shaders
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	Vector<GFXShaderConstDesc> mConstants;
   U32 mConstBufferSize;
   U8* mConstBuffer;
   HandleMap mHandles;
   Vector<GFXVulkanShaderConstHandle*> mValidHandles;

	std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos;

	//VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info;
	//VkPipelineTessellationStateCreateInfo tessellation_state_create_info;
	//VkPipelineViewportStateCreateInfo viewport_state_create_info;
	//VkPipelineRasterizationStateCreateInfo rasterization_state_create_info;
	//VkPipelineMultisampleStateCreateInfo multisample_state_create_info;
	//VkPipelineDepthStencilStateCreateInfo depth_and_stencil_state_create_info;
	//VkPipelineColorBlendStateCreateInfo blend_state_create_info;
	//VkPipelineDynamicStateCreateInfo dynamic_state_create_info;

	// Pipeline cache object
	VkPipelineCache pipelineCache;

	VkPipeline pipeline;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo;

};

class GFXVulkanShaderConstBuffer : public GFXShaderConstBuffer
{
public:
   GFXVulkanShaderConstBuffer(GFXVulkanShader* shader, U32 bufSize, U8* existingConstants);
   ~GFXVulkanShaderConstBuffer();
   
   /// Called by GFXVulkanDevice to activate this buffer.
   void activate();

   /// Called when the shader this buffer references is reloaded.
   void onShaderReload( GFXVulkanShader *shader );

   // GFXShaderConstBuffer
   virtual GFXShader* getShader() { return mShader; }
   virtual void set(GFXShaderConstHandle* handle, const F32 fv);
   virtual void set(GFXShaderConstHandle* handle, const Point2F& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point3F& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point4F& fv);
   virtual void set(GFXShaderConstHandle* handle, const PlaneF& fv);
   virtual void set(GFXShaderConstHandle* handle, const LinearColorF& fv);   
   virtual void set(GFXShaderConstHandle* handle, const S32 f);
   virtual void set(GFXShaderConstHandle* handle, const Point2I& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point3I& fv);
   virtual void set(GFXShaderConstHandle* handle, const Point4I& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv);   
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv);
   virtual void set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv);
   virtual void set(GFXShaderConstHandle* handle, const MatrixF& mat, const GFXShaderConstType matType = GFXSCT_Float4x4);
   virtual void set(GFXShaderConstHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType = GFXSCT_Float4x4);   

   // GFXResource
   virtual const String describeSelf() const;
   virtual void zombify() {}
   virtual void resurrect() {}

private:

   friend class GFXVulkanShader;
   U8* mBuffer;
   WeakRefPtr<GFXVulkanShader> mShader;
   
   template<typename ConstType>
   void internalSet(GFXShaderConstHandle* handle, const ConstType& param);
   
   template<typename ConstType>
   void internalSet(GFXShaderConstHandle* handle, const AlignedArray<ConstType>& fv);
};

#endif // _GFXVulkanSHADER_H_