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





const bool Q_weights_freezed = false;
float learning_rate = 0.003;
float discount_factor = 0.9;
float exploration_rate_max = 0.1; 
const float er_half_life = 1e7;


//bool SOM_weights_freezed = false;
const float speed_unity = 2;
const std::vector<float> speed_actions = {
    -2*speed_unity, 
    -1*speed_unity, 
    1*speed_unity, 
    2*speed_unity
};
const std::vector<float> steering_actions = {
    -M_PI/4, 
    -M_PI/8, 
    0, 
    M_PI/8, 
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

    env = Enviroment();
    env.car = CarState::generate_random_state();;
    state_encoded = env.car.discretize_state(x_divide, y_divide, theta_divide); //state_encoded = som.findBMU(car_st_vect);

    car_picture = map_into_window(env.car.to_polygon());
    env_picture = map_into_window(env.env_polygon);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playButton_clicked()
{   
    speed_controller.lr = ui->learning_rate->value();
    steering_controller.lr = ui->learning_rate->value();
    speed_controller.exploration_rate = ui->epsilon->value();
    steering_controller.exploration_rate = ui->epsilon->value();
    speed_controller.exploration_rate_max = ui->epsilon->value();
    steering_controller.exploration_rate_max = ui->epsilon->value();

    timer->start();
}

void MainWindow::on_pauseButton_clicked()
{
    timer->stop();
}

void MainWindow::on_stopButton_clicked()
{
    env.car = CarState::generate_random_state();;
    car_picture = map_into_window(env.car.to_polygon());
    update();
    timer->stop();

}


void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawPolygon(env_picture);
    painter.setBrush(Qt::blue); // Set car_picture color
    car_picture = map_into_window(env.car.to_polygon());
    painter.drawPolygon(car_picture);

    painter.setBrush(Qt::red); // Set car_picture color
    painter.drawEllipse(map_into_window(QPointF(env.car.x, env.car.y)), 2, 2);


}



void MainWindow::enviroment_iteration(int timestep) {
    int speed_action = last_speed_action;
    int steering_action = last_steering_action;

    auto new_env_st = env.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], timestep);


    auto new_state_encoded = new_env_st.car.discretize_state(x_divide, y_divide, theta_divide);


    if(!new_env_st.car_allowed()) {
        hit_counter++;
        env.car = CarState::generate_random_state();;
        state_encoded = env.car.discretize_state(x_divide, y_divide, theta_divide); 
        last_speed_action = speed_controller.chooseAction(state_encoded, ui->eval->isChecked());
        last_steering_action = steering_controller.chooseAction(state_encoded, ui->eval->isChecked());
    }
    else if(new_env_st.car_parked()) {
        success_counter++;
        env.car = CarState::generate_random_state();;
        state_encoded = env.car.discretize_state(x_divide, y_divide, theta_divide);
        last_speed_action = speed_controller.chooseAction(state_encoded, ui->eval->isChecked());
        last_steering_action = steering_controller.chooseAction(state_encoded, ui->eval->isChecked());
    }

    else {
       env = new_env_st;
       state_encoded = new_state_encoded;
    }
}

void MainWindow::model_iteration() {

    int speed_action = speed_controller.chooseAction(state_encoded, ui->eval->isChecked());
    int steering_action = steering_controller.chooseAction(state_encoded, ui->eval->isChecked());
    last_speed_action = speed_action;
    last_steering_action = steering_action;
    //CarState new_car_st = env.car.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], MSEC*TIME_RATIO * ANIMATION_SPEED);
    Enviroment new_env_st = env.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], MSEC*TIME_RATIO * ANIMATION_SPEED);


    auto new_state_encoded = new_env_st.car.discretize_state(x_divide, y_divide, theta_divide);//auto new_state_encoded = som.findBMU(new_car_st_vect);

    if(!ui->eval->isChecked()) {

        float reward = new_env_st.reward();
        speed_controller.train(state_encoded, speed_action, reward, new_state_encoded);
        steering_controller.train(state_encoded, steering_action, reward, new_state_encoded);
    }

}






void MainWindow::on_trainButton_clicked()
{
    speed_controller.lr = ui->learning_rate->value();
    steering_controller.lr = ui->learning_rate->value();
    speed_controller.exploration_rate = ui->epsilon->value();
    steering_controller.exploration_rate = ui->epsilon->value();
    speed_controller.exploration_rate_max = ui->epsilon->value();
    steering_controller.exploration_rate_max = ui->epsilon->value();

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
        }
    }
    
    std::cout << "Trained "<< num_iter <<std::endl;
    file.close();
}


void MainWindow::on_resetButton_clicked() {
    speed_controller.reset();
    steering_controller.reset();
    std::cout << "Q-Table reset" << std::endl;
}

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

