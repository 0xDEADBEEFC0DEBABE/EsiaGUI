#version 330
#if defined(GL_EXT_control_flow_attributes)
#extension GL_EXT_control_flow_attributes : require
#define SPIRV_CROSS_FLATTEN [[flatten]]
#define SPIRV_CROSS_BRANCH [[dont_flatten]]
#define SPIRV_CROSS_UNROLL [[unroll]]
#define SPIRV_CROSS_LOOP [[dont_unroll]]
#else
#define SPIRV_CROSS_FLATTEN
#define SPIRV_CROSS_BRANCH
#define SPIRV_CROSS_UNROLL
#define SPIRV_CROSS_LOOP
#endif

vec2 _1632;

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _89;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _121;

uniform sampler2D SPIRV_Cross_CombinedgTexgLinear;

in vec4 esia_v0;
in vec2 esia_v1;
flat in vec4 esia_v2;
flat in vec4 esia_v3;
flat in vec2 esia_v4;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec4 _1630;
    do
    {
        vec4 _787 = texture(SPIRV_Cross_CombinedgTexgLinear, esia_v1);
        SPIRV_CROSS_BRANCH
        if (esia_v4.x > (-0.5))
        {
            float _846 = floor(esia_v4.x + 0.5);
            float _849 = max(esia_v4.y, 9.9999997473787516355514526367188e-05);
            float _1624;
            SPIRV_CROSS_BRANCH
            if (_846 > 4.5)
            {
                vec2 _970 = esia_v1 - esia_v2.xy;
                float _1623;
                SPIRV_CROSS_BRANCH
                if (_846 < 5.5)
                {
                    _1623 = clamp((esia_v3.y - abs(dot(_970, normalize(vec2(esia_v2.y - esia_v2.w, esia_v2.z - esia_v2.x))))) / max(esia_v3.y - esia_v3.x, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                else
                {
                    float _1001 = floor(esia_v3.z + 0.5);
                    float _1003 = 3.1415927410125732421875 / _1001;
                    vec2 _1007 = normalize(esia_v2.xy - esia_v2.zw);
                    vec2 _1013 = vec2(-_1007.y, _1007.x);
                    float _1031 = (clamp(floor(atan(max(dot(_970, _1007), 0.0), dot(_970, _1013)) / _1003), 0.0, _1001 - 1.0) + 0.5) * _1003;
                    float _1034 = cos(1.57079637050628662109375 / _1001);
                    _1623 = (dot(_970, _970) < 9.9999999600419720025001879548654e-13) ? 1.0 : clamp(((esia_v3.y * _1034) - dot(_970, (_1013 * cos(_1031)) + (_1007 * sin(_1031)))) / max((esia_v3.y - esia_v3.x) * _1034, 9.9999999747524270787835121154785e-07), 0.0, 1.0);
                }
                _1624 = _1623;
            }
            else
            {
                float _1625;
                if (_846 < 3.5)
                {
                    float _1622;
                    SPIRV_CROSS_BRANCH
                    if (_846 >= 1.5)
                    {
                        vec2 _1072 = esia_v1 - esia_v2.xy;
                        vec2 _1075 = esia_v2.zw - esia_v2.xy;
                        _1622 = length(_1072 - (_1075 * clamp(dot(_1072, _1075) / max(dot(_1075, _1075), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                    }
                    else
                    {
                        vec2 _885 = esia_v1 - ((esia_v2.xy + esia_v2.zw) * 0.5);
                        vec2 _894 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        float _1140 = min(_894.x, _894.y);
                        float _1143 = min((_885.x > 0.0) ? ((_885.y > 0.0) ? esia_v3.x : esia_v3.x) : ((_885.y > 0.0) ? esia_v3.x : esia_v3.x), _1140);
                        float _1149 = _1143 * (1.0 + (0.60000002384185791015625 * esia_v3.y));
                        float _1618;
                        float _1619;
                        if (_1149 > _1140)
                        {
                            float _1163 = esia_v3.y * clamp((_1140 - _1143) / max(0.60000002384185791015625 * _1143, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _1619 = _1163;
                            _1618 = _1143 * (1.0 + (0.60000002384185791015625 * _1163));
                        }
                        else
                        {
                            _1619 = esia_v3.y;
                            _1618 = _1149;
                        }
                        vec2 _1192 = (abs(_885) - _894) + vec2(_1618);
                        vec2 _1194 = max(_1192, vec2(0.0));
                        float _1620;
                        SPIRV_CROSS_BRANCH
                        if ((_1194.x > 0.0) && (_1194.y > 0.0))
                        {
                            float _1621;
                            if ((_1619 > 0.001000000047497451305389404296875) && (_1618 > 9.9999997473787516355514526367188e-05))
                            {
                                float _1211 = 2.0 + (2.0 * _1619);
                                vec2 _1216 = _1194 / vec2(max(_1618, 9.9999997473787516355514526367188e-05));
                                _1621 = pow(pow(_1216.x, _1211) + pow(_1216.y, _1211), 1.0 / _1211) * _1618;
                            }
                            else
                            {
                                _1621 = length(_1194);
                            }
                            _1620 = _1621;
                        }
                        else
                        {
                            _1620 = max(_1194.x, _1194.y);
                        }
                        _1622 = (min(max(_1192.x, _1192.y), 0.0) + _1620) - _1618;
                    }
                    _1625 = ((_846 - (2.0 * floor(_846 * 0.5))) > 0.5) ? (clamp(0.5 - ((_1622 - (esia_v3.z * esia_v3.w)) / _849), 0.0, 1.0) - clamp(0.5 - ((_1622 + (esia_v3.z * (1.0 - esia_v3.w))) / _849), 0.0, 1.0)) : clamp(0.5 - (_1622 / _849), 0.0, 1.0);
                }
                else
                {
                    _1625 = 1.0;
                }
                _1624 = _1625;
            }
            vec2 _1260 = gl_FragCoord.xy + _89.gConv.yy;
            vec2 _1626;
            if (_89.gConv.x > 0.5)
            {
                vec2 _1582;
                _1582.y = _89.gTarget.y - _1260.y;
                _1626 = _1582;
            }
            else
            {
                _1626 = _1260;
            }
            float _1627;
            if (_121.gFade.z > 0.0)
            {
                _1627 = smoothstep(0.0, 1.0, clamp((_1626.y - _121.gFade.x) / _121.gFade.z, 0.0, 1.0));
            }
            else
            {
                _1627 = 1.0;
            }
            float _1628;
            if (_121.gFade.w > 0.0)
            {
                _1628 = _1627 * smoothstep(0.0, 1.0, clamp((_121.gFade.y - _1626.y) / _121.gFade.w, 0.0, 1.0));
            }
            else
            {
                _1628 = _1627;
            }
            vec4 _800 = vec4(esia_v0.xyz, (esia_v0.w * _1624) * _1628);
            vec4 _1629;
            if (_89.gTime.w > 0.5)
            {
                vec3 _1332 = clamp(_800.xyz, vec3(0.0), vec3(1.0));
                vec3 _1338 = pow((_1332 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
                vec3 _1341 = _1332 * vec3(0.077399380505084991455078125);
                bvec3 _1343 = lessThanEqual(_1332, vec3(0.040449999272823333740234375));
                vec3 _1344 = vec3(_1343.x ? _1341.x : _1338.x, _1343.y ? _1341.y : _1338.y, _1343.z ? _1341.z : _1338.z);
                vec4 _1584 = _800;
                _1584.x = _1344.x;
                _1584.y = _1344.y;
                _1584.z = _1344.z;
                _1629 = _1584;
            }
            else
            {
                _1629 = _800;
            }
            _1630 = _1629;
            break;
        }
        vec4 _1353 = esia_v0 * _787;
        vec2 _1372 = gl_FragCoord.xy + _89.gConv.yy;
        vec2 _1611;
        if (_89.gConv.x > 0.5)
        {
            vec2 _1591;
            _1591.y = _89.gTarget.y - _1372.y;
            _1611 = _1591;
        }
        else
        {
            _1611 = _1372;
        }
        float _1612;
        if (_121.gFade.z > 0.0)
        {
            _1612 = smoothstep(0.0, 1.0, clamp((_1611.y - _121.gFade.x) / _121.gFade.z, 0.0, 1.0));
        }
        else
        {
            _1612 = 1.0;
        }
        float _1613;
        if (_121.gFade.w > 0.0)
        {
            _1613 = _1612 * smoothstep(0.0, 1.0, clamp((_121.gFade.y - _1611.y) / _121.gFade.w, 0.0, 1.0));
        }
        else
        {
            _1613 = _1612;
        }
        float _1360 = _1353.w * _1613;
        vec4 _1594 = _1353;
        _1594.w = _1360;
        vec4 _1617;
        if (_89.gTime.w > 0.5)
        {
            vec3 _1444 = clamp(_1594.xyz, vec3(0.0), vec3(1.0));
            vec3 _1450 = pow((_1444 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
            vec3 _1453 = _1444 * vec3(0.077399380505084991455078125);
            bvec3 _1455 = lessThanEqual(_1444, vec3(0.040449999272823333740234375));
            _1617 = vec4(vec3(_1455.x ? _1453.x : _1450.x, _1455.y ? _1453.y : _1450.y, _1455.z ? _1453.z : _1450.z), _1360);
        }
        else
        {
            _1617 = _1594;
        }
        _1630 = _1617;
        break;
    } while(false);
    _entryPointOutput = _1630;
}

