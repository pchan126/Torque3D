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

#include "math/mMath.h"
#include "gfx/gfxStructs.h"

#include "windowManager/GLFW/GLFWWindow.h"
#include "windowManager/GLFW/GLFWWindowMgr.h"
#include "windowManager/GLFW/GLFWCursorController.h"
#include "platformGLFW/GLFWInput.h"
#include "platform/platformInput.h"

#include "gfx/gfxDevice.h"

#ifdef TORQUE_OS_LINUX
#define GLFW_VIDEO_DRIVER_X11  // TODO GLFW
#endif

#include "GLFW/glfw3.h"

#define SCREENSAVER_QUERY_DENY 0 // Disable screensaver

#ifndef IDI_ICON1 
#define IDI_ICON1 107
#endif

namespace 
{
   U32 getTorqueModFromGLFW(int mod)
   {
      U32 ret = 0;

      if (mod & GLFW_MOD_SHIFT)
      {
         ret |= SI_SHIFT;
      }

      if (mod & GLFW_MOD_ALT )
      {
         ret |= SI_ALT;
      }

      if (mod & GLFW_MOD_CONTROL)
      {
         ret |= SI_CTRL;
      }

	  if (mod & GLFW_MOD_SUPER)
	  {
		  // nothing, super key not handled
	  }

      return ret;
   }
}

PlatformWindowGLFW::PlatformWindowGLFW():
mOwningManager(NULL),
mNextWindow(NULL),
mWindowHandle(NULL),
mOldParent(NULL),
mDevice(NULL),
mTarget(NULL),
mPosition(0,0),
mMouseLocked(false),
mShouldLockMouse(false),
mSuppressReset(false),
mMenuHandle(NULL)
{
   mCursorController = new PlatformCursorControllerGLFW( this );

   mVideoMode.bitDepth = 32;
   mVideoMode.fullScreen = false;
   mVideoMode.refreshRate = 60;
   mVideoMode.resolution.set(800,600);
}

PlatformWindowGLFW::~PlatformWindowGLFW()
{
   // delete our GLFW handle..
	glfwDestroyWindow(mWindowHandle);

   // unlink ourselves from the window list...
   AssertFatal(mOwningManager, "PlatformWindowGLFW::~PlatformWindowGLFW - orphan window, cannot unlink!");
   mOwningManager->unlinkWindow(this);
}

GFXDevice * PlatformWindowGLFW::getGFXDevice()
{
   return mDevice;
}

GFXWindowTarget * PlatformWindowGLFW::getGFXTarget()
{
   return mTarget;
}

const GFXVideoMode & PlatformWindowGLFW::getVideoMode()
{
   return mVideoMode;
}

void* PlatformWindowGLFW::getSystemWindow(const WindowSystem system)
{
//     GLFW_SysWMinfo info;
//     GLFW_VERSION(&info.version);
//     GLFW_GetWindowWMInfo(mWindowHandle,&info);     
//
//#ifdef TORQUE_OS_WIN
//     if( system == WindowSystem_Windows && info.subsystem == GLFW_SYSWM_WINDOWS)
//        return info.info.win.window;
//#endif
//
//#if defined(TORQUE_OS_LINUX)
//     if( system == WindowSystem_X11 && info.subsystem == GLFW_SYSWM_X11)
//        return (void*)info.info.x11.window;
//#endif
//
//    AssertFatal(0, "");
    return nullptr;
}

void PlatformWindowGLFW::setVideoMode( const GFXVideoMode &mode )
{
   mVideoMode = mode;
   mSuppressReset = true;

   // Set our window to have the right style based on the mode
   if(mode.fullScreen && !Platform::getWebDeployment() && !mOffscreenRender)
   {     
      setSize(mode.resolution);

	  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

      // When switching to Fullscreen, reset device after setting style
      if(mTarget.isValid())
         mTarget->resetMode();
   }
   else
   {
      // Reset device *first*, so that when we call setSize() and let it
      // access the monitor settings, it won't end up with our fullscreen
      // geometry that is just about to change.

      if(mTarget.isValid())
         mTarget->resetMode();

      if (!mOffscreenRender)
      {
		  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
      }

      setSize(mode.resolution);
      centerWindow();
   }

   mSuppressReset = false;
}

