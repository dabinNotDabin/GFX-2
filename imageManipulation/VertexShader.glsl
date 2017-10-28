#version 410

layout(location = 0) in vec2 position;

in vec3 	vertexColour;
out vec3 	toFragShaderColour;

mat4 viewMatrix4 = mat4
(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	-1.0, -1.0, 0.0, 1.0
);

uniform mat4 uniformViewMatrix;


void main()
{
	vec4 positionXYZW = vec4 (position, 0.0, 1.0);

//	gl_Position = viewMatrix4*positionXYZW;

	gl_Position = uniformViewMatrix*positionXYZW;

	
	toFragShaderColour = vertexColour;
}
