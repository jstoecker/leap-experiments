#include "ExperimentController.h"
#include <sstream>
#include "experiments/RotationExperiment.h"
#include "experiments/Cursor3DExperiment.h"
#include "experiments/Cursor2DExperiment.h"
#include "experiments/PoseExperiment.h"
#include "experiments/PlaneExperiment.h"
#include "experiments/MaskExperiment.h"
#include <GLFW/glfw3.h>

using namespace gl;

ExperimentController::ExperimentController()
{
}

void ExperimentController::init()
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	text_.loadFont("menlo18");
	text_.hAlign(TextRenderer::HAlign::left);
	text_.vAlign(TextRenderer::VAlign::bottom);

    experiments_.emplace_back(new PoseExperiment(4));
    experiments_.emplace_back(new Cursor2DExperiment({ 0.06f, 0.02f }, 1));
    experiments_.emplace_back(new Cursor3DExperiment({ 0.065f, 0.0375f }, 1));
    experiments_.emplace_back(new RotationExperiment({ 5.0f * deg_to_rad, 2.5f * deg_to_rad, 1.0f * deg_to_rad }, 1));
    experiments_.emplace_back(new PlaneExperiment({0.2f, 0.15f, 0.10f}, 10));
    experiments_.emplace_back(new MaskExperiment({0.50f, 0.10f}, {0.90f, 0.99f}));
    
	experiment_ = experiments_.begin();
	(*experiment_)->start();
}

void ExperimentController::resize(int width, int height)
{
	viewport_.width = width;
	viewport_.height = height;
}

void ExperimentController::startNext()
{
	if (experiment_ != experiments_.end()) {
		(*experiment_)->stop();
		experiment_++;
		if (experiment_ != experiments_.end()) {
			(*experiment_)->start();
		}
	}
}

void ExperimentController::update()
{
	if (experiment_ != experiments_.end()) {
		(*experiment_)->leapInput(leap_.frame());
		if ((*experiment_)->done()) {
			startNext();
		}
	}
}

void ExperimentController::draw()
{
    glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	viewport_.apply();

	if (experiment_ != experiments_.end()) {
		(*experiment_)->draw(viewport_);

		std::stringstream is;
		is << "Experiment: ";
		is << (experiment_ - experiments_.begin()) + 1;
		is << "/";
		is << experiments_.size();
        text_.hAlign(TextRenderer::HAlign::left);
		text_.color(0.0f, 0.0f, 0.0, 1.0f);
		text_.clear();
		text_.viewport(viewport_);
		text_.add(is.str(), 0.0f, 0.0f);
        is.str("");
        is << "Trial: ";
		is << ((*experiment_)->trialsCompleted() + 1);
		is << "/";
		is << (*experiment_)->trialsTotal();
        text_.hAlign(TextRenderer::HAlign::right);
        text_.add(is.str(), viewport_.width, 0.0f);
		text_.draw();
	}
}

void ExperimentController::keyInput(int key, int action, int mods)
{
	if (experiment_ != experiments_.end()) {
		(*experiment_)->keyInput(key, action, mods);
	}
    
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        startNext();
    }
}

void ExperimentController::mouseButton(int button, int action, int mods)
{
	if (experiment_ != experiments_.end()) {
		(*experiment_)->mouseButton(button, action, mods);
	}
}

void ExperimentController::mouseMotion(double x, double y)
{
	if (experiment_ != experiments_.end()) {
		(*experiment_)->mouseMotion(x, y);
	}
}

void ExperimentController::mouseScroll(double x, double y)
{
	if (experiment_ != experiments_.end()) {
		(*experiment_)->mouseScroll(x, y);
	}
}