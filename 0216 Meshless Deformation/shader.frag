#version 150 core

out vec4 out_Color;
uniform vec4 color;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 ambientLight;
uniform float shininess = 0.0f;
uniform vec3 cameraPosition;

//texture
uniform int TexOrColor = 0;
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;
uniform sampler2D heightTex;
uniform sampler2D shadowTex;

in vec3 normal;
in vec3 worldPosition;
in vec2 texCoords;
in vec4 shadowCoord;


float PCFShadow(sampler2D shadowMap, vec2 shadowTexCoord, float currentDepth) {
	float shadow = 0.0;
	float bias = 0.0001f;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, shadowTexCoord.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth > pcfDepth + bias ? 0.0 : 1.0;
		}    
	}
	return shadow / 9.0f;
}


void main(void)
{
	vec3 l = lightPosition - worldPosition;
	vec3 L = normalize(l);
	vec3 N = normalize(normal);

	vec3 V = normalize(cameraPosition - worldPosition);
	vec3 R = 2*dot(L,N)*N-L;
	vec3 I = lightColor/dot(l,l);


	float depth = shadowCoord.z/shadowCoord.w;
	vec2 shadowTexCoord = (shadowCoord.xy/shadowCoord.w+vec2(1))*0.5;
	float tDepth = texture(shadowTex, shadowTexCoord).r; 

	/*
	float visibility =1;
	if(depth>tDepth+0.001) visibility = 0.5;
	*/

	float visibility = PCFShadow(shadowTex, shadowTexCoord, depth);
	//clamp: visibility가 0.0보다 작거나 1.0보다 큰 경우 0.0, 1.0으로 설정......
    visibility = clamp(visibility, 0.0, 1.0);


	if(TexOrColor>0){
		vec4 Diffuse = texture(diffuseTex,texCoords);
		vec4 Specular = texture(specularTex,texCoords);
		vec4 Normal = texture(normalTex,texCoords);

		vec3 final_color = Diffuse.rgb*max(0,dot(L,N))*I;
		final_color += pow(dot(R,V),shininess)*I*Specular.rgb;
		final_color *= visibility;

		out_Color = vec4(pow(final_color,vec3(1/2.2)),1);
	}
	else{
		out_Color = color*visibility;
//		out_Color = vec4(vec3(tDepth),1);
	}

}
