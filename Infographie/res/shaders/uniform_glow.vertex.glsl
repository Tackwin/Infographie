#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 model_space;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	gl_Position = vec4(model_space, 1) * (model * view * projection);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
}