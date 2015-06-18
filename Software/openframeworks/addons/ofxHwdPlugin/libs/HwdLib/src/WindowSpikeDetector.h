/// \file
/// \brief WindowSpikeDetector class
/// \author Alexander Konin
/// \date 08.Oct.2014

#pragma once

#include "@libhdr.h"
#include <memory>

namespace Akon {
namespace HwdLib {

// WindowSpikeDetector

class HWDLIB_EXPORT WindowSpikeDetector
{
	WindowSpikeDetector(const WindowSpikeDetector&);
	WindowSpikeDetector& operator=(const WindowSpikeDetector&);

public:
	WindowSpikeDetector(int binsPerUsingRangeOffset, int binsPerUsingRange,
		int binsPerWhistleRangeOffset, int binsPerWhistleRange);

	//void readSettings(const QSettings& settings);
	//void writeSettings(QSettings& settings) const;

	int binsPerWindow() const { return binsPerWindow_; }
	void setBinsPerWindow(int value);

	double operator()(const double* usingRangePowerSpectrum,
		double& spikeToSpikeAndSecondSpikeRatio, double& secondSpike,
		double& spikeSharpness, double& secondSpikeSharpness, double& spikeLevel);

//Q_SIGNALS:
//	void binsPerWindowChanged(int value);

private:
	int binsPerUsingRangeOffset_;
	int binsPerUsingRange_;
	int binsPerWhistleFromUsingRangeOffset_;
	int binsPerWhistleRange_;
	int binsPerWindow_;
	std::unique_ptr<double[]> contrastedSpectrum_;
};

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