bool PlatformWindowGLFW::clearFullscreen()
{
   return true;
}

bool PlatformWindowGLFW::isFullscreen()
{   
	return glfwGetWindowAttrib(mWindowHandle, GLFW_MAXIMIZED);
}

void PlatformWindowGLFW::_setFullscreen(const bool fullscreen)
{
   if( isFullscreen() )
      return;

   if(fullscreen && !mOffscreenRender)
   {
      Con::printf("PlatformWindowGLFW::setFullscreen (full) enter");
	  glfwMaximizeWindow( mWindowHandle );
   }
   else
   {
      Con::printf("PlatformWindowGLFW::setFullscreen (windowed) enter");
      if (!mOffscreenRender)
      {
		  glfwMaximizeWindow( mWindowHandle );
      }

      setSize(mVideoMode.resolution);

   }
   Con::printf("PlatformWindowGLFW::setFullscreen exit");   
}

bool PlatformWindowGLFW::setCaption( const char *cap )
{
	glfwSetWindowTitle(mWindowHandle, cap);
	caption = StringTable->insert(cap);
   return true;
}

const char * PlatformWindowGLFW::getCaption()
{
   return caption;
}

void PlatformWindowGLFW::setFocus()
{
   glfwFocusWindow(mWindowHandle);
}

void PlatformWindowGLFW::setClientExtent( const Point2I newExtent )
{
   Point2I oldExtent = getClientExtent();
   if (oldExtent == newExtent)
      return;   

   glfwSetWindowSize(mWindowHandle, newExtent.x, newExtent.y);
}

const Point2I PlatformWindowGLFW::getClientExtent()
{
   // Fetch Client Rect from Windows
   Point2I size;
   glfwGetWindowSize(mWindowHandle, &size.x, &size.y);

   return size;
}

void PlatformWindowGLFW::setBounds( const RectI &newBounds )
{
   // TODO GLFW
}

const RectI PlatformWindowGLFW::getBounds() const
{
   // TODO GLFW
   return RectI(0, 0, 0, 0);   
}

void PlatformWindowGLFW::setPosition(const Point2I newPosition)
{
	glfwSetWindowPos(mWindowHandle, newPosition.x, newPosition.y);
}

const Point2I PlatformWindowGLFW::getPosition()
{
   Point2I position;
   glfwGetWindowPos(mWindowHandle, &position.x, &position.y);
   // Return position
   return position;
}

Point2I PlatformWindowGLFW::clientToScreen( const Point2I& pos )
{
   Point2I position = getPosition();
   return pos + position;
}

Point2I PlatformWindowGLFW::screenToClient( const Point2I& pos )
{
	Point2I position = getPosition();
	return pos - position;
}

void PlatformWindowGLFW::centerWindow()
{
	auto size = getClientExtent();

	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);
   
   U32 posX = (mode->width/2) - (size.x/2);
   U32 posY = (mode->height/2) - (size.y/2);

   glfwSetWindowPos( mWindowHandle, posX, posY);
}

bool PlatformWindowGLFW::setSize( const Point2I &newSize )
{
   glfwSetWindowSize(mWindowHandle, newSize.x, newSize.y);

   // Let GFX get an update about the new resolution
   if (mTarget.isValid())
      mTarget->resetMode();

   return true;
}

bool PlatformWindowGLFW::isOpen()
{
   return mWindowHandle;
}

bool PlatformWindowGLFW::isVisible()
{
   // Is the window open and visible, ie. not minimized?
   if(!mWindowHandle)
      return false;

   if (mOffscreenRender)
      return true;

   int visible = glfwGetWindowAttrib(mWindowHandle, GLFW_VISIBLE);

   return (bool)visible;
}

bool PlatformWindowGLFW::isFocused()
{
   if (mOffscreenRender)
      return true;

   int focused = glfwGetWindowAttrib(mWindowHandle, GLFW_FOCUSED);
   return (bool)focused;
}

