#include "tasklistwindow.h"
#include "ui_timetablewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include "calendarmodel.h"
#include "treecalendarmodel.h"
#include "timelistdialog.h"

TaskListWindow::TaskListWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TimeTableWindow),
    mModel(NULL),
    mTreeModel(NULL)
{
    ui->setupUi(this);

    // set-up the rest of the UI
    connect(ui->actionOpen, &QAction::triggered, this, &TaskListWindow::openFileWithDialog);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &TaskListWindow::timeDetailsFor);
    connect(ui->actionTaskListing, &QAction::triggered, this, &TaskListWindow::timeDetailsForSelection);

    ui->actionOpen->setIcon(this->style()->standardIcon(QStyle::SP_DialogOpenButton));
}
void TaskListWindow::timeDetailsForSelection(){
    if(ui->treeView->selectionModel())
        timeDetailsFor(ui->treeView->selectionModel()->selection().first().topLeft());
}
void TaskListWindow::timeDetailsFor(const QModelIndex &selection){
    if(selection.isValid()){
        TimeListDialog timeList(this);
        timeList.setContent(mTreeModel->taskForIndex(selection));
        timeList.exec();
    }
}
void TaskListWindow::taskSelectionChanged(const QItemSelection &selected, const QItemSelection&){
    bool enabled = !selected.isEmpty();
    ui->actionTaskListing->setEnabled(enabled);
}
void TaskListWindow::openFileWithDialog(){
    openFile(QFileDialog::getOpenFileName(this, "Open time track", "", "KTimeTracker (*.ics)"));

}
void TaskListWindow::openFile(const QString& fileName){
    if(!fileName.isEmpty()){
        if(mModel != NULL)
            delete mModel;
        if(mTreeModel != NULL)
            delete mTreeModel;
        mModel = new CalendarModel(this);

        // create new model
        if(!mModel->load(fileName)){
            QMessageBox::critical(this, "Error", "Can not load KTimeTracker file");
        }
        mTreeModel = new TreeCalendarModel(mModel, this);

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
    delete ui;
}
