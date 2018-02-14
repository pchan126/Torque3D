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

#include "platform/platformInput.h"
#include "console/console.h"
#include "core/util/journal/process.h"
#include "windowManager/platformWindowMgr.h"

#include "glfwInput.h"
#include "platform/platformInput.h"
#include "GLFW/glfw3.h"

#ifdef LOG_INPUT
#include <time.h>
#include <stdarg.h>
#endif

#if defined( TORQUE_GLFW ) && !defined( TORQUE_SDL )  

// Static class variables:
InputManager*  Input::smManager;
bool           Input::smActive;
U8             Input::smModifierKeys;
bool           Input::smLastKeyboardActivated;
bool           Input::smLastMouseActivated;
bool           Input::smLastJoystickActivated;
InputEvent     Input::smInputEvent;

#ifdef LOG_INPUT
static HANDLE gInputLog;
#endif

static void fillAsciiTable() {}

//------------------------------------------------------------------------------
//
// This function gets the standard ASCII code corresponding to our key code
// and the existing modifier key state.
//
//------------------------------------------------------------------------------
struct AsciiData
{
   struct KeyData
   {
      U16   ascii;
      bool  isDeadChar;
   };

   KeyData upper;
   KeyData lower;
   KeyData goofy;
};


#define NUM_KEYS ( KEY_OEM_102 + 1 )
#define KEY_FIRST KEY_ESCAPE

//------------------------------------------------------------------------------
void Input::init()
{
   Con::printf( "Input Init:" );

   destroy();

   smActive = false;
   smLastKeyboardActivated = true;
   smLastMouseActivated = true;
   smLastJoystickActivated = true;

   //GLFW_InitSubSystem( GLFW_INIT_JOYSTICK | GLFW_INIT_HAPTIC | GLFW_INIT_GAMECONTROLLER | GLFW_INIT_EVENTS );

   // Init the current modifier keys
   setModifierKeys(0);
   fillAsciiTable();
   Con::printf( "" );

   // Set ourselves to participate in per-frame processing.
   Process::notify(Input::process, PROCESS_INPUT_ORDER);

}

//------------------------------------------------------------------------------
ConsoleFunction( isJoystickDetected, bool, 1, 1, "isJoystickDetected()" )
{
	for (auto i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++)
		if (glfwJoystickPresent(i))
			return true;
   return false;
}

//------------------------------------------------------------------------------
ConsoleFunction( getJoystickAxes, const char*, 2, 2, "getJoystickAxes( instance )" )
{
   // TODO GLFW
   return( "" );
}

//------------------------------------------------------------------------------
U16 Input::getKeyCode( U16 asciiCode )
{
    //if( asciiCode > 255 )
        return 0;

    //char c[2];
    //c[0]= asciiCode;
    //c[1] = NULL;
    //return KeyMapGLFW::getTorqueScanCodeFromGLFW( GLFW_GetScancodeFromName( c ));
}

//------------------------------------------------------------------------------
U16 Input::getAscii( U16 keyCode, KEY_STATE keyState )
{
   //if ( keyCode >= NUM_KEYS )
      return 0;

   //U32 GLFWKey = KeyMapGLFW::getGLFWScanCodeFromTorque( keyCode );
   //GLFWKey = GLFW_GetKeyFromScancode( (GLFW_Scancode)GLFWKey );

   //const char *text = GLFW_GetKeyName( GLFWKey );
   //if(text[1] != 0)
   //   return 0;
   //U8 ret = text[0];

   //if( !dIsalpha(ret) )
   //   return ret;

   //switch ( keyState )
   //{
   //   case STATE_LOWER:
   //      return dTolower( ret );
   //   case STATE_UPPER:
   //      return dToupper( ret );
   //   case STATE_GOOFY:
   //      return 0; // TODO GLFW
   //   default:
   //      return(0);

   //}
}

//------------------------------------------------------------------------------
void Input::destroy()
{
   Process::remove(Input::process);

   //GLFW_QuitSubSystem( GLFW_INIT_JOYSTICK | GLFW_INIT_HAPTIC | GLFW_INIT_GAMECONTROLLER );

}

//------------------------------------------------------------------------------
bool Input::enable()
{
   if ( smManager && !smManager->isEnabled() )
      return( smManager->enable() );

   return( false );
}

//------------------------------------------------------------------------------
void Input::disable()
{
   if ( smManager && smManager->isEnabled() )
      smManager->disable();
}

