#include "app/app_widget.h"
#include "renderer_widget.h"
#include "mvcs/map/map_model.h"
#include "mvcs/map_editor/map_editor_mvc.h"

#include <QApplication>

#include "defs.h"

int main(int argc, char *argv[]) {
    try {
        QApplication app(argc, argv);

        Map_Model map_model("../maps/map.cub");

        Map_Editor_Mvc editor(&map_model);
        Renderer_Widget renderer(&map_model);

        v2<u32> app_widget_dims(1280, 720);
        App_Widget app_widget(app_widget_dims, &renderer, &editor);

        app_widget.exec();

    } catch (const std::exception& err) {
        LOG(err.what());
    }
}
