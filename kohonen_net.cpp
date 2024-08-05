#include "kohonen_net.h"
#include <cmath>
#include <random>
/*
hyperparameters:
    - learning_rate_max
    - learning_rate_min
    - r_max (radius)
    - r_min (radius)
*/
SOM::SOM(int dim_1, int dim_2, int dim_3, int input_dim, float learning_rate_max, float learning_rate_min, float r_max, float r_min) :
    dim_1(dim_1), dim_2(dim_2), dim_3(dim_3), input_dim(input_dim), learning_rate_max(learning_rate_max),
    learning_rate_min(learning_rate_min), r_max(r_max), r_min(r_min), learning_rate(learning_rate_max), r(r_max) {
    weights.resize(dim_1 * dim_2 * dim_3, std::vector<float>(input_dim));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    for (int i = 0; i < dim_1 * dim_2 * dim_3; ++i) {
        for (int j = 0; j < input_dim; ++j) {
            weights[i][j] = dis(gen);
        }
    }
}

void SOM::updateWeights(const std::vector<float>& input, int bmuIndex, int epoch) {
    int center1 = bmuIndex / (dim_2 * dim_3);
    int center2 = (bmuIndex % (dim_2 * dim_3)) / dim_3;
    int center3 = bmuIndex % dim_3;

    for (int i = 0; i < dim_1*dim_2*dim_3; ++i) {
        int x = i / (dim_2 * dim_3);
        int y = (i % (dim_2 * dim_3)) / dim_3;
        int z = i % dim_3;
        float squared_distToBMU = std::pow(x - center1, 2) + std::pow(y - center2, 2) + std::pow(z - center3, 2);
        if(squared_distToBMU < r*r) {
            float influence = std::exp(-squared_distToBMU / (2 * r * r));
            for (int d = 0; d < input_dim; ++d) {
                weights[i][d] += influence * learning_rate * (input[d] - weights[i][d]);
            }
        }

    }

    /*
    for (int z = 0; z < dim_3; ++z) {
        for (int y = 0; y < dim_2; ++y) {
            for (int x = 0; x < dim_1; ++x) {
                float squared_distToBMU = std::pow(x - center1, 2) + std::pow(y - center2, 2);
                if(squared_distToBMU < r*r) {
                    float influence = std::exp(-squared_distToBMU / (2 * r * r));
                    int index = y * dim_1 + x;
                    for (int d = 0; d < input_dim; ++d) {
                        weights[index][d] += influence * learning_rate * (input[d] - weights[index][d]);
                    }
                }
            }
        }
    }
    */
}


int SOM::findBMU(const std::vector<float>& input) {
    float minDist = std::numeric_limits<float>::max();
    int bmuIndex = -1;
    for (int i = 0; i < dim_1 * dim_2 * dim_3; ++i) {
        float dist = euclideanDistance(input, weights[i]);
        if (dist < minDist) {
            minDist = dist;
            bmuIndex = i;
        }
    }
    return bmuIndex;
}

void SOM::trainOnline(const std::vector<float>& sample, int epoch) {
    updateWeights(sample, findBMU(sample), epoch);

    learning_rate = (learning_rate-learning_rate_min)*0.99999 + learning_rate_min;
    r = (r-r_min)*0.99999 + r_min;
}


void SOM::train(const std::vector<std::vector<float>>& data, int epochs) {
    for (int epoch = 0; epoch < epochs; ++epoch) {
        for (const auto& sample : data) {
            int bmuIndex = findBMU(sample);
            updateWeights(sample, bmuIndex, epoch);
        }
    }
}

std::vector<std::vector<float>> SOM::getWeights() const {
    return weights;
}

bool SOM::storeWeights(const std::string& filename) {
    std::string filepath = "parameters/" + filename; // Adjust the directory path here

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    for (const auto& neuronWeights : weights) {
        for (const auto& weight : neuronWeights) {
            file << weight << " ";
        }
        file << std::endl;
    }
    std::cout << "Weights stored in " << filename << std::endl;

    file.close();
    return true;
}

bool SOM::loadWeights(const std::string& filename) {
    std::string filepath = "parameters/" + filename; // Adjust the directory path here

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    weights.clear();

    std::vector<float> neuronWeights;
    float weight;
    while (file >> weight) {
        neuronWeights.push_back(weight);
        if (neuronWeights.size() == input_dim) {
            weights.push_back(neuronWeights);
            neuronWeights.clear();
        }
    }
    std::cout << "Weights loaded from "<< filename << std::endl;

    file.close();
    return true;
}





