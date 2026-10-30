#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct gFxData
{
    float4 _data[1];
};

struct WgtFrame
{
    float4 gXform;
    float4 gTarget;
    float4 gDisplay;
    float4 gTime;
    float4 gLevel[6];
    float4 gText;
    float4 gConv;
};

struct WgtDraw
{
    float4 gFade;
    float4 gDrawInfo;
};

struct esia_main_out
{
    float4 _entryPointOutput_col [[user(locn0)]];
    float2 _entryPointOutput_uv [[user(locn1)]];
    float4 _entryPointOutput_shapeRect [[user(locn2)]];
    float4 _entryPointOutput_shape [[user(locn3)]];
    float2 _entryPointOutput_kind [[user(locn4)]];
    float4 gl_Position [[position]];
};

vertex esia_main_out esia_main(constant WgtFrame& _58 [[buffer(0)]], constant WgtDraw& _258 [[buffer(2)]], const device gFxData& gFxData_1 [[buffer(10)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    esia_main_out out = {};
    uint _699 = gl_InstanceIndex + uint(_258.gDrawInfo.x);
    uint _974 = uint(_58.gConv.w) + (_699 * 2u);
    uint _988 = (uint(_58.gConv.w) + (_699 * 2u)) + 1u;
    bool _707 = (gl_VertexIndex & 1u) != 0u;
    bool _711 = (gl_VertexIndex & 2u) != 0u;
    float _999 = floor(gFxData_1._data[_988].z * 1.52587890625e-05);
    float _1006 = floor((gFxData_1._data[_988].z - (_999 * 65536.0)) * 0.00390625);
    float4 _1022 = float4((gFxData_1._data[_988].z - (_999 * 65536.0)) - (_1006 * 256.0), _1006, _999, gFxData_1._data[_988].w) * float4(0.0039215688593685626983642578125);
    float2 _1275;
    float2 _1277;
    float4 _1279;
    float2 _1284;
    float4 _1285;
    if (gFxData_1._data[_988].x < 0.0)
    {
        float _725 = (-1.0) - gFxData_1._data[_988].x;
        float _728 = floor(_725 * 0.125);
        float _732 = floor(gFxData_1._data[_988].w * 0.00390625);
        float _736 = _725 - (_728 * 8.0);
        float4 _1224 = _1022;
        _1224.w = (gFxData_1._data[_988].w - (_732 * 256.0)) * 0.0039215688593685626983642578125;
        float _760 = fast::max((2.0 * _58.gTarget.z) / fast::max(abs(_58.gXform.x), 9.9999999747524270787835121154785e-07), (2.0 * _58.gTarget.w) / fast::max(abs(_58.gXform.y), 9.9999999747524270787835121154785e-07));
        float2 _1273;
        float4 _1280;
        if (_736 > 4.5)
        {
            float _766 = floor(_728 * 0.03125);
            float _777 = gFxData_1._data[_988].y + (_760 * 0.5);
            bool _784 = _736 > 5.5;
            float _792 = _777 * (1.0 + ((_728 - (_766 * 32.0)) * 0.0625));
            float2 _1272;
            do
            {
                bool _1036 = _707 && (!_784);
                float _1041 = (_1036 ? _732 : _766) * 3.0517578125e-05;
                float2 _1055 = fast::normalize(select(float2(1.0 - _1041, 2.0 - _1041), float2(1.0 - _1041, _1041), bool2(_1041 <= 1.0)));
                float2 _1074 = select(_1055, -_1055, bool2(dot(_1055, float2(gFxData_1._data[_974].y - gFxData_1._data[_974].w, gFxData_1._data[_974].z - gFxData_1._data[_974].x)) < 0.0));
                float2 _1089 = select(gFxData_1._data[_974].xy, gFxData_1._data[_974].zw, bool2(_1036)) + (_1074 * (_711 ? _792 : (-_792)));
                if (!(_784 && _707))
                {
                    _1272 = _1089;
                    break;
                }
                float2 _1102 = float2(_1074.y, -_1074.x);
                _1272 = _1089 + (select(_1102, -_1102, bool2(dot(_1102, gFxData_1._data[_974].xy - gFxData_1._data[_974].zw) < 0.0)) * _777);
                break;
            } while(false);
            _1280 = float4(fast::max(gFxData_1._data[_988].y - (_760 * 0.5), 0.0), _777, _732, 0.0);
            _1273 = _1272;
        }
        else
        {
            float _803 = floor(_728 * 0.0009765625);
            float _812 = _732 * 0.015625;
            float _814 = _803 * 0.015625;
            float _836 = (_736 > 3.5) ? 0.0 : (_760 + ((_736 - (2.0 * floor(_736 * 0.5))) * fast::max(_812 * _814, 0.0)));
            float2 _1274;
            if ((_736 >= 1.5) && (_736 < 3.5))
            {
                float2 _847 = gFxData_1._data[_974].zw - gFxData_1._data[_974].xy;
                float _849 = length(_847);
                float2 _857 = select(float2(1.0, 0.0), _847 / float2(_849), bool2(_849 > 9.9999997473787516355514526367188e-05));
                float _867 = gFxData_1._data[_988].y + _836;
                _1274 = select(gFxData_1._data[_974].xy - (_857 * _867), gFxData_1._data[_974].zw + (_857 * _867), bool2(_707)) + (float2(-_857.y, _857.x) * (_711 ? _867 : (-_867)));
            }
            else
            {
                _1274 = float2(_707 ? (gFxData_1._data[_974].z + _836) : (gFxData_1._data[_974].x - _836), _711 ? (gFxData_1._data[_974].w + _836) : (gFxData_1._data[_974].y - _836));
            }
            _1280 = float4(gFxData_1._data[_988].y, (_728 - (_803 * 1024.0)) * 0.000977517105638980865478515625, _812, _814);
            _1273 = _1274;
        }
        _1285 = _1224;
        _1284 = _1273;
        _1279 = _1280;
        _1277 = float2(_736, _760);
        _1275 = _1273;
    }
    else
    {
        float2 _1278;
        float4 _1289;
        if (gFxData_1._data[_988].w > 255.5)
        {
            float4 _1250 = _1022;
            _1250.w = (gFxData_1._data[_988].w - 256.0) * 0.0039215688593685626983642578125;
            _1289 = _1250;
            _1278 = float2(-2.0, 0.0);
        }
        else
        {
            _1289 = _1022;
            _1278 = float2(-1.0, 0.0);
        }
        float _1131 = floor(gFxData_1._data[_988].x * 0.000244140625);
        float _1140 = floor(gFxData_1._data[_988].y * 0.000244140625);
        _1285 = _1289;
        _1284 = float2((gFxData_1._data[_988].x - (_1131 * 4096.0)) + (_707 ? (gFxData_1._data[_988].y - (_1140 * 4096.0)) : 0.0), _1131 + (_711 ? _1140 : 0.0)) * float2(0.000244140625);
        _1279 = float4(0.0);
        _1277 = _1278;
        _1275 = float2(_707 ? gFxData_1._data[_974].z : gFxData_1._data[_974].x, _711 ? gFxData_1._data[_974].w : gFxData_1._data[_974].y);
    }
    out.gl_Position = float4((_1275 * _58.gXform.xy) + _58.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_col = _1285;
    out._entryPointOutput_uv = _1284;
    out._entryPointOutput_shapeRect = gFxData_1._data[_974];
    out._entryPointOutput_shape = _1279;
    out._entryPointOutput_kind = _1277;
    return out;
}

