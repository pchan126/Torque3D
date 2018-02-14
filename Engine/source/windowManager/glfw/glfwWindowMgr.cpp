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

#include "windowManager/glfw/glfwWindowMgr.h"
#include "gfx/gfxDevice.h"
#include "core/util/journal/process.h"
#include "core/strings/unicode.h"
#include "gfx/bitmap/gBitmap.h"

#include "glfw/glfw3.h"

// ------------------------------------------------------------------------

void GLFW_CloseSplashWindow(void* hinst);

#ifdef TORQUE_GLFW

PlatformWindowManager * CreatePlatformWindowManager()
{
   return new PlatformWindowManagerGLFW();
}

#endif

// ------------------------------------------------------------------------

PlatformWindowManagerGLFW::PlatformWindowManagerGLFW()
{
   // Register in the process list.
   mOnProcessSignalSlot.setDelegate( this, &PlatformWindowManagerGLFW::_process );
   Process::notify( mOnProcessSignalSlot, PROCESS_INPUT_ORDER );

   // Init our list of allocated windows.
   mWindowListHead = NULL;

   // By default, we have no parent window.
   mParentWindow = NULL;

   mCurtainWindow = NULL;

   mDisplayWindow = true;
   mOffscreenRender = false;

   mInputState = KeyboardInputState::NONE;

   buildMonitorsList();
}

PlatformWindowManagerGLFW::~PlatformWindowManagerGLFW()
{
   // Kill all our windows first.
   while(mWindowListHead)
      // The destructors update the list, so this works just fine.
      delete mWindowListHead;

   glfwTerminate();
}

RectI PlatformWindowManagerGLFW::getPrimaryDesktopArea()
{
   // TODO GLFW
   AssertFatal(0, "");
   return RectI(0,0,0,0);
}

Point2I PlatformWindowManagerGLFW::getDesktopResolution()
{
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	
    // Return Resolution
    return Point2I(mode->width, mode->height);
}

S32 PlatformWindowManagerGLFW::getDesktopBitDepth()
{
    // Return Bits per Pixel
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    return mode->blueBits;
}

void PlatformWindowManagerGLFW::buildMonitorsList()
{
   // TODO GLFW
}

S32 PlatformWindowManagerGLFW::findFirstMatchingMonitor(const char* name)
{
   /// TODO GLFW
   AssertFatal(0, "");

   return 0;
}

U32 PlatformWindowManagerGLFW::getMonitorCount()
{
   // TODO GLFW
   AssertFatal(0, "");
   return 1;
}

const char* PlatformWindowManagerGLFW::getMonitorName(U32 index)
{
   int count;
   auto monitors = glfwGetMonitors(&count);
   if (index < count)
	   return glfwGetMonitorName(monitors[index]);

   AssertFatal(0, "index out of range");
   return "Monitor";
}

RectI PlatformWindowManagerGLFW::getMonitorRect(U32 index)
{
   // TODO GLFW
   AssertFatal(0, "");

   return RectI(0, 0, 0,0 );
}

void PlatformWindowManagerGLFW::getMonitorRegions(Vector<RectI> &regions)
{
   // TODO GLFW
   AssertFatal(0, "");
}

void PlatformWindowManagerGLFW::getWindows(VectorPtr<PlatformWindow*> &windows)
{
   PlatformWindowGLFW *win = mWindowListHead;
   while(win)
   {
      windows.push_back(win);
      win = win->mNextWindow;
   }
}

