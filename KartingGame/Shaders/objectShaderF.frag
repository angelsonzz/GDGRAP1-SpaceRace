#version 330 core

uniform sampler2D tex;

//Texture Transparency
uniform float transparency;

//Point Light
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightLumens;

uniform float ambientStr;
uniform vec3 ambientColor;

uniform vec3 cameraPos;
uniform float specStr;
uniform float specPhong;

//Direction Light
uniform vec3 dirLightDirection;
uniform vec3 dirLightColor;
uniform float dirLightLumens;

uniform float dirAmbientStr;
uniform vec3 dirAmbientColor;

uniform float dirSpecStr;
uniform float dirSpecPhong;

in vec2 texCoord;
in vec3 normCoord;
in vec3 fragPos;

out vec4 FragColor;

void main(){
	vec4 pixelColor=texture(tex, texCoord);
	pixelColor.a=transparency;
	//Alpha Cut off Shader
	if (pixelColor.a<0.0001){
		//Discard the pixel
		discard; //Similar to a break;

	}
	vec3 normal = normalize(normCoord);
	//Point Light
	vec3 lightDir = normalize(lightPos - fragPos);

	float lightObjectDist=length(lightPos-fragPos);
	float apparentBrightness=lightLumens/(lightObjectDist*lightObjectDist);

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 ambientCol = ambientColor * ambientStr;

	vec3 viewDir = normalize(cameraPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(reflectDir, viewDir), 0.1), specPhong);
	vec3 specColor = spec* specStr * lightColor;
	vec4 pointLightVal = vec4(specColor + diffuse + ambientCol, 1.0) * apparentBrightness;

	vec3 dirLightDir = normalize(dirLightDirection);

	float dirLightDiff = max(dot(normal, dirLightDir), 0.0);
	vec3 dirLightDiffuse = dirLightDiff * dirLightColor;

	vec3 dirAmbientCol = dirAmbientColor * dirAmbientStr;

	vec3 dirViewDir = normalize(cameraPos - fragPos);
	vec3 dirReflectDir = reflect(-dirLightDir, normal);
	float dirSpec = pow(max(dot(dirReflectDir, dirViewDir), 0.1), dirSpecPhong);
	vec3 dirSpecColor = dirSpec* dirSpecStr * dirLightColor;
	vec4 directionLightVal = vec4(dirSpecColor + dirLightDiffuse + dirAmbientCol, 1.0)*dirLightLumens;


	vec4 finalLightVal=pointLightVal+directionLightVal;

	FragColor = finalLightVal*pixelColor;
}