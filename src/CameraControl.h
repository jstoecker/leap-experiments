#ifndef __LEAP_EXPERIMENTS_CAMERA_H__
#define __LEAP_EXPERIMENTS_CAMERA_H__

#include "Leap.h"
#include "poses/PoseTracker.h"
#include "gl/math/Math.h"
#include "gl/geom/Box.h"
#include "Camera.h"

class CameraControl
{
public:
	CameraControl();

	void rotationAllowed(bool allowed) { rotation_allowed_ = allowed; }
	void translationAllowed(bool allowed) { translation_allowed_ = allowed; }
	void zoomAllowed(bool allowed) { zoom_allowed_ = allowed; }
	void bounds(const gl::Box& bounds);
	void leapInput(const Leap::Frame& frame);
	void mouseButton(int button, int action, int mods);
	void mouseMotion(double x, double y);
	void mouseScroll(double x, double y);
	Camera& camera() { return camera_; }
	PoseTracker& poses() { return poses_; }

private:
	Camera camera_;
	PoseTracker poses_;
	gl::Box bounds_;
	bool mouse_drag_l_;
	bool mouse_drag_r_;
	double mouse_last_x_;
	double mouse_last_y_;
	bool rotation_allowed_;
	bool translation_allowed_;
	bool zoom_allowed_;

	void leapRotate();
	void leapTranslate();
	void rotate(float delta_yaw, float delta_pitch);
	void move(const gl::Vec3& delta);
	void zoom(float delta_radius);
};

#endif
