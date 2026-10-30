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
    uint _686 = gl_InstanceIndex + uint(_258.gDrawInfo.x);
    uint _950 = uint(_58.gConv.w) + (_686 * 2u);
    uint _964 = (uint(_58.gConv.w) + (_686 * 2u)) + 1u;
    bool _694 = (gl_VertexIndex & 1u) != 0u;
    bool _698 = (gl_VertexIndex & 2u) != 0u;
    float _975 = floor(gFxData_1._data[_964].z * 1.52587890625e-05);
    float _982 = floor((gFxData_1._data[_964].z - (_975 * 65536.0)) * 0.00390625);
    float4 _998 = float4((gFxData_1._data[_964].z - (_975 * 65536.0)) - (_982 * 256.0), _982, _975, gFxData_1._data[_964].w) * float4(0.0039215688593685626983642578125);
    float2 _1243;
    float2 _1244;
    float4 _1245;
    float2 _1249;
    float4 _1250;
    if (gFxData_1._data[_964].x < 0.0)
    {
        float _712 = (-1.0) - gFxData_1._data[_964].x;
        float _715 = floor(_712 * 0.125);
        float _719 = floor(gFxData_1._data[_964].w * 0.00390625);
        float _723 = _712 - (_715 * 8.0);
        float4 _1198 = _998;
        _1198.w = (gFxData_1._data[_964].w - (_719 * 256.0)) * 0.0039215688593685626983642578125;
        float _747 = fast::max((2.0 * _58.gTarget.z) / fast::max(abs(_58.gXform.x), 9.9999999747524270787835121154785e-07), (2.0 * _58.gTarget.w) / fast::max(abs(_58.gXform.y), 9.9999999747524270787835121154785e-07));
        float2 _1241;
        float4 _1246;
        if (_723 > 4.5)
        {
            float _753 = floor(_715 * 0.03125);
            float _764 = gFxData_1._data[_964].y + (_747 * 0.5);
            bool _771 = _723 > 5.5;
            float _779 = _764 * (1.0 + ((_715 - (_753 * 32.0)) * 0.0625));
            float2 _1240;
            do
            {
                bool _1012 = _694 && (!_771);
                float _1017 = (_1012 ? _719 : _753) * 3.0517578125e-05;
                float2 _1031 = fast::normalize(select(float2(1.0 - _1017, 2.0 - _1017), float2(1.0 - _1017, _1017), bool2(_1017 <= 1.0)));
                float2 _1050 = select(_1031, -_1031, bool2(dot(_1031, float2(gFxData_1._data[_950].y - gFxData_1._data[_950].w, gFxData_1._data[_950].z - gFxData_1._data[_950].x)) < 0.0));
                float2 _1065 = select(gFxData_1._data[_950].xy, gFxData_1._data[_950].zw, bool2(_1012)) + (_1050 * (_698 ? _779 : (-_779)));
                if (!(_771 && _694))
                {
                    _1240 = _1065;
                    break;
                }
                float2 _1078 = float2(_1050.y, -_1050.x);
                _1240 = _1065 + (select(_1078, -_1078, bool2(dot(_1078, gFxData_1._data[_950].xy - gFxData_1._data[_950].zw) < 0.0)) * _764);
                break;
            } while(false);
            _1246 = float4(fast::max(gFxData_1._data[_964].y - (_747 * 0.5), 0.0), _764, _719, 0.0);
            _1241 = _1240;
        }
        else
        {
            float _790 = floor(_715 * 0.0009765625);
            float _799 = _719 * 0.015625;
            float _801 = _790 * 0.015625;
            float _823 = (_723 > 3.5) ? 0.0 : (_747 + ((_723 - (2.0 * floor(_723 * 0.5))) * fast::max(_799 * _801, 0.0)));
            float2 _1242;
            if ((_723 >= 1.5) && (_723 < 3.5))
            {
                float2 _834 = gFxData_1._data[_950].zw - gFxData_1._data[_950].xy;
                float _836 = length(_834);
                float2 _844 = select(float2(1.0, 0.0), _834 / float2(_836), bool2(_836 > 9.9999997473787516355514526367188e-05));
                float _854 = gFxData_1._data[_964].y + _823;
                _1242 = select(gFxData_1._data[_950].xy - (_844 * _854), gFxData_1._data[_950].zw + (_844 * _854), bool2(_694)) + (float2(-_844.y, _844.x) * (_698 ? _854 : (-_854)));
            }
            else
            {
                _1242 = float2(_694 ? (gFxData_1._data[_950].z + _823) : (gFxData_1._data[_950].x - _823), _698 ? (gFxData_1._data[_950].w + _823) : (gFxData_1._data[_950].y - _823));
            }
            _1246 = float4(gFxData_1._data[_964].y, (_715 - (_790 * 1024.0)) * 0.000977517105638980865478515625, _799, _801);
            _1241 = _1242;
        }
        _1250 = _1198;
        _1249 = _1241;
        _1245 = _1246;
        _1244 = float2(_723, _747);
        _1243 = _1241;
    }
    else
    {
        float _1107 = floor(gFxData_1._data[_964].x * 0.000244140625);
        float _1116 = floor(gFxData_1._data[_964].y * 0.000244140625);
        _1250 = _998;
        _1249 = float2((gFxData_1._data[_964].x - (_1107 * 4096.0)) + (_694 ? (gFxData_1._data[_964].y - (_1116 * 4096.0)) : 0.0), _1107 + (_698 ? _1116 : 0.0)) * float2(0.000244140625);
        _1245 = float4(0.0);
        _1244 = float2(-1.0, 0.0);
        _1243 = float2(_694 ? gFxData_1._data[_950].z : gFxData_1._data[_950].x, _698 ? gFxData_1._data[_950].w : gFxData_1._data[_950].y);
    }
    out.gl_Position = float4((_1243 * _58.gXform.xy) + _58.gXform.zw, 0.0, 1.0);
    out._entryPointOutput_col = _1250;
    out._entryPointOutput_uv = _1249;
    out._entryPointOutput_shapeRect = gFxData_1._data[_950];
    out._entryPointOutput_shape = _1245;
    out._entryPointOutput_kind = _1244;
    return out;
}

