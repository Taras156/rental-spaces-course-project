#ifndef THEMETOGGLESWITCH_H
#define THEMETOGGLESWITCH_H

#include <QAbstractButton>

class ThemeToggleSwitch : public QAbstractButton
{
    Q_OBJECT

public:
    explicit ThemeToggleSwitch(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // THEMETOGGLESWITCH_H
