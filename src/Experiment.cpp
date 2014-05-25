#include "Experiment.h"

using namespace std::chrono;

Experiment::Experiment(int trials_total) :
    trials_total_(trials_total),
    trials_completed_(0),
    trial_in_progress_(false)
{
}

Experiment::~Experiment()
{
}

bool Experiment::done()
{
	return trials_completed_ == trials_total_;
}

void Experiment::start()
{
    std::cout << "Start Experiment: " << name() << std::endl;
    std::cout << "Num Trials: " << trials_total_ << std::endl;
}

void Experiment::stop()
{
    std::cout << "Stop Experiment: " << name() << std::endl;
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

void Experiment::startTrial()
{
    if (!trial_in_progress_) {
        trial_in_progress_ = true;
        trial_start_ = high_resolution_clock::now();
    }
}

void Experiment::stopTrial()
{
    if (trial_in_progress_) {
        trial_stop_ = high_resolution_clock::now();
        trial_in_progress_ = false;
        saveTrial();
        if (++trials_completed_ != trials_total_) {
            initTrial();
        }
    }
}

milliseconds Experiment::trialTime() const
{
    return duration_cast<milliseconds>(trial_stop_ - trial_start_);
}