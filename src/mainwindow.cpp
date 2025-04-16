#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <chrono>
// log constant
#define LOG_FREQ 1000000 // how many iterations between each log
#define WHEEL_RADIUS 0.3
#define SHOW_WHEEL 1
using namespace std;

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
    //controller(state_count, speed_actions.size(), learning_rate, discount_factor, er_max, er_half_life), 
    //controller(state_count, steering_actions.size(), learning_rate, discount_factor, er_max, er_half_life),
    iter(0), 
    hit_counter(0), 
    success_counter(0),
    last_speed_action(0),
    last_steering_action(0),
    avg_tdr(0)
{
    setlocale(LC_NUMERIC, "C");
    conf = readConfig("configuration/default.conf");

    int n_steering_actions = stoi(conf["N_STEERING_ACTIONS"]);
    int n_speed_actions = stoi(conf["N_SPEED_ACTIONS"]);
    conf["N_ACTIONS"] = to_string(n_steering_actions*n_speed_actions);
    conf["N_STATES"] = to_string(stoi(conf["X_DIVIDE"]) * stoi(conf["Y_DIVIDE"]) * stoi(conf["THETA_DIVIDE"]));
    cout << stoi(conf["N_STATES"])*stoi(conf["N_ACTIONS"])  << endl;
    controller = QLearningModel(conf);

    ui->setupUi(this);
    setWindowTitle("Animation Controller");

    resize(stoi(conf["WINDOW_WIDTH"]), stoi(conf["WINDOW_HEIGHT"]));

    msec = stoi(conf["MSEC"]);
    animation_speed = stoi(conf["ANIMATION_SPEED"]);
    time_ratio = stoi(conf["TIME_RATIO"]);

    speed_actions = progression(stoi(conf["N_SPEED_ACTIONS"]), stof(conf["SPEED_UNITY"]));
    steering_actions = progression(stoi(conf["N_STEERING_ACTIONS"]), M_PI*stof(conf["STEERING_UNITY"])/180);



    ui->learning_rate->setValue(controller.lr_max);
    ui->discount_factor->setValue(controller.discount_factor);
    ui->epsilon->setValue(controller.er_max);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if(iter % time_ratio == 0) {
            //cout << "**********************************************" << endl;
            model_iteration();
        }
        enviroment_iteration(animation_speed*msec);
        iter++;
        update();
    });

    timer->setInterval(msec); 

    env = Enviroment(conf);
    //env.car.x = env.width_car*2.15;
    //env.car.y = env.len_car*1.60;
    //env.car.theta = -M_PI/2;
    env.set_random_carstate();
    //cout << "Inizializzato e settato stato casuale:" << endl;
    //env.car = CarState::generate_random_state();;
    state_encoded = env.discretize_state();


    int m = stoi(conf["MARGIN"]);
    int r = stoi(conf["PIXEL_RATIO"]);
    car_picture = map_into_window(env.car_polygon(), m, r);
    env_picture = map_into_window(env.env_polygon, m, r);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playButton_clicked()
{   
    controller.lr = ui->learning_rate->value();
    //controller.lr = ui->learning_rate->value();
    controller.er = ui->epsilon->value();
    //controller.er = ui->epsilon->value();
    controller.er_max = ui->epsilon->value();
    //controller.er_max = ui->epsilon->value();

    timer->start();

}

void MainWindow::on_pauseButton_clicked()
{
    timer->stop();
}

void MainWindow::on_stopButton_clicked()
{
    //env.car = CarState::generate_random_state();
    env.set_random_carstate();
    int m = stoi(conf["MARGIN"]);
    int r = stoi(conf["PIXEL_RATIO"]);
    car_picture = map_into_window(env.car_polygon(), m, r);
    update();
    timer->stop();

}


