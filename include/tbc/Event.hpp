// =============================================================================
//  Event.hpp
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
	\file		tbc/Event.h
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_EVENT_H
#define TBC_EVENT_H

// Includes --------------------------------------------------------------------
#include "tbc/SyncObjectException.h"
#include "tbc/Thread.h"


// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Throwable interface class
	// -------------------------------------------------------------------------
	class	Event
	{
	public:
		// Constructors and Destructor -----------------------------------------
								Event(bool inIsManualReset = false)
								{
								#ifdef _WIN32	//	Win32 specific -------------
									mEvent = ::CreateEvent(NULL, false, inIsManualReset, NULL);
								#elif _PTHREAD	//	pthread specific -----------
									mIsManualReset = inIsManualReset;
									mIsSignaled = false;
									mIsPulsed = false;
									mMutexInitError = pthread_mutex_init(&mMutex, NULL);
									mCondInitError = pthread_cond_init(&mCond, NULL);
								#endif	// specific parts end ------------------
								}
								~Event()
								{
								#ifdef _WIN32	//	Win32 specific -------------
									if (mEvent != NULL)
										::CloseHandle(mEvent);
								#elif _PTHREAD	//	pthread specific -----------
									if (mMutexInitError == 0)
										pthread_mutex_destroy(&mMutex);
									if (mCondInitError == 0)
										pthread_cond_destroy(&mCond);
								#endif	// specific parts end ------------------
								}

		// Member Functions ----------------------------------------------------
		void					wait()
		{
			timedWait(Thread::WAIT_INFINITE);
		}
		bool					timedWait(timeout_t inMilliseconds)
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mEvent == NULL)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"mEvent == NULL", TBC_EXCEPTION_LOCATION_MACRO);
			}

			if (inMilliseconds == Thread::WAIT_INFINITE)
				inMilliseconds = INFINITE;

			DWORD   result = ::WaitForSingleObject(mEvent, inMilliseconds);
			if (result == WAIT_ABANDONED)
			{
				throw SyncObjectException( SyncObjectException::WAIT_CANCELED,
								"result == WAIT_ABANDONED", TBC_EXCEPTION_LOCATION_MACRO);
			}

			if (result == WAIT_TIMEOUT)
				return false;

			if (result != WAIT_OBJECT_0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"result != WAIT_OBJECT_0", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			return true;
		#elif _PTHREAD	//	pthread specific -----------------------------------
			if (mMutexInitError != 0)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"pthread_mutex_init() failed", TBC_EXCEPTION_LOCATION_MACRO);
			}
			if (mCondInitError != 0)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"pthread_cond_init() failed", TBC_EXCEPTION_LOCATION_MACRO);
			}

			bool	isSignaled;
			int		error;

			error = pthread_mutex_lock(&mMutex);
			if (error != 0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"pthread_mutex_lock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}

			if (mIsSignaled == true)
			{
				if (mIsManualReset == false)
					mIsSignaled = false;
				
				error = pthread_mutex_unlock(&mMutex);
				if (error != 0)
				{
					throw SyncObjectException( Exception::OS_ERROR,
							"pthread_mutex_unlock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
				}
				return true;
			}

			if (inMilliseconds == Thread::WAIT_INFINITE)
			{
				while (mIsSignaled == false)
				{
					error = pthread_cond_wait(&mCond, &mMutex);
					if (error != 0)
					{
						pthread_mutex_unlock(&mMutex);
						throw SyncObjectException( Exception::OS_ERROR,
								"pthread_cond_wait() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
					}
					
					if (mIsPulsed != false)
						break;
				}
				isSignaled = true;
			}
			else
			{
				struct timespec waitUntil;
				
				error = Thread::getUnixTimeout(&waitUntil, inMilliseconds);
				if (error != 0)
				{
					pthread_mutex_unlock(&mMutex);
					throw SyncObjectException( Exception::OS_ERROR,
							"Thread::getUnixTimeout() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
				}

				while (mIsSignaled == false)
				{
					error = pthread_cond_timedwait(&mCond, &mMutex, &waitUntil);
					if (error == 0)
						isSignaled = true;
					else
					{
						if (error == ETIMEDOUT)
						{
							isSignaled = false;
							break;
						}
						else
						{
							pthread_mutex_unlock(&mMutex);
							throw SyncObjectException( Exception::OS_ERROR,
									"pthread_cond_wait() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
						}
					}
					if (mIsPulsed != false)
						break;
				}
			}

			if (isSignaled == true && mIsManualReset == false)
			{
				mIsSignaled = false;
				mIsPulsed = false;
			}

			error = pthread_mutex_unlock(&mMutex);
			if (error != 0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"pthread_mutex_unlock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}

			return isSignaled;
		#endif	// specific parts end ------------------------------------------
		}
		void					signal()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mEvent == NULL)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"mEvent == NULL", TBC_EXCEPTION_LOCATION_MACRO);
			}

			if (::SetEvent(mEvent) == false)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"::SetEvent(mEvent) == false", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
		#elif _PTHREAD	//	pthread specific -----------------------------------
			pthread_cond_signal(false);
		#endif	// specific parts end ------------------------------------------
		}
		void					pulse()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mEvent == NULL)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"mEvent == NULL", TBC_EXCEPTION_LOCATION_MACRO);
			}

			if (::PulseEvent(mEvent) == false)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"::PulseEvent(mEvent) == false", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
		#elif _PTHREAD	//	pthread specific -----------------------------------
			pthread_cond_signal(true);
		#endif	// specific parts end ------------------------------------------
		}
		void					reset()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mEvent == NULL)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"mEvent == NULL", TBC_EXCEPTION_LOCATION_MACRO);
			}

			if (::ResetEvent(mEvent) == false)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"::ResetEvent(mEvent) == false", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
		#elif _PTHREAD	//	pthread specific -----------------------------------
			if (mMutexInitError != 0)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"pthread_mutex_init() failed", TBC_EXCEPTION_LOCATION_MACRO);
			}
			if (mCondInitError != 0)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"pthread_cond_init() failed", TBC_EXCEPTION_LOCATION_MACRO);
			}

			error = pthread_mutex_lock(&mMutex);
			if (error != 0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"pthread_mutex_lock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}

			mIsSignaled = false;
			mIsPulsed = false;

			error = pthread_mutex_unlock(&mMutex);
			if (error != 0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"pthread_mutex_unlock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}
		#endif	// specific parts end ------------------------------------------
		}

	private:
	#ifdef _WIN32	//	Win32 specific -----------------------------------------
		// Member Variables ----------------------------------------------------
		HANDLE					mEvent;
	#elif _PTHREAD	//	pthread specific ---------------------------------------
		// Member Functions ----------------------------------------------------
		void					pthread_cond_signal(bool inIsPulse = false)
		{
			if (mMutexInitError != 0)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"pthread_mutex_init() failed", TBC_EXCEPTION_LOCATION_MACRO);
			}
			if (mCondInitError != 0)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"pthread_cond_init() failed", TBC_EXCEPTION_LOCATION_MACRO);
			}

			int		error;

			error = pthread_mutex_lock(&mMutex);
			if (error != 0)
				throw MySyncObjectException(
					MySyncObjectException::OS_ERROR, "pthread_mutex_lock() failed", "MyEvent::Signal", error);

			if (mIsManuaReset == false)
			{
				error = pthread_cond_signal(&mCond);
				if (error != 0)
				{
					pthread_mutex_unlock(&mMutex);
					throw SyncObjectException( Exception::OS_ERROR,
							"pthread_cond_broadcast() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
				}
			}
			else
			{
				error = pthread_cond_broadcast(&mCond);
				if (error != 0)
				{
					pthread_mutex_unlock(&mMutex);
					throw SyncObjectException( Exception::OS_ERROR,
							"pthread_cond_broadcast() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
				}
			}
			
			if (inIsPulse == false)
			{
				mIsSignaled = true;
				mIsPulsed = false;
			}
			else
			{
				mIsSignaled = false;
				mIsPulsed = true;
			}
			
			error = pthread_mutex_unlock(&mMutex);
			if (error != 0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"pthread_mutex_unlock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}
		}
		
		// Member Variables ----------------------------------------------------
		bool					mIsManualReset;
		bool					mIsSignaled;
		bool					mIsPulsed;
		pthread_mutex_t			mMutex;
		pthread_cond_t			mCond;
		int						mMutexInitError;
		int						mCondInitError;
	#endif			// specific parts end --------------------------------------
};

#endif	// #ifdef TBC_EVENT_H

