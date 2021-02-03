#include <windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <ShlObj.h>

void SuspendProcess(DWORD m_dwPID)
{
	HANDLE m_hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 m_thEntry;
	m_thEntry.dwSize = sizeof(THREADENTRY32);
	Thread32First(m_hThreadSnapshot, &m_thEntry);
	do {
		if (m_thEntry.th32OwnerProcessID == m_dwPID) {
			HANDLE m_hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_thEntry.th32ThreadID);
			SuspendThread(m_hThread);
			CloseHandle(m_hThread);
		}
	} while (Thread32Next(m_hThreadSnapshot, &m_thEntry));
	CloseHandle(m_hThreadSnapshot);
}

void ResumeProcess(DWORD m_dwPID)
{
	HANDLE m_hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 m_thEntry;
	m_thEntry.dwSize = sizeof(THREADENTRY32);
	Thread32First(m_hThreadSnapshot, &m_thEntry);
	do {
		if (m_thEntry.th32OwnerProcessID == m_dwPID) {
			HANDLE m_hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_thEntry.th32ThreadID);
			ResumeThread(m_hThread);
			CloseHandle(m_hThread);
		}
	} while (Thread32Next(m_hThreadSnapshot, &m_thEntry));
	CloseHandle(m_hThreadSnapshot);
}

DWORD GetProcessFromEXEName(LPCSTR m_szEXEName) {
	HANDLE m_hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);;
	PROCESSENTRY32 m_preProcEntry;
	m_preProcEntry.dwSize = sizeof(m_preProcEntry);
	do {
		if (strstr(m_preProcEntry.szExeFile, m_szEXEName)) {
			DWORD m_dwPID = m_preProcEntry.th32ProcessID;
			CloseHandle(m_hProcess);
			return m_dwPID;
		}
	}
	while (Process32Next(m_hProcess, &m_preProcEntry));
}

BOOL WINAPI WinMain(HINSTANCE m_hInstance, HINSTANCE m_hPrevInstance, LPSTR m_lpCmdLine, int m_nShowCmd) {
	DWORD m_dwProcess = 0;
	HANDLE m_hProcess;
	char m_szDLLFullPath[MAX_PATH];
	char m_szProgramFiles[MAX_PATH];
	char m_szSteam[MAX_PATH];
	GetFullPathNameA("trustedbypass.dll", MAX_PATH, m_szDLLFullPath, NULL);
	SHGetSpecialFolderPath(0, m_szProgramFiles, CSIDL_PROGRAM_FILESX86, FALSE);
	sprintf_s(m_szSteam, sizeof(m_szSteam), "%s\\Steam\\steam.exe -applaunch 730", m_szProgramFiles, "Steam");
	WinExec(m_szSteam, 0);
	while (m_dwProcess == 0) {
		m_dwProcess = GetProcessFromEXEName("csgo.exe");
	}
	SuspendProcess(m_dwProcess); 
	m_hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, m_dwProcess);
	LPVOID m_pMemory = VirtualAllocEx(m_hProcess, NULL, sizeof(m_szDLLFullPath), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(m_hProcess, m_pMemory, m_szDLLFullPath, sizeof(m_szDLLFullPath), NULL);
	CreateRemoteThread(m_hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, m_pMemory, 0, NULL);
	MessageBoxA(0, "Successfully injected!", "TrustedBypass Injector", MB_OK);
	ResumeProcess(m_dwProcess);
	CloseHandle(m_hProcess);
	Beep(626, 50);
	return 0;
}