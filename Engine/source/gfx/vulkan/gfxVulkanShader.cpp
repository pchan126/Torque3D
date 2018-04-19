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

#include "platform/platform.h"
#include "gfx/Vulkan/gfxVulkanShader.h"
//#include "gfx/Vulkan/gfxVulkanVertexAttribLocation.h"
#include "gfx/Vulkan/gfxVulkanDevice.h"
#include "gfx/Vulkan/gfxVulkanEnumTranslate.h"
#include "gfx/Vulkan/gfxVulkanWindowTarget.h"

#include "core/frameAllocator.h"
#include "core/stream/fileStream.h"
#include "core/strings/stringFunctions.h"
#include "math/mPoint2.h"
#include "gfx/gfxStructs.h"
#include "console/console.h"
#include <fstream>
#include <map>
#include "gfxVulkanVertexDecl.h"

//#define CHECK_AARG(pos, name) static StringTableEntry attr_##name = StringTable->insert(#name); if (argName == attr_##name) { VulkanBindAttribLocation(mProgram, pos, attr_##name); continue; }


class GFXVulkanShaderConstHandle : public GFXShaderConstHandle
{
   friend class GFXVulkanShader;

public:  
   
   GFXVulkanShaderConstHandle( GFXVulkanShader *shader );
   GFXVulkanShaderConstHandle( GFXVulkanShader *shader, const GFXShaderConstDesc &desc, U32 bindNum, S32 samplerNum );
   virtual ~GFXVulkanShaderConstHandle();
   
   void reinit(const GFXShaderConstDesc &desc, U32 bindNum, S32 samplerNum);

   const String& getName() const { return mDesc.name; }
   GFXShaderConstType getType() const { return mDesc.constType; }
   U32 getArraySize() const { return mDesc.arraySize; }

   U32 getSize() const;
   void setValid( bool valid ) { mValid = valid; }   
   /// @warning This will always return the value assigned when the shader was
   /// initialized.  If the value is later changed this method won't reflect that.
   S32 getSamplerRegister() const { return mSamplerNum; }

   GFXShaderConstDesc mDesc;
   GFXVulkanShader* mShader;
   //Vulkanuint mLocation;
   U32 mOffset;
   U32 mSize;  
   S32 mSamplerNum; 
   U32 mBindNum;
   bool mInstancingConstant;
};

GFXVulkanShaderConstHandle::GFXVulkanShaderConstHandle( GFXVulkanShader *shader )
 : mShader( shader ), mOffset(0), mSize(0), mSamplerNum(-1), mBindNum(0), mInstancingConstant(false)
{
   mValid = false;
}


static U32 shaderConstTypeSize(GFXShaderConstType type)
{
   switch(type) 
   {
   case GFXSCT_Float:
   case GFXSCT_Int:
   case GFXSCT_Sampler:
   case GFXSCT_SamplerCube:
      return 4;
   case GFXSCT_Float2:
   case GFXSCT_Int2:
      return 8;
   case GFXSCT_Float3:
   case GFXSCT_Int3:
      return 12;
   case GFXSCT_Float4:
   case GFXSCT_Int4:
      return 16;
   case GFXSCT_Float2x2:
      return 16;
   case GFXSCT_Float3x3:
      return 36;
   case GFXSCT_Float4x3:
      return 48;
   case GFXSCT_Float4x4:
      return 64;
   default:
      AssertFatal(false,"shaderConstTypeSize - Unrecognized constant type");
      return 0;
   }
}

GFXVulkanShaderConstHandle::GFXVulkanShaderConstHandle( GFXVulkanShader *shader, const GFXShaderConstDesc &desc, U32 bindNum, S32 samplerNum ) 
 : mShader(shader), mInstancingConstant(false)
{
   reinit(desc, bindNum, samplerNum);
}

void GFXVulkanShaderConstHandle::reinit(const GFXShaderConstDesc& desc, U32 bindNum, S32 samplerNum)
{
   mDesc = desc;
   mSamplerNum = samplerNum;
   mOffset = 0;
   mInstancingConstant = false;
   
   U32 elemSize = shaderConstTypeSize(mDesc.constType);
   AssertFatal(elemSize, "GFXVulkanShaderConst::GFXVulkanShaderConst - elemSize is 0");
   mSize = mDesc.arraySize * elemSize;
   mValid = true;
}


U32 GFXVulkanShaderConstHandle::getSize() const
{
   return mSize;
}

GFXVulkanShaderConstHandle::~GFXVulkanShaderConstHandle()
{
}

GFXVulkanShaderConstBuffer::GFXVulkanShaderConstBuffer(GFXVulkanShader* shader, U32 bufSize, U8* existingConstants)
{
   mShader = shader;
   mBuffer = new U8[bufSize];
   mWasLost = true;

   // Copy the existing constant buffer to preserve sampler numbers
   /// @warning This preserves a lot more than sampler numbers, obviously. If there
   /// is any code that assumes a new constant buffer will have everything set to
   /// 0, it will break.
   dMemcpy(mBuffer, existingConstants, bufSize);
}

