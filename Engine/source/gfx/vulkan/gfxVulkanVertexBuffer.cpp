//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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
#include "gfx/vulkan/gfxVulkanVertexBuffer.h"
#include "console/console.h"

#include "gfx/vulkan/gfxVulkanCircularVolatileBuffer.h"

VulkanCircularVolatileBuffer* getCircularVolatileVertexBuffer()
{
	static VulkanCircularVolatileBuffer sCircularVolatileVertexBuffer = VulkanCircularVolatileBuffer();
   return &sCircularVolatileVertexBuffer;
}

GFXVulkanVertexBuffer::GFXVulkanVertexBuffer(   GFXDevice *device, 
                                                   U32 numVerts, 
                                                   const GFXVertexFormat *vertexFormat, 
                                                   U32 vertexSize, 
                                                   GFXBufferType bufferType )
   : GFXVertexBuffer( device, numVerts, vertexFormat, vertexSize, bufferType )
{
   mIsFirstLock = true;
   mClearAtFrameEnd = false;
   lockedVertexEnd = lockedVertexStart = 0;
   mLockedBuffer = NULL;
	mBufferOffset = 0;

#ifdef TORQUE_DEBUG
   mDebugGuardBuffer = NULL;   
#endif

	if( mBufferType == GFXBufferTypeVolatile )
   {
      vertexBuffer = getCircularVolatileVertexBuffer()->getHandle();
      return;
   }

   // Generate a buffer
   if (!GFXVulkan->createBuffer(numVerts*vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer))
	   Con::errorf("failed to create vertex buffer");

   if (!GFXVulkan->createBuffer(numVerts, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer))
	   Con::errorf("failed to create index buffer");

	////and allocate the needed memory
	if (!GFXVulkan->allocateAndBindMemoryObjectForBuffer(vertexBuffer, vertex_buffer_memory_, static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)))
	   Con::errorf("failed to allocate memory for vertex buffer");

	if (!GFXVulkan->allocateAndBindMemoryObjectForBuffer(indexBuffer, index_buffer_memory_, static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)))
	   Con::errorf("failed to allocate memory for index buffer");

}

GFXVulkanVertexBuffer::~GFXVulkanVertexBuffer() 
{
	GFXVulkan->freeMemoryObject(vertex_buffer_memory_);
	vkDestroyBuffer(GFXVulkan->getLogicalDevice(), vertexBuffer, nullptr);
	GFXVulkan->freeMemoryObject(index_buffer_memory_);
	vkDestroyBuffer(GFXVulkan->getLogicalDevice(), indexBuffer, nullptr);
}

void GFXVulkanVertexBuffer::lock(U32 vertexStart, U32 vertexEnd, void **vertexPtr)
{
   PROFILE_SCOPE(GFXVulkanVertexBuffer_lock);

   if( mBufferType == GFXBufferTypeVolatile )
   {
      AssertFatal(vertexStart == 0, "");
      //if( GFXGL->mCapabilities.vertexAttributeBinding )
      //{
         getCircularVolatileVertexBuffer()->lock( mNumVerts * mVertexSize, 0, mBufferOffset, *vertexPtr );
      //}
      //else
      //{
      //   getCircularVolatileVertexBuffer()->lock( mNumVerts * mVertexSize, mVertexSize, mBufferOffset, *vertexPtr );
      //   mBufferVertexOffset = mBufferOffset / mVertexSize;
      //}
   }
   else
   {
      //mFrameAllocator.lock( mNumVerts * mVertexSize );

//      lockedVertexPtr = (void*)(mFrameAllocator.getlockedPtr() + (vertexStart * mVertexSize));
//      *vertexPtr = lockedVertexPtr;
		if (vkMapMemory( GFXVulkan->getLogicalDevice(), vertex_buffer_memory_, 0, mVertexSize, 0, vertexPtr ) != VK_SUCCESS )
		{
			Con::errorf("GFXVulkanVertexBuffer::lock - error mapping memory");
		}
   }

	lockedVertexStart = vertexStart;
	lockedVertexEnd   = vertexEnd;
}

void GFXVulkanVertexBuffer::unlock()
{
   PROFILE_SCOPE(GFXVulkanVertexBuffer_unlock);

   if( mBufferType == GFXBufferTypeVolatile )
   {
      getCircularVolatileVertexBuffer()->unlock();
   }
   else
   {
      U32 offset = lockedVertexStart * mVertexSize;
      U32 length = (lockedVertexEnd - lockedVertexStart) * mVertexSize;
   
      //PRESERVE_VERTEX_BUFFER();
      //glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   
      //if( !lockedVertexStart && lockedVertexEnd == mNumVerts)
      //   glBufferData(GL_ARRAY_BUFFER, mNumVerts * mVertexSize, NULL, GFXGLBufferType[mBufferType]); // orphan the buffer

      //glBufferSubData(GL_ARRAY_BUFFER, offset, length, mFrameAllocator.getlockedPtr() + offset );

      //mFrameAllocator.unlock();
		vkUnmapMemory( GFXVulkan->getLogicalDevice(), vertex_buffer_memory_ );
   }

   lockedVertexStart = 0;
	lockedVertexEnd   = 0;
   lockedVertexPtr = nullptr;
}

void GFXVulkanVertexBuffer::prepare()
{
//      glBindVertexBuffer( stream, mBuffer, mBufferOffset, mVertexSize );
			//VkDeviceSize offsets[1] = { 0 };
			//vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &vertexBuffer.buffer, offsets);
			//vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void GFXVulkanVertexBuffer::zombify()
{
   AssertFatal(lockedVertexStart == 0 && lockedVertexEnd == 0, "GFXVulkanVertexBuffer::zombify - Cannot zombify a locked buffer!");

   if(mBufferType == GFXBufferTypeDynamic)
   {
   //   SAFE_RELEASE(vb);
   }
}

void GFXVulkanVertexBuffer::resurrect()
{

	if(mBufferType == GFXBufferTypeDynamic)
   {
		//Vulkan_BUFFER_DESC desc;
		//desc.ByteWidth = mVertexSize * mNumVerts;
		//desc.Usage = Vulkan_USAGE_DYNAMIC;
		//desc.BindFlags = Vulkan_BIND_VERTEX_BUFFER;
		//desc.CPUAccessFlags = Vulkan_CPU_ACCESS_WRITE;
		//desc.MiscFlags = 0;
		//desc.StructureByteStride = 0;

		//HRESULT hr = VulkanDEVICE->CreateBuffer(&desc, NULL, &vb);

  //    if(FAILED(hr)) 
  //    {
		//   AssertFatal(false, "GFXVulkanVertexBuffer::resurrect - Failed to allocate VB");
  //    }
   }
}

