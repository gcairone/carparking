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
    std::vector<std::vector<float>> q_table;
    int state_count;
    int action_count;

    float lr_max;
    float lr_min;
    float lr_half_life;

    float discount_factor;

    float exploration_rate_max;
    float exploration_rate_min;
    float er_half_life;

    std::mt19937 rng; // Mersenne Twister random number generator

public:
    float lr;
    float exploration_rate;
    QLearningModel(int state_count, 
                    int action_count, 
                    float lr_max, 
                    float discount_factor,
                    float exploration_rate_max);

    int chooseAction(int state);
    int bestAction(int state);

    void train(int state, int action, float reward, int nextState);
    bool storeWeights(const std::string& filename);
    bool loadWeights(const std::string& filename);
};

#endif // Q_LEARNING_H
