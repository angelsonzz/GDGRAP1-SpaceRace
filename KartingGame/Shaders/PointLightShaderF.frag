#version 330 core

uniform vec3 lightColor;
uniform float lightLumens;

out vec4 FragColor;

void main(){
	FragColor=vec4(lightColor,1.0f)*lightLumens;
}