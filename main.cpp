//
//  main.cpp
//  Service
//
//  Created by Andrea Giovacchini on 04/01/14.
//  Copyright (c) 2014 Andrea Giovacchini. All rights reserved.
//
#include "BaseTypes.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#if defined _WIN32
	#include <windows.h>
	#include <tchar.h>
	#include <strsafe.h>
	#pragma comment(lib, "advapi32.lib")

	#define SVC_ERROR                        ((DWORD)0xC0020001L)
	#define BUFSIZE 4096
	SERVICE_STATUS          gSvcStatus; 
	SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
	HANDLE                  ghSvcStopEvent = NULL;

	TCHAR szCommand[10];
	TCHAR szSvcName[80];

	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	VOID SvcInstall();

	VOID WINAPI SvcCtrlHandler( DWORD ); 
	VOID WINAPI SvcMain( DWORD, LPTSTR * ); 

	VOID ReportSvcStatus( DWORD, DWORD, DWORD );
	VOID SvcInit( DWORD, LPTSTR * ); 
	VOID SvcReportEvent( LPTSTR );
	
#endif 

using namespace std;

const int max_length = 1024;

std::map <string, string> configurationMap ;

string mainPath = "" ;
string logsPath = "";
string iniFileName = "";
string myName = "";

src::severity_logger_mt< boost::log::trivial::severity_level > my_logger_main;


long setConfigValue(string confMapKey, string treeSearchKey, boost::property_tree::ptree* configTree)
{
    configurationMap[confMapKey] = configTree->get<std::string>(treeSearchKey) ;
    return RC_OK ;
}

void init(string pMainPath, string pIniFileName)
{
	std::cout << "Init - Start " << pMainPath << " - " << pIniFileName << std::endl;
	
    long rc = 0 ;
	boost::property_tree::ptree pt;

	fs::path dir(pMainPath);
	fs::path file(pIniFileName);
	fs::path full_path = dir / file;

	std::cout << "Path: " << full_path.string() << std::endl;
    boost::property_tree::ini_parser::read_ini(full_path.string(), pt);
	
	rc = rc + setConfigValue("ServiceName", "Service.Name", &pt );

	// DWORD size = GetModuleFileNameA(NULL, myName, MAX_PATH);
	myName = configurationMap["ServiceName"];
	//std::cout << "Init - 1\n";
	fileDelete(mainPath + "/LOGS/" + myName + ".log.prev");
	//std::cout << "Init - 2\n";
	fileMove(mainPath + "/LOGS/" + myName + ".log", mainPath + "LOGS/" + myName + ".log.prev");
	//std::cout << "Init - 3\n";

    logging::add_file_log
    (
		keywords::file_name = pMainPath + "/LOGS/" + myName + ".log",
		keywords::auto_flush = true, 
     // This makes the sink to write log records that look like this:
     // YYYY-MM-DD HH:MI:SS: <normal> A normal severity message
     // YYYY-MM-DD HH:MI:SS: <error> An error severity message
     keywords::format =
     (
      expr::stream
      << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
      << ": <" << logging::trivial::severity
      << "> " << expr::smessage
      )
     );
	//std::cout << "Init - 4\n";

#if !defined(_WIN32)

	// Block all signals for background thread.
    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif
	//std::cout << "Init - 5\n";
		

    logging::add_common_attributes();
	src::severity_logger_mt< boost::log::trivial::severity_level > my_logger_ma;

	//std::cout << "Init - 5\n";
	BOOST_LOG_SEV(my_logger_ma, lt::info) << "- init - Config file: " << pMainPath << pIniFileName ;
    BOOST_LOG_SEV(my_logger_ma, lt::info) << "- init - Config load start" ;
    
    try {
		rc = rc + setConfigValue("LineToLaunchExecutable", "LineToLaunch.Executable", &pt );
		rc = rc + setConfigValue("LineToLaunchOptions", "LineToLaunch.Options", &pt);
		std::cout << "Executable: " << configurationMap["LineToLaunchExecutable"] << std::endl;
		std::cout << "Parameters: " << configurationMap["LineToLaunchOptions"] << std::endl;
	}
	catch (std::exception const& e)
    {
        BOOST_LOG_SEV(my_logger_ma, lt::fatal) << "- init - Config error: " << e.what() ;
    }
	std::cout << "Init - End";

}


