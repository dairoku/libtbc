// =============================================================================
//  ConsoleLog.hpp
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
// ==============================================================================
/*!
	\file		tbc/ConsoleLog.hpp
	\author		Dairoku Sekiguchi
	\version	3.0.1
	\date		2014/01/10
	\brief		Header file for tbc library types

	This file defines types for tbc library
*/

#ifndef TBC_CONSOLE_LOG_HPP
#define TBC_CONSOLE_LOG_HPP

// Includes --------------------------------------------------------------------
#include <iostream>
#include "tbc/log/Log.hpp"
#include "tbc/Mutex.hpp"

// Namespace -------------------------------------------------------------------
namespace tbc
{
	// -------------------------------------------------------------------------
	// Exception class
	// -------------------------------------------------------------------------
	class	ConsoleLog : public virtual LogBase
	{
	public:
		// Constructors and Destructor -----------------------------------------
								ConsoleLog() {}
								~ConsoleLog() {}

		// Member Functions ----------------------------------------------------
		virtual void			write(unsigned int inType, unsigned char inLenvel, const char *inMessage)
		{
			if (!isLogOutMessage(inType, inLevel))
				return;

			try
			{
				mMutex.lock();
			}

			catch (Exception &ex)
			{
				std::cerr << "Can't lock mutex" << std::endl;
				ex.dump();
			}

			const size_t	bufSize = 80;
			char	buf[bufSize];

			makeTimeStampStr(buf, bufSize);
			std::cout << buf;

			makeTypeStr(inType, buf, bufSize);
			std::cout << buf;

			makeLevelStr(inLevel, buf, bufSize);
			std::cout << buf;

			std::cout << inMessage << std::endl;

			try
			{
				mMutex.unlock();
			}

			catch (MyException &ex)
			{
				std::cerr << "Can't unlock mutex" << std::endl;
				ex.dump();
			}
		}
		virtual void			binayDump(int inDumpType, const char *inDumpName, const unsigned char *inData, int inDataLen)
		{
		}

	private:
		// Member Variables ----------------------------------------------------
		Mutex					mMutex;
}

#endif // TBC_CONSOLE_LOG_HPP
