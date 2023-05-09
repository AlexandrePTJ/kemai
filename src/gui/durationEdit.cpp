#include "durationEdit.h"

#include <QRegularExpressionValidator>

using namespace kemai;

DurationEdit::DurationEdit(QWidget* parent) : QLineEdit(parent), mDurationRx("^([0-9]*)(:?([0-9]{2})?)$")
{
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setValidator(new QRegularExpressionValidator(mDurationRx, this));
}

void DurationEdit::setSeconds(int seconds)
{
    setText(QString("%1:%2").arg(seconds / 3600, seconds % 3600));
}

int DurationEdit::seconds() const
{
    int nSecs = 0;

    auto match = mDurationRx.match(text());
    if (match.hasMatch())
    {
        // hours
        auto hourStr = match.captured(1);
        if (!hourStr.isEmpty())
        {
            nSecs += 3600 * hourStr.toInt();
        }

        // minutes
        auto minStr = match.captured(3);
        if (!minStr.isEmpty())
        {
            nSecs += 60 * minStr.toInt();
        }
    }

    return nSecs;
}
