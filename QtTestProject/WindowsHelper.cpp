#include "WindowsHelper.h"

bool WindowsHelper::runAsAdmin(const QString& programPath, const QStringList& arguments /*= {}*/)
{
    QString args = arguments.join(" ");

    std::wstring wApp = programPath.toStdWString();
    std::wstring wArgs = args.toStdWString();

    HINSTANCE result = ShellExecuteW
    (
        nullptr,
        L"runas",
        wApp.c_str(),
        wArgs.c_str(),
        nullptr,
        SW_SHOWNORMAL
    );

    if ((int)result <= 32)
    {
        return false;
    }

    return true;
}

bool WindowsHelper::killProcess(const DWORD& pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);

    if (hProcess == NULL)
    {
        return false;
    }

    BOOL result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result;
}

QString WindowsHelper::getProcessPathByPID(const DWORD& pid)
{
    HANDLE hProcess = OpenProcess
    (
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        pid
    );

    if (!hProcess)
        return QString();

    wchar_t buffer[MAX_PATH];
    DWORD size = MAX_PATH;
    QString result;

    if (QueryFullProcessImageNameW(hProcess, 0, buffer, &size))
        result = QString::fromWCharArray(buffer);

    CloseHandle(hProcess);
    return result;
}

QVector<WindowsHelper::ProcessInfo> WindowsHelper::getAllRunningProcesses()
{
    QVector<WindowsHelper::ProcessInfo> processList;
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return processList;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        return processList;
    }

    do
    {
        WindowsHelper::ProcessInfo process{ pe32.th32ProcessID, QString::fromWCharArray(pe32.szExeFile) };
        processList.append(process);
    }
    while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return processList;
}

DWORD WindowsHelper::getCurrentPID()
{
    return GetCurrentProcessId();
}

QStringList WindowsHelper::getProcessLoadedDlls(const DWORD& pid)
{
    QStringList dlls;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

    if (snapshot == NULL)
        return dlls;

    MODULEENTRY32W moduleEntry = MODULEENTRY32W();
    moduleEntry.dwSize = sizeof(MODULEENTRY32W);

    if (Module32FirstW(snapshot, &moduleEntry))
    {
        do
        {
            QString dllName = QString::fromStdWString(moduleEntry.szModule);
            dlls.push_back(dllName);
        }
        while (Module32NextW(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return dlls;
}

bool WindowsHelper::isProcessRunWithAdmin(const DWORD& pid)
{
    HANDLE hToken = NULL;
    HANDLE hprocess = OpenProcess
    (
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        pid
    );

    if (hprocess == NULL)
        return false;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return false;

    TOKEN_ELEVATION elevation;
    DWORD dwSize;
    bool isAdmin = false;

    if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
    {
        isAdmin = elevation.TokenIsElevated;
    }

    CloseHandle(hToken);
    CloseHandle(hprocess);
    return isAdmin;
}