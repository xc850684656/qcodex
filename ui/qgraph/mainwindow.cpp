#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qpainter.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QImage img;
    img.load(QString("/home/gao/Pictures/test.png"));
    QPainter pt(this);
    pt.drawImage(QPoint(), img);
    // this->update();
}
