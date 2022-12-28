#include "taskwidget.h"
#include "ui_taskwidget.h"

#include <QAction>
#include <QTimer>

#include <spdlog/spdlog.h>

using namespace kemai;

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
    connect(mUi->tbRefresh, &QPushButton::clicked, this, &TaskWidget::updateTasks);
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
        updateTasks();
    }

    setEnabled(mClient != nullptr);
}

void TaskWidget::setKemaiSession(QSharedPointer<KemaiSession> kemaiSession)
{
    mTaskProxyModel.setUserId(kemaiSession->me.id);
}

void TaskWidget::updateTasks()
{
    auto tasksResult = mClient->requestTasks();

    connect(tasksResult, &KimaiApiBaseResult::ready, this, [this, tasksResult] {
        mTaskModel.setTasks(tasksResult->getResult());
        tasksResult->deleteLater();
    });
    connect(tasksResult, &KimaiApiBaseResult::error, [tasksResult] { tasksResult->deleteLater(); });
}

void TaskWidget::onTaskItemChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    mUi->btStartStop->setEnabled(current.isValid());
    mUi->btClose->setEnabled(current.isValid());
}

void TaskWidget::onStartStopClicked()
{
    auto taskId = mTaskModel.data(mTaskProxyModel.mapToSource(mUi->lvTasks->currentIndex()), TaskListModel::TaskIDRole).toInt();

    auto taskResult = mClient->startTask(taskId);
    connect(taskResult, &KimaiApiBaseResult::ready, this, [this, taskResult] {
        updateTasks();
        taskResult->deleteLater();
    });
    connect(taskResult, &KimaiApiBaseResult::error, [taskResult] { taskResult->deleteLater(); });
}

void TaskWidget::onCloseClicked()
{
    auto taskId = mTaskModel.data(mTaskProxyModel.mapToSource(mUi->lvTasks->currentIndex()), TaskListModel::TaskIDRole).toInt();

    auto taskResult = mClient->closeTask(taskId);
    connect(taskResult, &KimaiApiBaseResult::ready, this, [this, taskResult] {
        updateTasks();
        taskResult->deleteLater();
    });
    connect(taskResult, &KimaiApiBaseResult::error, [taskResult] { taskResult->deleteLater(); });
}
