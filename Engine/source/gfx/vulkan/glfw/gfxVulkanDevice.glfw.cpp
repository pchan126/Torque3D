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
//#if defined( TORQUE_GLFW ) && !defined( TORQUE_DEDICATED )

#define GLFW_INCLUDE_VULKAN
//#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "gfx/gfxCubemap.h"
#include "gfx/screenshot.h"

#include "gfx/vulkan/gfxVulkanDevice.h"

#include "windowManager/glfw/glfwWindow.h"
#include "gfx/vulkan/gfxVulkanWindowTarget.h"
//#include "platform/platformGL.h"
#include <iostream>
#include "gfx/vulkan/gfxVulkanCardProfiler.h"

void GFXVulkanDevice::_EnumerateVideoModes(Vector<GFXVideoMode>& outModes)
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
		outMode.wideScreen = (mode.width / mode.height) >(4 / 3);
	    outMode.fullScreen = true;

		outModes.push_back(outMode);
	}
}

void GFXVulkanDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
	/* Initialize the library */
	if (!glfwInit())
		return;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (!glfwVulkanSupported())
		return;

	GFXAdapter *toAdd = new GFXAdapter;
	toAdd->mIndex = 0;

	dStrcpy(toAdd->mName, "GFX Vulkan");

	toAdd->mType = Vulkan;
	toAdd->mShaderModel = 0.f;
	toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;

	// Enumerate all available resolutions:
	_EnumerateVideoModes(toAdd->mAvailableModes);

	// Add to the list of available adapters.
	adapterList.push_back(toAdd);
}

void GFXVulkanDevice::enumerateVideoModes()
{
    mVideoModes.clear();
    _EnumerateVideoModes(mVideoModes);
}

void GFXVulkanDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    AssertFatal(window, "GFXVulkanDevice::init - no window specified, can't init device without a window!");
    //PlatformWindowSDL* sdlWindow = dynamic_cast<PlatformWindowSDL*>(window);
    //AssertFatal(sdlWindow, "Window is not a valid PlatformWindowSDL object");

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Torque 3D Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

//#ifdef TORQUE_DEBUG
//    std::vector enabledInstanceLayers;
//    std::vector enabledInstanceExtensions;
//#endif

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	desired_extensions.clear();
	for (U32 i = 0; i < glfwExtensionCount; i++)
		desired_extensions.push_back(glfwExtensions[i]);

	createInfo.enabledLayerCount = 0;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
	mContext = &instance;
	//

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	extensions.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    Con::printf("available extensions:");

	//std::cout << "available extensions:" << std::endl;

	for (const auto& extension : extensions) {
	    Con::printf("available extensions: %s", extension.extensionName);
		//std::cout << "\t" << extension.extensionName << std::endl;
	}

	initVulkanState();
    
    mProjectionMatrix.identity();
    
    mInitialized = true;
    deviceInited();
}

bool GFXVulkanDevice::beginSceneInternal()
{
   mCanCurrentlyRender = true;
   return true;
}

U32 GFXVulkanDevice::getTotalVideoMemory()
{
   //return getTotalVideoMemory_GL_EXT();
	return 0;
}

//------------------------------------------------------------------------------

GFXWindowTarget *GFXVulkanDevice::allocWindowTarget( PlatformWindow *window )
{
    AssertFatal(!mContext, "This GFXVulkanDevice is already assigned to a window");

	if (!mContext)
	{
		// no context, init the device now
		init(window->getVideoMode(), window);
	}

	PlatformWindowGLFW* temp = (PlatformWindowGLFW*)window;
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(instance, temp->getGLFWWindow(), NULL, &surface);
	if (err)
	{
		// Window surface creation failed
		Con::errorf("Window creation failed");
		return nullptr;
	}
	assignQueue(surface);
	GFXVulkanWindowTarget* ggwt = new GFXVulkanWindowTarget(window, this);
	ggwt->registerResourceWithDevice(this);
	ggwt->init(surface);
	allocateCommandBuffer(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ggwt->getSwapchainImageCount(), drawCmdBuffers);

	if (mWindowRT == nullptr)
		mWindowRT = new GFXWindowTargetRef(ggwt);

	return ggwt;
}

GFXFence* GFXVulkanDevice::_createPlatformSpecificFence()
{
    return nullptr;
}


//-----------------------------------------------------------------------------

//void GFXGLWindowTarget::_WindowPresent()
//{   
//   SDL_GL_SwapWindow( static_cast<PlatformWindowSDL*>( getWindow() )->getSDLWindow() );
//}

//void GFXGLWindowTarget::_teardownCurrentMode()
//{
//
//}

//void GFXGLWindowTarget::_setupNewMode()
//{
//}
//
//void GFXGLWindowTarget::_makeContextCurrent()
//{
//   PlatformGL::MakeCurrentGL(mWindow, mContext);
//}

//#endif
