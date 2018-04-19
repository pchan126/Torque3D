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

#ifndef _GFXVulkanCARDPROFILE_H
#define _GFXVulkanCARDPROFILE_H

#include "gfx/gfxCardProfile.h"
#include "gfxVulkanDevice.h"

class GFXVulkanCardProfiler : public GFXCardProfiler
{
public:
   void init();
   	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; };

protected:
   virtual const String& getRendererString() const override { return mRendererString; }
   virtual void setupCardCapabilities() override;
   virtual bool _queryCardCap(const String& query, U32& foundResult) override;
   virtual bool _queryFormat(const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips) override;

private:
   String mRendererString;
   typedef GFXCardProfiler Parent;
   VkPhysicalDeviceProperties physicalProperties;
	bool checkSwapChainSupport();

	VkPhysicalDevice physicalDevice;
};

#endif
