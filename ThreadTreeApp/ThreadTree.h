#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <DbgHelp.h>
#include <TlHelp32.h>

struct StackFrame
{
	std::wstring moduleName;
	std::wstring functionName;
};

struct ThreadNode
{
	enum State { Running, Finished };

	DWORD id;
	State state;
	std::vector<StackFrame> stacktrace;
};

class ThreadTree
{
	std::vector<ThreadNode> root;

	bool getDebugPrivilege();
	bool getProcessThreads(DWORD processID);
	ThreadNode::State getThreadState(HANDLE hThread);
	std::vector<StackFrame> getStackTrace(HANDLE hProcess, HANDLE hThread);
public:
	ThreadTree() {}
	ThreadTree(DWORD processID);
	void createTree(DWORD processID);
	const std::vector<ThreadNode>& getRoot();
	std::wstring toXML();
};

