#ifndef ENVIROMENT_H
#define ENVIROMENT_H

#include <QPolygon>
#include <QLineF>
#include "utils.h"
#include <vector>

/**
 * @brief Class that contains the state of the car.
 * 
 * The CarState class represents the state of a car in terms of its position (x, y),
 * orientation (theta), and provides methods to calculate new states, check
 * validity, and compute rewards based on the car's configuration.
 */
class CarState {
public:
    float x;     ///< The x-coordinate of the car.
    float y;     ///< The y-coordinate of the car.
    float theta; ///< The orientation of the car in radians (angle).

    /**
     * @brief Default constructor for CarState.
     * 
     * Initializes the car's state with default values (x = 0, y = 0, theta = 0).
     */
    CarState() : x(0), y(0), theta(0) {};

    /**
     * @brief Constructor for CarState with specified values.
     * 
     * @param x The x-coordinate of the car.
     * @param y The y-coordinate of the car.
     * @param theta The orientation (angle) of the car in radians.
     */
    CarState(float x, float y, float theta) : x(x), y(y), theta(theta) {};

    /**
     * @brief Converts the car's state to a polygon representation.
     * 
     * @return A QPolygonF representing the car's shape in the current state.
     */
    QPolygonF to_polygon();

    /**
     * @brief Checks if the current state of the car is valid.
     * 
     * The state is valid if the car is inside the designated area and does not
     * collide with walls or other obstacles.
     * 
     * @return true if the state is allowed, false otherwise.
     */
    bool allowed();

    /**
     * @brief Computes the car's new state based on speed and steering input.
     * 
     * Calculates the new position and orientation of the car given the speed, 
     * steering angle, and time step.
     * 
     * @param speed The speed of the car.
     * @param steering The steering angle in radians.
     * @param timestep The time step for the state change.
     * @return The new CarState after applying the inputs.
     */
    CarState compute_new_state(float speed, float steering, int timestep);

    /**
     * @brief Generates a random car state.
     * 
     * Creates a random state for the car, typically used for exploration or
     * initializing a simulation.
     * 
     * @return A randomly generated CarState.
     */
    static CarState generate_random_state();

    /**
     * @brief Checks if the car is parked correctly.
     * 
     * Determines if the car is in a valid parked position based on predefined
     * conditions.
     * 
     * @return true if the car is parked correctly, false otherwise.
     */
    bool parked();

    /**
     * @brief Computes a reward based on the car's state.
     * 
     * The reward function returns a score that can be used in reinforcement learning
     * algorithms to assess the car's performance based on its state.
     * 
     * @return A float representing the reward value.
     */
    float reward();

    /**
     * @brief Converts the car's state to a normalized vector.
     * 
     * Returns a vector where each state component (x, y, theta) is normalized to a
     * scale suitable for input into machine learning models or algorithms.
     * 
     * @return A vector of normalized state values.
     */
    std::vector<float> to_vector_normalized();

    /**
     * @brief Discretizes the car's state for grid-based processing.
     * 
     * Divides the state space (x, y, theta) into discrete intervals for
     * quantization, which is often used in simulation or reinforcement learning.
     * 
     * @param divide_x The number of divisions for the x-coordinate.
     * @param divide_y The number of divisions for the y-coordinate.
     * @param divide_theta The number of divisions for the theta (angle).
     * @return An integer representing the discrete state.
     */
    int discretize_state(int divide_x, int divide_y, int divide_theta);

    /**
     * @brief Destructor for CarState.
     * 
     * Cleans up resources associated with the car state.
     */
    ~CarState();
};



/**
* @brief Builder of the enviroment
* Build the enviroment, with walls and the parking slot
* 
* @return A QPolygonF object that contains the entire enviroment 
*/
QPolygonF build_env();
//QLineF simulateLidar(const QPoint& origin, double direction, double maxDistance, const QPolygon& polygon);





#endif // ENVIROMENT_H
