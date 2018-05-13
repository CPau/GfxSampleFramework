/*	http://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats/
	http://sjbrown.co.uk/2006/01/19/dxt-compression-techniques/
	http://fileadmin.cs.lth.se/cs/education/edan35/lectures/l8-texcomp.pdf
	http://developer.download.nvidia.com/compute/cuda/1.1-Beta/x86_website/projects/dxtc/doc/cuda_dxtc.pdf
	https://pdfs.semanticscholar.org/presentation/9410/6e86ee70426b81b7f64d392a068c5ebda06a.pdf
	http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.215.7942&rep=rep1&type=pdf
	http://fileadmin.cs.lth.se/graphics/research/papers/gputc2006/thesis.pdf
*/
#include "shaders/def.glsl"

uniform sampler2D txSrc;

layout(std430) restrict writeonly buffer _bfDst
{
	uvec2 bfDst[];
};

uint Pack_RGB565(in vec3 _rgb)
{
	uint ret = 0;
	ret = bitfieldInsert(ret, uint(_rgb.r * 31.0), 11, 5);
	ret = bitfieldInsert(ret, uint(_rgb.g * 63.0), 5,  6);
	ret = bitfieldInsert(ret, uint(_rgb.b * 31.0), 0,  5);
	return ret;
}

shared vec3 s_srcBlock[16];   // raw block texels
shared uint s_dstIndices[16]; // per-texel palette indices

float cov(in int _x, in int _y, in vec3 _avg)
{
	float ret = 0.0;
	for (int i = 0; i < 16; ++i) {
		ret += (s_srcBlock[i][_x] - _avg[_x]) * (s_srcBlock[i][_y] - _avg[_y]);
	}
	ret *= 1.0/16.0;
	return ret;
} 

void main()
{
 // gather block texels (each thread reads 1 texel)
	ivec2 iuv = ivec2(gl_WorkGroupID.xy * 4 + gl_LocalInvocationID.xy);
	s_srcBlock[gl_LocalInvocationIndex] = texelFetch(txSrc, iuv, 0).rgb;
	groupMemoryBarrier();

	uvec2 dst;
	
 // find endpoints
	vec3 ep0 = vec3(1.0);
	vec3 ep1 = vec3(0.0);
	#if 1
	 // fast, low-quality: find the color space bounding box, endpoints are min/max
	 // \todo atomicMin/atomicMax?
	 	ep0 = ep1 = s_srcBlock[0];
		for (int i = 1; i < 16; ++i) {
			ep0 = min(ep0, s_srcBlock[i]);
			ep1 = max(ep1, s_srcBlock[i]);
		}
	#else
	 // slow, high-quality: use principal component analysis
	 // \todo build the covariance matrix via parallel reduction
	 // http://www.visiondummy.com/2014/04/geometric-interpretation-covariance-matrix/
	 	vec3 avg = vec3(0.0);
		for (int i = 0; i < 16; ++i) {
			avg += s_srcBlock[i];
		}
		avg *= 1.0/16.0;
		float cRR = cov(0, 0, avg);
		float cRG = cov(0, 1, avg);
		float cRB = cov(0, 2, avg);
		float cGG = cov(1, 1, avg);
		float cGB = cov(1, 2, avg);
		float cBB = cov(2, 2, avg);
		mat3  C = mat3(
			cRR, cRG, cRB,
			cRG, cGG, cGB,
			cRB, cGB, cBB
			);
	#endif
	
 // export endpoints (type 1)
	uint ep0i = Pack_RGB565(ep0);	
	uint ep1i = Pack_RGB565(ep1);
	dst[0] = 0;
	if (ep0i > ep1i) {
		ep0i = ep1i = Pack_RGB565(vec3(1,0,1)); // \todo for now reversed blocks are shown as pink
		/*dst[0] = bitfieldInsert(dst[0], ep0i, 0,  16);
		dst[0] = bitfieldInsert(dst[0], ep1i, 16, 16);
		vec3 tmp = ep1;
		ep1 = ep0;
		ep0 = tmp;*/
	} else {
		dst[0] = bitfieldInsert(dst[0], ep1i, 0,  16);
		dst[0] = bitfieldInsert(dst[0], ep0i, 16, 16);
	}

 // find palette indices per texel
	vec3 palette[4];
	palette[0] = ep1;
	palette[1] = ep0;
	palette[2] = 2.0/3.0 * palette[0] + 1.0/3.0 * palette[1];
	palette[3] = 1.0/3.0 * palette[0] + 2.0/3.0 * palette[1];

	int idx = 0;
	float minErr = 999.0;
 // \todo see Nvidia reference for simplifying this ?
	for (int i = 0; i < 4; ++i) {
		float err = length2(s_srcBlock[gl_LocalInvocationIndex] - palette[i]);
		if (err < minErr) {
			minErr = err;
			idx = i;
		}
		s_dstIndices[gl_LocalInvocationIndex] = idx;
	}
	groupMemoryBarrier();

 // final
	if (gl_LocalInvocationIndex == 0) {
	 // pack palette indices
		dst[1] = 0;
		for (int i = 0; i < 16; ++i) {
			dst[1] = bitfieldInsert(dst[1], s_dstIndices[i], (i * 2), 2);
		}
	
	 // write block data
		bfDst[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x] = dst;
	}
}
