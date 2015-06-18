/// \file
/// \brief FFT class
/// \date 16.Jun.2014

#include "Fft.h"
#include <algorithm>
#include <functional>

namespace Akon {
namespace Fftw {

// Fft

/*!
	\class Fft
	\brief Fft computes forward FFT of real signals with output in the CCS format.
	\internal
*/

/// Creates Fft object. \a order specifies an input signal size (order = log2(signal_size)).
/// \throw Fftw::Exception if an IPP function fails.
Fft::Fft(int order) :
	order_(order),
	in_(checkedFftwMalloc<double>(fftSize())),
	out_(checkedFftwMalloc<fftw_complex>(fftOutputSize())),
	plan_(checkPlan(fftw_plan_dft_r2c_1d(fftSize(), &in_[0], &out_[0],
		/*FFTW_PRESERVE_INPUT | */FFTW_ESTIMATE)))
{
}

Fft::~Fft()
{
}

/// Computes forward FFT of the real signal \a src placing result in \a dst. \a src size is
/// fftSize(), \a dst size is fftOutputSize() in complex elements. \a dst has CCS format:
/// R0 0 R1 I1 . . . RN/2-1 IN/2-1 RN/2 0 (total N/2+1 complex elements).
void Fft::operator()(const double* src, fftw_complex* dst)
{
	fftw_execute_dft_r2c(&*plan_, const_cast<double*>(src), dst);

	//##note: FFTW does not normalize on N, do it
	const double normalizationFactor = 1.0 / fftSize();
	std::transform(&*dst[0], &*dst[fftOutputSize()], &*dst[0],
		std::bind2nd(std::multiplies<double>(), normalizationFactor));
}

}  // Fftw::
}  // Akon::


/* ==TRASH==
/// Creates Fft object. \a order specifies an input signal size (order = log2(signal_size)).
/// \throw Fftw::Exception if an IPP function fails.
Fft::Fft(int order) :
	order_(order),
	plan_(checkedPlan(fftw_plan_dft_r2c_1d(fftSize(), 0, 0, FFTW_PRESERVE_INPUT | FFTW_ESTIMATE)))
{
}

Fft::~Fft()
{
}

/// Computes forward FFT of the real signal \a src placing result in \a dst. \a src size is
/// fftSize(), \a dst size is fftOutputSize() in complex elements. \a dst has CCS format:
/// R0 0 R1 I1 . . . RN/2-1 IN/2-1 RN/2 0 (total N/2+1 complex elements).
void Fft::operator()(const double* src, fftw_complex* dst)
{
	fftw_execute_dft_r2c(&*plan_, const_cast<double*>(src), dst);

	const double normalizationFactor = 1.0 / fftSize();
	std::transform(&*dst[0], &*dst[fftOutputSize()], &*dst[0],
		std::bind2nd(std::multiplies<double>(), normalizationFactor));
}
*/
