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
    setText(QString("%1:%2").arg(seconds / 60).arg(seconds % 60));
}

int DurationEdit::seconds() const
{
    int nsecs = 0;

    auto match = mDurationRx.match(text());
    if (match.hasMatch())
    {
        // minutes
        auto minstr = match.captured(1);
        if (not minstr.isEmpty())
        {
            nsecs += 60 * minstr.toInt();
        }

        // seconds
        auto secstr = match.captured(3);
        if (not secstr.isEmpty())
        {
            nsecs += secstr.toInt();
        }
    }

    return nsecs;
}
