#ifndef GLFW_INPUT_H
#define GLFW_INPUT_H

#include "platform/types.h"

namespace KeyMapGLFW
{
   U32 getTorqueScanCodeFromGLFW(int GLFW);

   U32 getGLFWScanCodeFromTorque(U32 torque);
}

#endif
