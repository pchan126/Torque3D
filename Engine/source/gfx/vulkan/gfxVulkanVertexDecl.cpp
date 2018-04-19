#include "gfx/Vulkan/gfxVulkanDevice.h"
#include "gfx/Vulkan/gfxVulkanStateCache.h"
#include "gfx/Vulkan/gfxVulkanVertexAttribLocation.h"
#include "gfx/Vulkan/gfxVulkanVertexDecl.h"

void GFXVulkanVertexDecl::init(const GFXVertexFormat *format)
{
   AssertFatal(!mFormat, "");
   mFormat = format;
  
   for(int i = 0; i < GFXVulkan->getNumVertexStreams(); ++i)
      _initVerticesFormat(i);   
}

void GFXVulkanVertexDecl::prepareVertexFormat() const
{
   AssertFatal(mFormat, "GFXVulkanVertexDecl - Not inited");
   //if( GFXVulkan->mCapabilities.vertexAttributeBinding )
   //{
   //   for ( U32 i=0; i < VulkanVerticesFormat.size(); i++ )
   //   {
   //      const VulkanVertexAttribData &VulkanElement = VulkanVerticesFormat[i];
   //   
   //      VulkanVertexAttribFormat( VulkanElement.attrIndex, VulkanElement.elementCount, VulkanElement.type, VulkanElement.normalized, (uintptr_t)VulkanElement.pointerFirst );
   //      VulkanVertexAttribBinding( VulkanElement.attrIndex, VulkanElement.stream );
   //   }

   //   updateActiveVertexAttrib( GFXVulkan->getOpenVulkanCache()->getCacheVertexAttribActive() );

   //   return;
   //}
}

//void GFXVulkanVertexDecl::prepareBuffer_old(U32 stream, Vulkanint mBuffer, Vulkanint mDivisor) const
//{
//   PROFILE_SCOPE(GFXVulkanVertexDecl_prepare);
//   AssertFatal(mFormat, "GFXVulkanVertexDecl - Not inited");
//
//   if( GFXVulkan->mCapabilities.vertexAttributeBinding )
//      return;   
//
//	// Bind the buffer...
//   VulkanBindBuffer(Vulkan_ARRAY_BUFFER, mBuffer);
//   GFXVulkan->getOpenVulkanCache()->setCacheBinded(Vulkan_ARRAY_BUFFER, mBuffer);
//
//   // Loop thru the vertex format elements adding the array state...   
//   for ( U32 i=0; i < VulkanVerticesFormat.size(); i++ )
//   {
//      // VulkanEnableVertexAttribArray are called and cache in GFXVulkanDevice::preDrawPrimitive
//
//      const VulkanVertexAttribData &e = VulkanVerticesFormat[i];
//      if(e.stream != stream)
//         continue;
//      
//      VulkanVertexAttribPointer(
//         e.attrIndex,      // attribute
//         e.elementCount,   // number of elements per vertex, here (r,g,b)
//         e.type,           // the type of each element
//         e.normalized,     // take our values as-is
//         e.stride,         // stride between each position
//         e.pointerFirst    // offset of first element
//      );
//      VulkanVertexAttribDivisor( e.attrIndex, mDivisor );
//   }
//}

void GFXVulkanVertexDecl::updateActiveVertexAttrib(U32 lastActiveMask) const
{
   AssertFatal(mVertexAttribActiveMask, "GFXVulkanVertexDecl::updateActiveVertexAttrib - No vertex attribute are active");

   //U32 lastActiveVerxtexAttrib = GFXVulkan->getOpenVulkanCache()->getCacheVertexAttribActive();
   //if(mVertexAttribActiveMask == lastActiveVerxtexAttrib)
   //   return;

   //U32 forActiveMask = mVertexAttribActiveMask & ~lastActiveVerxtexAttrib;
   //U32 forDeactiveMask = ~mVertexAttribActiveMask & lastActiveVerxtexAttrib;
   //for(int i = 0; i < Torque::Vulkan_VertexAttrib_COUNT; ++i)
   //{         
   //   if( BIT(i) & forActiveMask ) //if is active but not in last mask
   //      VulkanEnableVertexAttribArray(i);
   //   else if( BIT(i) & forDeactiveMask ) // if not active but in last mask
   //      VulkanDisableVertexAttribArray(i);
   //}

   //GFXVulkan->getOpenVulkanCache()->setCacheVertexAttribActive(mVertexAttribActiveMask);
}

