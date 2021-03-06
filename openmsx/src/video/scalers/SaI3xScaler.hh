// $Id: SaI3xScaler.hh 12123 2011-04-17 20:30:07Z m9710797 $

#ifndef SAI3XSCALER_HH
#define SAI3XSCALER_HH

#include "Scaler3.hh"
#include "PixelOperations.hh"

namespace openmsx {

/** 2xSaI algorithm: edge-detection which produces a rounded look.
  * Algorithm was developed by Derek Liauw Kie Fa.
  */
template <typename Pixel>
class SaI3xScaler: public Scaler3<Pixel>
{
public:
	explicit SaI3xScaler(const PixelOperations<Pixel>& pixelOps);
	virtual void scaleBlank1to3(
		FrameSource& src, unsigned srcStartY, unsigned srcEndY,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);
	virtual void scale1x1to3x3(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);

private:
	inline Pixel blend(Pixel p1, Pixel p2);

	template <unsigned NX, unsigned NY>
	void scaleFixed(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);
	void scaleAny(FrameSource& src,
		unsigned srcStartY, unsigned srcEndY, unsigned srcWidth,
		ScalerOutput<Pixel>& dst, unsigned dstStartY, unsigned dstEndY);

	PixelOperations<Pixel> pixelOps;
};

} // namespace openmsx

#endif
