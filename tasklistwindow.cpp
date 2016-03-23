#include "tasklistwindow.h"
#include "ui_timetablewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QBoxLayout>
#include <QLineEdit>
#include <QTextStream>
#include <QFile>

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
    mActiveTask(NULL),
    mSettings()
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

    mCaptionBase = windowTitle();

    QWidgetAction* fixMenuAction = new QWidgetAction(this);
    fixMenuAction->setDefaultWidget(&mFixTimeMenuWidget);
    mFixTimeMenu.addAction(fixMenuAction);
    connect(&mFixTimeMenuWidget, &FixTimeWidget::done, &mFixTimeMenu, &QMenu::close);

    mPlay = QIcon(":/icons/start.svg");
    mStop = QIcon(":/icons/stop.svg");

    modelExistenceChanged();
    updateActiveTaskUi();
    taskSelectionChanged(QItemSelection(), QItemSelection());
}
void TaskListWindow::checkForImport(){
    QString defaultPath = CalendarModel::pathDefaultCalendar();
    bool defaultExists = QFile(defaultPath).exists();
    if(!defaultExists){
        QString oldPath = CalendarModel::pathKTimeTracker();
        bool oldExists = QFile(oldPath).exists();
        if(oldExists){
            if(!mSettings.contains("importDismissed") || !mSettings.value("importDismissed").toBool()){
                doImport();
            }
        }
    }
}
void TaskListWindow::openDefault(){
    checkForImport();
    QString path = CalendarModel::pathDefaultCalendar();
    if(QFile::exists(path)){
        openFile(path);
    }else{
        createFile(path);
        if(mModel)
            mModel->createExampleTask();
    }
}
void TaskListWindow::doImport(){
    QMessageBox askImport(this);
    askImport.setWindowTitle("Import KTimeTracker data?");
    askImport.setText(QString(
        "<p>A saved file from KTimeTracker was found on this computer (in <i>%1</i>). "
        "Do You want to import the tracking information into Q Time Tracker?</p>"
        "<p><b>Note:</b> The tracking information will be stored in <i>%2</i>. The file"
        "format will still be compatible with KTimeTracker.</p>")
                      .arg(CalendarModel::pathKTimeTracker())
                      .arg(CalendarModel::pathDefaultCalendar()));
    askImport.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if(askImport.exec() == QMessageBox::Yes){
        QFileInfo file(CalendarModel::pathDefaultCalendar());
        if(!file.dir().mkpath(file.path())){
            QMessageBox::critical(this, "Error", "Can not create directory " + file.path());
            return;
        }
        if(!QFile::copy(CalendarModel::pathKTimeTracker(), CalendarModel::pathDefaultCalendar())){
            QMessageBox::critical(this, "Error", QString("Can not copy KTimTracker information from %1 to %2")
                                  .arg(CalendarModel::pathKTimeTracker())
                                  .arg(CalendarModel::pathDefaultCalendar()));
            return;
        }
    }else{
        mSettings.setValue("importDismissed", QVariant(true));
    }
}
void TaskListWindow::showError(const QString &from, const QString &text){
    QMessageBox::critical(this, from, text);
}
void TaskListWindow::addNewTask(){
    if(mTreeModel != NULL){
        CalendarTask* task = mModel->addTask(NULL, "New Task");
        ui->treeView->selectionModel()->select(mTreeModel->indexForTask(task), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ui->treeView->edit(mTreeModel->indexForTask(task));
    }
}
void TaskListWindow::removeSelectedTask(){
    auto selectedTask = this->selectedTask();
    if(selectedTask != NULL){
        bool warnTooMuchTime = selectedTask->duration(false).msec > DELETE_WARNING_TRESHOLD_MSEC;
        bool warnSubtasks = selectedTask->subtasks().length() > 0;
        if(warnTooMuchTime || warnSubtasks){
            QMessageBox box(this);
            box.setTextFormat(Qt::RichText);
            box.setWindowTitle("Confirm task deletion");
            box.setIcon(QMessageBox::Warning);
            box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
            box.setDefaultButton(QMessageBox::No);
            QString message;
            QTextStream stream(&message);
            stream << "Are You sure You want to delete the task named <b>" << selectedTask->summary() << "</b>?";
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

        // check if the task has an active subtask and stop it if necessary
        auto task = mActiveTask;
        while(task != NULL){
            if(task == selectedTask){
                mActiveTask->stopLogging(QDateTime::currentDateTime());
                mActiveTask = NULL;
                updateActiveTaskUi();
            }
            task = task->parent();
        }

        mModel->removeTask(selectedTask);
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
    // the selection does not matter, the method fetches the selection from the model
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
}
void TaskListWindow::taskSelectionChanged(const QItemSelection &, const QItemSelection&){
    CalendarTask* selected = selectedTask();
    bool enabled = selected != NULL;
    ui->actionTaskListing->setEnabled(enabled);
    ui->actionToggleTask->setEnabled(enabled);
    ui->actionRemoveTask->setEnabled(enabled);

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
    QString file = QFileDialog::getOpenFileName(this, "Open time track", "", "KTimeTracker (*.ics)");
    if(!file.isEmpty())
        openFile(file, true);

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
void TaskListWindow::createFile(const QString &fileName, bool showInCaption){
    clearModel();
    mModel = new CalendarModel(this);
    connect(mModel, &CalendarModel::error, this, &TaskListWindow::showError);

    if(!mModel->create(fileName)){
        delete mModel;
        mModel = NULL;
        clearCaption();
        return;
    }
    connectNewModel();
    updateCaption(fileName, showInCaption);
}
void TaskListWindow::openFile(const QString& fileName, bool showInCaption){
    clearModel();
    mModel = new CalendarModel(this);
    connect(mModel, &CalendarModel::error, this, &TaskListWindow::showError);

    if(!mModel->load(fileName)){
        delete mModel;
        mModel = NULL;
        clearCaption();
        return;
    }
    connectNewModel();
    updateCaption(fileName, showInCaption);
}
void TaskListWindow::clearCaption(){
    setWindowTitle(mCaptionBase);
}
void TaskListWindow::updateCaption(const QString &fileName, bool showInCaption){
    if(showInCaption){
        setWindowTitle(QString("%1 [%2]").arg(mCaptionBase, fileName));
    }else{
        setWindowTitle(mCaptionBase);
    }
}

void TaskListWindow::connectNewModel(){
    connect(&mPeriodicSave, &QTimer::timeout, mModel, &CalendarModel::save);

    mTreeModel = new TreeCalendarModel(mModel, this);

    // set-up UI (model-dependent)
    ui->treeView->setModel(mTreeModel);
    taskSelectionChanged(QItemSelection(), QItemSelection());
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TaskListWindow::taskSelectionChanged);
    connect(mTreeModel, &TreeCalendarModel::itemDropped, [this](const QModelIndex& idx){
        ui->treeView->expand(idx.parent());
        ui->treeView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    });

    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    modelExistenceChanged();
}

TaskListWindow::~TaskListWindow()
{
    if(mActiveTask != NULL){
        mActiveTask->stopLogging(QDateTime::currentDateTime());
        mActiveTask = NULL;
    }
    if(mModel != NULL){
        mModel->save();
    }

    delete ui;
}
