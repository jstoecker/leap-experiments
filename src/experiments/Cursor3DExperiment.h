#ifndef __LEAP_EXPERIMENTS_CURSOR3D_H__
#define __LEAP_EXPERIMENTS_CURSOR3D_H__

#include "Experiment.h"
#include "util/CameraControl.h"
#include "gl/util/Draw.h"
#include "poses/LPose.h"
#include "gl/util/TextRenderer.h"

class Cursor3DExperiment : public Experiment
{
public:
	Cursor3DExperiment(const std::vector<float>& thresholds, int trials_per_threshold);
    std::string name() const override { return "Cursor 3D"; }
	void draw(const gl::Viewport& viewport) override;
	void leapInput(const Leap::Frame& frame) override;
	void mouseButton(int button, int action, int mods) override;
	void mouseMotion(double x, double y) override;
	void mouseScroll(double x, double y) override;
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

	LPose pose_;
    TextRenderer text_;
	CameraControl cam_control_;
	gl::Draw drawing_;
    gl::Draw g_sphere_;
    gl::Draw r_sphere_;
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

#endif // __LEAP_EXPERIMENTS_CURSOR3D_H__
