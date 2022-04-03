// =============================================================================
//  ConsoleLog.hpp
//
//  Written in 2014 by Dairoku Sekiguchi (sekiguchi at acm dot org)
//
//  To the extent possible under law, the author(s) have dedicated all copyright
//  and related and neighboring rights to this software to the public domain worldwide.
//  This software is distributed without any warranty.
//
//  You should have received a copy of the CC0 Public Domain Dedication along with
//  this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
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