GFXVulkanShaderConstBuffer::~GFXVulkanShaderConstBuffer()
{
   delete[] mBuffer;

   if ( mShader )
      mShader->_unlinkBuffer( this );
}

template<typename ConstType>
void GFXVulkanShaderConstBuffer::internalSet(GFXShaderConstHandle* handle, const ConstType& param)
{
   AssertFatal(handle, "GFXVulkanShaderConstBuffer::internalSet - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXVulkanShaderConstBuffer::internalSet - Handle is not valid!" );
   AssertFatal(dynamic_cast<GFXVulkanShaderConstHandle*>(handle), "GFXVulkanShaderConstBuffer::set - Incorrect const buffer type");

   GFXVulkanShaderConstHandle* _VulkanHandle = static_cast<GFXVulkanShaderConstHandle*>(handle);
   AssertFatal(mShader == _VulkanHandle->mShader, "GFXVulkanShaderConstBuffer::set - Should only set handles which are owned by our shader");
   U8 *buf = mBuffer + _VulkanHandle->mOffset;

   if(_VulkanHandle->mInstancingConstant)            
      buf = mInstPtr + _VulkanHandle->mOffset;

   dMemcpy(buf, &param, sizeof(ConstType));
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const F32 fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point2F& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point3F& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point4F& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const PlaneF& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const LinearColorF& fv)
{
   internalSet(handle, fv);
}
 
void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const S32 fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point2I& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point3I& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const Point4I& fv)
{
   internalSet(handle, fv);
}

template<typename ConstType>
void GFXVulkanShaderConstBuffer::internalSet(GFXShaderConstHandle* handle, const AlignedArray<ConstType>& fv)
{
   AssertFatal(handle, "GFXVulkanShaderConstBuffer::internalSet - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXVulkanShaderConstBuffer::internalSet - Handle is not valid!" );
   AssertFatal(dynamic_cast<GFXVulkanShaderConstHandle*>(handle), "GFXVulkanShaderConstBuffer::set - Incorrect const buffer type");

   GFXVulkanShaderConstHandle* _VulkanHandle = static_cast<GFXVulkanShaderConstHandle*>(handle);
   AssertFatal(mShader == _VulkanHandle->mShader, "GFXVulkanShaderConstBuffer::set - Should only set handles which are owned by our shader");
   AssertFatal(!_VulkanHandle->mInstancingConstant, "GFXVulkanShaderConstBuffer::set - Instancing not supported for array");
   const U8* fvBuffer = static_cast<const U8*>(fv.getBuffer());
   for(U32 i = 0; i < fv.size(); ++i)
   {
      dMemcpy(mBuffer + _VulkanHandle->mOffset + i * sizeof(ConstType), fvBuffer, sizeof(ConstType));
      fvBuffer += fv.getElementSize();
   }
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<F32>& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point2F>& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point3F>& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point4F>& fv)   
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<S32>& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point2I>& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point3I>& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const AlignedArray<Point4I>& fv)
{
   internalSet(handle, fv);
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const MatrixF& mat, const GFXShaderConstType matType)
{
   AssertFatal(handle, "GFXVulkanShaderConstBuffer::set - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXVulkanShaderConstBuffer::set - Handle is not valid!" );
   AssertFatal(dynamic_cast<GFXVulkanShaderConstHandle*>(handle), "GFXVulkanShaderConstBuffer::set - Incorrect const buffer type");

   GFXVulkanShaderConstHandle* _VulkanHandle = static_cast<GFXVulkanShaderConstHandle*>(handle);
   AssertFatal(mShader == _VulkanHandle->mShader, "GFXVulkanShaderConstBuffer::set - Should only set handles which are owned by our shader");
   AssertFatal(!_VulkanHandle->mInstancingConstant || matType == GFXSCT_Float4x4, "GFXVulkanShaderConstBuffer::set - Only support GFXSCT_Float4x4 for instancing");
   
   switch(matType)
   {
   case GFXSCT_Float2x2:
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[0] = mat[0];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[1] = mat[1];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[2] = mat[4];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[3] = mat[5];
      break;
   case GFXSCT_Float3x3:
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[0] = mat[0];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[1] = mat[1];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[2] = mat[2];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[3] = mat[4];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[4] = mat[5];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[5] = mat[6];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[6] = mat[8];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[7] = mat[9];
      reinterpret_cast<F32*>(mBuffer + _VulkanHandle->mOffset)[8] = mat[10];
      break;
   case GFXSCT_Float4x3:
      dMemcpy(mBuffer + _VulkanHandle->mOffset, (const F32*)mat, (sizeof(F32) * 12));// matrix with end row chopped off
      break;
   case GFXSCT_Float4x4:
   {      
      if(_VulkanHandle->mInstancingConstant)
      {
         MatrixF transposed;   
         mat.transposeTo(transposed);
         dMemcpy( mInstPtr + _VulkanHandle->mOffset, (const F32*)transposed, sizeof(MatrixF) );
         return;
      }
      
      dMemcpy(mBuffer + _VulkanHandle->mOffset, (const F32*)mat, sizeof(MatrixF));
      break;
   }
   default:
      AssertFatal(false, "GFXVulkanShaderConstBuffer::set - Invalid matrix type");
      break;
   }
}

