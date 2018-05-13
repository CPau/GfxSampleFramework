#include "shaders/def.glsl"

layout(std430) restrict readonly buffer _bfSrc
{
	uvec2 bfSrc[];
};

uniform writeonly image2D txDst;

vec3 Unpack_RGB565(in uint _565)
{
	vec3 ret;
	ret.r = float(bitfieldExtract(_565, 11, 5)) / 31.0;
	ret.g = float(bitfieldExtract(_565, 5,  6)) / 63.0;
	ret.b = float(bitfieldExtract(_565, 0,  5)) / 31.0;
	return ret;
}

void main()
{
	uvec2 blockXy = gl_GlobalInvocationID.xy / 4;

	uvec2 block = bfSrc[blockXy.y * (gl_NumWorkGroups.x) + blockXy.x];

	uint epMax  = bitfieldExtract(block[0], 0,  16);
	uint epMin  = bitfieldExtract(block[0], 16, 16);
	vec3 colMax = Unpack_RGB565(epMax);
	vec3 colMin = Unpack_RGB565(epMin);

	vec3 ret = colMax;
	if (epMin > epMax) {
		ret = Color_Magenta;
	}

	imageStore(txDst, ivec2(gl_GlobalInvocationID.xy), vec4(ret, 1.0));
}
