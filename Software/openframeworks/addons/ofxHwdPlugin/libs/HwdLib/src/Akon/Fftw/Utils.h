/// \file
/// \brief Some utils for pleasant FFTW usage
/// \date 16.Jun.2014

#pragma once

#include "Exception.h"
#include <fftw3.h>

namespace Akon {
namespace Fftw {

//##note: fftw_complex defined as: typedef double fftw_complex[2];

// Public

/// Allocates aligned memory buffer. The buffer must be freed with fftw_free().
/// \throws Exception
template <typename T>
inline T* checkedFftwMalloc(int size)
{
	T* result = static_cast<T*>(::fftw_malloc(sizeof(T) * size));
	if (result) return result;
	throw Exception("Memory allocation error with a FFTW function");
}

/// Checks allocated plan.
/// \throws Exception
inline fftw_plan checkPlan(fftw_plan plan)
{
	if (plan) return plan;
	throw Exception("Error with a FFTW plan creation function");
}

// DataDeleters for smart pointers holding FFTW buffers

template <typename T>
struct DataDeleter
{
	void operator()(T* ptr) const { fftw_free(ptr); }
};

template <typename T>
struct DataDeleter<T[]>
{
	void operator()(T* ptr) const { fftw_free(ptr); }
};

// PlanDeleter

struct PlanDeleter
{
	void operator()(fftw_plan plan) const { fftw_destroy_plan(plan); }
};


}  // Fftw::
}  // Akon::


/* ==TRASH==
//template <typename T>
//inline T* checkMemAllocFunc(T* allocatedMemory)
//{
//	if (allocatedMemory) return allocatedMemory;
//	throw Exception("Memory allocation error with a FFTW function");
//}
*/
