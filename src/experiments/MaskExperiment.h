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

class MaskExperiment : public Experiment
{
public:
	MaskExperiment(int threshold, int trials);
    std::string name() const override { return "Mask"; }
	void draw(const gl::Viewport& viewport) override;
	void leapInput(const Leap::Frame& frame) override;
    
protected:
    void initTrial() override;
    void saveTrial() override;
    
private:
	CameraControl cam_control_;
	gl::Draw drawing_;
    int threshold_;
    gl::Box bounds_;
    BoxSlicer slicer_;
    GLsizei slice_indices_;
    gl::Buffer vbo_;
    gl::Buffer ibo_;
    gl::Texture tex_mask_;
    gl::Program prog_;
    PoseTracker poses_;
    BoxMask box_mask_;
    
    void clearMask();
    void slice();
	void applyEdit();
    void moveCursor();
};

#endif // __LEAP_EXPERIMENTS_MASK_H__