/// \file
/// \brief DyWaObertoneDetector class
/// \author Alexander Konin
/// \date 22.Sep.2014

#pragma once

#include "@libhdr.h"
#include <memory>

#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable : 4251)  // dll-interface
#endif

struct _dywapitchtracker;

namespace Akon {
namespace HwdLib {

// DyWaObertoneDetector

class HWDLIB_EXPORT DyWaObertoneDetector
{
	DyWaObertoneDetector(const DyWaObertoneDetector&);
	DyWaObertoneDetector& operator=(const DyWaObertoneDetector&);

public:
	DyWaObertoneDetector(int sampleRate, int signalSize);
	~DyWaObertoneDetector();

	double operator()(const double* signal);

private:
	int sampleRate_;
	int signalSize_;
	std::unique_ptr<_dywapitchtracker> pitchTracker_;
};

}  // HwdLib::
}  // Akon::

#if defined(_MSC_VER)
	#pragma warning(pop)
#endif


/* ==TRASH==
*/
