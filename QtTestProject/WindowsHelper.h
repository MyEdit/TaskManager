#pragma once

#include <concurrent_vector.h>

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDebug>

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>

class WindowsHelper
{
public:
	struct ProcessInfo
	{
		DWORD pid {0};
		QString name {"EMPTY"};
	};

	static bool runAsAdmin(const QString& programPath, const QStringList& arguments = {});
	static bool killProcess(const DWORD& pid);
	static bool isProcessRunWithAdmin(const DWORD& pid);
	static DWORD getCurrentPID();
	static QString getProcessPathByPID(const DWORD& pid);
	static QStringList getProcessLoadedDlls(const DWORD& pid);
	static QVector<WindowsHelper::ProcessInfo> getAllRunningProcesses();
};

