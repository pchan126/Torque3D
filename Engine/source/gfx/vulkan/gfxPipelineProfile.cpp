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
#include "gfx/Vulkan/gfxPipelineProfile.h"

#include "gfx/Vulkan/gfxPipelineObject.h"
#include "gfx/bitmap/gBitmap.h"
#include "core/strings/stringFunctions.h"
#include "console/console.h"
#include "console/engineAPI.h"


// Set up defaults...

//GFX_ImplementPipelineProfile(GFXRenderTargetProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::PreserveSize | GFXPipelineProfile::NoMipmap | GFXPipelineProfile::RenderTarget,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXRenderTargetSRGBProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::PreserveSize | GFXPipelineProfile::NoMipmap | GFXPipelineProfile::RenderTarget | GFXPipelineProfile::SRGB,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXStaticPipelineProfile, GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::Static,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXStaticPipelineSRGBProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::Static | GFXPipelineProfile::SRGB,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXPipelinePersistentProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::PreserveSize | GFXPipelineProfile::Static | GFXPipelineProfile::KeepBitmap,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXPipelinePersistentSRGBProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::PreserveSize | GFXPipelineProfile::Static | GFXPipelineProfile::KeepBitmap | GFXPipelineProfile::SRGB,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXSystemMemPipelineProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::PreserveSize | GFXPipelineProfile::NoMipmap | GFXPipelineProfile::SystemMemory,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXNormalMapProfile,
//                            GFXPipelineProfile::NormalMap,
//                            GFXPipelineProfile::Static,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXNormalMapBC3Profile,
//                            GFXPipelineProfile::NormalMap,
//                            GFXPipelineProfile::Static,
//                            GFXPipelineProfile::BC3);
//GFX_ImplementPipelineProfile(GFXNormalMapBC5Profile,
//                            GFXPipelineProfile::NormalMap,
//                            GFXPipelineProfile::Static,
//                            GFXPipelineProfile::BC5);
//GFX_ImplementPipelineProfile(GFXZTargetProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::PreserveSize | GFXPipelineProfile::NoMipmap | GFXPipelineProfile::ZTarget | GFXPipelineProfile::NoDiscard,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXDynamicPipelineProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::Dynamic,
//                            GFXPipelineProfile::NONE);
//GFX_ImplementPipelineProfile(GFXDynamicPipelineSRGBProfile,
//                            GFXPipelineProfile::DiffuseMap,
//                            GFXPipelineProfile::Dynamic | GFXPipelineProfile::SRGB,
//                            GFXPipelineProfile::NONE);

//-----------------------------------------------------------------------------

GFXPipelineProfile *GFXPipelineProfile::smHead = NULL;
U32 GFXPipelineProfile::smProfileCount = 0;

GFXPipelineProfile::GFXPipelineProfile(const String &name)
:  mName( name )
{
   //// Take type, flag, and compression and produce a munged profile word.
   //mProfile = (type & (BIT(TypeBits + 1) - 1)) |
   //          ((flag & (BIT(FlagBits + 1) - 1)) << TypeBits) | 
   //          ((compression & (BIT(CompressionBits + 1) - 1)) << (FlagBits + TypeBits));   

   // Stick us on the linked list.
   mNext = smHead;
   smHead = this;
   ++smProfileCount;

   mDownscale = 0;
}

void GFXPipelineProfile::init()
{
   // Do something, anything?
}

GFXPipelineProfile * GFXPipelineProfile::find(const String &name)
{
   // Not really necessary at this time.
   return NULL;
}

//void GFXPipelineProfile::collectStats( Flags flags, GFXPipelineProfileStats *stats )
//{
//   // Walk the profile list.
//   GFXPipelineProfile *curr = smHead;
//   while ( curr )
//   {
//      if ( curr->testFlag( flags ) )
//         (*stats) += curr->getStats();
//
//      curr = curr->mNext;
//   }
//}
//
//void GFXPipelineProfile::updateStatsForCreation(GFXPipelineObject *t)
//{
//   if(t->mProfile)
//   {
//      t->mProfile->incActiveCopies();
//      t->mProfile->mStats.allocatedPipelines++;
//      
//      U32 texSize = t->getHeight() * t->getWidth();
//      U32 byteSize = t->getEstimatedSizeInBytes();
//
//      t->mProfile->mStats.allocatedTexels += texSize;
//      t->mProfile->mStats.allocatedBytes  += byteSize;
//
//      t->mProfile->mStats.activeTexels += texSize;
//      t->mProfile->mStats.activeBytes += byteSize; 
//   }
//}
//
//void GFXPipelineProfile::updateStatsForDeletion(GFXPipelineObject *t)
//{
//   if(t->mProfile)
//   {
//      t->mProfile->decActiveCopies();
//      
//      U32 texSize = t->getHeight() * t->getWidth();
//      U32 byteSize = t->getEstimatedSizeInBytes();
//
//      t->mProfile->mStats.activeTexels -= texSize;
//      t->mProfile->mStats.activeBytes -= byteSize; 
//   }
//}
//
//DefineEngineFunction( getPipelineProfileStats, String, (),,
//   "Returns a list of Pipeline profiles in the format: ProfileName PipelineCount PipelineMB\n"
//   "@ingroup GFX\n" )
//{
//   StringBuilder result;
//
//   GFXPipelineProfile *profile = GFXPipelineProfile::getHead();
//   while ( profile )
//   {
//      const GFXPipelineProfileStats &stats = profile->getStats();
//
//      F32 mb = ( stats.activeBytes / 1024.0f ) / 1024.0f;
//
//      result.format( "%s %d %0.2f\n",
//         profile->getName().c_str(),
//         stats.activeCount,
//         mb );
//
//      profile = profile->getNext();
//   }
//
//   return result.end();
//}
//
