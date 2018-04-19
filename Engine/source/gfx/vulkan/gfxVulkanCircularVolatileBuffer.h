#ifndef Vulkan_CIRCULAR_VOLATILE_BUFFER_H
#define Vulkan_CIRCULAR_VOLATILE_BUFFER_H

#include "gfx/Vulkan/gfxVulkanDevice.h"
#include "gfx/Vulkan/gfxVulkanUtils.h"

class VulkanFenceRange 
{
public:
   VulkanFenceRange() : mStart(0), mEnd(0)
   {         
     
   }

   ~VulkanFenceRange()
   {
      //the order of creation/destruction of static variables is indetermined... depends on detail of the build
      //looks like for some reason on windows + sdl + openVulkan the order make invalid / wrong the process TODO: Refactor -LAR
      //AssertFatal( mSync == 0, "");
   }

   void init(U32 start, U32 end)
   {  
      PROFILE_SCOPE(GFXVulkanQueryFence_issue);
      mStart = start;
      mEnd = end;
      //mSync = VulkanFenceSync(Vulkan_SYNC_GPU_COMMANDS_COMPLETE, 0);
   }

   bool checkOverlap(U32 start, U32 end) 
   {         
      if( mStart < end && start < mEnd )
         return true;

      return false;
   }

   void wait()
   {   
      PROFILE_SCOPE(GFXVulkanQueryFence_block);
      //Vulkanbitfield waitFlags = 0;
      //Vulkanuint64 waitDuration = 0;
      while( 1 ) 
      {
         //Vulkanenum waitRet = VulkanClientWaitSync( mSync, waitFlags, waitDuration );
         //if( waitRet == Vulkan_ALREADY_SIGNALED || waitRet == Vulkan_CONDITION_SATISFIED ) 
         //{
         //   break;
         //}

         //if( waitRet == Vulkan_WAIT_FAILED ) 
         //{
         //   AssertFatal(0, "VulkanSync failed.");
         //   break;
         //}
         //
         //waitFlags = Vulkan_SYNC_FLUSH_COMMANDS_BIT;
         //waitDuration = scOneSecondInNanoSeconds;
      }     

      //VulkanDeleteSync(mSync);0
      //mSync = 0;
   }

   void swap( VulkanFenceRange &r )
   {
      VulkanFenceRange temp;
      temp = *this;
      *this = r;
      r = temp;
   }

protected:
   U32 mStart, mEnd;
   //Vulkansync mSync;
   //static const Vulkanuint64 scOneSecondInNanoSeconds = 1000000000;

   VulkanFenceRange( const VulkanFenceRange &);
   VulkanFenceRange& operator=(const VulkanFenceRange &r)
   {
      mStart = r.mStart;
      mEnd = r.mEnd;
      //mSync = r.mSync;
      return *this;
   }
};

class VulkanOrderedFenceRangeManager
{
public:

   ~VulkanOrderedFenceRangeManager( )
   {
      //the order of creation/destruction of static variables is indetermined... depends on detail of the build
      //looks like for some reason on windows + sdl + openVulkan the order make invalid / wrong the process TODO: Refactor -LAR
      //waitAllRanges( );
   }

   void protectOrderedRange( U32 start, U32 end )
   {
      mFenceRanges.increment();
      VulkanFenceRange &range = mFenceRanges.last();
      range.init( start, end );
   }

   void waitFirstRange( U32 start, U32 end )
   {
      if( !mFenceRanges.size() || !mFenceRanges[0].checkOverlap( start, end ) )
         return;
         
      mFenceRanges[0].wait();
      mFenceRanges.pop_front();
   }

   void waitOverlapRanges( U32 start, U32 end )
   {
      for( U32 i = 0; i < mFenceRanges.size(); ++i )
      {
         if( !mFenceRanges[i].checkOverlap( start, end ) )
            continue;
         
         mFenceRanges[i].wait();
         mFenceRanges.erase(i);
      }
   }

   void waitAllRanges()
   {
      for( int i = 0; i < mFenceRanges.size(); ++i )            
         mFenceRanges[i].wait();      

      mFenceRanges.clear();
   }

protected:
   Vector<VulkanFenceRange> mFenceRanges;
};

class VulkanCircularVolatileBuffer
{
public:
   VulkanCircularVolatileBuffer() 
      : mBufferSize(0), mBufferFreePos(0), mCurrectUsedRangeStart(0)
   { 
      init();
   }

   ~VulkanCircularVolatileBuffer()
   {
      //VulkanDeleteBuffers(1, &mBufferName);
   }

   void init()
   {
      //VulkanGenBuffers(1, &mBufferName);

      //PRESERVE_BUFFER( mBinding );
      //VulkanBindBuffer(mBinding, mBufferName);
     
      const U32 cSizeInMB = 10;
      mBufferSize = (cSizeInMB << 20);

   // Generate a buffer
   if (!GFXVulkan->createBuffer(mBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, mBufferName))
	   Con::errorf("failed to create vertex buffer");

   ////and allocate the needed memory
	if (!GFXVulkan->allocateAndBindMemoryObjectForBuffer(mBufferName, buffer_memory, static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)))
	   Con::errorf("failed to allocate memory for vertex buffer");

