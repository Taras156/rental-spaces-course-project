#include "themetoggleswitch.h"
#include "thememanager.h"

#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QSizePolicy>

ThemeToggleSwitch::ThemeToggleSwitch(QWidget* parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setToolTip(QString::fromUtf8("Сменить тему"));
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedSize(sizeHint());
    setChecked(ThemeManager::currentTheme() == ThemeManager::Theme9_SkyLight);
}

QSize ThemeToggleSwitch::sizeHint() const
{
    return QSize(54, 28);
}

QSize ThemeToggleSwitch::minimumSizeHint() const
{
    return QSize(54, 28);
}

void ThemeToggleSwitch::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    const bool lightTheme = isChecked();

    QColor background = lightTheme ? QColor("#38BDF8") : QColor("#1E293B");
    QColor border = lightTheme ? QColor("#7DD3FC") : QColor("#475569");
    QColor knob = QColor("#F8FAFC");

    if (underMouse())
        background = lightTheme ? QColor("#0EA5E9") : QColor("#334155");

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF trackRect(1, 1, width() - 2, height() - 2);
    const qreal radius = trackRect.height() / 2.0;

    painter.setPen(QPen(border, 1));
    painter.setBrush(background);
    painter.drawRoundedRect(trackRect, radius, radius);

    const qreal margin = 4.0;
    const qreal diameter = height() - margin * 2.0;
    const qreal x = lightTheme ? width() - diameter - margin : margin;
    QRectF knobRect(x, margin, diameter, diameter);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 35));
    painter.drawEllipse(knobRect.translated(0, 1));

    painter.setBrush(knob);
    painter.drawEllipse(knobRect);
}
