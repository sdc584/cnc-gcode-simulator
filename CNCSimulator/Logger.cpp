#include "pch.h"
#include "Logger.h"
#include <iostream>
#include <iomanip>

CLogger::CLogger()
	: m_nMinLevel(LOG_INFO)
	, m_bInitialized(FALSE)
{
	InitializeCriticalSection(&m_cs);
}

CLogger::~CLogger()
{
	Cleanup();
	DeleteCriticalSection(&m_cs);
}

CLogger& CLogger::GetInstance()
{
	static CLogger instance;
	return instance;
}

void CLogger::Initialize(const CString& strLogFile)
{
	EnterCriticalSection(&m_cs);
	
	if (m_bInitialized)
	{
		m_logFile.close();
	}
	
	// 转换CString到std::string
	CT2A converter(strLogFile);
	std::string logFileName(converter);
	
	m_logFile.open(logFileName, std::ios::out | std::ios::app);
	m_bInitialized = m_logFile.is_open();
	
	if (m_bInitialized)
	{
		WriteLog(LOG_INFO, _T("Logger system initialized"));
	}
	
	LeaveCriticalSection(&m_cs);
}

void CLogger::Cleanup()
{
	EnterCriticalSection(&m_cs);
	
	if (m_bInitialized)
	{
		WriteLog(LOG_INFO, _T("Logger system shutdown"));
		m_logFile.close();
		m_bInitialized = FALSE;
	}
	
	LeaveCriticalSection(&m_cs);
}

void CLogger::WriteLog(LogLevel level, const CString& strMessage)
{
	if (level < m_nMinLevel || !m_bInitialized)
		return;
	
	EnterCriticalSection(&m_cs);
	
	try
	{
		CString strLogEntry;
		strLogEntry.Format(_T("[%s] [%s] %s"),
			GetCurrentTime(),
			GetLevelString(level),
			strMessage);
		
		// 转换为std::string并写入文件
		CT2A converter(strLogEntry);
		std::string logLine(converter);
		
		m_logFile << logLine << std::endl;
		m_logFile.flush();
		
		// Also output to debug window
		OutputDebugString(strLogEntry + _T("\n"));
	}
	catch (...)
	{
		// Ignore log write errors
	}
	
	LeaveCriticalSection(&m_cs);
}

void CLogger::WriteLog(LogLevel level, LPCTSTR lpszFormat, ...)
{
	if (level < m_nMinLevel)
		return;
	
	va_list args;
	va_start(args, lpszFormat);
	
	CString strMessage;
	strMessage.FormatV(lpszFormat, args);
	
	va_end(args);
	
	WriteLog(level, strMessage);
}

CString CLogger::GetLevelString(LogLevel level) const
{
	switch (level)
	{
	case LOG_DEBUG:   return _T("DEBUG");
	case LOG_INFO:    return _T("INFO ");
	case LOG_WARNING: return _T("WARN ");
	case LOG_ERROR:   return _T("ERROR");
	case LOG_FATAL:   return _T("FATAL");
	default:          return _T("UNKNW");
	}
}

CString CLogger::GetCurrentTime() const
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	CString strTime;
	strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d.%03d"),
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	
	return strTime;
}
