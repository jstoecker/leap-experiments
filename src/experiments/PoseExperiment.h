#ifndef __LEAP_EXPERIMENTS_POSE_H__
#define __LEAP_EXPERIMENTS_POSE_H__

#include "Experiment.h"
#include "CameraControl.h"
#include "gl/util/Draw.h"
#include "gl/Program.h"
#include "gl/Buffer.h"
#include "poses/PoseTracker.h"
#include "gl/util/TextRenderer.h"

class PoseExperiment : public Experiment
{
public:
	PoseExperiment(int trials_per_pose);
    std::string name() const override { return "Pose"; }
	void draw(const gl::Viewport& viewport) override;
	void leapInput(const Leap::Frame& frame) override;

protected:
    void initTrial() override;
    void saveTrial() override;
    
private:
    enum PoseType
    {
        V_OPEN,
        V_CLOSED,
        L_OPEN,
        L_CLOSED,
        CARRY,
        FIST_CLOSED,
        FIST_THUMB,
        FIST_THREE,
        PALMS_FACE,
        PINCH,
        POINT,
        POINT2,
        PUSH,
        num_poses
    };
    
	CameraControl cam_control_;
	gl::Draw drawing_;
    bool valid_pose_;
    bool done_pose_;
    gl::Mat4 view_projection_;
    gl::Program prog_;
	gl::Buffer joint_vbo_;
	gl::Buffer joint_ibo_;
    GLsizei joint_triangle_count_;
    Leap::Frame frame_;
    PoseTracker poses_;
    int trials_per_pose_;
    TextRenderer text_;
    std::string pose_name_;
    int breaks_;
    std::chrono::high_resolution_clock::time_point start_valid_;

    void updatePose();
    gl::Vec3 convert(const Leap::Vector& v);
	void drawHand(const Leap::Hand& hand);
	void drawJoint(Leap::Vector center, float scale = 1.0f);
	void drawLine(Leap::Vector start, Leap::Vector end);
};

#endif // __LEAP_EXPERIMENTS_POSE_H__