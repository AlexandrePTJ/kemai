#pragma once

#include <QLineEdit>
#include <QRegularExpression>

namespace kemai::app {

class DurationEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(int seconds READ seconds WRITE setSeconds)

public:
    DurationEdit(QWidget* parent = nullptr);
    ~DurationEdit() = default;

    void setSeconds(int seconds);
    int seconds() const;

private:
    const QRegularExpression mDurationRx;
};

} // namespace kemai::app
