#include "mainwindow.h"
#include "ui_mainwindow.h"

// frequency of choices and screen update

#define MSEC 30 // latency in msec of timestep
#define ANIMATION_SPEED 1 // x1
#define TIME_RATIO 10 // TIME_RATIO * MSEC is the frequency of the choice, used by the controller

// video animation constants
#define PIXEL_RATIO 30 // how many pixel is a meter
#define MARGIN 30 // distance in pixel between window and enviroment representation
#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

// other constants
// #define L_MAP 6
// #define DIM_INPUT 3 // som input: (x, y, theta)




const bool Q_weights_freezed = false;
float learning_rate = 0.003;
float discount_factor = 0.9;
float exploration_rate_max = 0.1; 
const float er_half_life = 1e7;


//bool SOM_weights_freezed = false;
const float speed_unity = 1;
const std::vector<float> speed_actions = {
    -4*speed_unity, 
    -3*speed_unity, 
    -2*speed_unity, 
    -speed_unity, 
    0.1*speed_unity,
    speed_unity, 
    2*speed_unity, 
    3*speed_unity, 
    4*speed_unity
};
const std::vector<float> steering_actions = {
    -M_PI/4, 
    -3*M_PI/16, 
    -M_PI/8, 
    -M_PI/16, 
    0, 
    M_PI/16, 
    M_PI/8, 
    3*M_PI/16, 
    M_PI/4
};
const int x_divide = 8;
const int y_divide = 8;
const int theta_divide = 20;
const int state_count = x_divide * y_divide * theta_divide;


QPoint map_into_window(const QPointF &p) {
    return QPoint((int)(MARGIN + PIXEL_RATIO*p.x()), (int)(MARGIN + PIXEL_RATIO*p.y()));
}

QPolygon map_into_window(const QPolygonF &p) {
    QPolygon polygon;

    for(auto point: p) {
        polygon << map_into_window(point);
    }

    return polygon;
}

QLine map_into_window(const QLineF &l) {
    return QLine(map_into_window(l.p1()), map_into_window(l.p2()));
}

QVector<QLine> map_into_window(const QVector<QLineF> &v) {
    QVector<QLine> r;
    for(auto line: v) {
        r.push_back(map_into_window(line));
    }
    return r;
}

MainWindow::MainWindow(QWidget *parent): 
    QMainWindow(parent), 
    ui(new Ui::MainWindow), 
    speed_controller(state_count, speed_actions.size(), learning_rate, discount_factor, exploration_rate_max, er_half_life), 
    steering_controller(state_count, steering_actions.size(), learning_rate, discount_factor, exploration_rate_max, er_half_life),
    iter(0), 
    hit_counter(0), 
    success_counter(0),
    last_speed_action(0),
    last_steering_action(0)
{
    ui->setupUi(this);
    setWindowTitle("Animation Controller");
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    ui->learning_rate->setValue(learning_rate);
    ui->discount_factor->setValue(discount_factor);
    ui->epsilon->setValue(exploration_rate_max);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if(iter % TIME_RATIO == 0) model_iteration();
        enviroment_iteration(ANIMATION_SPEED*MSEC);
        iter++;
        update();
    });
    timer->setInterval(ANIMATION_SPEED*MSEC); 

    car_st = CarState::generate_random_state();
    //car_st = CarState(3, 3, M_PI*0.5);
    //car_st_vect = car_st.to_vector_normalized();
    //std::cout<< state_count << std::endl;
    state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide); //state_encoded = som.findBMU(car_st_vect);

    rectangle = map_into_window(car_st.to_polygon());
    env = map_into_window(build_env());

    //std::cout << "Distanza percorsa tra scelte " << 2.0 *speed_unity * TIME_RATIO * MSEC / 1000.0 << std::endl;


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playButton_clicked()
{   
    timer->start();
}

void MainWindow::on_pauseButton_clicked()
{
    timer->stop();
}

void MainWindow::on_stopButton_clicked()
{
    car_st = CarState::generate_random_state();
    rectangle = map_into_window(car_st.to_polygon());
    //lidar = car_st.lidar_lines();
    update();
    timer->stop();

}
/*
void MainWindow::iteration_animation()
{
    if(iter % TIME_RATIO == 0) model_iteration();
    iteration();
    iter++;
    update();
}
*/

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawPolygon(env);
    painter.setBrush(Qt::blue); // Set rectangle color
    rectangle = map_into_window(car_st.to_polygon());
    painter.drawPolygon(rectangle);

    painter.setBrush(Qt::red); // Set rectangle color
    painter.drawEllipse(map_into_window(QPointF(car_st.x, car_st.y)), 2, 2);


}



void MainWindow::enviroment_iteration(int timestep) {
    int speed_action = last_speed_action;
    int steering_action = last_steering_action;

    auto new_car_st = car_st.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], timestep);
    //auto new_car_st_vect = new_car_st.to_vector_normalized();

    auto new_state_encoded = new_car_st.discretize_state(x_divide, y_divide, theta_divide);//auto new_state_encoded = som.findBMU(new_car_st_vect);


    if(!new_car_st.allowed()) {
        //std::cout << new_state_encoded << " not allowed " << std::endl;
        hit_counter++;
        //std::cout << "HIT" << std::endl;
        car_st = CarState::generate_random_state();
        state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide); //state_encoded = som.findBMU(car_st_vect);
        last_speed_action = speed_controller.chooseAction(state_encoded, ui->eval->isChecked());
        last_steering_action = steering_controller.chooseAction(state_encoded, ui->eval->isChecked());
    }
    else if(new_car_st.parked()) {
        //std::cout << new_state_encoded << " parked " << std::endl;
        success_counter++;
        //std::cout << "PARK" << std::endl;
        car_st = CarState::generate_random_state();
        state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide);
        last_speed_action = speed_controller.chooseAction(state_encoded, ui->eval->isChecked());
        last_steering_action = steering_controller.chooseAction(state_encoded, ui->eval->isChecked());
    }

    else {
       //std::cout << new_state_encoded << "allowed "<<std::endl;
       car_st = new_car_st;
       //car_st_vect = new_car_st_vect;
       state_encoded = new_state_encoded;
    }
}

