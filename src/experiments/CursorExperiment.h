#ifndef __LEAP_EXPERIMENTS_CURSOR_H__
#define __LEAP_EXPERIMENTS_CURSOR_H__

#include "Experiment.h"
#include "CameraControl.h"
#include "gl/util/Draw.h"
#include "poses/VPose.h"

class CursorExperiment : public Experiment
{
public:
	CursorExperiment();
	bool done() override;
	void start() override;
	void stop() override;
	void update() override;
	void draw(const gl::Viewport& viewport) override;
	void leapInput(const Leap::Frame& frame) override;
	void mouseButton(int button, int action, int mods) override;
	void mouseMotion(double x, double y) override;

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
		Input input;
		int illuminated;
	};

	VPose pose_;
	CameraControl cam_control_;
	gl::Draw drawing_;
	Trial trial_;
	gl::Vec3 cursor_;
	Polyline polyline_;
	std::vector<float> thresholds_;
	const int trials_per_threshold_;
	const int trials_per_input_;
	const int trials_total_;
	int trials_completed_;
	bool trial_in_progress_;

	void startTrial();
	void stopTrial();
	void createTrial();
	void saveTrial();
	bool withinThreshold();
	void illuminatePoint();
};

#endif // __LEAP_EXPERIMENTS_CURSOR_H__