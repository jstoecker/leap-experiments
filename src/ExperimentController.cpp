#include "ExperimentController.h"
#include <sstream>
#include "experiments/RotationExperiment.h"
#include "experiments/CursorExperiment.h"

using namespace gl;

ExperimentController::ExperimentController()
{
	srand(time(NULL));
}

void ExperimentController::init()
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	text_.loadFont("menlo18");
	text_.hAlign(TextRenderer::HAlign::left);
	text_.vAlign(TextRenderer::VAlign::bottom);
    
    std::vector<float> rot_threhsolds = { 5.0f * deg_to_rad, 2.5f * deg_to_rad, 1.0f * deg_to_rad };
	experiments_.emplace_back(new RotationExperiment(rot_threhsolds, 1));
    
    std::vector<float> cursor_thresholds = { 0.1f, 0.05f, 0.025f };
	experiments_.emplace_back(new CursorExperiment(cursor_thresholds, 1));

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	viewport_.apply();

	if (experiment_ != experiments_.end()) {
		(*experiment_)->draw(viewport_);

		std::stringstream is;
		is << "Experiment: ";
		is << (experiment_ - experiments_.begin()) + 1;
		is << "/";
		is << experiments_.size();
		text_.color(1.0f, 1.0f, 1.0f, 1.0f);
		text_.clear();
		text_.viewport(viewport_);
		text_.add(is.str(), 0.0f, 0.0f);
		text_.draw();
	}
}

void ExperimentController::keyInput(int key, int action, int mods)
{
	if (experiment_ != experiments_.end()) {
		(*experiment_)->keyInput(key, action, mods);
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