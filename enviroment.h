#ifndef ENVIROMENT_H
#define ENVIROMENT_H
#include <QPolygon>
#include <QLineF>
#include "utils.h"
#include <vector>

class CarState {
public:
    float x;
    float y;
    float theta;
    CarState(): x(0), y(0), theta(0) {};
    CarState(float x, float y, float theta): x(x), y(y), theta(theta) {};
    QPolygonF to_polygon();
    bool allowed(); //  state is allowed if the car is inside the area, not hitting walls
    CarState get_new_state(float speed, float steering, int timestep);
    static CarState generate_random_state();
    //QVector<QLineF> lidar_lines();
    //static std::vector<float> lidar_distances(const QVector<QLineF> &v);
    bool parked();
    float reward();
    std::vector<float> to_vector_normalized();
    int discretize_state(int divide_x, int divide_y, int divide_theta); // without SOM
    ~CarState();
};






QPolygonF build_env();
//QLineF simulateLidar(const QPoint& origin, double direction, double maxDistance, const QPolygon& polygon);





#endif // ENVIROMENT_H
