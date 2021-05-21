#include "taskwidget.h"
#include "ui_taskwidget.h"

#include "client/kimairequestfactory.h"

#include <spdlog/spdlog.h>

using namespace kemai::app;
using namespace kemai::client;

TaskWidget::TaskWidget(QWidget* parent) : QWidget(parent), mUi(new Ui::TaskWidget)
{
    mUi->setupUi(this);
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
    default:
        break;
    }
}
