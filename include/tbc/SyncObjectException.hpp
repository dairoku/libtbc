// =============================================================================
//  SyncObjectException.hpp
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
