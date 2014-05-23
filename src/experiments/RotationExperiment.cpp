#include "RotationExperiment.h"
#include "gl/geom/Box.h"
#include "gl/geom/Plane.h"
#include <GLFW/glfw3.h>

using namespace gl;
using namespace std;
using namespace std::chrono;
using namespace Leap;

RotationExperiment::RotationExperiment() :
	thresholds_{ { 5.0f * deg_to_rad, 2.5f * deg_to_rad, 1.0f * deg_to_rad } },
	trials_per_threshold_(2),
	trials_per_input_(trials_per_threshold_ * thresholds_.size()),
	trials_total_(num_inputs * trials_per_input_),
	trials_completed_(0),
	trial_in_progress_(false)
{
	cam_control_.rotationAllowed(true);
	cam_control_.zoomAllowed(false);
	cam_control_.translationAllowed(false);
	cam_control_.camera().radius(1.8f);
	createTrial();
}

bool RotationExperiment::done()
{
	return trials_completed_ == trials_total_;
}

void RotationExperiment::start()
{
	cout << "start experiment" << endl;
	cout << "trials      : " << trials_total_ << endl;
	cout << endl;
}

void RotationExperiment::stop()
{
	cout << "stop experiment" << endl;
}

void RotationExperiment::update()
{
}

bool RotationExperiment::withinThreshold()
{
	Vec3 a = trial_.target;
	Vec3 b = -cam_control_.camera().forward();
	float radians = acos(a.dot(b));
	return radians <= trial_.threshold;
}

void RotationExperiment::startTrial()
{
	trial_in_progress_ = true;
	trial_.start_time = high_resolution_clock::now();
}

void RotationExperiment::stopTrial()
{
	trial_.stop_time = high_resolution_clock::now();
	trial_in_progress_ = false;
	saveTrial();

	if (++trials_completed_ != trials_total_) {
		createTrial();
	}
}

void RotationExperiment::saveTrial()
{
	milliseconds elapsed = duration_cast<milliseconds>(trial_.stop_time - trial_.start_time);
	const Vec3& t = trial_.target;

	cout << "exp trial   : " << (trials_completed_ + 1) << endl;
	cout << "input trial : " << (trials_completed_ % trials_per_input_ + 1) << endl;
	cout << "input       : " << ((trial_.input == leap) ? "leap" : "mouse") << endl;
	cout << "threshold   : " << trial_.threshold << endl;
	cout << "target      : " << t.x << ", " << t.y << ", " << t.z << endl;
	cout << "time (ms)   : " << elapsed.count() << endl;
	cout << endl;
}

void RotationExperiment::createTrial()
{
	cam_control_.camera().yaw(0.0f);
	cam_control_.camera().pitch(0.0f);

	static const float min_angle = 25.0f * deg_to_rad;
	float angle;
	do {
		trial_.target = ((Vec3::random() - 0.5f) * 2.0f).normalize();
		angle = acos(trial_.target.dot(-cam_control_.camera().forward()));
	} while (angle < min_angle);
	trial_.color = Vec3::random() + Vec3(0.25f);


	trial_.threshold = thresholds_[(trials_completed_ % trials_per_input_) / trials_per_threshold_];
	trial_.input = (Input)(trials_completed_ / trials_per_input_);
}

void RotationExperiment::leapInput(const Leap::Frame& frame)
{
	if (trial_.input != leap) {
		return;
	}

	if (cam_control_.poses().fist().tracking()) {

		if (cam_control_.poses().fist().state() == FistPose::State::closed && !trial_in_progress_) {
			startTrial();
		}

		if (cam_control_.poses().fist().state() != FistPose::State::closed && trial_in_progress_ && withinThreshold()) {
			stopTrial();
		}

	}

	cam_control_.leapInput(frame);
}

void RotationExperiment::mouseButton(int button, int action, int mods)
{
	if (trial_.input != mouse) {
		return;
	}

	cam_control_.mouseButton(button, action, mods);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !trial_in_progress_) {
		startTrial();
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && withinThreshold()) {
		stopTrial();
	}
}

void RotationExperiment::mouseMotion(double x, double y)
{
	if (trial_.input != mouse) {
		return;
	}

	cam_control_.mouseMotion(x, y);
}


void RotationExperiment::draw(const gl::Viewport& viewport)
{
	Camera& camera = cam_control_.camera();
	camera.aspect(viewport.aspect());

	// grid plane
	drawing_.color(.15f, .15f, .15f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawing_.begin(GL_TRIANGLES);
	drawing_.setModelViewProj(camera.projection() * camera.view() * scale(5, 5, 5));
	drawing_.geometry(Plane(Vec3::yAxis(), Vec3()).triangles(32, 32));
	drawing_.end();
	drawing_.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// bounding box
	drawing_.setModelViewProj(camera.projection() * camera.view());
	drawing_.color(.25f, .25f, .25f);
	drawing_.begin(GL_LINES);
	drawing_.geometry(Box(0.25f).lines());
	drawing_.vertex(0.0f, 0.0f, 0.0f);
	drawing_.vertex(0.0f, 0.25f, 0.0f);
	drawing_.end();
	drawing_.draw();

	// target vector
	drawing_.color(trial_.color.x, trial_.color.y, trial_.color.z);
	drawing_.begin(GL_LINES);
	drawing_.vertex(0.0f, 0.0f, 0.0f);
	drawing_.vertex(trial_.target.x, trial_.target.y, trial_.target.z);
	drawing_.end();
	drawing_.draw();

	// 2D status circle
	if (trial_in_progress_) {
		if (withinThreshold()) {
			drawing_.color(0.25f, 1.0f, 0.25f);
		} else {
			drawing_.color(1.0f, 0.25f, 0.25f);
		}
		drawing_.setModelViewProj(viewport.orthoProjection());
		drawing_.begin(GL_TRIANGLE_FAN);
		drawing_.circle(50.0f, viewport.height - 50.0f, 30.0f, 32);
		drawing_.end();
		drawing_.draw();
	}
}