   	//if( GFXVulkan->mCapabilities.bufferStorage )
      //{      
      //   const Vulkanbitfield flags = Vulkan_MAP_WRITE_BIT | Vulkan_MAP_PERSISTENT_BIT | Vulkan_MAP_COHERENT_BIT;
      //   VulkanBufferStorage(mBinding, mBufferSize, NULL, flags);
      //   mBufferPtr = VulkanMapBufferRange(mBinding, 0, mBufferSize, flags);
      //}
      //else
      //{
      //   VulkanBufferData(mBinding, mBufferSize, NULL, Vulkan_DYNAMIC_DRAW);
      //}
   }

   struct 
   {
      U32 mOffset, mSize;
   }_getBufferData;

   void lock(const U32 size, U32 offsetAlign, U32 &outOffset, void* &outPtr)
   {
      if( !size )
      {
         AssertFatal(0, "");
         outOffset = 0;
         outPtr = NULL;
      }

      mLockManager.waitFirstRange( mBufferFreePos, (mBufferFreePos + size)-1 );

      if( mBufferFreePos + size > mBufferSize )
      {         
         mUsedRanges.push_back( UsedRange( mBufferFreePos, mBufferSize-1 ) );
         mBufferFreePos = 0;
      }

      // force offset buffer align
      if( offsetAlign )
         mBufferFreePos = ( (mBufferFreePos/offsetAlign) + 1 ) * offsetAlign;

      outOffset = mBufferFreePos;

      //if( GFXVulkan->mCapabilities.bufferStorage )
      //{         
      //   outPtr = (U8*)(mBufferPtr) + mBufferFreePos; 
      //}
      //else if( GFXVulkan->VulkanUseMap() )
      //{
      //   PRESERVE_BUFFER( mBinding );
      vkMapMemory(GFXVulkan->getLogicalDevice(), buffer_memory, outOffset, size, 0, &outPtr);

      //   const Vulkanbitfield access = Vulkan_MAP_WRITE_BIT | Vulkan_MAP_INVALIDATE_RANGE_BIT | Vulkan_MAP_UNSYNCHRONIZED_BIT;
      //   outPtr = VulkanMapBufferRange(mBinding, outOffset, size, access);
      //}
      //else
      //{
      //   _getBufferData.mOffset = outOffset;
      //   _getBufferData.mSize = size;

      //   outPtr = mFrameAllocator.lock( size );
      //}      

      //set new buffer pos
      mBufferFreePos = mBufferFreePos + size;

      //align 4bytes
      mBufferFreePos = ( (mBufferFreePos/4) + 1 ) * 4;
   }

   void unlock()
   {
      //if( GFXVulkan->mCapabilities.bufferStorage )
      //{
      //   return;
      //}
      //else if( GFXVulkan->VulkanUseMap() )
      //{
      //   PRESERVE_BUFFER( mBinding );
      //   VulkanBindBuffer(mBinding, mBufferName);

      //   VulkanUnmapBuffer(mBinding);
	   vkUnmapMemory(GFXVulkan->getLogicalDevice(), buffer_memory);
      //}
      //else
      //{
      //   PRESERVE_BUFFER( mBinding );
      //   VulkanBindBuffer(mBinding, mBufferName);

      //   VulkanBufferSubData( mBinding, _getBufferData.mOffset, _getBufferData.mSize, mFrameAllocator.getlockedPtr() );

      //   _getBufferData.mOffset = 0;
      //   _getBufferData.mSize = 0;

      //   mFrameAllocator.unlock();
      //}
      
   }

   VkBuffer getHandle() const { return mBufferName; }

   void protectUsedRange()
   {
      for( int i = 0; i < mUsedRanges.size(); ++i )
      {
         mLockManager.protectOrderedRange( mUsedRanges[i].start, mUsedRanges[i].end );
      }
      mUsedRanges.clear();

      if( mCurrectUsedRangeStart < mBufferFreePos )
      {
         mLockManager.protectOrderedRange( mCurrectUsedRangeStart, mBufferFreePos-1 );      
         mCurrectUsedRangeStart = mBufferFreePos;
      }
   }

protected:   

   //Vulkanuint mBinding;
   VkBuffer mBufferName;
	VkDeviceMemory buffer_memory;
   void *mBufferPtr;
   U32 mBufferSize;
   U32 mBufferFreePos;
   U32 mCurrectUsedRangeStart;

   VulkanOrderedFenceRangeManager mLockManager;
   //FrameAllocatorLockableHelper mFrameAllocator;

   struct UsedRange
   {
      UsedRange(U32 _start = 0, U32 _end = 0)
         : start(_start), end(_end)
      {

      }
      U32 start, end;
   };
   Vector<UsedRange> mUsedRanges;
};


#endif
