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

float reward_for_hit = -200;
float reward_for_park = 1000;
float reward_for_nothing = -10;



QPolygonF CarState::to_polygon() {
    float sin_t = sin(this->theta);
    float cos_t = cos(this->theta);

    float xA = x + 0.5*len_car*cos_t + 0.5*width_car*sin_t;
    float yA = y + 0.5*len_car*sin_t - 0.5*width_car*cos_t;

    float xB = x + 0.5*len_car*cos_t - 0.5*width_car*sin_t;
    float yB = y + 0.5*len_car*sin_t + 0.5*width_car*cos_t;

    float xC = x - 0.5*len_car*cos_t - 0.5*width_car*sin_t;
    float yC = y - 0.5*len_car*sin_t + 0.5*width_car*cos_t;

    float xD = x - 0.5*len_car*cos_t + 0.5*width_car*sin_t;
    float yD = y - 0.5*len_car*sin_t - 0.5*width_car*cos_t;


    QVector<QPointF> vertices = {
        QPointF(xA, yA),  // Top-right corner
        QPointF(xB, yB),  // bottom-right corner
        QPointF(xC, yC),  // bottom-left corner
        QPointF(xD, yD),  // Top-left corner
    };

    QPolygonF rectangle(vertices);

    return rectangle;
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

    return CarState(x_new, y_new, theta_new);
}

QPolygonF build_env() {
    // animation is in the top-left corner of the window
    float len_park = len_car + 2 * tol;
    float width_park = width_car + tol;
    QVector<QPointF> vertices = {
        QPointF(0, 0),  // top-left corner
        QPointF(0, len_env),  //   Bottom-left corner
        QPointF(width_env-width_park, len_env),  // Bottom-right corner
        QPointF(width_env-width_park, len_env-free_park*len_park), // PARK CORNER  bottom-right
        QPointF(width_env, len_env-free_park*len_park), // PARK CORNER  bottom-left
        QPointF(width_env, len_env-(free_park+1)*len_park), // PARK CORNER  top-left
        QPointF(width_env-width_park, len_env-(free_park+1)*len_park),  // PARK CORNER  top-right
        QPointF(width_env-width_park, 0),  // Top-right corner
    };

    QPolygonF env(vertices);

    return env;

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


Enviroment::Enviroment() {
    car = CarState();
    float len_park = len_car + 2 * tol;
    float width_park = width_car + tol;
    QVector<QPointF> vertices = {
        QPointF(0, 0),  // top-left corner
        QPointF(0, len_env),  //   Bottom-left corner
        QPointF(width_env-width_park, len_env),  // Bottom-right corner
        QPointF(width_env-width_park, len_env-free_park*len_park), // PARK CORNER  bottom-right
        QPointF(width_env, len_env-free_park*len_park), // PARK CORNER  bottom-left
        QPointF(width_env, len_env-(free_park+1)*len_park), // PARK CORNER  top-left
        QPointF(width_env-width_park, len_env-(free_park+1)*len_park),  // PARK CORNER  top-right
        QPointF(width_env-width_park, 0),  // Top-right corner
    };
    env_polygon = QPolygonF(vertices);

}


bool Enviroment::car_allowed() {
    QPolygonF car_rect = car.to_polygon();

    for(auto point: car_rect) {
        if(!env_polygon.containsPoint(point, Qt::OddEvenFill)) {
            return false;
        }
    }
    if(car_rect.containsPoint(env_polygon[3], Qt::OddEvenFill)) {
        return false;
    }
    if(car_rect.containsPoint(env_polygon[6], Qt::OddEvenFill)) {
        return false;
    }

    return true;
}


bool Enviroment::car_parked() {
    QVector<QPointF> car_rect = car.to_polygon();

    QPolygonF parkspace({env_polygon[3], env_polygon[4], env_polygon[5], env_polygon[6]});

    for(auto point: car_rect) {
        if(!parkspace.containsPoint(point, Qt::OddEvenFill)) return false;
    }
    return true;
}


float Enviroment::reward() {
    float r;
    if(!car_allowed()) r= reward_for_hit;
    else if(car_parked()) r= reward_for_park;
    else r= reward_for_nothing;
    return r;
}

Enviroment Enviroment::compute_new_state(float speed, float steering, int timestep) {
    Enviroment e = *this;
    e.car = e.car.compute_new_state(speed, steering, timestep);
    return e;
}