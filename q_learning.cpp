#include "q_learning.h"
#include <algorithm>

QLearningModel::QLearningModel(int stateCount, int actionCount, float learningRate, float discountFactor, float explorationRate_max)
    : qTable(stateCount, std::vector<float>(actionCount, 0)), stateCount(stateCount), actionCount(actionCount), learningRate(learningRate),
      discountFactor(discountFactor), explorationRate_max(explorationRate_max), explorationRate(explorationRate_max) {
        std::random_device rd;
        rng.seed(rd());

        // Initialize Q-values randomly
        std::uniform_real_distribution<float> distribution(0.0, 1.0);
        for (int i = 0; i < stateCount; ++i) {
            for (int j = 0; j < actionCount; ++j) {
                qTable[i][j] = distribution(rng);
            }
        }
}

int QLearningModel::bestAction(int state) {
    if(state >= stateCount) {
        std::cerr << "non valid state " << state << std::endl;
        throw 0;
    }

    auto maxAction = std::max_element(qTable[state].begin(), qTable[state].end());
    return std::distance(qTable[state].begin(), maxAction);
}

int QLearningModel::chooseAction(int state) {
    if(state >= stateCount) {
        std::cerr << "non valid state " << state << std::endl;
        throw 0;
    }
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    if (distribution(rng) < explorationRate) {
        // Explore
        //std::cout << "Casuale" << std::endl;
        std::uniform_int_distribution<int> actionDistribution(0, qTable[state].size() - 1);
        return actionDistribution(rng);
    } else {
        // Exploit
        //std::cout << "Migliore" << std::endl;
        auto maxAction = std::max_element(qTable[state].begin(), qTable[state].end());
        return std::distance(qTable[state].begin(), maxAction);
    }
}

void QLearningModel::train(int state, int action, float reward, int nextState) {
    if(state >= stateCount) {
        std::cerr << "non valid state " << state << std::endl;
        throw 0;
    }
    if(nextState >= stateCount) {
        std::cerr << "non valid state " << nextState << std::endl;
        throw 0;
    }
    if(action >= actionCount) {
        std::cerr << "non valid action " << action << std::endl;
        throw 0;
    }

    float maxNextQValue = *std::max_element(qTable[nextState].begin(), qTable[nextState].end());
    float QTarget = reward + discountFactor * maxNextQValue;
    qTable[state][action] += learningRate * (QTarget - qTable[state][action]);

    //explorationRate *= 0.99999;
}



bool QLearningModel::storeWeights(const std::string& filename) {
    std::string filepath = "parameters/" + filename; // Adjust the directory path here

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    for (const auto& qrow : qTable) {
        for (const auto& value : qrow) {
            file << value << " ";
        }
        file << std::endl;
    }
    std::cout << "QTable stored in " << filename << std::endl;

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

    qTable.clear();

    std::vector<float> neuronWeights;
    float weight;
    while (file >> weight) {
        neuronWeights.push_back(weight);
        if (neuronWeights.size() == (unsigned int)(actionCount)) {
            qTable.push_back(neuronWeights);
            neuronWeights.clear();
        }
    }
    std::cout << "QTable loaded from " << filename << std::endl;

    file.close();
    return true;
}
