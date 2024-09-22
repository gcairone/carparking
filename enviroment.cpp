#include "enviroment.h"
#include <cmath>
#include <QLineF>
#define APPROX_MOTION true // if true, it compute next state with approximate kinematics

float len_car = 4;
float width_car = 1.7;
float len_env = 15;   // y-axis  
float width_env = 8; // x-axis
float tol = 1.5;                    
float free_park = 0.6;

float reward_for_hit = -100;
float reward_for_park = 1000;
float reward_for_nothing = -1;



QPolygonF CarState::to_polygon() {
    float sin_t = sin(this->theta);
    float cos_t = cos(this->theta);
    /*
     con il punto (x, y) a metà del lato corto
    float xA = len_car * cos_t + 0.5 * width_car * sin_t + x;
    float yA = len_car * sin_t - 0.5 * width_car * cos_t + y;

    float xB = len_car * cos_t - 0.5 * width_car * sin_t + x;
    float yB = len_car * sin_t + 0.5 * width_car * cos_t + y;

    float xC = - 0.5 * width_car * sin_t + x;
    float yC = 0.5 * width_car * cos_t + y;

    float xD = 0.5 * width_car * sin_t + x;
    float yD = - 0.5 * width_car * cos_t + y;
    */
    float xA = x + 0.5*len_car*cos_t + 0.5*width_car*sin_t;
    float yA = y + 0.5*len_car*sin_t - 0.5*width_car*cos_t;

    //std::cout << x << std::endl;
    //std::cout << sin_t << std::endl;

    float xB = x + 0.5*len_car*cos_t - 0.5*width_car*sin_t;
    float yB = y + 0.5*len_car*sin_t + 0.5*width_car*cos_t;

    float xC = x - 0.5*len_car*cos_t - 0.5*width_car*sin_t;
    float yC = y - 0.5*len_car*sin_t + 0.5*width_car*cos_t;

    float xD = x - 0.5*len_car*cos_t + 0.5*width_car*sin_t;
    float yD = y - 0.5*len_car*sin_t - 0.5*width_car*cos_t;


    // Trasforma i vertici in pixel
    QVector<QPointF> vertices = {
        QPointF(xA, yA),  // Top-right corner
        QPointF(xB, yB),  // bottom-right corner
        QPointF(xC, yC),  // bottom-left corner
        QPointF(xD, yD),  // Top-left corner
    };

    QPolygonF rectangle(vertices);

    return rectangle;
}


bool CarState::allowed() {
    // returns false if hit walls
    QPolygonF car_rect = this->to_polygon();

    QPolygonF env_poly = build_env();

    for(auto point: car_rect) {
        if(!env_poly.containsPoint(point, Qt::OddEvenFill)) {
            return false;
        }
    }
    if(car_rect.containsPoint(env_poly[3], Qt::OddEvenFill)) {
        return false;
    }
    if(car_rect.containsPoint(env_poly[6], Qt::OddEvenFill)) {
        return false;
    }

    return true;
}

CarState::~CarState() {}



CarState CarState::compute_new_state(float speed, float steering, int timestep) {
    float dt = timestep / 1000.0;

    if(APPROX_MOTION) {
        float x_r = x - 0.5 * len_car * cos(this->theta); // x ruota dietro
        float y_r = y - 0.5 * width_car * sin(this->theta); // y ruota dietro

        float x_r_next = x_r + dt * cos(this->theta) * speed;
        float y_r_next = y_r + dt * sin(this->theta) * speed;

        float x_next = x_r_next + 0.5*len_car*cos(this->theta);
        float y_next = y_r_next + 0.5*width_car*sin(this->theta);
        float theta_next = theta - dt * sin(steering) * speed / len_car;

        return CarState(x_next, y_next, theta_next);
    }
    else {
        if(steering==0.0) { // if there is no steering, rotation do not make sense
            return CarState(
                x + dt*speed*cos(theta), 
                y + dt*speed*sin(theta),
                theta
            );
        }
        // angle of rotation
        float alpha = dt*speed*tan(steering)/len_car;
        // center of rotation
        float x_cr = x - 0.5*len_car*cos(theta) - len_car*sin(theta)/tan(steering);
        float y_cr = y - 0.5*len_car*sin(theta) + len_car*cos(theta)/tan(steering);
        // next state as rotation of (x, y, theta) respect to (x_cr, y_cr) of angle alpha
        float x_next = (x-x_cr)*cos(alpha) - (y-y_cr)*sin(alpha) + x_cr;
        float y_next = (x-x_cr)*sin(alpha) + (y-y_cr)*cos(alpha) + y_cr;
        float theta_next = theta-alpha;
        
        if(std::isnan(x_next)) {
            std::cout << "TROVATO NAN" << " alpha: " << alpha << " x_cr: " <<x_cr<< " y_cr: " <<y_cr << std::endl;
        }

        return CarState(x_next, y_next, theta_next);
    }
}


