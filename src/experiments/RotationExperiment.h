#ifndef __LEAP_EXPERIMENTS_ROTATION_H__
#define __LEAP_EXPERIMENTS_ROTATION_H__

#include "Experiment.h"
#include "util/CameraControl.h"
#include "gl/util/Draw.h"
#include "gl/util/TextRenderer.h"

class RotationExperiment : public Experiment
{
public:
	RotationExperiment(const std::vector<float>& thresholds, int trials_per_threshold);
    std::string name() const override { return "Rotation"; }
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

	struct Trial
	{
		float threshold;
		gl::Vec3 target;
		Input input;
	};

	CameraControl cam_control_;
    TextRenderer text_;
	gl::Draw drawing_;
	Trial trial_;
    gl::Vec3 arrow_r_;
    gl::Vec3 arrow_u_;
	std::vector<float> thresholds_;
	const int trials_per_threshold_;
	const int trials_per_input_;

	bool withinThreshold();
};

#endif // __LEAP_EXPERIMENTS_ROTATION_H__