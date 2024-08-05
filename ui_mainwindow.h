/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *playButton;
    QPushButton *stopButton;
    QPushButton *pauseButton;
    QPushButton *trainButton;
    QSpinBox *num_iterations;
    QLineEdit *file_name;
    QPushButton *som_load;
    QPushButton *som_store;
    QPushButton *qtable_load_sp;
    QPushButton *qtable_store_sp;
    QFrame *line;
    QPushButton *qtable_store_st;
    QPushButton *qtable_load_st;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        playButton = new QPushButton(centralwidget);
        playButton->setObjectName(QString::fromUtf8("playButton"));
        playButton->setGeometry(QRect(530, 390, 80, 23));
        stopButton = new QPushButton(centralwidget);
        stopButton->setObjectName(QString::fromUtf8("stopButton"));
        stopButton->setGeometry(QRect(620, 390, 80, 23));
        pauseButton = new QPushButton(centralwidget);
        pauseButton->setObjectName(QString::fromUtf8("pauseButton"));
        pauseButton->setGeometry(QRect(710, 390, 80, 23));
        trainButton = new QPushButton(centralwidget);
        trainButton->setObjectName(QString::fromUtf8("trainButton"));
        trainButton->setGeometry(QRect(620, 310, 101, 23));
        num_iterations = new QSpinBox(centralwidget);
        num_iterations->setObjectName(QString::fromUtf8("num_iterations"));
        num_iterations->setGeometry(QRect(620, 280, 101, 24));
        num_iterations->setMaximum(1000000);
        file_name = new QLineEdit(centralwidget);
        file_name->setObjectName(QString::fromUtf8("file_name"));
        file_name->setGeometry(QRect(622, 40, 161, 23));
        som_load = new QPushButton(centralwidget);
        som_load->setObjectName(QString::fromUtf8("som_load"));
        som_load->setGeometry(QRect(620, 70, 151, 23));
        som_store = new QPushButton(centralwidget);
        som_store->setObjectName(QString::fromUtf8("som_store"));
        som_store->setGeometry(QRect(620, 100, 151, 23));
        qtable_load_sp = new QPushButton(centralwidget);
        qtable_load_sp->setObjectName(QString::fromUtf8("qtable_load_sp"));
        qtable_load_sp->setGeometry(QRect(620, 130, 151, 23));
        qtable_store_sp = new QPushButton(centralwidget);
        qtable_store_sp->setObjectName(QString::fromUtf8("qtable_store_sp"));
        qtable_store_sp->setGeometry(QRect(620, 160, 151, 23));
        line = new QFrame(centralwidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(620, 240, 151, 20));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        qtable_store_st = new QPushButton(centralwidget);
        qtable_store_st->setObjectName(QString::fromUtf8("qtable_store_st"));
        qtable_store_st->setGeometry(QRect(620, 220, 151, 23));
        qtable_load_st = new QPushButton(centralwidget);
        qtable_load_st->setObjectName(QString::fromUtf8("qtable_load_st"));
        qtable_load_st->setGeometry(QRect(620, 190, 151, 23));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(620, 360, 91, 16));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(620, 260, 91, 16));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(630, 20, 91, 16));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 20));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        playButton->setText(QCoreApplication::translate("MainWindow", "Play", nullptr));
        stopButton->setText(QCoreApplication::translate("MainWindow", "Stop", nullptr));
        pauseButton->setText(QCoreApplication::translate("MainWindow", "Pause", nullptr));
        trainButton->setText(QCoreApplication::translate("MainWindow", "Train", nullptr));
        som_load->setText(QCoreApplication::translate("MainWindow", "Load SOM", nullptr));
        som_store->setText(QCoreApplication::translate("MainWindow", "Store SOM", nullptr));
        qtable_load_sp->setText(QCoreApplication::translate("MainWindow", "Load speed QTable", nullptr));
        qtable_store_sp->setText(QCoreApplication::translate("MainWindow", "Store speed QTable", nullptr));
        qtable_store_st->setText(QCoreApplication::translate("MainWindow", "Store steering QTable", nullptr));
        qtable_load_st->setText(QCoreApplication::translate("MainWindow", "Load steering QTable", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Animation", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Iterations", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "File name", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
