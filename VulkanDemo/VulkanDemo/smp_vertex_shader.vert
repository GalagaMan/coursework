#version 450

//vec2 positions[3] = vec2[](
//vec2 (0.0, -0.5),
//vec2 (0.5, 0.5),
//vec2 (-0.0, 0.5));

layout (location = 0) out vec4 outColor;
layout(location = 0) in vec4 inVertex;

void main()
{
	gl_Position = vec4(inVertex);
}
//layout (location = 1) in vec4 in_position;
//layout (location = 2) in vec4 in_color;
//layout (location = 3) out vec4 ex_color;
//
//void main()
//{
//	gl_Position = in_position.xyzw;
//	ex_color.rgba = in_color.rgba;
//}