#include "MaskExperiment.h"
#include "gl/geom/Box.h"
#include <GLFW/glfw3.h>

using namespace gl;
using namespace std;
using namespace std::chrono;
using namespace Leap;

static const GLubyte CLEAR_VOXEL = 0;
static const GLubyte GREEN_VOXEL = 64;
static const GLubyte RED_VOXEL = 128;

MaskExperiment::MaskExperiment(const std::vector<float>& red_thresholds, const std::vector<float>& green_thresholds) :
Experiment(red_thresholds.size()),
red_thresholds_(red_thresholds),
green_thresholds_(green_thresholds),
bounds_(1.0f),
box_mask_(Box(0.1f)),
voxels_(64, 64, 64),
must_remove_hand_(false),
remove_op_(true)
{
    cam_control_.translationAllowed(false);
    cam_control_.zoomAllowed(true);
    cam_control_.camera().yaw(0.0f * deg_to_rad);
    cam_control_.camera().pitch(30.0f * deg_to_rad);
	cam_control_.camera().radius(1.8f);
    poses_.v().enabled(true);
    poses_.carry().enabled(true);
}

bool MaskExperiment::voxelsGood()
{
    float r = (float)red_voxels_remaining_ / red_voxels_;
    float g = (float)green_voxels_remaining_ / green_voxels_;
    return r <= red_thresholds_[trialsCompleted()] && g >= green_thresholds_[trialsCompleted()];
}

void MaskExperiment::saveTrial()
{
	cout << "trial       : " << (trialsCompleted() + 1) << endl;
	cout << "r threshold : " << red_thresholds_[trialsCompleted()] << endl;
    cout << "g threshold : " << green_thresholds_[trialsCompleted()] << endl;
	cout << "time (ms)   : " << trialTime().count() << endl;
}

void MaskExperiment::initTrial()
{
    clearMask();
    createVolume();
}

