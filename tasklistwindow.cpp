#include "tasklistwindow.h"
#include "ui_timetablewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "calendarmodel.h"
#include "treecalendarmodel.h"
#include "timelistdialog.h"
#include "calendartask.h"
#include "calendartimespan.h"

TaskListWindow::TaskListWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TimeTableWindow),
    mModel(NULL),
    mTreeModel(NULL),
    mActiveTask(NULL)
{
    ui->setupUi(this);

    // set-up the rest of the UI
    connect(ui->actionOpen, &QAction::triggered, this, &TaskListWindow::openFileWithDialog);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &TaskListWindow::timeDetailsForSelection);
    connect(ui->actionTaskListing, &QAction::triggered, this, &TaskListWindow::timeDetailsForCurrentSelection);
    connect(ui->actionToggleTask, &QAction::triggered, this, &TaskListWindow::toggleTask);
    connect(&mActiveTaskTimeUpdater, &QTimer::timeout, this, &TaskListWindow::updateActiveTaskTicker);

    ui->actionOpen->setIcon(this->style()->standardIcon(QStyle::SP_DialogOpenButton));
}
CalendarTask* TaskListWindow::selectedTask() const{
    auto smodel = ui->treeView->selectionModel();
    if(smodel != NULL && !smodel->selection().isEmpty())
        return mTreeModel->taskForIndex(smodel->selection().first().topLeft());
    return NULL;
}
void TaskListWindow::toggleTask(){
    CalendarTask* newTask = selectedTask();

    if(mActiveTask != NULL){
        mActiveTask->stopLogging(QDateTime::currentDateTime());
    }

    if(newTask == mActiveTask){
        mActiveTask = NULL;
    }else{
        mActiveTask = newTask;
    }

    if(mActiveTask != NULL){
        mActiveTimeSpan = mActiveTask->startLogging(QDateTime::currentDateTime());
    }

    updateActiveTaskUi();
}
void TaskListWindow::updateActiveTaskUi(){
    bool previousVisibility = ui->activeTaskInfoPane->isVisible();
    if(mActiveTask != NULL){
        if(!previousVisibility)
            resize(size().width(), size().height() + ui->activeTaskInfoPane->size().height());

        ui->activeTaskInfoPane->setVisible(true);
        ui->activeTaskSummary->setText(mActiveTask->summary());
        updateActiveTaskTicker();
        mActiveTaskTimeUpdater.start(1000);
    }else{
        if(previousVisibility)
            resize(size().width(), size().height() - ui->activeTaskInfoPane->size().height());

        ui->activeTaskInfoPane->setVisible(false);
        mActiveTaskTimeUpdater.stop();
    }
}
void TaskListWindow::updateActiveTaskTicker(){
    ui->activeTaskTime->setText((QDateTime::currentDateTime() - mActiveTimeSpan->start()).description());
    if(QDate::currentDate() == mActiveTimeSpan->start().date())
        ui->activeTaskStart->setText(mActiveTimeSpan->start().time().toString());
    else
        ui->activeTaskStart->setText(mActiveTimeSpan->start().toString());
}
void TaskListWindow::timeDetailsForSelection(const QModelIndex &selection){
    timeDetailsFor(mTreeModel->taskForIndex(selection));
}
void TaskListWindow::timeDetailsForCurrentSelection(){
    timeDetailsFor(selectedTask());
}
void TaskListWindow::timeDetailsFor(CalendarTask* task){
    if(task != NULL){
        TimeListDialog timeList(this);
        timeList.setContent(task);
        timeList.exec();
    }
}
void TaskListWindow::taskSelectionChanged(const QItemSelection &, const QItemSelection&){
    CalendarTask* selected = selectedTask();
    bool enabled = selected != NULL;
    ui->actionTaskListing->setEnabled(enabled);
    ui->actionToggleTask->setEnabled(enabled);

    if(selected != NULL){
        ui->actionToggleTask->setIcon(this->style()->standardIcon(QStyle::SP_MediaPlay));
        ui->actionToggleTask->setText("Log time for selected task");
        ui->actionToggleTask->setEnabled(true);
    }else if(mActiveTask != NULL){
        ui->actionToggleTask->setIcon(this->style()->standardIcon(QStyle::SP_MediaStop));
        ui->actionToggleTask->setText("Stop tracking time");
        ui->actionToggleTask->setEnabled(true);
    }else{
        ui->actionToggleTask->setIcon(this->style()->standardIcon(QStyle::SP_MediaPlay));
        ui->actionToggleTask->setEnabled(false);
    }
}
void TaskListWindow::openFileWithDialog(){
    openFile(QFileDialog::getOpenFileName(this, "Open time track", "", "KTimeTracker (*.ics)"));

}
void TaskListWindow::clearModel(){
    mActiveTask = NULL;
    updateActiveTaskUi();

    if(mModel != NULL)
        delete mModel;
    if(mTreeModel != NULL)
        delete mTreeModel;
}

void TaskListWindow::openFile(const QString& fileName){
    if(!fileName.isEmpty()){
        clearModel();
        mModel = new CalendarModel(this);
        mTreeModel = new TreeCalendarModel(mModel, this);
        if(!mModel->load(fileName)){
            QMessageBox::critical(this, "Error", "Can not load KTimeTracker file");
        }

        // set-up UI (model-dependent)
        ui->treeView->setModel(mTreeModel);
        taskSelectionChanged(QItemSelection(), QItemSelection());
        connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TaskListWindow::taskSelectionChanged);

        ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        ui->treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    }
}
TaskListWindow::~TaskListWindow()
{
    if(mActiveTask){
        mActiveTask->stopLogging(QDateTime::currentDateTime());
        mActiveTask = NULL;
    }

    delete ui;
}
