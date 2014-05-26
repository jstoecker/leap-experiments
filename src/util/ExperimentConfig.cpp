#include "ExperimentConfig.h"
#include <fstream>
#include <iostream>

using namespace std;

ExperimentConfig::ExperimentConfig()
{
    std::string cfg_filename = "experiments.cfg";
    if (!load(cfg_filename)) {
        std::cout << "Creating default configuration" << std::endl;
        putValue("v-pose separation", 30.0f);
        putValue("l-pose close angle", 0.3f);
        save(cfg_filename);
    }
    load(cfg_filename);
}