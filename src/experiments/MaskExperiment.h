#ifndef __LEAP_EXPERIMENTS_MASK_H__
#define __LEAP_EXPERIMENTS_MASK_H__

#include "Experiment.h"
#include "util/CameraControl.h"
#include "gl/util/Draw.h"
#include "gl/geom/Box.h"
#include "gl/Program.h"
#include "gl/Buffer.h"
#include "util/BoxSlicer.h"

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
    gl::Buffer vbo_;
    gl::Buffer ibo_;
    gl::Program prog_;
    std::chrono::high_resolution_clock::time_point valid_start_;
    
	bool withinThreshold();
};

#endif // __LEAP_EXPERIMENTS_MASK_H__