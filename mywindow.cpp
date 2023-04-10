
#include "mywindow.h"
#include "./ui_mywindow.h"

#include <QPainter>
#include <QKeyEvent>

#include "clamp.h"

MyWindow::MyWindow(u32 width, u32 height, CubMap_Model* cubmap_model)
    : ui(new Ui::MyWindow)
    , first_time(true)
    , is_alive(true)
    , camera(1.0f, 1.0f, 0.0f)

{
    ui->setupUi(this);
    this->setBaseSize(width, height);
    setMouseTracking(true);
    centralWidget()->setMouseTracking(true);
    cubmap_view.setModel(cubmap_model);
    cubmap_view.setBaseSize(width / 2, height / 2);
    cubmap_view.show();
}

MyWindow::~MyWindow()
{
    delete ui;
}

void MyWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Escape: {
        is_alive = false;
    } break ;
    case Qt::Key_F1: {
        this->setMouseTracking(!this->hasMouseTracking());
    } break ;
    }
}

void MyWindow::mouseMoveEvent(QMouseEvent* event) {
    if (!this->hasMouseTracking()) {
        return ;
    }
    if (first_time) {
        first_time = false;
        mouse_prev_x = event->pos().x();
    }
    i32 mouse_x = event->pos().x();
    r32 mouse_dx = mouse_x - mouse_prev_x;
    mouse_prev_x = mouse_x;

    r32 max_mouse_dx = width();
    mouse_dx = clamp_value(-max_mouse_dx, mouse_dx, max_mouse_dx);
    mouse_dx = (mouse_dx < 0.0f ? -1.5 : 1.5 ) * sqrt(abs(mouse_dx));
    mouse_dx /= 30.0f;
    camera.updateDirection(mouse_dx);
}

void MyWindow::closeEvent(QCloseEvent* event) {
    this->is_alive = false;
}

void MyWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    drawFloor(painter);
    drawWall(painter);
    cubmap_view.update();
}

void MyWindow::drawFloor(QPainter& painter) {
    v2<r32> offset(width() / 2, height() / 2);
    v2<r32> dir = v2_scale(camera.dir, 50.0f);
    v2<r32> plane = v2_scale(camera.plane, 50.0f);
    painter.setPen(Qt::blue);
    painter.drawLine(
        QPointF(offset.x, offset.y),
        QPointF(dir.x + offset.x, dir.y + offset.y)
    );
    painter.setPen(Qt::red);
    painter.drawLine(
        QPointF(offset.x, offset.y),
        QPointF(plane.x + offset.x, plane.y + offset.y)
    );
}

void MyWindow::drawWall(QPainter& painter) {
}

void MyWindow::updateAndRender(r32 dt) {
    this->dt = dt;
    this->update(); // calls paintEvent
}

bool MyWindow::isAlive(void) const {
    return this->is_alive;
}
