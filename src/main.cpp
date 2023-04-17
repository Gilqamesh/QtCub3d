#include "renderer_widget.h"
#include "app/app_widget.h"
#include "mvcs/map/map_model.h"
#include "mvcs/map_editor/map_editor_mvc.h"

#include <QApplication>
#include <QMainWindow>

#include "defs.h"

int main(int argc, char *argv[]) {
    try {
        QApplication app(argc, argv);

        Map_Model map_model("../Cub3d/maps/map.cub");

        Map_Editor_Mvc map_editor(&map_model);

        QMainWindow main_window;
        Renderer_Widget renderer_widget(&map_model);

        v2<u32> app_widget_dims(1280, 720);
        App_Widget app_widget;
        app_widget.getWidget()->setGeometry(QRect(0, 0, app_widget_dims.x, app_widget_dims.y));

        app_widget.addWidget(&renderer_widget);
        app_widget.getLayout()->addWidget(renderer_widget.getWidget(), 1, 0);

        app_widget.addWidget(&map_editor);
        app_widget.getLayout()->addWidget(map_editor.getWidget(), 1, 1);

        app_widget.getWidget()->show();
        app_widget.exec();

    } catch (const std::exception& err) {
        LOG(err.what());
    }
}
