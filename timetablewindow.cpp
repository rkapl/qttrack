#include "timetablewindow.h"
#include "ui_timetablewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "calendarmodel.h"
#include "treecalendarmodel.h"

TimeTableWindow::TimeTableWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TimeTableWindow),
    mModel(NULL),
    mTreeModel(NULL)
{
    ui->setupUi(this);

    // set-up the rest of the UI
    connect(ui->actionOpen, &QAction::triggered, this, &TimeTableWindow::openFileWithDialog);

    ui->actionOpen->setIcon(this->style()->standardIcon(QStyle::SP_DialogOpenButton));
}

void TimeTableWindow::openFileWithDialog(){
    openFile(QFileDialog::getOpenFileName(this, "Open time track", "", "KTimeTracker (*.ics)"));

}
void TimeTableWindow::openFile(const QString& fileName){
    if(!fileName.isEmpty()){
        if(mModel != NULL)
            delete mModel;
        if(mTreeModel != NULL)
            delete mTreeModel;
        mModel = new CalendarModel(this);
        if(!mModel->load(fileName)){
            QMessageBox::critical(this, "Error", "Can not load KTimeTracker file");
        }

        mTreeModel = new TreeCalendarModel(mModel, this);
        ui->treeView->setModel(mTreeModel);
        ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        ui->treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    }
}
TimeTableWindow::~TimeTableWindow()
{
    delete ui;
}