void GFXVulkanShaderConstBuffer::set(GFXShaderConstHandle* handle, const MatrixF* mat, const U32 arraySize, const GFXShaderConstType matrixType)
{
   AssertFatal(handle, "GFXVulkanShaderConstBuffer::set - Handle is NULL!" );
   AssertFatal(handle->isValid(), "GFXVulkanShaderConstBuffer::set - Handle is not valid!" );

   GFXVulkanShaderConstHandle* _VulkanHandle = static_cast<GFXVulkanShaderConstHandle*>(handle);
   AssertFatal(mShader == _VulkanHandle->mShader, "GFXVulkanShaderConstBuffer::set - Should only set handles which are owned by our shader");  
   AssertFatal(!_VulkanHandle->mInstancingConstant, "GFXVulkanShaderConstBuffer::set - Instancing not supported for matrix arrays");

   switch (matrixType) {
      case GFXSCT_Float4x3:
         // Copy each item with the last row chopped off
         for (int i = 0; i<arraySize; i++)
         {
            dMemcpy(mBuffer + _VulkanHandle->mOffset + (i*(sizeof(F32) * 12)), (F32*)(mat + i), sizeof(F32) * 12);
         }
      break;
      case GFXSCT_Float4x4:
         dMemcpy(mBuffer + _VulkanHandle->mOffset, (F32*)mat, _VulkanHandle->getSize());
         break;
      default:
         AssertFatal(false, "GFXVulkanShaderConstBuffer::set - setting array of non 4x4 matrices!");
         break;
   }
}

void GFXVulkanShaderConstBuffer::activate()
{
   PROFILE_SCOPE(GFXVulkanShaderConstBuffer_activate);
   mShader->setConstantsFromBuffer(this);
   mWasLost = false;
}

const String GFXVulkanShaderConstBuffer::describeSelf() const
{
   return String();
}

void GFXVulkanShaderConstBuffer::onShaderReload( GFXVulkanShader *shader )
{
   AssertFatal( shader == mShader, "GFXVulkanShaderConstBuffer::onShaderReload, mismatched shaders!" );

   delete[] mBuffer;
   mBuffer = new U8[mShader->mConstBufferSize];
   dMemset(mBuffer, 0, mShader->mConstBufferSize);
   mWasLost = true;
}

GFXVulkanShader::GFXVulkanShader() :
   mVertexShader(0),
   mPixelShader(0),
   mConstBufferSize(0),
   mConstBuffer(NULL),
	pipeline(VK_NULL_HANDLE),
	vertexInputInfo({})
{
}

GFXVulkanShader::~GFXVulkanShader()
{
   clearShaders();
   for(HandleMap::Iterator i = mHandles.begin(); i != mHandles.end(); i++)
      delete i->value;
   
   delete[] mConstBuffer;
}

void GFXVulkanShader::clearShaders()
{
    vkDestroyShaderModule(GFXVulkan->getLogicalDevice(), mVertexShader, nullptr);
    vkDestroyShaderModule(GFXVulkan->getLogicalDevice(), mPixelShader, nullptr);
}

bool GFXVulkanShader::_init()
{
   PROFILE_SCOPE(GFXVulkanShader_Init);
   // Don't initialize empty shaders.
   if ( mVertexFile.isEmpty() && mPixelFile.isEmpty() )
      return false;

   clearShaders();
   
   // Set the macros and add the global ones.
   Vector<GFXShaderMacro> macros;
   macros.merge( mMacros );
   //macros.merge( smVulkanobalMacros );
   
   macros.increment();
   macros.last().name = "TORQUE_SM";
   macros.last().value = 40;
   macros.increment();
   macros.last().name = "TORQUE_VERTEX_SHADER";
   macros.last().value = "";
   
   // Default to true so we're "successful" if a vertex/pixel shader wasn't specified.
   bool compiledVertexShader = true;
   bool compiledPixelShader = true;
   
   // Compile the vertex and pixel shaders if specified.
   if(!mVertexFile.isEmpty())
      compiledVertexShader = initShader(mVertexFile, true, macros);

   macros.last().name = "TORQUE_PIXEL_SHADER";
   if(!mPixelFile.isEmpty())
      compiledPixelShader = initShader(mPixelFile, false, macros);
      
   // If either shader was present and failed to compile, bail.
   if(!compiledVertexShader || !compiledPixelShader)
      return false;


   shaderStages[0] = {};
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = mVertexShader;
	shaderStages[0].pName = "main";

	shaderStages[1] = {};
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = mPixelShader;
	shaderStages[1].pName = "main";

   Con::printf ("VertexFile: %s, PixelFile: %s", mVertexFile.getFullPath().c_str(), mPixelFile.getFullPath().c_str());

   initConstantDescs();   
   initHandles();
   //
   //// Notify Buffers we might have changed in size. 
   //// If this was our first init then we won't have any activeBuffers 
   //// to worry about unnecessarily calling.
   //Vector<GFXShaderConstBuffer*>::iterator biter = mActiveBuffers.begin();
   //for ( ; biter != mActiveBuffers.end(); biter++ )   
   //   ((GFXVulkanShaderConstBuffer*)(*biter))->onShaderReload( this );
   // Vertex layout for this example

	return true;
}

