#include "Cursor3DExperiment.h"
#include "gl/geom/Box.h"
#include "gl/geom/Plane.h"
#include <GLFW/glfw3.h>
#include "gl/geom/Sphere.h"

using namespace gl;
using namespace std;
using namespace std::chrono;
using namespace Leap;

Cursor3DExperiment::Cursor3DExperiment(const vector<float>& thresholds, int trials_per_threshold) :
    Experiment(2 * trials_per_threshold * thresholds.size()),
	bounds_(1.25f),
	thresholds_(thresholds),
	trials_per_threshold_(trials_per_threshold),
	trials_per_input_(trials_per_threshold * thresholds.size()),
	mouse_drag_l_(false)
{
	cam_control_.rotationAllowed(false);
	cam_control_.zoomAllowed(false);
	cam_control_.translationAllowed(false);
	cam_control_.camera().radius(1.8f);
	initTrial();
    
	static const int num_points = 256;
	float angle = 0.0f;
	float angle_step = two_pi / num_points;
	for (int i = 0; i < num_points; i++) {
        float x = cos(angle) * .55f + sin(angle * 12) * 0.125f;
		float y = sin(angle) * .55f + cos(angle * 12) * 0.125f;
//		float x = cos(angle);// * .5f + sin(angle*8) * 0.00325f;
//		float y = sin(angle);// * .5f + cos(angle * 8) * 0.00325f;
		float z = 0.0f;
		z = .05f * sin(angle*12);
		polyline_.points.push_back({ x, y, z });
		angle += angle_step;
	}
}

bool Cursor3DExperiment::withinThreshold()
{
	return (cursor_ - polyline_.points[trial_.illuminated]).length() < trial_.threshold;
}

void Cursor3DExperiment::illuminatePoint()
{
	if (trial_.illuminated < polyline_.points.size()) {
		trial_.trace.points.push_back(cursor_);
		trial_.illuminated++;
	}
}

void Cursor3DExperiment::saveTrial()
{
	cout << "trial       : " << (trialsCompleted() + 1) << endl;
	cout << "input       : " << ((trial_.input == leap) ? "leap" : "mouse") << endl;
	cout << "threshold   : " << trial_.threshold << endl;
	cout << "time (ms)   : " << trialTime().count() << endl;
//	cout << "trace       : " << trial_.trace.points.size() << endl;
//	for (const Vec3& v : trial_.trace.points) {
//        cout << v << endl;
//	}
}

void Cursor3DExperiment::initTrial()
{
    cursor_.set(0.0f, 0.0f, 0.0f);

	float yaw = (((double)rand() / ((double)RAND_MAX + 1)) - 0.5f) * 2.0f * 45 * deg_to_rad;
	float pitch = (((double)rand() / ((double)RAND_MAX + 1)) - 0.5f) * 2.0f * 45.0f * deg_to_rad;

	Camera& camera = cam_control_.camera();
	camera.yaw(45.0f * deg_to_rad);
	camera.pitch(0.0f * deg_to_rad);

	trial_.trace.points.clear();
    trial_.camera_pitch = pitch;
    trial_.camera_yaw = yaw;
	trial_.color = Vec3::random() + Vec3(0.25f);
	trial_.threshold = thresholds_[(trialsCompleted() % trials_per_input_) / trials_per_threshold_];
	trial_.input = (Input)(trialsCompleted() / trials_per_input_);
	trial_.illuminated = 0;
}

void Cursor3DExperiment::leapInput(const Leap::Frame& frame)
{
	if (trial_.input != leap) {
		return;
	}

	pose_.update(frame);
	if (pose_.tracking()) {
		if (pose_.isClosed()) {
			if (!trialInProgress()) {
				startTrial();
			}

			const Mat4& eye2world = cam_control_.camera().viewInverse();
			Vec4 pdelta = pose_.pointerDelta().toVector4<Vec4>();
			Vec4 hand_delta_ws = eye2world * pdelta;
			cursor_ = cursor_ + hand_delta_ws / 150.0f;

			if (trial_.illuminated < polyline_.points.size() && withinThreshold()) {
				illuminatePoint();
			}

		} else {
			if (trialInProgress() && trial_.illuminated == polyline_.points.size()) {
				stopTrial();
			}
		}
	}

	cam_control_.leapInput(frame);
}

void Cursor3DExperiment::mouseButton(int button, int action, int mods)
{
	if (trial_.input != mouse) {
		return;
	}

	cam_control_.mouseButton(button, action, mods);

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		mouse_drag_l_ = action == GLFW_PRESS;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !trialInProgress()) {
		startTrial();
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE &&  trial_.illuminated == polyline_.points.size()) {
		stopTrial();
	}
}

