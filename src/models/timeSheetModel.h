#pragma once

#include <QAbstractListModel>

#include <context/kemaiSession.h>

namespace kemai {

class TimeSheetModel : public QAbstractListModel
{
public:
    void setKemaiSession(const std::shared_ptr<KemaiSession>& kemaiSession);
    void setFilterTerm(const QString& term);

    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    void fetchMore(const QModelIndex& parent) override;
    bool canFetchMore(const QModelIndex& parent) const override;

private:
    void resetModel();

    std::shared_ptr<KemaiSession> mSession;
    std::vector<std::unique_ptr<TimeSheet>> mTimeSheets;
    bool mEndReached     = false;
    bool mIsFetching     = false;
    int mLastPageFetched = 0;
    QString mFilterTerm;
};

} // namespace kemai
