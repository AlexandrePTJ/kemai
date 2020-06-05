#include "durationedit.h"

#include <QRegularExpressionValidator>

using namespace kemai::app;

DurationEdit::DurationEdit(QWidget* parent) : QLineEdit(parent), mDurationRx("^([0-9]*)(:?([0-9]{2})?)$")
{
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setValidator(new QRegularExpressionValidator(mDurationRx, this));
}

void DurationEdit::setSeconds(int seconds)
{
    setText(QString("%1:%2").arg(seconds / 3600).arg(seconds % 3600));
}

int DurationEdit::seconds() const
{
    int nsecs = 0;

    auto match = mDurationRx.match(text());
    if (match.hasMatch())
    {
        // hours
        auto hourstr = match.captured(1);
        if (not hourstr.isEmpty())
        {
            nsecs += 3600 * hourstr.toInt();
        }

        // minutes
        auto minstr = match.captured(3);
        if (not minstr.isEmpty())
        {
            nsecs += 60 * minstr.toInt();
        }
    }

    return nsecs;
}
