#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec4 inColor;
layout (location = 0) out vec4 outColor;

void main()
{
	outColor = inColor.rgba;
}

//layout (location = 1) in vec4 in_color;
//layout (location = 0) out vec4 out_color;
//
//void main()
//{
//	out_color.rgba = in_color.rgba;
//}