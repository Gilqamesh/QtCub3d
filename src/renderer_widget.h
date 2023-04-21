#ifndef RENDERER_WIDGET_H
#define RENDERER_WIDGET_H

#include <QMainWindow>
#include <QMenu>
#include <QCursor>
#include <QImage>
#include <QPainter>

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
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

    void updateAndRender(r32 dt);
    bool isAlive() const { return is_alive; }

private:
    Ui::Renderer_Widget* ui;
    Map_Model* _map_model;
    QImage framebuffer;
    QImage gun_tex;
    bool is_alive;

    enum class Mode {
        Playing,
        NotPlaying
    };
    Mode mode;

    enum class InputKey {
        KEY_W,
        KEY_S,
        KEY_A,
        KEY_D,

        SIZE
    };
    struct InputState {
        bool is_down;
    } input_state[static_cast<u32>(InputKey::SIZE)];

private:
    bool isPWalkable(u32 x, u32 y);
    void updatePosition(r32 dt);
    void updateOrientation();
    void drawFloorAndCeiling();
    void drawWall();
    void drawMinimap();
    void drawCrosshair();
    void drawGun(QPainter& painter);

    void setMode(Mode mode);
    void clearInputState();
    void drawRectangle(v2<i32> top_left, v2<i32> bot_right, QRgb color);
};

#endif // RENDERER_WIDGET_H
