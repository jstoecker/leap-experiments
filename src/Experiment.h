#ifndef __LEAP_EXPERIMENTS_EXPERIMENT_H__
#define __LEAP_EXPERIMENTS_EXPERIMENT_H__

#include "Leap.h"
#include "gl/Viewport.h"

class Experiment
{
public:
	virtual ~Experiment();
	virtual bool done();
	virtual void start();
	virtual void stop();
	virtual void update();
	virtual void draw(const gl::Viewport& viewport);
	virtual void leapInput(const Leap::Frame& frame);
	virtual void keyInput(int key, int action, int mods);
	virtual void mouseButton(int button, int action, int mods);
	virtual void mouseMotion(double x, double y);
	virtual void mouseScroll(double x, double y);
};

#endif // __LEAP_EXPERIMENTS_EXPERIMENT_H__