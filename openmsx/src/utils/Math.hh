// $Id: Math.hh 12802 2012-08-13 17:10:14Z m9710797 $

#ifndef MATH_HH
#define MATH_HH

#include "openmsx.hh"
#include "static_assert.hh"
#include "inline.hh"
#include "likely.hh"
#include "build-info.hh"
#include <algorithm>
#include <cmath>

namespace openmsx {

#ifdef _MSC_VER
	// C99 math functions missing from VC++'s CRT as of 2008
	// TODO - define HAVE_C99MATHOPS instead
	long lrint(double x);
	long lrintf(float x);
	float truncf(float x);
	double round(double x);
#endif

namespace Math {


/** Is the given number an integer power of 2?
  * Not correct for zero (according to this test 0 is a power of 2).
  */
inline bool isPowerOfTwo(unsigned a)
{
	return (a & (a - 1)) == 0;
}

/** Returns the smallest number that is both >=a and a power of two.
  */
unsigned powerOfTwo(unsigned a);

/** Returns two gaussian distributed random numbers.
  * We return two numbers instead of one because the second number comes for
  * free in the current implementation.
  */
void gaussian2(double& r1, double& r2);

/** Clips x to the range [LO,HI].
  * Slightly faster than    std::min(HI, std::max(LO, x))
  * especially when no clipping is required.
  */
template <int LO, int HI>
inline int clip(int x)
{
	return unsigned(x - LO) <= unsigned(HI - LO) ? x : (x < HI ? LO : HI);
}

/** Clip x to range [-32768,32767]. Special case of the version above.
  * Optimized for the case when no clipping is needed.
  */
inline short clipIntToShort(int x)
{
	STATIC_ASSERT((-1 >> 1) == -1); // right-shift must preserve sign
	return likely(short(x) == x) ? x : (0x7FFF - (x >> 31));
}

/** Clip x to range [0,255].
  * Optimized for the case when no clipping is needed.
  */
inline byte clipIntToByte(int x)
{
	STATIC_ASSERT((-1 >> 1) == -1); // right-shift must preserve sign
	return likely(byte(x) == x) ? x : ~(x >> 31);
}

/** Clips r * factor to the range [LO,HI].
  */
template <int LO, int HI>
inline int clip(double r, double factor)
{
	int a = int(round(r * factor));
	return std::min(std::max(a, LO), HI);
}

/** Calculate greatest common divider of two strictly positive integers.
  * Classical implementation is like this:
  *    while (unsigned t = b % a) { b = a; a = t; }
  *    return a;
  * The following implementation avoids the costly modulo operation. It
  * is about 40% faster on my machine.
  *
  * require: a != 0  &&  b != 0
  */
inline unsigned gcd(unsigned a, unsigned b)
{
	unsigned k = 0;
	while (((a & 1) == 0) && ((b & 1) == 0)) {
		a >>= 1; b >>= 1; ++k;
	}

	// either a or b (or both) is odd
	while ((a & 1) == 0) a >>= 1;
	while ((b & 1) == 0) b >>= 1;

	// both a and b odd
	while (a != b) {
		if (a >= b) {
			a -= b;
			do { a >>= 1; } while ((a & 1) == 0);
		} else {
			b -= a;
			do { b >>= 1; } while ((b & 1) == 0);
		}
	}
	return b << k;
}

/** Reverse the lower N bits of a given value.
  * The upper 32-N bits from the input are ignored and will be returned as 0.
  * For example reverseNBits('xxxabcde', 5) returns '000edcba' (binary notation).
  */
inline unsigned reverseNBits(unsigned x, unsigned bits)
{
	unsigned ret = 0;
	while (bits--) {
		ret = (ret << 1) | (x & 1);
		x >>= 1;
	}
	return ret;

	/* Just for fun I tried the asm version below (the carry-flag trick
	 * cannot be described in plain C). It's correct and generates shorter
	 * code (both less instructions and less bytes). But it doesn't
	 * actually run faster on the machine I tested on, or only a tiny bit
	 * (possibly because of dependency chains and processor stalls???).
	 * However a big disadvantage of this asm version is that when called
	 * with compile-time constant arguments, this version performs exactly
	 * the same, while the version above can be further optimized by the
	 * compiler (constant-propagation, loop unrolling, ...).
	unsigned ret = 0;
	if (bits) {
		asm (
		"1:	shr	%[VAL]\n"
		"	adc	%[RET],%[RET]\n"
		"	dec	%[BITS]\n"
		"	jne	1b\n"
			: [VAL]  "+r" (val)
			, [BITS] "+r" (bits)
			, [RET]  "+r" (ret)
		);
	}
	return ret;
	*/

	/* Maarten suggested the following approach with O(lg(N)) time
	 * complexity (the version above is O(N)).
	 *  - reverse full (32-bit) word: O(lg(N))
	 *  - shift right over 32-N bits: O(1)
	 * Note: In some lower end CPU the shift-over-N-bits instruction itself
	 *       is O(N), in that case this whole algorithm is O(N)
	 * Note2: Instead of '32' it's also possible to use a lower power of 2,
	 *        as long as it's bigger than or equal to N.
	 * This algorithm may or may not be faster than the version above, I
	 * didn't try it yet. Also because this routine is _NOT_ performance
	 * critical _AT_ALL_ currently.
	 */
}

/** Reverse the bits in a byte.
  * This is equivalent to (but faster than) reverseNBits(x, 8);
  */
inline byte reverseByte(byte a)
{
	// Classical implementation (can be extended to 16 and 32 bits)
	//   a = ((a & 0xF0) >> 4) | ((a & 0x0F) << 4);
	//   a = ((a & 0xCC) >> 2) | ((a & 0x33) << 2);
	//   a = ((a & 0xAA) >> 1) | ((a & 0x55) << 1);
	//   return a;

	// The versions below are specific to reverse a single byte (can't
	// easily be extended to wider types). Found these tricks on:
	//    http://graphics.stanford.edu/~seander/bithacks.html
#ifdef __x86_64
	// on 64-bit systems this is slightly faster
	return (((a * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL) >> 32;
#else
	// on 32-bit systems this is faster
	return (((a * 0x0802 & 0x22110) | (a * 0x8020 & 0x88440)) * 0x10101) >> 16;
#endif
}

/** Returns the smallest number of the form 2^n-1 that is greater or equal
  * to the given number.
  * The resulting number has the same number of leading zeros as the input,
  * but starting from the first 1-bit in the input all bits more to the right
  * are also 1.
  */
inline unsigned floodRight(unsigned x)
{
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x;
}

/** Count the number of leading zero-bits in the given word.
  * The result is undefined when the input is zero (all bits are zero).
  */
inline unsigned countLeadingZeros(unsigned x)
{
#ifdef __GNUC__
	// actually this only exists starting from gcc-3.4.x
	return __builtin_clz(x); // undefined when x==0
#else
	// gives incorrect result for x==0, but that doesn't matter here
	unsigned lz = 0;
	if (x <= 0x0000ffff) { lz += 16; x <<= 16; }
	if (x <= 0x00ffffff) { lz +=  8; x <<=  8; }
	if (x <= 0x0fffffff) { lz +=  4; x <<=  4; }
	lz += (0x55ac >> ((x >> 27) & 0x1e)) & 0x3;
	return lz;
#endif
}

} // namespace Math

} // namespace openmsx

#endif // MATH_HH