void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawPolygon(env_picture);
    painter.setBrush(Qt::blue); 
    int m = stoi(conf["MARGIN"]);
    int r = stoi(conf["PIXEL_RATIO"]);

    car_picture = map_into_window(env.car_polygon(), m, r);
    painter.drawPolygon(car_picture);

    painter.setBrush(Qt::red); // Set car_picture color
    painter.drawEllipse(map_into_window(QPointF(env.car.x, env.car.y), m, r), 2, 2);
    // centro dell'asse posteriore 
    float sin_t = sin(env.car.theta);
    float cos_t = cos(env.car.theta);

    float xR = env.car.x - (0.5*env.len_car-env.rear_overhang)*cos_t;
    float yR = env.car.y - (0.5*env.len_car-env.rear_overhang)*sin_t;
    painter.drawEllipse(map_into_window(QPointF(xR, yR), m, r), 2, 2);
    
    if(SHOW_WHEEL) {
        QPen pen(Qt::green);       // Imposta il colore della penna
        pen.setWidth(3);           // Imposta lo spessore a 5px
        
        painter.setPen(pen);       // Applica la penna al painter
        //painter.setBrush(Qt::red);
        // ruota dietro destra
        float xRR = xR - 0.5*env.width_car*sin_t;
        float yRR = yR + 0.5*env.width_car*cos_t;
        QLineF RR(
            xRR - WHEEL_RADIUS*cos_t,
            yRR - WHEEL_RADIUS*sin_t,
            xRR + WHEEL_RADIUS*cos_t,
            yRR + WHEEL_RADIUS*sin_t
        );
        painter.drawLine(map_into_window(RR, m, r));

        // ruota dietro sinistra
        float xRL = xR + 0.5*env.width_car*sin_t;
        float yRL = yR - 0.5*env.width_car*cos_t;
        QLineF RL(
            xRL - WHEEL_RADIUS*cos_t,
            yRL - WHEEL_RADIUS*sin_t,
            xRL + WHEEL_RADIUS*cos_t,
            yRL + WHEEL_RADIUS*sin_t
        );
        painter.drawLine(map_into_window(RL, m, r));
        //painter.drawEllipse(map_into_window(QPointF(xRL, yRL), m, r), 2, 2);
        
        // ruota davanti destra
        float sin_t_a = sin(env.car.theta+steering_actions[last_steering_action]);
        float cos_t_a = cos(env.car.theta+steering_actions[last_steering_action]);
        float xFR = xRR + (env.len_car-env.front_overhang-env.rear_overhang)*cos_t;
        float yFR = yRR + (env.len_car-env.front_overhang-env.rear_overhang)*sin_t;
        QLineF FR(
            xFR - WHEEL_RADIUS*cos_t_a,
            yFR - WHEEL_RADIUS*sin_t_a,
            xFR + WHEEL_RADIUS*cos_t_a,
            yFR + WHEEL_RADIUS*sin_t_a
        );
        painter.drawLine(map_into_window(FR, m, r));
        //painter.drawEllipse(map_into_window(QPointF(xFR, yFR), m, r), 2, 2);

        // ruota davanti sinistra
        float xFL = xRL + (env.len_car-env.front_overhang-env.rear_overhang)*cos_t;
        float yFL = yRL + (env.len_car-env.front_overhang-env.rear_overhang)*sin_t;
        QLineF FL(
            xFL - WHEEL_RADIUS*cos_t_a,
            yFL - WHEEL_RADIUS*sin_t_a,
            xFL + WHEEL_RADIUS*cos_t_a,
            yFL + WHEEL_RADIUS*sin_t_a
        );
        //painter.drawEllipse(map_into_window(QPointF(xFL, yFL), m, r), 2, 2);
        painter.drawLine(map_into_window(FL, m, r));

        painter.setBrush(Qt::red);
        painter.setPen(Qt::red);
        painter.drawEllipse(map_into_window(QPointF(xRR, yRR), m, r), 2, 2);
        painter.drawEllipse(map_into_window(QPointF(xRL, yRL), m, r), 2, 2);
        painter.drawEllipse(map_into_window(QPointF(xFR, yFR), m, r), 2, 2);
        painter.drawEllipse(map_into_window(QPointF(xFL, yFL), m, r), 2, 2);

    }

}



void MainWindow::enviroment_iteration(int timestep) {
    //cout << "--ENV ITERATION--" << endl;
    int speed_action = last_speed_action;
    int steering_action = last_steering_action;
    //cout << "Recuperate azioni: " << last_speed_action << ", " << last_steering_action << endl;

    Enviroment new_env_st = env.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], timestep);

    //cout << "Stato successivo: ";
    int new_state_encoded = new_env_st.discretize_state();


    if(!new_env_st.car_allowed()) {
        //cout << "Urto, setto nuovo stato casuale:" << endl;
        hit_counter++;
        //env.car = CarState::generate_random_state();
        env.set_random_carstate();
        state_encoded = env.discretize_state(); 
        int action = controller.chooseAction(state_encoded, ui->eval->isChecked());
        last_speed_action = action / steering_actions.size();
        last_steering_action = action % steering_actions.size();
        //cout << "Scelgo " << last_speed_action << ", " << last_steering_action << endl;
    }
    else if(new_env_st.car_parked()) {
        //cout << "Parcheggio, setto nuovo stato causuale" << endl;
        success_counter++;
        //env.car = CarState::generate_random_state();
        env.set_random_carstate();
        state_encoded = env.discretize_state();
        int action = controller.chooseAction(state_encoded, ui->eval->isChecked());
        last_speed_action = action / steering_actions.size();
        last_steering_action = action % steering_actions.size();
        //cout << "Scelgo " << last_speed_action << ", " << last_steering_action << endl;
    }

    else {
        //cout << "Buono, aggiorno lo stato" << endl;
        env = new_env_st;
        state_encoded = new_state_encoded;
    }
}

