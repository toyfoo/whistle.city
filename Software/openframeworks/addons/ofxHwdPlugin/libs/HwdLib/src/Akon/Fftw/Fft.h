/// \file
/// \brief FFT class
/// \date 15.Jun.2014

#pragma once

#include "Utils.h"
#include <memory>
#include <type_traits>

namespace Akon {
namespace Fftw {

// Fft

class Fft
{
	Fft(const Fft&);
	Fft& operator=(const Fft&);

public:
	explicit Fft(int order);
	~Fft();

	/// Returns FFT order i.e. log2(fftSize()).
	int order() const { return order_; }

	/// Returns input signals size.
	int fftSize() const { return 1 << order_; }

	/// Returns FFT output size. The output format is CCS:
	/// R0 0 R1 I1 . . . RN/2-1 IN/2-1 RN/2 0 (total N/2+1 complex elements).
	int fftOutputSize() const { return fftSize() / 2 + 1; }

	void operator()(const double* src, fftw_complex* dst);

private:
	int order_;
	// fftw_plan itself pointer to fftw_plan_s
	std::unique_ptr<double[], Akon::Fftw::DataDeleter<double[]> > in_;
	std::unique_ptr<fftw_complex[], Akon::Fftw::DataDeleter<fftw_complex[]> > out_;
	std::unique_ptr<std::remove_pointer<fftw_plan>::type, Akon::Fftw::PlanDeleter> plan_;
};

}  // Fftw::
}  // Akon::


/* ==TRASH==
class Fft
{
	Fft(const Fft&);
	Fft& operator=(const Fft&);

public:
	explicit Fft(int order);
	~Fft();

	/// Returns FFT order i.e. log2(fftSize()).
	int order() const { return order_; }

	/// Returns input signals size.
	int fftSize() const { return 1 << order_; }

	/// Returns FFT output size. The output format is CCS:
	/// R0 0 R1 I1 . . . RN/2-1 IN/2-1 RN/2 0 (total N/2+1 complex elements).
	int fftOutputSize() const { return fftSize() / 2 + 1; }

	void operator()(const double* src, fftw_complex* dst);

private:
	int order_;
	// fftw_plan itself pointer to fftw_plan_s
	std::unique_ptr<std::remove_pointer<fftw_plan>::type, Akon::Fftw::PlanDeleter> plan_;
};
*/
