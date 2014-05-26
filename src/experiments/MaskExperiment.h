#ifndef __LEAP_EXPERIMENTS_MASK_H__
#define __LEAP_EXPERIMENTS_MASK_H__

#include "Experiment.h"
#include "util/CameraControl.h"
#include "gl/util/Draw.h"
#include "gl/geom/Box.h"
#include "gl/Program.h"
#include "gl/Texture.h"
#include "gl/Buffer.h"
#include "util/BoxSlicer.h"
#include "poses/PoseTracker.h"
#include "util/BoxMask.h"
#include "gl/util/TextRenderer.h"

class MaskExperiment : public Experiment
{
public:
	MaskExperiment(std::vector<float>& red_thresholds, std::vector<float>& green_thresholds);
    std::string name() const override { return "Mask"; }
	void draw(const gl::Viewport& viewport) override;
	void leapInput(const Leap::Frame& frame) override;
	void keyInput(int key, int action, int mods) override;
    
protected:
    void initTrial() override;
    void saveTrial() override;
    
private:
	CameraControl cam_control_;
    TextRenderer text_;
	gl::Draw drawing_;
    std::vector<float> red_thresholds_;
    std::vector<float> green_thresholds_;
    gl::Box bounds_;
    BoxSlicer slicer_;
    GLsizei slice_indices_;
    gl::Buffer vbo_;
    gl::Buffer ibo_;
    gl::Texture tex_mask_;
    gl::Texture tex_volume_;
    gl::Program prog_;
    PoseTracker poses_;
    BoxMask box_mask_;
    gl::Vector3<int> voxels_;
    std::vector<GLubyte> volume_voxels_;
    std::vector<GLubyte> mask_voxels_;
    int red_voxels_remaining_;
    int red_voxels_;
    int green_voxels_remaining_;
    int green_voxels_;
    bool must_remove_hand_;
    bool remove_op_;
    
    bool voxelsGood();
    void init();
    void createVolume();
    void clearMask();
    void slice();
	void applyEdit();
    void moveCursor();
};

#endif // __LEAP_EXPERIMENTS_MASK_H__