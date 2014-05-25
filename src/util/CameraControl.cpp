#include "CameraControl.h"
#include <GLFW/glfw3.h>

using namespace Leap;
using namespace std;
using namespace gl;

CameraControl::CameraControl() :
	bounds_(1.0f),
	mouse_drag_l_(false),
	mouse_drag_r_(false),
	rotation_allowed_(true),
	translation_allowed_(true),
	zoom_allowed_(true),
    updated_(true)
{
	poses_.fist().enabled(true);
	poses_.fist().maxHandEngageSpeed(150.0f);
	poses_.l().enabled(true);
}

void CameraControl::bounds(const Box& bounds)
{
	bounds_ = bounds;
}

void CameraControl::leapInput(const Frame& frame)
{
    updated_ = false;
	poses_.update(frame);
	if (poses_.fist().tracking() && poses_.fist().state() == FistPose::State::closed && poses_.fist().hand().confidence() > 0.2f) {
		leapRotate();
	} else if (poses_.l().tracking()) {
		leapTranslate();
	}
}

void CameraControl::leapTranslate()
{
	if (poses_.l().isClosed()) {
		Vector t = poses_.l().handPositionDelta() / 300.0f;
		Mat4 eye2world = camera_.view().rotScale().transpose();
		Vec4 v = Vec4(t.x, t.y, t.z, 0);
		move(eye2world * -v);
        updated_ = true;
	}
}

void CameraControl::leapRotate()
{
	Vector a = poses_.fist().hand().stabilizedPalmPosition();
	Vector b = poses_.fist().handPrevious().stabilizedPalmPosition();
	Vector v = (a - b) / 200.0f;

	float d_yaw = v.x * pi;
	float d_pitch = -v.y;
	rotate(d_yaw, d_pitch);

	float palm_speed = poses_.fist().hand().palmVelocity().magnitude();
	if (palm_speed > 0) {
		float z_speed = abs(poses_.fist().hand().palmVelocity().z);
		float z_scale = (z_speed / 120.0f) / (palm_speed / 100.0f);
		float d_radius = -poses_.fist().handPositionDelta().z / 100.0f * min(1.0f, z_scale * z_scale);
		zoom(d_radius);
	}
    
    updated_ = true;
}

void CameraControl::mouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		mouse_drag_l_ = action == GLFW_PRESS;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		mouse_drag_r_ = action == GLFW_PRESS;
	}
}

void CameraControl::mouseMotion(double x, double y)
{
	double dx = x - mouse_last_x_;
	double dy = y - mouse_last_y_;

	if (mouse_drag_l_) {
		float d_yaw = static_cast<float>(dx * 0.005);
		float d_pitch = static_cast<float>(-dy * 0.005);
		rotate(d_yaw, d_pitch);
	}

	if (mouse_drag_r_) {
		Vec4 d;
		d.x = -static_cast<float>(dx * 0.001f);
		d.y = -static_cast<float>(dy * 0.001f);
		move(camera_.viewInverse() * d);
	}

	mouse_last_x_ = x;
	mouse_last_y_ = y;
}

void CameraControl::mouseScroll(double x, double y)
{
	if (mouse_drag_r_) {
		move(camera_.viewInverse() * Vec4(0.0f, 0.0f, -y * 0.05f, 0.0f));
	} else {
		zoom(y * 0.05);
	}
}

void CameraControl::move(const Vec3& delta)
{
	if (translation_allowed_) {
		Vec3 new_center = bounds_.clamp(camera_.center() + delta);
		camera_.center(new_center);
	}
}

void CameraControl::rotate(float delta_yaw, float delta_pitch)
{
	if (rotation_allowed_) {
		camera_.yaw(camera_.yaw() + delta_yaw);
		camera_.pitch(camera_.pitch() + delta_pitch);
	}
}

void CameraControl::zoom(float delta_radius)
{
	if (zoom_allowed_) {
		camera_.radius(camera_.radius() + delta_radius);
	}
}