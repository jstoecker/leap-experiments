#ifndef __LEAP_EXPERIMENTS_CURSOR2D_H__
#define __LEAP_EXPERIMENTS_CURSOR2D_H__

#include "Experiment.h"
#include "util/CameraControl.h"
#include "gl/util/Draw.h"
#include "poses/VPose.h"
#include "gl/util/TextRenderer.h"

class Cursor2DExperiment : public Experiment
{
public:
	Cursor2DExperiment(const std::vector<float>& thresholds, int trials_per_threshold);
    std::string name() const override { return "Cursor"; }
	void draw(const gl::Viewport& viewport) override;
	void leapInput(const Leap::Frame& frame) override;
	void mouseButton(int button, int action, int mods) override;
	void mouseMotion(double x, double y) override;
    void keyInput(int key, int action, int mods) override;
    
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
		std::vector<gl::Vec2> points;
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
    TextRenderer text_;
	CameraControl cam_control_;
	gl::Draw drawing_;
	Trial trial_;
	gl::Vec2 cursor_;
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

#endif // __LEAP_EXPERIMENTS_CURSOR2D_H__