void Cursor3DExperiment::mouseMotion(double x, double y)
{
	if (trial_.input != mouse) {
		return;
	}

	if (mouse_drag_l_) {
		// unproject, change X/Y coords to match mouse on screen
		Vec4 cursor_ndc = cam_control_.camera().projection() * cam_control_.camera().view() * Vec4(cursor_, 1.0f);
		cursor_ndc /= cursor_ndc.w;
		cursor_ndc.x = (x / viewport_.width - 0.5f) * 2.0f;
		cursor_ndc.y = -(y / viewport_.height - 0.5f) * 2.0f;

		// reproject
		Vec4 cursor_ws = cam_control_.camera().viewInverse() * cam_control_.camera().projection().inverse() * cursor_ndc;
		cursor_ws /= cursor_ws.w;
		cursor_ = cursor_ws;

		if (trial_.illuminated < polyline_.points.size() && withinThreshold()) {
			illuminatePoint();
		}
	}
}

void Cursor3DExperiment::mouseScroll(double x, double y)
{
	if (trial_.input != mouse) {
		return;
	}

	if (mouse_drag_l_) {
		// unproject, change Z
		Vec4 cursor_ndc = cam_control_.camera().projection() * cam_control_.camera().view() * Vec4(cursor_, 1.0f);
		cursor_ndc /= cursor_ndc.w;
		cursor_ndc.z += y / 500.0f;
		cursor_ndc.z = min(max(0.6f, cursor_ndc.z), 0.95f);

		// reproject
		Vec4 cursor_ws = cam_control_.camera().viewInverse() * cam_control_.camera().projection().inverse() * cursor_ndc;
		cursor_ws /= cursor_ws.w;
		cursor_ = cursor_ws;

		if (trial_.illuminated < polyline_.points.size() && withinThreshold()) {
			illuminatePoint();
		}
	}
}


void Cursor3DExperiment::draw(const gl::Viewport& viewport)
{
    static bool first_pass = true;
	static Draw grid;
	if (first_pass) {
		first_pass = false;
		grid.color(.7f, .7f, .7f, 0.75f);
		grid.begin(GL_TRIANGLES);
		grid.geometry(Plane(Vec3::zAxis(), Vec3()).triangles(16, 16));
		grid.end();
        
        g_sphere_.color(0.25f, 1.0f, 0.25f);
        g_sphere_.begin(GL_TRIANGLES);
        g_sphere_.geometry(Sphere(Vec3(0.0f, 0.0f, 0.0f), .01).triangles(8));
        g_sphere_.end();
        
        r_sphere_.color(1.0f, 0.25f, 0.25f);
        r_sphere_.begin(GL_TRIANGLES);
        r_sphere_.geometry(Sphere(Vec3(0.0f, 0.0f, 0.0f), .01).triangles(8));
        r_sphere_.end();
        text_.loadFont("menlo18");
	}
    
	viewport_ = viewport;

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	grid.draw();
    
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	Camera& camera = cam_control_.camera();
	camera.aspect(viewport.aspect());

    Mat4 mv = camera.projection() * camera.view();
	drawing_.setModelViewProj(camera.projection() * camera.view());

	// polyline
	drawing_.color(0.5f, 0.5f, 0.5f);
	drawing_.begin(GL_LINE_LOOP);
	for (const Vec3& p : polyline_.points) {
		drawing_.vertex(p.x, p.y, p.z);
	}
	drawing_.end();
	drawing_.draw();
//	glPointSize(8);
//	drawing_.color(0.25f, 1.0f, 0.25f);
//	drawing_.begin(GL_POINTS);
	for (int i = 0; i < trial_.illuminated; i++) {
		const Vec3& p = polyline_.points[i];
        g_sphere_.setModelViewProj(mv * translation(p.x, p.y, p.z));
        g_sphere_.draw();
	}
//	drawing_.end();
//	drawing_.draw();
	drawing_.color(1.0f, 0.25f, 0.25f);
	for (int i = trial_.illuminated; i < polyline_.points.size(); i++) {
		const Vec3& p = polyline_.points[i];
        r_sphere_.setModelViewProj(mv * translation(p.x, p.y, p.z));
        r_sphere_.draw();
	}

	// vector from cursor to next point
	drawing_.color(0.0f, 0.0f, 0.0f);
	drawing_.begin(GL_LINES);
	if (trial_.illuminated < polyline_.points.size()) {
		const Vec3& p = polyline_.points[trial_.illuminated];
		drawing_.vertex(cursor_.x, cursor_.y, cursor_.z);
		drawing_.vertex(p.x, p.y, p.z);
	}
	drawing_.end();
	drawing_.draw();

	// cursor sphere
	drawing_.color(0.0f, 0.0f, 0.0f);
	drawing_.begin(GL_TRIANGLES);
	drawing_.geometry(Sphere(cursor_, 0.03f).triangles(3));
	drawing_.end();
	drawing_.draw();

	// grid plane

	grid.setModelViewProj(camera.projection() * camera.view() * scale(1.5f, 1.5f, 1.5f));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	grid.draw();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
    
    text_.color(0, 0, 0);
    text_.hAlign(TextRenderer::HAlign::center);
    text_.vAlign(TextRenderer::VAlign::top);
    text_.viewport(viewport);
    text_.clear();
    string t = (trial_.input == mouse) ? "Mouse" : "Leap";
    text_.add("Input: " + t, viewport.center().x, viewport.height);
    text_.draw();
}

void Cursor3DExperiment::keyInput(int key, int action, int mods)
{
    if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
        trial_.input = (trial_.input == mouse) ? leap : mouse;
    }
}
