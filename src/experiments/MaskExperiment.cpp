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
bounds_(1.0f),
box_mask_(Box(0.1f))
{
    cam_control_.translationAllowed(false);
    cam_control_.zoomAllowed(true);
    cam_control_.camera().yaw(0.0f * deg_to_rad);
    cam_control_.camera().pitch(30.0f * deg_to_rad);
	cam_control_.camera().radius(1.8f);
    poses_.v().enabled(true);
	initTrial();
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
	poses_.update(frame);
    if (poses_.v().tracking()) {
        moveCursor();
        if (poses_.v().isClosed()) {
            applyEdit();
        }
    } else {
        cam_control_.leapInput(frame);
    }
}

void MaskExperiment::slice()
{
    slicer_.slice(bounds_, cam_control_.camera(), 0.1f, 64, 128);
    
    vbo_.bind();
    vbo_.data(&slicer_.getVertices()[0], slicer_.getVertices().size() * sizeof(Vec3));
    ibo_.bind();
    ibo_.data(&slicer_.getIndices()[0], slicer_.getIndices().size() * sizeof(GLushort));
    slice_indices_ = slicer_.getIndices().size();
}

void MaskExperiment::clearMask()
{
    vector<GLubyte> dat;
    dat.resize(64*64*64, 0);
    tex_mask_.bind();
    tex_mask_.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    tex_mask_.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    tex_mask_.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    tex_mask_.setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    tex_mask_.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    tex_mask_.setData3D(GL_R8, 64, 64, 64, GL_RED, GL_UNSIGNED_BYTE, &dat[0]);
}

void MaskExperiment::draw(const gl::Viewport& viewport)
{
	Camera& camera = cam_control_.camera();
	camera.aspect(viewport.aspect());
    
    if (!vbo_.id()) {
        vbo_.generateVBO(GL_DYNAMIC_DRAW);
        ibo_.generateIBO(GL_DYNAMIC_DRAW);
        prog_ = Program::create("shaders/mask_volume.vert", "shaders/mask_volume.frag");
        
        tex_mask_.generate(GL_TEXTURE_3D);
        glUniform1i(prog_.getUniform("tex_mask"), 0);
        glUniform1i(prog_.getUniform("tex_volume"), 1);

        clearMask();
        slice();
    }
    
    if (cam_control_.updated()) {
        slice();
    }
    
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
    
    // mask box
	drawing_.color(.45f, .7f, .45f);
	drawing_.begin(GL_LINES);
	drawing_.geometry(box_mask_.geometry());
	drawing_.end();
	drawing_.draw();
    
    prog_.enable();
    prog_.uniform("model_view_projection", cam_control_.camera().projection() * cam_control_.camera().view());
    prog_.uniform("bounds_min", bounds_.min());
    prog_.uniform("bounds_size", bounds_.max() - bounds_.min());
    prog_.uniform("cursor_ws", box_mask_.center());
    prog_.uniform("cursor_size", 0.05f);
    vbo_.bind();
    ibo_.bind();
    
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    tex_mask_.bind();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(65535);
    glDrawElements(GL_TRIANGLE_FAN, slice_indices_, GL_UNSIGNED_SHORT, 0);
    glDisable(GL_PRIMITIVE_RESTART);

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MaskExperiment::moveCursor()
{
	const Mat4& eye2world = cam_control_.camera().viewInverse();
	Vec4 hand_delta_ws = eye2world * poses_.v().handPositionDelta().toVector4<Vec4>();
	Vec3 center = box_mask_.center() + hand_delta_ws / 200.0f;
	center = bounds_.clamp(center);
    box_mask_.center(center);
}

void MaskExperiment::applyEdit()
{
    box_mask_.apply(bounds_, tex_mask_, MaskVolume::Operation::sub);
}