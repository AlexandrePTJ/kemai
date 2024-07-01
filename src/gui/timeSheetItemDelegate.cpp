#include "timeSheetItemDelegate.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QStyle>

#include "misc/fontAwesomeHolder.h"
#include "models/timeSheetModel.h"

using namespace kemai;

static const auto gIndicatorWidth = 2;
static const auto gTextSpacing    = 2;
static const auto gTextLeftOffset = gIndicatorWidth + gTextSpacing;
static const auto gPadding        = 4;
static const auto gButtonWidth    = 64;
static const auto gButtonIconSide = 24;

// gIndicatorWidth + gPadding (1 x blue rect + 1 x empty)
// |  |
// || < gPadding >
// || Customer name
// || < gTextSpacing >
// || Project name (on gProjectMaxWidth) < gTextSpacing > Activity name
// || < gPadding >

void TimeSheetItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto isHover   = (option.state & QStyle::State_MouseOver) != 0;
    auto timeSheet = reinterpret_cast<const TimeSheet*>(index.constInternalPointer());

    /*
     * Background frame
     */
    QStyleOptionFrame frameOption;
    frameOption.frameShape = QFrame::StyledPanel;
    frameOption.rect       = option.rect;
    if (isHover)
    {
        frameOption.palette.setColor(QPalette::Base, option.palette.color(QPalette::Base).lighter(120));
    }
    qApp->style()->drawPrimitive(QStyle::PE_Frame, &frameOption, painter);

    /*
     * Text
     */
    const auto textX       = option.rect.left() + gTextLeftOffset;
    const auto textH       = option.fontMetrics.height();
    const auto widthThird  = option.rect.width() / 3;
    const auto secondLineY = option.rect.top() + gPadding + option.fontMetrics.height() + gTextSpacing;

    // Customer
    auto customerRect    = QRect(textX, option.rect.top() + gPadding, option.rect.width(), textH);
    auto customerPalette = option.palette;
    if (!timeSheet->project.customer.color.isEmpty())
    {
        customerPalette.setColor(QPalette::WindowText, timeSheet->project.customer.color);
    }
    qApp->style()->drawItemText(painter, customerRect, Qt::AlignLeft, customerPalette, isHover, timeSheet->project.customer.name, QPalette::WindowText);

    // Project
    auto projectRect    = QRect(textX, secondLineY, widthThird, textH);
    auto projectPalette = option.palette;
    if (!timeSheet->project.color.isEmpty())
    {
        projectPalette.setColor(QPalette::WindowText, timeSheet->project.color);
    }
    qApp->style()->drawItemText(painter, projectRect, Qt::AlignLeft, projectPalette, isHover, timeSheet->project.name, QPalette::WindowText);

    // Activity
    auto activityRect = QRect(textX + widthThird, secondLineY, 2 * widthThird, textH);
    qApp->style()->drawItemText(painter, activityRect, Qt::AlignLeft, option.palette, isHover, timeSheet->activity.name);

    /*
     * Buttons
     */
    const auto buttonY = option.rect.top() + gPadding;
    const auto buttonH = option.rect.height() - 2 * gPadding;

    QVariantMap fontAwesomeOptions;
    if (isHover)
    {
        fontAwesomeOptions.insert("color-disabled", QColor::fromString("#009d39"));
    }

    // Reload button
    QStyleOptionButton reloadButtonOption;
    reloadButtonOption.features = QStyleOptionButton::Flat;
    reloadButtonOption.icon     = FontAwesomeHolder::get()->icon(fa::fa_solid, fa::fa_play, fontAwesomeOptions);
    reloadButtonOption.iconSize = {gButtonIconSide, gButtonIconSide};
    reloadButtonOption.rect     = QRect(option.rect.right() - 3 * gPadding - 2 * gButtonWidth, buttonY, gButtonWidth, buttonH);

    qApp->style()->drawControl(QStyle::CE_PushButton, &reloadButtonOption, painter);

    // Restart button
    QStyleOptionButton restartButtonOption;
    restartButtonOption.features = QStyleOptionButton::Flat;
    restartButtonOption.icon     = FontAwesomeHolder::get()->icon(fa::fa_solid, fa::fa_rotate_right, fontAwesomeOptions);
    restartButtonOption.iconSize = {gButtonIconSide, gButtonIconSide};
    restartButtonOption.rect     = QRect(option.rect.right() - gPadding - gButtonWidth, buttonY, gButtonWidth, buttonH);

    qApp->style()->drawControl(QStyle::CE_PushButton, &restartButtonOption, painter);
}

QSize TimeSheetItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return {option.rect.width(), option.fontMetrics.height() * 2 + gTextSpacing + 2 * gPadding};
}

bool TimeSheetItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        return true;

    default:
        return false;
    }
}