void MainWindow::model_iteration() {

    int speed_action = speed_controller.chooseAction(state_encoded, ui->eval->isChecked());
    int steering_action = steering_controller.chooseAction(state_encoded, ui->eval->isChecked());
    last_speed_action = speed_action;
    last_steering_action = steering_action;
    auto new_car_st = car_st.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], MSEC*TIME_RATIO * ANIMATION_SPEED);
    //auto new_car_st_vect = new_car_st.to_vector_normalized();

    auto new_state_encoded = new_car_st.discretize_state(x_divide, y_divide, theta_divide);//auto new_state_encoded = som.findBMU(new_car_st_vect);
    ui->epsilon->setValue(speed_controller.exploration_rate);

    //if(!SOM_weights_freezed) som.trainOnline(car_st_vect, iter);

    if(!ui->eval->isChecked()) {
        float reward = new_car_st.reward();
        speed_controller.train(state_encoded, speed_action, reward, new_state_encoded);
        steering_controller.train(state_encoded, steering_action, reward, new_state_encoded);
    }

    /*
    if(!new_car_st.allowed()) {
        //std::cout << "Choice " << new_state_encoded << " not allowed " << std::endl;
        hit_counter++;
        car_st = CarState::generate_random_state();
        //car_st_vect = car_st.to_vector_normalized();
        state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide); //state_encoded = som.findBMU(car_st_vect);
        //std::cout << "state: " << state_encoded << " speed: " << speed_actions[speed_action] << " steering " << steering_actions[steering_action] << "hit "<<std::endl;

    }
    else if(new_car_st.parked()) {
        //std::cout << "Choice " << new_state_encoded << " parked " << std::endl;
        success_counter++;
        car_st = CarState::generate_random_state();
        //car_st_vect = car_st.to_vector_normalized();
        state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide);
        //std::cout << "state: " << state_encoded << " speed: " << speed_actions[speed_action] << " steering " << steering_actions[steering_action] << "parked "<<std::endl;

    }

    else {
        //std::cout << "Choice " << new_state_encoded << std::endl;
        //car_st = new_car_st;
        //car_st_vect = new_car_st_vect;
        //state_encoded = new_state_encoded;
        //std::cout << "state: " << state_encoded << " speed: " << speed_actions[speed_action] << " steering " << steering_actions[steering_action] << std::endl;
        //std::cout << "x: " << car_st.x << " y: " << car_st.y << " theta: " << car_st.theta << std::endl;
    }
    */
}















void MainWindow::on_trainButton_clicked()
{
    std::ofstream file("log/log0.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open file: log/log0.txt" << std::endl;
    }

    int num_iter = ui->num_iterations->value();
    for(int i=0; i<num_iter; ++i) {
        model_iteration();
        enviroment_iteration(ANIMATION_SPEED*MSEC*TIME_RATIO);
        if(i%1000000==0) {
            std::cout << "{\"iteration\": \"" << i << "\", \"hit\": \"" << hit_counter << "\", \"success\": \"" << success_counter << "\", \"success_ratio\": \""<< 100 * success_counter / (float)(success_counter+hit_counter) << '%' << "\", \"lr\": \""<< speed_controller.lr << "\", \"er\": \"" << speed_controller.exploration_rate << "\"}"<< std::endl;
            file << "{\"iteration\": \"" << i << "\", \"hit\": \"" << hit_counter << "\", \"success\": \"" << success_counter << "\", \"success_ratio\": \""<< 100 * success_counter / (float)(success_counter+hit_counter) << '%' << "\", \"lr\": \""<< speed_controller.lr << "\", \"er\": \"" << speed_controller.exploration_rate << "\"}"<< std::endl;
            hit_counter=0;
            success_counter=0;
            update();
            // update hyperparameter values on interface
            //ui->epsilon->setValue(speed_controller.exploration_rate);
            QCoreApplication::processEvents();
        }
    }
    
    std::cout << "Trained "<< num_iter <<std::endl;
    file.close();
}



/*
void MainWindow::on_som_load_clicked()
{
    som.loadWeights(ui->file_name->text().toStdString());
}

void MainWindow::on_som_store_clicked()
{
    som.storeWeights(ui->file_name->text().toStdString());
}
*/

void MainWindow::on_qtable_load_sp_clicked()
{
    speed_controller.loadWeights(ui->file_name->text().toStdString());
}

void MainWindow::on_qtable_store_sp_clicked()
{
    speed_controller.storeWeights(ui->file_name->text().toStdString());
}

void MainWindow::on_qtable_load_st_clicked()
{
    steering_controller.loadWeights(ui->file_name->text().toStdString());
}

void MainWindow::on_qtable_store_st_clicked()
{
    steering_controller.storeWeights(ui->file_name->text().toStdString());
}

