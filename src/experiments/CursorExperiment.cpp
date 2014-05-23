#include "CursorExperiment.h"
#include "gl/geom/Box.h"
#include "gl/geom/Plane.h"
#include <GLFW/glfw3.h>
#include "gl/geom/Sphere.h"

using namespace gl;
using namespace std;
using namespace std::chrono;
using namespace Leap;

CursorExperiment::CursorExperiment() :
	thresholds_{ { 5.0f, 2.5f, 1.0f } },
	trials_per_threshold_(2),
	trials_per_input_(trials_per_threshold_ * trials_per_input_),
	trials_total_(num_inputs * trials_per_input_),
	trials_completed_(0),
	trial_in_progress_(false)
{
	cam_control_.rotationAllowed(true);
	cam_control_.zoomAllowed(false);
	cam_control_.translationAllowed(false);
	cam_control_.camera().radius(1.8f);
	createTrial();

	static const int num_points = 16;
	float angle = 0.0f;
	float angle_step = two_pi / num_points;
	for (int i = 0; i < num_points; i++) {
		float x = cos(angle) * .5f;
		float y = sin(angle) * .5f;
		float z = 0.0f;
		polyline_.points.push_back({ x, y, z });
		angle += angle_step;
	}
}

bool CursorExperiment::done()
{
	return trials_completed_ == trials_total_;
}

void CursorExperiment::start()
{
	cout << "start experiment" << endl;
	cout << "trials      : " << trials_total_ << endl;
	cout << endl;
}

void CursorExperiment::stop()
{
	cout << "stop experiment" << endl;
}

void CursorExperiment::update()
{
}

bool CursorExperiment::withinThreshold()
{
	return (cursor_ - polyline_.points[trial_.illuminated]).length() < 0.1f;
}

void CursorExperiment::startTrial()
{
	trial_in_progress_ = true;
	trial_.start_time = high_resolution_clock::now();
}

void CursorExperiment::stopTrial()
{
	trial_.stop_time = high_resolution_clock::now();
	trial_in_progress_ = false;
	saveTrial();

	if (++trials_completed_ != trials_total_) {
		createTrial();
	}
}

void CursorExperiment::illuminatePoint()
{
	if (trial_.illuminated < polyline_.points.size()) {

		trial_.illuminated++;
	}
}

void CursorExperiment::saveTrial()
{
	milliseconds elapsed = duration_cast<milliseconds>(trial_.stop_time - trial_.start_time);
	//const Vec3& t = trial_.target;

	cout << "exp trial   : " << (trials_completed_ + 1) << endl;
	cout << "input trial : " << (trials_completed_ % trials_per_input_ + 1) << endl;
	cout << "input       : " << ((trial_.input == leap) ? "leap" : "mouse") << endl;
	cout << "threshold   : " << trial_.threshold << endl;
	//cout << "target      : " << t.x << ", " << t.y << ", " << t.z << endl;
	cout << "time (ms)   : " << elapsed.count() << endl;
	cout << endl;
}

void CursorExperiment::createTrial()
{
	float yaw = (((double)rand() / ((double)RAND_MAX + 1)) - 0.5f) * 2.0f * 60.0 * deg_to_rad;
	float pitch = (((double)rand() / ((double)RAND_MAX + 1)) - 0.5f) * 2.0f * 45.0f * deg_to_rad;

	Camera& camera = cam_control_.camera();
	camera.yaw(yaw);
	camera.pitch(pitch);

	trial_.color = Vec3::random() + Vec3(0.25f);
	trial_.threshold = thresholds_[(trials_completed_ % trials_per_input_) / trials_per_threshold_];
	trial_.input = (Input)(trials_completed_ / trials_per_input_);
	trial_.input = leap;
	trial_.illuminated = 0;
}

void CursorExperiment::leapInput(const Leap::Frame& frame)
{
	if (trial_.input != leap) {
		return;
	}

	pose_.update(frame);
	if (pose_.tracking()) {
		if (pose_.isClosed()) {
			if (!trial_in_progress_) {
				startTrial();
			}

			const Mat4& eye2world = cam_control_.camera().viewInverse();
			Vec4 hand_delta_ws = eye2world * pose_.handPositionDelta().toVector4<Vec4>();
			cursor_ = Box(1.5f).clamp(cursor_ + hand_delta_ws / 200.0f);

			if (trial_.illuminated < polyline_.points.size() && withinThreshold()) {
				illuminatePoint();
			}

		} else {
			if (trial_in_progress_ && trial_.illuminated == polyline_.points.size()) {
				stopTrial();
			}
		}
	}

	cam_control_.leapInput(frame);
}

void CursorExperiment::mouseButton(int button, int action, int mods)
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

void CursorExperiment::mouseMotion(double x, double y)
{
	if (trial_.input != mouse) {
		return;
	}

	cam_control_.mouseMotion(x, y);
}


void CursorExperiment::draw(const gl::Viewport& viewport)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	Camera& camera = cam_control_.camera();
	camera.aspect(viewport.aspect());

	drawing_.setModelViewProj(camera.projection() * camera.view());

	// polyline
	drawing_.color(0.5f, 0.5f, 0.5f);
	drawing_.begin(GL_LINE_LOOP);
	for (const Vec3& p : polyline_.points) {
		drawing_.vertex(p.x, p.y, p.z);
	}
	drawing_.end();
	drawing_.draw();
	glPointSize(8);
	drawing_.color(0.25f, 1.0f, 0.25f);
	drawing_.begin(GL_POINTS);
	for (int i = 0; i < trial_.illuminated; i++) {
		const Vec3& p = polyline_.points[i];
		drawing_.vertex(p.x, p.y, p.z);
	}
	drawing_.end();
	drawing_.draw();
	drawing_.color(1.0f, 0.25f, 0.25f);
	drawing_.begin(GL_POINTS);
	for (int i = trial_.illuminated; i < polyline_.points.size(); i++) {
		const Vec3& p = polyline_.points[i];
		drawing_.vertex(p.x, p.y, p.z);
	}
	drawing_.end();
	drawing_.draw();

	// vector from cursor to next point
	drawing_.color(1.0f, 1.0f, 0.5f);
	drawing_.begin(GL_LINES);
	if (trial_.illuminated < polyline_.points.size()) {
		const Vec3& p = polyline_.points[trial_.illuminated];
		drawing_.vertex(cursor_.x, cursor_.y, cursor_.z);
		drawing_.vertex(p.x, p.y, p.z);
	}
	drawing_.end();
	drawing_.draw();

	// cursor sphere
	drawing_.color(1.0f, 1.0f, 0.5f);
	drawing_.begin(GL_TRIANGLES);
	drawing_.geometry(Sphere(cursor_, 0.02f).triangles(8));
	drawing_.end();
	drawing_.draw();

	// grid plane
	static bool first_pass = true;
	static Draw grid;
	if (first_pass) {
		first_pass = false;
		grid.color(.5f, .5f, .5f, 0.75f);
		grid.begin(GL_TRIANGLES);
		grid.geometry(Plane(Vec3::zAxis(), Vec3()).triangles(16, 16));
		grid.end();
	}
	grid.setModelViewProj(camera.projection() * camera.view() * scale(1.5f, 1.5f, 1.5f));
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	grid.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	grid.draw();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}