#if defined _WIN32
int __cdecl _tmain(int argc, TCHAR *argv[]) 
{ 
    // If command-line parameter is "install", install the service. 
    // Otherwise, the service is probably being started by the SCM.
    /*if( argc != 1 )
    {
        printf("ERROR: Incorrect number of arguments\n\n");
        DisplayUsage();
        return 0;
    }*/
    
    /*Visual Studio: Works only if in "Project settings | Configuration properties | General"
     the setting "Character Set" is "Use Multi-Byte Character Set"
     http://stackoverflow.com/questions/6006319/converting-tchar-to-string-in-c
	 */


    if( lstrcmpi( argv[1], TEXT("install")) == 0 )
    {
		mainPath = argv[2];
		iniFileName = argv[3];
		init(mainPath, iniFileName);
		std::cout << "mainPath: " << mainPath << ", iniFileName: " << iniFileName;
		BOOST_LOG_SEV(my_logger_main, lt::info) << "mainPath: " << mainPath << ", iniFileName: " << iniFileName;
        SvcInstall();
        return 0;
    }
	else
	{
		mainPath = argv[1];
		iniFileName = argv[2];
	}

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { const_cast<LPSTR>(myName.c_str()), (LPSERVICE_MAIN_FUNCTION) SvcMain },
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        SvcReportEvent(TEXT("StartServiceCtrlDispatcher")); 
    } 

} 

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID SvcInstall()
{
	//std::cout << "InstSvc1: " << SVCNAME << std::endl;
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];

    if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
    {
        std::cout << "Cannot install service (" << GetLastError() << ")" << std::endl;
        return;
    }

    // Get a handle to the SCM database. 

	//std::cout << "InstSvc2\n" ;
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
		std::cout << "OpenSCManager failed (" << GetLastError() << ")" << std::endl;
        return;
    }

    // Create the service

	//std::cout << "InstSvc3\n" ;
    schService = CreateService( 
        schSCManager,              // SCM database 
		myName.c_str(),                   // name of service 
		myName.c_str(),                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        (string(szPath) + " " + mainPath + " " + iniFileName).c_str(),                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 

	//std::cout << "InstSvc4\n" ;
    if (schService == NULL) 
    {
		std::cout << "CreateService failed (" << GetLastError() << ")" << std::endl;
        CloseServiceHandle(schSCManager);
        return;
    }
    else std::cout << "Service installed successfully" << std::endl; 

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}


//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler( 
		myName.c_str(),
        SvcCtrlHandler);

    if( !gSvcStatusHandle )
    { 
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler")); 
        return; 
    } 

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    SvcInit( dwArgc, lpszArgv );
}

//
// Purpose: 
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

    if ( ghSvcStopEvent == NULL)
    {
        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }

    // Report running status when initialization is complete.
    
	std::cout << mainPath << endl;
	init(mainPath, iniFileName);

	ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

	PROCESS_INFORMATION processInfo;
	
	BOOST_LOG_SEV(my_logger_main, lt::info) << "- MA - Trying to start executable: " << configurationMap["LineToLaunchExecutable"];
	BOOST_LOG_SEV(my_logger_main, lt::info) << "- MA - Launch params: " << configurationMap["LineToLaunchOptions"];
	//if (CreateProcess((char*)configurationMap["LineToLaunchExecutable"].c_str(), (char*)configurationMap["LineToLaunchOptions"].c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	string fname_stdout = mainPath + "/LOGS/" + myName.c_str() + string("_stdout.log");
	string fname_stderr = mainPath + "/LOGS/" + myName.c_str() + string("_stderr.log");

	HANDLE h_stdout = CreateFile(_T(fname_stdout.c_str()),
		FILE_APPEND_DATA,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		&sa,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);


	HANDLE h_stderr = CreateFile(_T(fname_stderr.c_str()),
		FILE_APPEND_DATA,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		&sa,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	STARTUPINFO startupInfo;
	BOOL ret = FALSE;
	DWORD flags = CREATE_NO_WINDOW;

	ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;
	startupInfo.hStdInput = NULL;
	startupInfo.hStdError = h_stderr;
	startupInfo.hStdOutput = h_stdout;

	if (CreateProcess(NULL, (char*)(configurationMap["LineToLaunchExecutable"] + " " + configurationMap["LineToLaunchOptions"]).c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo))
	{
		BOOST_LOG_SEV(my_logger_main, lt::info) << "- MA - Executable started with pid: " << processInfo.dwProcessId;
		// ::WaitForSingleObject(processInfo.hProcess, INFINITE);
	}
	else {
		BOOST_LOG_SEV(my_logger_main, lt::error) << "- MA - Executable not started";
	}
		
	BOOST_LOG_SEV(my_logger_main, lt::info) << "- MA - Service running with pid: " << _getpid();

    while(1)
    {
        // Check whether to stop the service.
		
        WaitForSingleObject(ghSvcStopEvent, INFINITE);

		BOOST_LOG_SEV(my_logger_main, lt::info) << "- MA - Received stop request";

		GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, processInfo.dwProcessId);
		boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
		TerminateProcess(processInfo.hProcess, 0);

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);

		// std::cout << "Process exit " << fileDelete(pidFileName) << std::endl;

        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );

		BOOST_LOG_SEV(my_logger_main, lt::info) << "- MA - Finished processing stop request";
		return;
    }
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   // Handle the requested control code. 

   switch(dwCtrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.

         SetEvent(ghSvcStopEvent);
         ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
         
         return;
 
      case SERVICE_CONTROL_INTERROGATE: 
         break; 
 
      default: 
         break;
   } 
   
}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction) 
{ 
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, myName.c_str());

    if( NULL != hEventSource )
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = myName.c_str();
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    SVC_ERROR,           // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}

#endif
