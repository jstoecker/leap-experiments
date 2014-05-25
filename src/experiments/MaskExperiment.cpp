#include "MaskExperiment.h"
#include "gl/geom/Box.h"
#include <GLFW/glfw3.h>

using namespace gl;
using namespace std;
using namespace std::chrono;
using namespace Leap;

MaskExperiment::MaskExperiment(int threshold, int trials) :
Experiment(trials),
threshold_{threshold},
bounds_(1.0f)
{
    cam_control_.translationAllowed(false);
    cam_control_.zoomAllowed(false);
    cam_control_.camera().yaw(0.0f * deg_to_rad);
    cam_control_.camera().pitch(30.0f * deg_to_rad);
	cam_control_.camera().radius(1.8f);
	initTrial();
}

bool MaskExperiment::withinThreshold()
{
    return false;
}

void MaskExperiment::saveTrial()
{
	cout << "trial       : " << (trialsCompleted() + 1) << endl;
	cout << "threshold   : " << threshold_ << endl;
	cout << "time (ms)   : " << trialTime().count() << endl;
}

void MaskExperiment::initTrial()
{
}

void MaskExperiment::leapInput(const Leap::Frame& frame)
{
    cam_control_.leapInput(frame);
    slicer_.slice(bounds_, cam_control_.camera(), 0.1f, 64, 128);
    if (!vbo_.id()) {
        vbo_.generateVBO(GL_DYNAMIC_DRAW);
        ibo_.generateIBO(GL_DYNAMIC_DRAW);
        prog_ = Program::create("shaders/mask_volume.vert", "shaders/mask_volume.frag");
    }
    
    vbo_.bind();
    vbo_.data(&slicer_.getVertices()[0], slicer_.getVertices().size() * sizeof(Vec3));
    ibo_.bind();
    ibo_.data(&slicer_.getIndices()[0], slicer_.getIndices().size() * sizeof(GLushort));
}

void MaskExperiment::draw(const gl::Viewport& viewport)
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
    
    prog_.enable();
    prog_.uniform("model_view_projection", cam_control_.camera().projection() * cam_control_.camera().view());
    prog_.uniform("bounds_min", bounds_.min());
    prog_.uniform("bounds_size", bounds_.max() - bounds_.min());
    
    vbo_.bind();
    ibo_.bind();
    
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    
    glDrawElements(GL_TRIANGLES, count, type, 0);
}