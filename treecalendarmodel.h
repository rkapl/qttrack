#ifndef TREECALENDARMODEL_H
#define TREECALENDARMODEL_H

#include <QAbstractItemModel>

class CalendarModel;

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
private:
    CalendarModel* mModel;
};

#endif // TREECALENDARMODEL_H