void GFXVulkanShader::initConstantDescs()
{
   mConstants.clear();
   U32 numUniforms = 0;
   //VulkanGetProgramiv(mProgram, Vulkan_ACTIVE_UNIFORMS, &numUniforms);
   //Vulkanint maxNameLength;
   //VulkanGetProgramiv(mProgram, Vulkan_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

   //if(!maxNameLength)
   //   return;

   //FrameTemp<Vulkanchar> uniformName(maxNameLength);
   
   //for(U32 i = 0; i < numUniforms; i++)
   //{
   //   Vulkanint size;
   //   Vulkanenum type;
   //   VulkanGetActiveUniform(mProgram, i, maxNameLength, NULL, &size, &type, uniformName);
   //   GFXShaderConstDesc desc;
   //   
   //   desc.name = String((char*)uniformName);
   //   
   //   // Remove array brackets from the name
   //   desc.name = desc.name.substr(0, desc.name.find('['));
   //   
   //   // Insert $ to match D3D behavior of having a $ prepended to parameters to main.
   //   desc.name.insert(0, '$');
   //   desc.arraySize = size;
   //   
   //   switch(type)
   //   {
   //      case Vulkan_FLOAT:
   //         desc.constType = GFXSCT_Float;
   //         break;
   //      case Vulkan_FLOAT_VEC2:
   //         desc.constType = GFXSCT_Float2;
   //         break;
   //      case Vulkan_FLOAT_VEC3:
   //         desc.constType = GFXSCT_Float3;
   //         break;
   //      case Vulkan_FLOAT_VEC4:
   //         desc.constType = GFXSCT_Float4;
   //         break;
   //      case Vulkan_INT:
   //         desc.constType = GFXSCT_Int;
   //         break;
   //      case Vulkan_INT_VEC2:
   //         desc.constType = GFXSCT_Int2;
   //         break;
   //      case Vulkan_INT_VEC3:
   //         desc.constType = GFXSCT_Int3;
   //         break;
   //      case Vulkan_INT_VEC4:
   //         desc.constType = GFXSCT_Int4;
   //         break;
   //      case Vulkan_FLOAT_MAT2:
   //         desc.constType = GFXSCT_Float2x2;
   //         break;
   //      case Vulkan_FLOAT_MAT3:
   //         desc.constType = GFXSCT_Float3x3;
   //         break;
   //      case Vulkan_FLOAT_MAT4:
   //         desc.constType = GFXSCT_Float4x4;
   //         break;
   //      case Vulkan_FLOAT_MAT4x3: // jamesu - columns, rows
   //         desc.constType = GFXSCT_Float4x3;
   //         break;
   //      case Vulkan_SAMPLER_1D:
   //      case Vulkan_SAMPLER_2D:
   //      case Vulkan_SAMPLER_3D:
   //      case Vulkan_SAMPLER_1D_SHADOW:
   //      case Vulkan_SAMPLER_2D_SHADOW:
   //         desc.constType = GFXSCT_Sampler;
   //         break;
   //      case Vulkan_SAMPLER_CUBE:
   //         desc.constType = GFXSCT_SamplerCube;
   //         break;
   //      default:
   //         AssertFatal(false, "GFXVulkanShader::initConstantDescs - unrecognized uniform type");
   //         // If we don't recognize the constant don't add its description.
   //         continue;
   //   }
      
      //mConstants.push_back(desc);
   //}
}