CarState CarState::generate_random_state() {

    float x_new = randomFloat(width_car*0.75, width_env - 1.75*width_car - tol);
    float y_new = randomFloat(len_car*0.75, len_env - len_car*0.75);
    float theta_new = M_PI*0.5;
    if(randomFloat(0.0, 1.0)>0.5) theta_new = -M_PI*0.5;
    //std::cout << "random state generated" << std::endl;

    return CarState(x_new, y_new, theta_new);
}

QPolygonF build_env() {
    // The animation is in the top-left corner of the window
    float len_park = len_car + 2 * tol;
    float width_park = width_car + tol;
    QVector<QPointF> vertices = {
        QPointF(0, 0),  // Top-left corner
        QPointF(0, len_env),  // Bottom-left corner
        QPointF(width_env-width_park, len_env),  // Bottom-right corner
        QPointF(width_env-width_park, len_env-free_park*len_park), // PARK CORNER - bottom-right
        QPointF(width_env, len_env-free_park*len_park), // PARK CORNER - bottom-left
        QPointF(width_env, len_env-(free_park+1)*len_park), // PARK CORNER - top-left
        QPointF(width_env-width_park, len_env-(free_park+1)*len_park),  // PARK CORNER - top-right
        QPointF(width_env-width_park, 0),  // Top-right corner
    };

    QPolygonF env(vertices);

    return env;

}
/*
QLineF simulateLidar(const QPointF& origin, double direction, const QPolygonF& polygon) {
    QPointF endPoint(origin.x() + lidar_maxDistance * cos(direction),
                     origin.y() + lidar_maxDistance * sin(direction));

    for (int i = 0; i<polygon.size(); ++i) {

        int j = (i + 1)%polygon.size();
        QLineF edge(polygon[i], polygon[j]);

        QPointF intersection;
        if (edge.intersects(QLineF(origin, endPoint), &intersection) == QLineF::BoundedIntersection) {
            endPoint = intersection;
        }
    }
    return QLineF(origin, endPoint);
}


QVector<QLineF> CarState::lidar_lines() {

    QVector<QPointF> origins = this->to_polygon();

    QPolygonF env = build_env();

    // Add medium points

    origins << QPointF((origins[0].x() + origins[1].x()) / 2, (origins[0].y() + origins[1].y()) / 2);
    origins << QPointF((origins[1].x() + origins[2].x()) / 2, (origins[1].y() + origins[2].y()) / 2);
    origins << QPointF((origins[2].x() + origins[3].x()) / 2, (origins[2].y() + origins[3].y()) / 2);
    origins << QPointF((origins[3].x() + origins[0].x()) / 2, (origins[3].y() + origins[0].y()) / 2);

    //
    return {
        simulateLidar(origins[0], theta - M_PI/4, env), // top-right CORNER
        simulateLidar(origins[4], theta, env), // right EDGE
        simulateLidar(origins[1], theta + M_PI/4, env), // bottom-right CORNER
        simulateLidar(origins[5], theta + M_PI/2, env), //  bottom EDGE
        simulateLidar(origins[2], theta + 3*M_PI/4, env), // bottom-left CORNER
        simulateLidar(origins[6], theta + M_PI, env), // left EDGE
        simulateLidar(origins[3], theta + 5*M_PI/4, env), // top-left CORNER
        simulateLidar(origins[7], theta + 3*M_PI/2, env), // top EDGE
    };
}

std::vector<float> CarState::lidar_distances(const QVector<QLineF> &v) {
    std::vector<float> r;
    for(auto line: v) {
        r.push_back(line.length());
        //
    }
    return r;
}
*/


bool CarState::parked() {
    QVector<QPointF> car_rect = this->to_polygon();

    QVector<QPointF> env = build_env();

    QPolygonF parkspace({env[3], env[4], env[5], env[6]});

    for(auto point: car_rect) {
        if(!parkspace.containsPoint(point, Qt::OddEvenFill)) return false;
    }
    return true;
}

float CarState::reward() {
    if(!allowed()) return reward_for_hit;
    if(parked()) return reward_for_park;
    return reward_for_nothing;
}



std::vector<float> CarState::to_vector_normalized() {
    float theta_norm = fmod(theta, 2.0 * M_PI);
    if (theta_norm < 0)
        theta_norm += 2.0 * M_PI;
    theta_norm /= (float)(2*M_PI);
    return {x/width_env, y/len_env, theta_norm};
}


int CarState::discretize_state(int divide_x, int divide_y, int divide_theta) {

    float x_section = (float)(width_env) / divide_x;
    float y_section = (float)(len_env) / divide_y;
    float theta_section = 2*M_PI / divide_theta;

    int x_state = (int)(x / x_section);
    if(x_state >= divide_x) x_state = divide_x - 1;
    int y_state = (int)(y / y_section);
    if(y_state >= divide_y) y_state = divide_y - 1;
    float theta_mod = fmod(theta, 2.0 * M_PI);
    if (theta_mod < 0)
        theta_mod += 2.0 * M_PI;
    int theta_state = (int)(theta_mod / theta_section);
    if(theta_state >= divide_theta) theta_state = divide_theta -1;
    int state_ret = theta_state + divide_theta * y_state + divide_theta * divide_y * x_state;

    return state_ret;
}





