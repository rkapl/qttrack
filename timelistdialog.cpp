#include "timelistdialog.h"
#include "ui_timelistdialog.h"
#include "calendartask.h"
#include "calendartimespan.h"
#include "exporttextdialog.h"
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class TimeListTimeWalker{
public:
    int year;
    QTreeWidgetItem* yearItem;
    TimeSpan yearDuration;

    int month;
    QTreeWidgetItem* monthItem;
    TimeSpan monthDuration;

    int day;
    QTreeWidgetItem* dayItem;
    TimeSpan dayDuration;

};

TimeListDialog::TimeListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeListDialog)
{
    ui->setupUi(this);
    connect(ui->expandDates, &QPushButton::clicked, ui->treeWidget, &QTreeWidget::expandAll);
    connect(ui->recursive, &QCheckBox::toggled, this, &TimeListDialog::updateTree);
    connect(ui->exportAsText, &QPushButton::clicked, this, &TimeListDialog::exportAsText);

    ui->treeWidget->setAlternatingRowColors(true);

}

void TimeListDialog::exportAsText() {
    auto dlg = new ExportTextDialog();
    dlg->setContent(mCurrentTask);
    dlg->exec();
}

void TimeListDialog::setContent(CalendarTask *task){
    mCurrentTask = task;
    updateTree();
}

void TimeListDialog::updateTree(){
    if(mCurrentTask == NULL) return;
    bool recursive =  ui->recursive->isChecked();
    ui->treeWidget->clear();
    ui->summary->setText(mCurrentTask->summary());
    ui->timeSpent->setText(mCurrentTask->duration(recursive).description(TimeSpan::MAX_DETAIL));

    ui->treeWidget->setColumnCount(recursive ? 4 : 3);
    QStringList headerLabels{tr("Start"), tr("Duration"), tr("End")};
    if(recursive)
        headerLabels.append(tr("Task"));
    ui->treeWidget->setHeaderLabels(headerLabels);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    if(recursive)
        ui->treeWidget->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    // Walk throught the time spans and create the tree structure (with nodes for years, months, days and spans).
    // The `walker` keeps track of the name we are in now.
    TimeListTimeWalker walker = {
        -1, NULL, TimeSpan(),
        -1, NULL, TimeSpan(),
        -1, NULL, TimeSpan(),
    };

    QFont italic;
    italic.setItalic(true);

    for(auto span: mCurrentTask->timeSpans(recursive)){
        updateWalker(walker, span);
        QTreeWidgetItem* item = new QTreeWidgetItem(walker.dayItem);
        item->setData(0, Qt::DisplayRole, span->start().time());
        item->setData(1, Qt::DisplayRole, span->duration().description());
        if(span->isFix()){
            item->setData(2, Qt::DisplayRole, tr("Time Fix"));
            item->setData(2, Qt::FontRole, italic);
        }else{
            // display only time (not date) if the start date is the same as end date
            if(span->start().date() != span->end().date()){
                item->setData(2, Qt::DisplayRole, span->end());
            }else{
                item->setData(2, Qt::DisplayRole, span->end().time());
            }
        }
        if(recursive){
            item->setData(3, Qt::DisplayRole, span->task()->summary());
        }
    }

    updateItemWithDuration(walker.yearItem, walker.yearDuration);
    updateItemWithDuration(walker.monthItem, walker.monthDuration);
    updateItemWithDuration(walker.dayItem, walker.dayDuration);
}
void TimeListDialog::updateItemWithDuration(QTreeWidgetItem* item, const TimeSpan &duration){
    if(item != NULL)
        item->setData(1, Qt::DisplayRole, duration.description(TimeSpan::MAX_DETAIL));
}
void TimeListDialog::updateWalker(TimeListTimeWalker &walker, CalendarTimeSpan *span){
    // TODO: maybe also track of the tasks done in the given year/month/day and display them as list?
    QDate start = span->start().date();

    if(start.year() != walker.year){
        updateItemWithDuration(walker.yearItem, walker.yearDuration);
        walker.yearDuration = TimeSpan();

        walker.yearItem = new QTreeWidgetItem(ui->treeWidget, QStringList{QString::number(start.year())});
        walker.year = start.year();
        walker.month = -1;
        walker.day = -1;
    }
    if(start.month() != walker.month){
        updateItemWithDuration(walker.monthItem, walker.monthDuration);
        walker.monthDuration = TimeSpan();

        QString name = QDate::longMonthName(start.month(), QDate::StandaloneFormat);
        walker.monthItem = new QTreeWidgetItem(walker.yearItem, QStringList{name});
        walker.month = start.month();
        walker.day = -1;
    }
    if(start.day() != walker.day){
        updateItemWithDuration(walker.dayItem, walker.dayDuration);
        walker.dayDuration = TimeSpan();

        walker.dayItem = new QTreeWidgetItem(walker.monthItem, QStringList{start.toString("d. MMMM")});
        walker.day = start.day();
    }
    walker.dayDuration += span->duration();
    walker.monthDuration += span->duration();
    walker.yearDuration += span->duration();
}

TimeListDialog::~TimeListDialog()
{
    delete ui;
}
