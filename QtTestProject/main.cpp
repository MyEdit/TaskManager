#include "MainPanel.h"
#include <QTranslator>
#include <QSystemTrayIcon>
#include <SingleApplication-3.5.4\SingleApplication.h>

int main(int argc, char *argv[])
{
    SingleApplication application(argc, argv);
    application.setWindowIcon(QIcon(":/QtTestProject/Icons/logo.ico"));

    QTranslator translator;
    if (translator.load(QCoreApplication::applicationDirPath() + "/Translation_ru.qm"))
        application.installTranslator(&translator);

    QtTestProject window;

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMenu* menu = new QMenu();
        QAction* showAction = menu->addAction("Show");
        QAction* exitAction = menu->addAction("Exit");

        QSystemTrayIcon* trayIcon = new QSystemTrayIcon(&application);
        trayIcon->setIcon(QIcon(":/QtTestProject/Icons/logo.ico"));
        trayIcon->setContextMenu(menu);
        trayIcon->show();

        QObject::connect(exitAction, &QAction::triggered, &application, &QApplication::quit);
        QObject::connect(trayIcon, &QSystemTrayIcon::activated, [&window] (QSystemTrayIcon::ActivationReason reason)
            {
                if (reason == QSystemTrayIcon::DoubleClick)
                {
                    window.showNormal();
                    window.activateWindow();
                }
            });
        QObject::connect(showAction, &QAction::triggered, [&window]()
            {
                window.showNormal();
                window.activateWindow();
            });
    }

    window.show();
    return application.exec();
}
