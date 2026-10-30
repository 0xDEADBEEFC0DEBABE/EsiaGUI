// Esia - the loops of a polyline's quads (Painter::LineQuads), for every level of esia::cpu::Simd: plain C++, SSE2,
// AVX2, AVX-512 and NEON, chosen per call. They write the same bits on every level (line_kernels.cpp).
#pragma once

#include "esia/core/draw_list.hpp"

namespace esia::render::lines
{
    // The segments' unit normals ((-dy, dx) / |d|, the geometry's) into normal[0 .. n - 2], the points' bounds, and the
    // shortest segment's larger extent along x or y. false: two points closer than BuildPath keeps apart (1e-4).
    bool Normals(const Vec2* path, int n, Vec2* normal, Vec2& lo, Vec2& hi, float& shortest);
    // The joints' directions (QuadInstance::LineDirectionCode) into code[0 .. n - 1] - an end's the segment's normal, a
    // joint's between the normals' bisector - and the cosine of half the sharpest turn, squared. false: a turn sharper
    // than 120 degrees (the geometry clips its miter).
    bool Joints(const Vec2* normal, int n, float* code, float& lowest);
    // Every piece clear of its joints' lines inside its quad (Painter::LineQuads): longer than `clear` along x or y, or its
    // far joint's line clear of its near corners and its near line of the far ones by a margin, the quad reaching `r`
    // along them. false: a piece too short for the turns at its ends.
    bool Clear(const Vec2* path, const Vec2* normal, int n, float r, float clear);
    // The pieces' quads into quad[0 .. n - 2] (QuadInstance::SetLinePiece's fields): `head` = -1 - (LinePiece + 8 reach),
    // `rgb` and `alpha` the color's fields.
    void Pieces(const Vec2* path, int n, const float* code, float head, float hw, float rgb, float alpha, QuadInstance* quad);
}
