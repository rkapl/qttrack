#include "tasklistwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QtTest/QTest>
#include <stdio.h>
#include "tests/calendarmodeltest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("rkapl");
    QApplication::setApplicationName("qtimetracker");
    QApplication::setOrganizationDomain("rkapl.cz");
    QApplication::setApplicationVersion("1.0");

    QCommandLineParser cmdline;
    cmdline.addHelpOption();
    cmdline.addVersionOption();
    cmdline.addPositionalArgument("calendar-file", "Calendar file to open (in KTimeTracker format)", "[calendar-file]");
#ifdef TESTS
    QCommandLineOption optTests("tests", "Run built-in tests");
    cmdline.addOption(optTests);
#endif
    cmdline.process(a);

#ifdef TESTS
    if(cmdline.isSet(optTests)){
        QStringList args = a.arguments();
        args.removeAll("--tests");
        return QTest::qExec(new CalendarModelTest(), args);
    }
#endif

    TaskListWindow w;
    w.show();

    if(cmdline.positionalArguments().length() == 1){
        w.openFile(cmdline.positionalArguments()[0]);
    }else if(cmdline.positionalArguments().length() > 1){
        fprintf(stderr, "Error: More than two arguments given.");
        cmdline.showHelp(1);
    }else{
        w.openDefault();
    }

    return a.exec();
}
