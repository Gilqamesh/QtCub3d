#ifndef RENDERER_WIDGET_H
#define RENDERER_WIDGET_H

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
#include "mvcs/map/map_model.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Renderer_Widget; }
QT_END_NAMESPACE

class Renderer_Widget: public QMainWindow {
    Q_OBJECT

public:
    Renderer_Widget(Map_Model* map_model);
    ~Renderer_Widget();

    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

    void updateAndRender(r32 dt);
    bool isAlive() const { return is_alive; }

private:
    Ui::Renderer_Widget* ui;
    Map_Model* _map_model;
    QImage floor_ceiling_framebuffer;
    QImage wall_framebuffer;
    bool is_alive;

    enum class Mode {
        Playing,
        NotPlaying
    };
    Mode mode;

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
    bool isPWalkable(u32 x, u32 y);
    void updatePosition(r32 dt);
    void updateOrientation();
    void updateFloorAndCeiling();
    void updateWall();

    void setMode(Mode mode);
};

#endif // RENDERER_WIDGET_H
