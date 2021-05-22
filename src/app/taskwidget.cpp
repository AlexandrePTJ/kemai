#include "taskwidget.h"
#include "ui_taskwidget.h"

#include <QAction>

#include "client/kimairequestfactory.h"

using namespace kemai::app;
using namespace kemai::client;

TaskWidget::TaskWidget(QWidget* parent) : QWidget(parent), mUi(new Ui::TaskWidget)
{
    mUi->setupUi(this);

    mTaskProxyModel.setSourceModel(&mTaskModel);
    mUi->lvTasks->setModel(&mTaskProxyModel);

    auto clearTextAction = mUi->leFilter->addAction(QIcon(":/icons/backspace"), QLineEdit::TrailingPosition);

    connect(clearTextAction, &QAction::triggered, mUi->leFilter, &QLineEdit::clear);
    connect(mUi->leFilter, &QLineEdit::textChanged, [&](const QString& text) { mTaskProxyModel.setFilterRegularExpression(QString(".*%1.*").arg(text)); });
}

TaskWidget::~TaskWidget()
{
    delete mUi;
}

void TaskWidget::setKimaiClient(QSharedPointer<client::KimaiClient> kimaiClient)
{
    mClient = std::move(kimaiClient);
    if (mClient)
    {
        connect(mClient.get(), &KimaiClient::replyReceived, this, &TaskWidget::onClientReply);

        mClient->sendRequest(KimaiRequestFactory::tasks());
    }

    setEnabled(mClient != nullptr);
}

void TaskWidget::onClientReply(const KimaiReply& reply)
{
    if (!reply.isValid())
        return;

    switch (reply.method())
    {
    case ApiMethod::Tasks: {
        auto tasks = reply.get<Tasks>();
        mTaskModel.setTasks(tasks);
    }
    break;

    default:
        break;
    }
}