bool PlatformWindowGLFW::isMinimized()
{
   if (mOffscreenRender)
      return false;

   int iconified = glfwGetWindowAttrib(mWindowHandle, GLFW_ICONIFIED);
    return (bool)iconified;
}

bool PlatformWindowGLFW::isMaximized()
{
   if (mOffscreenRender)
      return true;

   int maximized = glfwGetWindowAttrib(mWindowHandle, GLFW_MAXIMIZED);
    return (bool)maximized;
}

WindowId PlatformWindowGLFW::getWindowId()
{
   return mWindowId;
}

void PlatformWindowGLFW::minimize()
{
   if (mOffscreenRender)
      return;

   glfwIconifyWindow(mWindowHandle);
}

void PlatformWindowGLFW::maximize()
{
   if (mOffscreenRender)
      return;

   glfwMaximizeWindow(mWindowHandle);
}

void PlatformWindowGLFW::restore()
{
   if (mOffscreenRender)
      return;

   glfwRestoreWindow( mWindowHandle );
}

void PlatformWindowGLFW::hide()
{
   if (mOffscreenRender)
      return;

   glfwHideWindow( mWindowHandle );
}

void PlatformWindowGLFW::show()
{
   if (mOffscreenRender)
      return;

   glfwShowWindow( mWindowHandle );
}

void PlatformWindowGLFW::close()
{
   delete this;
}

void PlatformWindowGLFW::defaultRender()
{
   // TODO GLFW
}

void PlatformWindowGLFW::_triggerMouseLocationNotify(double xpos, double ypos)
{
   //U32 mods = getTorqueModFromGLFW(GLFW_GetModState());

   //if(!mMouseLocked)
   //   mouseEvent.trigger(getWindowId(), mods, evt.motion.x, evt.motion.y, false);
   //else
   //   mouseEvent.trigger(getWindowId(), mods, evt.motion.xrel, evt.motion.yrel, true);
}

void PlatformWindowGLFW::_triggerMouseWheelNotify(const GLFW_Event& evt)
{
   //U32 mods = getTorqueModFromGLFW(GLFW_GetModState());
   //S32 wheelDelta = Con::getIntVariable("$pref::Input::MouseWheelSpeed", 120);
   //wheelEvent.trigger(getWindowId(), mods, evt.wheel.x * wheelDelta, evt.wheel.y * wheelDelta);
}

void PlatformWindowGLFW::_triggerMouseButtonNotify(const GLFW_Event& event)
{
   //S32 action = (event.type == GLFW_MOUSEBUTTONDOWN) ? SI_MAKE : SI_BREAK;
   //S32 button = -1;

   //switch (event.button.button)
   //{
   //   case GLFW_BUTTON_LEFT:
   //      button = 0;
   //      break;
   //   case GLFW_BUTTON_RIGHT:
   //      button = 1;
   //      break;
   //   case GLFW_BUTTON_MIDDLE:
   //      button = 2;
   //      break;
   //   default:
   //      return;
   //}
   //
   //U32 mod = getTorqueModFromGLFW( GLFW_GetModState() );
   //buttonEvent.trigger(getWindowId(), mod, action, button );
}

void PlatformWindowGLFW::_triggerKeyNotify(int key, int scancode, int action, int mods)
{
	U32 inputAction;
	
	switch (action)
	{
	case GLFW_RELEASE:
		inputAction = IA_BREAK;
		break;
	case GLFW_REPEAT:
		inputAction = IA_REPEAT;
		break;
	default:
		inputAction = IA_MAKE;
	}

   U32 torqueModifiers = getTorqueModFromGLFW(mods);
   U32 torqueKey = KeyMapGLFW::getTorqueScanCodeFromGLFW(scancode);

   if(scancode)
   {
      keyEvent.trigger(getWindowId(), torqueModifiers, inputAction, torqueKey);
      ////Con::printf("Key %d : %d", tKey.sym, inputAction);

      //if (inputAction == IA_MAKE && GLFW_IsTextInputActive())
      //{
      //   // We have to check if we already have a first responder active.
      //   // We don't want to type the character if it actually creates another responder!
      //   if (mWindowInputGenerator->lastKeyWasGlobalActionMap())
      //   {
      //      // Turn off Text input, and the next frame turn it back on. This tells GLFW
      //      // to not generate a text event for this global action map key.
      //      GLFW_StopTextInput();
      //      mOwningManager->updateGLFWTextInputState(PlatformWindowManagerGLFW::KeyboardInputState::TEXT_INPUT);
      //   }
      //}
   }
}

