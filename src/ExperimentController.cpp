#include "ExperimentController.h"
#include <sstream>
#include "experiments/RotationExperiment.h"
#include "experiments/CursorExperiment.h"
#include "experiments/PoseExperiment.h"
#include "experiments/Clip3DExperiment.h"
#include "experiments/MaskExperiment.h"

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

    //experiments_.emplace_back(new PoseExperiment(1));
    
//	experiments_.emplace_back(new RotationExperiment({ 5.0f * deg_to_rad, 2.5f * deg_to_rad, 1.0f * deg_to_rad }, 1));

//    experiments_.emplace_back(new CursorExperiment({ 0.1f, 0.05f, 0.025f }, 1));
    
    //experiments_.emplace_back(new Clip3DExperiment({0.2f, 0.15f, 0.10f}, 5));
    
    std::vector<float> rt = {0.80f, 0.95f};
    std::vector<float> gt = {0.95f, 0.95f};
    experiments_.emplace_back(new MaskExperiment(rt, gt));
    
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
        is.str("");
        is << "Trial: ";
		is << ((*experiment_)->trialsCompleted() + 1);
		is << "/";
		is << (*experiment_)->trialsTotal();
        text_.add(is.str(), 0.0f, 30.0f);
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