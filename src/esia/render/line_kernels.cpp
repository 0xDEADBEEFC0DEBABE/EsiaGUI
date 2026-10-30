// Esia - a polyline's loops (line_kernels.hpp) on every level of esia::cpu::Simd. Every level does the same IEEE
// operations in the same order - products, sums, exact square roots and divisions, min / max, sign flips - and none
// fused (contraction is off for this file and the vector code names every operation), so they all write the plain C++'s
// bits (test_painter: PolylineKernelsAgree). A level's loop hands what is left to the next narrower one, down to the plain
// C++, which gives every element the same bits.
#include "line_kernels.hpp"
#include "esia/core/cpu.hpp"
#include <cmath>

#if defined(_M_X64) || defined(__x86_64__)
#define ESIA_LINES_X86 1
#include <immintrin.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ESIA_LINES_NEON 1
#include <arm_neon.h>
#endif

#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif
// (GCC: -ffp-contract=off for this file, src/esia/CMakeLists.txt)

#if defined(ESIA_LINES_X86) && (defined(__GNUC__) || defined(__clang__))
#define ESIA_AVX2 __attribute__((target("avx2")))
#define ESIA_AVX512 __attribute__((target("avx2,avx512f,avx512vl,avx512bw,avx512dq")))
#else
#define ESIA_AVX2
#define ESIA_AVX512
#endif

namespace esia::render::lines
{
    namespace
    {
        constexpr float kTiny = 1e-8f;   // BuildPath's repeat, squared

        // ------------------------------------------------------------------ plain C++, and the vector loops' tails
        bool NormalOne(Vec2 a, Vec2 b, Vec2& normal, float& extent)
        {
            const float dx = b.x - a.x;
            const float dy = b.y - a.y;
            const float xx = dx * dx;
            const float yy = dy * dy;
            const float l2 = xx + yy;
            if (!(l2 > kTiny))
                return false;
            const float root = std::sqrt(l2);
            const float inv = 1.0f / root;
            normal = Vec2(-dy * inv, dx * inv);
            const float ax = std::fabs(dx), ay = std::fabs(dy);
            extent = ax > ay ? ax : ay;
            return true;
        }

        void Bound(Vec2 p, Vec2& lo, Vec2& hi)
        {
            lo = Vec2(p.x < lo.x ? p.x : lo.x, p.y < lo.y ? p.y : lo.y);
            hi = Vec2(p.x > hi.x ? p.x : hi.x, p.y > hi.y ? p.y : hi.y);
        }

        bool NormalsScalar(const Vec2* path, int n, int from, Vec2* normal, Vec2& lo, Vec2& hi, float& shortest)
        {
            for (int i = from; i + 1 < n; ++i)
            {
                float extent;
                if (!NormalOne(path[i], path[i + 1], normal[i], extent))
                    return false;
                shortest = extent < shortest ? extent : shortest;
                Bound(path[i], lo, hi);
            }
            return true;
        }

        bool JointsScalar(const Vec2* normal, int n, int from, float* code, float& lowest)
        {
            for (int i = from; i + 1 < n; ++i)
            {
                const Vec2 a = normal[i - 1], b = normal[i];
                const float xx = a.x * b.x;
                const float yy = a.y * b.y;
                const float dot = xx + yy;
                const float h = 0.5f * (1.0f + dot);
                if (!(h >= 0.25f))
                    return false;
                lowest = h < lowest ? h : lowest;
                code[i] = QuadInstance::LineDirectionCode(Vec2(a.x + b.x, a.y + b.y));
            }
            return true;
        }

        // a joint's line as a unit vector: the normal at an end, the normals' bisector between
        Vec2 Miter(const Vec2* normal, int n, int j)
        {
            if (j == 0 || j == n - 1)
                return normal[j == 0 ? 0 : n - 2];
            const Vec2 a = normal[j - 1], b = normal[j];
            const float xx = a.x * b.x;
            const float yy = a.y * b.y;
            const float dot = xx + yy;
            const float h = 0.5f * (1.0f + dot);
            const float k = 0.5f / std::sqrt(h);
            return Vec2((a.x + b.x) * k, (a.y + b.y) * k);
        }

        // a piece clear of its joints' lines (line_kernels.hpp, Clear)
        bool ClearOne(const Vec2* path, const Vec2* normal, int n, int i, float r, float clear)
        {
            const float dx = path[i + 1].x - path[i].x;
            const float dy = path[i + 1].y - path[i].y;
            const float ax = std::fabs(dx), ay = std::fabs(dy);
            if ((ax > ay ? ax : ay) > clear)
                return true;
            const Vec2 u0 = Miter(normal, n, i), u1 = Miter(normal, n, i + 1);
            const float t0 = u0.x * u1.y;
            const float t1 = u0.y * u1.x;
            const float turn = std::fabs(t0 - t1) * r;
            const float taxi = ax + ay;
            const float margin = (taxi + r) * 1e-3f;
            const float p0 = u0.x * dy;
            const float q0 = u0.y * dx;
            const float p1 = u1.x * dy;
            const float q1 = u1.y * dx;
            return std::fabs(p0 - q0) - turn > margin && std::fabs(p1 - q1) - turn > margin;
        }

        bool ClearScalar(const Vec2* path, const Vec2* normal, int n, int from, int to, float r, float clear)
        {
            for (int i = from; i < to; ++i)
                if (!ClearOne(path, normal, n, i, r, clear))
                    return false;
            return true;
        }

        void PiecesScalar(const Vec2* path, int n, int from, const float* code, float head, float hw, float rgb, float alpha, QuadInstance* quad)
        {
            for (int i = from; i + 1 < n; ++i)
            {
                QuadInstance& q = quad[i];
                q.rect[0] = path[i].x;
                q.rect[1] = path[i].y;
                q.rect[2] = path[i + 1].x;
                q.rect[3] = path[i + 1].y;
                const float c0 = 256.0f * code[i];
                const float c1 = 256.0f * code[i + 1];
                q.packed[0] = head - c0;
                q.packed[1] = hw;
                q.packed[2] = rgb;
                q.packed[3] = alpha + c1;
            }
        }

#if defined(ESIA_LINES_X86) || defined(ESIA_LINES_NEON)
        constexpr float kMagic = 12582912.0f;   // 1.5 2^23 (QuadInstance::LineDirectionCode's rounding)

#endif

#if defined(ESIA_LINES_X86)
        // ------------------------------------------------------------------ SSE2: 4 floats (every x86-64)
        // a vector's smallest and largest lane, as Bound takes them (min / max: the same value in any order), by shuffles
        float Min4(__m128 v)
        {
            v = _mm_min_ps(v, _mm_movehl_ps(v, v));
            return _mm_cvtss_f32(_mm_min_ss(v, _mm_shuffle_ps(v, v, 1)));
        }

