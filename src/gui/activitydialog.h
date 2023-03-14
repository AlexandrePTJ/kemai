#pragma once

#include <QDialog>

#include "client/kimaiclient.h"

namespace Ui {
class ActivityDialog;
}

namespace kemai {

class ActivityDialog : public QDialog
{
    Q_OBJECT

public:
    ActivityDialog(QWidget* parent = nullptr);
    ~ActivityDialog() override;

    Activity activity() const;

private:
    void enableSave(bool enable);
    void validateForm();

private:
    Ui::ActivityDialog* mUi;
};

} // namespace kemai
