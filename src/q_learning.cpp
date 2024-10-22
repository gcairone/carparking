#include "q_learning.h"

using namespace std;
QLearningModel::QLearningModel() {}

QLearningModel::QLearningModel(int state_count, int action_count, float lr_max, float discount_factor, float exploration_rate_max, float er_half_life): 
        q_table(state_count, vector<float>(action_count, 0)), 
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
        lr = lr_max;
        exploration_rate = exploration_rate_max;
        lr_ratio = pow(0.5, 1.0/lr_half_life);
        er_ratio = pow(0.5, 1.0/er_half_life);
        reset();
}

QLearningModel::QLearningModel(map<string, string> config) {
    state_count = stoi(config["X_DIVIDE"]) * stoi(config["Y_DIVIDE"]) * stoi(config["THETA_DIVIDE"]);

    action_count = stoi(config["N_ACTIONS"]);

    lr_max = stof(config["LEARNING_RATE"]);
    lr_min = lr_max;
    lr_half_life = 1000; // no life
    discount_factor = stof(config["DISCOUNT_FACTOR"]);
    exploration_rate_max = stof(config["ER_MAX"]);
    exploration_rate_min = 0;
    er_half_life = stof(config["ER_HALF_LIFE"]);

    lr = lr_max;
    exploration_rate = exploration_rate_max;
    lr_ratio = pow(0.5, 1.0/lr_half_life);
    er_ratio = pow(0.5, 1.0/er_half_life);

    q_table.resize(state_count); 

    for (int i = 0; i < state_count; ++i) {
        q_table[i].resize(action_count, 0.0f);
    }


    reset();

}

void QLearningModel::reset() {
    random_device rd;
    rng.seed(rd());

    uniform_real_distribution<float> distribution(0.0, 1.0);
    for (int i = 0; i < state_count; ++i) {
        for (int j = 0; j < action_count; ++j) {
            q_table[i][j] = distribution(rng);
        }
    }

}

int QLearningModel::chooseAction(int state, bool eval) {
    if(state >= state_count) {
        cerr << "non valid state " << state << endl;
        throw 0;
    }
    uniform_real_distribution<float> distribution(0.0, 1.0);
    if (distribution(rng) < exploration_rate && !eval) {
        uniform_int_distribution<int> actionDistribution(0, q_table[state].size() - 1);
        return actionDistribution(rng);
    } else {
        auto maxAction = max_element(q_table[state].begin(), q_table[state].end());
        return distance(q_table[state].begin(), maxAction);
    }
}

float QLearningModel::train(int state, int action, float reward, int nextState) {
    if(state >= state_count) {
        cerr << "non valid state " << state << endl;
        throw 0;
    }
    if(nextState >= state_count) {
        cerr << "non valid state " << nextState << endl;
        throw 0;
    }
    if(action >= action_count) {
        cerr << "non valid action " << action << endl;
        throw 0;
    }

    float maxNextQValue = *max_element(q_table[nextState].begin(), q_table[nextState].end());
    float tdr = reward + discount_factor * maxNextQValue - q_table[state][action];
    q_table[state][action] += lr * tdr;

    exploration_rate = exploration_rate_min + er_ratio*(exploration_rate - exploration_rate_min);
    lr = lr_min + lr_ratio*(lr - lr_min);

    return tdr;
}



bool QLearningModel::storeWeights(const string& filename) {
    string filepath = "parameters/" + filename; // Adjust the directory path here

    ofstream file(filepath);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filepath << endl;
        return false;
    }

    for (const auto& qrow : q_table) {
        for (const auto& value : qrow) {
            file << value << " ";
        }
        file << endl;
    }
    cout << "q_table stored in " << filename << endl;

    file.close();
    return true;
}

bool QLearningModel::loadWeights(const string& filename) {
    string filepath = "parameters/" + filename; // Adjust the directory path here

    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return false;
    }

    q_table.clear();

    vector<float> neuronWeights;
    float weight;
    while (file >> weight) {
        neuronWeights.push_back(weight);
        if (neuronWeights.size() == (unsigned int)(action_count)) {
            q_table.push_back(neuronWeights);
            neuronWeights.clear();
        }
    }
    cout << "q_table loaded from " << filename << endl;

    file.close();
    return true;
}
