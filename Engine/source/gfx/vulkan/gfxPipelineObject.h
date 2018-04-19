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

#ifndef _GFXPipelineOBJECT_H_
#define _GFXPipelineOBJECT_H_

#ifndef _REFBASE_H_
#include "core/util/refBase.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif
#ifndef _GFXRESOURCE_H_
#include "gfx/gfxResource.h"
#endif
#include "gfxVulkanDevice.h"


class GFXPipelineObject : public StrongRefBase, public GFXResource
{
public:
   /// The path to the Pipeline file if the
   /// content was loaded from a resource.
   String mPath;

   bool mDead;

   /// The device this Pipeline belongs to.
   GFXVulkanDevice *mDevice;   

   /// The next Pipeline in the linked list.
   /// @see GFXPipelineManager::mListHead
   GFXPipelineObject *mNext;

   /// The previous Pipeline in the linked list.
   /// @see GFXPipelineManager::mListHead
   GFXPipelineObject *mPrev;

   /// The siblings in the cache hash table.
   /// @see GFXPipelineManager::mHashTable
   GFXPipelineObject *mHashNext;

   /// This is the file name or other unique string used 
   /// to hash this Pipeline object.
   String mPipelineLookupName;

   /// The time at which all references to this
   /// Pipeline were removed.
   U32 mDeleteTime;


   GFXPipelineObject(GFXDevice * aDevice);
   virtual ~GFXPipelineObject();

	VkPipeline pipeline;

   // GFXResource
   void zombify() override {};
   void resurrect() override {};

	//#ifdef TORQUE_DEBUG

   //// It is important for any derived objects to define this method
   //// and also call 'kill' from their destructors.  If you fail to
   //// do either, you will get a pure virtual function call crash
   //// in debug mode.  This is a precaution to make sure you don't
   //// forget to add 'kill' to your destructor.
   //virtual void pureVirtualCrash() = 0;

   //#endif

   virtual void kill();

   ///// Debug helper function for writing the Pipeline to disk.
   //bool dumpToDisk( const String &bmType, const String &path );

   // GFXResource interface
   /// The resource should put a description of itself (number of vertices, size/width of Pipeline, etc.) in buffer
	const String describeSelf() const override;

   // StrongRefBase
	void destroySelf() override;
};


#endif // _GFXPipelineOBJECT_H_
