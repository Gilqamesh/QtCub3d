#ifndef APP_WIDGET_H
#define APP_WIDGET_H

#include <QGridLayout>

#include "../defs.h"

#include "../mvcs/map_editor/map_editor_mvc.h"
#include "../renderer_widget.h"

class App_Widget: public QWidget {
public:
    App_Widget(v2<u32> dims, Renderer_Widget* renderer, Map_Editor_Mvc* editor);

    void exec();

public slots:
    void destroy();
    bool isAlive() const;

private:
    void initializeAppLayout(v2<u32> dims);
    void initializeMenuBar(QGridLayout* app_layout);

private:
    Renderer_Widget* _renderer;
    Map_Editor_Mvc* _editor;

    bool _is_alive;

private:
    void updateAndRender(r32 dt);
};

#endif
