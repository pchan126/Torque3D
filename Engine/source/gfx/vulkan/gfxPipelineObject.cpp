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
#include "gfx/Vulkan/gfxPipelineObject.h"

#include "gfx/gfxDevice.h"
#include "gfx/Vulkan/gfxPipelineManager.h"
#include "core/util/safeDelete.h"
#include "core/strings/stringFunctions.h"
#include "core/stream/fileStream.h"
#include "console/console.h"
#include "console/engineAPI.h"


// TODO: Change this to be in non-shipping builds maybe?
#ifdef TORQUE_DEBUG

//GFXPipelineObject *GFXPipelineObject::smHead = NULL;
//U32 GFXPipelineObject::smActiveTOCount = 0;


#endif // TORQUE_DEBUG

//-----------------------------------------------------------------------------
// GFXPipelineObject
//-----------------------------------------------------------------------------
GFXPipelineObject::GFXPipelineObject(GFXDevice *aDevice) 
{

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
GFXPipelineObject::~GFXPipelineObject() 
{
   kill();

}

void GFXPipelineObject::destroySelf()
{
}

//-----------------------------------------------------------------------------
// kill - this function clears out the data in Pipeline object.  It's done like
// this because the Pipeline object needs to release its pointers to Pipelines
// before the GFXDevice is shut down.  The Pipeline objects themselves get
// deleted by the refcount structure - which may be after the GFXDevice has
// been destroyed.
//-----------------------------------------------------------------------------
void GFXPipelineObject::kill()
{
   if( mDead )
      return;

   // If we're a dummy, don't do anything...
   if( !mDevice || !mDevice->mPipelineManager ) 
   {
      mDead = true;
      return;
   }

   // Remove ourselves from the Pipeline list and hash
   mDevice->mPipelineManager->deletePipeline(this);

   // Clean up linked list
   if(mNext)
      mNext->mPrev = mPrev;
   if(mPrev)
      mPrev->mNext = mNext;

   mDead = true;
}

const String GFXPipelineObject::describeSelf() const
{
	return "";
}