void GFXVulkanShader::initHandles()
{      
   // Mark all existing handles as invalid.
   // Those that are found when parsing the descriptions will then be marked valid again.
   for ( HandleMap::Iterator iter = mHandles.begin(); iter != mHandles.end(); ++iter )      
      (iter->value)->setValid( false );  
   mValidHandles.clear();

   // Loop through all ConstantDescriptions, 
   // if they aren't in the HandleMap add them, if they are reinitialize them.
   for ( U32 i = 0; i < mConstants.size(); i++ )
   {
      GFXShaderConstDesc &desc = mConstants[i];            

      //// Index element 1 of the name to skip the '$' we inserted earier.
      //Vulkanint loc = VulkanGetUniformLocation(mProgram, &desc.name.c_str()[1]);

      //AssertFatal(loc != -1, "");

      HandleMap::Iterator handle = mHandles.find(desc.name);
      S32 sampler = -1;
      if(desc.constType == GFXSCT_Sampler || desc.constType == GFXSCT_SamplerCube)
      {
         S32 idx = mSamplerNamesOrdered.find_next(desc.name);
         AssertFatal(idx != -1, "");
         sampler = idx; //assignedSamplerNum++;
      }
      if ( handle != mHandles.end() )
      {
         handle->value->reinit( desc, 0, sampler);         
      } 
      else 
      {
         mHandles[desc.name] = new GFXVulkanShaderConstHandle( this, desc, 0, sampler );      
      }
   }

   // Loop through handles once more to set their offset and calculate our
   // constBuffer size.

   if ( mConstBuffer )
      delete[] mConstBuffer;
   mConstBufferSize = 0;

   for ( HandleMap::Iterator iter = mHandles.begin(); iter != mHandles.end(); ++iter )
   {
      GFXVulkanShaderConstHandle* handle = iter->value;
      if ( handle->isValid() )
      {
      	mValidHandles.push_back(handle);
         handle->mOffset = mConstBufferSize;
         mConstBufferSize += handle->getSize();
      }
   }
   
   mConstBuffer = new U8[mConstBufferSize];
   dMemset(mConstBuffer, 0, mConstBufferSize);
   
   //// Set our program so uniforms are assigned properly.
   //VulkanUseProgram(mProgram);
   //// Iterate through uniforms to set sampler numbers.
   //for (HandleMap::Iterator iter = mHandles.begin(); iter != mHandles.end(); ++iter)
   //{
   //   GFXVulkanShaderConstHandle* handle = iter->value;
   //   if(handle->isValid() && (handle->getType() == GFXSCT_Sampler || handle->getType() == GFXSCT_SamplerCube))
   //   {
   //      // Set sampler number on our program.
   //      VulkanUniform1i(handle->mLocation, handle->mSamplerNum);
   //      // Set sampler in constant buffer so it does not get unset later.
   //      dMemcpy(mConstBuffer + handle->mOffset, &handle->mSamplerNum, handle->getSize());
   //   }
   //}
   //VulkanUseProgram(0);

   //instancing
   if (!mInstancingFormat)
      return;

   U32 offset = 0;

   for ( U32 i=0; i < mInstancingFormat->getElementCount(); i++ )
   {
      const GFXVertexElement &element = mInstancingFormat->getElement( i );
      
      String constName = String::ToString( "$%s", element.getSemantic().c_str() );

      HandleMap::Iterator handle = mHandles.find(constName);      
      if ( handle != mHandles.end() )
      {          
         AssertFatal(0, "");
      } 
      else 
      {
         GFXShaderConstDesc desc;
         desc.name = constName;
         desc.arraySize = 1;
         switch(element.getType())
         {
         case GFXDeclType_Float4:
            desc.constType = GFXSCT_Float4;
            break;

         default:
            desc.constType = GFXSCT_Float;
            break;
         }
         
         GFXVulkanShaderConstHandle *h = new GFXVulkanShaderConstHandle( this, desc, 0, -1 );
         h->mInstancingConstant = true;
         h->mOffset = offset;
         mHandles[constName] =  h;

         offset += element.getSizeInBytes();
         ++i;

         // If this is a matrix we will have 2 or 3 more of these
         // semantics with the same name after it.
         for ( ; i < mInstancingFormat->getElementCount(); i++ )
         {
            const GFXVertexElement &nextElement = mInstancingFormat->getElement( i );
            if ( nextElement.getSemantic() != element.getSemantic() )
            {
               i--;
               break;
            }
            ++desc.arraySize;
            if(desc.arraySize == 4 && desc.constType == GFXSCT_Float4)
            {
               desc.arraySize = 1;
               desc.constType = GFXSCT_Float4x4;
            }
            offset += nextElement.getSizeInBytes();
         }
      }

   }
}

GFXShaderConstHandle* GFXVulkanShader::getShaderConstHandle(const String& name)
{
   HandleMap::Iterator i = mHandles.find(name);
   if(i != mHandles.end())
      return i->value;
   else
   {
      GFXVulkanShaderConstHandle* handle = new GFXVulkanShaderConstHandle( this );
	  handle->mBindNum = mHandles.size();
      mHandles[ name ] = handle;
	  std::string temp(name.c_str());
	  handle->mDesc = GFXVulkanShaderConstDesc[name];

	  return handle;
   }
}

GFXShaderConstHandle* GFXVulkanShader::findShaderConstHandle(const String& name)
{
   HandleMap::Iterator i = mHandles.find(name);
   if(i != mHandles.end())
      return i->value;
   else
   {
      return NULL;
   }
}

