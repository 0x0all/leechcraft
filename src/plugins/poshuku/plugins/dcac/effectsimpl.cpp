/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "effectsimpl.h"
#include <cmath>
#include <QImage>

#if (defined (Q_PROCESSOR_X86_64) || QT_VERSION < 0x050000) && !defined (Q_OS_MAC)
#define SSE_ENABLED
#endif

#ifdef SSE_ENABLED
#include <tmmintrin.h>
#include <immintrin.h>
#include <avxintrin.h>
#include <avx2intrin.h>
#endif

#include <util/sys/cpufeatures.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace DCAC
{
	namespace
	{
		uint64_t CombineGray (uint64_t r, uint64_t g, uint64_t b)
		{
			return r * 11 + g * 16 + b * 5;
		}

		uint64_t GetGrayDefault (const QImage& image)
		{
			uint64_t r = 0, g = 0, b = 0;

			const auto height = image.height ();
			const auto width = image.width ();

			for (int y = 0; y < height; ++y)
			{
				const auto scanline = reinterpret_cast<const QRgb*> (image.scanLine (y));
				for (int x = 0; x < width; ++x)
				{
					auto color = scanline [x];
					r += qRed (color);
					g += qGreen (color);
					b += qBlue (color);
				}
			}
			return CombineGray (r, g, b);
		}

		void InvertRgbDefault (QImage& image)
		{
			const auto height = image.height ();
			const auto width = image.width ();

			for (int y = 0; y < height; ++y)
			{
				const auto scanline = reinterpret_cast<QRgb*> (image.scanLine (y));
				for (int x = 0; x < width; ++x)
					scanline [x] ^= 0x00ffffff;
			}
		}

		void ReduceLightnessInner (unsigned char *pixel, float recipFactor)
		{
			pixel [0] *= recipFactor;
			pixel [1] *= recipFactor;
			pixel [2] *= recipFactor;
		}

		void ReduceLightnessDefault (QImage& image, float factor)
		{
			const auto height = image.height ();
			const auto width = image.width ();

			for (int y = 0; y < height; ++y)
			{
				const auto scanline = image.scanLine (y);
				for (int x = 0; x < width; ++x)
					ReduceLightnessInner (&scanline [x * 4], 1 / factor);
			}
		}

#ifdef SSE_ENABLED
		template<int Alignment, typename F>
		void HandleLoopBegin (const uchar * const scanline, int width, int& x, int& bytesCount, F&& f)
		{
			const auto beginUnaligned = (scanline - static_cast<const uchar*> (nullptr)) % Alignment;
			bytesCount = width * 4;
			if (beginUnaligned)
			{
				x += Alignment - beginUnaligned;
				bytesCount -= Alignment - beginUnaligned;

				for (int i = 0; i < Alignment - beginUnaligned; i += 4)
					f (i);
			}

			bytesCount -= bytesCount % Alignment;
		}

		template<typename F>
		void HandleLoopEnd (int width, int x, F&& f)
		{
			for (int i = x; i < width * 4; i += 4)
				f (i);
		}

		__attribute__ ((target ("avx")))
		void InvertRgbAVX (QImage& image)
		{
			constexpr auto alignment = 32;

			const auto height = image.height ();
			const auto width = image.width ();

			const __m256 xorMask = _mm256_castsi256_ps (_mm256_set1_epi32 (0x00ffffff));

			for (int y = 0; y < height; ++y)
			{
				uchar * const scanline = image.scanLine (y);

				int x = 0;
				int bytesCount = 0;
				auto handler = [scanline] (int i)
				{
					*reinterpret_cast<QRgb*> (&scanline [i]) ^= 0x00ffffff;
				};
				constexpr auto unroll = 8;
				HandleLoopBegin<alignment * unroll> (scanline, width, x, bytesCount, handler);

				for (; x < bytesCount; x += alignment * unroll)
				{
					__m256i p0 = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x + alignment * 0));
					__m256i p1 = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x + alignment * 1));
					__m256i p2 = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x + alignment * 2));
					__m256i p3 = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x + alignment * 3));
					__m256i p4 = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x + alignment * 4));
					__m256i p5 = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x + alignment * 5));
					__m256i p6 = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x + alignment * 6));
					__m256i p7 = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x + alignment * 7));
					p0 = _mm256_castps_si256 (_mm256_xor_ps (_mm256_castsi256_ps (p0), xorMask));
					p1 = _mm256_castps_si256 (_mm256_xor_ps (_mm256_castsi256_ps (p1), xorMask));
					p2 = _mm256_castps_si256 (_mm256_xor_ps (_mm256_castsi256_ps (p2), xorMask));
					p3 = _mm256_castps_si256 (_mm256_xor_ps (_mm256_castsi256_ps (p3), xorMask));
					p4 = _mm256_castps_si256 (_mm256_xor_ps (_mm256_castsi256_ps (p4), xorMask));
					p5 = _mm256_castps_si256 (_mm256_xor_ps (_mm256_castsi256_ps (p5), xorMask));
					p6 = _mm256_castps_si256 (_mm256_xor_ps (_mm256_castsi256_ps (p6), xorMask));
					p7 = _mm256_castps_si256 (_mm256_xor_ps (_mm256_castsi256_ps (p7), xorMask));
					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x + alignment * 0), p0);
					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x + alignment * 1), p1);
					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x + alignment * 2), p2);
					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x + alignment * 3), p3);
					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x + alignment * 4), p4);
					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x + alignment * 5), p5);
					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x + alignment * 6), p6);
					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x + alignment * 7), p7);
				}

				HandleLoopEnd (width, x, handler);
			}
		}

		template<typename T, T... Fst, T... Snd>
		std::integer_sequence<T, Fst..., Snd...> ConcatImpl (std::integer_sequence<T, Fst...>, std::integer_sequence<T, Snd...>);

		template<typename... Seqs>
		struct ConcatS;

		template<typename... Seqs>
		using Concat = typename ConcatS<Seqs...>::type;

		template<typename Seq>
		struct ConcatS<Seq>
		{
			using type = Seq;
		};

		template<typename Seq1, typename Seq2, typename... Rest>
		struct ConcatS<Seq1, Seq2, Rest...>
		{
			using type = Concat<decltype (ConcatImpl (Seq1 {}, Seq2 {})), Rest...>;
		};

		template<typename T, T E, size_t C>
		struct RepeatS
		{
			template<T... Is>
			static auto RepeatImpl (std::integer_sequence<T, Is...>)
			{
				return std::integer_sequence<T, (static_cast<void> (Is), E)...> {};
			}

			using type = decltype (RepeatImpl (std::make_integer_sequence<T, C> {}));
		};

		template<typename T, T E, size_t C>
		using Repeat = typename RepeatS<T, E, C>::type;

		template<char From, char To>
		struct GenSeq;

		template<char From, char To>
		using EpiSeq = typename GenSeq<From, To>::type;

		template<char From, char To>
		struct GenSeq
		{
			using type = Concat<EpiSeq<From, From>, EpiSeq<From - 1, To>>;
		};

		template<char E>
		struct GenSeq<E, E>
		{
			using type = std::integer_sequence<uchar, 0x80, 0x80, 0x80, E>;
		};

		template<size_t BytesCount, size_t Bucket>
		struct GenRevSeqS
		{
			static constexpr uchar EndValue = BytesCount * 4 - 4;
			static constexpr auto TotalCount = BytesCount * 4;
			static constexpr auto BeforeEmpty = BytesCount * Bucket;
			static constexpr auto AfterEmpty = TotalCount - BytesCount - BeforeEmpty;

			static_assert (AfterEmpty >= 0, "negative sequel size");
			static_assert (BeforeEmpty >= 0, "negative prequel size");

			template<uchar... Is>
			static auto BytesImpl (std::integer_sequence<uchar, Is...>)
			{
				return std::integer_sequence<uchar, (EndValue - Is * 4)...> {};
			}

			using type = Concat<
					Repeat<uchar, 0x80, AfterEmpty>,
					decltype (BytesImpl (std::make_integer_sequence<uchar, BytesCount> {})),
					Repeat<uchar, 0x80, BeforeEmpty>
				>;
		};

		template<size_t BytesCount, size_t Bucket>
		using GenRevSeq = typename GenRevSeqS<BytesCount, Bucket>::type;

		template<uint16_t>
		struct Tag {};

		template<uchar... Is>
		auto MakeMaskImpl (Tag<128>, std::integer_sequence<uchar, Is...>)
		{
			return _mm_set_epi8 (Is...);
		}

		template<uchar... Is>
		__attribute__ ((target ("avx")))
		auto MakeMaskImpl (Tag<256>, std::integer_sequence<uchar, Is...>)
		{
			return _mm256_set_epi8 (Is..., Is...);
		}

		template<uint32_t Bits, char From, char To>
		auto MakeMask ()
		{
			return MakeMaskImpl (Tag<Bits> {}, EpiSeq<From, To> {});
		}

		template<uchar... Is>
		auto MakeRevMaskImpl (Tag<128>, std::integer_sequence<uchar, Is...>)
		{
			return _mm_set_epi8 (Is...);
		}

		template<uchar... Is>
		__attribute__ ((target ("avx")))
		auto MakeRevMaskImpl (Tag<256>, std::integer_sequence<uchar, Is...>)
		{
			return _mm256_set_epi8 (Is..., Is...);
		}

		template<uint32_t Bits, size_t BytesCount, size_t Bucket>
		auto MakeRevMask ()
		{
			return MakeRevMaskImpl (Tag<Bits> {}, GenRevSeq<BytesCount, Bucket> {});
		}

		__attribute__ ((target ("ssse3")))
		void ReduceLightnessSSSE3 (QImage& image, float factor)
		{
			constexpr auto alignment = 16;

			factor = 1 / factor;

			const auto height = image.height ();
			const auto width = image.width ();

			const __m128i pixel1msk = MakeMask<128, 3, 0> ();
			const __m128i pixel2msk = MakeMask<128, 7, 4> ();
			const __m128i pixel3msk = MakeMask<128, 11, 8> ();
			const __m128i pixel4msk = MakeMask<128, 15, 12> ();

			const __m128i pixel1revmask = MakeRevMask<128, 4, 0> ();
			const __m128i pixel2revmask = MakeRevMask<128, 4, 1> ();
			const __m128i pixel3revmask = MakeRevMask<128, 4, 2> ();
			const __m128i pixel4revmask = MakeRevMask<128, 4, 3> ();

			const __m128 divisor = _mm_set_ps (1, factor, factor, factor);

			for (int y = 0; y < height; ++y)
			{
				uchar * const scanline = image.scanLine (y);

				int x = 0;
				int bytesCount = 0;
				auto handler = [scanline, factor] (int i) { ReduceLightnessInner (&scanline [i], factor); };
				HandleLoopBegin<alignment> (scanline, width, x, bytesCount, handler);

				for (; x < bytesCount; x += alignment)
				{
					__m128i fourPixels = _mm_load_si128 (reinterpret_cast<const __m128i*> (scanline + x));

					__m128i px1 = _mm_shuffle_epi8 (fourPixels, pixel1msk);
					__m128i px2 = _mm_shuffle_epi8 (fourPixels, pixel2msk);
					__m128i px3 = _mm_shuffle_epi8 (fourPixels, pixel3msk);
					__m128i px4 = _mm_shuffle_epi8 (fourPixels, pixel4msk);

					px1 = _mm_cvtps_epi32 (_mm_mul_ps (_mm_cvtepi32_ps (px1), divisor));
					px2 = _mm_cvtps_epi32 (_mm_mul_ps (_mm_cvtepi32_ps (px2), divisor));
					px3 = _mm_cvtps_epi32 (_mm_mul_ps (_mm_cvtepi32_ps (px3), divisor));
					px4 = _mm_cvtps_epi32 (_mm_mul_ps (_mm_cvtepi32_ps (px4), divisor));

					px1 = _mm_shuffle_epi8 (px1, pixel1revmask);
					px2 = _mm_shuffle_epi8 (px2, pixel2revmask);
					px3 = _mm_shuffle_epi8 (px3, pixel3revmask);
					px4 = _mm_shuffle_epi8 (px4, pixel4revmask);

					fourPixels = _mm_add_epi32 (px1, px2);
					fourPixels = _mm_add_epi32 (fourPixels, px3);
					fourPixels = _mm_add_epi32 (fourPixels, px4);

					_mm_store_si128 (reinterpret_cast<__m128i*> (scanline + x), fourPixels);
				}

				HandleLoopEnd (width, x, handler);
			}
		}

		__attribute__((__always_inline__, __nodebug__, target("avx")))
		__m256i EmulMM256ShuffleEpi8 (__m256i reg, __m128i shuf)
		{
			__m128i reg0 = _mm256_castsi256_si128 (reg);
			__m128i reg1 = _mm256_extractf128_si256 (reg, 1);
			__m128i res0 = _mm_shuffle_epi8 (reg0, shuf);
			__m128i res1 = _mm_shuffle_epi8 (reg1, shuf);

			return _mm256_insertf128_si256 (_mm256_castsi128_si256 (res1), res0, 1);
		}

		__attribute__ ((target ("avx")))
		void ReduceLightnessAVX (QImage& image, float factor)
		{
			constexpr auto alignment = 32;

			factor = 1 / factor;

			const auto height = image.height ();
			const auto width = image.width ();

			const __m128i pixel1msk = MakeMask<128, 3, 0> ();
			const __m128i pixel2msk = MakeMask<128, 7, 4> ();
			const __m128i pixel3msk = MakeMask<128, 11, 8> ();
			const __m128i pixel4msk = MakeMask<128, 15, 12> ();

			const __m128i pixel1revmask = MakeRevMask<128, 4, 0> ();
			const __m128i pixel2revmask = MakeRevMask<128, 4, 1> ();
			const __m128i pixel3revmask = MakeRevMask<128, 4, 2> ();
			const __m128i pixel4revmask = MakeRevMask<128, 4, 3> ();

			const __m256 divisor = _mm256_set_ps (1, factor, factor, factor,
					1, factor, factor, factor);

			for (int y = 0; y < height; ++y)
			{
				uchar * const scanline = image.scanLine (y);

				int x = 0;
				int bytesCount = 0;
				auto handler = [scanline, factor] (int i) { ReduceLightnessInner (&scanline [i], factor); };
				HandleLoopBegin<alignment> (scanline, width, x, bytesCount, handler);

				for (; x < bytesCount; x += alignment)
				{
					__m256i eightPixels = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x));

					__m256i px1 = EmulMM256ShuffleEpi8 (eightPixels, pixel1msk);
					__m256i px2 = EmulMM256ShuffleEpi8 (eightPixels, pixel2msk);
					__m256i px3 = EmulMM256ShuffleEpi8 (eightPixels, pixel3msk);
					__m256i px4 = EmulMM256ShuffleEpi8 (eightPixels, pixel4msk);

					px1 = _mm256_cvtps_epi32 (_mm256_mul_ps (_mm256_cvtepi32_ps (px1), divisor));
					px2 = _mm256_cvtps_epi32 (_mm256_mul_ps (_mm256_cvtepi32_ps (px2), divisor));
					px3 = _mm256_cvtps_epi32 (_mm256_mul_ps (_mm256_cvtepi32_ps (px3), divisor));
					px4 = _mm256_cvtps_epi32 (_mm256_mul_ps (_mm256_cvtepi32_ps (px4), divisor));

					px1 = EmulMM256ShuffleEpi8 (px1, pixel1revmask);
					px2 = EmulMM256ShuffleEpi8 (px2, pixel2revmask);
					px3 = EmulMM256ShuffleEpi8 (px3, pixel3revmask);
					px4 = EmulMM256ShuffleEpi8 (px4, pixel4revmask);

					eightPixels = _mm256_castps_si256 (_mm256_or_ps (_mm256_castsi256_ps (px1), _mm256_castsi256_ps (px2)));
					eightPixels = _mm256_castps_si256 (_mm256_or_ps (_mm256_castsi256_ps (eightPixels), _mm256_castsi256_ps (px3)));
					eightPixels = _mm256_castps_si256 (_mm256_or_ps (_mm256_castsi256_ps (eightPixels), _mm256_castsi256_ps (px4)));

					_mm256_store_si256 (reinterpret_cast<__m256i*> (scanline + x), eightPixels);
				}

				HandleLoopEnd (width, x, handler);
			}
		}

		__attribute__ ((target ("sse4")))
		uint64_t GetGraySSE4 (const QImage& image)
		{
			uint32_t r = 0;
			uint32_t g = 0;
			uint32_t b = 0;

			__m128i sum = _mm_setzero_si128 ();

			const auto height = image.height ();
			const auto width = image.width ();

			const __m128i pixel1msk = MakeMask<128, 3, 0> ();
			const __m128i pixel2msk = MakeMask<128, 7, 4> ();
			const __m128i pixel3msk = MakeMask<128, 11, 8> ();
			const __m128i pixel4msk = MakeMask<128, 15, 12> ();

			constexpr auto alignment = 16;

			for (int y = 0; y < height; ++y)
			{
				const uchar * const scanline = image.scanLine (y);

				int x = 0;
				int bytesCount = 0;

				auto handler = [&r, &g, &b, scanline] (int i)
				{
					auto color = *reinterpret_cast<const QRgb*> (&scanline [i]);
					r += qRed (color);
					g += qGreen (color);
					b += qBlue (color);
				};
				HandleLoopBegin<alignment> (scanline, width, x, bytesCount, handler);

				#pragma unroll(8)
				for (; x < bytesCount; x += alignment)
				{
					const __m128i fourPixels = _mm_load_si128 (reinterpret_cast<const __m128i*> (scanline + x));

					sum = _mm_add_epi32 (sum, _mm_shuffle_epi8 (fourPixels, pixel1msk));
					sum = _mm_add_epi32 (sum, _mm_shuffle_epi8 (fourPixels, pixel2msk));
					sum = _mm_add_epi32 (sum, _mm_shuffle_epi8 (fourPixels, pixel3msk));
					sum = _mm_add_epi32 (sum, _mm_shuffle_epi8 (fourPixels, pixel4msk));
				}

				HandleLoopEnd (width, x, handler);
			}

			r += _mm_extract_epi32 (sum, 2);
			g += _mm_extract_epi32 (sum, 1);
			b += _mm_extract_epi32 (sum, 0);

			return CombineGray (r, g, b);
		}

		__attribute__ ((target ("avx2")))
		uint64_t GetGrayAVX2 (const QImage& image)
		{
			uint32_t r = 0;
			uint32_t g = 0;
			uint32_t b = 0;

			__m256i sum = _mm256_setzero_si256 ();

			const auto height = image.height ();
			const auto width = image.width ();

			const __m256i ppair1mask = MakeMask<256, 3, 0> ();
			const __m256i ppair2mask = MakeMask<256, 7, 4> ();
			const __m256i ppair3mask = MakeMask<256, 11, 8> ();
			const __m256i ppair4mask = MakeMask<256, 15, 12> ();

			constexpr auto alignment = 32;

			for (int y = 0; y < height; ++y)
			{
				const uchar * const scanline = image.scanLine (y);

				int x = 0;
				int bytesCount = 0;

				auto handler = [&r, &g, &b, scanline] (int i)
				{
					auto color = *reinterpret_cast<const QRgb*> (&scanline [i]);
					r += qRed (color);
					g += qGreen (color);
					b += qBlue (color);
				};
				HandleLoopBegin<alignment> (scanline, width, x, bytesCount, handler);

				for (; x < bytesCount; x += alignment)
				{
					const __m256i eightPixels = _mm256_load_si256 (reinterpret_cast<const __m256i*> (scanline + x));

					sum = _mm256_add_epi32 (sum, _mm256_shuffle_epi8 (eightPixels, ppair1mask));
					sum = _mm256_add_epi32 (sum, _mm256_shuffle_epi8 (eightPixels, ppair2mask));
					sum = _mm256_add_epi32 (sum, _mm256_shuffle_epi8 (eightPixels, ppair3mask));
					sum = _mm256_add_epi32 (sum, _mm256_shuffle_epi8 (eightPixels, ppair4mask));
				}

				HandleLoopEnd (width, x, handler);
			}

			r += _mm256_extract_epi32 (sum, 2);
			g += _mm256_extract_epi32 (sum, 1);
			b += _mm256_extract_epi32 (sum, 0);
			r += _mm256_extract_epi32 (sum, 6);
			g += _mm256_extract_epi32 (sum, 5);
			b += _mm256_extract_epi32 (sum, 4);

			return CombineGray (r, g, b);
		}
#endif

	}

	uint64_t GetGray (const QImage& image)
	{
#ifdef SSE_ENABLED
		static const auto ptr = Util::CpuFeatures::Choose ({
					{ Util::CpuFeatures::Feature::AVX2, &GetGrayAVX2 },
					{ Util::CpuFeatures::Feature::SSE41, &GetGraySSE4 }
				},
				&GetGrayDefault);

		return ptr (image);
#else
		return GetGrayDefault (image);
#endif
	}

	void InvertRgb (QImage& image)
	{
		InvertRgbDefault (image);
	}

	void ReduceLightness (QImage& image, float factor)
	{
		if (std::abs (factor - 1) < 1e-3)
			return;

#ifdef SSE_ENABLED
		static const auto ptr = Util::CpuFeatures::Choose ({
					{ Util::CpuFeatures::Feature::AVX, &ReduceLightnessAVX },
					{ Util::CpuFeatures::Feature::SSSE3, &ReduceLightnessSSSE3 }
				},
				&ReduceLightnessDefault);

		ptr (image, factor);
#else
		ReduceLightnessDefault (image, factor);
#endif
	}
}
}
}
