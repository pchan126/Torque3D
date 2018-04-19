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

#ifndef _GFXPipelinePROFILE_H_
#define _GFXPipelinePROFILE_H_

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

class GFXPipelineObject;


class GFXPipelineProfile
{
public:

   GFXPipelineProfile(const String &name );
   // Equality operators
   inline bool operator==(const GFXPipelineProfile &in_Cmp) const { return (mName == in_Cmp.mName && mProfile == in_Cmp.mProfile); }
   inline bool operator!=(const GFXPipelineProfile &in_Cmp) const { return !(*this == in_Cmp); }

   // Accessors
   String getName() const { return mName; };

   // Mutators
   const U32 getDownscale() const { return mDownscale; }
   void setDownscale(const U32 shift) { mDownscale = shift; }
   //void incActiveCopies() { mStats.activeCount++; }
   //void decActiveCopies() { AssertFatal( mStats.activeCount != 0, "Ran out of extant copies!"); mStats.activeCount--; }   

   // And static interface...
   static void init();
   static GFXPipelineProfile *find(const String &name);
   static void updateStatsForCreation(GFXPipelineObject *t);
   static void updateStatsForDeletion(GFXPipelineObject *t);

   /// Collects the total stats for all the profiles which
   /// include any of the flag bits.
   //static void collectStats( Flags flags, GFXPipelineProfileStats *stats );

   /// Returns the total profile count in the list.
   static U32 getProfileCount() { return smProfileCount; }

   /// Returns the head of the profile list.
   static GFXPipelineProfile* getHead() { return smHead; }

   /// Returns the next profile in the list.
   GFXPipelineProfile* getNext() const { return mNext; }


   // Helper functions...
   //inline bool doStoreBitmap() const { return testFlag(KeepBitmap); }
   //inline bool canDownscale() const { return !testFlag(PreserveSize); }
   //inline bool isDynamic() const { return testFlag(Dynamic); }
   //inline bool isRenderTarget() const { return testFlag(RenderTarget); }
   //inline bool isZTarget() const { return testFlag(ZTarget); }
   //inline bool isSystemMemory() const { return testFlag(SystemMemory); }
   //inline bool noMip() const { return testFlag(NoMipmap); }
   //inline bool isPooled() const { return testFlag(Pooled); }
   //inline bool canDiscard() const { return !testFlag(NoDiscard); }
   //inline bool isSRGB() const { return testFlag(SRGB); }
   //compare profile flags for equality
   inline bool compareFlags(const GFXPipelineProfile& in_Cmp) const{ return (mProfile == in_Cmp.mProfile); }
private:
   /// These constants control the packing for the profile; if you add flags, types, or
   /// compression info then make sure these are giving enough bits!
   enum Constants
   {
      TypeBits = 2,
      FlagBits = 12,
      CompressionBits = 3,
   };

   String    mName;        ///< Name of this profile...
   U32 mDownscale;         ///< Amount to shift Pipelines of this type down, if any.
   U32 mProfile;           ///< Stores a munged version of the profile data.
   U32 mActiveCount;       ///< Count of Pipelines of this profile type allocated.
   U32 mActiveTexels;      ///< Amount of texelspace currently allocated under this profile.
   U32 mActiveBytes;       ///< Amount of storage currently allocated under this profile.
   U32 mAllocatedPipelines; ///< Total number of Pipelines allocated under this profile.
   U32 mAllocatedTexels;   ///< Total number of texels allocated under this profile.
   U32 mAllocatedBytes;    ///< Total number of bytes allocated under this profile.

   
   /// The number of profiles in the system.
   static U32 smProfileCount;

   /// Keep a list of all the profiles.
   GFXPipelineProfile *mNext;
   static GFXPipelineProfile *smHead;
};

#define GFX_DeclarePipelineProfile(name)  extern GFXPipelineProfile name
#define GFX_ImplementPipelineProfile(name, type,  flags, compression) GFXPipelineProfile name(#name, type, flags, compression)

// Default Pipeline profiles
// Pipeline we can render to.
//GFX_DeclarePipelineProfile(GFXRenderTargetProfile);
//GFX_DeclarePipelineProfile(GFXRenderTargetSRGBProfile);
//// Standard static diffuse Pipelines
//GFX_DeclarePipelineProfile(GFXStaticPipelineProfile);
//GFX_DeclarePipelineProfile(GFXStaticPipelineSRGBProfile);
//// Standard static diffuse Pipelines that are persistent in memory
//GFX_DeclarePipelineProfile(GFXPipelinePersistentProfile);
//GFX_DeclarePipelineProfile(GFXPipelinePersistentSRGBProfile);
//// Pipeline that resides in system memory - used to copy data to
//GFX_DeclarePipelineProfile(GFXSystemMemPipelineProfile);
//// normal map profiles
//GFX_DeclarePipelineProfile(GFXNormalMapProfile);
//GFX_DeclarePipelineProfile(GFXNormalMapBC3Profile);
//GFX_DeclarePipelineProfile(GFXNormalMapBC5Profile);
//// Depth buffer Pipeline
//GFX_DeclarePipelineProfile(GFXZTargetProfile);
//// Dynamic Texure
//GFX_DeclarePipelineProfile(GFXDynamicPipelineProfile);
//GFX_DeclarePipelineProfile(GFXDynamicPipelineSRGBProfile);

#endif
