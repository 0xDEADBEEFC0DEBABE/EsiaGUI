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

constant float2 _1636 = {};

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

fragment esia_main_out esia_main(esia_main_in in [[stage_in]], constant WgtFrame& _89 [[buffer(0)]], constant WgtDraw& _121 [[buffer(2)]], texture2d<float> gTex [[texture(3)]], sampler gLinear [[sampler(11)]], float4 gl_FragCoord [[position]])
{
    esia_main_out out = {};
    float4 _1634;
    do
    {
        float4 _791 = gTex.sample(gLinear, in.i_uv);
        if (in.i_kind.x > (-0.5))
        {
            float _850 = floor(in.i_kind.x + 0.5);
            float _853 = fast::max(in.i_kind.y, 9.9999997473787516355514526367188e-05);
            float _1628;
            if (_850 > 4.5)
            {
                float2 _974 = in.i_uv - in.i_shapeRect.xy;
                float _1627;
                if (_850 < 5.5)
                {
                    _1627 = fast::clamp((in.i_shape.y - abs(dot(_974, fast::normalize(float2(in.i_shapeRect.y - in.i_shapeRect.w, in.i_shapeRect.z - in.i_shapeRect.x))))) / fast::max(in.i_shape.y - in.i_shape.x, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                else
                {
                    float _1005 = floor(in.i_shape.z + 0.5);
                    float _1007 = 3.1415927410125732421875 / _1005;
                    float2 _1011 = fast::normalize(in.i_shapeRect.xy - in.i_shapeRect.zw);
                    float2 _1017 = float2(-_1011.y, _1011.x);
                    float _1035 = (fast::clamp(floor(precise::atan2(fast::max(dot(_974, _1011), 0.0), dot(_974, _1017)) / _1007), 0.0, _1005 - 1.0) + 0.5) * _1007;
                    float _1038 = cos(1.57079637050628662109375 / _1005);
                    _1627 = (dot(_974, _974) < 9.9999999600419720025001879548654e-13) ? 1.0 : fast::clamp(((in.i_shape.y * _1038) - dot(_974, (_1017 * cos(_1035)) + (_1011 * sin(_1035)))) / fast::max((in.i_shape.y - in.i_shape.x) * _1038, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                _1628 = _1627;
            }
            else
            {
                float _1629;
                if (_850 < 3.5)
                {
                    float _1626;
                    if (_850 >= 1.5)
                    {
                        float2 _1076 = in.i_uv - in.i_shapeRect.xy;
                        float2 _1079 = in.i_shapeRect.zw - in.i_shapeRect.xy;
                        _1626 = length(_1076 - (_1079 * fast::clamp(dot(_1076, _1079) / fast::max(dot(_1079, _1079), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - in.i_shape.x;
                    }
                    else
                    {
                        float2 _889 = in.i_uv - ((in.i_shapeRect.xy + in.i_shapeRect.zw) * 0.5);
                        float2 _898 = fast::max((in.i_shapeRect.zw - in.i_shapeRect.xy) * 0.5, float2(0.001000000047497451305389404296875));
                        float _1144 = fast::min(_898.x, _898.y);
                        float _1147 = fast::min((_889.x > 0.0) ? ((_889.y > 0.0) ? in.i_shape.x : in.i_shape.x) : ((_889.y > 0.0) ? in.i_shape.x : in.i_shape.x), _1144);
                        float _1153 = _1147 * (1.0 + (0.60000002384185791015625 * in.i_shape.y));
                        float _1622;
                        float _1623;
                        if (_1153 > _1144)
                        {
                            float _1167 = in.i_shape.y * fast::clamp((_1144 - _1147) / fast::max(0.60000002384185791015625 * _1147, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _1623 = _1167;
                            _1622 = _1147 * (1.0 + (0.60000002384185791015625 * _1167));
                        }
                        else
                        {
                            _1623 = in.i_shape.y;
                            _1622 = _1153;
                        }
                        float2 _1196 = (abs(_889) - _898) + float2(_1622);
                        float2 _1198 = fast::max(_1196, float2(0.0));
                        float _1624;
                        if ((_1198.x > 0.0) && (_1198.y > 0.0))
                        {
                            float _1625;
                            if ((_1623 > 0.001000000047497451305389404296875) && (_1622 > 9.9999997473787516355514526367188e-05))
                            {
                                float _1215 = 2.0 + (2.0 * _1623);
                                float2 _1220 = _1198 / float2(fast::max(_1622, 9.9999997473787516355514526367188e-05));
                                _1625 = pow(pow(_1220.x, _1215) + pow(_1220.y, _1215), 1.0 / _1215) * _1622;
                            }
                            else
                            {
                                _1625 = length(_1198);
                            }
                            _1624 = _1625;
                        }
                        else
                        {
                            _1624 = fast::max(_1198.x, _1198.y);
                        }
                        _1626 = (fast::min(fast::max(_1196.x, _1196.y), 0.0) + _1624) - _1622;
                    }
                    _1629 = ((_850 - (2.0 * floor(_850 * 0.5))) > 0.5) ? (fast::clamp(0.5 - ((_1626 - (in.i_shape.z * in.i_shape.w)) / _853), 0.0, 1.0) - fast::clamp(0.5 - ((_1626 + (in.i_shape.z * (1.0 - in.i_shape.w))) / _853), 0.0, 1.0)) : fast::clamp(0.5 - (_1626 / _853), 0.0, 1.0);
                }
                else
                {
                    _1629 = 1.0;
                }
                _1628 = _1629;
            }
            float2 _1264 = gl_FragCoord.xy + _89.gConv.yy;
            float2 _1630;
            if (_89.gConv.x > 0.5)
            {
                float2 _1586;
                _1586.y = _89.gTarget.y - _1264.y;
                _1630 = _1586;
            }
            else
            {
                _1630 = _1264;
            }
            float _1631;
            if (_121.gFade.z > 0.0)
            {
                _1631 = smoothstep(0.0, 1.0, fast::clamp((_1630.y - _121.gFade.x) / _121.gFade.z, 0.0, 1.0));
            }
            else
            {
                _1631 = 1.0;
            }
            float _1632;
            if (_121.gFade.w > 0.0)
            {
                _1632 = _1631 * smoothstep(0.0, 1.0, fast::clamp((_121.gFade.y - _1630.y) / _121.gFade.w, 0.0, 1.0));
            }
            else
            {
                _1632 = _1631;
            }
            float4 _804 = float4(in.i_col.xyz, (in.i_col.w * _1628) * _1632);
            float4 _1633;
            if (_89.gTime.w > 0.5)
            {
                float3 _1336 = fast::clamp(_804.xyz, float3(0.0), float3(1.0));
                float3 _1348 = select(pow((_1336 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1336 * float3(0.077399380505084991455078125), _1336 <= float3(0.040449999272823333740234375));
                float4 _1588 = _804;
                _1588.x = _1348.x;
                _1588.y = _1348.y;
                _1588.z = _1348.z;
                _1633 = _1588;
            }
            else
            {
                _1633 = _804;
            }
            _1634 = _1633;
            break;
        }
        float4 _1357 = in.i_col * _791;
        float2 _1376 = gl_FragCoord.xy + _89.gConv.yy;
        float2 _1615;
        if (_89.gConv.x > 0.5)
        {
            float2 _1595;
            _1595.y = _89.gTarget.y - _1376.y;
            _1615 = _1595;
        }
        else
        {
            _1615 = _1376;
        }
        float _1616;
        if (_121.gFade.z > 0.0)
        {
            _1616 = smoothstep(0.0, 1.0, fast::clamp((_1615.y - _121.gFade.x) / _121.gFade.z, 0.0, 1.0));
        }
        else
        {
            _1616 = 1.0;
        }
        float _1617;
        if (_121.gFade.w > 0.0)
        {
            _1617 = _1616 * smoothstep(0.0, 1.0, fast::clamp((_121.gFade.y - _1615.y) / _121.gFade.w, 0.0, 1.0));
        }
        else
        {
            _1617 = _1616;
        }
        float _1364 = _1357.w * _1617;
        float4 _1598 = _1357;
        _1598.w = _1364;
        float4 _1621;
        if (_89.gTime.w > 0.5)
        {
            float3 _1448 = fast::clamp(_1598.xyz, float3(0.0), float3(1.0));
            _1621 = float4(select(pow((_1448 + float3(0.054999999701976776123046875)) * float3(0.947867333889007568359375), float3(2.400000095367431640625)), _1448 * float3(0.077399380505084991455078125), _1448 <= float3(0.040449999272823333740234375)), _1364);
        }
        else
        {
            _1621 = _1598;
        }
        _1634 = _1621;
        break;
    } while(false);
    out._entryPointOutput = _1634;
    return out;
}

