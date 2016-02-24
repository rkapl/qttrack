#ifndef TIMESPAN_H
#define TIMESPAN_H

#include <QObject>

class QDateTime;

struct TimeSpan
{
public:
    TimeSpan();
    TimeSpan(qint64 msec);
    QString description() const;

    TimeSpan& operator+=(const TimeSpan& add);
    TimeSpan operator+(const TimeSpan& b) const;

    qint64 msec;
};

TimeSpan operator-(const QDateTime& a, const QDateTime& b);


#endif // TIMESPAN_H
