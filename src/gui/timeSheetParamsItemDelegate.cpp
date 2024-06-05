#include "timeSheetParamsItemDelegate.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>

#include "models/timeSheetParamsModel.h"

using namespace kemai;

static const auto gIndicatorWidth = 2;
static const auto gTextSpacing    = 2;
static const auto gTextLeftOffset = gIndicatorWidth + gTextSpacing;
static const auto gPadding        = 4;

// gIndicatorWidth + gPadding (1 x blue rect + 1 x empty)
// |  |
// || < gPadding >
// || Customer name
// || < gTextSpacing >
// || Project name (on gProjectMaxWidth) < gTextSpacing > Activity name
// || < gPadding >

void TimeSheetParamsItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Indicate selection
    auto isSelected = (option.state & QStyle::State_Selected) != 0;
    if (isSelected)
    {
        painter->fillRect(option.rect, option.palette.mid());
        painter->fillRect(QRect(option.rect.topLeft(), QSize(gIndicatorWidth, option.rect.height())), option.palette.highlight());
    }

    const auto textX       = option.rect.left() + gTextLeftOffset;
    const auto textH       = option.fontMetrics.height();
    const auto widthThird  = option.rect.width() / 3;
    const auto secondLineY = option.rect.top() + gPadding + option.fontMetrics.height() + gTextSpacing;

    // Customer
    auto headerRect = QRect(textX, option.rect.top() + gPadding, option.rect.width(), textH);
    qApp->style()->drawItemText(painter, headerRect, Qt::AlignLeft, option.palette, isSelected, index.data(TimeSheetParamsModel::CustomerName).toString());

    // Project
    auto projectRect = QRect(textX, secondLineY, widthThird, textH);
    qApp->style()->drawItemText(painter, projectRect, Qt::AlignLeft, option.palette, isSelected, index.data(TimeSheetParamsModel::ProjectName).toString());

    // Activity
    auto activityRect = QRect(textX + widthThird, secondLineY, 2 * widthThird, textH);
    qApp->style()->drawItemText(painter, activityRect, Qt::AlignLeft, option.palette, isSelected, index.data(TimeSheetParamsModel::ActivityName).toString());

    if (option.showDecorationSelected)
    {
        qDebug() << index.data().toString();
    }
}

QSize TimeSheetParamsItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return {option.rect.width(), option.fontMetrics.height() * 2 + gTextSpacing + 2 * gPadding};
}
