//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Sys library.
// 
//*****************************************************************************

#include <process.h>
#include <iphlpapi.h>
#include "VuWin32Sys.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Dev/VuDevConfig.h"


// libs

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "iphlpapi.lib")


// defines

#define MAX_DEBUG_STRING_LENGTH 4096

// static variables

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuSys, VuWin32Sys);

//*****************************************************************************
#ifdef VUDEBUG
void VuDebugMemCpy(void* pDest, size_t destSize, const void* pSrc, size_t srcSize)
{
	long long destStart = (long long)pDest;
	long long destEnd = destStart + destSize;
	long long srcStart = (long long)pSrc;
	long long srcEnd = srcStart + srcSize;

	VUASSERT(srcStart >= destEnd || destStart >= srcEnd, "VuDebugMemCpy: src and dest overlap");

	memcpy_s(pDest, destSize, pSrc, srcSize);
}
#endif

//*****************************************************************************
VuWin32Sys::VuWin32Sys():
	mbErrorRaised(false)
{
	mCriticalSection = VuThread::IF()->createCriticalSection();
}

//*****************************************************************************
VuWin32Sys::~VuWin32Sys()
{
	VuThread::IF()->deleteCriticalSection(mCriticalSection);
}

//*****************************************************************************
bool VuWin32Sys::init(const char *forceLanguage, const char *logFileName)
{
	if ( !VuSys::init(forceLanguage, logFileName) )
		return false;

	LARGE_INTEGER li;

	if ( !QueryPerformanceFrequency(&li) )
		return false;
	mPerfFreq = li.QuadPart;

	if ( !QueryPerformanceCounter(&li) )
		return false;
	mPerfInitial = li.QuadPart;

	mLanguage = forceLanguage;
	if ( mLanguage.empty() )
		mLanguage = "en";

	// user identifier
	{
		DWORD bufferSize = 0;
		if ( GetAdaptersInfo(NULL, &bufferSize) == ERROR_BUFFER_OVERFLOW )
		{
			void *buffer = malloc(bufferSize);
			IP_ADAPTER_INFO *pAdapterInfo = static_cast<IP_ADAPTER_INFO *>(buffer);

			if ( GetAdaptersInfo(pAdapterInfo, &bufferSize) == ERROR_SUCCESS )
			{
				while ( pAdapterInfo )
				{
					if ( pAdapterInfo->AddressLength > 0 && strcmp(pAdapterInfo->GatewayList.IpAddress.String, "0.0.0.0") != 0 )
					{
						for ( UINT i = 0; i < pAdapterInfo->AddressLength; i++ )
						{
							char str[16];
							VU_SPRINTF(str, sizeof(str), "%02X", pAdapterInfo->Address[i]);
							if ( i < pAdapterInfo->AddressLength - 1 )
								VU_STRCAT(str, sizeof(str), ":");
							mUserIdentifier += str;
						}

						break;
					}
					pAdapterInfo = pAdapterInfo->Next;
				}
			}

			free(buffer);
		}
	}

#if !VU_DISABLE_DEBUG_OUTPUT
	// clear log file
	FILE *fp;
	fopen_s(&fp, "VUWIN32LOG.txt", "w");
	fclose(fp);
#endif
	return true;
}

//*****************************************************************************
void VuWin32Sys::release()
{
	if ( hasErrors() )
		exit(-1);
}

//*****************************************************************************
bool VuWin32Sys::error(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str) - 1, fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	// log the error
	printf("Error: %s\n", str);

	// show the error
	showMessageBox("Error", str);

	// raise error condition
	mbErrorRaised = true;

	return false;
}

//*****************************************************************************
bool VuWin32Sys::warning(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str) - 1, fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	// log the warning
	printf("Warning: %s\n", str);

	// show the error
	showMessageBox("Warning", str);

	return false;
}

//*****************************************************************************
bool VuWin32Sys::exitWithError(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str) - 1, fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	error(str);

	release();

	return false;
}

//*****************************************************************************
bool VuWin32Sys::hasErrors()
{
	return mbErrorRaised;
}

//*****************************************************************************
double VuWin32Sys::getTime()
{
	double t = 0.0;

	LARGE_INTEGER li;
	if ( QueryPerformanceCounter(&li) )
	{
		__int64 iPerfCur = li.QuadPart;
		__int64 iPerfDelta = iPerfCur - mPerfInitial;
		t = (double)iPerfDelta/(double)mPerfFreq;
	}

	return t;
}

//*****************************************************************************
VUUINT32 VuWin32Sys::getTimeMS()
{
	return (VUUINT32)(getTime()*1000.0f);
}

//*****************************************************************************
VUUINT64 VuWin32Sys::getPerfCounter()
{
	LARGE_INTEGER li;
	if ( QueryPerformanceCounter(&li) )
		return li.QuadPart;

	return 0;
}

