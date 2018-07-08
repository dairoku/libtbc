// =============================================================================
//  Exception.hpp
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
	\file		tbc/Exception.h
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_EXCEPTION_HPP
#define TBC_EXCEPTION_HPP

// Includes --------------------------------------------------------------------
#include <string.h>

// Macros ----------------------------------------------------------------------
#define TBC_EXCEPT_AT_STRINGIFY(x)		#x
#define TBC_EXCEPT_AT_TOSTRING(x)		TBC_EXCEPT_AT_STRINGIFY(x)
#define TBC_EXCEPTION_AT				__FILE__ ":" TBC_EXCEPT_AT_TOSTRING(__LINE__)
#define	TBC_EXCEPTION_LOCATION_MACRO	__FUNCTION__ "  (" TBC_EXCEPTION_AT ")"


// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Throwable interface class
	// -------------------------------------------------------------------------
	class	Throwable
	{
	public:
		//	member functions
		virtual int				getExceptionCode()	const = 0;
		virtual int				getOSErrorCode()	const = 0;
		virtual const char		*getExceptionName()	const = 0;
		virtual const char		*getDescription()	const = 0;
		virtual const char		*getLocation()		const = 0;
	};

	// -------------------------------------------------------------------------
	// Exception class
	// -------------------------------------------------------------------------
	class	Exception : public Throwable
	{
	public:
		// Destructor ----------------------------------------------------------
	virtual						~Exception() {};

		// Constatns -----------------------------------------------------------
		static const int		MESSAGE_BUFFER_SIZE				= 128;
		enum ExceptionCode
		{
			OS_ERROR			= 1,
			MEMORY_ERROR,
			PARAM_ERROR,
			INVALID_OPERATION_ERROR,
			INTERNAL_ERROR,

			SUB_CLASS_ERROR		= 32768
		};

		// Member Functions ----------------------------------------------------
		virtual int				getExceptionCode() const { return mExceptionCode; };
		virtual int				getOSErrorCode() const { return mOSErrorCode; };
		virtual const char		*getExceptionName() const { return mExceptionName; };
		virtual const char		*getDescription() const { return mDescription; };
		virtual const char		*getLocation() const { return mLocation; };
		virtual void			dump()
		{
			printf("Catch %s\n",
					getExceptionName());
			printf("Desc=%s\n",
					getDescription());
			printf("ExCode=%d (OSError=%d) in\n",
					getExceptionCode(), GetOSErrorCode());
			printf("%s\n",
					getLocation());
		};

	protected:
		// Constructors ---------------------------------------------------------
								Exception(const char *inExceptionName, int inExceptionCode, const char *inDescription,
											const char *inLocation, int inOSErrorCode = 0)
								{
									set(inExceptionName, inExceptionCode, inDescription,
										inLocation, inOSErrorCode);
								}
								Exception(const throwable &inEx)
								{
									set(inEx.GetExceptionName(), inEx.GetExceptionCode(), inEx.GetDescription(),
										inEx.GetLocation(), inEx.GetOSErrorCode());
								}
								Exception(const exception &inEx)
								{
									set(inEx.GetExceptionName(), inEx.GetExceptionCode(), inEx.GetDescription(),
										inEx.GetLocation(), inEx.GetOSErrorCode());
								}

		// Member Functions ----------------------------------------------------
		virtual void			set(const char *inExceptionName, int inExceptionCode, const char *inDescription,
									const char *inLocation, int inOSErrorCode)
		{
			setExceptionName(inExceptionName);
			setExceptionCode(inExceptionCode);
			setDescription(inDescription);
			setLocation(inLocation);
			setOSErrorCode(inOSErrorCode);
		};

		virtual void			setExceptionCode(int inExceptionCode) { mExceptionCode = inExceptionCode; };
		virtual void			setOSErrorCode(int inOSErrorCode) { mOSErrorCode = inOSErrorCode; };
		virtual void			setExceptionName(const char *inExceptionName)
		{
			setMessageToBuffer(inExceptionName, mExceptionName);
		};
		virtual void			setDescription(const char *inDescription)
		{
			setMessageToBuffer(inDescription, mDescription);
		};
		virtual void			setLocation(const char *inLocation)
		{
			setMessageToBuffer(inLocation, mLocation);
		};
		virtual void			setMessageToBuffer(const char *inMessage, char *inBuf)
		{
			size_t	len;
	
			len = ::strlen(inMessage);
			if (len >= MESSAGE_BUFFER_SIZE)
				len = MESSAGE_BUFFER_SIZE - 1;
			::memcpy(inBuf, inMessage, len);
			inBuf[len] = 0;
		};

	private:
		// Member Variables ----------------------------------------------------
		int						mExceptionCode, mOSErrorCode;
		char					mExceptionName[MESSAGE_BUFFER_SIZE];
		char					mDescription[MESSAGE_BUFFER_SIZE];
		char					mLocation[MESSAGE_BUFFER_SIZE];
	};
}

#endif // TBC_EXCEPTION_HPP
