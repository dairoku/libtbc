// =============================================================================
//  Thread.hpp
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
	\file		tbc/Thread.h
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_THREAD_H
#define TBC_THREAD_H

// Includes --------------------------------------------------------------------
#include "tbc/ThreadException.h"
#ifdef _WIN32	//	Win32 specific ---------------------------------------------
//	none
#elif _PTHREAD	//	pthread specific -------------------------------------------
 #include <pthread.h>
  #ifdef TBC_USE_CLOCK_GETTIME_
   #include <time.h>
  #endif
#endif			// specific parts end ------------------------------------------


// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Class pre-declaration
	// -------------------------------------------------------------------------
	class Mutex;

	// -------------------------------------------------------------------------
	// Throwable interface class
	// -------------------------------------------------------------------------
	class	Thread
	{
	public:
		// Constructors and Destructor -----------------------------------------
		// Destructor ----------------------------------------------------------
		virtual					~Thread()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mThread != NULL)
			{
				try
				{
					join();
				}

				catch(...)
				{
				}
				::CloseHandle(mThread);
			}
		#elif _PTHREAD	//	pthread specific -----------------------------------
			if (mIsThreadStarted != false)
			{
				try
				{
					join();
				}

				catch(...)
				{
				}
			}
		#endif	// specific parts end ------------------------------------------
		}

		// Member Functions ----------------------------------------------------
		void					start()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			mCallMutex.lock();
			{
				if (mThread != NULL)
				{
					if (::WaitForSingleObject(mThread, 0) != WAIT_OBJECT_0)
					{
						mCallMutex.unlock();
						throw ThreadException( Exception::PARAM_ERROR,
								"Thread is already started", TBC_EXCEPTION_LOCATION_MACRO);
					}
					::CloseHandle(mThread);
				}

				DWORD	threadID;
				mThread = ::CreateThread(NULL, 0,
										(LPTHREAD_START_ROUTINE )ThreadEntryFunc, (LPVOID )this,
										0, &threadID);
				if (mThread == NULL)
				{
					mCallMutex.unlock();
					throw ThreadException( Exception::OS_ERROR,
							"mThread == NULL", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
				}
			}
			mCallMutex.unlock();
		#elif _PTHREAD	//	pthread specific -----------------------------------
			mCallMutex.lock();
			{
				if (mIsThreadStarted != false && mIsThreadStopped == false)
				{
					mCallMutex.unlock();
					throw ThreadException( Exception::PARAM_ERROR,
							"Thread is already started", TBC_EXCEPTION_LOCATION_MACRO);
				}

				int error = pthread_create(&mThread, NULL, ThreadEntryFunc, (void *)this);
				if (error != 0)
				{
					mCallMutex.unlock();
					throw ThreadException( Exception::OS_ERROR,
							"mThread == NUL", TBC_EXCEPTION_LOCATION_MACRO, error);
				}
				mIsThreadStarted = true;
			}
			mCallMutex.unlock();
		#endif	// specific parts end ------------------------------------------
		}
		void					signalStop()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mThread == NULL)
			{
				throw ThreadException( Exception::PARAM_ERROR,
						"Thread is not started", TBC_EXCEPTION_LOCATION_MACRO);
			}

			mCallMutex.lock();
			{
				if (::WaitForSingleObject(mThread, 0) == WAIT_OBJECT_0)
				{
					mCallMutex.unlock();
					return;
				}
				stopper();
			}
			mCallMutex.unlock();
		#elif _PTHREAD	//	pthread specific -----------------------------------
			if (mIsThreadStarted == false)
				throw ThreadException( Exception::PARAM_ERROR,
						"Thread is not started", TBC_EXCEPTION_LOCATION_MACRO);

			mCallMutex.lock();
			{
				if (mIsThreadStopped != false)
				{
					mCallMutex.unlock();
					return;
				}
				stopper();
			}
			mCallMutex.unlock();
		#endif	// specific parts end ------------------------------------------
		}
		void					join()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mThread == NULL)
			{
				throw ThreadException( ThreadException::ILLEGAL_THREAD_STATE,
						"Thread is not started", TBC_EXCEPTION_LOCATION_MACRO);
			}

			DWORD   result = ::WaitForSingleObject(mThread, INFINITE);
			if (result == WAIT_ABANDONED)
			{
				throw ThreadException( Exception::THREAD_CANCELED,
						"result == WAIT_ABANDONED", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			if (result != WAIT_OBJECT_0)
			{
				throw ThreadException( Exception::OS_ERROR,
						"result != WAIT_OBJECT_0", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			return;
		#elif _PTHREAD	//	pthread specific -----------------------------------
			if (mIsThreadStarted == false)
			{
				throw ThreadException( ThreadException::ILLEGAL_THREAD_STATE,
						"Thread is not started", TBC_EXCEPTION_LOCATION_MACRO);
			}

			void	*valuePtr;
			int error = pthread_join(mThread, &valuePtr);
			if (error != 0)
			{
				throw ThreadException( Exception::OS_ERROR,
						"pthread_join() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}

			return;
		#endif	// specific parts end ------------------------------------------
		}
		bool					isAlive() const
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mThread == NULL)
				return false;

			if (::WaitForSingleObject(mThread, 0) == WAIT_OBJECT_0)
				return false;

			return true;
		#elif _PTHREAD	//	pthread specific -----------------------------------
			if (mIsThreadStarted == false)
				return false;

			if (mIsThreadStopped != false)
				return false;

			return true;
		#endif	// specific parts end ------------------------------------------
		}
		void					setPriority(int inPriority)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
		#elif _PTHREAD	//	pthread specific -----------------------------------
		#endif	// specific parts end ------------------------------------------
		}
		int						getPriority()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			return 0;
		#elif _PTHREAD	//	pthread specific -----------------------------------
			return 0;
		#endif	// specific parts end ------------------------------------------
		}

		// Static Functions ----------------------------------------------------
		static void				sleep(timeout_t inMilliseconds)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			::Sleep(inMilliseconds);
		#elif _PTHREAD	//	pthread specific -----------------------------------
			struct timespec	reqTime;

			reqTime.tv_sec = inMilliseconds / 1000;
			inMilliseconds -= (reqTime.tv_sec * 1000);
			reqTime.tv_nsec = inMilliseconds * 1000000;

			nanosleep(&reqTime, null);

			//usleep(inMilliseconds * 1000);  // microseconds to miliseconds
		#endif	// specific parts end ------------------------------------------
		}
		static unsigned int		getTickCount()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			return ::GetTickCount();
		#elif _PTHREAD	//	pthread specific -----------------------------------
		 #ifdef TBC_USE_CLOCK_GETTIME
			unsigned int    tick;
			struct timespec t;

			clock_gettime(CLOCK_REALTIME, &t);
			tick = t.tv_sec * MILLI_SECOND_UNIT;
			tick += (t.tv_nsec / MICRO_SECOND_UNIT);

			return tick;
		 #else
			unsigned int    tick;
			int		error;
			struct timeval  timeVal;
			struct timezone timeZone;

			error = gettimeofday(&timeVal, &timeZone);
			if (error != 0)
				return 0;

			tick = timeVal.tv_sec * MILLI_SECOND_UNIT;
			tick += (timeVal.tv_usec / MILLI_SECOND_UNIT);

			return tick;
		#endif	// specific parts end ------------------------------------------
		}
	//	pthread specific -------------------------------------------------------
	#if _PTHREAD	
		int getUnixTimeout(struct timespec *outTime, timeout_t inMilliseconds)
		{
			int		error;

			error = getUnixTime(outTime);
			if (error != 0)
				return error;
			calcUnixTimeout(outTime, inMilliseconds);

			return 0;
		}
		int	getUnixTime(struct timespec *outTime)
		{
		#ifdef TBC_USE_CLOCK_GETTIME
			return clock_gettime(CLOCK_REALTIME, outTime);
		#else
			int		error;
			struct timeval  timeVal;
			struct timezone timeZone;

			error = gettimeofday(&timeVal, &timeZone);
			if (error != 0)
				return error;

			outTime->tv_sec = timeVal.tv_sec;
			outTime->tv_nsec = timeVal.tv_usec * MY_MILLI_SECOND_UNIT;

			return error;
		#endif
		}
		void	calcUnixTimeout(struct timespec *ioTime, timeout_t inMilliseconds)
		{
			timeout_t		t;

			t = inMilliseconds / MY_MILLI_SECOND_UNIT;
			if (TIMEOUT_T_MAX - ioTime->tv_sec < t)
			{
				ioTime->tv_sec = TIMEOUT_T_MAX;
				ioTime->tv_nsec = TIMEOUT_T_MAX;
				return;
			}

			ioTime->tv_sec += t;
			inMilliseconds = inMilliseconds - t * MY_MILLI_SECOND_UNIT;
			ioTime->tv_nsec += inMilliseconds * MY_MICRO_SECOND_UNIT;
			if (ioTime->tv_nsec >= MY_NANO_SECOND_UNIT)
			{
				(ioTime->tv_nsec) -= MY_NANO_SECOND_UNIT;

				if (TIMEOUT_T_MAX - ioTime->tv_sec == 0)
				{
					ioTime->tv_nsec = MY_NANO_SECOND_UNIT;
					return;
				}
				(ioTime->tv_sec)++;
			}
		}
	#endif
	// specific parts end ------------------------------------------------------

		// Constatns -----------------------------------------------------------
		const static timeout_t	WAIT_INFINITE						= 0xFFFFFFFF;

	protected:
		// Constructors --------------------------------------------------------
								Thread()
								{
								#ifdef _WIN32	//	Win32 specific -------------
									mThread = NULL;
								#elif _PTHREAD	//	pthread specific -----------

									mIsThreadStarted = false;
									mIsThreadStopped = false;
								#endif	// specific parts end ------------------
								}

		// Constatns -----------------------------------------------------------
		const static timeout_t	MILLI_SECOND_UNIT					= 1000;
		const static timeout_t	MICRO_SECOND_UNIT					= 1000000;
		const static timeout_t	NANO_SECOND_UNIT					= 1000000000;

		// Member Functions ----------------------------------------------------
		virtual void			runner() = 0;
		virtual void			stopper() = 0;

	private:
		// Member Variables ----------------------------------------------------
		Mutex					mCallMutex;

	#ifdef _WIN32	//	Win32 specific -----------------------------------------
		static DWORD			threadEntryFunc(void *inObjPtr)
		{
			((MyThread *)inObjPtr)->runner();

			return 0;
		}

		HANDLE					mThread;
	#elif _PTHREAD	//	pthread specific ---------------------------------------
		static void				*threadEntryFunc(void *inObjPtr)
		{
			((MyThread *)inObjPtr)->runner();
			((MyThread *)inObjPtr)->mIsThreadStopped = true;

			return NULL;
		}

		pthread_t				mThread;
		bool					mIsThreadStarted, mIsThreadStopped;
	#endif			// specific parts end --------------------------------------
};

#endif	// #ifdef TBC_THREAD_H