//------------------------------------------------------------------------------

void Input::activate()
{
#ifdef UNICODE
   //winState.imeHandle = ImmGetContext( getWin32WindowHandle() );
   //ImmReleaseContext( getWin32WindowHandle(), winState.imeHandle );
#endif

   if ( !Con::getBoolVariable( "$enableDirectInput" ) )
      return;

   if ( smManager && smManager->isEnabled() && !smActive )
   {
      Con::printf( "Activating Input..." );
      smActive = true;
   }
}

//------------------------------------------------------------------------------
void Input::deactivate()
{
   if ( smManager && smManager->isEnabled() && smActive )
   {
      smActive = false;
      Con::printf( "Input deactivated." );
   }
}

//------------------------------------------------------------------------------
bool Input::isEnabled()
{
   if ( smManager )
      return smManager->isEnabled();
   return false;
}

//------------------------------------------------------------------------------
bool Input::isActive()
{
   return smActive;
}

//------------------------------------------------------------------------------
void Input::process()
{
   if ( smManager && smManager->isEnabled() && smActive )
      smManager->process();
}

//------------------------------------------------------------------------------
InputManager* Input::getManager()
{
   return( smManager );
}

//-----------------------------------------------------------------------------
// Clipboard functions
const char* Platform::getClipboard()
{
   //note - this function never returns NULL
	return "";
}

//-----------------------------------------------------------------------------
bool Platform::setClipboard(const char *text)
{
	if (!text)
		return false;

	//GLFW_SetClipboardText(text);

	return true;
}

//------------------------------------------------------------------------------
//
// This function translates a virtual key code to our corresponding internal
// key code using the preceding table.
//
//------------------------------------------------------------------------------
U8 TranslateOSKeyCode(U8 vcode)
{
	return vcode;
}

U8 TranslateKeyCodeToOS(U8 keycode)
{
	return keycode;
}

#endif

namespace KeyMapGLFW
{
	const int TableSize = 256;
	U32 GLFW_T3D[256];
	U32 T3D_GLFW[256];
	static bool _buildScanCode = true;

	void mapScanCode(U32 GLFW, U32 torque)
	{
		GLFW_T3D[GLFW] = torque;
		T3D_GLFW[torque] = GLFW;
	}



