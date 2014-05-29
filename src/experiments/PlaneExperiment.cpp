#include "PlaneExperiment.h"
#include "gl/geom/Box.h"
#include <GLFW/glfw3.h>

using namespace gl;
using namespace std;
using namespace std::chrono;
using namespace Leap;

PlaneExperiment::PlaneExperiment(const vector<float>& thresholds, int trials_per_threshold) :
Experiment(trials_per_threshold * thresholds.size()),
thresholds_{thresholds},
trials_per_threshold_(trials_per_threshold),
bounds_(1.0f),
valid_(false)
{
    cam_control_.camera().yaw(0.0f * deg_to_rad);
    cam_control_.camera().pitch(30.0f * deg_to_rad);
	cam_control_.camera().radius(1.8f);
	initTrial();
}

bool PlaneExperiment::withinThreshold()
{
    if (user_intersection_.empty())
        return false;
    
    float max_dist = -1;
    for (const Vec3& a : target_intersection_) {
        Vec3 x = user_plane_.intersect(a, -trial_.target.normal());
        max_dist = std::max(max_dist, (x-a).length());
    }
    
    return max_dist < trial_.threshold;
}

void PlaneExperiment::saveTrial()
{
	cout << "trial       : " << (trialsCompleted() + 1) << endl;
	cout << "threshold   : " << trial_.threshold << endl;
	cout << "targ. norml : " << trial_.target.normal() << endl;
    cout << "targ. dist  : " << trial_.target.distFromOrigin() << endl;
	cout << "time (ms)   : " << trialTime().count() << endl;
}

void PlaneExperiment::initTrial()
{
	static const float min_altitude = 15.0f * deg_to_rad;
	Vec3 n;
	float altitude;
	do {
		n = (Vec3::random().normalize() - 0.5f) * 2.0f;
        n.normalize();
        
        Vec3 projected(n.x, 0.0f, n.z);
        projected.normalize();
        
        altitude = acos(projected.dot(n));
        
	} while (altitude < min_altitude);

    
    trial_.target.normal(n);
	trial_.threshold = thresholds_[trialsCompleted() / trials_per_threshold_];
    trial_.target.distFromOrigin(((double) rand() / (RAND_MAX))*0.5f);
    target_intersection_ = bounds_.intersect(trial_.target);
    valid_ = false;
}

void PlaneExperiment::leapInput(const Leap::Frame& frame)
{
    Hand hand = frame.hands().frontmost();
    
    if (!trialInProgress() && frame.hands().count() == 1) {
        startTrial();
    }
    
    if (trialInProgress()) {
		const Mat4& inv = cam_control_.camera().viewInverse();
		Vec3 n = inv * hand.palmNormal().toVector4<Vec4>();
		Vec3 p = frame.interactionBox().normalizePoint(hand.palmPosition()).toVector4<Vec4>();
		p = (p - 0.5f) * 2.0f * 0.75f;
		p = inv * Vec4(p.x, p.y, p.z, 0.0f);
        //p = bounds_.clamp(p);
        
		user_plane_.normal(n);
        user_plane_.point(p);
        
        user_intersection_ = bounds_.intersect(user_plane_);
        
        bool was_valid = valid_;
        valid_ = withinThreshold();
        if (valid_ && !was_valid) {
            valid_start_ = high_resolution_clock::now();
        }
        
        milliseconds elapsed = duration_cast<milliseconds>(high_resolution_clock::now() - valid_start_);
        if (valid_ && elapsed.count() > 1000) {
            stopTrial();
        }
    }
}

void PlaneExperiment::draw(const gl::Viewport& viewport)
{
	Camera& camera = cam_control_.camera();
	camera.aspect(viewport.aspect());
    
	// grid plane
	drawing_.color(.25f, .25f, .25f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawing_.begin(GL_TRIANGLES);
	drawing_.setModelViewProj(camera.projection() * camera.view() * translation(0.0f, -0.5f, 0.0f) * scale(2, 2, 2));
	drawing_.geometry(Plane(Vec3::yAxis(), Vec3()).triangles(8, 8));
	drawing_.end();
	drawing_.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
	// bounding box
	drawing_.setModelViewProj(camera.projection() * camera.view());
	drawing_.color(.45f, .45f, .45f);
	drawing_.begin(GL_LINES);
	drawing_.geometry(bounds_.lines());
	drawing_.end();
	drawing_.draw();
    
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    
    // user plane
    if (valid_) {
        drawing_.color(0.5f, 1.0f, 0.5f, 1.0f);
    } else {
        drawing_.color(1.0f, 0.5f, 0.5f, 1.0f);
    }
    drawing_.begin(GL_TRIANGLE_FAN);
    for (const Vec3& v : user_intersection_) {
        drawing_.vertex(v.x, v.y, v.z);
    }
    drawing_.end();
    drawing_.draw();
    
    
    // target plane
    drawing_.color(0.5f, 0.25f, 0.5f, 0.5f);
    drawing_.begin(GL_TRIANGLE_FAN);
    for (const Vec3& v : target_intersection_) {
        drawing_.vertex(v.x, v.y, v.z);
    }
    drawing_.end();
    drawing_.draw();
    
    glDisable(GL_BLEND);
}

void PlaneExperiment::keyInput(int key, int action, int mods)
{
    if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
        initTrial();
    }
}