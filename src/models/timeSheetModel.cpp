#include "timeSheetModel.h"

#include <QCoreApplication>

using namespace kemai;

static const auto gMaxTimeSheetCount = 1000;

void TimeSheetModel::setKemaiSession(const std::shared_ptr<KemaiSession>& kemaiSession)
{
    mSession = kemaiSession;
    resetModel();
}

void TimeSheetModel::setFilterTerm(const QString& term)
{
    mFilterTerm = term;
    resetModel();
}

QVariant TimeSheetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.row() > mTimeSheets.size()))
    {
        return {};
    }

    const auto& timeSheet = mTimeSheets.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return QString("%1 / %2 / %3").arg(timeSheet.project.customer.name, timeSheet.project.name, timeSheet.activity.name);

    default:
        return {};
    }
}

int TimeSheetModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(mTimeSheets.size());
}

void TimeSheetModel::fetchMore(const QModelIndex& /*parent*/)
{
    if (mIsFetching || mEndReached || !mSession)
    {
        return;
    }

    mIsFetching           = true;
    auto timeSheetsResult = mSession->client()->requestTimeSheets(mFilterTerm, mLastPageFetched + 1);

    connect(timeSheetsResult, &KimaiApiBaseResult::ready, this, [this, timeSheetsResult] {
        auto timeSheets = timeSheetsResult->takeResult();

        beginInsertRows({}, static_cast<int>(mTimeSheets.size()), static_cast<int>(mTimeSheets.size() + timeSheets.size() - 1));
        mTimeSheets.insert(mTimeSheets.end(), timeSheets.begin(), timeSheets.end());
        endInsertRows();

        ++mLastPageFetched;
        mEndReached = mTimeSheets.size() >= gMaxTimeSheetCount;
        mIsFetching = false;
        timeSheetsResult->deleteLater();
    });
    connect(timeSheetsResult, &KimaiApiBaseResult::error, [this, timeSheetsResult]() {
        mEndReached = true;
        mIsFetching = false;
        timeSheetsResult->deleteLater();
    });
}

bool TimeSheetModel::canFetchMore(const QModelIndex& /*parent*/) const
{
    while (mIsFetching)
    {
        qApp->processEvents();
    }
    return !mEndReached;
}

void TimeSheetModel::resetModel()
{
    beginResetModel();
    mTimeSheets.clear();
    mEndReached      = mSession == nullptr;
    mLastPageFetched = 0;
    endResetModel();
}
