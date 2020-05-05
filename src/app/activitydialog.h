#pragma once

#include <QDialog>

#include "kemai/kimaiapi.h"

namespace Ui {
class ActivityDialog;
}

namespace kemai::app {

class ActivityDialog : public QDialog
{
    Q_OBJECT

public:
    ActivityDialog(QWidget* parent = nullptr);
    ~ActivityDialog();

    void setActivity(const client::Activity& activity);
    client::Activity activity() const;

private:
    void enableSave(bool enable);

private slots:
    void validateForm();

private:
    Ui::ActivityDialog* mUi;
};

} // namespace kemai::app
