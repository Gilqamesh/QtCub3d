
#include "mywindow.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QEventLoop>
#include "cubmap_model.h"

#include "defs.h"

int main(int argc, char *argv[])
{
    try {
        QApplication app(argc, argv);
        MyWindow window("../Cub3d/maps/map.cub");
        window.show();

        constexpr r64 frame_budge_fps = 60.0;
        constexpr u64 frame_budget_ns = 1000000000.0 / frame_budge_fps;

        QElapsedTimer timer;
        timer.start();
        u64 frame_time_end_ns = timer.nsecsElapsed();

        QEventLoop events;
        while (window.isAlive()) {
            u64 frame_time_start_ns = timer.nsecsElapsed();
            r32 dt_s = (frame_time_start_ns - frame_time_end_ns) / 1000000000.0;
            frame_time_end_ns = frame_time_start_ns;

            events.processEvents(QEventLoop::ExcludeSocketNotifiers);
            window.updateAndRender(dt_s);

            u64 cur_frame_time_end_ns = timer.nsecsElapsed();
            while (cur_frame_time_end_ns - frame_time_start_ns < frame_budget_ns) {
                _mm_pause();
                cur_frame_time_end_ns = timer.nsecsElapsed();
            }
        }
    } catch (const std::exception& err) {
        LOG(err.what());
    }
}