void GFXVulkanShader::setConstantsFromBuffer(GFXVulkanShaderConstBuffer* buffer)
{
   for(Vector<GFXVulkanShaderConstHandle*>::iterator i = mValidHandles.begin(); i != mValidHandles.end(); ++i)
   {
      GFXVulkanShaderConstHandle* handle = *i;
      AssertFatal(handle, "GFXVulkanShader::setConstantsFromBuffer - Null handle");

      if(handle->mInstancingConstant)
         continue;
      
      // Don't set if the value has not be changed.
      if(dMemcmp(mConstBuffer + handle->mOffset, buffer->mBuffer + handle->mOffset, handle->getSize()) == 0)
         continue;
         
      // Copy new value into our const buffer and set in Vulkan.
   //   dMemcpy(mConstBuffer + handle->mOffset, buffer->mBuffer + handle->mOffset, handle->getSize());
      switch(handle->mDesc.constType)
      {
         case GFXSCT_Float:
            //VulkanUniform1fv(handle->mLocation, handle->mDesc.arraySize, (Vulkanfloat*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Float2:
            //VulkanUniform2fv(handle->mLocation, handle->mDesc.arraySize, (Vulkanfloat*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Float3:
            //VulkanUniform3fv(handle->mLocation, handle->mDesc.arraySize, (Vulkanfloat*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Float4:
            //VulkanUniform4fv(handle->mLocation, handle->mDesc.arraySize, (Vulkanfloat*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Int:
         case GFXSCT_Sampler:
         case GFXSCT_SamplerCube:
            //VulkanUniform1iv(handle->mLocation, handle->mDesc.arraySize, (Vulkanint*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Int2:
            //VulkanUniform2iv(handle->mLocation, handle->mDesc.arraySize, (Vulkanint*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Int3:
            //VulkanUniform3iv(handle->mLocation, handle->mDesc.arraySize, (Vulkanint*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Int4:
            //VulkanUniform4iv(handle->mLocation, handle->mDesc.arraySize, (Vulkanint*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Float2x2:
            //VulkanUniformMatrix2fv(handle->mLocation, handle->mDesc.arraySize, true, (Vulkanfloat*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Float3x3:
            //VulkanUniformMatrix3fv(handle->mLocation, handle->mDesc.arraySize, true, (Vulkanfloat*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Float4x3:
            // NOTE: To save a transpose here we could store the matrix transposed (i.e. column major) in the constant buffer.
            // See _mesa_uniform_matrix in the mesa source for the correct transpose algorithm for a 4x3 matrix. 
            //VulkanUniformMatrix4x3fv(handle->mLocation, handle->mDesc.arraySize, true, (Vulkanfloat*)(mConstBuffer + handle->mOffset));
            break;
         case GFXSCT_Float4x4:
            //VulkanUniformMatrix4fv(handle->mLocation, handle->mDesc.arraySize, true, (Vulkanfloat*)(mConstBuffer + handle->mOffset));
            break;
         default:
            AssertFatal(0,"");
            break;
      }
   }
}

GFXShaderConstBufferRef GFXVulkanShader::allocConstBuffer()
{
   GFXVulkanShaderConstBuffer* buffer = new GFXVulkanShaderConstBuffer(this, mConstBufferSize, mConstBuffer);
   buffer->registerResourceWithDevice(getOwningDevice());
   mActiveBuffers.push_back( buffer );
   return buffer;
}

void GFXVulkanShader::useProgram()
{
   //VulkanUseProgram(mProgram);
}

void GFXVulkanShader::setupPipeline()
{
struct Vertex {
	float pos[4];
	float color[4];
};

		// Binding description
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
			VulkanUtils::vertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX),
		};

		// Attribute descriptions
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			VulkanUtils::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, pos)),	// Location 0 : Position
			VulkanUtils::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)),	// Location 1 : Color
		};

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data(); // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional
}

void GFXVulkanShader::setVertexFormat(const GFXVertexFormat* format)
{
	GFXVulkanVertexDecl* decl = dynamic_cast<GFXVulkanVertexDecl*>(format->getDecl());
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = decl->vertices.bindingDescriptions.size();
	vertexInputInfo.pVertexBindingDescriptions = decl->vertices.bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = decl->vertices.attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = decl->vertices.attributeDescriptions.data();
}

void GFXVulkanShader::zombify()
{
   clearShaders();
   dMemset(mConstBuffer, 0, mConstBufferSize);
}


bool GFXVulkanShader::_loadShaderFromStream(  VkShaderModule &shader, 
                                          FileStream *s )
{
	std::vector<char> code;
	s->read(code);

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkResult res = vkCreateShaderModule(GFXVulkan->getLogicalDevice(), &createInfo, nullptr, &shader);
	if ( res != VK_SUCCESS) {
		Con::errorf ("failed to create shader module!");
		return false;
	}

	return true;
}

