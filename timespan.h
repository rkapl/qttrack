#ifndef TIMESPAN_H
#define TIMESPAN_H

#include <QObject>

class QDateTime;

struct TimeSpan
{
public:
    TimeSpan();
    explicit TimeSpan(qint64 msec);
    QString description(int maxdetail = 2) const;
    TimeSpan& operator+=(const TimeSpan& add);
    TimeSpan operator+(const TimeSpan& b) const;

    static constexpr int MAX_DETAIL = 4;

    qint64 msec;
};

TimeSpan operator-(const QDateTime& a, const QDateTime& b);


#endif // TIMESPAN_H
