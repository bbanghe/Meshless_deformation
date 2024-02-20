#version 410

layout(location=0) in vec3 in_Position;
uniform mat4 shadowMVP;
out vec4 position;

void main(){
	position = shadowMVP*vec4(in_Position,1);
	gl_Position = position;
	
}