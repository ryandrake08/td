#include "TBActionClockWidget.hpp"

#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRadialGradient>
#include <QtMath>

struct TBActionClockWidget::impl
{
    // Clock state - driven entirely by tournament daemon
    double seconds = 0.0;

    // Configuration properties (matching TBActionClockView)
    bool enableShadows = true;
    bool enableGraduations = true;
    bool enableDigit = true;
    bool enableArc = true;
    bool arcFillsIn = true;

    // Colors matching macOS design
    QColor faceBackgroundColor = QColor(240, 240, 240, 200);
    QColor borderColor = QColor(100, 100, 100);
    QColor digitColor = QColor(60, 60, 60);
    QColor arcBackgroundColor = QColor(220, 50, 50, 180);
    QColor arcBorderColor = QColor(180, 20, 20);
    QColor handColor = QColor(40, 40, 40);

    // Dimensions
    double borderWidth = 3.0;
    double arcBorderWidth = 2.0;
    double handWidth = 2.0;
    double handLength = 60.0;
    double handOffsideLength = 20.0;
    double digitOffset = 0.0;
};

TBActionClockWidget::TBActionClockWidget(QWidget* parent) : QWidget(parent), pimpl(new impl())
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // Default size matching macOS design
    resize(200, 200);
}

TBActionClockWidget::~TBActionClockWidget() = default;

void TBActionClockWidget::setTimeRemaining(double seconds)
{
    pimpl->seconds = seconds;

    // Just update display - window handles visibility
    update();
}

void TBActionClockWidget::setEnableShadows(bool enable)
{
    pimpl->enableShadows = enable;
    update();
}

void TBActionClockWidget::setEnableGraduations(bool enable)
{
    pimpl->enableGraduations = enable;
    update();
}

void TBActionClockWidget::setEnableDigit(bool enable)
{
    pimpl->enableDigit = enable;
    update();
}

void TBActionClockWidget::setEnableArc(bool enable)
{
    pimpl->enableArc = enable;
    update();
}

void TBActionClockWidget::setArcFillsIn(bool fillsIn)
{
    pimpl->arcFillsIn = fillsIn;
    update();
}

QSize TBActionClockWidget::sizeHint() const
{
    return { 200, 200 };
}

void TBActionClockWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF rect = this->rect();
    const QPointF center = rect.center();
    const double radius = qMin(rect.width(), rect.height()) / 2.0 - pimpl->borderWidth;

    // Draw clock face with shadow if enabled
    if(pimpl->enableShadows)
    {
        // Shadow effect
        QRadialGradient shadowGradient(center.x() + 2, center.y() + 2, radius + 5);
        shadowGradient.setColorAt(0, QColor(0, 0, 0, 100));
        shadowGradient.setColorAt(1, QColor(0, 0, 0, 0));
        painter.setBrush(QBrush(shadowGradient));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center.x() - radius + 2, center.y() - radius + 2, radius * 2, radius * 2);
    }

    // Clock face background
    painter.setBrush(QBrush(pimpl->faceBackgroundColor));
    QPen borderPen(pimpl->borderColor, pimpl->borderWidth);
    painter.setPen(borderPen);
    painter.drawEllipse(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

    // Draw graduations (minute marks) if enabled
    if(pimpl->enableGraduations)
    {
        painter.setBrush(Qt::NoBrush);
        QPen graduationPen(pimpl->digitColor, 1.0);
        painter.setPen(graduationPen);

        for(int i = 0; i < 60; ++i)
        {
            const double angle = i * 6.0 * M_PI / 180.0;     // 6 degrees per minute
            const double length = (i % 5 == 0) ? 12.0 : 8.0; // Longer marks for 5-minute intervals
            const double startRadius = radius - length;
            const double endRadius = radius - 2.0;

            const QPointF start(center.x() + startRadius * qSin(angle), center.y() - startRadius * qCos(angle));
            const QPointF end(center.x() + endRadius * qSin(angle), center.y() - endRadius * qCos(angle));

            painter.drawLine(start, end);
        }
    }

    // Draw digits (5, 10, 15, etc.) if enabled
    if(pimpl->enableDigit)
    {
        QFont digitFont = font();
        digitFont.setPointSize(12);
        digitFont.setBold(true);
        painter.setFont(digitFont);
        painter.setPen(pimpl->digitColor);

        const QFontMetrics fm(digitFont);
        const double digitRadius = radius - 25.0 - pimpl->digitOffset;

        for(int i = 1; i <= 12; ++i)
        {
            const QString text = QString::number(i * 5);
            const QRect textRect = fm.boundingRect(text);
            const double angle = i * 30.0 * M_PI / 180.0; // 30 degrees per number

            const QPointF textPos(center.x() + digitRadius * qSin(angle) - textRect.width() / 2, center.y() - digitRadius * qCos(angle) + textRect.height() / 2);

            painter.drawText(textPos, text);
        }
    }

    // Draw countdown arc if enabled
    if(pimpl->enableArc && pimpl->seconds > 0)
    {
        const double arcRadius = radius - 15.0;
        const QRectF arcRect(center.x() - arcRadius, center.y() - arcRadius, arcRadius * 2, arcRadius * 2);

        // Calculate arc angle (0-360 degrees, starting from 12 o'clock)
        const double totalAngle = 360.0;
        const double remainingAngle = pimpl->arcFillsIn ? (pimpl->seconds / 60.0) * totalAngle : ((60.0 - pimpl->seconds) / 60.0) * totalAngle;

        // Draw arc background
        QPen arcPen(pimpl->arcBorderColor, pimpl->arcBorderWidth);
        painter.setPen(arcPen);
        painter.setBrush(QBrush(pimpl->arcBackgroundColor));

        // Start angle is 90 degrees (12 o'clock position)
        // Span angle is negative to go clockwise from top
        const int startAngle = 90 * 16; // Qt uses 1/16th degree units
        const int spanAngle = -remainingAngle * 16;

        painter.drawPie(arcRect, startAngle, spanAngle);
    }

    // Draw clock hand pointing to remaining time
    if(pimpl->seconds > 0)
    {
        QPen handPen(pimpl->handColor, pimpl->handWidth);
        painter.setPen(handPen);

        // Calculate hand angle (0-360 degrees, starting from 12 o'clock)
        const double handAngle = (pimpl->seconds / 60.0) * 360.0 * M_PI / 180.0;

        // Hand positions
        const QPointF handTip(center.x() + pimpl->handLength * qSin(handAngle), center.y() - pimpl->handLength * qCos(handAngle));
        const QPointF handBack(center.x() - pimpl->handOffsideLength * qSin(handAngle), center.y() + pimpl->handOffsideLength * qCos(handAngle));

        painter.drawLine(handBack, handTip);

        // Center dot
        painter.setBrush(QBrush(pimpl->handColor));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center.x() - 3, center.y() - 3, 6, 6);
    }
}