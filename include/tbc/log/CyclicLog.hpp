// =============================================================================
//  CyclicLog.hpp
//
//  MIT License
//
//  Copyright (c) 1998-2018 Dairoku Sekiguchi
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
// =============================================================================
/*!
	\file		tbc/CyclicLog.h
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_CONSOLE_LOG_HPP
#define TBC_CONSOLE_LOG_HPP

// Includes --------------------------------------------------------------------
#include <stdio.h>
#include "tbc/log/Log.hpp"
#include "tbc/Mutex.hpp"

// Macros ----------------------------------------------------------------------

#define	TBC_LOG_FILE_EOL			"\r\n"
#define	TBC_LOG_TERMINATE_STR		"@@@@@@@@@@@@" TBC_LOG_FILE_EOL
#define	TBC_LOG_FILE_ERR_OUT
#ifdef	TBC_LOG_FILE_ERR_OUT
 #define	TBC_LOG_FILE_WARNING_OUT
#endif


// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Exception class
	// -------------------------------------------------------------------------
	class	CyclicLog : public virtual LogBase
	{
	public:
		// Constructors and Destructor -----------------------------------------
								CyclicLog(const char *inFileName, const char *inMessage)
								{
									if (logFileOpen(inFileName) == false)
										return;
									mCurrentPos = 0;
									if (getCurrentPos() == false)
									{
										logFileClose();
										return;
									}
									writeHeaderMessage(inMessage);
								}
								~ConsoleLog()
								{
									logFileClose();
								}

		// Member Functions ----------------------------------------------------
		virtual void			write(unsigned int inType, unsigned char inLenvel, const char *inMessage)
		{
			if (!isLogOutMessage(inType, inLevel))
				return;

			const size_t	bufSize = 80;
			char	buf[bufSize];

			makeTimeStampStr(buf, bufSize);
			size_t	len = strlen(buf);
			makeTypeStr(inType, &(buf[len]), bufSize - len);
			writeLog(buf, inMessage, true, true);
		}
		virtual void			binayDump(int inDumpType, const char *inDumpName, const unsigned char *inData, int inDataLen)
		{
		}

	private:
		// Constatns -----------------------------------------------------------
		const static int	LOG_FILE_MIN_SIZE					= 512;
		const static int	LOG_FILE_HEADER_SIZE				= 10;
		const static int	LOG_FILE_BEGIN_POS					= 300;
		const static int	LOG_FILE_EOL_LEN					= 2;
		const static int	LOG_TERMINATE_STR_LEN				= 14;		// =strlen("@@@@@@@@@@@@" TBC_LOG_FILE_EOL)
		const static int	LOG_FILE_BUF_SIZE					= 512;

		// Member Functions ----------------------------------------------------
		bool					writeLog(const char *inMsgHeaderBuf, const char *inMsgBodyBuf, int inWriteCR, int inFlush)
		{
			bool	result;

			if (isLogFileOpened() == false)
				return false;
			try
			{
				mMutex.lock();
			}

			catch (Exception &ex)
			{
			#ifdef TBC_LOG_FILE_WARNING_OUT
				ex.dump();
				cerr << "Can't lock mutex" << endl;
			#endif
			}

			result = writeData(inMsgHeaderBuf, (unsigned int )strlen(inMsgHeaderBuf));
			result = writeData(inMsgBodyBuf, (unsigned int )strlen(inMsgBodyBuf));
			if (inWriteCR != false)
				result = writeData(TBC_LOG_FILE_EOL, LOG_FILE_EOL_LEN);

			try
			{
				mMutex.unlock();
			}

			catch (Exception &ex)
			{
			#ifdef TBC_LOG_FILE_WARNING_OUT
				ex.dump();
				cerr << "Can't unlock mutex" << endl;
			#endif
			}

			if (inFlush != false)
				logFileFlush();

			return result;
		}
		bool					getCurrentPos()
		{
			int		i, numStart;
			char	buf[LOG_FILE_HEADER_SIZE + 1];

			logFileSeek(0);
			if (logFileRead(buf, LOG_FILE_HEADER_SIZE) == false)
			{
				mCurrentPos = 0;
				return false;
			}

			for (i = 0, numStart = false; i < LOG_FILE_HEADER_SIZE; i++)
			{
				if (numStart == false && buf[i] != ' ')
					numStart = true;

				if (numStart != false)
				{
					if (buf[i] < '0' || buf[i] > '9')
					{
						#ifdef TBC_LOG_FILE_WARNING_OUT
							cerr << "warning: log file header incorrect " <<  '\"' << mFileName << '\"'  << endl;
						#endif
						return searchCurrentPos();
					}
				}
			}

			buf[LOG_FILE_HEADER_SIZE] = 0;
			mCurrentPos = atol(buf);

			if (mCurrentPos < 0 || mCurrentPos >= mFileSize - LOG_FILE_BEGIN_POS)
			{
					#ifdef TBC_LOG_FILE_WARNING_OUT
						cerr << "warning: log file header value incorrect " <<  '\"' << mFileName << '\"'  << endl;
					#endif
				return searchCurrentPos();
			}

			return true;
		}
		bool					searchCurrentPos()
		{
			int		len;
			unsigned long	i, readLen;
			char	buf[LOG_FILE_BUF_SIZE], prevChar = ' ';

			mCurrentPos = 0;
			len = mFileSize - LOG_FILE_BEGIN_POS;

			logFileSeek(LOG_FILE_BEGIN_POS);
			while (len != 0)
			{
				if (len < LOG_FILE_BUF_SIZE)
					readLen = len;
				else
					readLen = LOG_FILE_BUF_SIZE;

				if (logFileRead(buf, readLen) == false)
					return false;

				for (i = 0; i < readLen; i++)
				{
					if (prevChar == '@' && buf[i] == '@')
					{
						mCurrentPos = mFileSize - LOG_FILE_BEGIN_POS  - len + i - 1;
						return true;
					}

					prevChar = buf[i];
				}

				len -= readLen;
			}

			return true;
		}
		int						writeHeaderMessage(const char *inMessage)
		{
			int		i, len, t;
			const size_t	bufSize = LOG_FILE_BEGIN_POS + 1;
			char	buf[bufSize];
			const size_t	timeStampStrSize = 40;
			char	timeStampStr[timeStampStrSize];
			static const char	*logFileHeaderMsg[] = {
				"  <- Do not edit this number!!!" TBC_LOG_FILE_EOL "Message:" TBC_LOG_FILE_EOL " ",
				TBC_LOG_FILE_EOL "Last opened: "};


			snprintf(buf, bufSize, "%010u%s", mCurrentPos, logFileHeaderMsg[0]);
			len = (int )strlen(buf);

			t = (int )strlen(inMessage);
			if (LOG_FILE_BEGIN_POS - LOG_FILE_EOL_LEN - len > t)
			{
				strcpy(&buf[len], inMessage);
				len += t;
			}

			logFileMakeTimeStampStr(timeStampStr, timeStampStrSize);

			i = (int )strlen(logFileHeaderMsg[1]);
			t = i + (int )strlen(timeStampStr);
			if (LOG_FILE_BEGIN_POS - LOG_FILE_EOL_LEN - len > t)
			{
				strcpy(&buf[len], logFileHeaderMsg[1]);
				strcpy(&buf[len + i], timeStampStr);
				len += t;
			}

			for (i = len; i < LOG_FILE_BEGIN_POS - LOG_FILE_EOL_LEN; i++)
				buf[i] = '-';

			strcpy(&buf[LOG_FILE_BEGIN_POS - LOG_FILE_EOL_LEN], TBC_LOG_FILE_EOL);

			logFileSeek(0);

			if (logFileWrite(buf, LOG_FILE_BEGIN_POS) == false)
				return false;

			snprintf(buf, bufSize, TBC_LOG_FILE_EOL "############ <- %s", timeStampStr);	
			t = writeData(buf, (unsigned int)strlen(buf));
			logFileFlush();

			return t;
		}
		bool					writeCurrentPos()
		{
			const size_t	bufSize = 40;
			char	buf[bufSize];

			snprintf(buf, bufSize, "%010u", mCurrentPos);
			logFileSeek(0);
			if (logFileWrite(buf, 10) == false)
				return false;

			return true;
		}
		bool					writeData(const char *inDataBuf, unsigned int inDataLen);
		{
			int		oldPos, pos;

			if (inDataLen <= 0)
				return true;

			if (inDataLen > mFileSize - LOG_FILE_BEGIN_POS)
			{
				inDataBuf += inDataLen - mFileSize + LOG_FILE_BEGIN_POS;
				inDataLen = mFileSize;
			}

			if (mFileSize - mCurrentPos - LOG_FILE_BEGIN_POS >= inDataLen)
			{
				oldPos = mCurrentPos;
				mCurrentPos += inDataLen;

				if (writeCurrentPos() == false)
					return false;

				logFileSeek(oldPos + LOG_FILE_BEGIN_POS);

				if (logFileWrite(inDataBuf, inDataLen) == false)
					return false;
			}
			else
			{
				oldPos = mCurrentPos;
				pos = mFileSize - mCurrentPos - LOG_FILE_BEGIN_POS;
				mCurrentPos = inDataLen - pos;

				if (writeCurrentPos() == false)
					return false;

				logFileSeek(oldPos + LOG_FILE_BEGIN_POS);

				if (logFileWrite(inDataBuf, pos) == false)
					return false;

				logFileSeek(LOG_FILE_BEGIN_POS);

				if (logFileWrite(&inDataBuf[pos], mCurrentPos) == false)
					return false;
			}
			return writeTerminateStr();
		}
		bool					writeTerminateStr()
		{
			int		pos;
			char	*terminateStr = TBC_LOG_TERMINATE_STR;
			
			if (mFileSize - mCurrentPos - LOG_FILE_BEGIN_POS >= LOG_TERMINATE_STR_LEN)
			{
				logFileSeek(mCurrentPos + LOG_FILE_BEGIN_POS);
				if (logFileWrite(terminateStr, LOG_TERMINATE_STR_LEN) == false)
					return false;
			}
			else
			{
				pos = mFileSize - mCurrentPos - LOG_FILE_BEGIN_POS;
				logFileSeek(mCurrentPos + LOG_FILE_BEGIN_POS);
				if (logFileWrite(terminateStr, pos) == false)
					return false;
				logFileSeek(LOG_FILE_BEGIN_POS);
				if (logFileWrite(&terminateStr[pos], LOG_TERMINATE_STR_LEN - pos) == false)
					return false;
			}
			return true;
		}

		bool					logFileOpen(const char *inFileName)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			DWORD	errCode;

			strncpy(mFileName, inFileName, MAX_PATH);
			mFileHandle = ::CreateFile(
								mFileName,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								(LPSECURITY_ATTRIBUTES )NULL,
								OPEN_EXISTING,
		//						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
								FILE_ATTRIBUTE_NORMAL,
								(HANDLE )NULL);

			if (mFileHandle == INVALID_HANDLE_VALUE)
			{
				#ifdef TBC_LOG_FILE_ERR_OUT
					errCode = ::GetLastError();
					cerr << "error: can't open log file " <<  '\"' << mFileName << '\"'  << endl;
					cerr << "       error code = " << errCode << endl;
				#endif
				return false;
			}

			mFileSize = GetFileSize(mFileHandle, NULL);
			if (mFileSize == 0xFFFFFFFF || mFileSize < LOG_FILE_MIN_SIZE)
			{
				#ifdef TBC_LOG_FILE_ERR_OUT
					if (mFileSize == 0xFFFFFFFF)
					{
						errCode = ::GetLastError();
						cerr << "error: can't get file size " <<  '\"' << mFileName << '\"' << endl;
						cerr << "       error code = " << errCode << endl;
					}
					else
					{
						cerr << "error: log file size is too small " <<  '\"' << mFileName << '\"' << endl;
						cerr << "       file size = " << mFileSize << endl;
					}
				#endif

				CloseHandle(mFileHandle);
				mFileHandle = INVALID_HANDLE_VALUE;
				return false;
			}
			return true;
		#else
			strncpy(mFileName, inFileName, MAX_PATH);

			mFile = fopen(mFileName, "r+b");
			if (mFile == NULL)
			{
				#ifdef TBC_LOG_FILE_ERR_OUT
					cerr << "error: can't open log file " <<  '\"' << mFileName << '\"'  << endl;
				#endif
				return false;
			}

			long	size = 0;
			fseek(mFile, 0, SEEK_END);
			size = ftell(mFile);
			mFileSize = (unsigned int)size;

			if (mFileSize < LOG_FILE_MIN_SIZE)
			{
				#ifdef TBC_LOG_FILE_ERR_OUT
					cerr << "error: log file size is too small " <<  '\"' << mFileName << '\"' << endl;
					cerr << "       file size = " << mFileSize << endl;
				#endif
				fclose(mFile);
				mFile = NULL;
				return false;
			}
			return true;
		#endif	// specific parts end ------------------------------------------
		}
		bool					logFileClose()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mFileHandle != INVALID_HANDLE_VALUE)
				::CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;

			return true;
		#else
			if (mFile != NULL)
				fclose(mFile);
			mFile = NULL;

			return true;
		#endif	// specific parts end ------------------------------------------
		}
		bool					logFileWrite(const void *inBuf, unsigned int inWriteLen)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mFileHandle == INVALID_HANDLE_VALUE)
				return false;

			DWORD	writeLen;

			if (::WriteFile(mFileHandle,
							inBuf, inWriteLen,
							&writeLen, NULL) == false)
			{
				#ifdef TBC_LOG_FILE_WARNING_OUT
					DWORD	errCode;
					errCode = ::GetLastError();
					cerr << "warning: can't write to file: " <<  '\"' << mFileName << '\"'  << endl;
					cerr << "       error code = " << errCode << endl;
				#endif
				return false;
			}

			return true;
		#else
			if (mFile == NULL)
				return false;

			if (fwrite(inBuf, 1, inWriteLen, mFile) < inWriteLen)
			{
				#ifdef TBC_LOG_FILE_WARNING_OUT
					cerr << "warning: can't write to file: " <<  '\"' << mFileName << '\"'  << endl;
				#endif
				return false;
			}
			return true;
		#endif	// specific parts end ------------------------------------------
		}
		bool					logFileRead(void *outBuf, unsigned int inReadLen)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mFileHandle == INVALID_HANDLE_VALUE)
				return false;

			DWORD	readLen;

			if (::ReadFile(mFileHandle,
							outBuf, inReadLen,
							&readLen, NULL) == false)
			{
				#ifdef TBC_LOG_FILE_WARNING_OUT
					DWORD	errCode;
					errCode = ::GetLastError();
					cerr << "warning: can't read from file: " <<  '\"' << mFileName << '\"'  << endl;
					cerr << "       error code = " << errCode << endl;
				#endif
				return false;
			}

			return true;
		#else
			if (mFile == NULL)
				return false;

			if (fread(outBuf, 1, inReadLen, mFile) < inReadLen)
			{
				#ifdef TBC_LOG_FILE_WARNING_OUT
					cerr << "warning: can't read from file: " <<  '\"' << mFileName << '\"'  << endl;
				#endif
				return false;
			}

			return true;
		#endif	// specific parts end ------------------------------------------
		}
		bool					logFileSeek(unsigned int inPos)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mFileHandle == INVALID_HANDLE_VALUE)
				return false;
			::SetFilePointer(mFileHandle, inPos, NULL, FILE_BEGIN);
			return true;
		#else
			if (mFile == NULL)
				return false;
			fseek(mFile, inPos, SEEK_SET);
			return true;
		#endif	// specific parts end ------------------------------------------
		bool					logFileFlush()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mFileHandle == INVALID_HANDLE_VALUE)
				return false;
			::FlushFileBuffers(mFileHandle);
			return true;
		#else
			if (mFile == NULL)
				return false;
			fflush(mFile);
			return true;
		#endif	// specific parts end ------------------------------------------
		}
		bool					isLogFileOpened()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mFileHandle == INVALID_HANDLE_VALUE)
				return false;
			return true;
		#else
			if (mFile == NULL)
				return false;
			return true;
		#endif	// specific parts end ------------------------------------------
		}
		void					logFileMakeTimeStampStr(char *inBuf, const size_t inBufSize)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------

			SYSTEMTIME	time;

			::GetLocalTime(&time);
			snprintf(inBuf, inBufSize,
					"%4d/%02d/%02d %02d:%02d:%02d" TBC_LOG_FILE_EOL,
					time.wYear, time.wMonth, time.wDay, time.wHour,
					time.wMinute, time.wSecond);

		#else
			time_t	t;
			struct tm   *tmPtr;

			time(&t);
			tmPtr = localtime(&t);
			snprintf(inBuf, inBufSize,
				"%4d/%02d/%02d %02d:%02d:%02d" TBC_LOG_FILE_EOL,
				tmPtr->tm_year, tmPtr->tm_mon, tmPtr->tm_mday, tmPtr->tm_hour,
				tmPtr->tm_min, tmPtr->tm_sec);

		#endif	// specific parts end ------------------------------------------
		}

		// Member Variables ----------------------------------------------------
		unsigned int			mFileSize;
		char					mFileName[MAX_PATH + 1];
		unsigned int			mCurrentPos;

		MyMutex			mMutex;

	#ifdef _WIN32	//	Win32 specific -----------------------------------------
		HANDLE			mFileHandle;
	#else
		FILE			*mFile;
	#endif			// specific parts end --------------------------------------
}

#endif // TBC_CONSOLE_LOG_HPP
