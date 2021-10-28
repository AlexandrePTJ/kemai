#include "taskwidget.h"
#include "ui_taskwidget.h"

#include <QAction>
#include <QTimer>

#include <spdlog/spdlog.h>

#include "client/kimairequestfactory.h"

using namespace kemai::app;
using namespace kemai::core;
using namespace kemai::client;
using namespace kemai::models;

TaskWidget::TaskWidget(QWidget* parent) : QWidget(parent), mUi(new Ui::TaskWidget)
{
    mUi->setupUi(this);

    mTaskProxyModel.setSourceModel(&mTaskModel);
    mUi->lvTasks->setModel(&mTaskProxyModel);

    auto clearTextAction = mUi->leFilter->addAction(QIcon(":/icons/backspace"), QLineEdit::TrailingPosition);

    connect(clearTextAction, &QAction::triggered, mUi->leFilter, &QLineEdit::clear);
    connect(mUi->leFilter, &QLineEdit::textChanged, [&](const QString& text) { mTaskProxyModel.setFilterRegularExpression(QString(".*%1.*").arg(text)); });
    connect(mUi->lvTasks->selectionModel(), &QItemSelectionModel::currentChanged, this, &TaskWidget::onTaskItemChanged);
    connect(mUi->btStartStop, &QPushButton::clicked, this, &TaskWidget::onStartStopClicked);
    connect(mUi->btClose, &QPushButton::clicked, this, &TaskWidget::onCloseClicked);
    connect(mUi->tbRefresh, &QPushButton::clicked, this, &TaskWidget::onRefreshClicked);
}

TaskWidget::~TaskWidget()
{
    delete mUi;
}

void TaskWidget::setKimaiClient(QSharedPointer<KimaiClient> kimaiClient)
{
    mClient = std::move(kimaiClient);
    if (mClient)
    {
        connect(mClient.get(), &KimaiClient::replyReceived, this, &TaskWidget::onClientReply);

        mClient->sendRequest(KimaiRequestFactory::tasks());
    }

    setEnabled(mClient != nullptr);
}

void TaskWidget::setKemaiSession(QSharedPointer<KemaiSession> kemaiSession)
{
    mTaskProxyModel.setUserId(kemaiSession->me.id);
}

void TaskWidget::onClientReply(const KimaiReply& reply)
{
    if (!reply.isValid())
    {
        return;
    }

    switch (reply.method())
    {
    case ApiMethod::Tasks: {
        auto tasks = reply.get<Tasks>();
        mTaskModel.setTasks(tasks);
    }
    break;

    case ApiMethod::TaskStart:
    case ApiMethod::TaskClose:
    case ApiMethod::TimeSheets:
        mClient->sendRequest(KimaiRequestFactory::tasks());
        break;

    default:
        break;
    }
}

void TaskWidget::onTaskItemChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    mUi->btStartStop->setEnabled(current.isValid());
    mUi->btClose->setEnabled(current.isValid());
}

void TaskWidget::onRefreshClicked()
{
    mClient->sendRequest(KimaiRequestFactory::tasks());
}

void TaskWidget::onStartStopClicked()
{
    auto taskId = mTaskModel.data(mTaskProxyModel.mapToSource(mUi->lvTasks->currentIndex()), TaskListModel::TaskIDRole).toInt();
    mClient->sendRequest(KimaiRequestFactory::taskStart(taskId));

    // Delay update request to let kimai update status.
    QTimer::singleShot(500, [&]() {
        mClient->sendRequest(KimaiRequestFactory::tasks());
        mClient->sendRequest(KimaiRequestFactory::activeTimeSheets());
    });
}

void TaskWidget::onCloseClicked()
{
    auto taskId = mTaskModel.data(mTaskProxyModel.mapToSource(mUi->lvTasks->currentIndex()), TaskListModel::TaskIDRole).toInt();
    mClient->sendRequest(KimaiRequestFactory::taskClose(taskId));
}