void GFXVulkanShader::AddPipelineShaderStage(ShaderStageParameters shader_stage)
{
	shader_stage_create_infos.push_back({
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		nullptr,
		0,
		shader_stage.ShaderStage,
		shader_stage.ShaderModule,
		shader_stage.EntryPointName,
		shader_stage.SpecializationInfo
		});
}

void GFXVulkanShader::SetPipelineShaderStages(std::vector<ShaderStageParameters> const shader_stages)
{
	shader_stage_create_infos.clear();
	for (auto & shader_stage: shader_stages)
		AddPipelineShaderStage(shader_stage);
}


//void GFXVulkanShader::setPipelineInputAssembState(GFXPrimitiveType primType, VkBool32 primitive_restart_enable)
//{
//	input_assembly_state_create_info = {
//	VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
//	nullptr,
//	0,
//	 GFXVulkanPrimType[primType],
//	 primitive_restart_enable
//	};
//}
//
//void GFXVulkanShader::SpecPipelineTessellationState(U32 patch_control_points_count)
//{
//	tessellation_state_create_info = {
//	VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
//		nullptr,
//		0,
//		patch_control_points_count
//	};
//}
//
//void GFXVulkanShader::SpecPipelineViewportAndScissor(ViewportInfo viewport_infos)
//{
//	VkPipelineViewportStateCreateInfo viewport_state_create_info = {
//	VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
//	nullptr,
//	0,
//	static_cast<U32>(viewport_infos.Viewports.size()),
//	viewport_infos.Viewports.data(),
//	static_cast<U32>(viewport_infos.Scissors.size()),
//	viewport_infos.Scissors.data()
//	};
//}
//
//void GFXVulkanShader::SpecPipelineRasterizationState(VkBool32 depth_clamp_enable,
//                                                     VkBool32 rasterization_discard_enable,
//                                                     VkPolygonMode polygon_mode,
//                                                     VkCullModeFlags culling_mode,
//                                                     VkFrontFace front_face,
//                                                     VkBool32 depth_bias_enable,
//                                                     F32 depth_bias_constant_factor,
//                                                     F32 depth_bias_clamp,
//                                                     F32 depth_bias_slope_factor,
//                                                     F32 line_width)
//{
//	rasterization_state_create_info = {
//		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//		nullptr,
//		0,
//		depth_clamp_enable,
//		rasterization_discard_enable,
//		polygon_mode,
//		culling_mode,
//		front_face,
//		depth_bias_enable,
//		depth_bias_constant_factor,
//		depth_bias_clamp,
//		depth_bias_slope_factor,
//		line_width
//		};
//}
//
//void GFXVulkanShader::SpecPipelineMultisampleState(VkSampleCountFlagBits sample_count,
//                                                   VkBool32 per_sample_shading_enable,
//                                                   float min_sample_shading,
//                                                   const VkSampleMask* sample_masks,
//                                                   VkBool32 alpha_to_coverage_enable,
//                                                   VkBool32 alpha_to_one_enable)
//{
//	multisample_state_create_info = {
//		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
//		nullptr,
//		0,
//		sample_count,
//		per_sample_shading_enable,
//		min_sample_shading,
//		sample_masks,
//		alpha_to_coverage_enable,
//		alpha_to_one_enable
//		};
//}
//
//void GFXVulkanShader::SpecPipelineDepthandStencilState(VkBool32 depth_test_enable,
//                                                       VkBool32 depth_write_enable,
//                                                       VkCompareOp depth_compare_op,
//                                                       VkBool32 depth_bounds_test_enable,
//                                                       VkBool32 stencil_test_enable,
//                                                       VkStencilOpState front_stencil_test_parameters,
//                                                       VkStencilOpState back_stencil_test_parameters,
//                                                       float min_depth_bounds,
//                                                       float max_depth_bounds)
//{
//	VkPipelineDepthStencilStateCreateInfo depth_and_stencil_state_create_info = {
//	VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
//	nullptr,
//	0,
//		depth_test_enable,
//		depth_write_enable,
//		depth_compare_op,
//		depth_bounds_test_enable,
//		stencil_test_enable,
//		front_stencil_test_parameters,
//		back_stencil_test_parameters,
//		min_depth_bounds,
//		max_depth_bounds
//	};
//}
//
//void GFXVulkanShader::SpecPipelineBlendState(VkBool32 logic_op_enable,
//                                             VkLogicOp logic_op,
//                                             std::vector<VkPipelineColorBlendAttachmentState> attachment_blend_states,
//                                             float blend_constants[4])
//{
//	blend_state_create_info = {
//		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
//		nullptr, 
//		0,
//		logic_op_enable,
//		logic_op,
//		static_cast<U32>(attachment_blend_states.size()),
//		attachment_blend_states.data(),
//	{
//	blend_constants[0],
//	blend_constants[1],
//	blend_constants[2],
//	blend_constants[3]
//	}
//	};
//}
//
//void GFXVulkanShader::SpecPipelineDynamicStates(std::vector<VkDynamicState> dynamic_states)
//{
//	VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
//	VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
//		nullptr,
//		0,
//		static_cast<U32>(dynamic_states.size()),
//		dynamic_states.data()
//		};
//}
//
//
//void GFXVulkanShader::SpecPipelineCreationParams(VkPipelineCreateFlags& additional_options, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, U32 subpass, VkPipeline
//                                                 basePipelineHandle, S32 basePipelineIndex)
//{
//
//	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
//		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
//		nullptr,
//		additional_options,
//		static_cast<U32>(shader_stage_create_infos.size()),
//		shader_stage_create_infos.data(),
//		&vertex_input_state_create_info,
//		&input_assembly_state_create_info,
//		&tessellation_state_create_info,
//		&viewport_state_create_info,
//		&rasterization_state_create_info,
//		&multisample_state_create_info,
//		&depth_and_stencil_state_create_info,
//		&blend_state_create_info,
//		&dynamic_state_create_info,
//		pipeline_layout,
//		render_pass,
//		subpass,
//		basePipelineHandle,
//		basePipelineIndex
//	};
//}
//
//bool GFXVulkanShader::CreatePipelineCacheObject()
//{
//	std::vector<unsigned char> cache_data;
//	VkPipelineCacheCreateInfo pipeline_cache_create_info =
//	{
//		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
//		nullptr,
//		0,
//		static_cast<U32>(cache_data.size()),
//		cache_data.data()
//	};
//	VkPipelineCache pipeline_cache;
//	if (vkCreatePipelineCache(GFXVulkan->getLogicalDevice(), &pipeline_cache_create_info, nullptr, &pipeline_cache) != VK_SUCCESS)
//	{
//		Con::printf ("Could not create pipeline cache");
//		return false;
//	}
//	return true;
//}
//
//bool GFXVulkanShader::GetDataFromPipelineCache(VkPipelineCache& pipeline_cache, std::vector<unsigned char>& pipeline_cache_data)
//{
//	size_t data_size = 0;
//
//	if (vkGetPipelineCacheData(GFXVulkan->getLogicalDevice(), pipeline_cache, &data_size, nullptr) != VK_SUCCESS)
//	{
//		Con::errorf("Could not get the size of the pipeline cache.");
//		return false;
//	}
//	pipeline_cache_data.resize(data_size);
//	if (vkGetPipelineCacheData(GFXVulkan->getLogicalDevice(), pipeline_cache, &data_size, pipeline_cache_data.data()) != VK_SUCCESS)
//	{
//		Con::errorf("Could not aquire pipeline cache data.");
//		return false;
//	}
//	return true;
//}
//
//bool GFXVulkanShader::MergePipelineCacheObjects(VkPipelineCache & target_pipeline_cache, std::vector<VkPipelineCache>& source_pipeline_caches)
//{
//	if (vkMergePipelineCaches(GFXVulkan->getLogicalDevice(), target_pipeline_cache, static_cast<U32>(source_pipeline_caches.size()), source_pipeline_caches.data()) != VK_SUCCESS)
//	{
//		Con::errorf("Could not merge pipeline cache objects.");
//		return false;
//	};
//	return true;
//}

