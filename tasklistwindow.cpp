#include "tasklistwindow.h"
#include "ui_timetablewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QBoxLayout>
#include <QLineEdit>
#include <QTextStream>

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
    connect(ui->actionFixTaskTime, &QAction::triggered, [this]{fixTimeFor(selectedTask());});
    connect(ui->actionAddTask, &QAction::triggered, this, &TaskListWindow::addNewTask);
    connect(ui->actionRemoveTask, &QAction::triggered, this, &TaskListWindow::removeSelectedTask);
    connect(&mActiveTaskTimeUpdater, &QTimer::timeout, this, &TaskListWindow::updateActiveTaskTicker);

    ui->treeView->setDragEnabled(true);
    ui->treeView->setAcceptDrops(true);;
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->treeView->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

    QWidgetAction* fixMenuAction = new QWidgetAction(this);
    fixMenuAction->setDefaultWidget(&mFixTimeMenuWidget);
    mFixTimeMenu.addAction(fixMenuAction);
    connect(&mFixTimeMenuWidget, &FixTimeWidget::done, &mFixTimeMenu, &QMenu::close);

    mPlay = QIcon(":/icons/start.svg");
    mStop = QIcon(":/icons/stop.svg");

    modelExistenceChanged();
    updateActiveTaskUi();
}
void TaskListWindow::showError(const QString &from, const QString &text){
    QMessageBox::critical(this, from, text);
}
void TaskListWindow::addNewTask(){
    mModel->addTask(NULL, "New Task");
}
void TaskListWindow::removeSelectedTask(){
    if(selectedTask() != NULL){
        bool warnTooMuchTime = selectedTask()->duration(false).msec > DELETE_WARNING_TRESHOLD_MSEC;
        bool warnSubtasks = selectedTask()->subtasks().length() > 0;
        if(warnTooMuchTime || warnSubtasks){
            QMessageBox box(this);
            box.setTextFormat(Qt::RichText);
            box.setWindowTitle("Confirm task deletion");
            box.setIcon(QMessageBox::Warning);
            box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
            box.setDefaultButton(QMessageBox::No);
            QString message;
            QTextStream stream(&message);
            stream << "Are You sure You want to delete the task named <b>" << selectedTask()->summary() << "</b>?";
            stream <<" It can not be undone, so be aware that: <ul>";
            if(warnSubtasks)
                stream << "<li>You will delete the task including all its subtasks</li>";
            if(warnTooMuchTime)
                stream << "<li>You have already spent significant amount of time working on this task</li>";
            stream << "</ul>";
            box.setText(message);
            if(box.exec() !=  QMessageBox::Yes)
                return;
        }
        mModel->removeTask(selectedTask());
    }
}
void TaskListWindow::fixTimeFor(CalendarTask *task){
    if(task == NULL) return;
    if(mActiveTask != NULL) return;
    auto pos = ui->treeView->visualRect(mTreeModel->indexForTask(task, 1)).bottomLeft();
    mFixTimeMenuWidget.setTarget(task);
    mFixTimeMenu.exec(ui->treeView->viewport()->mapToGlobal(pos));
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
    taskSelectionChanged(QItemSelection(), QItemSelection());
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
        mPeriodicSave.start(LOGGING_SAVE_FREQUENCY);
    }else{
        if(previousVisibility)
            resize(size().width(), size().height() - ui->activeTaskInfoPane->size().height());

        ui->activeTaskInfoPane->setVisible(false);
        mActiveTaskTimeUpdater.stop();
        mPeriodicSave.stop();
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
void TaskListWindow::modelExistenceChanged(){
    ui->actionAddTask->setEnabled(mModel != NULL);
    ui->actionRemoveTask->setEnabled(mModel != NULL);
}
void TaskListWindow::taskSelectionChanged(const QItemSelection &, const QItemSelection&){
    CalendarTask* selected = selectedTask();
    bool enabled = selected != NULL;
    ui->actionTaskListing->setEnabled(enabled);
    ui->actionToggleTask->setEnabled(enabled);

    if(selected != NULL && selected != mActiveTask){
        ui->actionToggleTask->setIcon(mPlay);
        ui->actionToggleTask->setText("Log time for selected task");
        ui->actionToggleTask->setEnabled(true);
    }else if(mActiveTask != NULL){
        ui->actionToggleTask->setIcon(mStop);
        ui->actionToggleTask->setText("Stop tracking time");
        ui->actionToggleTask->setEnabled(true);
    }else{
        ui->actionToggleTask->setIcon(mPlay);
        ui->actionToggleTask->setEnabled(false);
    }


    ui->actionFixTaskTime->setEnabled(selected != NULL && mActiveTask != selected);
}
void TaskListWindow::openFileWithDialog(){
    openFile(QFileDialog::getOpenFileName(this, "Open time track", "", "KTimeTracker (*.ics)"));

}
void TaskListWindow::clearModel(){
    mActiveTask = NULL;
    updateActiveTaskUi();
    modelExistenceChanged();

    delete mModel;
    delete mTreeModel;
    mModel = NULL;
    mTreeModel = NULL;
}

void TaskListWindow::openFile(const QString& fileName){
    if(!fileName.isEmpty()){
        clearModel();
        mModel = new CalendarModel(this);
        connect(mModel, &CalendarModel::error, this, &TaskListWindow::showError);
        connect(&mPeriodicSave, &QTimer::timeout, mModel, &CalendarModel::requestSave);

        if(!mModel->load(fileName)){
            return;
        }

        mTreeModel = new TreeCalendarModel(mModel, this);

        // set-up UI (model-dependent)
        ui->treeView->setModel(mTreeModel);
        taskSelectionChanged(QItemSelection(), QItemSelection());
        connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TaskListWindow::taskSelectionChanged);
        connect(mTreeModel, &TreeCalendarModel::itemDropped, [this](const QModelIndex& idx){
            ui->treeView->expand(idx.parent());
            ui->treeView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
        });

        ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        ui->treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        modelExistenceChanged();
    }
}
TaskListWindow::~TaskListWindow()
{
    if(mActiveTask){
        mActiveTask->stopLogging(QDateTime::currentDateTime());
        mActiveTask = NULL;
    }
    if(mModel){
        mModel->doSave(QDateTime::currentDateTimeUtc());
    }

    delete ui;
}