	void buildScanCodeArray()
	{
		_buildScanCode = false;

		for (int i = 0; i < TableSize; ++i)
		{
			GLFW_T3D[i] = 0;
			T3D_GLFW[i] = 0;
		}

		// GLFW, Torque
		mapScanCode(GLFW_KEY_A, KEY_A);
		mapScanCode(GLFW_KEY_B, KEY_B);
		mapScanCode(GLFW_KEY_C, KEY_C);
		mapScanCode(GLFW_KEY_D, KEY_D);
		mapScanCode(GLFW_KEY_E, KEY_E);
		mapScanCode(GLFW_KEY_F, KEY_F);
		mapScanCode(GLFW_KEY_G, KEY_G);
		mapScanCode(GLFW_KEY_H, KEY_H);
		mapScanCode(GLFW_KEY_I, KEY_I);
		mapScanCode(GLFW_KEY_J, KEY_J);
		mapScanCode(GLFW_KEY_K, KEY_K);
		mapScanCode(GLFW_KEY_L, KEY_L);
		mapScanCode(GLFW_KEY_M, KEY_M);
		mapScanCode(GLFW_KEY_N, KEY_N);
		mapScanCode(GLFW_KEY_O, KEY_O);
		mapScanCode(GLFW_KEY_P, KEY_P);
		mapScanCode(GLFW_KEY_Q, KEY_Q);
		mapScanCode(GLFW_KEY_R, KEY_R);
		mapScanCode(GLFW_KEY_S, KEY_S);
		mapScanCode(GLFW_KEY_T, KEY_T);
		mapScanCode(GLFW_KEY_U, KEY_U);
		mapScanCode(GLFW_KEY_V, KEY_V);
		mapScanCode(GLFW_KEY_W, KEY_W);
		mapScanCode(GLFW_KEY_X, KEY_X);
		mapScanCode(GLFW_KEY_Y, KEY_Y);
		mapScanCode(GLFW_KEY_Z, KEY_Z);

		mapScanCode(GLFW_KEY_1, KEY_1);
		mapScanCode(GLFW_KEY_2, KEY_2);
		mapScanCode(GLFW_KEY_3, KEY_3);
		mapScanCode(GLFW_KEY_4, KEY_4);
		mapScanCode(GLFW_KEY_5, KEY_5);
		mapScanCode(GLFW_KEY_6, KEY_6);
		mapScanCode(GLFW_KEY_7, KEY_7);
		mapScanCode(GLFW_KEY_8, KEY_8);
		mapScanCode(GLFW_KEY_9, KEY_9);
		mapScanCode(GLFW_KEY_0, KEY_0);

		mapScanCode(GLFW_KEY_BACKSPACE, KEY_BACKSPACE);
		mapScanCode(GLFW_KEY_TAB, KEY_TAB);
		mapScanCode(GLFW_KEY_ENTER, KEY_RETURN);
		mapScanCode(GLFW_KEY_LEFT_CONTROL, KEY_CONTROL);
		mapScanCode(GLFW_KEY_RIGHT_CONTROL, KEY_CONTROL);
		mapScanCode(GLFW_KEY_LEFT_ALT, KEY_ALT);
		mapScanCode(GLFW_KEY_RIGHT_ALT, KEY_ALT);
		mapScanCode(GLFW_KEY_LEFT_SHIFT, KEY_SHIFT);
		mapScanCode(GLFW_KEY_RIGHT_SHIFT, KEY_SHIFT);
		mapScanCode(GLFW_KEY_PAUSE, KEY_PAUSE);
		mapScanCode(GLFW_KEY_CAPS_LOCK, KEY_CAPSLOCK);
		mapScanCode(GLFW_KEY_ESCAPE, KEY_ESCAPE);
		mapScanCode(GLFW_KEY_SPACE, KEY_SPACE);
		mapScanCode(GLFW_KEY_PAGE_DOWN, KEY_PAGE_DOWN);
		mapScanCode(GLFW_KEY_PAGE_UP, KEY_PAGE_UP);
		mapScanCode(GLFW_KEY_END, KEY_END);
		mapScanCode(GLFW_KEY_HOME, KEY_HOME);
		mapScanCode(GLFW_KEY_LEFT, KEY_LEFT);
		mapScanCode(GLFW_KEY_UP, KEY_UP);
		mapScanCode(GLFW_KEY_RIGHT, KEY_RIGHT);
		mapScanCode(GLFW_KEY_DOWN, KEY_DOWN);
		mapScanCode(GLFW_KEY_PRINT_SCREEN, KEY_PRINT);
		mapScanCode(GLFW_KEY_INSERT, KEY_INSERT);
		mapScanCode(GLFW_KEY_DELETE, KEY_DELETE);
		//mapScanCode(GLFW_KEY_HELP, KEY_HELP);

		mapScanCode(GLFW_KEY_GRAVE_ACCENT, KEY_TILDE);
		mapScanCode(GLFW_KEY_MINUS, KEY_MINUS);
		mapScanCode(GLFW_KEY_EQUAL, KEY_EQUALS);
		mapScanCode(GLFW_KEY_LEFT_BRACKET, KEY_LBRACKET);
		mapScanCode(GLFW_KEY_RIGHT_BRACKET, KEY_RBRACKET);
		mapScanCode(GLFW_KEY_BACKSLASH, KEY_BACKSLASH);
		mapScanCode(GLFW_KEY_SEMICOLON, KEY_SEMICOLON);
		mapScanCode(GLFW_KEY_APOSTROPHE, KEY_APOSTROPHE);
		mapScanCode(GLFW_KEY_COMMA, KEY_COMMA);
		mapScanCode(GLFW_KEY_PERIOD, KEY_PERIOD);
		mapScanCode(GLFW_KEY_SLASH, KEY_SLASH);
		mapScanCode(GLFW_KEY_KP_0, KEY_NUMPAD0);
		mapScanCode(GLFW_KEY_KP_1, KEY_NUMPAD1);
		mapScanCode(GLFW_KEY_KP_2, KEY_NUMPAD2);
		mapScanCode(GLFW_KEY_KP_3, KEY_NUMPAD3);
		mapScanCode(GLFW_KEY_KP_4, KEY_NUMPAD4);
		mapScanCode(GLFW_KEY_KP_5, KEY_NUMPAD5);
		mapScanCode(GLFW_KEY_KP_6, KEY_NUMPAD6);
		mapScanCode(GLFW_KEY_KP_7, KEY_NUMPAD7);
		mapScanCode(GLFW_KEY_KP_8, KEY_NUMPAD8);
		mapScanCode(GLFW_KEY_KP_9, KEY_NUMPAD9);
		mapScanCode(GLFW_KEY_KP_MULTIPLY, KEY_MULTIPLY);
		mapScanCode(GLFW_KEY_KP_ADD, KEY_ADD);
		mapScanCode(GLFW_KEY_KP_EQUAL, KEY_SEPARATOR);
		mapScanCode(GLFW_KEY_KP_SUBTRACT, KEY_SUBTRACT);
		mapScanCode(GLFW_KEY_KP_DECIMAL, KEY_DECIMAL);
		mapScanCode(GLFW_KEY_KP_DIVIDE, KEY_DIVIDE);
		mapScanCode(GLFW_KEY_KP_ENTER, KEY_NUMPADENTER);

		mapScanCode(GLFW_KEY_F1, KEY_F1);
		mapScanCode(GLFW_KEY_F2, KEY_F2);
		mapScanCode(GLFW_KEY_F3, KEY_F3);
		mapScanCode(GLFW_KEY_F4, KEY_F4);
		mapScanCode(GLFW_KEY_F5, KEY_F5);
		mapScanCode(GLFW_KEY_F6, KEY_F6);
		mapScanCode(GLFW_KEY_F7, KEY_F7);
		mapScanCode(GLFW_KEY_F8, KEY_F8);
		mapScanCode(GLFW_KEY_F9, KEY_F9);
		mapScanCode(GLFW_KEY_F10, KEY_F10);
		mapScanCode(GLFW_KEY_F11, KEY_F11);
		mapScanCode(GLFW_KEY_F12, KEY_F12);
		mapScanCode(GLFW_KEY_F13, KEY_F13);
		mapScanCode(GLFW_KEY_F14, KEY_F14);
		mapScanCode(GLFW_KEY_F15, KEY_F15);
		mapScanCode(GLFW_KEY_F16, KEY_F16);
		mapScanCode(GLFW_KEY_F17, KEY_F17);
		mapScanCode(GLFW_KEY_F18, KEY_F18);
		mapScanCode(GLFW_KEY_F19, KEY_F19);
		mapScanCode(GLFW_KEY_F20, KEY_F20);
		mapScanCode(GLFW_KEY_F21, KEY_F21);
		mapScanCode(GLFW_KEY_F22, KEY_F22);
		mapScanCode(GLFW_KEY_F23, KEY_F23);
		mapScanCode(GLFW_KEY_F24, KEY_F24);

		//mapScanCode(GLFW_KEY_LOCKINGNUMLOCK, KEY_NUMLOCK);
		//mapScanCode(GLFW_KEY_LOCKINGSCROLLLOCK, KEY_SCROLLLOCK);
		mapScanCode(GLFW_KEY_LEFT_CONTROL, KEY_LCONTROL);
		mapScanCode(GLFW_KEY_RIGHT_CONTROL, KEY_RCONTROL);
		mapScanCode(GLFW_KEY_LEFT_ALT, KEY_LALT);
		mapScanCode(GLFW_KEY_RIGHT_ALT, KEY_RALT);
		mapScanCode(GLFW_KEY_LEFT_SHIFT, KEY_LSHIFT);
		mapScanCode(GLFW_KEY_RIGHT_SHIFT, KEY_RSHIFT);
		//mapScanCode(____, KEY_WIN_LWINDOW);
		//mapScanCode(____, KEY_WIN_RWINDOW);
		//mapScanCode(____, KEY_WIN_APPS);
		//mapScanCode(____, KEY_OEM_102);

		//mapScanCode(____, KEY_MAC_OPT);
		//mapScanCode(____, KEY_MAC_LOPT);
		//mapScanCode(____, KEY_MAC_ROPT);

		//for(int i = 0; i < 48; ++i)
		//   mapScanCode(____, KEY_BUTTON0 + i );

		//mapScanCode(____, KEY_ANYKEY);   
	}

	U32 KeyMapGLFW::getTorqueScanCodeFromGLFW(int GLFW)
	{
		if (_buildScanCode)
			buildScanCodeArray();

		return GLFW_T3D[GLFW];
	}

	U32 KeyMapGLFW::getGLFWScanCodeFromTorque(U32 torque)
	{
		if (_buildScanCode)
			buildScanCodeArray();

		return T3D_GLFW[torque];
	}
}



