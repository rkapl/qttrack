#include "treecalendarmodel.h"
#include "calendarmodel.h"
#include "calendartask.h"

TreeCalendarModel::TreeCalendarModel(CalendarModel *model, QObject *parent) :
    QAbstractItemModel(parent),
    mModel(model)
{
    connect(model, &CalendarModel::timesChanged, this, &TreeCalendarModel::taskTimeChanged);
    connect(model, &CalendarModel::taskAboutToBeAdded, this, &TreeCalendarModel::taskAboutToBeAdded);
    connect(model, &CalendarModel::taskAboutToBeMoved, this, &TreeCalendarModel::taskAboutToBeMoved);
    connect(model, &CalendarModel::taskAboutToBeRemoved, this, &TreeCalendarModel::taskAboutToBeRemoved);
    connect(model, &CalendarModel::taskAdded, this, &TreeCalendarModel::taskAdded);
    connect(model, &CalendarModel::taskMoved, this, &TreeCalendarModel::taskMoved);
    connect(model, &CalendarModel::taskRemoved, this, &TreeCalendarModel::taskRemoved);
}
void TreeCalendarModel::taskAboutToBeAdded(CalendarTask*parent, CalendarTask* task, int position){
    Q_UNUSED(task);
    beginInsertRows(indexForTask(parent), position, position);
}
void TreeCalendarModel::taskAdded(CalendarTask *parent, CalendarTask *task, int position){
    Q_UNUSED(parent); Q_UNUSED(task); Q_UNUSED(position);
    endInsertRows();
}
void TreeCalendarModel::taskAboutToBeMoved(CalendarTask *task, CalendarTask *oldParent, CalendarTask *newParent,
                                           int oldPosition, int newPosition){
    Q_UNUSED(task);
    beginMoveRows(indexForTask(oldParent), oldPosition, oldPosition,
                  indexForTask(newParent), newPosition);
}
void TreeCalendarModel::taskMoved(CalendarTask *task, CalendarTask *oldParent, CalendarTask *newParent,
                                  int oldPosition, int newPosition){
    Q_UNUSED(task); Q_UNUSED(oldParent); Q_UNUSED(oldPosition); Q_UNUSED(newParent); Q_UNUSED(newPosition);
    endMoveRows();
}
void TreeCalendarModel::taskAboutToBeRemoved(CalendarTask *parent, CalendarTask *task, int position){
    Q_UNUSED(task);
    beginRemoveRows(indexForTask(parent), position, position);
}
void TreeCalendarModel::taskRemoved(CalendarTask *parent, CalendarTask *task, int position){
    Q_UNUSED(task); Q_UNUSED(parent); Q_UNUSED(position);
    endRemoveRows();
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
    // this can apparently be called with invalid rows ?
    if(!parent.isValid()){
        if( row >= 0 && row < mModel->rootTasks().length())
            return createIndex(row, column, mModel->rootTasks()[row]);
    }else{
        const auto& tasks = ((CalendarTask*)parent.internalPointer())->subtasks();
        if(row >= 0 && row < tasks.length())
            return createIndex(row, column, tasks[row]);
    }
    return QModelIndex();
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
    if(task == NULL)
        return QModelIndex();
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
