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

#ifndef _GFXPipelineMANAGER_H_
#define _GFXPipelineMANAGER_H_
#include "core/util/tVector.h"
#include "gui/containers/guiTabBookCtrl.h"
#include "gfxPipelineProfile.h"
#include <vulkan/vulkan.h>

namespace Torque
{
   class Path;
}

class GFXPipelineObject;

class GFXPipelineManager 
{   
public:
   enum
   {
      AA_MATCH_BACKBUFFER = -1
   };

   GFXPipelineManager();
   virtual ~GFXPipelineManager();

   /// Set up some global script interface stuff.
   void init();

   ///
   //static U32 getPipelineDownscalePower( GFXPipelineProfile *profile );

   virtual GFXPipelineObject *createPipeline(GFXPrimitiveType primType, GFXShader* shader);

   void deletePipeline( GFXPipelineObject *Pipeline );
   void reloadPipeline( GFXPipelineObject *Pipeline );

   /// Request that the Pipeline be deleted which will
   /// either occur immediately or delayed if its cached.
   void requestDeletePipeline( GFXPipelineObject *Pipeline );

   /// @name Pipeline Necromancy
   /// 
   /// Pipeline necromancy in three easy steps:
   /// - If you want to destroy the Pipeline manager, call kill().
   /// - If you want to switch resolutions, or otherwise reset the device, call zombify().
   /// - When you want to bring the manager back from zombie state, call resurrect().
   /// @{

   ///
   void kill();
   void zombify();
   void resurrect();

   /// This releases any pooled Pipelines which are 
   /// currently unused freeing up video memory.
   void cleanupPool();

   ///
   void reloadPipelines();

   /// This releases cached Pipelines that have not
   /// been referenced for a period of time.
   void cleanupCache( U32 secondsToLive = 0 );

   /// Registers a callback for Pipeline zombify and resurrect events.
   /// @see GFXTexCallbackCode
   /// @see removeEventDelegate
   template <class T,class U>
   static void addEventDelegate( T obj, U func );

   /// Unregisteres a Pipeline event callback.
   /// @see addEventDelegate
   template <class T,class U>
   static void removeEventDelegate( T obj, U func ) { smEventSignal.remove( obj, func ); }

   /// @}

protected:

	bool CreatePipelineCacheObject(char const *cache_data = nullptr);

	VkPipelineCache pipeline_cache;


	GFXPipelineObject *mListHead;
   GFXPipelineObject *mListTail;

   // We have a hash table for fast Pipeline lookups
   GFXPipelineObject **mHashTable;
   U32                mHashCount;
   GFXPipelineObject *hashFind( const String &name );
   void              hashInsert(GFXPipelineObject *object);
   void              hashRemove(GFXPipelineObject *object);

   /// The Pipelines waiting to be deleted.
   Vector<GFXPipelineObject*> mToDelete;

   enum PipelineManagerState
   {
      Living,
      Zombie,
      Dead

   } mPipelineManagerState;

   /// The Pipeline pool collection type.
   typedef HashTable<GFXPipelineProfile*,StrongRefPtr<GFXPipelineObject> > PipelinePoolMap;

   /// All the allocated Pipeline pool Pipelines.
   PipelinePoolMap mPipelinePool;

   //-----------------------------------------------------------------------
   // Protected methods
   //-----------------------------------------------------------------------

   /// Returns a free Pipeline of the requested attributes from
   /// from the shared Pipeline pool.  It returns NULL if no match
   /// is found.
   GFXPipelineObject* _findPooledPipeline(   U32 width, 
                                          U32 height, 
                                          GFXFormat format, 
                                          GFXPipelineProfile *profile,
                                          U32 numMipLevels,
                                          S32 antialiasLevel );

   GFXPipelineObject *_createPipeline( const String &resourceName,
                                       GFXPipelineProfile *profile,
                                       GFXPipelineObject *inObj );

   GFXPipelineObject *_createPipeline(GFXPrimitiveType primType, GFXShader* shader);

   /// Frees the API handles to the Pipeline, for D3D this is a release call
   ///
   /// @note freePipeline MUST NOT DELETE THE Pipeline OBJECT
   virtual void freePipeline( GFXPipelineObject *Pipeline, bool zombify = false );

   virtual void refreshPipeline( GFXPipelineObject *Pipeline );

