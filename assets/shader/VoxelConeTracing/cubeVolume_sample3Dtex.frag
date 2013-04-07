/*
Awaits the inputs of a Cube-vertex shader
*/

#version 420

in VertexData {
	vec3 posWS;
} In;

out vec4 color;

uniform sampler3D tex3D;

void main() {
	ivec3 texSize = textureSize(tex3D, 0);
	vec3 samplePos = In.posWS / vec3(texSize.x, texSize.y, texSize.z);

	color = texture(tex3D, samplePos);
}