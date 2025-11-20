#include "MainPanel.h"

QtTestProject::QtTestProject(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();

    // Первый раз вручную получим и закинем в UI данные
    getRunningProcesses(std::ref(processes));
    onTimerUpdateUITick();

    setupConnections();
    startTimers();
}

QtTestProject::~QtTestProject()
{
}

void QtTestProject::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    topLayout = new QHBoxLayout();

    restartWithAdminButton = new QPushButton("RestartWithAdmin", this);
    restartWithAdminButton->setEnabled(!WindowsHelper::isProcessRunWithAdmin(WindowsHelper::getCurrentPID()));

    processTreeWidget = new QTreeWidget(this);
    processTreeWidget->setColumnCount(2);
    processTreeWidget->setHeaderLabels(QStringList() << "PID" << "Name");
    processTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    processTreeWidget->header()->setStretchLastSection(true);

    topLayout->addWidget(restartWithAdminButton);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(processTreeWidget);

    contextMenu = new QMenu(this);
    actionEndTask = contextMenu->addAction("End task");
    actionSendData = contextMenu->addAction("Send data");
    actionGetData = contextMenu->addAction("Get data");

    timerGetProcess = new QTimer(this);
    timerUpdateUI = new QTimer(this);

    setWindowTitle(tr("gui.title"));
    resize(600, 400);
    setMinimumSize(QSize(600, 400));
}

void QtTestProject::setupConnections()
{
    connect(restartWithAdminButton, &QPushButton::clicked,                      this, &QtTestProject::onRestartWithAdmin);
    connect(processTreeWidget,      &QTreeWidget::customContextMenuRequested,   this, &QtTestProject::showContextMenu);
    connect(actionEndTask,          &QAction::triggered,                        this, &QtTestProject::onActionEndTask);
    connect(actionSendData,         &QAction::triggered,                        this, &QtTestProject::onActionSendData);
    connect(actionGetData,          &QAction::triggered,                        this, &QtTestProject::onActionGetData);
    connect(timerGetProcess,        &QTimer::timeout,                           this, &QtTestProject::onTimerGetProcessTick);
    connect(timerUpdateUI,          &QTimer::timeout,                           this, &QtTestProject::onTimerUpdateUITick);
}

void QtTestProject::startTimers()
{
    timerGetProcess->start(1000);
    timerUpdateUI->start(1000);
}

void QtTestProject::onTimerGetProcessTick()
{
    QtConcurrent::run(getRunningProcesses, std::ref(processes));
}

void QtTestProject::onTimerUpdateUITick()
{
    QSet<DWORD> currentPids;
    for (const auto& process : processes)
    {
        currentPids.insert(process.pid);
    }

    for (int i = 0; i < processTreeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = processTreeWidget->topLevelItem(i);
        DWORD itemPid = item->text(0).toULong();

        if (!currentPids.contains(itemPid))
        {
            delete item;
        }
    }

    for (const auto& process : processes)
    {
        QList<QTreeWidgetItem*> items = processTreeWidget->findItems(QString::number(process.pid), Qt::MatchExactly, 0);

        if (items.isEmpty())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(processTreeWidget);
            item->setText(0, QString::number(process.pid));
            item->setText(1, process.name);
        }
    }
}

void QtTestProject::getRunningProcesses(concurrency::concurrent_vector<WindowsHelper::ProcessInfo>& vector)
{
    vector.clear();

    for (const WindowsHelper::ProcessInfo& process : WindowsHelper::getAllRunningProcesses())
    {
        vector.push_back(process);
    }
}

void QtTestProject::showContextMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = processTreeWidget->itemAt(pos);
    if (item)
    {
        contextMenu->exec(processTreeWidget->mapToGlobal(pos));
    }
}

void QtTestProject::onRestartWithAdmin()
{
    WindowsHelper::runAsAdmin(QCoreApplication::applicationFilePath(), QApplication::arguments());
    WindowsHelper::killProcess(WindowsHelper::getCurrentPID());
}

void QtTestProject::onActionEndTask()
{
    QTreeWidgetItem* currentItem = processTreeWidget->currentItem();
    if (currentItem)
    {
        QString pid = currentItem->text(0);
        QString name = currentItem->text(1);

        if (!WindowsHelper::killProcess((DWORD)pid.toULong()))
        {
            QMessageBox::critical(this, tr("gui.messagebox.error"), tr("gui.messagebox.error.endtask").arg(name));
        }
    }
}

void QtTestProject::onActionSendData()
{
    QTreeWidgetItem* currentItem = processTreeWidget->currentItem();
    if (currentItem)
    {
        SimpleCrypt crypto(SECRET_KEY);
        QString pid = currentItem->text(0);
        QString encryptedData = crypto.encryptToString(WindowsHelper::getProcessLoadedDlls((DWORD)pid.toULong()).join(" "));

        nlohmann::json json_body =
        {
            { "cmd", 1 },
            { "rid", pid.toInt() },
            { "data", encryptedData.toUtf8().data() }
        };
    
        httplib::Client client("http://172.245.127.93");
        httplib::Result result = client.Post("/p/applicants.php", json_body.dump(), "application/json");
    
        if (!result || result->status != httplib::StatusCode::OK_200)
        {
            QMessageBox::critical(this, tr("gui.messagebox.error"), tr("gui.messagebox.error.senddata"));
        }
    }
}

void QtTestProject::onActionGetData()
{
    QTreeWidgetItem* currentItem = processTreeWidget->currentItem();
    if (currentItem)
    {
        QString pid = currentItem->text(0);
        nlohmann::json json_body =
        {
            { "cmd", 2 },
            { "rid", pid.toInt() },
        };

        httplib::Client client("http://172.245.127.93");
        httplib::Result result = client.Post("/p/applicants.php", json_body.dump(), "application/json");
        
        if (!result || result->status != httplib::StatusCode::OK_200)
        {
            QMessageBox::critical(this, tr("gui.messagebox.error"), tr("gui.messagebox.error.getdata"));
            return;
        }

        nlohmann::json responceBody = nlohmann::json::parse(result->body);
        SimpleCrypt crypto(SECRET_KEY);
        QString encryptedData = QString::fromStdString(responceBody.at("data").get<std::string>());
        QString decrypteddData = crypto.decryptToString(encryptedData);

        QMessageBox::information(this, tr("gui.messagebox.info"), decrypteddData);
    }
}

void QtTestProject::closeEvent(QCloseEvent* event)
{
    hide();
    event->ignore();
}