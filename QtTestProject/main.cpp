#include "MainPanel.h"
#include <QTranslator>
#include <SingleApplication-3.5.4\SingleApplication.h>

int main(int argc, char *argv[])
{
    SingleApplication application(argc, argv);
    //application.setStyle("Fusion");

    QTranslator translator;
    if (translator.load(QCoreApplication::applicationDirPath() + "/Translation_ru.qm"))
        application.installTranslator(&translator);

    QtTestProject window;
    window.show();
    return application.exec();
}
