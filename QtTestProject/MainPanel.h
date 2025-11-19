#pragma once

#define WIN32_LEAN_AND_MEAN

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QMessageBox>
#include <QPointer>
#include "QApplication.h"
#include <QDebug>
#include <QStringList>
#include <QtConcurrent/QtConcurrent>

#include "Httplib-0.27.0/httplib.h"
#include "Json-Nlohman-3.12.0/json.hpp"
#include "SimpleCrypt.h"
#include "WindowsHelper.h"

#define SECRET_KEY Q_UINT64_C(0x0c2ad4a4acb9f023)

class QtTestProject : public QMainWindow
{
    Q_OBJECT

public:
    QtTestProject(QWidget* parent = nullptr);
    ~QtTestProject();

private slots:
    void onRestartWithAdmin();
    void onActionEndTask();
    void onActionSendData();
    void onActionGetData();
    void onTimerGetProcessTick();
    void onTimerUpdateUITick();
    void showContextMenu(const QPoint& pos);

private:
    static void getRunningProcesses(concurrency::concurrent_vector<WindowsHelper::ProcessInfo>& vector);
    void setupUI();
    void setupConnections();
    void startTimers();

    QPointer<QWidget> centralWidget;
    QPointer<QVBoxLayout> mainLayout;
    QPointer<QHBoxLayout> topLayout;
    QPointer<QPushButton> restartWithAdminButton;
    QPointer<QTreeWidget> processTreeWidget;

    QPointer<QMenu> contextMenu;
    QPointer<QAction> actionEndTask;
    QPointer<QAction> actionSendData;
    QPointer<QAction> actionGetData;

    QPointer<QTimer> timerGetProcess;
    QPointer<QTimer> timerUpdateUI;

    concurrency::concurrent_vector<WindowsHelper::ProcessInfo> processes;
};