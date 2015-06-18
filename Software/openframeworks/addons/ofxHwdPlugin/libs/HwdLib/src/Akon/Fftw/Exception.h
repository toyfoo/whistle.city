/// \file
/// \brief Exception class
/// \date 14.Mar.2015

#pragma once

#include <stdexcept>

namespace Akon {
namespace Fftw {

// Exception

class Exception : public std::runtime_error
{
	typedef std::runtime_error Base;

public:
	explicit Exception(const std::string& what) : Base(what) {}
    explicit Exception(const char* what) : Base(what) {}
};

}  // Fftw::
}  // Akon::


/* ==TRASH==
*/
