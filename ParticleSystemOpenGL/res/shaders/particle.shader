#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 uni_MVP;
uniform mat4 uni_Transform;

void main()
{
	gl_Position = uni_MVP * uni_Transform * position;
};

#shader fragment 
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 uni_Color;

void main()
{
   color = uni_Color;
};
