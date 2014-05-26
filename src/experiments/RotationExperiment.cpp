#include "RotationExperiment.h"
#include "gl/geom/Box.h"
#include "gl/geom/Plane.h"
#include <GLFW/glfw3.h>

using namespace gl;
using namespace std;
using namespace std::chrono;
using namespace Leap;

RotationExperiment::RotationExperiment(const vector<float>& thresholds, int trials_per_threshold) :
    Experiment(num_inputs * trials_per_threshold * thresholds.size()),
	thresholds_{thresholds},
	trials_per_threshold_(trials_per_threshold),
	trials_per_input_(trials_per_threshold_ * thresholds_.size())
{
	cam_control_.rotationAllowed(true);
	cam_control_.zoomAllowed(false);
	cam_control_.translationAllowed(false);
	cam_control_.camera().radius(1.8f);
	initTrial();
}

bool RotationExperiment::withinThreshold()
{
	Vec3 a = trial_.target;
	Vec3 b = -cam_control_.camera().forward();
	float radians = acos(a.dot(b));
	return radians <= trial_.threshold;
}

void RotationExperiment::saveTrial()
{
	cout << "trial       : " << (trialsCompleted() + 1) << endl;
	cout << "input       : " << ((trial_.input == leap) ? "leap" : "mouse") << endl;
	cout << "threshold   : " << trial_.threshold << endl;
	cout << "target      : " << trial_.target << endl;
	cout << "time (ms)   : " << trialTime().count() << endl;
}

void RotationExperiment::initTrial()
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


	trial_.threshold = thresholds_[(trialsCompleted() % trials_per_input_) / trials_per_threshold_];
	trial_.input = (Input)(trialsCompleted() / trials_per_input_);
    
    
    arrow_r_ = Vec3::random().normalize().cross(trial_.target).normalize();
    arrow_u_ = arrow_r_.cross(trial_.target).normalize();
}

void RotationExperiment::leapInput(const Leap::Frame& frame)
{
	if (trial_.input != leap) {
		return;
	}

	if (cam_control_.poses().fist().tracking()) {
		if (cam_control_.poses().fist().state() == FistPose::State::closed && !trialInProgress()) {
			startTrial();
		}

		if (cam_control_.poses().fist().state() != FistPose::State::closed && trialInProgress() && withinThreshold()) {
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

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !trialInProgress()) {
		startTrial();
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && trialInProgress() && withinThreshold()) {
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
    static bool first = true;
    if (first) {
        first = false;
        text_.loadFont("menlo18");
    }
    
	Camera& camera = cam_control_.camera();
	camera.aspect(viewport.aspect());

	// grid plane
	drawing_.color(.85f, .85f, .85f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawing_.begin(GL_TRIANGLES);
	drawing_.setModelViewProj(camera.projection() * camera.view() * scale(10, 10, 10));
	drawing_.geometry(Plane(Vec3::yAxis(), Vec3()).triangles(32, 32));
	drawing_.end();
	drawing_.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	drawing_.setModelViewProj(camera.projection() * camera.view());

	// target vector
	drawing_.color(trial_.color.x, trial_.color.y, trial_.color.z);
	drawing_.begin(GL_LINES);
	drawing_.vertex(0.0f, 0.0f, 0.0f);
	drawing_.vertex(trial_.target.x, trial_.target.y, trial_.target.z);
    
    drawing_.color(.25f, .35f, .25f);
    drawing_.vertex(0.0f, -10.0f, 0.0f);
	drawing_.vertex(0.0f, 10.0f, 0.0f);
    
    drawing_.color(.35f, .25f, .25f);
    drawing_.vertex(-10.0f, 0.0f, 0.0f);
	drawing_.vertex(10.0f, 0.0f, 0.0f);
    
    drawing_.color(.25f, .25f, .35f);
    drawing_.vertex(0.0f, 0.0f, -10.0f);
	drawing_.vertex(0.0f, 0.0f, 10.0f);
	drawing_.end();
	drawing_.draw();
    
    
    // arrow
    glEnable(GL_DEPTH_TEST);

    int num_pts = 12;
    Vec3 c = trial_.target;
    Mat4 rot = rotation(two_pi / num_pts, trial_.target);
    Vec3 u = arrow_r_;
    drawing_.color(trial_.color.x, trial_.color.y, trial_.color.z);
    drawing_.begin(GL_TRIANGLE_FAN);
    drawing_.vertex(c.x, c.y, c.z);
    drawing_.color(trial_.color.x * .5f, trial_.color.y * .5f, trial_.color.z * .5f);
    for (int i = 0; i <= num_pts; i++) {
        Vec3 p = c * .9f + u * 0.025;
        drawing_.vertex(p.x, p.y, p.z);
        u = rot * Vec4(u, 0);
    }
    drawing_.end();
    drawing_.draw();
    glDisable(GL_DEPTH_TEST);

    
	// 2D status circle
	if (trialInProgress()) {
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
    
    text_.color(0, 0, 0);
    text_.hAlign(TextRenderer::HAlign::center);
    text_.vAlign(TextRenderer::VAlign::top);
    text_.viewport(viewport);
    text_.clear();
    string t = (trial_.input == mouse) ? "Mouse" : "Leap";
    text_.add("Input: " + t, viewport.center().x, viewport.height);
    text_.draw();
    

}

void RotationExperiment::keyInput(int key, int action, int mods)
{
    if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
        trial_.input = (trial_.input == mouse) ? leap : mouse;
    }
}