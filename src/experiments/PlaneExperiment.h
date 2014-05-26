#ifndef __LEAP_EXPERIMENTS_PLANE_H__
#define __LEAP_EXPERIMENTS_PLANE_H__

#include "Experiment.h"
#include "util/CameraControl.h"
#include "gl/util/Draw.h"
#include "gl/geom/Box.h"
#include "gl/geom/Plane.h"

class PlaneExperiment : public Experiment
{
public:
	PlaneExperiment(const std::vector<float>& thresholds, int trials_per_threshold);
    std::string name() const override { return "Plane"; }
	void draw(const gl::Viewport& viewport) override;
	void leapInput(const Leap::Frame& frame) override;
    void keyInput(int key, int action, int mods) override;
    
protected:
    void initTrial() override;
    void saveTrial() override;
    
private:
	struct Trial
	{
		float threshold;
        gl::Plane target;
	};
    
	CameraControl cam_control_;
	gl::Draw drawing_;
	std::vector<float> thresholds_;
	const int trials_per_threshold_;
    Trial trial_;
    gl::Box bounds_;
    std::vector<gl::Vec3> target_intersection_;
    std::vector<gl::Vec3> user_intersection_;
    gl::Plane user_plane_;
    bool valid_;
    std::chrono::high_resolution_clock::time_point valid_start_;

	bool withinThreshold();
};

#endif // __LEAP_EXPERIMENTS_PLANE_H__