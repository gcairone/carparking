#include "q_learning.h"
#include <algorithm>

QLearningModel::QLearningModel(int state_count, int action_count, float lr_max, float discount_factor, float exploration_rate_max, float er_half_life): 
        q_table(state_count, std::vector<float>(action_count, 0)), 
        state_count(state_count), 
        action_count(action_count), 
        lr_max(lr_max),
        lr_min(lr_max),
        lr_half_life(1e7),
        discount_factor(discount_factor), 
        exploration_rate_max(exploration_rate_max),
        exploration_rate_min(0),
        er_half_life(er_half_life)
{
        std::random_device rd;
        rng.seed(rd());

        lr = lr_max;
        exploration_rate = exploration_rate_max;
        lr_ratio = std::pow(0.5, 1.0/lr_half_life);
        er_ratio = std::pow(0.5, 1.0/er_half_life);

        // Initialize Q-values randomly
        std::uniform_real_distribution<float> distribution(0.0, 1.0);
        for (int i = 0; i < state_count; ++i) {
            for (int j = 0; j < action_count; ++j) {
                q_table[i][j] = distribution(rng);
            }
        }
}

int QLearningModel::bestAction(int state) {
    if(state >= state_count) {
        std::cerr << "non valid state " << state << std::endl;
        throw 0;
    }

    auto maxAction = std::max_element(q_table[state].begin(), q_table[state].end());
    return std::distance(q_table[state].begin(), maxAction);
}

int QLearningModel::chooseAction(int state) {
    if(state >= state_count) {
        std::cerr << "non valid state " << state << std::endl;
        throw 0;
    }
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    if (distribution(rng) < exploration_rate) {
        // Explore
        //std::cout << "Casuale" << std::endl;
        std::uniform_int_distribution<int> actionDistribution(0, q_table[state].size() - 1);
        return actionDistribution(rng);
    } else {
        // Exploit
        //std::cout << "Migliore" << std::endl;
        auto maxAction = std::max_element(q_table[state].begin(), q_table[state].end());
        return std::distance(q_table[state].begin(), maxAction);
    }
}

void QLearningModel::train(int state, int action, float reward, int nextState) {
    if(state >= state_count) {
        std::cerr << "non valid state " << state << std::endl;
        throw 0;
    }
    if(nextState >= state_count) {
        std::cerr << "non valid state " << nextState << std::endl;
        throw 0;
    }
    if(action >= action_count) {
        std::cerr << "non valid action " << action << std::endl;
        throw 0;
    }

    float maxNextQValue = *std::max_element(q_table[nextState].begin(), q_table[nextState].end());
    float QTarget = reward + discount_factor * maxNextQValue;
    q_table[state][action] += lr_max * (QTarget - q_table[state][action]);

    exploration_rate = exploration_rate_min + er_ratio*(exploration_rate - exploration_rate_min);
    lr = lr_min + lr_ratio*(lr - lr_min);
}



bool QLearningModel::storeWeights(const std::string& filename) {
    std::string filepath = "parameters/" + filename; // Adjust the directory path here

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    for (const auto& qrow : q_table) {
        for (const auto& value : qrow) {
            file << value << " ";
        }
        file << std::endl;
    }
    std::cout << "q_table stored in " << filename << std::endl;

    file.close();
    return true;
}

bool QLearningModel::loadWeights(const std::string& filename) {
    std::string filepath = "parameters/" + filename; // Adjust the directory path here

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    q_table.clear();

    std::vector<float> neuronWeights;
    float weight;
    while (file >> weight) {
        neuronWeights.push_back(weight);
        if (neuronWeights.size() == (unsigned int)(action_count)) {
            q_table.push_back(neuronWeights);
            neuronWeights.clear();
        }
    }
    std::cout << "q_table loaded from " << filename << std::endl;

    file.close();
    return true;
}
