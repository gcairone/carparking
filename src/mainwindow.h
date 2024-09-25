#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QRect>
#include <QPainter>
#include <QPoint>
#include <QPolygon>
#include <iostream>
#include <cmath>
#include "enviroment.h"
//#include "kohonen_net.h"
#include "q_learning.h"
#include <QFileDialog>





QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_playButton_clicked();

    void on_stopButton_clicked();

    void on_pauseButton_clicked();
    //void iteration_animation();

    void on_trainButton_clicked();

    void enviroment_iteration(int timestep);
    void model_iteration();

    //void on_num_iterations_valueChanged(int arg1);

    //void on_som_load_clicked();
    //void on_som_store_clicked();
    void on_qtable_load_sp_clicked();
    void on_qtable_store_sp_clicked();
    void on_qtable_load_st_clicked();
    void on_qtable_store_st_clicked();

protected:
    void paintEvent(QPaintEvent *event) override;


private:
    Ui::MainWindow *ui;

    QPolygon rectangle;


    CarState car_st;
    //std::vector<float> car_st_vect;


    QPolygon env;
    QVector<QLineF> lidar;

    //SOM som;
    QLearningModel speed_controller;
    QLearningModel steering_controller;
    int state_encoded;


    QTimer *timer;
    int iter;
    int hit_counter;
    int success_counter;

    int last_speed_action;
    int last_steering_action;
};

QPoint map_into_window(const QPointF &p);
QLine map_into_window(const QLineF &p);
QPolygon map_into_window(const QPolygonF &p);
QVector<QLine> map_into_window(const QVector<QLineF> &p);


#endif // MAINWINDOW_H
