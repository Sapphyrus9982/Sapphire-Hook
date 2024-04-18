#include <Windows.h>
#include "veh.h"
#include "LeoSpecial.h"
#include <iostream>
#include <winternl.h>

#include <TlHelp32.h>


namespace HookFuncs
{
	typedef HANDLE(WINAPI* _OpenProcess)(DWORD, BOOL, DWORD);
	_OpenProcess oOpenProcess;
	HANDLE WINAPI hkOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
	{
		std::cout << "OpenProcess Hooked!" << std::endl;
		return veh::CallOriginal<HANDLE>(oOpenProcess, dwDesiredAccess, bInheritHandle, dwProcessId);
	}

	typedef struct _CLIENT_ID
	{
		PVOID UniqueProcess;
		PVOID UniqueThread;
	} CLIENT_ID, * PCLIENT_ID;
	typedef NTSTATUS(NTAPI* tNtOpenProcess)(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId);
	tNtOpenProcess oNtOpenProcess = nullptr;
	NTSTATUS NTAPI hkNtOpenProcess(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId)
	{
		std::cout << "NtOpenProcess Hooked!" << std::endl;
		NTSTATUS Result = veh::CallOriginal<NTSTATUS>(oNtOpenProcess, ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
		return Result;
	}

	typedef HANDLE(WINAPI* tCreateSnapshot)(DWORD dwFlags, DWORD th32ProcessID);
	tCreateSnapshot oCreateSnapshot;
	HANDLE WINAPI hkCreateSnapshot(DWORD dwFlags, DWORD th32ProcessID)
	{
		HANDLE hSnapshot = veh::CallOriginal<HANDLE>(oCreateSnapshot, dwFlags, th32ProcessID);
		printf("CreateToolhelp32Snapshot Hooked! %d\n", hSnapshot);
		return hSnapshot;
	}
}

int main()
{
	LoadLibraryA("ntdll.dll");
	HookFuncs::oOpenProcess = (HookFuncs::_OpenProcess)GetProcAddress(GetModuleHandleA("kernel32.dll"), "OpenProcess");
	HookFuncs::oNtOpenProcess = (HookFuncs::tNtOpenProcess)(uintptr_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtReadVirtualMemory");
	HookFuncs::oCreateSnapshot = CreateToolhelp32Snapshot;

	veh::Setup();
	veh::Hook((void*)HookFuncs::oOpenProcess, (void*)HookFuncs::hkOpenProcess);
	//veh::Hook((void*)HookFuncs::oCreateSnapshot, (void*)HookFuncs::hkCreateSnapshot);
	//veh::Hook((void*)HookFuncs::oNtOpenProcess, (void*)HookFuncs::hkNtOpenProcess);



	OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	Sleep(100000000);
}