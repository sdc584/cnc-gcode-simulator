#pragma once

#include <fstream>

// Log level enumeration
enum LogLevel
{
	LOG_DEBUG = 0,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_FATAL
};

// Logger class
class CLogger
{
public:
	static CLogger& GetInstance();
	
	void Initialize(const CString& strLogFile);
	void Cleanup();
	
	void WriteLog(LogLevel level, const CString& strMessage);
	void WriteLog(LogLevel level, LPCTSTR lpszFormat, ...);
	
	void SetLogLevel(LogLevel level) { m_nMinLevel = level; }
	LogLevel GetLogLevel() const { return m_nMinLevel; }
	
	// Convenience methods
	void Debug(const CString& strMessage) { WriteLog(LOG_DEBUG, strMessage); }
	void Info(const CString& strMessage) { WriteLog(LOG_INFO, strMessage); }
	void Warning(const CString& strMessage) { WriteLog(LOG_WARNING, strMessage); }
	void Error(const CString& strMessage) { WriteLog(LOG_ERROR, strMessage); }
	void Fatal(const CString& strMessage) { WriteLog(LOG_FATAL, strMessage); }

private:
	CLogger();
	~CLogger();
	
	CLogger(const CLogger&) = delete;
	CLogger& operator=(const CLogger&) = delete;
	
	CString GetLevelString(LogLevel level) const;
	CString GetCurrentTime() const;
	
	std::ofstream m_logFile;
	LogLevel m_nMinLevel;
	CRITICAL_SECTION m_cs;
	BOOL m_bInitialized;
};

// Convenience macros
#define LOG_DEBUG(msg)   CLogger::GetInstance().Debug(msg)
#define LOG_INFO(msg)    CLogger::GetInstance().Info(msg)
#define LOG_WARNING(msg) CLogger::GetInstance().Warning(msg)
#define LOG_ERROR(msg)   CLogger::GetInstance().Error(msg)
#define LOG_FATAL(msg)   CLogger::GetInstance().Fatal(msg)