PlatformWindow *PlatformWindowManagerGLFW::createWindow(GFXDevice *device, const GFXVideoMode &mode)
{
   // Do the allocation.
   PlatformWindowGLFW *window = new PlatformWindowGLFW();   

   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   window->mWindowHandle = glfwCreateWindow(mode.resolution.x, mode.resolution.y, "", NULL, NULL);
   U32 id = 0;
   while (getWindowById(id) != nullptr)
	   id++;
   window->mWindowId = id;
   window->mOwningManager = this;
   mWindowMap[ window->mWindowId ] = window;

   //if (GFX->getAdapterType() == OpenGL)

   //else if (GFX->getAdapterType == Vulkan)
	   
	//Now, fetch our window icon, if any
   Torque::Path iconPath = Torque::Path(Con::getVariable( "$Core::windowIcon" ));

   if (iconPath.getExtension() == String("bmp"))
   {
      Con::errorf("Unable to use bmp format images for the window icon. Please use a different format.");
   }
   else
   {
      Resource<GBitmap> img = GBitmap::load(iconPath);
	  if (img != NULL)
	  {
		  GLFWimage images[1];
		  images[0].pixels = (unsigned char*)img->getBits();
		  images[0].height = img->getHeight();
		  images[0].width = img->getWidth();
		  glfwSetWindowIcon(window->mWindowHandle, 1, images);
	  }
   }

   if (device)
   {
      window->mDevice = device;
      window->mTarget = device->allocWindowTarget(window);
      AssertISV(window->mTarget, "PlatformWindowManagerGLFW::createWindow - failed to get a window target back from the device.");
   }
   else
   {
      Con::warnf("PlatformWindowManagerGLFW::createWindow - created a window with no device!");
   }

//   //Set it up for drag-n-drop events 
//#ifdef TORQUE_TOOLS
//   GLFW_EventState(GLFW_DROPBEGIN, GLFW_ENABLE);
//   GLFW_EventState(GLFW_DROPFILE, GLFW_ENABLE);
//   GLFW_EventState(GLFW_DROPCOMPLETE, GLFW_ENABLE);
//#endif

   linkWindow(window);

   return window;
}


void PlatformWindowManagerGLFW::setParentWindow(void* newParent)
{
   
}

void* PlatformWindowManagerGLFW::getParentWindow()
{
   return NULL;
}

void PlatformWindowManagerGLFW::_process()
{
	//while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	//}
	//GLFW_Event evt;
   //while( GLFW_PollEvent(&evt) )
   //{      
   //   switch(evt.type)
   //   {
   //       case GLFW_QUIT:
   //       {
   //          PlatformWindowGLFW *window = static_cast<PlatformWindowGLFW*>( getFirstWindow() );
   //          if(window)
   //            window->appEvent.trigger( window->getWindowId(), WindowClose );
   //          break;
   //       }

   //      case GLFW_KEYDOWN:
   //      case GLFW_KEYUP:
   //      {
   //         PlatformWindowGLFW *window = mWindowMap[evt.key.windowID];
   //         if(window)
   //            window->_processGLFWEvent(evt);
   //         break;
   //      }

   //      case GLFW_MOUSEWHEEL:
   //      {
   //         PlatformWindowGLFW *window = mWindowMap[evt.wheel.windowID];
   //         if (window)
   //            window->_processGLFWEvent(evt);
   //         break;
   //      }

   //      case GLFW_MOUSEMOTION:
   //      {
   //         PlatformWindowGLFW *window = mWindowMap[evt.motion.windowID];
   //         if(window)
   //            window->_processGLFWEvent(evt);
   //         break;
   //      }

   //      case GLFW_MOUSEBUTTONDOWN:
   //      case GLFW_MOUSEBUTTONUP:
   //      {
   //         PlatformWindowGLFW *window = mWindowMap[evt.button.windowID];
   //         if(window)
   //            window->_processGLFWEvent(evt);
   //         break;
   //      }

   //      case GLFW_TEXTINPUT:
   //      {
   //         PlatformWindowGLFW *window = mWindowMap[evt.text.windowID];
   //         if(window)
   //            window->_processGLFWEvent(evt);
   //         break;
   //      }

   //      case GLFW_WINDOWEVENT:
   //      {
   //         PlatformWindowGLFW *window = mWindowMap[evt.window.windowID];
   //         if(window)
   //            window->_processGLFWEvent(evt);
   //         break;
   //      }

   //      case(GLFW_DROPBEGIN):
   //      {
   //         if (!Con::isFunction("onDropBegin"))
   //            break;

   //         Con::executef("onDropBegin");
   //      }

   //      case (GLFW_DROPFILE):
   //      {
   //         // In case if dropped file
   //         if (!Con::isFunction("onDropFile"))
   //            break;

   //         char* fileName = evt.drop.file;

   //         if (!Platform::isFile(fileName))
   //            break;

   //         Con::executef("onDropFile", StringTable->insert(fileName));

   //         GLFW_free(fileName);    // Free dropped_filedir memory
   //         break;
   //      }

   //      case(GLFW_DROPCOMPLETE):
   //      {
   //         if (!Con::isFunction("onDropEnd"))
   //            break;

   //         Con::executef("onDropEnd");
   //      }

   //      default:
   //      {
   //         //Con::printf("Event: %d", evt.type);
   //      }
   //   }
   //}

   //// After the event loop is processed, we can now see if we have to notify
   //// GLFW that we want text based events. This fixes a bug where text based
   //// events would be generated while key presses would still be happening.
   //// See KeyboardInputState for further documentation.
   //if (mInputState != KeyboardInputState::NONE)
   //{
   //   // Update text mode toggling.
   //   if (mInputState == KeyboardInputState::TEXT_INPUT)
   //      GLFW_StartTextInput();
   //   else
   //      GLFW_StopTextInput();

   //   // Done until we need to update it again.
   //   mInputState = KeyboardInputState::NONE;
   //}
}

