#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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

constant float2 _1567 = {};

struct esia_main_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct esia_main_in
{
    float4 i_col [[user(locn0)]];
    float2 i_uv [[user(locn1)]];
    float4 i_shapeRect [[user(locn2), flat]];
    float4 i_shape [[user(locn3), flat]];
    float2 i_kind [[user(locn4), flat]];
};

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _76 [[buffer(0)]], constant WgtDraw& _108 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _1565;
    do
    {
        float4 _750 = gTex.sample(gLinear, in.i_uv);
        if (in.i_kind.x > (-0.5))
        {
            float _809 = floor(in.i_kind.x + 0.5);
            float _812 = fast::max(in.i_kind.y, 9.9999997473787516355514526367188e-05);
            float _1559;
            if (_809 > 4.5)
            {
                float2 _933 = in.i_uv - in.i_shapeRect.xy;
                float _1558;
                if (_809 < 5.5)
                {
                    _1558 = fast::clamp((in.i_shape.y - abs(dot(_933, fast::normalize(float2(in.i_shapeRect.y - in.i_shapeRect.w, in.i_shapeRect.z - in.i_shapeRect.x))))) / fast::max(in.i_shape.y - in.i_shape.x, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                else
                {
                    float _964 = floor(in.i_shape.z + 0.5);
                    float _966 = 3.1415927410125732421875 / _964;
                    float2 _970 = fast::normalize(in.i_shapeRect.xy - in.i_shapeRect.zw);
                    float2 _976 = float2(-_970.y, _970.x);
                    float _994 = (fast::clamp(floor(precise::atan2(fast::max(dot(_933, _970), 0.0), dot(_933, _976)) / _966), 0.0, _964 - 1.0) + 0.5) * _966;
                    float _997 = cos(1.57079637050628662109375 / _964);
                    _1558 = (dot(_933, _933) < 9.9999999600419720025001879548654e-13) ? 1.0 : fast::clamp(((in.i_shape.y * _997) - dot(_933, (_976 * cos(_994)) + (_970 * sin(_994)))) / fast::max((in.i_shape.y - in.i_shape.x) * _997, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                _1559 = _1558;
            }
            else
            {
                float _1560;
                if (_809 < 3.5)
                {
                    float _1557;
                    if (_809 >= 1.5)
                    {
                        float2 _1035 = in.i_uv - in.i_shapeRect.xy;
                        float2 _1038 = in.i_shapeRect.zw - in.i_shapeRect.xy;
                        _1557 = length(_1035 - (_1038 * fast::clamp(dot(_1035, _1038) / fast::max(dot(_1038, _1038), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_shape.x;
                    }
                    else
                    {
                        float2 _848 = in.i_uv - ((in.i_shapeRect.xy + in.i_shapeRect.zw) * 0.5);
                        float2 _857 = fast::max((in.i_shapeRect.zw - in.i_shapeRect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float _1091 = fast::min(_857.x, _857.y);
                        float _1094 = fast::min((_848.x > 0.0) ? ((_848.y > 0.0) ? in.i_shape.x : in.i_shape.x) : ((_848.y > 0.0) ? in.i_shape.x : in.i_shape.x), _1091);
                        float _1100 = _1094 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _1551;
                        float _1552;
                        if (_1100 > _1091)
                        {
                            float _1114 = in.i_shape.y * fast::clamp((_1091 - _1094) / fast::max(0.60000002384185791015625 * _1094, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _1552 = _1114;
                            _1551 = _1094 * (1.0 + (0.60000002384185791015625 * _1114));
                        }
                        else
                        {
                            _1552 = in.i_shape.y;
                            _1551 = _1100;
                        }
                        float2 _1127 = (abs(_848) - _857) + float2(_1551);
                        float2 _1129 = fast::max(_1127, float2(0.0));
                        float _1553;
                        if ((_1129.x > 0.0) && (_1129.y > 0.0))
                        {
                            float _1554;
                            if ((_1552 > 0.001000000047497451305389404296875) && (_1551 > 9.9999997473787516355514526367188e-05))
                            {
                                float _1146 = 2.0 + (2.0 * _1552);
                                float2 _1151 = _1129 / float2(fast::max(_1551, 9.9999997473787516355514526367188e-05));
                                _1554 = pow(pow(_1151.x, _1146) + pow(_1151.y, _1146), 1.0 / _1146) * _1551;
                            }
                            else
                            {
                                _1554 = length(_1129);
                            }
                            _1553 = _1554;
                        }
                        else
                        {
                            _1553 = fast::max(_1129.x, _1129.y);
                        }
                        _1557 = (fast::min(fast::max(_1127.x, _1127.y), 0.0) + _1553) - _1551;
                    }
                    _1560 = ((_809 - (2.0 * floor(_809 * 0.5))) > 0.5) ? (fast::clamp(0.5 - ((_1557 - (in.i_shape.z * in.i_shape.w)) / _812), 0.0, 1.0) - fast::clamp(0.5 - ((_1557 + (in.i_shape.z * (1.0 - in.i_shape.w))) / _812), 0.0, 1.0)) : fast::clamp(0.5 - (_1557 / _812), 0.0, 1.0);
                }
                else
                {
                    _1560 = 1.0;
                }
                _1559 = _1560;
            }
            float2 _1195 = gl_FragCoord.xy + _76.gConv.yy;
            float2 _1561;
            if (_76.gConv.x > 0.5)
            {
                float2 _1515;
                _1515.y = _76.gTarget.y - _1195.y;
                _1561 = _1515;
            }
            else
            {
                _1561 = _1195;
            }
            float _1562;
            if (_108.gFade.z > 0.0)
            {
                _1562 = smoothstep(0.0, 1.0, fast::clamp((_1561.y - _108.gFade.x) / _108.gFade.z, 0.0, 1.0));
            }
            else
            {
                _1562 = 1.0;
            }
            float _1563;
            if (_108.gFade.w > 0.0)
            {
                _1563 = _1562 * smoothstep(0.0, 1.0, fast::clamp((_108.gFade.y - _1561.y) / _108.gFade.w, 0.0, 1.0));
            }
            else
            {
                _1563 = _1562;
            }
            float4 _763 = float4(in.i_col.xyz, (in.i_col.w * _1559) * _1563);
            float4 _1564;
            if (_76.gTime.w > 0.5)
            {
                float3 _1267 = fast::clamp(_763.xyz, float3(0.0), float3(1.0));
                float3 _1279 = select(pow((_1267 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1267 * float3(0.077399380505084991455078125), _1267 <= float3(0.040449999272823333740234375));
                float4 _1517 = _763;
                _1517.x = _1279.x;
                _1517.y = _1279.y;
                _1517.z = _1279.z;
                _1564 = _1517;
            }
            else
            {
                _1564 = _763;
            }
            _1565 = _1564;
            break;
        }
        float4 _1288 = in.i_col * _750;
        float2 _1307 = gl_FragCoord.xy + _76.gConv.yy;
        float2 _1544;
        if (_76.gConv.x > 0.5)
        {
            float2 _1524;
            _1524.y = _76.gTarget.y - _1307.y;
            _1544 = _1524;
        }
        else
        {
            _1544 = _1307;
        }
        float _1545;
        if (_108.gFade.z > 0.0)
        {
            _1545 = smoothstep(0.0, 1.0, fast::clamp((_1544.y - _108.gFade.x) / _108.gFade.z, 0.0, 1.0));
        }
        else
        {
            _1545 = 1.0;
        }
        float _1546;
        if (_108.gFade.w > 0.0)
        {
            _1546 = _1545 * smoothstep(0.0, 1.0, fast::clamp((_108.gFade.y - _1544.y) / _108.gFade.w, 0.0, 1.0));
        }
        else
        {
            _1546 = _1545;
        }
        float _1295 = _1288.w * _1546;
        float4 _1527 = _1288;
        _1527.w = _1295;
        float4 _1550;
        if (_76.gTime.w > 0.5)
        {
            float3 _1379 = fast::clamp(_1527.xyz, float3(0.0), float3(1.0));
            _1550 = float4(select(pow((_1379 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1379 * float3(0.077399380505084991455078125), _1379 <= float3(0.040449999272823333740234375)), _1295);
        }
        else
        {
            _1550 = _1527;
        }
        _1565 = _1550;
        break;
    } while(false);
    out._entryPointOutput = _1565;
    return out;
}