        float Max4(__m128 v)
        {
            v = _mm_max_ps(v, _mm_movehl_ps(v, v));
            return _mm_cvtss_f32(_mm_max_ss(v, _mm_shuffle_ps(v, v, 1)));
        }

        void FoldSse(__m128 lox, __m128 loy, __m128 hix, __m128 hiy, __m128 sh, Vec2& lo, Vec2& hi, float& shortest)
        {
            Bound(Vec2(Min4(lox), Min4(loy)), lo, hi);
            Bound(Vec2(Max4(hix), Max4(hiy)), lo, hi);
            const float s = Min4(sh);
            shortest = s < shortest ? s : shortest;
        }

        int NormalsSse2(const Vec2* path, int n, int i, Vec2* normal, Vec2& lo, Vec2& hi, float& shortest)
        {
            const float* f = &path[0].x;
            const __m128 sign = _mm_set1_ps(-0.0f), tiny = _mm_set1_ps(kTiny), one = _mm_set1_ps(1.0f);
            __m128 lox = _mm_set1_ps(lo.x), loy = _mm_set1_ps(lo.y), hix = _mm_set1_ps(hi.x), hiy = _mm_set1_ps(hi.y), sh = _mm_set1_ps(shortest);
            for (; i + 4 < n; i += 4)   // segments i .. i + 3
            {
                const __m128 a0 = _mm_loadu_ps(f + 2 * i), a1 = _mm_loadu_ps(f + 2 * i + 4);
                const __m128 b0 = _mm_loadu_ps(f + 2 * i + 2), b1 = _mm_loadu_ps(f + 2 * i + 6);
                const __m128 ax = _mm_shuffle_ps(a0, a1, _MM_SHUFFLE(2, 0, 2, 0)), ay = _mm_shuffle_ps(a0, a1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m128 bx = _mm_shuffle_ps(b0, b1, _MM_SHUFFLE(2, 0, 2, 0)), by = _mm_shuffle_ps(b0, b1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m128 dx = _mm_sub_ps(bx, ax), dy = _mm_sub_ps(by, ay);
                const __m128 l2 = _mm_add_ps(_mm_mul_ps(dx, dx), _mm_mul_ps(dy, dy));
                if (_mm_movemask_ps(_mm_cmpgt_ps(l2, tiny)) != 0xF)
                    return -1;
                const __m128 inv = _mm_div_ps(one, _mm_sqrt_ps(l2));
                const __m128 nx = _mm_mul_ps(_mm_xor_ps(dy, sign), inv), ny = _mm_mul_ps(dx, inv);
                _mm_storeu_ps(&normal[i].x, _mm_unpacklo_ps(nx, ny));
                _mm_storeu_ps(&normal[i + 2].x, _mm_unpackhi_ps(nx, ny));
                sh = _mm_min_ps(_mm_max_ps(_mm_andnot_ps(sign, dx), _mm_andnot_ps(sign, dy)), sh);
                lox = _mm_min_ps(ax, lox);
                loy = _mm_min_ps(ay, loy);
                hix = _mm_max_ps(ax, hix);
                hiy = _mm_max_ps(ay, hiy);
            }
            FoldSse(lox, loy, hix, hiy, sh, lo, hi, shortest);
            return i;
        }

        int JointsSse2(const Vec2* normal, int n, int i, float* code, float& lowest)
        {
            const float* f = &normal[0].x;
            const __m128 sign = _mm_set1_ps(-0.0f), zero = _mm_setzero_ps(), one = _mm_set1_ps(1.0f), half = _mm_set1_ps(0.5f);
            const __m128 quarter = _mm_set1_ps(0.25f), least = _mm_set1_ps(1e-30f), steps = _mm_set1_ps(32768.0f);
            const __m128 magic = _mm_set1_ps(kMagic), wrap = _mm_set1_ps(65536.0f);
            __m128 low = _mm_set1_ps(lowest);
            for (; i + 4 < n; i += 4)   // joints i .. i + 3: normals i - 1 .. i + 3
            {
                const __m128 a0 = _mm_loadu_ps(f + 2 * i - 2), a1 = _mm_loadu_ps(f + 2 * i + 2);
                const __m128 b0 = _mm_loadu_ps(f + 2 * i), b1 = _mm_loadu_ps(f + 2 * i + 4);
                const __m128 ax = _mm_shuffle_ps(a0, a1, _MM_SHUFFLE(2, 0, 2, 0)), ay = _mm_shuffle_ps(a0, a1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m128 bx = _mm_shuffle_ps(b0, b1, _MM_SHUFFLE(2, 0, 2, 0)), by = _mm_shuffle_ps(b0, b1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m128 dot = _mm_add_ps(_mm_mul_ps(ax, bx), _mm_mul_ps(ay, by));
                const __m128 h = _mm_mul_ps(half, _mm_add_ps(one, dot));
                if (_mm_movemask_ps(_mm_cmpge_ps(h, quarter)) != 0xF)
                    return -1;
                low = _mm_min_ps(h, low);
                const __m128 wx = _mm_add_ps(ax, bx), wy = _mm_add_ps(ay, by);
                const __m128 s = _mm_add_ps(_mm_andnot_ps(sign, wx), _mm_andnot_ps(sign, wy));
                const __m128 p = _mm_div_ps(wx, _mm_max_ps(s, least));
                const __m128 t = _mm_sub_ps(one, _mm_xor_ps(p, _mm_and_ps(_mm_cmplt_ps(wy, zero), sign)));
                const __m128 r = _mm_sub_ps(_mm_add_ps(_mm_mul_ps(t, steps), magic), magic);
                _mm_storeu_ps(code + i, _mm_sub_ps(r, _mm_and_ps(_mm_cmpge_ps(r, wrap), wrap)));
            }
            const float least4 = Min4(low);
            lowest = least4 < lowest ? least4 : lowest;
            return i;
        }

        int PiecesSse2(const Vec2* path, int n, int i, const float* code, float head, float hw, float rgb, float alpha, QuadInstance* quad)
        {
            const float* f = &path[0].x;
            const __m128 headV = _mm_set1_ps(head), hwV = _mm_set1_ps(hw), rgbV = _mm_set1_ps(rgb), alphaV = _mm_set1_ps(alpha), k256 = _mm_set1_ps(256.0f);
            for (; i + 4 < n; i += 4)   // pieces i .. i + 3
            {
                const __m128 p0 = _mm_sub_ps(headV, _mm_mul_ps(k256, _mm_loadu_ps(code + i)));
                const __m128 p3 = _mm_add_ps(alphaV, _mm_mul_ps(k256, _mm_loadu_ps(code + i + 1)));
                const __m128 lo01 = _mm_unpacklo_ps(p0, hwV), hi01 = _mm_unpacklo_ps(rgbV, p3);   // p0 hw p0 hw, rgb p3 rgb p3
                const __m128 lo23 = _mm_unpackhi_ps(p0, hwV), hi23 = _mm_unpackhi_ps(rgbV, p3);
                const __m128 packed[4] = {_mm_movelh_ps(lo01, hi01), _mm_movehl_ps(hi01, lo01), _mm_movelh_ps(lo23, hi23), _mm_movehl_ps(hi23, lo23)};
                for (int k = 0; k < 4; ++k)
                {
                    float* q = quad[i + k].rect;
                    _mm_storeu_ps(q, _mm_loadu_ps(f + 2 * (i + k)));   // the piece's two points
                    _mm_storeu_ps(q + 4, packed[k]);
                }
            }
            return i;
        }

        // pieces i .. i + 3 of the inside (both joints between pieces): normals i - 1 .. i + 4, points i .. i + 4
        int ClearSse2(const Vec2* path, const Vec2* normal, int n, int i, float r, float clear)
        {
            const float* f = &path[0].x;
            const float* m = &normal[0].x;
            const __m128 sign = _mm_set1_ps(-0.0f), one = _mm_set1_ps(1.0f), half = _mm_set1_ps(0.5f), rV = _mm_set1_ps(r);
            const __m128 clearV = _mm_set1_ps(clear), milli = _mm_set1_ps(1e-3f);
            for (; i + 6 < n; i += 4)   // the last piece i + 3 <= n - 3
            {
#define ESIA_DEINTERLEAVE4(p, x, y)                                                                                                        \
    const __m128 x##0 = _mm_loadu_ps(p), x##1 = _mm_loadu_ps((p) + 4);                                                                     \
    const __m128 x = _mm_shuffle_ps(x##0, x##1, _MM_SHUFFLE(2, 0, 2, 0)), y = _mm_shuffle_ps(x##0, x##1, _MM_SHUFFLE(3, 1, 3, 1));
                ESIA_DEINTERLEAVE4(m + 2 * i - 2, ax, ay)
                ESIA_DEINTERLEAVE4(m + 2 * i, bx, by)
                ESIA_DEINTERLEAVE4(m + 2 * i + 2, cx, cy)
                ESIA_DEINTERLEAVE4(f + 2 * i, px, py)
                ESIA_DEINTERLEAVE4(f + 2 * i + 2, qx, qy)
#undef ESIA_DEINTERLEAVE4
                const __m128 k0 = _mm_div_ps(half, _mm_sqrt_ps(_mm_mul_ps(half, _mm_add_ps(one, _mm_add_ps(_mm_mul_ps(ax, bx), _mm_mul_ps(ay, by))))));
                const __m128 k1 = _mm_div_ps(half, _mm_sqrt_ps(_mm_mul_ps(half, _mm_add_ps(one, _mm_add_ps(_mm_mul_ps(bx, cx), _mm_mul_ps(by, cy))))));
                const __m128 u0x = _mm_mul_ps(_mm_add_ps(ax, bx), k0), u0y = _mm_mul_ps(_mm_add_ps(ay, by), k0);
                const __m128 u1x = _mm_mul_ps(_mm_add_ps(bx, cx), k1), u1y = _mm_mul_ps(_mm_add_ps(by, cy), k1);
                const __m128 dx = _mm_sub_ps(qx, px), dy = _mm_sub_ps(qy, py);
                const __m128 adx = _mm_andnot_ps(sign, dx), ady = _mm_andnot_ps(sign, dy);
                const __m128 turn = _mm_mul_ps(_mm_andnot_ps(sign, _mm_sub_ps(_mm_mul_ps(u0x, u1y), _mm_mul_ps(u0y, u1x))), rV);
                const __m128 margin = _mm_mul_ps(_mm_add_ps(_mm_add_ps(adx, ady), rV), milli);
                const __m128 c0 = _mm_sub_ps(_mm_andnot_ps(sign, _mm_sub_ps(_mm_mul_ps(u0x, dy), _mm_mul_ps(u0y, dx))), turn);
                const __m128 c1 = _mm_sub_ps(_mm_andnot_ps(sign, _mm_sub_ps(_mm_mul_ps(u1x, dy), _mm_mul_ps(u1y, dx))), turn);
                const __m128 ok = _mm_or_ps(_mm_cmpgt_ps(_mm_max_ps(adx, ady), clearV), _mm_and_ps(_mm_cmpgt_ps(c0, margin), _mm_cmpgt_ps(c1, margin)));
                if (_mm_movemask_ps(ok) != 0xF)
                    return -1;
            }
            return i;
        }

        // ------------------------------------------------------------------ AVX2: 8 floats
        // (Within each 128-bit half the shuffles deinterleave like SSE2's: the lanes hold elements 0 1 4 5 | 2 3 6 7, which
        // unpacking interleaves back in order; a joint's codes are put back in order by a permute.)
        ESIA_AVX2 int NormalsAvx2(const Vec2* path, int n, int i, Vec2* normal, Vec2& lo, Vec2& hi, float& shortest)
        {
            const float* f = &path[0].x;
            const __m256 sign = _mm256_set1_ps(-0.0f), tiny = _mm256_set1_ps(kTiny), one = _mm256_set1_ps(1.0f);
            __m256 lox = _mm256_set1_ps(lo.x), loy = _mm256_set1_ps(lo.y), hix = _mm256_set1_ps(hi.x), hiy = _mm256_set1_ps(hi.y);
            __m256 sh = _mm256_set1_ps(shortest);
            for (; i + 8 < n; i += 8)   // segments i .. i + 7
            {
                const __m256 a0 = _mm256_loadu_ps(f + 2 * i), a1 = _mm256_loadu_ps(f + 2 * i + 8);
                const __m256 b0 = _mm256_loadu_ps(f + 2 * i + 2), b1 = _mm256_loadu_ps(f + 2 * i + 10);
                const __m256 ax = _mm256_shuffle_ps(a0, a1, _MM_SHUFFLE(2, 0, 2, 0)), ay = _mm256_shuffle_ps(a0, a1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m256 bx = _mm256_shuffle_ps(b0, b1, _MM_SHUFFLE(2, 0, 2, 0)), by = _mm256_shuffle_ps(b0, b1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m256 dx = _mm256_sub_ps(bx, ax), dy = _mm256_sub_ps(by, ay);
                const __m256 l2 = _mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_mul_ps(dy, dy));
                if (_mm256_movemask_ps(_mm256_cmp_ps(l2, tiny, _CMP_GT_OQ)) != 0xFF)
                    return -1;
                const __m256 inv = _mm256_div_ps(one, _mm256_sqrt_ps(l2));
                const __m256 nx = _mm256_mul_ps(_mm256_xor_ps(dy, sign), inv), ny = _mm256_mul_ps(dx, inv);
                _mm256_storeu_ps(&normal[i].x, _mm256_unpacklo_ps(nx, ny));
                _mm256_storeu_ps(&normal[i + 4].x, _mm256_unpackhi_ps(nx, ny));
                sh = _mm256_min_ps(_mm256_max_ps(_mm256_andnot_ps(sign, dx), _mm256_andnot_ps(sign, dy)), sh);
                lox = _mm256_min_ps(ax, lox);
                loy = _mm256_min_ps(ay, loy);
                hix = _mm256_max_ps(ax, hix);
                hiy = _mm256_max_ps(ay, hiy);
            }
            FoldSse(_mm_min_ps(_mm256_castps256_ps128(lox), _mm256_extractf128_ps(lox, 1)), _mm_min_ps(_mm256_castps256_ps128(loy), _mm256_extractf128_ps(loy, 1)),
                    _mm_max_ps(_mm256_castps256_ps128(hix), _mm256_extractf128_ps(hix, 1)), _mm_max_ps(_mm256_castps256_ps128(hiy), _mm256_extractf128_ps(hiy, 1)),
                    _mm_min_ps(_mm256_castps256_ps128(sh), _mm256_extractf128_ps(sh, 1)), lo, hi, shortest);
            return i;
        }

        ESIA_AVX2 int JointsAvx2(const Vec2* normal, int n, int i, float* code, float& lowest)
        {
            const float* f = &normal[0].x;
            const __m256 sign = _mm256_set1_ps(-0.0f), zero = _mm256_setzero_ps(), one = _mm256_set1_ps(1.0f), half = _mm256_set1_ps(0.5f);
            const __m256 quarter = _mm256_set1_ps(0.25f), least = _mm256_set1_ps(1e-30f), steps = _mm256_set1_ps(32768.0f);
            const __m256 magic = _mm256_set1_ps(kMagic), wrap = _mm256_set1_ps(65536.0f);
            __m256 low = _mm256_set1_ps(lowest);
            for (; i + 8 < n; i += 8)   // joints i .. i + 7: normals i - 1 .. i + 7
            {
                const __m256 a0 = _mm256_loadu_ps(f + 2 * i - 2), a1 = _mm256_loadu_ps(f + 2 * i + 6);
                const __m256 b0 = _mm256_loadu_ps(f + 2 * i), b1 = _mm256_loadu_ps(f + 2 * i + 8);
                const __m256 ax = _mm256_shuffle_ps(a0, a1, _MM_SHUFFLE(2, 0, 2, 0)), ay = _mm256_shuffle_ps(a0, a1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m256 bx = _mm256_shuffle_ps(b0, b1, _MM_SHUFFLE(2, 0, 2, 0)), by = _mm256_shuffle_ps(b0, b1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m256 dot = _mm256_add_ps(_mm256_mul_ps(ax, bx), _mm256_mul_ps(ay, by));
                const __m256 h = _mm256_mul_ps(half, _mm256_add_ps(one, dot));
                if (_mm256_movemask_ps(_mm256_cmp_ps(h, quarter, _CMP_GE_OQ)) != 0xFF)
                    return -1;
                low = _mm256_min_ps(h, low);
                const __m256 wx = _mm256_add_ps(ax, bx), wy = _mm256_add_ps(ay, by);
                const __m256 s = _mm256_add_ps(_mm256_andnot_ps(sign, wx), _mm256_andnot_ps(sign, wy));
                const __m256 p = _mm256_div_ps(wx, _mm256_max_ps(s, least));
                const __m256 t = _mm256_sub_ps(one, _mm256_xor_ps(p, _mm256_and_ps(_mm256_cmp_ps(wy, zero, _CMP_LT_OQ), sign)));
                const __m256 r = _mm256_sub_ps(_mm256_add_ps(_mm256_mul_ps(t, steps), magic), magic);
                const __m256 c = _mm256_sub_ps(r, _mm256_and_ps(_mm256_cmp_ps(r, wrap, _CMP_GE_OQ), wrap));
                // lanes 0 1 4 5 | 2 3 6 7 -> 0 .. 7
                _mm256_storeu_ps(code + i, _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(c), _MM_SHUFFLE(3, 1, 2, 0))));
            }
            const float least8 = Min4(_mm_min_ps(_mm256_castps256_ps128(low), _mm256_extractf128_ps(low, 1)));
            lowest = least8 < lowest ? least8 : lowest;
            return i;
        }

        ESIA_AVX2 int PiecesAvx2(const Vec2* path, int n, int i, const float* code, float head, float hw, float rgb, float alpha, QuadInstance* quad)
        {
            const float* f = &path[0].x;
            const __m256 headV = _mm256_set1_ps(head), hwV = _mm256_set1_ps(hw), rgbV = _mm256_set1_ps(rgb), alphaV = _mm256_set1_ps(alpha);
            const __m256 k256 = _mm256_set1_ps(256.0f);
            for (; i + 8 < n; i += 8)   // pieces i .. i + 7
            {
                const __m256 p0 = _mm256_sub_ps(headV, _mm256_mul_ps(k256, _mm256_loadu_ps(code + i)));
                const __m256 p3 = _mm256_add_ps(alphaV, _mm256_mul_ps(k256, _mm256_loadu_ps(code + i + 1)));
                const __m256 lo = _mm256_unpacklo_ps(p0, hwV), hi = _mm256_unpacklo_ps(rgbV, p3);   // pieces 0 1 | 4 5
                const __m256 lo2 = _mm256_unpackhi_ps(p0, hwV), hi2 = _mm256_unpackhi_ps(rgbV, p3);   // pieces 2 3 | 6 7
                const __m256 q04 = _mm256_shuffle_ps(lo, hi, _MM_SHUFFLE(1, 0, 1, 0)), q15 = _mm256_shuffle_ps(lo, hi, _MM_SHUFFLE(3, 2, 3, 2));
                const __m256 q26 = _mm256_shuffle_ps(lo2, hi2, _MM_SHUFFLE(1, 0, 1, 0)), q37 = _mm256_shuffle_ps(lo2, hi2, _MM_SHUFFLE(3, 2, 3, 2));
                const __m128 packed[8] = {_mm256_castps256_ps128(q04), _mm256_castps256_ps128(q15), _mm256_castps256_ps128(q26),
                                          _mm256_castps256_ps128(q37), _mm256_extractf128_ps(q04, 1), _mm256_extractf128_ps(q15, 1),
                                          _mm256_extractf128_ps(q26, 1), _mm256_extractf128_ps(q37, 1)};
                for (int k = 0; k < 8; ++k)
                    _mm256_storeu_ps(quad[i + k].rect, _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_loadu_ps(f + 2 * (i + k))), packed[k], 1));
            }
            return i;
        }

        ESIA_AVX2 int ClearAvx2(const Vec2* path, const Vec2* normal, int n, int i, float r, float clear)
        {
            const float* f = &path[0].x;
            const float* m = &normal[0].x;
            const __m256 sign = _mm256_set1_ps(-0.0f), one = _mm256_set1_ps(1.0f), half = _mm256_set1_ps(0.5f), rV = _mm256_set1_ps(r);
            const __m256 clearV = _mm256_set1_ps(clear), milli = _mm256_set1_ps(1e-3f);
            for (; i + 10 < n; i += 8)   // the last piece i + 7 <= n - 3
            {
#define ESIA_DEINTERLEAVE8(p, x, y)                                                                                                        \
    const __m256 x##0 = _mm256_loadu_ps(p), x##1 = _mm256_loadu_ps((p) + 8);                                                               \
    const __m256 x = _mm256_shuffle_ps(x##0, x##1, _MM_SHUFFLE(2, 0, 2, 0)), y = _mm256_shuffle_ps(x##0, x##1, _MM_SHUFFLE(3, 1, 3, 1));
                ESIA_DEINTERLEAVE8(m + 2 * i - 2, ax, ay)
                ESIA_DEINTERLEAVE8(m + 2 * i, bx, by)
                ESIA_DEINTERLEAVE8(m + 2 * i + 2, cx, cy)
                ESIA_DEINTERLEAVE8(f + 2 * i, px, py)
                ESIA_DEINTERLEAVE8(f + 2 * i + 2, qx, qy)
#undef ESIA_DEINTERLEAVE8
                const __m256 k0 = _mm256_div_ps(half, _mm256_sqrt_ps(_mm256_mul_ps(half, _mm256_add_ps(one, _mm256_add_ps(_mm256_mul_ps(ax, bx), _mm256_mul_ps(ay, by))))));
                const __m256 k1 = _mm256_div_ps(half, _mm256_sqrt_ps(_mm256_mul_ps(half, _mm256_add_ps(one, _mm256_add_ps(_mm256_mul_ps(bx, cx), _mm256_mul_ps(by, cy))))));
                const __m256 u0x = _mm256_mul_ps(_mm256_add_ps(ax, bx), k0), u0y = _mm256_mul_ps(_mm256_add_ps(ay, by), k0);
                const __m256 u1x = _mm256_mul_ps(_mm256_add_ps(bx, cx), k1), u1y = _mm256_mul_ps(_mm256_add_ps(by, cy), k1);
                const __m256 dx = _mm256_sub_ps(qx, px), dy = _mm256_sub_ps(qy, py);
                const __m256 adx = _mm256_andnot_ps(sign, dx), ady = _mm256_andnot_ps(sign, dy);
                const __m256 turn = _mm256_mul_ps(_mm256_andnot_ps(sign, _mm256_sub_ps(_mm256_mul_ps(u0x, u1y), _mm256_mul_ps(u0y, u1x))), rV);
                const __m256 margin = _mm256_mul_ps(_mm256_add_ps(_mm256_add_ps(adx, ady), rV), milli);
                const __m256 c0 = _mm256_sub_ps(_mm256_andnot_ps(sign, _mm256_sub_ps(_mm256_mul_ps(u0x, dy), _mm256_mul_ps(u0y, dx))), turn);
                const __m256 c1 = _mm256_sub_ps(_mm256_andnot_ps(sign, _mm256_sub_ps(_mm256_mul_ps(u1x, dy), _mm256_mul_ps(u1y, dx))), turn);
                const __m256 ok = _mm256_or_ps(_mm256_cmp_ps(_mm256_max_ps(adx, ady), clearV, _CMP_GT_OQ),
                                               _mm256_and_ps(_mm256_cmp_ps(c0, margin, _CMP_GT_OQ), _mm256_cmp_ps(c1, margin, _CMP_GT_OQ)));
                if (_mm256_movemask_ps(ok) != 0xFF)
                    return -1;
            }
            return i;
        }

        // ------------------------------------------------------------------ AVX-512: 16 floats
        // (each 128-bit quarter k holds elements 2k, 2k + 1, 8 + 2k, 9 + 2k; unpacking interleaves them back in order)
        ESIA_AVX512 int NormalsAvx512(const Vec2* path, int n, int i, Vec2* normal, Vec2& lo, Vec2& hi, float& shortest)
        {
            const float* f = &path[0].x;
            const __m512 sign = _mm512_set1_ps(-0.0f), tiny = _mm512_set1_ps(kTiny), one = _mm512_set1_ps(1.0f);
            __m512 lox = _mm512_set1_ps(lo.x), loy = _mm512_set1_ps(lo.y), hix = _mm512_set1_ps(hi.x), hiy = _mm512_set1_ps(hi.y);
            __m512 sh = _mm512_set1_ps(shortest);
            for (; i + 16 < n; i += 16)   // segments i .. i + 15
            {
                const __m512 a0 = _mm512_loadu_ps(f + 2 * i), a1 = _mm512_loadu_ps(f + 2 * i + 16);
                const __m512 b0 = _mm512_loadu_ps(f + 2 * i + 2), b1 = _mm512_loadu_ps(f + 2 * i + 18);
                const __m512 ax = _mm512_shuffle_ps(a0, a1, _MM_SHUFFLE(2, 0, 2, 0)), ay = _mm512_shuffle_ps(a0, a1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m512 bx = _mm512_shuffle_ps(b0, b1, _MM_SHUFFLE(2, 0, 2, 0)), by = _mm512_shuffle_ps(b0, b1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m512 dx = _mm512_sub_ps(bx, ax), dy = _mm512_sub_ps(by, ay);
                const __m512 l2 = _mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy));
                if (_mm512_cmp_ps_mask(l2, tiny, _CMP_GT_OQ) != 0xFFFF)
                    return -1;
                const __m512 inv = _mm512_div_ps(one, _mm512_sqrt_ps(l2));
                const __m512 nx = _mm512_mul_ps(_mm512_xor_ps(dy, sign), inv), ny = _mm512_mul_ps(dx, inv);
                _mm512_storeu_ps(&normal[i].x, _mm512_unpacklo_ps(nx, ny));
                _mm512_storeu_ps(&normal[i + 8].x, _mm512_unpackhi_ps(nx, ny));
                sh = _mm512_min_ps(_mm512_max_ps(_mm512_andnot_ps(sign, dx), _mm512_andnot_ps(sign, dy)), sh);
                lox = _mm512_min_ps(ax, lox);
                loy = _mm512_min_ps(ay, loy);
                hix = _mm512_max_ps(ax, hix);
                hiy = _mm512_max_ps(ay, hiy);
            }
            const __m256 lx = _mm256_min_ps(_mm512_castps512_ps256(lox), _mm512_extractf32x8_ps(lox, 1));
            const __m256 ly = _mm256_min_ps(_mm512_castps512_ps256(loy), _mm512_extractf32x8_ps(loy, 1));
            const __m256 hx = _mm256_max_ps(_mm512_castps512_ps256(hix), _mm512_extractf32x8_ps(hix, 1));
            const __m256 hy = _mm256_max_ps(_mm512_castps512_ps256(hiy), _mm512_extractf32x8_ps(hiy, 1));
            const __m256 s8 = _mm256_min_ps(_mm512_castps512_ps256(sh), _mm512_extractf32x8_ps(sh, 1));
            FoldSse(_mm_min_ps(_mm256_castps256_ps128(lx), _mm256_extractf128_ps(lx, 1)), _mm_min_ps(_mm256_castps256_ps128(ly), _mm256_extractf128_ps(ly, 1)),
                    _mm_max_ps(_mm256_castps256_ps128(hx), _mm256_extractf128_ps(hx, 1)), _mm_max_ps(_mm256_castps256_ps128(hy), _mm256_extractf128_ps(hy, 1)),
                    _mm_min_ps(_mm256_castps256_ps128(s8), _mm256_extractf128_ps(s8, 1)), lo, hi, shortest);
            return i;
        }

        ESIA_AVX512 int JointsAvx512(const Vec2* normal, int n, int i, float* code, float& lowest)
        {
            const float* f = &normal[0].x;
            const __m512 sign = _mm512_set1_ps(-0.0f), zero = _mm512_setzero_ps(), one = _mm512_set1_ps(1.0f), half = _mm512_set1_ps(0.5f);
            const __m512 quarter = _mm512_set1_ps(0.25f), least = _mm512_set1_ps(1e-30f), steps = _mm512_set1_ps(32768.0f);
            const __m512 magic = _mm512_set1_ps(kMagic), wrap = _mm512_set1_ps(65536.0f);
            const __m512i order = _mm512_setr_epi32(0, 1, 4, 5, 8, 9, 12, 13, 2, 3, 6, 7, 10, 11, 14, 15);
            __m512 low = _mm512_set1_ps(lowest);
            for (; i + 16 < n; i += 16)   // joints i .. i + 15: normals i - 1 .. i + 15
            {
                const __m512 a0 = _mm512_loadu_ps(f + 2 * i - 2), a1 = _mm512_loadu_ps(f + 2 * i + 14);
                const __m512 b0 = _mm512_loadu_ps(f + 2 * i), b1 = _mm512_loadu_ps(f + 2 * i + 16);
                const __m512 ax = _mm512_shuffle_ps(a0, a1, _MM_SHUFFLE(2, 0, 2, 0)), ay = _mm512_shuffle_ps(a0, a1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m512 bx = _mm512_shuffle_ps(b0, b1, _MM_SHUFFLE(2, 0, 2, 0)), by = _mm512_shuffle_ps(b0, b1, _MM_SHUFFLE(3, 1, 3, 1));
                const __m512 dot = _mm512_add_ps(_mm512_mul_ps(ax, bx), _mm512_mul_ps(ay, by));
                const __m512 h = _mm512_mul_ps(half, _mm512_add_ps(one, dot));
                if (_mm512_cmp_ps_mask(h, quarter, _CMP_GE_OQ) != 0xFFFF)
                    return -1;
                low = _mm512_min_ps(h, low);
                const __m512 wx = _mm512_add_ps(ax, bx), wy = _mm512_add_ps(ay, by);
                const __m512 s = _mm512_add_ps(_mm512_andnot_ps(sign, wx), _mm512_andnot_ps(sign, wy));
                const __m512 p = _mm512_div_ps(wx, _mm512_max_ps(s, least));
                const __m512 t = _mm512_sub_ps(one, _mm512_mask_xor_ps(p, _mm512_cmp_ps_mask(wy, zero, _CMP_LT_OQ), p, sign));
                const __m512 r = _mm512_sub_ps(_mm512_add_ps(_mm512_mul_ps(t, steps), magic), magic);
                const __m512 c = _mm512_mask_sub_ps(r, _mm512_cmp_ps_mask(r, wrap, _CMP_GE_OQ), r, wrap);
                _mm512_storeu_ps(code + i, _mm512_permutexvar_ps(order, c));
            }
            const __m256 low8 = _mm256_min_ps(_mm512_castps512_ps256(low), _mm512_extractf32x8_ps(low, 1));
            const float least16 = Min4(_mm_min_ps(_mm256_castps256_ps128(low8), _mm256_extractf128_ps(low8, 1)));
            lowest = least16 < lowest ? least16 : lowest;
            return i;
        }
#endif

#if defined(ESIA_LINES_NEON)
        // ------------------------------------------------------------------ NEON: 4 floats (AArch64)
        // the lanes' smallest and largest, as Bound / the plain C++'s minimum takes them
        void Fold(const float* lox, const float* loy, const float* hix, const float* hiy, const float* sh, int lanes, Vec2& lo, Vec2& hi, float& shortest)
        {
            for (int k = 0; k < lanes; ++k)
            {
                Bound(Vec2(lox[k], loy[k]), lo, hi);
                Bound(Vec2(hix[k], hiy[k]), lo, hi);
                shortest = sh[k] < shortest ? sh[k] : shortest;
            }
        }

        int NormalsNeon(const Vec2* path, int n, int i, Vec2* normal, Vec2& lo, Vec2& hi, float& shortest)
        {
            const float* f = &path[0].x;
            const float32x4_t tiny = vdupq_n_f32(kTiny), one = vdupq_n_f32(1.0f);
            float32x4_t lox = vdupq_n_f32(lo.x), loy = vdupq_n_f32(lo.y), hix = vdupq_n_f32(hi.x), hiy = vdupq_n_f32(hi.y), sh = vdupq_n_f32(shortest);
            for (; i + 4 < n; i += 4)   // segments i .. i + 3
            {
                const float32x4x2_t a = vld2q_f32(f + 2 * i), b = vld2q_f32(f + 2 * i + 2);   // x and y of points i .. i + 3, i + 1 .. i + 4
                const float32x4_t dx = vsubq_f32(b.val[0], a.val[0]), dy = vsubq_f32(b.val[1], a.val[1]);
                const float32x4_t l2 = vaddq_f32(vmulq_f32(dx, dx), vmulq_f32(dy, dy));
                if (vminvq_u32(vcgtq_f32(l2, tiny)) == 0)
                    return -1;
                const float32x4_t inv = vdivq_f32(one, vsqrtq_f32(l2));
                float32x4x2_t out;
                out.val[0] = vmulq_f32(vnegq_f32(dy), inv);
                out.val[1] = vmulq_f32(dx, inv);
                vst2q_f32(&normal[i].x, out);
                const float32x4_t ex = vabsq_f32(dx), ey = vabsq_f32(dy);
                const float32x4_t extent = vbslq_f32(vcgtq_f32(ex, ey), ex, ey);
                sh = vbslq_f32(vcltq_f32(extent, sh), extent, sh);
                lox = vbslq_f32(vcltq_f32(a.val[0], lox), a.val[0], lox);
                loy = vbslq_f32(vcltq_f32(a.val[1], loy), a.val[1], loy);
                hix = vbslq_f32(vcgtq_f32(a.val[0], hix), a.val[0], hix);
                hiy = vbslq_f32(vcgtq_f32(a.val[1], hiy), a.val[1], hiy);
            }
            float v[5][4];
            vst1q_f32(v[0], lox);
            vst1q_f32(v[1], loy);
            vst1q_f32(v[2], hix);
            vst1q_f32(v[3], hiy);
            vst1q_f32(v[4], sh);
            Fold(v[0], v[1], v[2], v[3], v[4], 4, lo, hi, shortest);
            return i;
        }

        int JointsNeon(const Vec2* normal, int n, int i, float* code, float& lowest)
        {
            const float* f = &normal[0].x;
            const float32x4_t zero = vdupq_n_f32(0.0f), one = vdupq_n_f32(1.0f), half = vdupq_n_f32(0.5f), quarter = vdupq_n_f32(0.25f);
            const float32x4_t least = vdupq_n_f32(1e-30f), steps = vdupq_n_f32(32768.0f), magic = vdupq_n_f32(kMagic), wrap = vdupq_n_f32(65536.0f);
            float32x4_t low = vdupq_n_f32(lowest);
            for (; i + 4 < n; i += 4)   // joints i .. i + 3: normals i - 1 .. i + 3
            {
                const float32x4x2_t a = vld2q_f32(f + 2 * i - 2), b = vld2q_f32(f + 2 * i);
                const float32x4_t dot = vaddq_f32(vmulq_f32(a.val[0], b.val[0]), vmulq_f32(a.val[1], b.val[1]));
                const float32x4_t h = vmulq_f32(half, vaddq_f32(one, dot));
                if (vminvq_u32(vcgeq_f32(h, quarter)) == 0)
                    return -1;
                low = vbslq_f32(vcltq_f32(h, low), h, low);
                const float32x4_t wx = vaddq_f32(a.val[0], b.val[0]), wy = vaddq_f32(a.val[1], b.val[1]);
                const float32x4_t s = vaddq_f32(vabsq_f32(wx), vabsq_f32(wy));
                const float32x4_t p = vdivq_f32(wx, vbslq_f32(vcgtq_f32(s, least), s, least));
                const float32x4_t t = vsubq_f32(one, vbslq_f32(vcltq_f32(wy, zero), vnegq_f32(p), p));
                const float32x4_t r = vsubq_f32(vaddq_f32(vmulq_f32(t, steps), magic), magic);
                vst1q_f32(code + i, vbslq_f32(vcgeq_f32(r, wrap), vsubq_f32(r, wrap), r));
            }
            float v[4];
            vst1q_f32(v, low);
            for (float x : v)
                lowest = x < lowest ? x : lowest;
            return i;
        }

        int ClearNeon(const Vec2* path, const Vec2* normal, int n, int i, float r, float clear)
        {
            const float* f = &path[0].x;
            const float* m = &normal[0].x;
            const float32x4_t one = vdupq_n_f32(1.0f), half = vdupq_n_f32(0.5f), rV = vdupq_n_f32(r), clearV = vdupq_n_f32(clear), milli = vdupq_n_f32(1e-3f);
            for (; i + 6 < n; i += 4)
            {
                const float32x4x2_t a = vld2q_f32(m + 2 * i - 2), b = vld2q_f32(m + 2 * i), c = vld2q_f32(m + 2 * i + 2);
                const float32x4x2_t p = vld2q_f32(f + 2 * i), q = vld2q_f32(f + 2 * i + 2);
                const float32x4_t k0 = vdivq_f32(half, vsqrtq_f32(vmulq_f32(half, vaddq_f32(one, vaddq_f32(vmulq_f32(a.val[0], b.val[0]), vmulq_f32(a.val[1], b.val[1]))))));
                const float32x4_t k1 = vdivq_f32(half, vsqrtq_f32(vmulq_f32(half, vaddq_f32(one, vaddq_f32(vmulq_f32(b.val[0], c.val[0]), vmulq_f32(b.val[1], c.val[1]))))));
                const float32x4_t u0x = vmulq_f32(vaddq_f32(a.val[0], b.val[0]), k0), u0y = vmulq_f32(vaddq_f32(a.val[1], b.val[1]), k0);
                const float32x4_t u1x = vmulq_f32(vaddq_f32(b.val[0], c.val[0]), k1), u1y = vmulq_f32(vaddq_f32(b.val[1], c.val[1]), k1);
                const float32x4_t dx = vsubq_f32(q.val[0], p.val[0]), dy = vsubq_f32(q.val[1], p.val[1]);
                const float32x4_t adx = vabsq_f32(dx), ady = vabsq_f32(dy);
                const float32x4_t turn = vmulq_f32(vabsq_f32(vsubq_f32(vmulq_f32(u0x, u1y), vmulq_f32(u0y, u1x))), rV);
                const float32x4_t margin = vmulq_f32(vaddq_f32(vaddq_f32(adx, ady), rV), milli);
                const float32x4_t c0 = vsubq_f32(vabsq_f32(vsubq_f32(vmulq_f32(u0x, dy), vmulq_f32(u0y, dx))), turn);
                const float32x4_t c1 = vsubq_f32(vabsq_f32(vsubq_f32(vmulq_f32(u1x, dy), vmulq_f32(u1y, dx))), turn);
                const uint32x4_t far = vcgtq_f32(vbslq_f32(vcgtq_f32(adx, ady), adx, ady), clearV);
                const uint32x4_t ok = vorrq_u32(far, vandq_u32(vcgtq_f32(c0, margin), vcgtq_f32(c1, margin)));
                if (vminvq_u32(ok) == 0)
                    return -1;
            }
            return i;
        }
#endif
    }

    // Each level takes what fits its vectors and hands the rest down - AVX-512 to AVX2 to SSE2 to plain C++ - so a wide
    // level is never left with a long tail of plain C++ (a line of 64 points: 48 + 8 + 4 + 3)
    bool Normals(const Vec2* path, int n, Vec2* normal, Vec2& lo, Vec2& hi, float& shortest)
    {
        lo = hi = path[n - 1];
        shortest = 3.0e38f;
        int i = 0;
        switch (cpu::Active())
        {
#if defined(ESIA_LINES_X86)
        case cpu::Simd::Avx512:
            i = NormalsAvx512(path, n, i, normal, lo, hi, shortest);
            [[fallthrough]];
        case cpu::Simd::Avx2:
            i = i < 0 ? i : NormalsAvx2(path, n, i, normal, lo, hi, shortest);
            [[fallthrough]];
        case cpu::Simd::Sse2:
            i = i < 0 ? i : NormalsSse2(path, n, i, normal, lo, hi, shortest);
            break;
#elif defined(ESIA_LINES_NEON)
        case cpu::Simd::Neon:
            i = NormalsNeon(path, n, i, normal, lo, hi, shortest);
            break;
#endif
        default:
            break;
        }
        return i >= 0 && NormalsScalar(path, n, i, normal, lo, hi, shortest);
    }

    bool Joints(const Vec2* normal, int n, float* code, float& lowest)
    {
        lowest = 1.0f;
        code[0] = QuadInstance::LineDirectionCode(normal[0]);
        code[n - 1] = QuadInstance::LineDirectionCode(normal[n - 2]);
        int i = 1;
        switch (cpu::Active())
        {
#if defined(ESIA_LINES_X86)
        case cpu::Simd::Avx512:
            i = JointsAvx512(normal, n, i, code, lowest);
            [[fallthrough]];
        case cpu::Simd::Avx2:
            i = i < 0 ? i : JointsAvx2(normal, n, i, code, lowest);
            [[fallthrough]];
        case cpu::Simd::Sse2:
            i = i < 0 ? i : JointsSse2(normal, n, i, code, lowest);
            break;
#elif defined(ESIA_LINES_NEON)
        case cpu::Simd::Neon:
            i = JointsNeon(normal, n, i, code, lowest);
            break;
#endif
        default:
            break;
        }
        return i >= 0 && JointsScalar(normal, n, i, code, lowest);
    }

    bool Clear(const Vec2* path, const Vec2* normal, int n, float r, float clear)
    {
        // the two end pieces in plain C++ (an end's line is the normal), the inside on the vectors
        if (n < 4)
            return ClearScalar(path, normal, n, 0, n - 1, r, clear);
        if (!ClearOne(path, normal, n, 0, r, clear) || !ClearOne(path, normal, n, n - 2, r, clear))
            return false;
        int i = 1;
        switch (cpu::Active())
        {
#if defined(ESIA_LINES_X86)
        case cpu::Simd::Avx512:   // (AVX2's: the check is a fraction of a line's cost)
        case cpu::Simd::Avx2:
            i = ClearAvx2(path, normal, n, i, r, clear);
            [[fallthrough]];
        case cpu::Simd::Sse2:
            i = i < 0 ? i : ClearSse2(path, normal, n, i, r, clear);
            break;
#elif defined(ESIA_LINES_NEON)
        case cpu::Simd::Neon:
            i = ClearNeon(path, normal, n, i, r, clear);
            break;
#endif
        default:
            break;
        }
        return i >= 0 && ClearScalar(path, normal, n, i, n - 2, r, clear);
    }

    void Pieces(const Vec2* path, int n, const float* code, float head, float hw, float rgb, float alpha, QuadInstance* quad)
    {
        int i = 0;
        switch (cpu::Active())
        {
#if defined(ESIA_LINES_X86)
        case cpu::Simd::Avx512:   // (a quad is 32 bytes: AVX2's stores)
        case cpu::Simd::Avx2:
            i = PiecesAvx2(path, n, i, code, head, hw, rgb, alpha, quad);
            [[fallthrough]];
        case cpu::Simd::Sse2:
            i = PiecesSse2(path, n, i, code, head, hw, rgb, alpha, quad);
            break;
#endif
        default:
            break;
        }
        PiecesScalar(path, n, i, code, head, hw, rgb, alpha, quad);
    }
}
