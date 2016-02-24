#include "timelistdialog.h"
#include "ui_timelistdialog.h"
#include "calendartask.h"
#include "calendartimespan.h"
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
    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setHeaderLabels(QStringList{"Start", "Duration", "End"});
    ui->treeWidget->setAlternatingRowColors(true);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}
void TimeListDialog::setContent(CalendarTask *task){
    ui->treeWidget->clear();
    ui->summary->setText(task->summary());
    ui->timeSpent->setText(task->duration(false).description(TimeSpan::MAX_DETAIL));

    TimeListTimeWalker walker = {
        -1, NULL, TimeSpan(),
        -1, NULL, TimeSpan(),
        -1, NULL, TimeSpan(),
    };
    for(auto span: task->timeSpans()){
        updateWalker(walker, span);
        QTreeWidgetItem* item = new QTreeWidgetItem(walker.dayItem);
        item->setData(0, Qt::DisplayRole, span->start().time());
        item->setData(1, Qt::DisplayRole, span->duration().description());
        if(span->start().date() != span->end().date()){
            item->setData(2, Qt::DisplayRole, span->start());
        }else{
            item->setData(2, Qt::DisplayRole, span->start().time());
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
