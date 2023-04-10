
#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QMainWindow>
#include <vector>

#include "defs.h"
#include "camera.h"
#include "cubmap_view.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MyWindow; }
QT_END_NAMESPACE

class MyWindow : public QMainWindow
{
    Q_OBJECT

public:
    MyWindow(u32 width, u32 height, CubMap_Model* cubmap_model);
    ~MyWindow();

    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

    void updateAndRender(r32 dt);
    bool isAlive(void) const;

private:
    Ui::MyWindow *ui;
    CubMap_View cubmap_view;
    bool first_time;
    bool is_alive;
    Camera camera;
    r32 dt;
    i32 mouse_prev_x;

private:

    void drawFloor(QPainter& painter);
    void drawWall(QPainter& painter);
};

#endif // MYWINDOW_H
