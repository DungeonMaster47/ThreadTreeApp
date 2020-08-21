#include "stdafx.h"
#include "ThreadTree.h"

BOOL SetPrivilege(
	HANDLE hToken,          // access token handle
	LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
	BOOL bEnablePrivilege   // to enable or disable privilege
)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system
		lpszPrivilege,   // privilege to lookup 
		&luid))        // receives LUID of privilege
	{
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		return FALSE;
	}

	return TRUE;
}

ThreadTree::ThreadTree(DWORD processID)
{
	createTree(processID);
}

bool ThreadTree::getDebugPrivilege()
{
	HANDLE hToken;
	if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken))
	{
		if (GetLastError() == ERROR_NO_TOKEN)
		{
			if (!ImpersonateSelf(SecurityImpersonation))
				return false;

			if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken))
				return false;
		}
		else
			return false;
	}

	if (!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE))
	{
		CloseHandle(hToken);
		return false;
	}

	CloseHandle(hToken);
	return true;
}

bool ThreadTree::getProcessThreads(DWORD processID)
{
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return false;

	te32.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(hThreadSnap, &te32))
	{
		CloseHandle(hThreadSnap);
		return false;
	}

	do
	{
		if (te32.th32OwnerProcessID == processID)
		{
			ThreadNode tn = { te32.th32ThreadID };
			root.push_back(tn);
		}
	} while (Thread32Next(hThreadSnap, &te32));

	CloseHandle(hThreadSnap);
	return true;
}

ThreadNode::State ThreadTree::getThreadState(HANDLE hThread)
{
	DWORD exitCode;
	GetExitCodeThread(hThread, &exitCode);
	if (exitCode == STILL_ACTIVE)
		return ThreadNode::State::Running;
	else
		return ThreadNode::State::Finished;
}

std::vector<StackFrame> ThreadTree::getStackTrace(HANDLE hProcess, HANDLE hThread)
{
	std::vector<StackFrame> result;

	SymInitialize(hProcess, NULL, TRUE);

	CONTEXT context = {};
	context.ContextFlags = CONTEXT_FULL;
	GetThreadContext(hThread, &context);

#if _WIN64
	STACKFRAME frame = {};
	frame.AddrPC.Offset = context.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;
#else
	STACKFRAME frame = {};
	frame.AddrPC.Offset = context.Eip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Ebp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Esp;
	frame.AddrStack.Mode = AddrModeFlat;
#endif

#if _WIN64
	DWORD machine = IMAGE_FILE_MACHINE_AMD64;
#else
	DWORD machine = IMAGE_FILE_MACHINE_I386;
#endif

	while (StackWalk(machine, hProcess, hThread, &frame, &context, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL))
	{
		result.push_back(StackFrame());

#if _WIN64
		DWORD64 moduleBase = 0;
#else
		DWORD moduleBase = 0;
#endif

		moduleBase = SymGetModuleBase(hProcess, frame.AddrPC.Offset);

		wchar_t moduleBuff[MAX_PATH];
		if (moduleBase && GetModuleFileName((HINSTANCE)moduleBase, moduleBuff, MAX_PATH))
		{
			result.back().moduleName = moduleBuff;
		}
		else
		{
			result.back().moduleName = L"Unknown";
		}

		char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
		PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
		symbol->SizeOfStruct = (sizeof IMAGEHLP_SYMBOL) + 255;
		symbol->MaxNameLength = 254;
		if (SymGetSymFromAddr(hProcess, frame.AddrPC.Offset, NULL, symbol))
		{
			wchar_t wideBuf[255] = { 0 };
			MultiByteToWideChar(1251, NULL, symbol->Name, strlen(symbol->Name), wideBuf, 255);
			result.back().functionName = wideBuf;
		}
		else
		{
			result.back().functionName = L"Unknown";
		}
	}

	SymCleanup(hProcess);

	return result;
}

void ThreadTree::createTree(DWORD processID)
{
	root.clear();

	if (!getDebugPrivilege())
		throw std::exception("Cannot get debug privilege(try run with admin rights)");

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (!hProcess)
		throw std::exception("Cannot open process");

	if (!getProcessThreads(processID))
		throw std::exception("Cannot get list of process threads");


	for (auto& thread : root)
	{
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, thread.id);
		if (!hThread)
			throw std::exception("Cannot open thread");
		thread.state = getThreadState(hThread);
		thread.stacktrace = getStackTrace(hProcess, hThread);

		CloseHandle(hThread);
	}

	CloseHandle(hProcess);
}

const std::vector<ThreadNode>& ThreadTree::getRoot()
{
	return root;
}

std::wstring ThreadTree::toXML()
{
	std::wstring result = L"<?xml version=\"1.1\" ?>\n";
	result += L"<threadtree>\n";
	for (auto& thread : root)
	{
		result += L"<thread>\n";

		result += L"<id>\n";
		result += std::to_wstring(thread.id);
		result += L"\n</id>\n";

		result += L"<state>\n";
		result += (thread.state == ThreadNode::State::Running) ? L"Running" : L"Finished";
		result += L"\n</state>\n";

		result += L"<stacktrace>\n";

		for (auto& stackframe : thread.stacktrace)
		{
			result += L"<stackframe\n>";

			result += L"<modulename>\n";
			result += stackframe.moduleName;
			result += L"\n</modulename>\n";

			result += L"<functionname>\n";
			result += stackframe.functionName;
			result += L"\n</functionname>\n";

			result += L"</stackframe>\n";
		}

		result += L"</stacktrace>\n";

		result += L"</thread>\n";
	}
	result += L"</threadtree>\n";
	return result;
}