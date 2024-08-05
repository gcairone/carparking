#include "mainwindow.h"
#include "ui_mainwindow.h"

// frequency of choices and screen update
#define MSEC 20 // latency in msec of timestep
#define ANIMATION_SPEED 1 // x1
#define TIME_RATIO 30 // TIME_RATIO * MSEC is the frequency of the choice, used by the controller

// video animation constansts
#define PIXEL_RATIO 20 // how many pixel is a meter
#define MARGIN 20 // distance in pixel between window and enviroment representation
#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800

// other constants
#define L_MAP 6
#define DIM_INPUT 3 // som input: (x, y, theta)



// queste si devono mettere in un futuro file agent.cpp
bool Q_weights_freezed = false;
bool SOM_weights_freezed = false;
const float speed_unity = 2;
const std::vector<float> speed_actions = {-4*speed_unity, -3*speed_unity, -2*speed_unity, -speed_unity, 0, speed_unity, 2*speed_unity, 3*speed_unity, 4*speed_unity};
const std::vector<float> steering_actions = {-M_PI/4, -3*M_PI/16, -M_PI/8, -M_PI/16, 0, M_PI/16, M_PI/8, 3*M_PI/16, M_PI/4};
int x_divide = 2;
int y_divide = 2;
int theta_divide = 4;
int state_count = x_divide * y_divide * theta_divide;


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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), som(L_MAP, L_MAP, L_MAP, DIM_INPUT), speed_controller(state_count, 9), steering_controller(state_count, 9),
      iter(0), hit_counter(0), success_counter(0)
{
    ui->setupUi(this);
    setWindowTitle("Animation Controller");
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);



    // Create timer for animation
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::iteration_animation);
    timer->setInterval(MSEC); // Adjust interval as needed

    car_st = CarState::generate_random_state();
    car_st_vect = car_st.to_vector_normalized();
    //std::cout<< state_count << std::endl;
    state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide); //state_encoded = som.findBMU(car_st_vect);




    rectangle = map_into_window(car_st.to_polygon());
    env = map_into_window(build_env());


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
    lidar = car_st.lidar_lines();
    update();
    timer->stop();

}

void MainWindow::iteration_animation()
{
    if(iter % TIME_RATIO == 0) iteration_with_choice();
    else iteration();
    iter++;
    update();
}

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



void MainWindow::iteration() {
    int speed_action = speed_controller.chooseAction(state_encoded);
    int steering_action = steering_controller.chooseAction(state_encoded);

    auto new_car_st = car_st.get_new_state(speed_actions[speed_action], steering_actions[steering_action], ANIMATION_SPEED*MSEC);
    auto new_car_st_vect = new_car_st.to_vector_normalized();

    auto new_state_encoded = new_car_st.discretize_state(x_divide, y_divide, theta_divide);//auto new_state_encoded = som.findBMU(new_car_st_vect);


    if(!new_car_st.allowed()) {
        //std::cout << new_state_encoded << " not allowed " << std::endl;
        hit_counter++;
        car_st = CarState::generate_random_state();
        car_st_vect = car_st.to_vector_normalized();
        state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide); //state_encoded = som.findBMU(car_st_vect);
    }
    else if(new_car_st.parked()) {
        //std::cout << new_state_encoded << " parked " << std::endl;
        success_counter++;
        car_st = CarState::generate_random_state();
        car_st_vect = car_st.to_vector_normalized();
        state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide);
    }

    else {
       //std::cout << new_state_encoded << std::endl;
       car_st = new_car_st;
       car_st_vect = new_car_st_vect;
       state_encoded = new_state_encoded;
    }
}

void MainWindow::iteration_with_choice() {

    int speed_action = speed_controller.chooseAction(state_encoded);
    int steering_action = steering_controller.chooseAction(state_encoded);

    auto new_car_st = car_st.get_new_state(speed_actions[speed_action], steering_actions[steering_action], TIME_RATIO * MSEC * ANIMATION_SPEED);
    auto new_car_st_vect = new_car_st.to_vector_normalized();

    auto new_state_encoded = new_car_st.discretize_state(x_divide, y_divide, theta_divide);//auto new_state_encoded = som.findBMU(new_car_st_vect);

    //if(!SOM_weights_freezed) som.trainOnline(car_st_vect, iter);

    if(!Q_weights_freezed) {
        float reward = new_car_st.reward();
        speed_controller.train(state_encoded, speed_action, reward, new_state_encoded);
        steering_controller.train(state_encoded, steering_action, reward, new_state_encoded);
    }


    if(!new_car_st.allowed()) {
        //std::cout << "Choice " << new_state_encoded << " not allowed " << std::endl;
        hit_counter++;
        car_st = CarState::generate_random_state();
        car_st_vect = car_st.to_vector_normalized();
        state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide); //state_encoded = som.findBMU(car_st_vect);
    }
    else if(new_car_st.parked()) {
        //std::cout << "Choice " << new_state_encoded << " parked " << std::endl;
        success_counter++;
        car_st = CarState::generate_random_state();
        car_st_vect = car_st.to_vector_normalized();
        state_encoded = car_st.discretize_state(x_divide, y_divide, theta_divide);
    }

    else {
       //std::cout << "Choice " << new_state_encoded << std::endl;
       car_st = new_car_st;
       car_st_vect = new_car_st_vect;
       state_encoded = new_state_encoded;
    }
}















void MainWindow::on_trainButton_clicked()
{
    int num_iter = ui->num_iterations->value();
    for(int i=0; i<num_iter; ++i) {
        iteration();
        if(i%10000==0) {
            std::cout << "Iteration " << i << " | hit: " << hit_counter << " success: " << success_counter << std::endl;
            hit_counter=0;
            success_counter=0;
        }
    }
    update();
    std::cout << "Trained "<< num_iter <<std::endl;
}




void MainWindow::on_som_load_clicked()
{
    som.loadWeights(ui->file_name->text().toStdString());
}

void MainWindow::on_som_store_clicked()
{
    som.storeWeights(ui->file_name->text().toStdString());
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

