#ifndef Q_LEARNING_H
#define Q_LEARNING_H
#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

class QLearningModel {
private:
    std::vector<std::vector<float>> qTable;
    int stateCount;
    int actionCount;
    float learningRate;
    float discountFactor;
    float explorationRate_max;

    std::mt19937 rng; // Mersenne Twister random number generator

public:
    float explorationRate;
    QLearningModel(int stateCount, int actionCount, float learningRate = 0.1, float discountFactor = 0.9, float explorationRate_max = 0.1);

    int chooseAction(int state);
    int bestAction(int state);

    void train(int state, int action, float reward, int nextState);
    bool storeWeights(const std::string& filename);
    bool loadWeights(const std::string& filename);
};

#endif // Q_LEARNING_H
