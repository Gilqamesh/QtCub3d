#ifndef APP_WIDGET_H
#define APP_WIDGET_H

#include <QGridLayout>

#include "../defs.h"

#include "../mvcs/map_editor/map_editor_view.h"
#include "../renderer_widget.h"
#include "../mvcs/map/map_model.h"

class App_Widget: public QWidget {
public:
    App_Widget(v2<u32> dims, Map_Model* map_model);
    ~App_Widget();

    void exec();

public slots:
    void destroy();
    bool isAlive() const;

private:
    void initializeAppLayout(v2<u32> dims);
    void initializeMenuBar(QGridLayout* app_layout);

private:
    void closeEvent(QCloseEvent *event);

private:
    Map_Model* _map_model;
    Renderer_Widget *_renderer;
    Map_Editor_View *_editor;

    bool _is_alive;

private:
    void updateAndRender(r32 dt);
};

#endif
