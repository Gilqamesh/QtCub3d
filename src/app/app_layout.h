#ifndef APP_LAYOUT_H
#define APP_LAYOUT_H

#include "../defs.h"

#include <QLayout>

class App_Layout: public QLayout {
public:
    App_Layout(QWidget* parent);
    virtual ~App_Layout() = default;

    virtual void addItem(QLayoutItem* item) override;
    virtual int	count() const override;
    virtual QLayoutItem *itemAt(int index) const override;
    virtual QLayoutItem *takeAt(int index) override;
};

#endif