void MainWindow::model_iteration() {
    //cout << "--MODEL ITERATION--" << endl;
    int action = controller.chooseAction(state_encoded, ui->eval->isChecked());
    last_speed_action = action / steering_actions.size();
    last_steering_action = action % steering_actions.size();
    //cout << "Scelgo " << last_speed_action << ", " << last_steering_action << endl;

    //CarState new_car_st = env.car.compute_new_state(speed_actions[speed_action], steering_actions[steering_action], msec*time_ratio * animation_speed);
    Enviroment new_env_st = env.compute_new_state(speed_actions[last_speed_action], steering_actions[last_steering_action], msec*time_ratio * animation_speed);
    //cout << "Stato successivo sarà: ";

    ////cout << "Prova" << endl;
    int new_state_encoded = new_env_st.discretize_state();//auto new_state_encoded = som.findBMU(new_car_st_vect);

    if(!ui->eval->isChecked()) {

        float reward = new_env_st.reward();
        
        float tdr = controller.train(state_encoded, action, reward, new_state_encoded);

        //cout << "Reward, tdr: " << reward << ", " << tdr << endl;
        int i = iter/time_ratio;
        //cout << i << endl;
        if(i % 1000 == 0) {
            controller.er = controller.er_min + controller.er_ratio*(controller.er - controller.er_min);
            controller.lr = controller.lr_min + controller.lr_ratio*(controller.lr - controller.lr_min);   
            //cout << i << ", " << controller.er << endl; 
        }

        avg_tdr = (i*avg_tdr+abs(tdr))/(i+1);
    }


}






void MainWindow::on_trainButton_clicked()
{
    controller.lr = ui->learning_rate->value();
    controller.er = ui->epsilon->value();
    controller.er_max = ui->epsilon->value();

    ofstream file("log/log0.txt");
    if (!file.is_open()) {
        cerr << "Failed to open file: log/log0.txt" << endl;
    }

    int num_iter = ui->num_iterations->value();
    for(int i=0; i<num_iter; ++i) {
        model_iteration();
        enviroment_iteration(animation_speed*msec*time_ratio);
        if((i+1)%LOG_FREQ==0) {
            cout << "{\"iteration\": \"" << i+1 << "\", \"hit\": \"" << hit_counter << "\", \"success\": \"" << success_counter << "\", \"success_ratio\": \""<< 100 * success_counter / (float)(success_counter+hit_counter) << '%' << "\", \"lr\": \""<< controller.lr << "\", \"er\": \"" << controller.er << "\", \"avg_tdr\": \"" << avg_tdr <<"\"}"<< endl;
            file << "{\"iteration\": \"" << i+1 << "\", \"hit\": \"" << hit_counter << "\", \"success\": \"" << success_counter << "\", \"success_ratio\": \""<< 100 * success_counter / (float)(success_counter+hit_counter) << '%' << "\", \"lr\": \""<< controller.lr << "\", \"er\": \"" << controller.er << "\", \"avg_tdr\": \"" << avg_tdr <<"\"}"<< endl;
            hit_counter=0;
            success_counter=0;
            avg_tdr = 0;
            update();
        }
    }
    
    cout << "Trained "<< num_iter <<endl;
    file.close();

}


void MainWindow::on_resetButton_clicked() {
    controller.reset();
    cout << "Q-Table reset" << endl;
}

void MainWindow::on_qtable_load_sp_clicked()
{
    controller.loadWeights(ui->file_name->text().toStdString());
}

void MainWindow::on_qtable_store_sp_clicked()
{
    controller.storeWeights(ui->file_name->text().toStdString());
}

void MainWindow::on_qtable_load_st_clicked()
{
    controller.loadWeights(ui->file_name->text().toStdString());
}

void MainWindow::on_qtable_store_st_clicked()
{
    controller.storeWeights(ui->file_name->text().toStdString());
}

