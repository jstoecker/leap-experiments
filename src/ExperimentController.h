#ifndef __LEAP_EXPERIMENTS_EXPERIMENT_CONTROLLER_H__
#define __LEAP_EXPERIMENTS_EXPERIMENT_CONTROLLER_H__

#include <chrono>
#include <vector>
#include <memory>
#include "Leap.h"
#include "gl/Viewport.h"
#include "gl/util/TextRenderer.h"
#include "Experiment.h"

class ExperimentController
{
public:
	ExperimentController();
	void init();
	void resize(int width, int height);
	void update();
	void draw();
	void keyInput(int key, int action, int mods);
	void mouseButton(int button, int action, int mods);
	void mouseMotion(double x, double y);
	void mouseScroll(double x, double y);

protected:
	Leap::Controller leap_;
	gl::Viewport viewport_;
	TextRenderer text_;
	GLuint vao_;
	std::vector<std::unique_ptr<Experiment>> experiments_;
	std::vector<std::unique_ptr<Experiment>>::iterator experiment_;

	void startNext();
};

#endif // __LEAP_EXPERIMENTS_EXPERIMENT_CONTROLLER_H__