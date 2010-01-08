/*
 * fileNotFound.h
 *
 *  Created on: Jan 7, 2010
 *      Author: lars
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <stdexcept>
#include <string>

namespace GRAVID{

	/**
	 * a custom exception class used to denote an error caused due to a reference to a file, that has not been found
	 */
	class FileNotFound : public std::logic_error{
	public:
		FileNotFound(const std::string& __arg)
		: std::logic_error(__arg){}
	};
}

#endif /* EXCEPTIONS_H_ */