void MaskExperiment::leapInput(const Leap::Frame& frame)
{
    if (must_remove_hand_) {
        if (frame.hands().count() == 0) {
            must_remove_hand_ = false;
        }
        return;
    }
    
    bool was_carry_tracking = poses_.carry().tracking();
    
	poses_.update(frame);
    
    if (poses_.carry().tracking() && !was_carry_tracking) {
        remove_op_ = !remove_op_;
    }
    
    if (poses_.v().tracking()) {
        
        if (!trialInProgress()) {
            startTrial();
        }
        
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
    slicer_.slice(bounds_, cam_control_.camera(), 0.05f, 64, 128);
    
    vbo_.bind();
    vbo_.data(&slicer_.getVertices()[0], slicer_.getVertices().size() * sizeof(Vec3));
    ibo_.bind();
    ibo_.data(&slicer_.getIndices()[0], slicer_.getIndices().size() * sizeof(GLushort));
    slice_indices_ = slicer_.getIndices().size();
}

void MaskExperiment::clearMask()
{
    mask_voxels_.clear();
    mask_voxels_.resize(voxels_.x * voxels_.y * voxels_.z, 0);
    tex_mask_.bind();
    tex_mask_.setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    tex_mask_.setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    tex_mask_.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    tex_mask_.setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    tex_mask_.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    tex_mask_.setData3D(GL_R8, voxels_.x, voxels_.y, voxels_.z, GL_RED, GL_UNSIGNED_BYTE, &mask_voxels_[0]);
}

void MaskExperiment::draw(const gl::Viewport& viewport)
{
    glDisable(GL_DEPTH_TEST);
	Camera& camera = cam_control_.camera();
	camera.aspect(viewport.aspect());
    
    if (!vbo_.id()) {
        init();
    }
    
    if (cam_control_.updated()) {
        slice();
    }
    
	// grid plane
	drawing_.color(.85f, .85f, .85f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawing_.begin(GL_TRIANGLES);
	drawing_.setModelViewProj(camera.projection() * camera.view() * translation(0.0f, -0.5f, 0.0f) * scale(2, 2, 2));
	drawing_.geometry(Plane(Vec3::yAxis(), Vec3()).triangles(8, 8));
	drawing_.end();
	drawing_.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
	// bounding box
	drawing_.setModelViewProj(camera.projection() * camera.view());
	drawing_.color(.75f, .75f, .75f);
	drawing_.begin(GL_LINES);
	drawing_.geometry(bounds_.lines());
	drawing_.end();
	drawing_.draw();
    
    // mask box
	drawing_.color(.45f, .45f, .45f);
	drawing_.begin(GL_LINES);
	drawing_.geometry(box_mask_.geometry());
	drawing_.end();
	drawing_.draw();
    
    // cursor lines
    drawing_.color(0.25f, 0.25f, 0.25f);
    drawing_.begin(GL_LINES);
    drawing_.vertex(box_mask_.center().x - 100, box_mask_.center().y, box_mask_.center().z);
    drawing_.vertex(box_mask_.center().x + 100, box_mask_.center().y, box_mask_.center().z);
    drawing_.vertex(box_mask_.center().x, box_mask_.center().y - 100, box_mask_.center().z);
    drawing_.vertex(box_mask_.center().x, box_mask_.center().y + 100, box_mask_.center().z);
    drawing_.vertex(box_mask_.center().x, box_mask_.center().y, box_mask_.center().z - 100);
    drawing_.vertex(box_mask_.center().x, box_mask_.center().y, box_mask_.center().z + 100);
    drawing_.end();
    drawing_.draw();
    
    if (poses_.v().isClosed()) {
        drawing_.color(0, 0, 0);
    } else {
        drawing_.color(.75f, .75f, .75f);
    }
    drawing_.begin(GL_LINES);
    drawing_.vertex(bounds_.min().x, box_mask_.center().y, box_mask_.center().z);
    drawing_.vertex(bounds_.max().x, box_mask_.center().y, box_mask_.center().z);
    
    drawing_.vertex(box_mask_.center().x, bounds_.min().y, box_mask_.center().z);
    drawing_.vertex(box_mask_.center().x, bounds_.max().y, box_mask_.center().z);
    
    drawing_.vertex(box_mask_.center().x, box_mask_.center().y, bounds_.min().z);
    drawing_.vertex(box_mask_.center().x, box_mask_.center().y, bounds_.max().z);
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

    glActiveTexture(GL_TEXTURE1);
    tex_volume_.bind();
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
    
    
    if (must_remove_hand_) {
        text_.hAlign(TextRenderer::HAlign::center);
        text_.vAlign(TextRenderer::VAlign::center);
        text_.viewport(viewport);
        text_.clear();
        text_.color(1, 1, 1);
        text_.add("Remove Hand", viewport.center().x, viewport.center().y);
        text_.draw();
    } else if (trialInProgress()) {
        text_.hAlign(TextRenderer::HAlign::left);
        text_.vAlign(TextRenderer::VAlign::top);
        text_.viewport(viewport);
        text_.clear();
        stringstream ss;
        ss << "Voxels: ";
        ss << std::fixed << std::setprecision(2);
        ss << (float)red_voxels_remaining_/red_voxels_*100;
        ss << "%   maximum: ";
        ss << (float)red_thresholds_[trialsCompleted()]*100;
        ss << "%";
        text_.color(1, 0, 0);
        text_.add(ss.str(), 0, viewport.height);
        
        ss.str("");
        ss << "Voxels: ";
        ss << std::fixed << std::setprecision(2);
        ss << (float)green_voxels_remaining_/green_voxels_*100;
        ss << "%   minimum: ";
        ss << (float)green_thresholds_[trialsCompleted()]*100;
        ss << "%";
        text_.color(0, 1, 0);
        text_.add(ss.str(), 0, viewport.height - 20);
        
        text_.color(0, 0, 0);
        if (remove_op_) {
            text_.add("Mode: Subtract", 0, viewport.height - 40);
        } else {
            text_.add("Mode: Add", 0, viewport.height - 40);
        }
        text_.draw();
    }
    

}

void MaskExperiment::moveCursor()
{
	const Mat4& eye2world = cam_control_.camera().viewInverse();
	Vec4 hand_delta_ws = eye2world * poses_.v().handPositionDelta().toVector4<Vec4>();
	Vec3 center = box_mask_.center() + hand_delta_ws / 400.0f;
	center = bounds_.clamp(center);
    box_mask_.center(center);
}

void MaskExperiment::applyEdit()
{
    VolumeEdit edit;
    if (remove_op_) {
         edit = box_mask_.sub(bounds_, voxels_.x, voxels_.y, voxels_.z);
    } else {
         edit = box_mask_.add(bounds_, voxels_.x, voxels_.y, voxels_.z);
    }
    
    if (!edit.weights.empty()) {
        tex_mask_.bind();
        glTexSubImage3D(GL_TEXTURE_3D, 0, edit.x, edit.y, edit.z, edit.w, edit.h, edit.d, GL_RED, GL_UNSIGNED_BYTE, &edit.weights[0]);
        
        for (int i = 0; i < edit.w; i++) {
            for (int j = 0; j < edit.h; j++) {
                for (int k = 0; k < edit.d; k++) {
                    int x = i + edit.x;
                    int y = j + edit.y;
                    int z = k + edit.z;
                    int index = x + y * voxels_.x + z * voxels_.x * voxels_.y;
                    
                    GLubyte prev_mask_value = mask_voxels_[index];
                    
                    if (remove_op_) {
                        GLubyte new_mask_value = 255;
                        if (prev_mask_value != new_mask_value) {
                            GLubyte voxel_value = volume_voxels_[index];
                            if (voxel_value == GREEN_VOXEL) {
                                green_voxels_remaining_--;
                            } else if (voxel_value == RED_VOXEL) {
                                red_voxels_remaining_--;
                            }
                        }
                        mask_voxels_[index] = new_mask_value;
                    } else {
                        GLubyte new_mask_value = 0;
                        
                        if (prev_mask_value != new_mask_value) {
                            GLubyte voxel_value = volume_voxels_[index];
                            if (voxel_value == GREEN_VOXEL) {
                                green_voxels_remaining_++;
                            } else if (voxel_value == RED_VOXEL) {
                                red_voxels_remaining_++;
                            }
                        }
                        mask_voxels_[index] = new_mask_value;
                    }

                    if (trialInProgress() && voxelsGood()) {
                        stopTrial();
                        must_remove_hand_ = true;
                    }
                }
            }
        }
    }
}

void MaskExperiment::init()
{
    vbo_.generateVBO(GL_DYNAMIC_DRAW);
    ibo_.generateIBO(GL_DYNAMIC_DRAW);
    prog_ = Program::create("shaders/mask_volume.vert", "shaders/mask_volume.frag");
    
    tex_mask_.generate(GL_TEXTURE_3D);
    tex_volume_.generate(GL_TEXTURE_3D);
    prog_.enable();
    glUniform1i(prog_.getUniform("tex_mask"), 0);
    glUniform1i(prog_.getUniform("tex_volume"), 1);
    
    clearMask();
    createVolume();
    slice();
    
    text_.loadFont("menlo18");
}

void MaskExperiment::createVolume()
{
    volume_voxels_.clear();
    
    Vec3 bounds_size = bounds_.max() - bounds_.min();
    Vec3 voxel_size = bounds_size;
    voxel_size.x /= voxels_.x;
    voxel_size.y /= voxels_.y;
    voxel_size.z /= voxels_.z;
    
    red_voxels_ = 0;
    green_voxels_ = 0;
    int other_voxels = 0;
    
    for (int k = 0; k < voxels_.z; k++) {
        for (int j = 0; j < voxels_.y; j++) {
            for (int i = 0; i < voxels_.x; i++) {
                Vec3 p = Vec3(i + 0.5f, j + 0.5f, k + 0.5f) * voxel_size + bounds_.min();

                if (p.x <= 0.2f && p.x >= -0.2f && p.z <= 0.2f && p.z >= -0.2f) {
                    volume_voxels_.push_back(RED_VOXEL);
                    red_voxels_++;
                } else {
                    if (p.y >= -0.1f && p.y <= 0.1f) {
                        volume_voxels_.push_back(GREEN_VOXEL);
                        green_voxels_++;
                    } else {
                        volume_voxels_.push_back(CLEAR_VOXEL);
                        other_voxels++;
                    }
                }
            }
        }
    }
    
    red_voxels_remaining_ = red_voxels_;
    green_voxels_remaining_ = green_voxels_;
    

    tex_volume_.bind();
    tex_volume_.setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    tex_volume_.setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    tex_volume_.setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    tex_volume_.setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    tex_volume_.setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    tex_volume_.setData3D(GL_R8, voxels_.x, voxels_.y, voxels_.z, GL_RED, GL_UNSIGNED_BYTE, &volume_voxels_[0]);

}

void MaskExperiment::keyInput(int key, int action, int mods)
{
    if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
        remove_op_ = !remove_op_;
    }
}