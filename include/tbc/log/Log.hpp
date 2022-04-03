// =============================================================================
//  Log.hpp
//
//  Written in 2014 by Dairoku Sekiguchi (sekiguchi at acm dot org)
//
//  To the extent possible under law, the author(s) have dedicated all copyright
//  and related and neighboring rights to this software to the public domain worldwide.
//  This software is distributed without any warranty.
//
//  You should have received a copy of the CC0 Public Domain Dedication along with
//  this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
// =============================================================================
/*!
	\file		tbc/log/Log.hpp
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_LOG_HPP
#define TBC_LOG_HPP

// Includes --------------------------------------------------------------------
#include <stdio.h>

// Macros ----------------------------------------------------------------------
#ifndef _DEBUG
#define	TBC_LOG_OUT_DISABLE
#define	TBC_INFO_OUT_DISABLE
#define	TBC_WARNING_OUT_DISABLE
#define	TBC_ERROR_OUT_DISABLE
#define	TBC_TRACE_OUT_DISABLE
#define	TBC_DEBUG_OUT_DISABLE
#endif

#ifndef TBC_LOG_OUT_DISABLE
#define	LOG_OUT(type, level, outstr, log) { if(log!=null)log->write(type, level, outstr);}
#define	LOG_OUT_LIMIT(type, level, outstr, log, limit) { {static int c = 0; if (c < limit) {c++; LOG_OUT(type, level, outstr, log);};}; }
#define	LOG_OUT_ONCE(type, level, outstr, log) { LOG_OUT_LIMIT(type, level, outstr, log, 1); }
#else
#define	LOG_OUT_(type, level, outstr, log)
#define	LOG_OUT_LIMIT(type, level, outstr, log, limit)
#define	LOG_OUT_ONCE_type, level, outstr, log)
#endif

#ifndef TBC_INFO_OUT_DISABLE
#define	INFO_OUT(outstr, log) { LOG_OUT(tbc::Log::INFO_MSG, tbc::Log::NORMAL_LEVEL, outstr, log);}
#else
#define	INFO_OUT(outstr, log)
#endif

#ifndef TBC_WARNING_OUT_DISABLE_
#define	WARNING_OUT(outstr, log) { LOG_OUT(tbc::Log::WARNING_MSG, tbc::Log::NORMAL_LEVEL, outstr, log);}
#else
#define	WARNING_OUT(outstr, log)
#endif

#ifndef TBC_ERROR_OUT_DISABLE
#define	ERROR_OUT_(outstr, log) { LOG_OUT(tbc::Log::ERROR_MSG, tbc::Log::NORMAL_LEVEL, outstr, log);}
#else
#define	ERROR_OUT_(outstr, log)
#endif

#ifndef TBC_TRACE_OUT_DISABLE
#define	TRACE_OUT(outstr, log) { LOG_OUT(tbc::Log::TRACE_MSG, tbc::Log::NORMAL_LEVEL, outstr, log);}
#define	TRACE_OUTONCE_(outstr, log) { LOG_OUT_ONCE(tbc::Log::TRACE_MSG, tbc::Log::NORMAL_LEVEL, outstr, log);}
#else
#define	TRACE_OUT(outstr, log)
#define	TRACE_OUTONCE(outstr, log)
#endif

#ifndef TBC_DEBUG_OUT_DISABLE
#define	DEBUG_OUT(outstr, log) { LOG_OUT(tbc::Log::DEBUG_MSG, tbc::Log::NORMAL_LEVEL, outstr, log);}
#else
#define	DEBUG_OUT(outstr, log)
#endif


// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Log interface class
	// -------------------------------------------------------------------------
	class	Log
	{
	public:
		// Member Functions ----------------------------------------------------
		virtual void			write(unsigned int inType, unsigned char inLenvel, const char *inMessage) = 0;
		virtual void			binayDump(int inDumpType, const char *inDumpName, const unsigned char *inData, int inDataLen) = 0;


		// Constatns -----------------------------------------------------------
		const static unsigned int	INFO_MSG		= 0x00000001;
		const static unsigned int	WARNING_MSG		= 0x00000002;
		const static unsigned int	ERROR_MSG		= 0x00000004;
		const static unsigned int	DUMP_MSG		= 0x00000008;
		const static unsigned int	TRACE_MSG		= 0x40000000;
		const static unsigned int	DEBUG_MSG		= 0x80000000;

		const static unsigned char	GLOBAL_LEVEL	= 0;
		const static unsigned char	NORMAL_LEVEL	= 127;
		const static unsigned char	DETAIL_LEVEL	= 255;

		const static int	DUMP_RAW				= 0;
		const static int	DUMP_HEX				= 127;
		const static int	DUMP_SEPARATE			= 255;

	// -------------------------------------------------------------------------
	// LogBase class
	// -------------------------------------------------------------------------
	class	LogBase : public virtual Log
	{
	public:
		// Destructor ----------------------------------------------------------
		virtual					~LogBase() {}

		// Member Functions ----------------------------------------------------
		unsigned int			getLogOutTypeMask() { return mOutTypeMask; }
		void					setLogOutTypeMask(unsigned int inOutTypeMask) { mOutTypeMask = inOutTypeMask; }
		unsigned char			getLogOutLevel() { return mOutLevel; }
		void					setLogOutLevel(unsigned char inOutLevel) { mOutLevel = inOutLevel; }
		bool					isLogOutMessage(unsigned int inType, unsigned char inLevel)
								{
									if ((inType & mOutTypeMask) == 0)
										return false;

									if (inLevel > mOutLevel)
										return false;

									return true;
								}
	protected:
		// Constructor ---------------------------------------------------------
								LogBase()
								{
								#ifdef _DEBUG
									mOutTypeMask = INFO_MSG + WARNING_MSG + ERROR_MSG + DUMP_MSG + TRACE_MSG + DEBUG_MSG;
									mOutLevel = DETAIL_LEVEL;
								#else
									mOutTypeMask = INFO_MSG + WARNING_MSG + ERROR_MSG;
									mOutLevel = NORMAL_LEVEL;
								#endif
								}
								

		// Member Functions ----------------------------------------------------
		void					makeTypeStr(unsigned int inType, char *inBuf, const size_t inBufSize)
		{
			switch (inType)
			{
				case INFO_MSG:
					snprintf(inBuf, inBufSize, "[ INFO  ] ");
					break;
				case WARNING_MSG:
					snprintf(inBuf, inBufSize, "[WARNING] ");
					break;
				case ERROR_MSG:
					snprintf(inBuf, inBufSize, "[ ERROR ] ");
					break;
				case DUMP_MSG:
					snprintf(inBuf, inBufSize, "[ DUMP  ] ");
					break;
				case TRACE_MSG:
					snprintf(inBuf, inBufSize, "[ TRACE ] ");
					break;
				case DEBUG_MSG:
					snprintf(inBuf, inBufSize, "[ DEBUG ] ");
					break;
				default:
					snprintf(inBuf, inBufSize, "[USR:%03d]", inType);
					break;
			}
		}
		void					makeLevelStr(unsigned char inLevel, char *inBuf, const size_t inBufSize)
		{
			snprintf(inBuf, inBufSize, "[LEVEL:%03d] ", (int )inLevel);
		}
		void					makeTimeStampStr(char *inBuf, const size_t inBufSize)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			SYSTEMTIME	time;

			::GetLocalTime(&time);
			snprintf(inBuf, inBufSize,
					"%4d/%02d/%02d %02d:%02d:%02d ",
					time.wYear, time.wMonth, time.wDay, time.wHour,
					time.wMinute, time.wSecond);

		#elif _PTHREAD	//	pthread specific -----------------------------------
			time_t	t;
			struct tm   *tmPtr;

			time(&t);
			tmPtr = localtime(&t);
			snprintf(inBuf, inBufSize,
				"%4d/%02d/%02d %02d:%02d:%02d ",
				tmPtr->tm_year, tmPtr->tm_mon, tmPtr->tm_mday, tmPtr->tm_hour,
				tmPtr->tm_min, tmPtr->tm_sec);
		#endif	// specific parts end ------------------------------------------
		}
	}
}

#endif // TBC_LOG_HPP
