#include "Experiment.h"

Experiment::~Experiment()
{
}

bool Experiment::done()
{
	return true;
}

void Experiment::start()
{
}

void Experiment::stop()
{
}

void Experiment::update()
{
}

void Experiment::draw(const gl::Viewport& viewport)
{
}

void Experiment::leapInput(const Leap::Frame& frame)
{
}

void Experiment::keyInput(int key, int action, int mods)
{
}

void Experiment::mouseButton(int button, int action, int mods)
{
}

void Experiment::mouseMotion(double x, double y)
{
}

void Experiment::mouseScroll(double x, double y)
{
}