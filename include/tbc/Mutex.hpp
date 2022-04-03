// =============================================================================
//  Mutex.hpp
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
	\file		tbc/Mutex.h
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_MUTEX_H
#define TBC_MUTEX_H

// Includes --------------------------------------------------------------------
#include "tbc/SyncObjectException.h"
#include "tbc/Thread.h"


// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Throwable interface class
	// -------------------------------------------------------------------------
	class	Mutex
	{
	public:
		// Constructors and Destructor -----------------------------------------
								Mutex()
								{
								#ifdef _WIN32	//	Win32 specific -------------
									mMutex = ::CreateMutex(NULL, false, NULL);
								#elif _PTHREAD	//	pthread specific -----------
									mMutexInitError = pthread_mutex_init(&mMutex, NULL);
								#endif	// specific parts end ------------------
								}
		virtual					~Mutex()
								{
								#ifdef _WIN32	//	Win32 specific -------------
									if (mMutex != NULL)
										::CloseHandle(mMutex);
								#elif _PTHREAD	//	pthread specific -----------
									if (mMutexInitError == 0)
										pthread_mutex_destroy(&mMutex);
								#endif	// specific parts end ------------------
								}

		// Member Functions ----------------------------------------------------
		void					lock()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mMutex == NULL)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"mMutex == NULL", TBC_EXCEPTION_LOCATION_MACRO);
			}
			DWORD	result;

			result = ::WaitForSingleObject(mMutex, INFINITE);
			if (result == WAIT_ABANDONED)
			{
				throw SyncObjectException( SyncObjectException::WAIT_CANCELED,
								"result == WAIT_ABANDONED", TBC_EXCEPTION_LOCATION_MACRO);
			}

			if (result != WAIT_OBJECT_0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"result != WAIT_OBJECT_0", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			return;
		#elif _PTHREAD	//	pthread specific -----------------------------------
			if (mMutexInitError != 0)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"pthread_mutex_init() failed", TBC_EXCEPTION_LOCATION_MACRO);
			}

			int error = pthread_mutex_lock(&mMutex);
			if (error != 0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"pthread_mutex_lock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}
			return;
		#endif	// specific parts end ------------------------------------------
		}
		bool					tryLock()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mMutex == NULL)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"mMutex == NULL", TBC_EXCEPTION_LOCATION_MACRO);
			}

			DWORD	result = ::WaitForSingleObject(mMutex, 0);
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

			int error = pthread_mutex_trylock(&mMutex);

			if (error == 0)
				return true;

			if (error != EBUSY)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"pthread_mutex_trylock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}

			return false;
		#endif	// specific parts end ------------------------------------------
		}
		void					unlock()
		{
		#ifdef _WIN32	//	Win32 specific -------------------------------------
			if (mMutex == NULL)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"mMutex == NULL", TBC_EXCEPTION_LOCATION_MACRO);
			}

			if (::ReleaseMutex(mMutex) == false)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"::ReleaseMutex(mMutex) == false", TBC_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

		#elif _PTHREAD	//	pthread specific -----------------------------------
			if (mMutexInitError != 0)
			{
				throw SyncObjectException( Exception::INTERNAL_ERROR,
								"pthread_mutex_init() failed", TBC_EXCEPTION_LOCATION_MACRO);
			}

			int error = pthread_mutex_unlock(&mMutex);
			if (error != 0)
			{
				throw SyncObjectException( Exception::OS_ERROR,
						"pthread_mutex_unlock() failed", TBC_EXCEPTION_LOCATION_MACRO, error);
			}
		#endif	// specific parts end ------------------------------------------
		}

	private:
	#ifdef _WIN32	//	Win32 specific -----------------------------------------
		HANDLE					mMutex;
	#elif _PTHREAD	//	pthread specific ---------------------------------------
		pthread_mutex_t			mMutex;
		int						mMutexInitError;
	#endif			// specific parts end --------------------------------------
};

#endif	// #ifdef TBC_MUTEX_H

