#include "timespan.h"
#include <QDateTime>
#include <QStringList>
#include <QDebug>
#include <QRegularExpression>
#include <QMap>
#include <QString>
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
    QString part(QString("((\\d+(\\.\\d+)?)\\s*(%1))").arg(validNames.join("|")));
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
        QString amount_str;
        while(current() >= '0' && current() <= '9') {
            amount_str.push_back(current());
            pos++;
        }

        //decimal part
        if (current() == '.') {
            amount_str.push_back('.');
            pos++;
            while(current() >= '0' && current() <= '9') {
                amount_str.push_back(current());
                pos++;
            }
        }
        bool ok;
        double amount = amount_str.toDouble(&ok);
        if (!ok) {
            throw std::logic_error("Amount error");
        }
        skip_ws();
        // multiply by the unit
        switch (current()) {
        case 'D':
        case 'd':
            amount*=24;
            [[fallthrough]];
        case 'h':
        case 'H':
            amount*=60;
            [[fallthrough]];
        case 'm':
        case 'M':
            amount*=60;
            [[fallthrough]];
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