void PlatformWindowGLFW::_triggerTextNotify(unsigned int codepoint)
{
   // U32 mod = getTorqueModFromGLFW( GLFW_GetModState() );
   //
   //if( !evt.text.text[1] ) // get a char
   //{
   //   U16 wchar = evt.text.text[0];
   //   charEvent.trigger(getWindowId(), mod, wchar );
   //   //Con::printf("Char: %c", wchar);
   //   return;
   //}
   //else // get a wchar string
   //{
   //   const U32 len = strlen(evt.text.text);
   //   U16 wchar[16] = {};
   //   dMemcpy(wchar, evt.text.text, sizeof(char)*len);

   //   for(int i = 0; i < 16; ++i)
   //   {
   //      if( !wchar[i] )
   //         return;

   //      charEvent.trigger(getWindowId(), mod, wchar[i] );
   //   }
   //}
}

void PlatformWindowGLFW::_processGLFWEvent(GLFW_Event &evt)
{
   //switch(evt.type)
   //{        
   //   case GLFW_KEYDOWN:
   //   case GLFW_KEYUP:
   //   {
   //      _triggerKeyNotify(evt);
   //      break;
   //   }

   //   case GLFW_TEXTINPUT:
   //   {         
   //      _triggerTextNotify(evt);
   //      break;
   //   }

   //   case GLFW_MOUSEWHEEL:
   //   {
   //      _triggerMouseWheelNotify(evt);
   //      break;
   //   }

   //   case GLFW_MOUSEMOTION:
   //   {
   //      _triggerMouseLocationNotify(evt);
   //      break;
   //   }
   //   case GLFW_MOUSEBUTTONDOWN:
   //   case GLFW_MOUSEBUTTONUP:
   //   {
   //      appEvent.trigger(getWindowId(), GainFocus);
   //      _triggerMouseButtonNotify(evt);
   //      
   //      break;
   //   }

   //   case GLFW_WINDOWEVENT:
   //   {
   //      switch( evt.window.event )
   //      {
   //         case GLFW_WINDOWEVENT_MAXIMIZED:
   //         case GLFW_WINDOWEVENT_RESIZED:
   //         {
   //            int width, height;
   //            GLFW_GetWindowSize( mWindowHandle, &width, &height );
   //            mVideoMode.resolution.set( width, height );
   //            getGFXTarget()->resetMode();
   //            break;
   //         }

   //         default:
   //            break;
   //      }
   //   }
   //}

}

//-----------------------------------------------------------------------------
// Mouse Locking
//-----------------------------------------------------------------------------

void PlatformWindowGLFW::setMouseLocked( bool enable )
{
   if (mOffscreenRender)
      return;

   mMouseLocked = enable;
   
   //GLFW_SetWindowGrab( mWindowHandle, GLFW_bool(enable) );
   //GLFW_SetRelativeMouseMode( GLFW_bool(enable) );
}

const UTF16 *PlatformWindowGLFW::getWindowClassName()
{
   // TODO GLFW
   static String str("WindowClassName");
   return str.utf16();
}

const UTF16 *PlatformWindowGLFW::getCurtainWindowClassName()
{
   // TODO GLFW
   static String str("CurtainWindowClassName");
   return str.utf16();
}

void PlatformWindowGLFW::setKeyboardTranslation(const bool enabled)
{
   mEnableKeyboardTranslation = enabled;

   // Flag for update. Let GLFW know what kind of input state we are changing to.
   if (enabled)
      mOwningManager->updateGLFWTextInputState(PlatformWindowManagerGLFW::KeyboardInputState::TEXT_INPUT);
   else
      mOwningManager->updateGLFWTextInputState(PlatformWindowManagerGLFW::KeyboardInputState::RAW_INPUT);
}