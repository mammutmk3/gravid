/*
 * common.cpp
 *
 *  Created on: Jan 31, 2010
 *      Author: lars
 */

#include "common_gravid.h"

#include <string>
#include <stdexcept>

using namespace GRAVID;

void Common::errorHappened(const char* error) throw(std::logic_error){
	this->errorMsg = error;
	throw std::logic_error(this->errorMsg);
}
