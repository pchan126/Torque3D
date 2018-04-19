#ifndef GFX_Vulkan_VERTEX_ATTRIB_LOCATION_H
#define GFX_Vulkan_VERTEX_ATTRIB_LOCATION_H

namespace Torque
{
   enum Vulkan_AttributeLocation
   {
      Vulkan_VertexAttrib_Position = 0,
      Vulkan_VertexAttrib_Normal = 1,
      Vulkan_VertexAttrib_Color = 2,
      Vulkan_VertexAttrib_Tangent = 3,
      Vulkan_VertexAttrib_TangentW = 4,
      Vulkan_VertexAttrib_Binormal = 5,
      Vulkan_VertexAttrib_TexCoord0 = 6,
      Vulkan_VertexAttrib_TexCoord1 = 7,
      Vulkan_VertexAttrib_TexCoord2 = 8,
      Vulkan_VertexAttrib_TexCoord3 = 9,
      Vulkan_VertexAttrib_TexCoord4 = 10,
      Vulkan_VertexAttrib_TexCoord5 = 11,
      Vulkan_VertexAttrib_TexCoord6 = 12,
      Vulkan_VertexAttrib_TexCoord7 = 13,
      Vulkan_VertexAttrib_TexCoord8 = 14,
      Vulkan_VertexAttrib_TexCoord9 = 15,
      Vulkan_VertexAttrib_COUNT,

      Vulkan_VertexAttrib_LAST = Vulkan_VertexAttrib_TexCoord9,
      Vulkan_VertexAttrib_BlendWeight0 = Vulkan_VertexAttrib_TexCoord6,
      Vulkan_VertexAttrib_BlendIndex0 = Vulkan_VertexAttrib_TexCoord2,
   };
}


#endif //GFX_Vulkan_VERTEX_ATTRIB_LOCATION_H