#ifndef TIMESPAN_H
#define TIMESPAN_H

#include <QObject>
#include <QRegularExpression>

class QDateTime;

class TimeSpan
{
public:
    TimeSpan();
    explicit TimeSpan(qint64 msec);
    TimeSpan& operator+=(const TimeSpan& add);
    TimeSpan operator+(const TimeSpan& b) const;

    QString description(int maxdetail = 2) const;
    static TimeSpan parse(const QString& str, bool* ok);
    static const QRegularExpression &timeSpanRegex();
    static constexpr int MAX_DETAIL = 4;

    qint64 msec;

private:
    static const QRegularExpression mRegExp;
};

TimeSpan operator-(const QDateTime& a, const QDateTime& b);


#endif // TIMESPAN_H
