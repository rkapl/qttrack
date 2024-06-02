#ifndef EXPORTTEXTDIALOG_H
#define EXPORTTEXTDIALOG_H

#include <QDialog>
#include "calendartask.h"
#include "timeformat.h"

namespace Ui {
class ExportTextDialog;
}

class ExportTextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportTextDialog(QWidget *parent = nullptr);
    void setTimeFormat(const TimeFormat* mimeFormat);
    void setContent(CalendarTask* task);
    ~ExportTextDialog();

private:
    void exportTo(CalendarTask* task, QString &out, int level);

    const TimeFormat* mTimeFormat;
    Ui::ExportTextDialog *ui;
};

#endif // EXPORTTEXTDIALOG_H
