#include "app_layout.h"

App_Layout::App_Layout(QWidget* parent)
    : QLayout(parent) {
}

void App_Layout::addItem(QLayoutItem* item) {
}

int	App_Layout::count() const {
    return 0;
}

QLayoutItem *App_Layout::itemAt(int index) const {
    return nullptr;
}

QLayoutItem *App_Layout::takeAt(int index) {
    return nullptr;
}
