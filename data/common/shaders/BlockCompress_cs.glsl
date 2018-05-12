/*	http://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats/
	http://developer.download.nvidia.com/compute/cuda/1.1-Beta/x86_website/projects/dxtc/doc/cuda_dxtc.pdf
*/
#include "shaders/def.glsl"

uniform sampler2D txSrc;

layout(std430) restrict writeonly buffer _bfDst
{
	uvec2 bfDst[];
};

uint BlockCompress_RGB565(in vec3 _rgb)
{
	uint ret = 0;
	ret = bitfieldInsert(ret, uint(_rgb.r * 31.0), 11, 5);
	ret = bitfieldInsert(ret, uint(_rgb.g * 63.0), 5,  6);
	ret = bitfieldInsert(ret, uint(_rgb.b * 31.0), 0,  5);
	return ret;
}

void BlockCompress_BC1(in vec3 _src[16], out uint dst_[2])
{
 // find endpoints
	vec3 ep0 = vec3(1.0);
	vec3 ep1 = vec3(0.0);
	for (uint i = 0; i < 16; ++i) {
		ep0 = min(ep0, _src[i]);
		ep1 = max(ep1, _src[i]);
	}
	float range = max(length(ep1 - ep0), 1e-4);
	
 // export endpoints (type 1)
	uint ep0i = BlockCompress_RGB565(ep0);	
	uint ep1i = BlockCompress_RGB565(ep1);
	dst_[0] = 0;
	if (ep0i > ep1i) {
ep0i = ep1i = BlockCompress_RGB565(vec3(1,0,1)); // \todo for now reversed blocks are shown as pink
		/*dst_[0] = bitfieldInsert(dst_[0], ep0i, 0,  16);
		dst_[0] = bitfieldInsert(dst_[0], ep1i, 16, 16);
		vec3 tmp = ep1;
		ep1 = ep0;
		ep0 = tmp;*/
	} else {
		dst_[0] = bitfieldInsert(dst_[0], ep1i, 0,  16);
		dst_[0] = bitfieldInsert(dst_[0], ep0i, 16, 16);
	}

 // find/export color indices
	dst_[1] = 0;
	
	vec3 palette[4];
	palette[0] = ep1;
	palette[1] = ep0;
	palette[2] = 2.0/3.0 * palette[0] + 1.0/3.0 * palette[1];
	palette[3] = 1.0/3.0 * palette[0] + 2.0/3.0 * palette[1];
	for (int i = 0; i < 16; ++i) {
		int idx = 0;
		float minErr = 999.0;
		for (int j = 0; j < 4; ++j) {
			float err = length2(_src[i] - palette[j]);
			if (err < minErr) {
				minErr = err;
				idx = j;
			}
		}
		dst_[1] = bitfieldInsert(dst_[1], idx, (i * 2), 2);
	}
}

shared vec3 src[16];
void main()
{
 // gather block texels (each thread reads 1 texel)
	ivec2 iuv = ivec2(gl_WorkGroupID.xy * 4 + gl_LocalInvocationID.xy);
	src[gl_LocalInvocationIndex] = texelFetch(txSrc, iuv, 0).rgb;
	groupMemoryBarrier();

 // compress
	uint dst[2];
	if (gl_LocalInvocationIndex == 0) { // \todo multithreaded compression
		BlockCompress_BC1(src, dst);
	}

 // write block (first thread performs the write)
	if (gl_LocalInvocationIndex == 0) {
		bfDst[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x] = uvec2(dst[0], dst[1]);
	}
}
