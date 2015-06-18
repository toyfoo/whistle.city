/// \file
/// \brief Utils
/// \author Alexander Konin
/// \date 07.Oct.2014

#include "Utils.h"
#ifdef USE_IPP
	#include <Akon/Ipp/Utils.h>
#else
	#include <numeric>  // std::accumulate
#endif

namespace Akon {
namespace HwdLib {

// Public functions

double sum(const double* data, int size)
{
	double result;
#ifdef USE_IPP
	//WARNING: argument may not align on 16-bytes (not an error if AVX)
	Akon::Ipp::checkFunc(ippsSum_64f(data, size, &result));
#else
	result = std::accumulate(data, data + size, 0.0);
#endif
	return result;
}

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
