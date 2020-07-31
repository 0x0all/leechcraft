/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "colortemp.h"
#include <cmath>
#include <QImage>
#include <util/sys/cpufeatures.h>
#include "effectscommon.h"

#ifdef SSE_ENABLED
#include "ssecommon.h"
#endif

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	namespace
	{
		int Clamp (double color)
		{
			return std::max (std::min (static_cast<int> (std::round (color)), 255), 0);
		}

		int Temp2Red (double temperature)
		{
			if (temperature <= 66)
				return 255;

			return Clamp (329.6987 * (std::pow (temperature - 60, -0.1332)));
		}

		int Temp2Green (double temperature)
		{
			if (temperature <= 66)
				return Clamp (99.47 * std::log (temperature) - 161.12);
			else
				return Clamp (288.122 * std::pow (temperature - 60, -0.0755));
		}

		int Temp2Blue (double temperature)
		{
			if (temperature >= 66)
				return 255;
			if (temperature <= 19)
				return 0;

			return Clamp (138.52 * std::log (temperature - 10) - 305.0);
		}

		/** http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/ is used.
		 *
		 * Even though http://www.vendian.org/mncharity/dir3/blackbody/UnstableURLs/bbr_color.html
		 * for instance.
		 */
		QRgb Temp2Rgb (double temperature)
		{
			temperature /= 100;
			return qRgb (Temp2Red (temperature), Temp2Green (temperature), Temp2Blue (temperature));
		}

		void AdjustColorTempInner (unsigned char* pixel, float red, float green, float blue)
		{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
			pixel [0] *= blue;
			pixel [1] *= green;
			pixel [2] *= red;
#else
			pixel [1] *= red;
			pixel [2] *= green;
			pixel [3] *= blue;
#endif
		}

		void AdjustColorTempDefault (QImage& image, int temperature)
		{
			const auto rgb = Temp2Rgb (temperature);
			const auto red = qRed (rgb) / 255.0;
			const auto green = qGreen (rgb) / 255.0;
			const auto blue = qBlue (rgb) / 255.0;

			const auto height = image.height ();
			const auto width = image.width ();

			for (int y = 0; y < height; ++y)
			{
				const auto scanline = image.scanLine (y);
				for (int x = 0; x < width; ++x)
					AdjustColorTempInner (&scanline [x * 4], red, green, blue);
			}
		}

#ifdef SSE_ENABLED
		__attribute__ ((target ("ssse3")))
		void AdjustColorTempSSSE3 (QImage& image, int temperature)
		{
			constexpr auto alignment = 16;

			const auto rgb = Temp2Rgb (temperature);
			const auto red = qRed (rgb);
			const auto green = qGreen (rgb);
			const auto blue = qBlue (rgb);
			const auto redF = red / 255.0;
			const auto greenF = green / 255.0;
			const auto blueF = blue / 255.0;

			const auto height = image.height ();
			const auto width = image.width ();

			const __m128i pixel1msk = MakeMask<7, 0> (Mask128);
			const __m128i pixel2msk = MakeMask<15, 8> (Mask128);

			const __m128i pixel1revmask = MakeRevMask<8, 0, 1> (Mask128);
			const __m128i pixel2revmask = MakeRevMask<8, 1, 1> (Mask128);

			const __m128i mul = _mm_set_epi16 (256, red, green, blue,
					256, red, green, blue);

			for (int y = 0; y < height; ++y)
			{
				uchar * const scanline = image.scanLine (y);

				int x = 0;
				int bytesCount = 0;
				auto handler = [scanline, redF, greenF, blueF] (int i) { AdjustColorTempInner (&scanline [i], redF, greenF, blueF); };
				HandleLoopBegin<alignment> (scanline, width, x, bytesCount, handler);

				for (; x < bytesCount; x += alignment)
				{
					__m128i fourPixels = _mm_load_si128 (reinterpret_cast<const __m128i*> (scanline + x));

					__m128i pair1 = _mm_shuffle_epi8 (fourPixels, pixel1msk);
					pair1 = _mm_mullo_epi16 (pair1, mul);
					pair1 = _mm_shuffle_epi8 (pair1, pixel1revmask);

					__m128i pair2 = _mm_shuffle_epi8 (fourPixels, pixel2msk);
					pair2 = _mm_mullo_epi16 (pair2, mul);
					pair2 = _mm_shuffle_epi8 (pair2, pixel2revmask);

					fourPixels = _mm_or_si128 (pair1, pair2);

					_mm_store_si128 (reinterpret_cast<__m128i*> (scanline + x), fourPixels);
				}

				HandleLoopEnd (width, x, handler);
			}
		}

		__attribute__ ((target ("avx2")))
		void AdjustColorTempAVX2 (QImage& image, int temperature)
		{
			constexpr auto alignment = 32;

			const auto rgb = Temp2Rgb (temperature);
			const auto red = qRed (rgb);
			const auto green = qGreen (rgb);
			const auto blue = qBlue (rgb);
			const auto redF = red / 255.0;
			const auto greenF = green / 255.0;
			const auto blueF = blue / 255.0;

			const auto height = image.height ();
			const auto width = image.width ();

			const __m256i pixel1msk = MakeMask<7, 0> (Mask256);
			const __m256i pixel2msk = MakeMask<15, 8> (Mask256);

			const __m256i pixel1revmask = MakeRevMask<8, 0, 1> (Mask256);
			const __m256i pixel2revmask = MakeRevMask<8, 1, 1> (Mask256);

			const __m256i mul = _mm256_set_epi16 (256, red, green, blue,
					256, red, green, blue,
					256, red, green, blue,
					256, red, green, blue);

			for (int y = 0; y < height; ++y)
			{
				uchar * const scanline = image.scanLine (y);

				int x = 0;
				int bytesCount = 0;
				auto handler = [scanline, redF, greenF, blueF] (int i) { AdjustColorTempInner (&scanline [i], redF, greenF, blueF); };
				HandleLoopBegin<alignment> (scanline, width, x, bytesCount, handler);

				for (; x < bytesCount; x += alignment)
				{
					__m256i pixels = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x));

					__m256i part1 = _mm256_shuffle_epi8 (pixels, pixel1msk);
					part1 = _mm256_mullo_epi16 (part1, mul);
					part1 = _mm256_shuffle_epi8 (part1, pixel1revmask);

					__m256i part2 = _mm256_shuffle_epi8 (pixels, pixel2msk);
					part2 = _mm256_mullo_epi16 (part2, mul);
					part2 = _mm256_shuffle_epi8 (part2, pixel2revmask);

					pixels = _mm256_or_si256 (part1, part2);

					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x), pixels);
				}

				HandleLoopEnd (width, x, handler);
			}
		}
#endif
	}

	void AdjustColorTemp (QImage& image, int temperature)
	{
#ifdef SSE_ENABLED
		static const auto ptr = Util::CpuFeatures::Choose ({
					{ Util::CpuFeatures::Feature::AVX2, &AdjustColorTempAVX2 },
					{ Util::CpuFeatures::Feature::SSSE3, &AdjustColorTempSSSE3 }
				},
				&AdjustColorTempDefault);

		ptr (image, temperature);
#else
		AdjustColorTempDefault (image, temperature);
#endif
	}
}
}
}
