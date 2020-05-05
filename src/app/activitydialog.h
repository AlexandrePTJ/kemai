#pragma once

#include <QDialog>

#include "kemai/kimaiclient.h"

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
    void onClientReply(const client::KimaiReply& reply);
    void onCbCustomerTextChanged(const QString& text);
    void validateForm();

private:
    Ui::ActivityDialog* mUi;
    QSharedPointer<client::KimaiClient> mClient;
};

} // namespace kemai::app
