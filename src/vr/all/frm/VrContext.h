#pragma once

#include <frm/def.h>
#include <frm/Camera.h>

namespace frm {

class Texture;

////////////////////////////////////////////////////////////////////////////////
// VrContext
// \todo
// - What does update() need to do? Need to allow the app to poll the tracking
//   state multiple times e.g. for late latching.
// - Internally manage HMD connection losses as per https://developer.oculus.com/documentation/pcsdk/latest/concepts/dg-vr-focus/
// - Proxy input devices for tracked controllers (including the HMD?). Use as
//   a gamepad - need to add grip axes.
// - Per-hand haptic feedback https://developer.oculus.com/documentation/pcsdk/latest/concepts/dg-input-touch-haptic/.
// - Track velocity and angular velocity.
// - Need to modify the Framebuffer class so as to allow aliasing the same 
//   texture with 2 different viewports as well as modifying the viewport.
// - Default controller rendering? Just provide meshes and skinning data?
////////////////////////////////////////////////////////////////////////////////
class VrContext: private apt::non_copyable<VrContext>
{
public:

	enum Eye_
	{
		Eye_Left,
		Eye_Right,

		Eye_Count
	};
	typedef int Eye;

	enum Hand_
	{
		Hand_Left,
		Hand_Right,

		Hand_Count
	};
	typedef int Hand;

	enum Layer_
	{
		Layer_Main,
		Layer_Text,

		Layer_Count
	};
	typedef int Layer;

	static VrContext*  Create();
	static void        Destroy(VrContext*& _ctx_);

	static VrContext*  GetCurrent();


	// Poll device state, update cameras and poses.
	void               update();

	// Signal that rendering is complete for the specified _eye and _layer.
	void               commitEye(Eye _eye, Layer _layer);

	bool               hasFocus() const;
	bool               isHmdMounted() const;

	const mat4&        getHeadPose() const;
	const mat4&        getEyePose(Eye _eye) const;
	const mat4&        getHandPose(Hand _hand) const;

	const Camera&      getEyeCamera(Eye _eye) const;
	const Texture*     getEyeTexture(Eye _eye, Layer _layer) const;
	const Framebuffer* getEyeFramebuffer(Eye _eye, Layer _layer) const;

private:

	struct Impl;
	Impl* m_impl;
};

} // namespace frm