#version 450


layout (location = 0) out vec4 outColor;
layout (location = 0) in vec4 inColor;

void main()
{
	outColor = inColor;	 
}

//layout (location = 1) in vec4 in_color;
//layout (location = 0) out vec4 out_color;
//
//void main()
//{
//	out_color.rgba = in_color.rgba;
//}