// =============================================================================
//  SyncObjectException.hpp
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
	\file		tbc/SyncObjectException.h
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_SYNC_OBJECT_EXCEPTION_HPP
#define TBC_SYNC_OBJECT_EXCEPTION_HPP

// Includes --------------------------------------------------------------------
#include "tbc/Exception"

// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Exception class
	// -------------------------------------------------------------------------
	class	SyncObjectException : public Exception
	{
	public:
		// Constructors and Destructor -----------------------------------------
								SyncObjectException(int inExceptionCode, const char *inDescription,
											const char *inLocation, int inOSErrorCode = 0)
									: Exception("ThreadException", inExceptionCode, inDescription,
														inLocation, inOSErrorCode)
								{
								}
								SyncObjectException(const ThreadException &inEx)
									: Exception(inEx);
								{
								}
		virtual					~SyncObjectException() {};

		// Constatns -----------------------------------------------------------
		enum ExceptionCode
		{
			ILLEGAL_OBJECT_STATE		= Exception::SUB_CLASS_ERROR,
			WAIT_CANCELED
		};
}

#endif // TBC_SYNC_OBJECT_EXCEPTION_HPP
