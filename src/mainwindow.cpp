#include "mainwindow.h"
#include "ui_mainwindow.h"

// log constant
#define LOG_FREQ 1000000 // how many iterations between each log


QPoint map_into_window(const QPointF &p, int m, int r) {
    return QPoint((int)(m + r*p.x()), (int)(m + r*p.y()));
}

QPolygon map_into_window(const QPolygonF &p, int m, int r) {
    QPolygon polygon;

    for(auto point: p) {
        polygon << map_into_window(point, m, r);
    }

    return polygon;
}

QLine map_into_window(const QLineF &l, int m, int r) {
    return QLine(map_into_window(l.p1(), m, r), map_into_window(l.p2(), m, r));
}

QVector<QLine> map_into_window(const QVector<QLineF> &v, int m, int r) {
    QVector<QLine> vi;
    for(auto line: v) {
        vi.push_back(map_into_window(line, m, r));
    }
    return vi;
}

MainWindow::MainWindow(QWidget *parent): 
    QMainWindow(parent), 
    ui(new Ui::MainWindow), 
    //speed_controller(state_count, speed_actions.size(), learning_rate, discount_factor, exploration_rate_max, er_half_life), 
    //steering_controller(state_count, steering_actions.size(), learning_rate, discount_factor, exploration_rate_max, er_half_life),
    iter(0), 
    hit_counter(0), 
    success_counter(0),
    last_speed_action(0),
    last_steering_action(0)
{
    std::setlocale(LC_NUMERIC, "C");
    conf = readConfig("configuration/default.conf");
    conf["N_ACTIONS"] = conf["N_SPEED_ACTIONS"];
    speed_controller = QLearningModel(conf);
    conf["N_ACTIONS"] = conf["N_STEERING_ACTIONS"];
    steering_controller = QLearningModel(conf);
    ui->setupUi(this);
    setWindowTitle("Animation Controller");

    resize(std::stoi(conf["WINDOW_WIDTH"]), std::stoi(conf["WINDOW_HEIGHT"]));

    msec = std::stoi(conf["MSEC"]);
    animation_speed = std::stoi(conf["ANIMATION_SPEED"]);
    time_ratio = std::stoi(conf["TIME_RATIO"]);

    speed_actions = progression(std::stoi(conf["N_SPEED_ACTIONS"]), std::stof(conf["SPEED_UNITY"]));
    steering_actions = progression(std::stoi(conf["N_STEERING_ACTIONS"]), M_PI*std::stof(conf["STEERING_UNITY"]));



    ui->learning_rate->setValue(speed_controller.lr_max);
    ui->discount_factor->setValue(speed_controller.discount_factor);
    ui->epsilon->setValue(speed_controller.exploration_rate_max);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if(iter % time_ratio == 0) model_iteration();
        enviroment_iteration(animation_speed*msec);
        iter++;
        update();
    });

    timer->setInterval(animation_speed*msec); 

    env = Enviroment();
    env.car = CarState::generate_random_state();;
    state_encoded = env.car.discretize_state(); //state_encoded = som.findBMU(car_st_vect);



    int m = std::stoi(conf["MARGIN"]);
    int r = std::stoi(conf["PIXEL_RATIO"]);
    car_picture = map_into_window(env.car.to_polygon(), m, r);
    env_picture = map_into_window(env.env_polygon, m, r);



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
    env.car = CarState::generate_random_state();
    int m = std::stoi(conf["MARGIN"]);
    int r = std::stoi(conf["PIXEL_RATIO"]);
    car_picture = map_into_window(env.car.to_polygon(), m, r);
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
    int m = std::stoi(conf["MARGIN"]);
    int r = std::stoi(conf["PIXEL_RATIO"]);

    car_picture = map_into_window(env.car.to_polygon(), m, r);
    painter.drawPolygon(car_picture);

    painter.setBrush(Qt::red); // Set car_picture color
    painter.drawEllipse(map_into_window(QPointF(env.car.x, env.car.y), m, r), 2, 2);


}



void MainWindow::enviroment_iteration(int timestep) {
    int speed_action = last_speed_action;
    int steering_action = last_steering_action;

    auto new_env_st = env.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], timestep);


    auto new_state_encoded = new_env_st.car.discretize_state();


    if(!new_env_st.car_allowed()) {
        hit_counter++;
        env.car = CarState::generate_random_state();;
        state_encoded = env.car.discretize_state(); 
        last_speed_action = speed_controller.chooseAction(state_encoded, ui->eval->isChecked());
        last_steering_action = steering_controller.chooseAction(state_encoded, ui->eval->isChecked());
    }
    else if(new_env_st.car_parked()) {
        success_counter++;
        env.car = CarState::generate_random_state();;
        state_encoded = env.car.discretize_state();
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

    //CarState new_car_st = env.car.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], msec*time_ratio * animation_speed);
    Enviroment new_env_st = env.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], msec*time_ratio * animation_speed);


    auto new_state_encoded = new_env_st.car.discretize_state();//auto new_state_encoded = som.findBMU(new_car_st_vect);

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
        enviroment_iteration(animation_speed*msec*time_ratio);
        if((i+1)%LOG_FREQ==0) {
            std::cout << "{\"iteration\": \"" << i+1 << "\", \"hit\": \"" << hit_counter << "\", \"success\": \"" << success_counter << "\", \"success_ratio\": \""<< 100 * success_counter / (float)(success_counter+hit_counter) << '%' << "\", \"lr\": \""<< speed_controller.lr << "\", \"er\": \"" << speed_controller.exploration_rate << "\"}"<< std::endl;
            file << "{\"iteration\": \"" << i+1 << "\", \"hit\": \"" << hit_counter << "\", \"success\": \"" << success_counter << "\", \"success_ratio\": \""<< 100 * success_counter / (float)(success_counter+hit_counter) << '%' << "\", \"lr\": \""<< speed_controller.lr << "\", \"er\": \"" << speed_controller.exploration_rate << "\"}"<< std::endl;
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

