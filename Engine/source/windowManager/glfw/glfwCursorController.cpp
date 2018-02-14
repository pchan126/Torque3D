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

#include "core/strings/unicode.h"
#include "math/mMath.h"
#include "windowManager/GLFW/GLFWWindow.h"
#include "windowManager/GLFW/GLFWWindowMgr.h"
#include "windowManager/GLFW/GLFWCursorController.h"
#include "platform/platformInput.h"

#include "glfw/glfw3.h"

//static struct { U32 id; GLFW_SystemCursor resourceID; GLFW_Cursor *cursor;} sgCursorShapeMap[]=
//{
//   { PlatformCursorController::curArrow,       GLFW_SYSTEM_CURSOR_ARROW  ,     NULL },
//   { PlatformCursorController::curWait,        GLFW_SYSTEM_CURSOR_WAIT,        NULL },
//   { PlatformCursorController::curPlus,        GLFW_SYSTEM_CURSOR_CROSSHAIR,   NULL },
//   { PlatformCursorController::curResizeVert,  GLFW_SYSTEM_CURSOR_SIZEWE,      NULL },
//   { PlatformCursorController::curResizeHorz,  GLFW_SYSTEM_CURSOR_SIZENS,      NULL },
//   { PlatformCursorController::curResizeAll,   GLFW_SYSTEM_CURSOR_SIZEALL,     NULL },
//   { PlatformCursorController::curIBeam,       GLFW_SYSTEM_CURSOR_IBEAM,       NULL },
//   { PlatformCursorController::curResizeNESW,  GLFW_SYSTEM_CURSOR_SIZENESW,    NULL },
//   { PlatformCursorController::curResizeNWSE,  GLFW_SYSTEM_CURSOR_SIZENWSE,    NULL },
//   { PlatformCursorController::curHand,        GLFW_SYSTEM_CURSOR_HAND,        NULL },
//   { 0,                                        GLFW_SYSTEM_CURSOR_NO,          NULL },
//};


U32 PlatformCursorControllerGLFW::getDoubleClickTime()
{
   // TODO GLFW
   return 500;
}
S32 PlatformCursorControllerGLFW::getDoubleClickWidth()
{
   // TODO GLFW
   return 32;
}
S32 PlatformCursorControllerGLFW::getDoubleClickHeight()
{
   // TODO GLFW
   return 32;
}

void PlatformCursorControllerGLFW::setCursorPosition( S32 x, S32 y )
{
   //if( PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow() )
   //{
   //   AssertFatal( dynamic_cast<PlatformWindowGLFW*>( PlatformWindowManager::get()->getFirstWindow() ), "");
   //   PlatformWindowGLFW *window = static_cast<PlatformWindowGLFW*>( PlatformWindowManager::get()->getFirstWindow() );
   //   GLFW_WarpMouseInWindow(window->getGLFWWindow(), x, y);
   //}
}

void PlatformCursorControllerGLFW::getCursorPosition( Point2I &point )
{
   //GLFW_GetMouseState( &point.x, &point.y );
}

void PlatformCursorControllerGLFW::setCursorVisible( bool visible )
{
   //GLFW_ShowCursor( visible );
}

bool PlatformCursorControllerGLFW::isCursorVisible()
{
   //return GLFW_ShowCursor( -1 );;
	return false;
}

void PlatformCursorControllerGLFW::setCursorShape(U32 cursorID)
{
   //GLFW_Cursor* cursor = NULL;

   //for(S32 i = 0; sgCursorShapeMap[i].resourceID != GLFW_SYSTEM_CURSOR_NO; ++i)
   //{
   //   if(cursorID == sgCursorShapeMap[i].id)
   //   {  
   //      if( !sgCursorShapeMap[i].cursor )
   //         sgCursorShapeMap[i].cursor = GLFW_CreateSystemCursor( sgCursorShapeMap[i].resourceID );

   //      cursor = sgCursorShapeMap[i].cursor;
   //      break;
   //   }
   //}

   //if( !cursor )
   //   return;   
  
   //GLFW_SetCursor(cursor);
}


void PlatformCursorControllerGLFW::setCursorShape( const UTF8 *fileName, bool reload )
{
   // TODO GLFW
   AssertWarn(0, "PlatformCursorControllerGLFW::setCursorShape - Not implemented");
}
