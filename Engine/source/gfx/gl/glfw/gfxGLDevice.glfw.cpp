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
#if defined( TORQUE_GLFW ) && !defined( TORQUE_DEDICATED )

#include "gfx/gfxCubemap.h"
#include "gfx/screenshot.h"

#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#include "gfx/gl/gfxGLVertexBuffer.h"
#include "gfx/gl/gfxGLPrimitiveBuffer.h"
#include "gfx/gl/gfxGLTextureTarget.h"
#include "gfx/gl/gfxGLWindowTarget.h"
#include "gfx/gl/gfxGLTextureManager.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "gfx/gl/gfxGLCubemap.h"
#include "gfx/gl/gfxGLCardProfiler.h"

#include "windowManager/glfw/glfwWindow.h"
#include "platform/platformGL.h"
#include "GLFW/glfw3.h"

extern void loadGLCore();
extern void loadGLExtensions(void* context);

namespace GLFWDevice {
	void EnumerateVideoModes(Vector<GFXVideoMode>& outModes)
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		int count;
		auto modes = glfwGetVideoModes(monitor, &count);

		if (count < 0)
		{
			AssertFatal(0, "");
			return;
		}

		for (int i = 0; i < count; ++i)
		{
			auto mode = modes[i];

			GFXVideoMode outMode;
			outMode.resolution.set(mode.width, mode.height);
			outMode.refreshRate = mode.refreshRate;
			outMode.bitDepth = mode.redBits;
			outMode.wideScreen = (mode.width / mode.height) > (4 / 3);
			outMode.fullScreen = true;

			outModes.push_back(outMode);
		}
	}
}

void GFXGLDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
	/* Initialize the library */
	if (!glfwInit())
		return;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

	GFXAdapter *toAdd = new GFXAdapter;
	toAdd->mIndex = 0;

	dStrcpy(toAdd->mName, "GFX Vulkan");

	toAdd->mType = Vulkan;
	toAdd->mShaderModel = 0.f;
	toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;

	// Enumerate all available resolutions:
	GLFWDevice::EnumerateVideoModes(toAdd->mAvailableModes);

	// Add to the list of available adapters.
	adapterList.push_back(toAdd);
}

void GFXGLDevice::enumerateVideoModes() 
{
    mVideoModes.clear();
	GLFWDevice::EnumerateVideoModes(mVideoModes);
}

void GFXGLDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    AssertFatal(window, "GFXGLDevice::init - no window specified, can't init device without a window!");
    PlatformWindowGLFW* glfwWindow = dynamic_cast<PlatformWindowGLFW*>(window);
    AssertFatal(glfwWindow, "Window is not a valid PlatformWindowGLFW object");

    // Create OpenGL context
    PlatformGL::MakeCurrentGL( glfwWindow, nullptr );
        
    loadGLCore();
    loadGLExtensions(mContext);
    
    // It is very important that extensions be loaded before we call initGLState()
    initGLState();
    
    mProjectionMatrix.identity();
    
    mInitialized = true;
    deviceInited();
}

bool GFXGLDevice::beginSceneInternal() 
{
   mCanCurrentlyRender = true;
   return true;
}

U32 GFXGLDevice::getTotalVideoMemory()
{
   return getTotalVideoMemory_GL_EXT();
}

//------------------------------------------------------------------------------

GFXWindowTarget *GFXGLDevice::allocWindowTarget( PlatformWindow *window )
{
    AssertFatal(!mContext, "This GFXGLDevice is already assigned to a window");
    
    GFXGLWindowTarget* ggwt = 0;
    if( !mContext )
    {
        // no context, init the device now
        init(window->getVideoMode(), window);
        ggwt = new GFXGLWindowTarget(window, this);
        ggwt->registerResourceWithDevice(this);
        ggwt->mContext = mContext;
    }

    return ggwt;
}

GFXFence* GFXGLDevice::_createPlatformSpecificFence()
{
    return NULL;
}


//-----------------------------------------------------------------------------

void GFXGLWindowTarget::_WindowPresent()
{   
//   GLFW_GL_SwapWindow( static_cast<PlatformWindowGLFW*>( getWindow() )->getGLFWWindow() );
}

void GFXGLWindowTarget::_teardownCurrentMode()
{

}

void GFXGLWindowTarget::_setupNewMode()
{
}

void GFXGLWindowTarget::_makeContextCurrent()
{
   PlatformGL::MakeCurrentGL(mWindow, mContext);
}

#endif
