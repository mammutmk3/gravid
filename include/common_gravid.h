/*
 * common.h
 *
 *  Created on: Jan 31, 2010
 *      Author: lars
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <CL/cl.h>
#include <string>
#include <stdexcept>

namespace GRAVID{

	class Common{
	protected:
		cl_int errorCode;
		std::string errorMsg;

		void errorHappened(const char* error) throw(std::logic_error);
	};
}

#endif /* COMMON_H_ */
