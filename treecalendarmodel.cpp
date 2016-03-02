#include "treecalendarmodel.h"
#include "calendarmodel.h"
#include "calendartask.h"

TreeCalendarModel::TreeCalendarModel(CalendarModel *model, QObject *parent) :
    QAbstractItemModel(parent),
    mModel(model)
{
    connect(model, &CalendarModel::timesChanged, this, &TreeCalendarModel::taskTimeChanged);
}
void TreeCalendarModel::taskTimeChanged(CalendarTask *task){
    QModelIndex index = indexForTask(task,1);
    while(index.isValid()){
        emit dataChanged(index, sibling(index.row(), 2, index));
        index = parent(index);
    }
}
CalendarTask* TreeCalendarModel::taskForIndex(const QModelIndex& idx) const{
    return static_cast<CalendarTask*>(idx.internalPointer());
}
QModelIndex TreeCalendarModel::index(int row, int column, const QModelIndex &parent) const{
    if(!parent.isValid())
        return createIndex(row, column, mModel->rootTasks()[row]);
    else
        return createIndex(row, column, ((CalendarTask*)parent.internalPointer())->subtasks()[row]);
}
QModelIndex TreeCalendarModel::parent(const QModelIndex &child) const{
    if(!child.isValid()){
        return QModelIndex();
    }else{
        CalendarTask* parent = static_cast<CalendarTask*>(child.internalPointer())->parent();
        if(parent == NULL){
            return QModelIndex();
        }else{
            return indexForTask(parent);
        }
    }
}
QModelIndex TreeCalendarModel::indexForTask(CalendarTask *task, int column) const{
    int indexInParent;
    if(task->parent() == NULL)
        indexInParent = mModel->rootTasks().indexOf(task);
    else
        indexInParent = task->parent()->subtasks().indexOf(task);
    return createIndex(indexInParent, column, task);
}

int TreeCalendarModel::rowCount(const QModelIndex &parent) const{
    if(parent.isValid()){
        return static_cast<CalendarTask*>(parent.internalPointer())->subtasks().length();
    }else{
        return mModel->rootTasks().length();
    }
}
int TreeCalendarModel::columnCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return 3;
}
QVariant TreeCalendarModel::data(const QModelIndex &index, int role) const{
    CalendarTask* task = static_cast<CalendarTask*>(index.internalPointer());
    if(role == Qt::DisplayRole){
        switch(index.column()){
        case 0:
            return task->summary();
        case 1:
            return task->duration(false).description();
        case 2:
            return task->duration(true).description();
        }
    }
    return QVariant();
}
QVariant TreeCalendarModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch (section) {
        case 0:
            return "Summary";
        case 1:
            return "Time";
        case 2:
            return "Total time";
        }
    }
    return QVariant();
}
