#version 150 core

out vec4 out_Color;
uniform vec4 color;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 ambientLight;

uniform float shininess = 0.0f;
uniform vec3 cameraPosition;
uniform int TexOrColor = 0;

//texture
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;
uniform sampler2D heightTex;
uniform sampler2D shadowTex;

in vec3 normal;
in vec3 worldPosition;
in vec2 texCoords;
in vec4 shadowCoord;

mat3 getTBN(vec3 N){
	vec3 Q1 = dFdx(worldPosition), Q2 = dFdy(worldPosition);
	vec2 st1 = dFdx(texCoords), st2 = dFdy(texCoords);
	float D = st1.s*st2.t - st1.t*st2.s;
	return mat3( normalize( (Q1*st2.t-Q2*st1.t)*D ), 
				 normalize( (-Q1*st2.s+Q2*st1.s)*D ),
				 N);
}

void main(void)
{
	vec3 l = lightPosition - worldPosition;
	vec3 L = normalize(l);
	vec3 N = normalize(normal);


/*
	mat3 tbn = getTBN(N);
	float dBdu = texture(bumpTex, texCoords+vec2(0.00001, 0)).r
				-texture(bumpTex, texCoords-vec2(0.00001, 0)).r;
	float dBdv = texture(bumpTex, texCoords+vec2(0, 0.00001)).r
				-texture(bumpTex, texCoords-vec2(0, 0.00001)).r;

	N = normalize(N - dBdu* tbn[0]*100 - dBdv*tbn[1]*100);
*/


	vec3 V = normalize(cameraPosition - worldPosition);
	vec3 R = 2*dot(L,N)*N-L;

	vec3 I = lightColor/dot(l,l);

	vec3 final_color = vec3(0.0f);

	float depth = shadowCoord.z/shadowCoord.w;
	vec2 shadowTexCoord = (shadowCoord.xy/shadowCoord.w+vec2(1))*0.5;
	float tDepth = texture(shadowTex, shadowTexCoord).r; //rgb 모두 같은 depth로 받아왔기 때문.
	
	float visibility =1;
	if(depth>tDepth+0.001) visibility = 0;

	//texture의 경우
	if(TexOrColor>0){
		vec4 Diffuse = texture(diffuseTex,texCoords);
		vec4 Specular = texture(specularTex,texCoords);
		vec4 Normal = texture(normalTex,texCoords);

		final_color = Diffuse.rgb*max(0,dot(L,N))*I+Diffuse.rgb*ambientLight;
		final_color += pow(dot(R,V),shininess)*I*Specular.rgb;
		final_color *= visibility;
		out_Color = vec4(pow(final_color,vec3(1/2.2)),1);
	}
	else{
		out_Color = color*visibility;
	}

}
