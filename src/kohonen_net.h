#ifndef KOHONEN_NET_H
#define KOHONEN_NET_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "utils.h"

class SOM {
private:
    int dim_1;
    int dim_2;
    int dim_3;
    int input_dim;
    float learning_rate_max;
    float learning_rate_min;
    float r_max;
    float r_min;
    std::vector<std::vector<float>> weights;
    // std::vector<std::vector<std::vector<float>>> weights;

    void updateWeights(const std::vector<float>& input, int bmuIndex, int epoch);
public:
    float learning_rate;
    float r;
    SOM(int dim_1, int dim_2, int dim_3, int input_dim, float learning_rate_max=0.1, float learning_rate_min=0.1, float r_max=4, float r_min=4);
    void train(const std::vector<std::vector<float>>& data, int epochs);
    void trainOnline(const std::vector<float>& sample, int epoch);
    std::vector<std::vector<float>> getWeights() const;
    int findBMU(const std::vector<float>& input);
    bool storeWeights(const std::string& filename);
    bool loadWeights(const std::string& filename);
};


#endif // KOHONEN_NET_H
