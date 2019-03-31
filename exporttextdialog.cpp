#include "exporttextdialog.h"
#include "ui_exporttextdialog.h"
#include "calendartimespan.h"

static void indent(QString& out, int level) {
    for(int i = 0; i<level; i++) {
        out.append('\t');
    }
}

ExportTextDialog::ExportTextDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportTextDialog)
{
    ui->setupUi(this);
}

void ExportTextDialog::setContent(CalendarTask* task) {
    QString out;
    exportTo(task, out, 0);
    ui->text->setPlainText(out);
}

void ExportTextDialog::exportTo(CalendarTask* task, QString &out, int level) {
    indent(out, level);
    out.append(QString("#%1, duration: %2, with subtasks: %3\n").arg(task->summary(),
       task->duration(false).description(2),
       task->duration(true).description(2)));

    for (CalendarTimeSpan *span: task->timeSpans(false)) {
        indent(out, level + 1);
        auto duration = span->duration().description(2);
        auto start = span->start().toString();
        if (span->isFix()) {
            out.append(QString("FIX at %1: %2").arg(start, duration));
        } else {
            out.append(QString("WORK at %1: %2").arg(start, duration));
        }
        out.append("\n");
    }
    for (CalendarTask *sub: task->subtasks()) {
        exportTo(sub, out, level + 1);
    }
}

ExportTextDialog::~ExportTextDialog()
{
    delete ui;
}
