#ifndef __LEAP_EXPERIMENTS_CURSOR_H__
#define __LEAP_EXPERIMENTS_CURSOR_H__

#include "Experiment.h"
#include "CameraControl.h"
#include "gl/util/Draw.h"
#include "poses/VPose.h"

class CursorExperiment : public Experiment
{
public:
	CursorExperiment(std::vector<float> thresholds, int trials_per_threshold);
    std::string name() const override { return "Cursor"; }
	void draw(const gl::Viewport& viewport) override;
	void leapInput(const Leap::Frame& frame) override;
	void mouseButton(int button, int action, int mods) override;
	void mouseMotion(double x, double y) override;
	void mouseScroll(double x, double y) override;

protected:
    void initTrial() override;
    void saveTrial() override;
    
private:
	enum Input
	{
		mouse,
		leap,
		num_inputs
	};

	struct Polyline
	{
		std::vector<gl::Vec3> points;
	};

	struct Trial
	{
		std::chrono::high_resolution_clock::time_point start_time;
		std::chrono::high_resolution_clock::time_point stop_time;
		float threshold;
		gl::Vec3 color;
        float camera_yaw;
        float camera_pitch;
		Polyline trace;
		Input input;
		int illuminated;
	};

	VPose pose_;
	CameraControl cam_control_;
	gl::Draw drawing_;
	Trial trial_;
	gl::Vec3 cursor_;
	gl::Box bounds_;
	gl::Viewport viewport_;
	Polyline polyline_;
	std::vector<float> thresholds_;
	const int trials_per_threshold_;
	const int trials_per_input_;
	bool mouse_drag_l_;

	bool withinThreshold();
	void illuminatePoint();
};

#endif // __LEAP_EXPERIMENTS_CURSOR_H__