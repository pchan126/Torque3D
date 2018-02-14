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

#ifndef  _WINDOWMANAGER_GLFW_WINDOW_
#define  _WINDOWMANAGER_GLFW_WINDOW_

#include "windowManager/platformWindowMgr.h"
#include "gfx/gfxTarget.h"
#include "gfx/gfxStructs.h"
#include "sim/actionMap.h"
#include "GLFW\glfw3.h"

class PlatformWindowManagerGLFW;
struct GLFW_Window;
union GLFW_Event;

/// Implementation of a window on GLFW.
class PlatformWindowGLFW : public PlatformWindow
{
	friend class PlatformWindowManagerGLFW;

private:

	/// @name Active window list
	///
	/// Items used to track window instances.
	///
	/// @{

	/// Which manager created us?
	PlatformWindowManagerGLFW *mOwningManager;

	/// Which window comes next in list?
	PlatformWindowGLFW *mNextWindow;

	/// @}

	/// @name Window Information
	///
	/// @{

	/// Our GLFW window.
	GLFWwindow *mWindowHandle;

	/// Our former Parent
	GLFWwindow *mOldParent;

	/// The GFX device that we're tied to.
	GFXDevice *mDevice;

	/// Reference to the render target allocated on this window.
	GFXWindowTargetRef mTarget;

	/// Our current size/resolution/fullscreen status.
	GFXVideoMode mVideoMode;

	/// Our position on the desktop.
	Point2I mPosition;

	/// Is the mouse locked to this window?
	bool mMouseLocked;

	/// Determines whether this window should lock the mouse when it has an opportunity
	bool mShouldLockMouse;

	/// When set, we don't trigger device resets due to sizing events.
	bool mSuppressReset;

	/// Menu associated with this window.  This is a passive property of the window and is not required to be used at all.
	void* mMenuHandle;

	StringTableEntry caption;
	/// @}

	void _processGLFWEvent(GLFW_Event &evt);
	void _triggerMouseLocationNotify(double xpos, double ypos);
	void _triggerMouseButtonNotify(const GLFW_Event& event);
	void _triggerMouseWheelNotify(const GLFW_Event& event);
	void _triggerKeyNotify(int key, int scancode, int action, int mods);
	void _triggerTextNotify(unsigned int codepoint);

public:
	PlatformWindowGLFW();
	~PlatformWindowGLFW();

	virtual void* getSystemWindow(const WindowSystem system);

	void* &getMenuHandle()
	{
		return mMenuHandle;
	}

	void setMenuHandle(void* menuHandle)
	{
		mMenuHandle = menuHandle;
	}

	GLFWwindow* getWindowHandle() const { return mWindowHandle; }

   virtual GFXDevice *getGFXDevice();
   virtual GFXWindowTarget *getGFXTarget();
   
   virtual void setVideoMode(const GFXVideoMode &mode);
   virtual const GFXVideoMode &getVideoMode();
   virtual bool clearFullscreen();
   virtual bool isFullscreen();
   virtual void _setFullscreen(const bool fullscreen);
   
   virtual bool setCaption(const char *cap);
   virtual const char *getCaption();
   
   // Window Client Area Extent
   virtual void setClientExtent( const Point2I newExtent );
   virtual const Point2I getClientExtent();
  
   // Window Bounds
   virtual void setBounds(const RectI &newBounds);
   virtual const RectI getBounds() const;

   // Window Position
   virtual void setPosition( const Point2I newPosition );
   virtual const Point2I getPosition();
   virtual void centerWindow();
   virtual bool setSize(const Point2I &newSize);
   
   // Coordinate space conversion.
   virtual Point2I clientToScreen( const Point2I& pos );
   virtual Point2I screenToClient( const Point2I& pos );

   virtual bool isOpen();
   virtual bool isVisible();
   virtual bool isFocused();
   virtual bool isMinimized();
   virtual bool isMaximized();

   virtual void minimize();
   virtual void maximize();
   virtual void hide();
   virtual void show();
   virtual void close();
   virtual void restore();
   virtual void setFocus();

   virtual void setMouseLocked(bool enable);
   virtual bool isMouseLocked() const { return mMouseLocked; };
   virtual bool shouldLockMouse() const { return mShouldLockMouse; };

   /// Set if relevant keypress events should be translated into character input events.
   virtual void setKeyboardTranslation(const bool enabled);

   virtual WindowId getWindowId();

   GLFWwindow* getGLFWWindow() const { return mWindowHandle; }

   virtual PlatformWindow * getNextWindow() const
   {
      return mNextWindow;
   }

   /// Provide a simple GDI-based render for when the game is not rendering.
   virtual void defaultRender();

   /// Return the class name for the windows we create with this class.
   static const UTF16 *getWindowClassName();

   /// Return the class name for the curtain window class.
   static const UTF16 *getCurtainWindowClassName();

   /// Return the platform specific object needed to create or attach an
   /// accelerated graohics drawing context on or to the window
   virtual void* getPlatformDrawable() const { return mWindowHandle; }
};
#endif
