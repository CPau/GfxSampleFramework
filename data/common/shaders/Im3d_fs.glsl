#include "shaders/def.glsl"
#include "shaders/Im3d.glsl"

in VertexData vData;

layout(location=0) out vec4 fResult;

void main() 
{
	fResult = vData.m_color;
	
	#if defined(LINES)
		float d = abs(vData.m_edgeDistance) / vData.m_size;
		d = smoothstep(1.0, 1.0 - (kAntialiasing / vData.m_size), d);
		fResult.a *= d;
	#endif

	#if defined(POINTS)
		float d = length(gl_PointCoord.xy - vec2(0.5));
		d = smoothstep(0.5, 0.5 - (kAntialiasing / vData.m_size), d);
		fResult.a *= d;
	#endif
	
}
