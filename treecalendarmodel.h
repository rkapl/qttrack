#ifndef TREECALENDARMODEL_H
#define TREECALENDARMODEL_H

#include <QAbstractItemModel>

class CalendarModel;
class CalendarTask;

class TreeCalendarModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeCalendarModel(CalendarModel* mModel, QObject *parent = 0);
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    CalendarTask *taskForIndex(const QModelIndex& idx) const;
    QModelIndex indexForTask(CalendarTask* task, int column = 0) const;
private slots:
    void taskTimeChanged(CalendarTask* task);
    void taskAboutToBeAdded(CalendarTask *parent, CalendarTask* task, int position);
    void taskAdded(CalendarTask *parent, CalendarTask* task, int position);
    void taskAboutToBeRemoved(CalendarTask *parent, CalendarTask* task, int position);
    void taskRemoved(CalendarTask *parent, CalendarTask* task, int position);
    void taskAboutToBeMoved(CalendarTask* task, CalendarTask* oldParent, CalendarTask* newParent,
                            int oldPosition, int newPosition);
    void taskMoved(CalendarTask* task, CalendarTask* oldParent, CalendarTask* newParent,
                   int oldPosition, int newPosition);
private:
    CalendarModel* mModel;
};

#endif // TREECALENDARMODEL_H