   /// @group Internal Pipeline Manager Interface
   ///
   /// These pure virtual functions are overloaded by each API-specific
   /// subclass.
   ///
   /// The order of calls is:
   /// @code
   /// _createPipeline()
   /// _loadPipeline
   /// _refreshPipeline()
   /// _refreshPipeline()
   /// _refreshPipeline()
   /// ...
   /// _freePipeline()
   /// @endcode
   ///
   /// @{

   /// Allocate a Pipeline with the internal API.
   ///
   /// @param  height   Height of the Pipeline.
   /// @param  width    Width of the Pipeline.
   /// @param  depth    Depth of the Pipeline. (Will normally be 1 unless
   ///                  we are doing a cubemap or volumePipeline.)
   /// @param  format   Pixel format of the Pipeline.
   /// @param  profile  Profile for the Pipeline.
   /// @param  numMipLevels   If not-NULL, then use that many mips.
   ///                        If NULL create the full mip chain
   /// @param  antialiasLevel, Use GFXPipelineManager::AA_MATCH_BACKBUFFER to match the backbuffer settings (for render targets that want to share
   ///                         the backbuffer z buffer.  0 for no antialiasing, > 0 for levels that match the GFXVideoMode struct.
   //virtual GFXPipelineObject *_createPipelineObject( U32 height, 
   //                                                U32 width, 
   //                                                U32 depth, 
   //                                                GFXFormat format, 
   //                                                GFXPipelineProfile *profile, 
   //                                                U32 numMipLevels, 
   //                                                bool forceMips = false, 
   //                                                S32 antialiasLevel = 0, 
   //                                                GFXPipelineObject *inTex = NULL ) = 0;

   ///// Load a Pipeline from a proper DDSFile instance.
   //virtual bool _loadPipeline(GFXPipelineObject *Pipeline, DDSFile *dds)=0;

   ///// Load data into a Pipeline from a GBitmap using the internal API.
   //virtual bool _loadPipeline(GFXPipelineObject *Pipeline, GBitmap *bmp)=0;

   ///// Load data into a Pipeline from a raw buffer using the internal API.
   /////
   ///// Note that the size of the buffer is assumed from the parameters used
   ///// for this GFXPipelineObject's _createPipeline call.
   //virtual bool _loadPipeline(GFXPipelineObject *Pipeline, void *raw)=0;

   ///// Refresh a Pipeline using the internal API.
   //virtual bool _refreshPipeline(GFXPipelineObject *Pipeline)=0;

   /// Free a Pipeline (but do not delete the GFXPipelineObject) using the internal
   /// API.
   ///
   /// This is only called during zombification for Pipelines which need it, so you
   /// don't need to do any internal safety checks.
   //virtual bool _freePipeline(GFXPipelineObject *Pipeline, bool zombify=false)=0;

   /// @}

   /// Store Pipeline into the hash table cache and linked list.
   void _linkPipeline( GFXPipelineObject *obj );

   /// Validate the parameters for creating a Pipeline.
   void _validateTexParams( const U32 width, const U32 height, const GFXPipelineProfile *profile, 
      U32 &inOutNumMips, GFXFormat &inOutFormat );

   // New Pipeline manager methods for the cleanup work:
   GFXPipelineObject *_lookupPipeline( const char *filename, const GFXPipelineProfile *profile  );

   void _onFileChanged( const Torque::Path &path );

   /// The Pipeline event signal type.
   typedef Signal<void(GFXTexCallbackCode code)> EventSignal;

   /// The Pipeline event signal.
   static EventSignal smEventSignal;
};


template <class T,class U>
inline void GFXPipelineManager::addEventDelegate( T obj, U func ) 
{
   EventSignal::DelegateSig d( obj, func );
   
   AssertFatal( !smEventSignal.contains( d ), 
      "GFXPipelineManager::addEventDelegate() - This is already registered!" );

   smEventSignal.notify( d ); 
}

inline GFXPipelineObject * GFXPipelineManager::createPipeline(GFXPrimitiveType primType, GFXShader* shader)
{
	return _createPipeline( primType, shader);
}

inline void GFXPipelineManager::reloadPipeline( GFXPipelineObject *Pipeline )
{
   refreshPipeline( Pipeline );
}

/// Returns the GFXPipelineManager singleton.  Should only be
/// called after the GFX device has been initialized.
//#define PIPEMGR GFXDevice::get()->getPipelineManager()

#endif // _GFXPipelineMANAGER_H_
