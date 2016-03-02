#include "timespan.h"
#include <QDateTime>
#include <QStringList>
#include <QDebug>
#include <QRegularExpression>
#include <QMap>
#include <QStringRef>


TimeSpan::TimeSpan():
    msec(0)
{

}
TimeSpan::TimeSpan(qint64 msec):
    msec(msec){

}
const QRegularExpression TimeSpan::mRegExp = []{
    QStringList validNames{
        "s","sec","secs","second","seconds",
        "m","min","mins","minute","minutes",
        "h","hour","hours",
        "d","day","days"
    };
    QString part(QString("((\\d+)\\s*(%1))").arg(validNames.join("|")));
    return QRegularExpression(QString("^\\s*[+-]?\\s*%1(\\s+%1)*\\s*$").arg(part),QRegularExpression::CaseInsensitiveOption);
}();

TimeSpan TimeSpan::parse(const QString &str, bool *ok){
    *ok = false;

    // first a complete check for correctness
    auto match = timeSpanRegex().match(str);
    if(!match.hasMatch()){
        return TimeSpan();
    }

    int sign = 1;
    TimeSpan span;

    // lex the string
    int pos = 0;
    auto current = [&]{return pos < str.length() ? str.at(pos).unicode() : '$';};
    auto skip_ws = [&]{
      while(pos < str.length() && str.at(pos).isSpace())
          pos++;
    };
    // lex sign
    skip_ws();
    if(current() == '-'){
        sign = -1;
        pos++;
    }
    if(current() == '+'){
        sign = 1;
        pos++;
    }
    skip_ws();

    while(current() != '$'){
        // lex the time
        quint64 amount = 0;
        while(current() >= '0' && current() <= '9'){
            amount = amount*10 + current() - '0';
            pos++;
        }
        skip_ws();
        // multiply by the unit
        switch (current()) {
        case 'D':
        case 'd':
            amount*=24;
        case 'h':
        case 'H':
            amount*=60;
        case 'm':
        case 'M':
            amount*=60;
        case 's':
        case 'S':
            amount*=1000;
            break;
        default:
            throw std::logic_error("Lex error");
        }
        span.msec += amount;
        // skip the rest of the unit
        while((current() >= 'a' && current() <= 'z') || (current() >= 'A' && current() <= 'Z')){
            pos++;
        }
        skip_ws();
    }
    span.msec *= sign;

    *ok = true;
    return span;
}
const QRegularExpression& TimeSpan::timeSpanRegex(){
    return mRegExp;
}

/**
 * Writes a part of TimeSpan description (such as minutes, hours ...)
 * into a string if it is appropriate: leading zero parts and parts
 * exceeding the wanted detail are emitted.
 */
#define wpart(name, var) \
    if( (var != 0 || noskip) && detail < maxdetail){ \
        noskip = true; \
        detail++; \
        text.append(QString("%1 " name).arg(var)); \
    }
QString TimeSpan::description(int maxdetail) const{
    QStringList text;
    qint64 msec = TimeSpan::msec;
    if(msec < 0){
        msec = -msec;
        text.append("-");
    }

    qint64 seconds = msec / 1000;
    qint64 minutes = seconds / 60;
    seconds -= minutes * 60;
    qint64 hours = minutes / 60;
    minutes -= hours * 60;
    qint64 days = hours / 24;
    hours -= days * 24;



    bool noskip = false;
    int detail = 0;
    wpart("d", days);
    wpart("h", hours);
    wpart("m", minutes);
    noskip = true;
    wpart("s", seconds);
    return text.join(" ");
}
TimeSpan operator-(const QDateTime& a, const QDateTime& b){
    return TimeSpan(b.msecsTo(a));
}
TimeSpan& TimeSpan::operator+=(const TimeSpan& add){
    msec += add.msec;
    return *this;
}
TimeSpan TimeSpan::operator+(const TimeSpan& add) const{
    return TimeSpan(msec + add.msec);
}