void GFXVulkanVertexDecl::_initVerticesFormat2()
{
   for( U32 i=0; i < GFXVulkan->getNumVertexStreams(); ++i )
   {
      _initVerticesFormat(i);
   }
}

void GFXVulkanVertexDecl::_initVerticesFormat(U32 stream)
{   
   U32 buffer = 0;
   U32 vertexSize = 0;

   for ( U32 i=0; i < mFormat->getElementCount(); i++ )
   {
      const GFXVertexElement &element = mFormat->getElement( i );

      if(element.getStreamIndex() != stream)
         continue;

      AssertFatal(!mFormat->hasBlendIndices() || !element.isSemantic(GFXSemantic::TEXCOORD) || (mFormat->hasBlendIndices() && element.isSemantic(GFXSemantic::TEXCOORD) && element.getSemanticIndex() < 2), "skinning with more than 2 used texcoords!");

      vertexSize += element.getSizeInBytes();
   }

   // Loop thru the vertex format elements adding the array state...
   U32 texCoordIndex = 0;
   
   for ( U32 i=0; i < mFormat->getElementCount(); i++ )
   {
      const GFXVertexElement &element = mFormat->getElement( i );

      if(element.getStreamIndex() != stream)
         continue;

      //VulkanVerticesFormat.increment();
      //VulkanVertexAttribData &VulkanElement = VulkanVerticesFormat.last();
      //VulkanElement.stream = element.getStreamIndex();
	   vertices.bindingDescriptions.setSize(1);
       vertices.bindingDescriptions[0] = VulkanUtils::vertexInputBindingDescription(0, vertexSize, VK_VERTEX_INPUT_RATE_VERTEX); 

	   vertices.attributeDescriptions.increment();
	   VkVertexInputAttributeDescription &inputAttributeDesc = vertices.attributeDescriptions.last();

    if ( element.isSemantic( GFXSemantic::POSITION ) )
      {           
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_Position, VK_FORMAT_R32G32B32A32_SFLOAT, buffer);
         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::NORMAL ) )
      {
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_Normal, VK_FORMAT_R32G32B32_SFLOAT, buffer);

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::TANGENT ) )
      {
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_Tangent, VK_FORMAT_R32G32B32_SFLOAT, buffer);

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::TANGENTW ) )
      {
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_TangentW, VK_FORMAT_R32G32B32_SFLOAT, buffer);

      	buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::BINORMAL ) )
      {
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_Binormal, VK_FORMAT_R32G32B32_SFLOAT, buffer);
         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::COLOR ) )
      {
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_Color, VK_FORMAT_R32G32B32A32_SFLOAT, buffer);

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::BLENDWEIGHT ) )
      {
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_BlendWeight0 + element.getSemanticIndex(), VK_FORMAT_R32G32B32A32_SFLOAT, buffer);

         buffer += element.getSizeInBytes();
      }
      else if ( element.isSemantic( GFXSemantic::BLENDINDICES ) )
      {
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_BlendIndex0 + element.getSemanticIndex(), VK_FORMAT_R32G32B32A32_SFLOAT, buffer);

         buffer += element.getSizeInBytes();
      }
      else // Everything else is a texture coordinate.
      {
      	inputAttributeDesc = VulkanUtils::vertexInputAttributeDescription(0, Torque::Vulkan_VertexAttrib_TexCoord0 + texCoordIndex, VK_FORMAT_R32G32_SFLOAT, buffer);

         buffer += element.getSizeInBytes();
         ++texCoordIndex;
      }

      //AssertFatal(!( mVertexAttribActiveMask & BIT(VulkanElement.attrIndex) ), "GFXVulkanVertexBuffer::_initVerticesFormat - Duplicate vertex attrib index");
      //mVertexAttribActiveMask |= BIT(VulkanElement.attrIndex);
   }

	vertices.inputState = VulkanUtils::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();

	//mVertexSize[stream] = vertexSize;
   AssertFatal(vertexSize == buffer, "");
}