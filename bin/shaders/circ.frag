#version 330

// input from vertex shader
in vec3 norm;
in vec2 tc;

uniform vec4 solid_color;

// the only output variable
out vec4 fragColor;

void main()
{
	if(solid_color==vec4(0.0f,0.0f,0.0f,0.0f)) fragColor=vec4(tc.xy,0,1);
	else fragColor=solid_color;
}