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

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2(  0.94558609, -0.76890725 ), 
   vec2( -0.094184101,-0.92938870 ), 
   vec2(  0.34495938,  0.29387760 ), 
   vec2( -0.91588581,  0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543,  0.27676845 ), 
   vec2(  0.97484398,  0.75648379 ), 
   vec2(  0.44323325, -0.97511554 ), 
   vec2(  0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2(  0.79197514,  0.19090188 ), 
   vec2( -0.24188840,  0.99706507 ), 
   vec2( -0.81409955,  0.91437590 ), 
   vec2(  0.19984126,  0.78641367 ), 
   vec2(  0.14383161, -0.14100790 ) 
);

float PCFShadow(sampler2D shadowMap, vec2 shadowTexCoord, float currentDepth, float cosTheta) {

	//shadow acne 제거 bias값 
    float shadow = 1.0;
	float bias = 0.005*tan(acos(cosTheta)); //cosTheta -> normal || light 방향으로 이동 -> shadow map 이동
	bias = clamp(bias, 0,0.01f);

	int shadownum = 16;
	float spread = 700.0;

	float r = random(gl_FragCoord.xyy, 3)*2*3.14159265358979;
	mat2 rotation = mat2(cos(r), sin(r), -sin(r), cos(r));

    for (int i = 0; i < shadownum; i++) {

		//index
		//int index = int(16.0*random(floor(worldPosition.xyz*1000.0), i))%16;
//		int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
		float pcfDepth = texture(shadowMap, shadowTexCoord.xy + rotation*poissonDisk[i] / spread).r;

		
		if (pcfDepth < currentDepth - bias) {
            //shadow -= 0.2; 
			shadow -= 1./shadownum /** (1.0 - pcfDepth)*/;

        }
		//shadow -= 0.1 * (1.0 - pcfDepth);
		//shadow -= 0.10 * (1.0 - pcfDepth > currentDepth - bias ? 1.0 : 0.0);	
    }
    return shadow;
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

	float cosTheta = dot(N,L);
	float visibility = PCFShadow(shadowTex, shadowTexCoord, depth, cosTheta);
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
		//out_Color = vec4(vec3(tDepth),1);
	}

}