//*****************************************************************************
void VuWin32Sys::printf(const char *fmt, ...)
{
	va_list args;
	char str[MAX_DEBUG_STRING_LENGTH];

	va_start(args, fmt);
	VU_VSNPRINTF(str, sizeof(str), sizeof(str) - 1, fmt, args);
	va_end(args);

	str[sizeof(str)-1] = '\0';

	print(str);
}

//*****************************************************************************
void VuWin32Sys::print(const char *str)
{
	VuThread::IF()->enterCriticalSection(mCriticalSection);

#if !VU_DISABLE_DEBUG_OUTPUT
	OutputDebugString(str);

	FILE *fp;
	fopen_s(&fp, "VUWIN32LOG.txt", "a");
	fputs(str, fp);
	fclose(fp);
#endif

	for ( LogCallbacks::iterator iter = mLogCallbacks.begin(); iter != mLogCallbacks.end(); iter++ )
		(*iter)->append(str);

	VuThread::IF()->leaveCriticalSection(mCriticalSection);
}

//*****************************************************************************
void VuWin32Sys::addLogCallback(LogCallback *pCB)
{
	mLogCallbacks.push_back(pCB);
}

//*****************************************************************************
void VuWin32Sys::removeLogCallback(LogCallback *pCB)
{
	mLogCallbacks.remove(pCB);
}

//*****************************************************************************
const char *VuWin32Sys::getLanguage()
{
	// not implemented yet
	return mLanguage.c_str();
}

//*****************************************************************************
const char *VuWin32Sys::getLocale()
{
	// not implemented yet
	return "UnitedStates";
}

//*****************************************************************************
const char *VuWin32Sys::getRegion()
{
	// not implemented yet
	return "NorthAmericaAll";
}

//*****************************************************************************
bool VuWin32Sys::hasTouch()
{
	if ( VuDevConfig::IF() && VuDevConfig::IF()->hasParam("HasTouch") )
		return VuDevConfig::IF()->getParam("HasTouch").asBool();

	return true;
}

//*****************************************************************************
const char *VuWin32Sys::getUserIdentifier()
{
	return mUserIdentifier.c_str();
}

//*****************************************************************************
bool VuWin32Sys::createProcess(const char *strApplicationName, const char *strCommandLine, const char *strCurDir, bool bWait, bool bLog)
{
	HANDLE hStdOut_Read = NULL;
	HANDLE hStdOut_Write = NULL;

	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	if ( bWait )
	{
		// create output pipe
		{
			SECURITY_ATTRIBUTES saAttr;
			saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;

			if ( !CreatePipe(&hStdOut_Read, &hStdOut_Write, &saAttr, 0) )
			{
				VUPRINTF("ERROR: Unable to create pipe.\n");
			}

			if ( !SetHandleInformation(hStdOut_Read, HANDLE_FLAG_INHERIT, 0) )
			{
				VUPRINTF("ERROR: Unable to set output piple handle info.\n");
			}
		}

		si.hStdOutput = hStdOut_Write;
		si.hStdError = hStdOut_Write;
		si.dwFlags = STARTF_USESTDHANDLES;
	}

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	char strCmdLine[1024];
	VU_SPRINTF(strCmdLine, sizeof(strCmdLine), "\"%s\" %s", strApplicationName, strCommandLine);
	if ( bLog )
	{
		VUPRINTF("CreateProcess: %s\n", strCmdLine);
	}
	if ( CreateProcess(strApplicationName, strCmdLine, NULL, NULL, bWait, CREATE_NO_WINDOW, NULL, strCurDir, &si, &pi) == 0 )
	{
		VUPRINTF("ERROR: Unable to execute '%s'\n", strCmdLine);
		return false;
	}

	DWORD exitCode = 0;
	if ( bWait )
	{
		while ( GetExitCodeProcess(pi.hProcess, &exitCode) )
		{
			if ( exitCode != STILL_ACTIVE )
				break;
			Sleep(10);
			printOutput(hStdOut_Read);
		}
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if ( bWait )
	{
		CloseHandle(hStdOut_Write);
		printOutput(hStdOut_Read);
		CloseHandle(hStdOut_Read);
	}

	return exitCode == 0;
}

//*****************************************************************************
void VuWin32Sys::printOutput(HANDLE hStdOut_Read)
{
	if ( GetFileSize(hStdOut_Read, NULL) )
	{
		char buffer[1024];
		DWORD bytesRead;
		if ( ReadFile( hStdOut_Read, buffer, sizeof(buffer) - 1, &bytesRead, NULL) )
		{
			buffer[bytesRead] = '\0';
			VUPRINTF(buffer);
		}
	}
}

//*****************************************************************************
void VuWin32Sys::showMessageBox(const char *strTitle, const char *strText)
{
	ShowCursor(TRUE);

	int choice = MessageBox(NULL, strText, strTitle, MB_ABORTRETRYIGNORE|MB_TASKMODAL);

	if ( choice == IDABORT )
		exit(-1);
	else if ( choice == IDRETRY )
		__debugbreak();

	ShowCursor(FALSE);
}
