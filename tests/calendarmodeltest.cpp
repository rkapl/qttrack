#include "calendarmodeltest.h"
#include "calendarmodel.h"
#include <QTest>
#include <QDebug>
#include "../timespan.h"

void CalendarModelTest::formatCycle(){
    QFile forDeletion("example.ics.test");
    forDeletion.remove();
    bool copyOk = QFile::copy("example.ics", "example.ics.test");

    QStringList fileBefore = fetchLines("example.ics");
    QVERIFY(copyOk);

    CalendarModel m;
    connect(&m, &CalendarModel::error, this, &CalendarModelTest::error);
    bool loadOk = m.load("example.ics.test");
    QVERIFY(loadOk);
    m.save();

    QStringList fileAfter = fetchLines("example.ics.test");
    QVERIFY(fileAfter.length() == fileBefore.length());
    for(int i = 0; i<fileBefore.length(); i++){
        if(fileBefore[i].startsWith("PRODID")){
            QCOMPARE(fileAfter[i], QString("PRODID:") + CalendarModel::ICAL_PRODID_QTTRACK);
        }else{
            QCOMPARE(fileBefore[i], fileAfter[i]);
        }
    }
}

void CalendarModelTest::error(const QString& source, const QString& problem)
{
	qCritical() << "Ical problem: " << problem;
	abort();
}

qint64 CalendarModelTest::parseOrDie(const QString &data){
    bool ok;
    TimeSpan span(TimeSpan::parse(data, &ok));
    if(!QTest::qVerify(ok, "ok", QString("Parsing '%1'").arg(data).toLocal8Bit().data(), __FILE__, __LINE__))
        return 0;
    return span.msec/1000;
}

void CalendarModelTest::parseTimeSpan(){
    QCOMPARE(parseOrDie("1 sec"), 1);
    QCOMPARE(parseOrDie("-1 sec"), -1);
    qint64 expected = 23*24*3600 + 18*60 + 3*3600 + 2 + 3600;
    QCOMPARE(parseOrDie("23 day 18 m 3 h 2s 1h"), expected);
    QCOMPARE(parseOrDie("   -  23days 18 M 3 hours 2s 1h  "), -expected);

}

QStringList CalendarModelTest::fetchLines(const QString &file){
    QStringList lines;
    QFile f(file);
    if(!f.open(QFile::ReadOnly))
        return lines;
    QTextStream s(&f);
    while(!s.atEnd()){
        lines.append(s.readLine());
    }
    return lines;
}
