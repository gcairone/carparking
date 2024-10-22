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
     * @brief Discretizes the car's state for grid-based processing.
     * 
     * Divides the state space (x, y, theta) into discrete intervals for
     * quantization, which is often used in simulation or reinforcement learning.
     * 
     * @return An integer representing the discrete state.
     */
    int discretize_state();

    /**
     * @brief Destructor for CarState.
     * 
     * Cleans up resources associated with the car state.
     */
    ~CarState();
};




class Enviroment {
public:
    CarState car;
    QPolygonF env_polygon;
    //float reward_for_hit;
    //float reward_for_nothing;
    //float reward_for_park;
    Enviroment();
    bool car_parked();
    bool car_allowed();
    float reward();
    Enviroment compute_new_state(float speed, float steering, int timestep);
};



#endif // ENVIROMENT_H
