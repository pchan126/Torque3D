#if defined(TORQUE_GLFW) 

#include <glad/glad.h>
#include "windowManager/glfw/glfwWindow.h"
#include "console/console.h"
#include "gfx/gl/gfxGLUtils.h"
#include "GLFW/glfw3.h"

namespace PlatformGL
{

   void init()
   {
       const U32 majorOGL = 3;
       const U32 minorOGL = 2;
       U32 debugFlag = 0;

	   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorOGL);
	   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorOGL);
//#ifdef TORQUE_DEBUG
//       debugFlag |= GLFW_GL_CONTEXT_DEBUG_FLAG;
//#endif
//
//       GLFW_GL_SetAttribute(GLFW_GL_CONTEXT_MAJOR_VERSION, majorOGL);
//       GLFW_GL_SetAttribute(GLFW_GL_CONTEXT_MINOR_VERSION, minorOGL);
//       GLFW_GL_SetAttribute(GLFW_GL_CONTEXT_PROFILE_MASK, GLFW_GL_CONTEXT_PROFILE_CORE);
//       GLFW_GL_SetAttribute(GLFW_GL_CONTEXT_FLAGS, debugFlag);
//       GLFW_GL_SetAttribute(GLFW_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
//#ifdef TORQUE_GL_SOFTWARE
//       GLFW_GL_SetAttribute(GLFW_GL_ACCELERATED_VISUAL, 0);
//#endif
//
//       GLFW_ClearError();
   }

   void* CreateContextGL( PlatformWindow *window )
   {
       init();
       return nullptr;
   }

   // glfw window already has context created
   void MakeCurrentGL( PlatformWindow *window, void *glContext )
   {
       PlatformWindowGLFW* windowSdl = dynamic_cast<PlatformWindowGLFW*>(window);
       AssertFatal( windowSdl && glContext, "" );
	   glfwMakeContextCurrent( windowSdl->getWindowHandle() );
   }

   void setVSync(const int i)
   {
	   glfwSwapInterval(i);
   }

}

#endif