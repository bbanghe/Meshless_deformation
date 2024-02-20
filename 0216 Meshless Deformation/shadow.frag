#version 410

out vec4 outColor;
in vec4 position;


void main(){
	float depth = position.z/position.w;
	outColor = vec4(depth,depth,depth,1);
	
}