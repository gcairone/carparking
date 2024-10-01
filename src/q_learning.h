#ifndef Q_LEARNING_H
#define Q_LEARNING_H

#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

/**
 * @brief Class that implements a Q-Learning model for reinforcement learning.
 * 
 * This class manages the Q-table for storing state-action values and provides methods
 * to train the model, choose actions, and adjust learning and exploration rates over time.
 */
class QLearningModel {
private:
    std::vector<std::vector<float>> q_table; ///< Q-table storing the Q-values for each state-action pair.
    int state_count;  ///< The total number of states in the environment.
    int action_count; ///< The total number of actions available in each state.

    float lr_max;          ///< The maximum learning rate.
    float lr_min;          ///< The minimum learning rate.
    float lr_half_life;    ///< The half-life for reducing the learning rate.
    float lr_ratio;        ///< The ratio by which the learning rate decreases each iteration.

    float discount_factor; ///< The discount factor for future rewards.

    float exploration_rate_max; ///< The maximum exploration rate (epsilon in epsilon-greedy).
    float exploration_rate_min; ///< The minimum exploration rate.
    float er_half_life;         ///< The half-life for reducing the exploration rate.
    float er_ratio;             ///< The ratio by which the exploration rate decreases over time.

    std::mt19937 rng; ///< Mersenne Twister random number generator for stochastic actions.

public:
    float lr;             ///< Current learning rate (adjusted over time).
    float exploration_rate; ///< Current exploration rate (epsilon, adjusted over time).

    /**
     * @brief Constructor for the QLearningModel class.
     * 
     * Initializes the Q-learning model with a given state and action space,
     * and sets the initial learning and exploration rates.
     * 
     * @param state_count The number of states in the environment.
     * @param action_count The number of actions available per state.
     * @param lr_max The initial (maximum) learning rate.
     * @param discount_factor The discount factor for future rewards.
     * @param exploration_rate_max The initial (maximum) exploration rate.
     * @param er_half_life The half-life used to decrease the exploration rate.
     */
    QLearningModel(int state_count, 
                   int action_count, 
                   float lr_max, 
                   float discount_factor,
                   float exploration_rate_max,
                   float er_half_life);

    /**
     * @brief Chooses an action using the epsilon-greedy strategy.
     * 
     * With probability equal to the current exploration rate, it chooses a random action.
     * Otherwise, it chooses the best action for the given state.
     * 
     * @param state The current state for which to choose an action.
     * @return The index of the action chosen (either random or best action).
     */
    int chooseAction(int state, bool eval);

    /**
     * @brief Returns the best action for a given state.
     * 
     * Selects the action that has the highest Q-value for the specified state.
     * If in evaluation mode, it does not choose randomly
     * 
     * @param state The current state.
     * @param eval
     * @return The index of the best action for the given state.
     */
    int bestAction(int state);

    /**
     * @brief Updates the Q-table based on the action taken.
     * 
     * Performs the Q-learning update for the Q-value of the (state, action) pair
     * based on the reward received and the next state.
     * 
     * @param state The current state.
     * @param action The action taken in the current state.
     * @param reward The reward received after taking the action.
     * @param nextState The next state after taking the action.
     */
    void train(int state, int action, float reward, int nextState);

    /**
     * @brief Stores the Q-table to a file.
     * 
     * Saves the current Q-table to a specified file, allowing the model to be
     * reloaded and reused later.
     * 
     * @param filename The name of the file where the Q-table will be stored.
     * @return true if the Q-table was successfully saved, false otherwise.
     */
    bool storeWeights(const std::string& filename);

    /**
     * @brief Loads the Q-table from a file.
     * 
     * Loads a previously saved Q-table from a file, restoring the model to its
     * saved state.
     * 
     * @param filename The name of the file from which to load the Q-table.
     * @return true if the Q-table was successfully loaded, false otherwise.
     */
    bool loadWeights(const std::string& filename);
};

#endif // Q_LEARNING_H
