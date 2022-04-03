// =============================================================================
//  ThreadException.hpp
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
	\file		tbc/ThreadException.h
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_THREAD_EXCEPTION_HPP
#define TBC_THREAD_EXCEPTION_HPP

// Includes --------------------------------------------------------------------
#include "tbc/Exception"

// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Exception class
	// -------------------------------------------------------------------------
	class	ThreadException : public Exception
	{
	public:
		// Constructors and Destructor -----------------------------------------
								ThreadException(int inExceptionCode, const char *inDescription,
											const char *inLocation, int inOSErrorCode = 0)
									: Exception("ThreadException", inExceptionCode, inDescription,
														inLocation, inOSErrorCode)
								{
								}
								ThreadException(const ThreadException &inEx)
									: Exception(inEx);
								{
								}
		virtual					~ThreadException() {};

		// Constatns -----------------------------------------------------------
		enum ExceptionCode
		{
			ILLEGAL_THREAD_STATE		= Exception::SUB_CLASS_ERROR,
			THREAD_CANCELED
		};
}

#endif // TBC_THREAD_EXCEPTION_HPP
