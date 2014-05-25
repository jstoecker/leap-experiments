#ifndef __LEAP_EXPERIMENTS_EXPERIMENT_H__
#define __LEAP_EXPERIMENTS_EXPERIMENT_H__

#include "Leap.h"
#include "gl/Viewport.h"
#include <chrono>

class Experiment
{
public:
    Experiment(int trials_total);
	virtual ~Experiment();
    
    bool trialInProgress() const { return trial_in_progress_; }
    int trialsTotal() const { return trials_total_; }
    int trialsCompleted() const { return trials_completed_; }
    
    bool done();
    void start();
    void stop();
    
    virtual void leapInput(const Leap::Frame& frame);
	virtual void keyInput(int key, int action, int mods);
	virtual void mouseButton(int button, int action, int mods);
	virtual void mouseMotion(double x, double y);
	virtual void mouseScroll(double x, double y);
    
    virtual std::string name() const = 0;
	virtual void draw(const gl::Viewport& viewport) = 0;
    
protected:
    std::chrono::high_resolution_clock::time_point trial_start_;
    std::chrono::high_resolution_clock::time_point trial_stop_;
    
    std::chrono::milliseconds trialTime() const;
    void startTrial();
	void stopTrial();
    virtual void initTrial() = 0;
	virtual void saveTrial() = 0;
    
private:
    int trials_total_;
	int trials_completed_;
    bool trial_in_progress_;
};

#endif // __LEAP_EXPERIMENTS_EXPERIMENT_H__