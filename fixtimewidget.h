#ifndef FIXTIMEWIDGET_H
#define FIXTIMEWIDGET_H

#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRegularExpressionValidator>

class CalendarTask;

class FixTimeWidget: public QWidget
{
    Q_OBJECT
public:
    FixTimeWidget();
    void setTarget(CalendarTask* ctask);
    CalendarTask* target() const;
signals:
    void done();
public slots:
    void applyFix();
    void applyFixKeepTarget();
private slots:
    void textChanged(const QString& str);
private:
    bool applyFixInternal();

    CalendarTask* mTarget;
    QHBoxLayout mLayout;
    QLineEdit mEntry;
    QPushButton mApply;
};

#endif // FIXTIMEWIDGET_H
