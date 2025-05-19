#version 330 core

layout(location = 0) in vec3 aPos;

//gets data at attrib index 2
//converts it and stores it into vec2-atext
layout(location = 2) in vec2 aTex;

//Transformation matrix
uniform mat4 transform;

//Projection matrix
uniform mat4 projection;

//View matrix
uniform mat4 view;

//Passes data to fragment shader
out vec2 texCoord;

layout(location = 1) in vec3 vertexNormal;

out vec3 normCoord;
out vec3 fragPos;

void main(){
	gl_Position = projection * view * transform * vec4(aPos, 1.0);

	texCoord = aTex;

	normCoord = mat3(transpose(inverse(transform))) * vertexNormal;
	fragPos = vec3(transform * vec4(aPos, 1.0));
}