bool GFXVulkanShader::initShader( const Torque::Path &file, 
                              bool isVertex, 
                              const Vector<GFXShaderMacro> &macros )
{
   PROFILE_SCOPE(GFXVulkanShader_CompileShader);

	// Ok it's not in the shader gen manager, so ask Torque for it
   FileStream stream;
   if ( !stream.open( file, Torque::FS::File::Read ) )
   {
      AssertISV(false, avar("GFXVulkanShader::initShader - failed to open shader '%s'.", file.getFullPath().c_str()));

      if ( smLogErrors )
         Con::errorf( "GFXVulkanShader::initShader - Failed to open shader file '%s'.", 
            file.getFullPath().c_str() );

      return false;
   }
   
   if ( !_loadShaderFromStream(isVertex ? mVertexShader : mPixelShader, &stream ) )
      return false;

   return true;
}

/// Returns our list of shader constants, the material can get this and just set the constants it knows about
const Vector<GFXShaderConstDesc>& GFXVulkanShader::getShaderConstDesc() const
{
   PROFILE_SCOPE(GFXVulkanShader_GetShaderConstants);
   return mConstants;
}

/// Returns the alignment value for constType
U32 GFXVulkanShader::getAlignmentValue(const GFXShaderConstType constType) const
{
   // Alignment is the same thing as size for us.
   return shaderConstTypeSize(constType);
}

const String GFXVulkanShader::describeSelf() const
{
   String ret;
   ret += String::ToString("   Vertex Path: %s", mVertexFile.getFullPath().c_str());
   ret += String::ToString("   Pixel Path: %s", mPixelFile.getFullPath().c_str());
   
   return ret;
}
