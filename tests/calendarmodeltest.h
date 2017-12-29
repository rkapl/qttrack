#ifndef CALENDARMODELTEST_H
#define CALENDARMODELTEST_H

#include <QObject>

class CalendarModelTest: public QObject
{
    Q_OBJECT
public:
private:
    static QStringList fetchLines(const QString& file);
    static qint64 parseOrDie(const QString& data);
private slots:
    void formatCycle();
    void parseTimeSpan();
    void error(const QString& source, const QString& icalError);
};

#endif // CALENDARMODELTEST_H
