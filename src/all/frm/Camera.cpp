#include <frm/Camera.h>

#include <frm/Scene.h>

using namespace frm;
using namespace apt;

// PUBLIC

void Camera::setIsOrtho(bool _ortho)
{
	if (m_isOrtho != _ortho) {
		APT_ASSERT(false); // \todo implement
	}
	m_isOrtho = _ortho;
	m_projDirty = true;
}

void Camera::setIsSymmetric(bool _symmetric)
{
	if (_symmetric) {
		float h = (m_up + m_down) * 0.5f;
		m_up = m_down = h;
		float w = (m_left + m_right) * 0.5f;
		m_left = m_right = w;
	}
	m_isSymmetric = _symmetric;
	m_projDirty = true;
}

void Camera::setVerticalFov(float _radians)
{
	float aspect = (m_left + m_right) / (m_up + m_down);
	m_up = tan(_radians * 0.5f);
	m_down = m_up;
	m_left = m_up * aspect;
	m_right = m_left;
	m_projDirty = true;
}

void Camera::setHorizontalFov(float _radians)
{
	float aspect = (m_up + m_down) / (m_left + m_right);
	m_left = tan(_radians * 0.5f);
	m_right = m_up;
	m_up = m_left * aspect;
	m_down = m_up;
	m_projDirty = true;
}

void Camera::setAspect(float _aspect)
{
	m_isSymmetric = true;
	float horizontal = _aspect * (m_up + m_down);
	m_left = horizontal * 0.5f;
	m_right = m_left;
	m_projDirty = true;
}

void Camera::setProjMatrix(const mat4& _proj)
{
	m_proj = _proj; 
	m_localFrustum = Frustum(inverse(_proj));

	vec3* frustum = m_localFrustum.m_vertices;
	m_clipNear = -frustum[0].z;
	m_clipFar  = -frustum[4].z;
	m_isOrtho  =  frustum[0].x == frustum[4].x;

	m_up    =  frustum[0].y;
	m_down  = -frustum[3].y;
	m_left  = -frustum[3].x;
	m_right =  frustum[1].x;
	if (!m_isOrtho) {
		m_up    /= m_clipNear;
		m_down  /= m_clipNear;
		m_left  /= m_clipNear;
		m_right /= m_clipNear;
	}
	m_projDirty = false;
}

void Camera::build()
{
	if (m_projDirty) {
		m_localFrustum = Frustum(m_up, m_down, m_left, m_right, m_clipNear, m_clipFar, m_isOrtho);
		
		if (isOrtho()) {
			m_proj = apt::ortho(m_left, m_right, m_down, m_up, m_clipNear, m_clipFar);
		} else {
			float t = m_localFrustum.m_vertices[0].y;
			float b = m_localFrustum.m_vertices[3].y;
			float l = m_localFrustum.m_vertices[0].x;
			float r = m_localFrustum.m_vertices[1].x;
			float n = m_clipNear;
			float f = m_clipFar;
			m_proj = mat4(
				(2.0f*n)/(r-l),     0.0f,          0.0f,         0.0f,
				     0.0f,      (2.0f*n)/(t-b),    0.0f,         0.0f,
				  (r+l)/(r-l),    (t+b)/(t-b), (n+f)/(n-f),     -1.0f,
				     0.0f,          0.0f,      (2.0f*n*f)/(n-f), 0.0f
				);
		}
		m_projDirty = false;
	}
	
	if (m_node) {
		m_world = m_node->getWorldMatrix();
	}
	m_view = inverse(m_world);
	m_viewProj = m_proj * m_view;
	m_worldFrustum = m_localFrustum;
	m_worldFrustum.transform(m_world);
}
