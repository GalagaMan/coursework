//#version 450
//#extension GL_KHR_vulkan_glsl : enable
//#extension GL_ARB_separate_shader_objects : enable
////vec2 positions[3] = vec2[](
////vec2 (0.0, -0.5),
////vec2 (0.5, 0.5),
////vec2 (-0.0, 0.5));
//
//layout (location = 1) in vec4 inColor;
//layout(location = 0) in vec4 inVertex;
//
//layout(location = 1) out vec4 exColor;
//
//void main()
//{
//	gl_Position = vec4(inVertex);
//	exColor = inColor;
//}
//layout (location = 1) in vec4 in_position;
//layout (location = 2) in vec4 in_color;
//layout (location = 3) out vec4 ex_color;
//
//void main()
//{
//	gl_Position = in_position.xyzw;
//	ex_color.rgba = in_color.rgba;
//}
#version 450
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform buf {
       mat4 mvp;
} unibuf;
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;
layout (location = 0) out vec4 outColor;

void main() {
   gl_Position = unibuf.mvp * pos;
   outColor = inColor;
}