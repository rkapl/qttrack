#ifndef EXPORTTEXTDIALOG_H
#define EXPORTTEXTDIALOG_H

#include <QDialog>
#include "calendartask.h"

namespace Ui {
class ExportTextDialog;
}

class ExportTextDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportTextDialog(QWidget *parent = nullptr);
    void setContent(CalendarTask* task);
    ~ExportTextDialog();

private:
    void exportTo(CalendarTask* task, QString &out, int level);
    Ui::ExportTextDialog *ui;
};

#endif // EXPORTTEXTDIALOG_H