PlatformWindow * PlatformWindowManagerGLFW::getWindowById( WindowId id )
{
   // Walk the list and find the matching id, if any.
   PlatformWindowGLFW *win = mWindowListHead;
   while(win)
   {
      if(win->getWindowId() == id)
         return win;

      win = win->mNextWindow;
   }

   return nullptr; 
}

PlatformWindow * PlatformWindowManagerGLFW::getFirstWindow()
{
   return mWindowListHead != NULL ? mWindowListHead : NULL;
}

PlatformWindow* PlatformWindowManagerGLFW::getFocusedWindow()
{
   PlatformWindowGLFW* window = mWindowListHead;
   while( window )
   {
      if( window->isFocused() )
         return window;

      window = window->mNextWindow;
   }

   return NULL;
}

PlatformWindowGLFW * PlatformWindowManagerGLFW::getWindowByHandle(GLFWwindow * handle)
{
	// Walk the list and find the matching id, if any.
	PlatformWindowGLFW *win = mWindowListHead;
	while (win)
	{
		if (win->mWindowHandle == handle)
			return win;

		win = win->mNextWindow;
	}

	return nullptr;
}

void PlatformWindowManagerGLFW::linkWindow( PlatformWindowGLFW *w )
{
   w->mNextWindow = mWindowListHead;
   mWindowListHead = w;
}

void PlatformWindowManagerGLFW::unlinkWindow( PlatformWindowGLFW *w )
{
   PlatformWindowGLFW **walk = &mWindowListHead;
   while(*walk)
   {
      if(*walk != w)
      {
         // Advance to next item in list.
         walk = &(*walk)->mNextWindow;
         continue;
      }

      // Got a match - unlink and return.
      *walk = (*walk)->mNextWindow;
      return;
   }
}

void PlatformWindowManagerGLFW::_processCmdLineArgs( const S32 argc, const char **argv )
{
   // TODO GLFW
}

void PlatformWindowManagerGLFW::lowerCurtain()
{
   if(mCurtainWindow)
      return;

   // TODO GLFW
}

void PlatformWindowManagerGLFW::raiseCurtain()
{
   if(!mCurtainWindow)
      return;

   // TODO GLFW
}

void PlatformWindowManagerGLFW::updateGLFWTextInputState(KeyboardInputState state)
{
   // Force update state. This will respond at the end of the event loop.
   mInputState = state;
}


void PlatformWindowManagerGLFW::key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	getWindowByHandle(window)->_triggerKeyNotify(key, scancode, action, mods);
}

void PlatformWindowManagerGLFW::character_callback(GLFWwindow * window, unsigned int codepoint)
{
	getWindowByHandle(window)->_triggerTextNotify(codepoint);
}

void PlatformWindowManagerGLFW::cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
{
}

#ifdef TORQUE_GLFW

void Platform::openFolder(const char* path)
{
	AssertFatal(0, "Not Implemented");
}

void Platform::openFile(const char* path)
{
	AssertFatal(0, "Not Implemented");
}

#endif

//------------------------------------------------------------------------------

//namespace GL
//{
//   void gglPerformBinds();
//}

//void InitWindowingSystem()
//{
//
//}

AFTER_MODULE_INIT(gfx)
{
   //int res = GLFW_Init(GLFW_INIT_VIDEO | GLFW_INIT_JOYSTICK | GLFW_INIT_HAPTIC | GLFW_INIT_GAMECONTROLLER | GLFW_INIT_EVENTS | GLFW_INIT_NOPARACHUTE);
   //AssertFatal(res != -1, avar("GLFW error:%s", GLFW_GetError()));

   //// By default, GLFW enables text input. We disable it on initialization, and
   //// we will enable it whenever the time is right.
   //GLFW_StopTextInput();
}
