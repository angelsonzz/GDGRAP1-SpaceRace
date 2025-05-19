#version 330 core

out vec4 FragColor;

in vec3 skyTexCoord;

uniform samplerCube skybox;

void main(){
	FragColor = texture(skybox, skyTexCoord);
}