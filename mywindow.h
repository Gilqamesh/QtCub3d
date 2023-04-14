#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QCursor>
#include <QPixmap>
#include <QImage>
#include <QWindow>

#include <vector>
#include <array>

#include "defs.h"
#include "camera.h"
#include "cubmap_view.h"
#include "cubmap_delegate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MyWindow; }
QT_END_NAMESPACE

class MyWindow : public QMainWindow
{
    Q_OBJECT

public:
    MyWindow(const std::string& cubmap_path);
    ~MyWindow();

    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

    void updateAndRender(r32 dt);
    bool isAlive(void) const;

private:
    Ui::MyWindow* ui;
    CubMap_Model* cubmap_model;
    CubMap_View* cubmap_view;
    CubMap_Delegate* cubmap_delegate;
    QMenu* cubmap_editor;
    QCursor cursor;
    QImage framebuffer;
    QImage framebuffer_rl;
    bool is_alive;

    bool is_cursor_free;
    bool is_w_down;
    bool is_s_down;
    bool is_a_down;
    bool is_d_down;

    enum class WallTexId {
        North,
        South,
        West,
        East,

        SIZE
    };
    std::array<QImage, static_cast<u32>(WallTexId::SIZE)> wall_textures;
    QImage floor_tex;
    QImage ceiling_tex;

private:
    void updateFloor();
    void updateWall();
    void render();

    bool isPWalkable(u32 x, u32 y);
    void updatePosition(r32 dt);
    void updateOrientation();
};

#endif // MYWINDOW_H
