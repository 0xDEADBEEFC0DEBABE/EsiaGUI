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

#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 4294967295u
#endif
const uint kEsiaFxFeatures = SPIRV_CROSS_CONSTANT_ID_0;
const uint _2279 = (32768u & kEsiaFxFeatures);
const uint _2294 = (262144u & kEsiaFxFeatures);
const uint _2365 = (512u & kEsiaFxFeatures);
const uint _2417 = (262144u & kEsiaFxFeatures);
const uint _2437 = (32768u & kEsiaFxFeatures);
const uint _2444 = (512u & kEsiaFxFeatures);
const vec3 _2986[6] = vec3[](vec3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), vec3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), vec3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), vec3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), vec3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), vec3(0.660000026226043701171875, 0.5, 1.0));
const uint _3629 = (512u & kEsiaFxFeatures);
const uint _3649 = (32768u & kEsiaFxFeatures);
const uint _3682 = (131072u & kEsiaFxFeatures);
const uint _3817 = (2u & kEsiaFxFeatures);
const uint _3827 = (1024u & kEsiaFxFeatures);
const uint _3892 = (32u & kEsiaFxFeatures);
const uint _3900 = (4u & kEsiaFxFeatures);
const uint _3905 = (256u & kEsiaFxFeatures);
const uint _3969 = (8u & kEsiaFxFeatures);
const uint _3998 = (8192u & kEsiaFxFeatures);
const uint _4214 = (65536u & kEsiaFxFeatures);
const uint _4290 = (32u & kEsiaFxFeatures);
const uint _4354 = (1u & kEsiaFxFeatures);
const uint _4383 = (64u & kEsiaFxFeatures);
const uint _4436 = (16384u & kEsiaFxFeatures);
const vec4 _4598[4] = vec4[](vec4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), vec4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), vec4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), vec4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875));
const vec2 _4615[4] = vec2[](vec2(0.0), vec2(0.100000001490116119384765625, 1.2999999523162841796875), vec2(0.550000011920928955078125, 3.900000095367431640625), vec2(0.20000000298023223876953125, 5.19999980926513671875));
const uint _4867 = (4u & kEsiaFxFeatures);
const uint _4871 = (256u & kEsiaFxFeatures);
const uint _4931 = (16u & kEsiaFxFeatures);
const uint _5000 = (2u & kEsiaFxFeatures);
const uint _5133 = (128u & kEsiaFxFeatures);
const uint _5215 = (2048u & kEsiaFxFeatures);
const uint _5255 = (8u & kEsiaFxFeatures);
const uint _5259 = (4u & kEsiaFxFeatures);

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _292;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _334;

uniform sampler2D SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_CombinedgTexgLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop0gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop1gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop2gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop3gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop4gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop5gLinear;

in vec2 esia_v0;
flat in uint esia_v1;
flat in vec4 esia_v2;
flat in vec4 esia_v3;
flat in vec4 esia_v4;
flat in vec4 esia_v5;
flat in vec4 esia_v6;
flat in uvec4 esia_v7;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec2 _7024 = gl_FragCoord.xy + _292.gConv.yy;
    vec2 _41461;
    if (_292.gConv.x > 0.5)
    {
        vec2 _39138 = _7024;
        _39138.y = _292.gTarget.y - _7024.y;
        _41461 = _39138;
    }
    else
    {
        _41461 = _7024;
    }
    float _5625 = dFdx(esia_v0.x);
    float _5629 = dFdy(esia_v0.x);
    float _5632 = max(abs(_5625) + abs(_5629), 9.9999997473787516355514526367188e-05);
    vec4 _41466;
    vec4 _41469;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.z == 2u) || ((esia_v7.x & _3629) != 0u))
    {
        uint _7043 = max(uint(_292.gConv.z), 1u);
        uint _7084 = max(uint(_292.gConv.z), 1u);
        _41469 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _7043) * 24u) + 15u), int(esia_v1 / _7043), 0).xy, 0);
        _41466 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _7084) * 24u) + 16u), int(esia_v1 / _7084), 0).xy, 0);
    }
    else
    {
        _41469 = vec4(0.0);
        _41466 = vec4(0.0);
    }
    vec4 _41462;
    vec4 _41468;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _3649) != 0u)
    {
        uint _7125 = max(uint(_292.gConv.z), 1u);
        uint _7166 = max(uint(_292.gConv.z), 1u);
        _41468 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _7125) * 24u) + 15u), int(esia_v1 / _7125), 0).xy, 0);
        _41462 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _7166) * 24u) + 18u), int(esia_v1 / _7166), 0).xy, 0);
    }
    else
    {
        _41468 = _41469;
        _41462 = vec4(0.0);
    }
    vec2 _7245 = (esia_v2.xy + esia_v2.zw) * 0.5;
    vec2 _7254 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
    float _41577;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _2279) != 0u)
    {
        float _7371 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
        vec4 _7405 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_41462.xy, _41462.zw, (clamp(esia_v0, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
        float _7429 = (_7405.w - 0.5) * esia_v3.y;
        vec2 _7470 = abs(esia_v0 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _7371), vec2(0.0));
        _41577 = max((-mix((max(min(_7405.x, _7405.y), min(max(_7405.x, _7405.y), _7405.z)) - 0.5) * esia_v3.x, _7429, clamp(abs(_7429) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _7371, length(max(_7470, vec2(0.0))) + min(max(_7470.x, _7470.y), 0.0)) - esia_v5.w;
    }
    else
    {
        float _41578;
        if ((esia_v7.x & _2294) != 0u)
        {
            float _41525;
            do
            {
                bool _7536 = esia_v3.w > 0.5;
                float _7540 = floor((esia_v3.z + 1.0) * 0.5);
                float _7544 = floor((_7540 + 127.0) * 0.0078125);
                float _41518;
                float _41520;
                SPIRV_CROSS_BRANCH
                if (_7536)
                {
                    uint _7720 = uint(esia_v3.z);
                    uint _7723 = (_7720 + 31u) >> 5u;
                    uint _7726 = esia_v1 + 1u;
                    uint _7809 = max(uint(_292.gConv.z), 1u);
                    ivec2 _7819 = ivec2(int((_7726 % _7809) * 24u), int(_7726 / _7809));
                    ivec2 _41510;
                    uint _41511;
                    ivec2 _41571;
                    _41571 = _7819;
                    _41511 = 0u;
                    _41510 = _7819;
                    float _49390;
                    uint _49391;
                    ivec2 _49393;
                    ivec2 _41574;
                    uint _41509 = 0u;
                    float _41572 = 3.0000000054977557577780399428115e+38;
                    SPIRV_CROSS_LOOP
                    for (; _41509 < _7723; _41572 = _49390, _41571 = _41574, _41511 = _49391, _41510 = _49393, _41509++)
                    {
                        vec4 _7847 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41571.x + 20, _41571.y, 0).xy, 0);
                        vec2 _7860 = max(max(_7847.xy - esia_v0, esia_v0 - _7847.zw), vec2(0.0));
                        float _7863 = dot(_7860, _7860);
                        bool _7741 = _7863 < _41572;
                        _49390 = _7741 ? _7863 : _41572;
                        _49391 = _7741 ? _41509 : _41511;
                        bvec2 _49392 = bvec2(_7741);
                        _49393 = ivec2(_49392.x ? _41571.x : _41510.x, _49392.y ? _41571.y : _41510.y);
                        int _7868 = _41571.x + 24;
                        ivec2 _39168 = _41571;
                        _39168.x = _7868;
                        if (_7868 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                        {
                            _41574 = ivec2(0, _41571.y + 1);
                        }
                        else
                        {
                            _41574 = _39168;
                        }
                    }
                    uint _7923 = _41511 * 32u;
                    uint _7927 = min((_7720 - _7923), 32u);
                    vec4 _8054 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41510.x + 21, _41510.y, 0).xy, 0);
                    vec4 _8082 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41510.x + 22, _41510.y, 0).xy, 0);
                    float _41514;
                    uint _41515;
                    _41515 = 4294967295u;
                    _41514 = 1000000015047466219876688855040.0;
                    float _48396;
                    uint _48397;
                    SPIRV_CROSS_LOOP
                    for (uint _41513 = 0u; (_41513 * 8u) < _7927; _41515 = _48397, _41514 = _48396, _41513++)
                    {
                        vec4 _8110 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41510.x + int(16u + _41513), _41510.y, 0).xy, 0);
                        float _8128 = ((sqrt(_41514) * 1.000003814697265625) + 0.03125) + ((abs(esia_v0.x) + abs(esia_v0.y)) * 3.814697265625e-06);
                        vec2 _8148 = max(max(_8110.xy - esia_v0, esia_v0 - _8110.zw), vec2(0.0));
                        if (dot(_8148, _8148) > (_8128 * _8128))
                        {
                            _48397 = _41515;
                            _48396 = _41514;
                            continue;
                        }
                        bvec4 _7952 = bvec4(_41513 < 2u);
                        vec4 _7953 = vec4(_7952.x ? _8054.x : _8082.x, _7952.y ? _8054.y : _8082.y, _7952.z ? _8054.z : _8082.z, _7952.w ? _8054.w : _8082.w);
                        vec2 _7958 = _7953.zw;
                        vec2 _7960 = _7953.xy;
                        bvec2 _7961 = bvec2((_41513 & 1u) != 0u);
                        uint _7967 = min(_7927, ((_41513 * 8u) + 8u));
                        uint _7969 = _41513 * 8u;
                        vec2 _41556;
                        float _41557;
                        uint _41558;
                        _41558 = _41515;
                        _41557 = _41514;
                        _41556 = vec2(_7961.x ? _7958.x : _7960.x, _7961.y ? _7958.y : _7960.y);
                        vec2 _48445;
                        float _48446;
                        uint _48447;
                        SPIRV_CROSS_LOOP
                        for (uint _41555 = _7969; _41555 < _7967; _41558 = _48447, _41557 = _48446, _41556 = _48445, _41555 += 2u)
                        {
                            vec4 _8179 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41510.x + int(_41555 >> 1u), _41510.y, 0).xy, 0);
                            uint _7982 = _7923 + _41555;
                            vec2 _7985 = _8179.xy;
                            vec2 _8187 = _41556 - _7985;
                            vec2 _8190 = esia_v0 - _7985;
                            vec2 _8203 = _8190 - (_8187 * clamp(dot(_8190, _8187) / max(dot(_8187, _8187), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _8206 = dot(_8203, _8203);
                            bool _8217 = (_8206 < _41557) || ((_8206 == _41557) && (_7982 < _41558));
                            uint _49394 = _8217 ? _7982 : _41558;
                            float _49395 = _8217 ? _8206 : _41557;
                            vec2 _7995 = _8179.xy;
                            if ((_41555 + 1u) < _7967)
                            {
                                uint _8004 = (_7923 + _41555) + 1u;
                                vec2 _8007 = _8179.zw;
                                vec2 _8230 = _7995 - _8007;
                                vec2 _8233 = esia_v0 - _8007;
                                vec2 _8246 = _8233 - (_8230 * clamp(dot(_8233, _8230) / max(dot(_8230, _8230), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _8249 = dot(_8246, _8246);
                                bool _8260 = (_8249 < _49395) || ((_8249 == _49395) && (_8004 < _49394));
                                _48447 = _8260 ? _8004 : _49394;
                                _48446 = _8260 ? _8249 : _49395;
                                _48445 = _8179.zw;
                            }
                            else
                            {
                                _48447 = _49394;
                                _48446 = _49395;
                                _48445 = _7995;
                            }
                        }
                        _48397 = _41558;
                        _48396 = _41557;
                    }
                    uint _7766 = esia_v1 + 1u;
                    uint _8272 = max(uint(_292.gConv.z), 1u);
                    float _41517;
                    ivec2 _41528;
                    _41528 = ivec2(int((_7766 % _8272) * 24u), int(_7766 / _8272));
                    _41517 = _41514;
                    ivec2 _41535;
                    float _48399;
                    uint _48429;
                    SPIRV_CROSS_LOOP
                    for (uint _41516 = 0u, _41529 = _41515; _41516 < _7723; _41529 = _48429, _41528 = _41535, _41517 = _48399, _41516++)
                    {
                        vec4 _8310 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41528.x + 20, _41528.y, 0).xy, 0);
                        float _8328 = ((sqrt(_41517) * 1.000003814697265625) + 0.03125) + ((abs(esia_v0.x) + abs(esia_v0.y)) * 3.814697265625e-06);
                        vec2 _8348 = max(max(_8310.xy - esia_v0, esia_v0 - _8310.zw), vec2(0.0));
                        if ((_41516 != _41511) && (!(dot(_8348, _8348) > (_8328 * _8328))))
                        {
                            uint _8389 = _41516 * 32u;
                            uint _8393 = min((_7720 - _8389), 32u);
                            vec4 _8520 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41528.x + 21, _41528.y, 0).xy, 0);
                            vec4 _8548 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41528.x + 22, _41528.y, 0).xy, 0);
                            float _41531;
                            uint _41532;
                            _41532 = _41529;
                            _41531 = _41517;
                            float _48430;
                            uint _48431;
                            SPIRV_CROSS_LOOP
                            for (uint _41530 = 0u; (_41530 * 8u) < _8393; _41532 = _48431, _41531 = _48430, _41530++)
                            {
                                vec4 _8576 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41528.x + int(16u + _41530), _41528.y, 0).xy, 0);
                                float _8594 = ((sqrt(_41531) * 1.000003814697265625) + 0.03125) + ((abs(esia_v0.x) + abs(esia_v0.y)) * 3.814697265625e-06);
                                vec2 _8614 = max(max(_8576.xy - esia_v0, esia_v0 - _8576.zw), vec2(0.0));
                                if (dot(_8614, _8614) > (_8594 * _8594))
                                {
                                    _48431 = _41532;
                                    _48430 = _41531;
                                    continue;
                                }
                                bvec4 _8418 = bvec4(_41530 < 2u);
                                vec4 _8419 = vec4(_8418.x ? _8520.x : _8548.x, _8418.y ? _8520.y : _8548.y, _8418.z ? _8520.z : _8548.z, _8418.w ? _8520.w : _8548.w);
                                vec2 _8424 = _8419.zw;
                                vec2 _8426 = _8419.xy;
                                bvec2 _8427 = bvec2((_41530 & 1u) != 0u);
                                uint _8433 = min(_8393, ((_41530 * 8u) + 8u));
                                uint _8435 = _41530 * 8u;
                                vec2 _41540;
                                float _41541;
                                uint _41542;
                                _41542 = _41532;
                                _41541 = _41531;
                                _41540 = vec2(_8427.x ? _8424.x : _8426.x, _8427.y ? _8424.y : _8426.y);
                                vec2 _48442;
                                float _48443;
                                uint _48444;
                                SPIRV_CROSS_LOOP
                                for (uint _41539 = _8435; _41539 < _8433; _41542 = _48444, _41541 = _48443, _41540 = _48442, _41539 += 2u)
                                {
                                    vec4 _8645 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41528.x + int(_41539 >> 1u), _41528.y, 0).xy, 0);
                                    uint _8448 = _8389 + _41539;
                                    vec2 _8451 = _8645.xy;
                                    vec2 _8653 = _41540 - _8451;
                                    vec2 _8656 = esia_v0 - _8451;
                                    vec2 _8669 = _8656 - (_8653 * clamp(dot(_8656, _8653) / max(dot(_8653, _8653), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _8672 = dot(_8669, _8669);
                                    bool _8683 = (_8672 < _41541) || ((_8672 == _41541) && (_8448 < _41542));
                                    uint _49398 = _8683 ? _8448 : _41542;
                                    float _49399 = _8683 ? _8672 : _41541;
                                    vec2 _8461 = _8645.xy;
                                    if ((_41539 + 1u) < _8433)
                                    {
                                        uint _8470 = (_8389 + _41539) + 1u;
                                        vec2 _8473 = _8645.zw;
                                        vec2 _8696 = _8461 - _8473;
                                        vec2 _8699 = esia_v0 - _8473;
                                        vec2 _8712 = _8699 - (_8696 * clamp(dot(_8699, _8696) / max(dot(_8696, _8696), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _8715 = dot(_8712, _8712);
                                        bool _8726 = (_8715 < _49399) || ((_8715 == _49399) && (_8470 < _49398));
                                        _48444 = _8726 ? _8470 : _49398;
                                        _48443 = _8726 ? _8715 : _49399;
                                        _48442 = _8645.zw;
                                    }
                                    else
                                    {
                                        _48444 = _49398;
                                        _48443 = _49399;
                                        _48442 = _8461;
                                    }
                                }
                                _48431 = _41542;
                                _48430 = _41541;
                            }
                            _48429 = _41532;
                            _48399 = _41531;
                        }
                        else
                        {
                            _48429 = _41529;
                            _48399 = _41517;
                        }
                        int _8736 = _41528.x + 24;
                        ivec2 _39215 = _41528;
                        _39215.x = _8736;
                        if (_8736 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                        {
                            _41535 = ivec2(0, _41528.y + 1);
                        }
                        else
                        {
                            _41535 = _39215;
                        }
                    }
                    _41520 = 1.0;
                    _41518 = _41517;
                }
                else
                {
                    float _7558 = floor((esia_v3.z - 1.0) * 0.5);
                    float _8761 = floor(_7558 * 0.0625);
                    uint _8766 = (esia_v1 + 1u) + uint(_8761);
                    uint _8779 = max(uint(_292.gConv.z), 1u);
                    vec4 _8813 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_8766 % _8779) * 24u) + uint(_7558 - (_8761 * 16.0))), int(_8766 / _8779), 0).xy, 0);
                    vec2 _7570 = _8813.xy;
                    vec2 _7572 = _8813.zw;
                    bvec2 _7573 = bvec2(((esia_v3.z - 1.0) - (_7558 * 2.0)) < 0.5);
                    vec2 _41488;
                    if (_7536)
                    {
                        float _8821 = floor(0.0);
                        uint _8826 = (esia_v1 + 1u) + uint(_8821);
                        uint _8839 = max(uint(_292.gConv.z), 1u);
                        _41488 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_8826 % _8839) * 24u) + uint(_8821 * (-16.0))), int(_8826 / _8839), 0).xy, 0).xy;
                    }
                    else
                    {
                        _41488 = vec2(_7573.x ? _7570.x : _7572.x, _7573.y ? _7570.y : _7572.y);
                    }
                    vec2 _41487;
                    float _41490;
                    float _41493;
                    _41493 = 1.0;
                    _41490 = 1000000015047466219876688855040.0;
                    _41487 = _41488;
                    vec2 _41486;
                    float _41489;
                    float _41492;
                    SPIRV_CROSS_LOOP
                    for (float _41483 = 0.0; _41483 < 16.0; _41493 = _41492, _41490 = _41489, _41487 = _41486, _41483 += 1.0)
                    {
                        if (_41483 >= _7544)
                        {
                            break;
                        }
                        _41492 = _41493;
                        _41489 = _41490;
                        _41486 = _41487;
                        vec2 _48385;
                        float _48387;
                        float _48389;
                        SPIRV_CROSS_LOOP
                        for (float _41484 = 0.0; _41484 < 128.0; _41492 = _48389, _41489 = _48387, _41486 = _48385, _41484 += 1.0)
                        {
                            float _7600 = (_41483 * 128.0) + _41484;
                            if (_7600 >= _7540)
                            {
                                break;
                            }
                            float _8881 = floor(_7600 * 0.0625);
                            uint _8886 = (esia_v1 + 1u) + uint(_8881);
                            uint _8899 = max(uint(_292.gConv.z), 1u);
                            vec4 _8933 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_8886 % _8899) * 24u) + uint(_7600 - (_8881 * 16.0))), int(_8886 / _8899), 0).xy, 0);
                            vec2 _7612 = _8933.xy;
                            vec2 _8944 = _41486 - _7612;
                            vec2 _8947 = esia_v0 - _7612;
                            vec2 _8960 = _8947 - (_8944 * clamp(dot(_8947, _8944) / max(dot(_8944, _8944), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                            float _8963 = dot(_8960, _8960);
                            float _49402 = (_8963 < _41489) ? _8963 : _41489;
                            bool _8975 = esia_v0.y >= _8933.y;
                            bool _8980 = esia_v0.y < _41486.y;
                            bool _8991 = (_8944.x * _8947.y) > (_8944.y * _8947.x);
                            float _41498;
                            if (((_8975 && _8980) && _8991) || (((!_8975) && (!_8980)) && (!_8991)))
                            {
                                _41498 = -_41492;
                            }
                            else
                            {
                                _41498 = _41492;
                            }
                            vec2 _7622 = _8933.xy;
                            if (((2.0 * _7600) + 1.0) < esia_v3.z)
                            {
                                vec2 _7631 = _8933.zw;
                                vec2 _9020 = _7622 - _7631;
                                vec2 _9023 = esia_v0 - _7631;
                                vec2 _9036 = _9023 - (_9020 * clamp(dot(_9023, _9020) / max(dot(_9020, _9020), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _9039 = dot(_9036, _9036);
                                bool _9051 = esia_v0.y >= _8933.w;
                                bool _9056 = esia_v0.y < _8933.y;
                                bool _9067 = (_9020.x * _9023.y) > (_9020.y * _9023.x);
                                float _41502;
                                if (((_9051 && _9056) && _9067) || (((!_9051) && (!_9056)) && (!_9067)))
                                {
                                    _41502 = -_41498;
                                }
                                else
                                {
                                    _41502 = _41498;
                                }
                                _48389 = _41502;
                                _48387 = (_9039 < _49402) ? _9039 : _49402;
                                _48385 = _8933.zw;
                            }
                            else
                            {
                                _48389 = _41498;
                                _48387 = _49402;
                                _48385 = _7622;
                            }
                        }
                    }
                    _41520 = _41493;
                    _41518 = _41490;
                }
                float _7653 = sqrt(_41518);
                if (_7536)
                {
                    _41525 = _7653 - esia_v3.x;
                    break;
                }
                _41525 = (_41520 * _7653) - esia_v3.x;
                break;
            } while(false);
            _41578 = _41525;
        }
        else
        {
            float _41579;
            if (esia_v7.z == 1u)
            {
                vec2 _7279 = esia_v0 - _7245;
                float _41482;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _41482 = abs(length(_7279) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _9110 = esia_v5.z + (esia_v5.w * 0.5);
                    float _9112 = cos(_9110);
                    float _9114 = sin(_9110);
                    float _9123 = dot(_7279, vec2(-_9114, _9112));
                    float _9126 = dot(_7279, vec2(_9112, _9114));
                    vec2 _9127 = vec2(_9123, _9126);
                    float _9130 = abs(_9123);
                    _9127.x = _9130;
                    float _9133 = esia_v5.w * 0.5;
                    float _9135 = sin(_9133);
                    float _9137 = cos(_9133);
                    _41482 = (((_9137 * _9130) > (_9135 * _9126)) ? length(_9127 - (vec2(_9135, _9137) * esia_v3.x)) : abs(length(_9127) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _41579 = _41482;
            }
            else
            {
                float _41580;
                if (esia_v7.z == 2u)
                {
                    vec2 _9173 = esia_v0 - _41468.xy;
                    vec2 _9176 = _41468.zw - _41468.xy;
                    _41580 = length(_9173 - (_9176 * clamp(dot(_9173, _9176) / max(dot(_9176, _9176), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _7306 = esia_v0 - _7245;
                    float _9241 = min(_7254.x, _7254.y);
                    float _9244 = min((_7306.x > 0.0) ? ((_7306.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_7306.y > 0.0) ? esia_v3.w : esia_v3.x), _9241);
                    float _9250 = _9244 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _41471;
                    float _41472;
                    if (_9250 > _9241)
                    {
                        float _9264 = esia_v5.y * clamp((_9241 - _9244) / max(0.60000002384185791015625 * _9244, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _41472 = _9264;
                        _41471 = _9244 * (1.0 + (0.60000002384185791015625 * _9264));
                    }
                    else
                    {
                        _41472 = esia_v5.y;
                        _41471 = _9250;
                    }
                    vec2 _9293 = (abs(_7306) - _7254) + vec2(_41471);
                    vec2 _9295 = max(_9293, vec2(0.0));
                    float _41473;
                    SPIRV_CROSS_BRANCH
                    if ((_9295.x > 0.0) && (_9295.y > 0.0))
                    {
                        float _41474;
                        if ((_41472 > 0.001000000047497451305389404296875) && (_41471 > 9.9999997473787516355514526367188e-05))
                        {
                            float _9312 = 2.0 + (2.0 * _41472);
                            vec2 _9317 = _9295 / vec2(max(_41471, 9.9999997473787516355514526367188e-05));
                            _41474 = pow(pow(_9317.x, _9312) + pow(_9317.y, _9312), 1.0 / _9312) * _41471;
                        }
                        else
                        {
                            _41474 = length(_9295);
                        }
                        _41473 = _41474;
                    }
                    else
                    {
                        _41473 = max(_9295.x, _9295.y);
                    }
                    float _9352 = (min(max(_9293.x, _9293.y), 0.0) + _41473) - _41471;
                    float _41581;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _2365) != 0u)
                    {
                        vec2 _7334 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _7337 = esia_v0 - ((_41468.xy + _41468.zw) * 0.5);
                        float _9400 = min(_7334.x, _7334.y);
                        float _9403 = min((_7337.x > 0.0) ? ((_7337.y > 0.0) ? _41466.x : _41466.x) : ((_7337.y > 0.0) ? _41466.x : _41466.x), _9400);
                        float _9409 = _9403 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _41475;
                        float _41476;
                        if (_9409 > _9400)
                        {
                            float _9423 = esia_v5.y * clamp((_9400 - _9403) / max(0.60000002384185791015625 * _9403, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _41476 = _9423;
                            _41475 = _9403 * (1.0 + (0.60000002384185791015625 * _9423));
                        }
                        else
                        {
                            _41476 = esia_v5.y;
                            _41475 = _9409;
                        }
                        vec2 _9452 = (abs(_7337) - _7334) + vec2(_41475);
                        vec2 _9454 = max(_9452, vec2(0.0));
                        float _41477;
                        SPIRV_CROSS_BRANCH
                        if ((_9454.x > 0.0) && (_9454.y > 0.0))
                        {
                            float _41478;
                            if ((_41476 > 0.001000000047497451305389404296875) && (_41475 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9471 = 2.0 + (2.0 * _41476);
                                vec2 _9476 = _9454 / vec2(max(_41475, 9.9999997473787516355514526367188e-05));
                                _41478 = pow(pow(_9476.x, _9471) + pow(_9476.y, _9471), 1.0 / _9471) * _41475;
                            }
                            else
                            {
                                _41478 = length(_9454);
                            }
                            _41477 = _41478;
                        }
                        else
                        {
                            _41477 = max(_9454.x, _9454.y);
                        }
                        float _9511 = (min(max(_9452.x, _9452.y), 0.0) + _41477) - _41475;
                        float _9516 = max(_41466.y, 9.9999997473787516355514526367188e-05);
                        float _9525 = max(_9516 - abs(_9352 - _9511), 0.0) / _9516;
                        _41581 = min(_9352, _9511) - (((_9525 * _9525) * _9516) * 0.25);
                    }
                    else
                    {
                        _41581 = _9352;
                    }
                    _41580 = _41581;
                }
                _41579 = _41580;
            }
            _41578 = _41579;
        }
        _41577 = _41578;
    }
    float _42238;
    float _47433;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _3682) != 0u)
    {
        uint _9542 = max(uint(_292.gConv.z), 1u);
        uint _9583 = max(uint(_292.gConv.z), 1u);
        vec4 _9617 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _9583) * 24u) + 6u), int(esia_v1 / _9583), 0).xy, 0);
        vec2 _5685 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _9542) * 24u) + 21u), int(esia_v1 / _9542), 0).xy, 0).xy;
        float _5692 = clamp(ceil(length(_5685) / (4.0 * _5632)), 1.0, 8.0);
        float _5697 = _9617.x * _9617.y;
        float _5703 = _9617.x * (1.0 - _9617.y);
        vec2 _5707 = esia_v0 + (_5685 * 0.5);
        vec2 _9662 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _9671 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _41814;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & _2279) != 0u)
        {
            float _9788 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
            vec4 _9822 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_41462.xy, _41462.zw, (clamp(_5707, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
            float _9846 = (_9822.w - 0.5) * esia_v3.y;
            vec2 _9887 = abs(_5707 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _9788), vec2(0.0));
            _41814 = max((-mix((max(min(_9822.x, _9822.y), min(max(_9822.x, _9822.y), _9822.z)) - 0.5) * esia_v3.x, _9846, clamp(abs(_9846) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _9788, length(max(_9887, vec2(0.0))) + min(max(_9887.x, _9887.y), 0.0)) - esia_v5.w;
        }
        else
        {
            float _41815;
            if ((esia_v7.x & _2294) != 0u)
            {
                float _41762;
                do
                {
                    bool _9953 = esia_v3.w > 0.5;
                    float _9957 = floor((esia_v3.z + 1.0) * 0.5);
                    float _9961 = floor((_9957 + 127.0) * 0.0078125);
                    float _41755;
                    float _41757;
                    SPIRV_CROSS_BRANCH
                    if (_9953)
                    {
                        uint _10137 = uint(esia_v3.z);
                        uint _10140 = (_10137 + 31u) >> 5u;
                        uint _10143 = esia_v1 + 1u;
                        uint _10226 = max(uint(_292.gConv.z), 1u);
                        ivec2 _10236 = ivec2(int((_10143 % _10226) * 24u), int(_10143 / _10226));
                        ivec2 _41747;
                        uint _41748;
                        ivec2 _41808;
                        _41808 = _10236;
                        _41748 = 0u;
                        _41747 = _10236;
                        float _49404;
                        uint _49405;
                        ivec2 _49407;
                        ivec2 _41811;
                        uint _41746 = 0u;
                        float _41809 = 3.0000000054977557577780399428115e+38;
                        SPIRV_CROSS_LOOP
                        for (; _41746 < _10140; _41809 = _49404, _41808 = _41811, _41748 = _49405, _41747 = _49407, _41746++)
                        {
                            vec4 _10264 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41808.x + 20, _41808.y, 0).xy, 0);
                            vec2 _10277 = max(max(_10264.xy - _5707, _5707 - _10264.zw), vec2(0.0));
                            float _10280 = dot(_10277, _10277);
                            bool _10158 = _10280 < _41809;
                            _49404 = _10158 ? _10280 : _41809;
                            _49405 = _10158 ? _41746 : _41748;
                            bvec2 _49406 = bvec2(_10158);
                            _49407 = ivec2(_49406.x ? _41808.x : _41747.x, _49406.y ? _41808.y : _41747.y);
                            int _10285 = _41808.x + 24;
                            ivec2 _39322 = _41808;
                            _39322.x = _10285;
                            if (_10285 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                            {
                                _41811 = ivec2(0, _41808.y + 1);
                            }
                            else
                            {
                                _41811 = _39322;
                            }
                        }
                        uint _10340 = _41748 * 32u;
                        uint _10344 = min((_10137 - _10340), 32u);
                        vec4 _10471 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41747.x + 21, _41747.y, 0).xy, 0);
                        vec4 _10499 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41747.x + 22, _41747.y, 0).xy, 0);
                        float _41751;
                        uint _41752;
                        _41752 = 4294967295u;
                        _41751 = 1000000015047466219876688855040.0;
                        float _48467;
                        uint _48468;
                        SPIRV_CROSS_LOOP
                        for (uint _41750 = 0u; (_41750 * 8u) < _10344; _41752 = _48468, _41751 = _48467, _41750++)
                        {
                            vec4 _10527 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41747.x + int(16u + _41750), _41747.y, 0).xy, 0);
                            float _10545 = ((sqrt(_41751) * 1.000003814697265625) + 0.03125) + ((abs(_5707.x) + abs(_5707.y)) * 3.814697265625e-06);
                            vec2 _10565 = max(max(_10527.xy - _5707, _5707 - _10527.zw), vec2(0.0));
                            if (dot(_10565, _10565) > (_10545 * _10545))
                            {
                                _48468 = _41752;
                                _48467 = _41751;
                                continue;
                            }
                            bvec4 _10369 = bvec4(_41750 < 2u);
                            vec4 _10370 = vec4(_10369.x ? _10471.x : _10499.x, _10369.y ? _10471.y : _10499.y, _10369.z ? _10471.z : _10499.z, _10369.w ? _10471.w : _10499.w);
                            vec2 _10375 = _10370.zw;
                            vec2 _10377 = _10370.xy;
                            bvec2 _10378 = bvec2((_41750 & 1u) != 0u);
                            uint _10384 = min(_10344, ((_41750 * 8u) + 8u));
                            uint _10386 = _41750 * 8u;
                            vec2 _41793;
                            float _41794;
                            uint _41795;
                            _41795 = _41752;
                            _41794 = _41751;
                            _41793 = vec2(_10378.x ? _10375.x : _10377.x, _10378.y ? _10375.y : _10377.y);
                            vec2 _48516;
                            float _48517;
                            uint _48518;
                            SPIRV_CROSS_LOOP
                            for (uint _41792 = _10386; _41792 < _10384; _41795 = _48518, _41794 = _48517, _41793 = _48516, _41792 += 2u)
                            {
                                vec4 _10596 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41747.x + int(_41792 >> 1u), _41747.y, 0).xy, 0);
                                uint _10399 = _10340 + _41792;
                                vec2 _10402 = _10596.xy;
                                vec2 _10604 = _41793 - _10402;
                                vec2 _10607 = _5707 - _10402;
                                vec2 _10620 = _10607 - (_10604 * clamp(dot(_10607, _10604) / max(dot(_10604, _10604), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _10623 = dot(_10620, _10620);
                                bool _10634 = (_10623 < _41794) || ((_10623 == _41794) && (_10399 < _41795));
                                uint _49408 = _10634 ? _10399 : _41795;
                                float _49409 = _10634 ? _10623 : _41794;
                                vec2 _10412 = _10596.xy;
                                if ((_41792 + 1u) < _10384)
                                {
                                    uint _10421 = (_10340 + _41792) + 1u;
                                    vec2 _10424 = _10596.zw;
                                    vec2 _10647 = _10412 - _10424;
                                    vec2 _10650 = _5707 - _10424;
                                    vec2 _10663 = _10650 - (_10647 * clamp(dot(_10650, _10647) / max(dot(_10647, _10647), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _10666 = dot(_10663, _10663);
                                    bool _10677 = (_10666 < _49409) || ((_10666 == _49409) && (_10421 < _49408));
                                    _48518 = _10677 ? _10421 : _49408;
                                    _48517 = _10677 ? _10666 : _49409;
                                    _48516 = _10596.zw;
                                }
                                else
                                {
                                    _48518 = _49408;
                                    _48517 = _49409;
                                    _48516 = _10412;
                                }
                            }
                            _48468 = _41795;
                            _48467 = _41794;
                        }
                        uint _10183 = esia_v1 + 1u;
                        uint _10689 = max(uint(_292.gConv.z), 1u);
                        float _41754;
                        ivec2 _41765;
                        _41765 = ivec2(int((_10183 % _10689) * 24u), int(_10183 / _10689));
                        _41754 = _41751;
                        ivec2 _41772;
                        float _48470;
                        uint _48500;
                        SPIRV_CROSS_LOOP
                        for (uint _41753 = 0u, _41766 = _41752; _41753 < _10140; _41766 = _48500, _41765 = _41772, _41754 = _48470, _41753++)
                        {
                            vec4 _10727 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41765.x + 20, _41765.y, 0).xy, 0);
                            float _10745 = ((sqrt(_41754) * 1.000003814697265625) + 0.03125) + ((abs(_5707.x) + abs(_5707.y)) * 3.814697265625e-06);
                            vec2 _10765 = max(max(_10727.xy - _5707, _5707 - _10727.zw), vec2(0.0));
                            if ((_41753 != _41748) && (!(dot(_10765, _10765) > (_10745 * _10745))))
                            {
                                uint _10806 = _41753 * 32u;
                                uint _10810 = min((_10137 - _10806), 32u);
                                vec4 _10937 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41765.x + 21, _41765.y, 0).xy, 0);
                                vec4 _10965 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41765.x + 22, _41765.y, 0).xy, 0);
                                float _41768;
                                uint _41769;
                                _41769 = _41766;
                                _41768 = _41754;
                                float _48501;
                                uint _48502;
                                SPIRV_CROSS_LOOP
                                for (uint _41767 = 0u; (_41767 * 8u) < _10810; _41769 = _48502, _41768 = _48501, _41767++)
                                {
                                    vec4 _10993 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41765.x + int(16u + _41767), _41765.y, 0).xy, 0);
                                    float _11011 = ((sqrt(_41768) * 1.000003814697265625) + 0.03125) + ((abs(_5707.x) + abs(_5707.y)) * 3.814697265625e-06);
                                    vec2 _11031 = max(max(_10993.xy - _5707, _5707 - _10993.zw), vec2(0.0));
                                    if (dot(_11031, _11031) > (_11011 * _11011))
                                    {
                                        _48502 = _41769;
                                        _48501 = _41768;
                                        continue;
                                    }
                                    bvec4 _10835 = bvec4(_41767 < 2u);
                                    vec4 _10836 = vec4(_10835.x ? _10937.x : _10965.x, _10835.y ? _10937.y : _10965.y, _10835.z ? _10937.z : _10965.z, _10835.w ? _10937.w : _10965.w);
                                    vec2 _10841 = _10836.zw;
                                    vec2 _10843 = _10836.xy;
                                    bvec2 _10844 = bvec2((_41767 & 1u) != 0u);
                                    uint _10850 = min(_10810, ((_41767 * 8u) + 8u));
                                    uint _10852 = _41767 * 8u;
                                    vec2 _41777;
                                    float _41778;
                                    uint _41779;
                                    _41779 = _41769;
                                    _41778 = _41768;
                                    _41777 = vec2(_10844.x ? _10841.x : _10843.x, _10844.y ? _10841.y : _10843.y);
                                    vec2 _48513;
                                    float _48514;
                                    uint _48515;
                                    SPIRV_CROSS_LOOP
                                    for (uint _41776 = _10852; _41776 < _10850; _41779 = _48515, _41778 = _48514, _41777 = _48513, _41776 += 2u)
                                    {
                                        vec4 _11062 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41765.x + int(_41776 >> 1u), _41765.y, 0).xy, 0);
                                        uint _10865 = _10806 + _41776;
                                        vec2 _10868 = _11062.xy;
                                        vec2 _11070 = _41777 - _10868;
                                        vec2 _11073 = _5707 - _10868;
                                        vec2 _11086 = _11073 - (_11070 * clamp(dot(_11073, _11070) / max(dot(_11070, _11070), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _11089 = dot(_11086, _11086);
                                        bool _11100 = (_11089 < _41778) || ((_11089 == _41778) && (_10865 < _41779));
                                        uint _49412 = _11100 ? _10865 : _41779;
                                        float _49413 = _11100 ? _11089 : _41778;
                                        vec2 _10878 = _11062.xy;
                                        if ((_41776 + 1u) < _10850)
                                        {
                                            uint _10887 = (_10806 + _41776) + 1u;
                                            vec2 _10890 = _11062.zw;
                                            vec2 _11113 = _10878 - _10890;
                                            vec2 _11116 = _5707 - _10890;
                                            vec2 _11129 = _11116 - (_11113 * clamp(dot(_11116, _11113) / max(dot(_11113, _11113), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _11132 = dot(_11129, _11129);
                                            bool _11143 = (_11132 < _49413) || ((_11132 == _49413) && (_10887 < _49412));
                                            _48515 = _11143 ? _10887 : _49412;
                                            _48514 = _11143 ? _11132 : _49413;
                                            _48513 = _11062.zw;
                                        }
                                        else
                                        {
                                            _48515 = _49412;
                                            _48514 = _49413;
                                            _48513 = _10878;
                                        }
                                    }
                                    _48502 = _41779;
                                    _48501 = _41778;
                                }
                                _48500 = _41769;
                                _48470 = _41768;
                            }
                            else
                            {
                                _48500 = _41766;
                                _48470 = _41754;
                            }
                            int _11153 = _41765.x + 24;
                            ivec2 _39369 = _41765;
                            _39369.x = _11153;
                            if (_11153 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                            {
                                _41772 = ivec2(0, _41765.y + 1);
                            }
                            else
                            {
                                _41772 = _39369;
                            }
                        }
                        _41757 = 1.0;
                        _41755 = _41754;
                    }
                    else
                    {
                        float _9975 = floor((esia_v3.z - 1.0) * 0.5);
                        float _11178 = floor(_9975 * 0.0625);
                        uint _11183 = (esia_v1 + 1u) + uint(_11178);
                        uint _11196 = max(uint(_292.gConv.z), 1u);
                        vec4 _11230 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_11183 % _11196) * 24u) + uint(_9975 - (_11178 * 16.0))), int(_11183 / _11196), 0).xy, 0);
                        vec2 _9987 = _11230.xy;
                        vec2 _9989 = _11230.zw;
                        bvec2 _9990 = bvec2(((esia_v3.z - 1.0) - (_9975 * 2.0)) < 0.5);
                        vec2 _41725;
                        if (_9953)
                        {
                            float _11238 = floor(0.0);
                            uint _11243 = (esia_v1 + 1u) + uint(_11238);
                            uint _11256 = max(uint(_292.gConv.z), 1u);
                            _41725 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_11243 % _11256) * 24u) + uint(_11238 * (-16.0))), int(_11243 / _11256), 0).xy, 0).xy;
                        }
                        else
                        {
                            _41725 = vec2(_9990.x ? _9987.x : _9989.x, _9990.y ? _9987.y : _9989.y);
                        }
                        vec2 _41724;
                        float _41727;
                        float _41730;
                        _41730 = 1.0;
                        _41727 = 1000000015047466219876688855040.0;
                        _41724 = _41725;
                        vec2 _41723;
                        float _41726;
                        float _41729;
                        SPIRV_CROSS_LOOP
                        for (float _41720 = 0.0; _41720 < 16.0; _41730 = _41729, _41727 = _41726, _41724 = _41723, _41720 += 1.0)
                        {
                            if (_41720 >= _9961)
                            {
                                break;
                            }
                            _41729 = _41730;
                            _41726 = _41727;
                            _41723 = _41724;
                            vec2 _48456;
                            float _48458;
                            float _48460;
                            SPIRV_CROSS_LOOP
                            for (float _41721 = 0.0; _41721 < 128.0; _41729 = _48460, _41726 = _48458, _41723 = _48456, _41721 += 1.0)
                            {
                                float _10017 = (_41720 * 128.0) + _41721;
                                if (_10017 >= _9957)
                                {
                                    break;
                                }
                                float _11298 = floor(_10017 * 0.0625);
                                uint _11303 = (esia_v1 + 1u) + uint(_11298);
                                uint _11316 = max(uint(_292.gConv.z), 1u);
                                vec4 _11350 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_11303 % _11316) * 24u) + uint(_10017 - (_11298 * 16.0))), int(_11303 / _11316), 0).xy, 0);
                                vec2 _10029 = _11350.xy;
                                vec2 _11361 = _41723 - _10029;
                                vec2 _11364 = _5707 - _10029;
                                vec2 _11377 = _11364 - (_11361 * clamp(dot(_11364, _11361) / max(dot(_11361, _11361), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _11380 = dot(_11377, _11377);
                                float _49416 = (_11380 < _41726) ? _11380 : _41726;
                                bool _11392 = _5707.y >= _11350.y;
                                bool _11397 = _5707.y < _41723.y;
                                bool _11408 = (_11361.x * _11364.y) > (_11361.y * _11364.x);
                                float _41735;
                                if (((_11392 && _11397) && _11408) || (((!_11392) && (!_11397)) && (!_11408)))
                                {
                                    _41735 = -_41729;
                                }
                                else
                                {
                                    _41735 = _41729;
                                }
                                vec2 _10039 = _11350.xy;
                                if (((2.0 * _10017) + 1.0) < esia_v3.z)
                                {
                                    vec2 _10048 = _11350.zw;
                                    vec2 _11437 = _10039 - _10048;
                                    vec2 _11440 = _5707 - _10048;
                                    vec2 _11453 = _11440 - (_11437 * clamp(dot(_11440, _11437) / max(dot(_11437, _11437), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _11456 = dot(_11453, _11453);
                                    bool _11468 = _5707.y >= _11350.w;
                                    bool _11473 = _5707.y < _11350.y;
                                    bool _11484 = (_11437.x * _11440.y) > (_11437.y * _11440.x);
                                    float _41739;
                                    if (((_11468 && _11473) && _11484) || (((!_11468) && (!_11473)) && (!_11484)))
                                    {
                                        _41739 = -_41735;
                                    }
                                    else
                                    {
                                        _41739 = _41735;
                                    }
                                    _48460 = _41739;
                                    _48458 = (_11456 < _49416) ? _11456 : _49416;
                                    _48456 = _11350.zw;
                                }
                                else
                                {
                                    _48460 = _41735;
                                    _48458 = _49416;
                                    _48456 = _10039;
                                }
                            }
                        }
                        _41757 = _41730;
                        _41755 = _41727;
                    }
                    float _10070 = sqrt(_41755);
                    if (_9953)
                    {
                        _41762 = _10070 - esia_v3.x;
                        break;
                    }
                    _41762 = (_41757 * _10070) - esia_v3.x;
                    break;
                } while(false);
                _41815 = _41762;
            }
            else
            {
                float _41816;
                if (esia_v7.z == 1u)
                {
                    vec2 _9696 = _5707 - _9662;
                    float _41719;
                    do
                    {
                        if (esia_v5.w >= 6.282185077667236328125)
                        {
                            _41719 = abs(length(_9696) - esia_v3.x) - esia_v3.y;
                            break;
                        }
                        float _11527 = esia_v5.z + (esia_v5.w * 0.5);
                        float _11529 = cos(_11527);
                        float _11531 = sin(_11527);
                        float _11540 = dot(_9696, vec2(-_11531, _11529));
                        float _11543 = dot(_9696, vec2(_11529, _11531));
                        vec2 _11544 = vec2(_11540, _11543);
                        float _11547 = abs(_11540);
                        _11544.x = _11547;
                        float _11550 = esia_v5.w * 0.5;
                        float _11552 = sin(_11550);
                        float _11554 = cos(_11550);
                        _41719 = (((_11554 * _11547) > (_11552 * _11543)) ? length(_11544 - (vec2(_11552, _11554) * esia_v3.x)) : abs(length(_11544) - esia_v3.x)) - esia_v3.y;
                        break;
                    } while(false);
                    _41816 = _41719;
                }
                else
                {
                    float _41817;
                    if (esia_v7.z == 2u)
                    {
                        vec2 _11590 = _5707 - _41468.xy;
                        vec2 _11593 = _41468.zw - _41468.xy;
                        _41817 = length(_11590 - (_11593 * clamp(dot(_11590, _11593) / max(dot(_11593, _11593), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                    }
                    else
                    {
                        vec2 _9723 = _5707 - _9662;
                        float _11658 = min(_9671.x, _9671.y);
                        float _11661 = min((_9723.x > 0.0) ? ((_9723.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_9723.y > 0.0) ? esia_v3.w : esia_v3.x), _11658);
                        float _11667 = _11661 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _41708;
                        float _41709;
                        if (_11667 > _11658)
                        {
                            float _11681 = esia_v5.y * clamp((_11658 - _11661) / max(0.60000002384185791015625 * _11661, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _41709 = _11681;
                            _41708 = _11661 * (1.0 + (0.60000002384185791015625 * _11681));
                        }
                        else
                        {
                            _41709 = esia_v5.y;
                            _41708 = _11667;
                        }
                        vec2 _11710 = (abs(_9723) - _9671) + vec2(_41708);
                        vec2 _11712 = max(_11710, vec2(0.0));
                        float _41710;
                        SPIRV_CROSS_BRANCH
                        if ((_11712.x > 0.0) && (_11712.y > 0.0))
                        {
                            float _41711;
                            if ((_41709 > 0.001000000047497451305389404296875) && (_41708 > 9.9999997473787516355514526367188e-05))
                            {
                                float _11729 = 2.0 + (2.0 * _41709);
                                vec2 _11734 = _11712 / vec2(max(_41708, 9.9999997473787516355514526367188e-05));
                                _41711 = pow(pow(_11734.x, _11729) + pow(_11734.y, _11729), 1.0 / _11729) * _41708;
                            }
                            else
                            {
                                _41711 = length(_11712);
                            }
                            _41710 = _41711;
                        }
                        else
                        {
                            _41710 = max(_11712.x, _11712.y);
                        }
                        float _11769 = (min(max(_11710.x, _11710.y), 0.0) + _41710) - _41708;
                        float _41818;
                        SPIRV_CROSS_BRANCH
                        if ((esia_v7.x & _2365) != 0u)
                        {
                            vec2 _9751 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                            vec2 _9754 = _5707 - ((_41468.xy + _41468.zw) * 0.5);
                            float _11817 = min(_9751.x, _9751.y);
                            float _11820 = min((_9754.x > 0.0) ? ((_9754.y > 0.0) ? _41466.x : _41466.x) : ((_9754.y > 0.0) ? _41466.x : _41466.x), _11817);
                            float _11826 = _11820 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                            float _41712;
                            float _41713;
                            if (_11826 > _11817)
                            {
                                float _11840 = esia_v5.y * clamp((_11817 - _11820) / max(0.60000002384185791015625 * _11820, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _41713 = _11840;
                                _41712 = _11820 * (1.0 + (0.60000002384185791015625 * _11840));
                            }
                            else
                            {
                                _41713 = esia_v5.y;
                                _41712 = _11826;
                            }
                            vec2 _11869 = (abs(_9754) - _9751) + vec2(_41712);
                            vec2 _11871 = max(_11869, vec2(0.0));
                            float _41714;
                            SPIRV_CROSS_BRANCH
                            if ((_11871.x > 0.0) && (_11871.y > 0.0))
                            {
                                float _41715;
                                if ((_41713 > 0.001000000047497451305389404296875) && (_41712 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _11888 = 2.0 + (2.0 * _41713);
                                    vec2 _11893 = _11871 / vec2(max(_41712, 9.9999997473787516355514526367188e-05));
                                    _41715 = pow(pow(_11893.x, _11888) + pow(_11893.y, _11888), 1.0 / _11888) * _41712;
                                }
                                else
                                {
                                    _41715 = length(_11871);
                                }
                                _41714 = _41715;
                            }
                            else
                            {
                                _41714 = max(_11871.x, _11871.y);
                            }
                            float _11928 = (min(max(_11869.x, _11869.y), 0.0) + _41714) - _41712;
                            float _11933 = max(_41466.y, 9.9999997473787516355514526367188e-05);
                            float _11942 = max(_11933 - abs(_11769 - _11928), 0.0) / _11933;
                            _41818 = min(_11769, _11928) - (((_11942 * _11942) * _11933) * 0.25);
                        }
                        else
                        {
                            _41818 = _11769;
                        }
                        _41817 = _41818;
                    }
                    _41816 = _41817;
                }
                _41815 = _41816;
            }
            _41814 = _41815;
        }
        float _42104;
        float _42147;
        _42147 = 0.0;
        _42104 = 0.0;
        float _5742;
        float _5771;
        float _42055;
        int _41819 = 1;
        float _42061 = _41814;
        SPIRV_CROSS_LOOP
        for (; _41819 <= 8; _42147 = _5771, _42104 = _5742, _42061 = _42055, _41819++)
        {
            if (float(_41819) > _5692)
            {
                break;
            }
            vec2 _5729 = esia_v0 + (_5685 * (0.5 - (float(_41819) / _5692)));
            vec2 _11997 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _12006 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            SPIRV_CROSS_BRANCH
            if ((esia_v7.x & _2279) != 0u)
            {
                float _12123 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
                vec4 _12157 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_41462.xy, _41462.zw, (clamp(_5729, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
                float _12181 = (_12157.w - 0.5) * esia_v3.y;
                vec2 _12222 = abs(_5729 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _12123), vec2(0.0));
                _42055 = max((-mix((max(min(_12157.x, _12157.y), min(max(_12157.x, _12157.y), _12157.z)) - 0.5) * esia_v3.x, _12181, clamp(abs(_12181) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _12123, length(max(_12222, vec2(0.0))) + min(max(_12222.x, _12222.y), 0.0)) - esia_v5.w;
            }
            else
            {
                float _42056;
                if ((esia_v7.x & _2294) != 0u)
                {
                    float _42003;
                    do
                    {
                        bool _12288 = esia_v3.w > 0.5;
                        float _12292 = floor((esia_v3.z + 1.0) * 0.5);
                        float _12296 = floor((_12292 + 127.0) * 0.0078125);
                        float _41996;
                        float _41998;
                        SPIRV_CROSS_BRANCH
                        if (_12288)
                        {
                            uint _12472 = uint(esia_v3.z);
                            uint _12475 = (_12472 + 31u) >> 5u;
                            uint _12478 = esia_v1 + 1u;
                            uint _12561 = max(uint(_292.gConv.z), 1u);
                            ivec2 _12571 = ivec2(int((_12478 % _12561) * 24u), int(_12478 / _12561));
                            ivec2 _41988;
                            uint _41989;
                            ivec2 _42049;
                            _42049 = _12571;
                            _41989 = 0u;
                            _41988 = _12571;
                            float _49418;
                            uint _49419;
                            ivec2 _49421;
                            ivec2 _42052;
                            uint _41987 = 0u;
                            float _42050 = 3.0000000054977557577780399428115e+38;
                            SPIRV_CROSS_LOOP
                            for (; _41987 < _12475; _42050 = _49418, _42049 = _42052, _41989 = _49419, _41988 = _49421, _41987++)
                            {
                                vec4 _12599 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42049.x + 20, _42049.y, 0).xy, 0);
                                vec2 _12612 = max(max(_12599.xy - _5729, _5729 - _12599.zw), vec2(0.0));
                                float _12615 = dot(_12612, _12612);
                                bool _12493 = _12615 < _42050;
                                _49418 = _12493 ? _12615 : _42050;
                                _49419 = _12493 ? _41987 : _41989;
                                bvec2 _49420 = bvec2(_12493);
                                _49421 = ivec2(_49420.x ? _42049.x : _41988.x, _49420.y ? _42049.y : _41988.y);
                                int _12620 = _42049.x + 24;
                                ivec2 _39472 = _42049;
                                _39472.x = _12620;
                                if (_12620 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                {
                                    _42052 = ivec2(0, _42049.y + 1);
                                }
                                else
                                {
                                    _42052 = _39472;
                                }
                            }
                            uint _12675 = _41989 * 32u;
                            uint _12679 = min((_12472 - _12675), 32u);
                            vec4 _12806 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41988.x + 21, _41988.y, 0).xy, 0);
                            vec4 _12834 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41988.x + 22, _41988.y, 0).xy, 0);
                            float _41992;
                            uint _41993;
                            _41993 = 4294967295u;
                            _41992 = 1000000015047466219876688855040.0;
                            float _48664;
                            uint _48665;
                            SPIRV_CROSS_LOOP
                            for (uint _41991 = 0u; (_41991 * 8u) < _12679; _41993 = _48665, _41992 = _48664, _41991++)
                            {
                                vec4 _12862 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41988.x + int(16u + _41991), _41988.y, 0).xy, 0);
                                float _12880 = ((sqrt(_41992) * 1.000003814697265625) + 0.03125) + ((abs(_5729.x) + abs(_5729.y)) * 3.814697265625e-06);
                                vec2 _12900 = max(max(_12862.xy - _5729, _5729 - _12862.zw), vec2(0.0));
                                if (dot(_12900, _12900) > (_12880 * _12880))
                                {
                                    _48665 = _41993;
                                    _48664 = _41992;
                                    continue;
                                }
                                bvec4 _12704 = bvec4(_41991 < 2u);
                                vec4 _12705 = vec4(_12704.x ? _12806.x : _12834.x, _12704.y ? _12806.y : _12834.y, _12704.z ? _12806.z : _12834.z, _12704.w ? _12806.w : _12834.w);
                                vec2 _12710 = _12705.zw;
                                vec2 _12712 = _12705.xy;
                                bvec2 _12713 = bvec2((_41991 & 1u) != 0u);
                                uint _12719 = min(_12679, ((_41991 * 8u) + 8u));
                                uint _12721 = _41991 * 8u;
                                vec2 _42034;
                                float _42035;
                                uint _42036;
                                _42036 = _41993;
                                _42035 = _41992;
                                _42034 = vec2(_12713.x ? _12710.x : _12712.x, _12713.y ? _12710.y : _12712.y);
                                vec2 _48713;
                                float _48714;
                                uint _48715;
                                SPIRV_CROSS_LOOP
                                for (uint _42033 = _12721; _42033 < _12719; _42036 = _48715, _42035 = _48714, _42034 = _48713, _42033 += 2u)
                                {
                                    vec4 _12931 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_41988.x + int(_42033 >> 1u), _41988.y, 0).xy, 0);
                                    uint _12734 = _12675 + _42033;
                                    vec2 _12737 = _12931.xy;
                                    vec2 _12939 = _42034 - _12737;
                                    vec2 _12942 = _5729 - _12737;
                                    vec2 _12955 = _12942 - (_12939 * clamp(dot(_12942, _12939) / max(dot(_12939, _12939), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _12958 = dot(_12955, _12955);
                                    bool _12969 = (_12958 < _42035) || ((_12958 == _42035) && (_12734 < _42036));
                                    uint _49422 = _12969 ? _12734 : _42036;
                                    float _49423 = _12969 ? _12958 : _42035;
                                    vec2 _12747 = _12931.xy;
                                    if ((_42033 + 1u) < _12719)
                                    {
                                        uint _12756 = (_12675 + _42033) + 1u;
                                        vec2 _12759 = _12931.zw;
                                        vec2 _12982 = _12747 - _12759;
                                        vec2 _12985 = _5729 - _12759;
                                        vec2 _12998 = _12985 - (_12982 * clamp(dot(_12985, _12982) / max(dot(_12982, _12982), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _13001 = dot(_12998, _12998);
                                        bool _13012 = (_13001 < _49423) || ((_13001 == _49423) && (_12756 < _49422));
                                        _48715 = _13012 ? _12756 : _49422;
                                        _48714 = _13012 ? _13001 : _49423;
                                        _48713 = _12931.zw;
                                    }
                                    else
                                    {
                                        _48715 = _49422;
                                        _48714 = _49423;
                                        _48713 = _12747;
                                    }
                                }
                                _48665 = _42036;
                                _48664 = _42035;
                            }
                            uint _12518 = esia_v1 + 1u;
                            uint _13024 = max(uint(_292.gConv.z), 1u);
                            float _41995;
                            ivec2 _42006;
                            _42006 = ivec2(int((_12518 % _13024) * 24u), int(_12518 / _13024));
                            _41995 = _41992;
                            ivec2 _42013;
                            float _48667;
                            uint _48697;
                            SPIRV_CROSS_LOOP
                            for (uint _41994 = 0u, _42007 = _41993; _41994 < _12475; _42007 = _48697, _42006 = _42013, _41995 = _48667, _41994++)
                            {
                                vec4 _13062 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42006.x + 20, _42006.y, 0).xy, 0);
                                float _13080 = ((sqrt(_41995) * 1.000003814697265625) + 0.03125) + ((abs(_5729.x) + abs(_5729.y)) * 3.814697265625e-06);
                                vec2 _13100 = max(max(_13062.xy - _5729, _5729 - _13062.zw), vec2(0.0));
                                if ((_41994 != _41989) && (!(dot(_13100, _13100) > (_13080 * _13080))))
                                {
                                    uint _13141 = _41994 * 32u;
                                    uint _13145 = min((_12472 - _13141), 32u);
                                    vec4 _13272 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42006.x + 21, _42006.y, 0).xy, 0);
                                    vec4 _13300 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42006.x + 22, _42006.y, 0).xy, 0);
                                    float _42009;
                                    uint _42010;
                                    _42010 = _42007;
                                    _42009 = _41995;
                                    float _48698;
                                    uint _48699;
                                    SPIRV_CROSS_LOOP
                                    for (uint _42008 = 0u; (_42008 * 8u) < _13145; _42010 = _48699, _42009 = _48698, _42008++)
                                    {
                                        vec4 _13328 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42006.x + int(16u + _42008), _42006.y, 0).xy, 0);
                                        float _13346 = ((sqrt(_42009) * 1.000003814697265625) + 0.03125) + ((abs(_5729.x) + abs(_5729.y)) * 3.814697265625e-06);
                                        vec2 _13366 = max(max(_13328.xy - _5729, _5729 - _13328.zw), vec2(0.0));
                                        if (dot(_13366, _13366) > (_13346 * _13346))
                                        {
                                            _48699 = _42010;
                                            _48698 = _42009;
                                            continue;
                                        }
                                        bvec4 _13170 = bvec4(_42008 < 2u);
                                        vec4 _13171 = vec4(_13170.x ? _13272.x : _13300.x, _13170.y ? _13272.y : _13300.y, _13170.z ? _13272.z : _13300.z, _13170.w ? _13272.w : _13300.w);
                                        vec2 _13176 = _13171.zw;
                                        vec2 _13178 = _13171.xy;
                                        bvec2 _13179 = bvec2((_42008 & 1u) != 0u);
                                        uint _13185 = min(_13145, ((_42008 * 8u) + 8u));
                                        uint _13187 = _42008 * 8u;
                                        vec2 _42018;
                                        float _42019;
                                        uint _42020;
                                        _42020 = _42010;
                                        _42019 = _42009;
                                        _42018 = vec2(_13179.x ? _13176.x : _13178.x, _13179.y ? _13176.y : _13178.y);
                                        vec2 _48710;
                                        float _48711;
                                        uint _48712;
                                        SPIRV_CROSS_LOOP
                                        for (uint _42017 = _13187; _42017 < _13185; _42020 = _48712, _42019 = _48711, _42018 = _48710, _42017 += 2u)
                                        {
                                            vec4 _13397 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42006.x + int(_42017 >> 1u), _42006.y, 0).xy, 0);
                                            uint _13200 = _13141 + _42017;
                                            vec2 _13203 = _13397.xy;
                                            vec2 _13405 = _42018 - _13203;
                                            vec2 _13408 = _5729 - _13203;
                                            vec2 _13421 = _13408 - (_13405 * clamp(dot(_13408, _13405) / max(dot(_13405, _13405), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _13424 = dot(_13421, _13421);
                                            bool _13435 = (_13424 < _42019) || ((_13424 == _42019) && (_13200 < _42020));
                                            uint _49426 = _13435 ? _13200 : _42020;
                                            float _49427 = _13435 ? _13424 : _42019;
                                            vec2 _13213 = _13397.xy;
                                            if ((_42017 + 1u) < _13185)
                                            {
                                                uint _13222 = (_13141 + _42017) + 1u;
                                                vec2 _13225 = _13397.zw;
                                                vec2 _13448 = _13213 - _13225;
                                                vec2 _13451 = _5729 - _13225;
                                                vec2 _13464 = _13451 - (_13448 * clamp(dot(_13451, _13448) / max(dot(_13448, _13448), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _13467 = dot(_13464, _13464);
                                                bool _13478 = (_13467 < _49427) || ((_13467 == _49427) && (_13222 < _49426));
                                                _48712 = _13478 ? _13222 : _49426;
                                                _48711 = _13478 ? _13467 : _49427;
                                                _48710 = _13397.zw;
                                            }
                                            else
                                            {
                                                _48712 = _49426;
                                                _48711 = _49427;
                                                _48710 = _13213;
                                            }
                                        }
                                        _48699 = _42020;
                                        _48698 = _42019;
                                    }
                                    _48697 = _42010;
                                    _48667 = _42009;
                                }
                                else
                                {
                                    _48697 = _42007;
                                    _48667 = _41995;
                                }
                                int _13488 = _42006.x + 24;
                                ivec2 _39519 = _42006;
                                _39519.x = _13488;
                                if (_13488 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                {
                                    _42013 = ivec2(0, _42006.y + 1);
                                }
                                else
                                {
                                    _42013 = _39519;
                                }
                            }
                            _41998 = 1.0;
                            _41996 = _41995;
                        }
                        else
                        {
                            float _12310 = floor((esia_v3.z - 1.0) * 0.5);
                            float _13513 = floor(_12310 * 0.0625);
                            uint _13518 = (esia_v1 + 1u) + uint(_13513);
                            uint _13531 = max(uint(_292.gConv.z), 1u);
                            vec4 _13565 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_13518 % _13531) * 24u) + uint(_12310 - (_13513 * 16.0))), int(_13518 / _13531), 0).xy, 0);
                            vec2 _12322 = _13565.xy;
                            vec2 _12324 = _13565.zw;
                            bvec2 _12325 = bvec2(((esia_v3.z - 1.0) - (_12310 * 2.0)) < 0.5);
                            vec2 _41966;
                            if (_12288)
                            {
                                float _13573 = floor(0.0);
                                uint _13578 = (esia_v1 + 1u) + uint(_13573);
                                uint _13591 = max(uint(_292.gConv.z), 1u);
                                _41966 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_13578 % _13591) * 24u) + uint(_13573 * (-16.0))), int(_13578 / _13591), 0).xy, 0).xy;
                            }
                            else
                            {
                                _41966 = vec2(_12325.x ? _12322.x : _12324.x, _12325.y ? _12322.y : _12324.y);
                            }
                            vec2 _41965;
                            float _41968;
                            float _41971;
                            _41971 = 1.0;
                            _41968 = 1000000015047466219876688855040.0;
                            _41965 = _41966;
                            vec2 _41964;
                            float _41967;
                            float _41970;
                            SPIRV_CROSS_LOOP
                            for (float _41961 = 0.0; _41961 < 16.0; _41971 = _41970, _41968 = _41967, _41965 = _41964, _41961 += 1.0)
                            {
                                if (_41961 >= _12296)
                                {
                                    break;
                                }
                                _41970 = _41971;
                                _41967 = _41968;
                                _41964 = _41965;
                                vec2 _48653;
                                float _48655;
                                float _48657;
                                SPIRV_CROSS_LOOP
                                for (float _41962 = 0.0; _41962 < 128.0; _41970 = _48657, _41967 = _48655, _41964 = _48653, _41962 += 1.0)
                                {
                                    float _12352 = (_41961 * 128.0) + _41962;
                                    if (_12352 >= _12292)
                                    {
                                        break;
                                    }
                                    float _13633 = floor(_12352 * 0.0625);
                                    uint _13638 = (esia_v1 + 1u) + uint(_13633);
                                    uint _13651 = max(uint(_292.gConv.z), 1u);
                                    vec4 _13685 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_13638 % _13651) * 24u) + uint(_12352 - (_13633 * 16.0))), int(_13638 / _13651), 0).xy, 0);
                                    vec2 _12364 = _13685.xy;
                                    vec2 _13696 = _41964 - _12364;
                                    vec2 _13699 = _5729 - _12364;
                                    vec2 _13712 = _13699 - (_13696 * clamp(dot(_13699, _13696) / max(dot(_13696, _13696), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _13715 = dot(_13712, _13712);
                                    float _49430 = (_13715 < _41967) ? _13715 : _41967;
                                    bool _13727 = _5729.y >= _13685.y;
                                    bool _13732 = _5729.y < _41964.y;
                                    bool _13743 = (_13696.x * _13699.y) > (_13696.y * _13699.x);
                                    float _41976;
                                    if (((_13727 && _13732) && _13743) || (((!_13727) && (!_13732)) && (!_13743)))
                                    {
                                        _41976 = -_41970;
                                    }
                                    else
                                    {
                                        _41976 = _41970;
                                    }
                                    vec2 _12374 = _13685.xy;
                                    if (((2.0 * _12352) + 1.0) < esia_v3.z)
                                    {
                                        vec2 _12383 = _13685.zw;
                                        vec2 _13772 = _12374 - _12383;
                                        vec2 _13775 = _5729 - _12383;
                                        vec2 _13788 = _13775 - (_13772 * clamp(dot(_13775, _13772) / max(dot(_13772, _13772), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _13791 = dot(_13788, _13788);
                                        bool _13803 = _5729.y >= _13685.w;
                                        bool _13808 = _5729.y < _13685.y;
                                        bool _13819 = (_13772.x * _13775.y) > (_13772.y * _13775.x);
                                        float _41980;
                                        if (((_13803 && _13808) && _13819) || (((!_13803) && (!_13808)) && (!_13819)))
                                        {
                                            _41980 = -_41976;
                                        }
                                        else
                                        {
                                            _41980 = _41976;
                                        }
                                        _48657 = _41980;
                                        _48655 = (_13791 < _49430) ? _13791 : _49430;
                                        _48653 = _13685.zw;
                                    }
                                    else
                                    {
                                        _48657 = _41976;
                                        _48655 = _49430;
                                        _48653 = _12374;
                                    }
                                }
                            }
                            _41998 = _41971;
                            _41996 = _41968;
                        }
                        float _12405 = sqrt(_41996);
                        if (_12288)
                        {
                            _42003 = _12405 - esia_v3.x;
                            break;
                        }
                        _42003 = (_41998 * _12405) - esia_v3.x;
                        break;
                    } while(false);
                    _42056 = _42003;
                }
                else
                {
                    float _42057;
                    if (esia_v7.z == 1u)
                    {
                        vec2 _12031 = _5729 - _11997;
                        float _41960;
                        do
                        {
                            if (esia_v5.w >= 6.282185077667236328125)
                            {
                                _41960 = abs(length(_12031) - esia_v3.x) - esia_v3.y;
                                break;
                            }
                            float _13862 = esia_v5.z + (esia_v5.w * 0.5);
                            float _13864 = cos(_13862);
                            float _13866 = sin(_13862);
                            float _13875 = dot(_12031, vec2(-_13866, _13864));
                            float _13878 = dot(_12031, vec2(_13864, _13866));
                            vec2 _13879 = vec2(_13875, _13878);
                            float _13882 = abs(_13875);
                            _13879.x = _13882;
                            float _13885 = esia_v5.w * 0.5;
                            float _13887 = sin(_13885);
                            float _13889 = cos(_13885);
                            _41960 = (((_13889 * _13882) > (_13887 * _13878)) ? length(_13879 - (vec2(_13887, _13889) * esia_v3.x)) : abs(length(_13879) - esia_v3.x)) - esia_v3.y;
                            break;
                        } while(false);
                        _42057 = _41960;
                    }
                    else
                    {
                        float _42058;
                        if (esia_v7.z == 2u)
                        {
                            vec2 _13925 = _5729 - _41468.xy;
                            vec2 _13928 = _41468.zw - _41468.xy;
                            _42058 = length(_13925 - (_13928 * clamp(dot(_13925, _13928) / max(dot(_13928, _13928), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                        }
                        else
                        {
                            vec2 _12058 = _5729 - _11997;
                            float _13993 = min(_12006.x, _12006.y);
                            float _13996 = min((_12058.x > 0.0) ? ((_12058.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_12058.y > 0.0) ? esia_v3.w : esia_v3.x), _13993);
                            float _14002 = _13996 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                            float _41949;
                            float _41950;
                            if (_14002 > _13993)
                            {
                                float _14016 = esia_v5.y * clamp((_13993 - _13996) / max(0.60000002384185791015625 * _13996, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _41950 = _14016;
                                _41949 = _13996 * (1.0 + (0.60000002384185791015625 * _14016));
                            }
                            else
                            {
                                _41950 = esia_v5.y;
                                _41949 = _14002;
                            }
                            vec2 _14045 = (abs(_12058) - _12006) + vec2(_41949);
                            vec2 _14047 = max(_14045, vec2(0.0));
                            float _41951;
                            SPIRV_CROSS_BRANCH
                            if ((_14047.x > 0.0) && (_14047.y > 0.0))
                            {
                                float _41952;
                                if ((_41950 > 0.001000000047497451305389404296875) && (_41949 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _14064 = 2.0 + (2.0 * _41950);
                                    vec2 _14069 = _14047 / vec2(max(_41949, 9.9999997473787516355514526367188e-05));
                                    _41952 = pow(pow(_14069.x, _14064) + pow(_14069.y, _14064), 1.0 / _14064) * _41949;
                                }
                                else
                                {
                                    _41952 = length(_14047);
                                }
                                _41951 = _41952;
                            }
                            else
                            {
                                _41951 = max(_14047.x, _14047.y);
                            }
                            float _14104 = (min(max(_14045.x, _14045.y), 0.0) + _41951) - _41949;
                            float _42059;
                            SPIRV_CROSS_BRANCH
                            if ((esia_v7.x & _2365) != 0u)
                            {
                                vec2 _12086 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                                vec2 _12089 = _5729 - ((_41468.xy + _41468.zw) * 0.5);
                                float _14152 = min(_12086.x, _12086.y);
                                float _14155 = min((_12089.x > 0.0) ? ((_12089.y > 0.0) ? _41466.x : _41466.x) : ((_12089.y > 0.0) ? _41466.x : _41466.x), _14152);
                                float _14161 = _14155 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                float _41953;
                                float _41954;
                                if (_14161 > _14152)
                                {
                                    float _14175 = esia_v5.y * clamp((_14152 - _14155) / max(0.60000002384185791015625 * _14155, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                    _41954 = _14175;
                                    _41953 = _14155 * (1.0 + (0.60000002384185791015625 * _14175));
                                }
                                else
                                {
                                    _41954 = esia_v5.y;
                                    _41953 = _14161;
                                }
                                vec2 _14204 = (abs(_12089) - _12086) + vec2(_41953);
                                vec2 _14206 = max(_14204, vec2(0.0));
                                float _41955;
                                SPIRV_CROSS_BRANCH
                                if ((_14206.x > 0.0) && (_14206.y > 0.0))
                                {
                                    float _41956;
                                    if ((_41954 > 0.001000000047497451305389404296875) && (_41953 > 9.9999997473787516355514526367188e-05))
                                    {
                                        float _14223 = 2.0 + (2.0 * _41954);
                                        vec2 _14228 = _14206 / vec2(max(_41953, 9.9999997473787516355514526367188e-05));
                                        _41956 = pow(pow(_14228.x, _14223) + pow(_14228.y, _14223), 1.0 / _14223) * _41953;
                                    }
                                    else
                                    {
                                        _41956 = length(_14206);
                                    }
                                    _41955 = _41956;
                                }
                                else
                                {
                                    _41955 = max(_14206.x, _14206.y);
                                }
                                float _14263 = (min(max(_14204.x, _14204.y), 0.0) + _41955) - _41953;
                                float _14268 = max(_41466.y, 9.9999997473787516355514526367188e-05);
                                float _14277 = max(_14268 - abs(_14104 - _14263), 0.0) / _14268;
                                _42059 = min(_14104, _14263) - (((_14277 * _14277) * _14268) * 0.25);
                            }
                            else
                            {
                                _42059 = _14104;
                            }
                            _42058 = _42059;
                        }
                        _42057 = _42058;
                    }
                    _42056 = _42057;
                }
                _42055 = _42056;
            }
            float _5734 = _42061 / _5632;
            float _5735 = 0.5 - _5734;
            float _5738 = _42055 / _5632;
            float _5739 = 0.5 - _5738;
            float _14295 = _5739 - _5735;
            _5742 = _42104 + ((abs(_14295) < 0.001000000047497451305389404296875) ? clamp(0.5 * (_5735 + _5739), 0.0, 1.0) : ((((_5739 <= 0.0) ? 0.0 : ((_5739 >= 1.0) ? (-_5738) : ((0.5 * _5739) * _5739))) - ((_5735 <= 0.0) ? 0.0 : ((_5735 >= 1.0) ? (-_5734) : ((0.5 * _5735) * _5735)))) / _14295));
            float _5747 = (_42061 - _5697) / _5632;
            float _5748 = 0.5 - _5747;
            float _5753 = (_42055 - _5697) / _5632;
            float _5754 = 0.5 - _5753;
            float _14347 = _5754 - _5748;
            float _5760 = (_42061 + _5703) / _5632;
            float _5761 = 0.5 - _5760;
            float _5766 = (_42055 + _5703) / _5632;
            float _5767 = 0.5 - _5766;
            float _14399 = _5767 - _5761;
            _5771 = _42147 + (((abs(_14347) < 0.001000000047497451305389404296875) ? clamp(0.5 * (_5748 + _5754), 0.0, 1.0) : ((((_5754 <= 0.0) ? 0.0 : ((_5754 >= 1.0) ? (-_5753) : ((0.5 * _5754) * _5754))) - ((_5748 <= 0.0) ? 0.0 : ((_5748 >= 1.0) ? (-_5747) : ((0.5 * _5748) * _5748)))) / _14347)) - ((abs(_14399) < 0.001000000047497451305389404296875) ? clamp(0.5 * (_5761 + _5767), 0.0, 1.0) : ((((_5767 <= 0.0) ? 0.0 : ((_5767 >= 1.0) ? (-_5766) : ((0.5 * _5767) * _5767))) - ((_5761 <= 0.0) ? 0.0 : ((_5761 >= 1.0) ? (-_5760) : ((0.5 * _5761) * _5761)))) / _14399)));
        }
        _47433 = ((esia_v7.x & _3817) != 0u) ? (_42147 / _5692) : (-1.0);
        _42238 = _42104 / _5692;
    }
    else
    {
        _47433 = -1.0;
        _42238 = clamp(0.5 - (_41577 / _5632), 0.0, 1.0);
    }
    float _47879;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _3827) != 0u)
    {
        uint _14450 = max(uint(_292.gConv.z), 1u);
        vec4 _14484 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _14450) * 24u) + 19u), int(esia_v1 / _14450), 0).xy, 0);
        uint _14491 = max(uint(_292.gConv.z), 1u);
        vec4 _14525 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _14491) * 24u) + 20u), int(esia_v1 / _14491), 0).xy, 0);
        vec2 _5816 = max((_14484.zw - _14484.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        vec2 _5819 = esia_v0 - ((_14484.xy + _14484.zw) * 0.5);
        float _5825 = _14525.y;
        float _14573 = min(_5816.x, _5816.y);
        float _14576 = min((_5819.x > 0.0) ? ((_5819.y > 0.0) ? _14525.x : _14525.x) : ((_5819.y > 0.0) ? _14525.x : _14525.x), _14573);
        float _14582 = _14576 * (1.0 + (0.60000002384185791015625 * _5825));
        float _42233;
        float _42234;
        if (_14582 > _14573)
        {
            float _14596 = _5825 * clamp((_14573 - _14576) / max(0.60000002384185791015625 * _14576, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _42234 = _14596;
            _42233 = _14576 * (1.0 + (0.60000002384185791015625 * _14596));
        }
        else
        {
            _42234 = _5825;
            _42233 = _14582;
        }
        vec2 _14625 = (abs(_5819) - _5816) + vec2(_42233);
        vec2 _14627 = max(_14625, vec2(0.0));
        float _42235;
        SPIRV_CROSS_BRANCH
        if ((_14627.x > 0.0) && (_14627.y > 0.0))
        {
            float _42236;
            if ((_42234 > 0.001000000047497451305389404296875) && (_42233 > 9.9999997473787516355514526367188e-05))
            {
                float _14644 = 2.0 + (2.0 * _42234);
                vec2 _14649 = _14627 / vec2(max(_42233, 9.9999997473787516355514526367188e-05));
                _42236 = pow(pow(_14649.x, _14644) + pow(_14649.y, _14644), 1.0 / _14644) * _42233;
            }
            else
            {
                _42236 = length(_14627);
            }
            _42235 = _42236;
        }
        else
        {
            _42235 = max(_14627.x, _14627.y);
        }
        float _5831 = clamp(0.5 - (((min(max(_14625.x, _14625.y), 0.0) + _42235) - _42233) / _5632), 0.0, 1.0);
        if (_5831 <= 0.0)
        {
            discard;
        }
        _47879 = _5831;
    }
    else
    {
        _47879 = 1.0;
    }
    bool _5842 = ((esia_v7.x & _3892) != 0u) && (_42238 >= 0.999000012874603271484375);
    vec4 _42513;
    SPIRV_CROSS_BRANCH
    if ((((esia_v7.x & _3900) != 0u) && (!((esia_v7.x & _3905) != 0u))) && (!_5842))
    {
        uint _14691 = max(uint(_292.gConv.z), 1u);
        vec4 _14725 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _14691) * 24u) + 7u), int(esia_v1 / _14691), 0).xy, 0);
        uint _14732 = max(uint(_292.gConv.z), 1u);
        vec4 _14766 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _14732) * 24u) + 8u), int(esia_v1 / _14732), 0).xy, 0);
        vec2 _42242;
        SPIRV_CROSS_BRANCH
        if (_334.gDrawInfo.w > 0.5)
        {
            vec2 _14808 = _334.gDrawInfo.yz - ((esia_v2.xy + esia_v2.zw) * 0.5);
            bvec2 _14812 = bvec2(_334.gDrawInfo.w > 1.0);
            vec2 _14813 = vec2(_14812.x ? _14808.x : _334.gDrawInfo.yz.x, _14812.y ? _14808.y : _334.gDrawInfo.yz.y);
            _42242 = (-(_14813 / vec2(max(length(_14813), 9.9999997473787516355514526367188e-05)))) * length(_14766.zw);
        }
        else
        {
            _42242 = _14766.zw;
        }
        vec2 _5872 = esia_v0 - _42242;
        vec2 _14864 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _14873 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _42370;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & _2279) != 0u)
        {
            float _14990 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
            vec4 _15024 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_41462.xy, _41462.zw, (clamp(_5872, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
            float _15048 = (_15024.w - 0.5) * esia_v3.y;
            vec2 _15089 = abs(_5872 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _14990), vec2(0.0));
            _42370 = max((-mix((max(min(_15024.x, _15024.y), min(max(_15024.x, _15024.y), _15024.z)) - 0.5) * esia_v3.x, _15048, clamp(abs(_15048) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _14990, length(max(_15089, vec2(0.0))) + min(max(_15089.x, _15089.y), 0.0)) - esia_v5.w;
        }
        else
        {
            float _42371;
            if ((esia_v7.x & _2294) != 0u)
            {
                float _42318;
                do
                {
                    bool _15155 = esia_v3.w > 0.5;
                    float _15159 = floor((esia_v3.z + 1.0) * 0.5);
                    float _15163 = floor((_15159 + 127.0) * 0.0078125);
                    float _42311;
                    float _42313;
                    SPIRV_CROSS_BRANCH
                    if (_15155)
                    {
                        uint _15339 = uint(esia_v3.z);
                        uint _15342 = (_15339 + 31u) >> 5u;
                        uint _15345 = esia_v1 + 1u;
                        uint _15428 = max(uint(_292.gConv.z), 1u);
                        ivec2 _15438 = ivec2(int((_15345 % _15428) * 24u), int(_15345 / _15428));
                        ivec2 _42303;
                        uint _42304;
                        ivec2 _42364;
                        _42364 = _15438;
                        _42304 = 0u;
                        _42303 = _15438;
                        float _49432;
                        uint _49433;
                        ivec2 _49435;
                        ivec2 _42367;
                        uint _42302 = 0u;
                        float _42365 = 3.0000000054977557577780399428115e+38;
                        SPIRV_CROSS_LOOP
                        for (; _42302 < _15342; _42365 = _49432, _42364 = _42367, _42304 = _49433, _42303 = _49435, _42302++)
                        {
                            vec4 _15466 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42364.x + 20, _42364.y, 0).xy, 0);
                            vec2 _15479 = max(max(_15466.xy - _5872, _5872 - _15466.zw), vec2(0.0));
                            float _15482 = dot(_15479, _15479);
                            bool _15360 = _15482 < _42365;
                            _49432 = _15360 ? _15482 : _42365;
                            _49433 = _15360 ? _42302 : _42304;
                            bvec2 _49434 = bvec2(_15360);
                            _49435 = ivec2(_49434.x ? _42364.x : _42303.x, _49434.y ? _42364.y : _42303.y);
                            int _15487 = _42364.x + 24;
                            ivec2 _39643 = _42364;
                            _39643.x = _15487;
                            if (_15487 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                            {
                                _42367 = ivec2(0, _42364.y + 1);
                            }
                            else
                            {
                                _42367 = _39643;
                            }
                        }
                        uint _15542 = _42304 * 32u;
                        uint _15546 = min((_15339 - _15542), 32u);
                        vec4 _15673 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42303.x + 21, _42303.y, 0).xy, 0);
                        vec4 _15701 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42303.x + 22, _42303.y, 0).xy, 0);
                        float _42307;
                        uint _42308;
                        _42308 = 4294967295u;
                        _42307 = 1000000015047466219876688855040.0;
                        float _48735;
                        uint _48736;
                        SPIRV_CROSS_LOOP
                        for (uint _42306 = 0u; (_42306 * 8u) < _15546; _42308 = _48736, _42307 = _48735, _42306++)
                        {
                            vec4 _15729 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42303.x + int(16u + _42306), _42303.y, 0).xy, 0);
                            float _15747 = ((sqrt(_42307) * 1.000003814697265625) + 0.03125) + ((abs(_5872.x) + abs(_5872.y)) * 3.814697265625e-06);
                            vec2 _15767 = max(max(_15729.xy - _5872, _5872 - _15729.zw), vec2(0.0));
                            if (dot(_15767, _15767) > (_15747 * _15747))
                            {
                                _48736 = _42308;
                                _48735 = _42307;
                                continue;
                            }
                            bvec4 _15571 = bvec4(_42306 < 2u);
                            vec4 _15572 = vec4(_15571.x ? _15673.x : _15701.x, _15571.y ? _15673.y : _15701.y, _15571.z ? _15673.z : _15701.z, _15571.w ? _15673.w : _15701.w);
                            vec2 _15577 = _15572.zw;
                            vec2 _15579 = _15572.xy;
                            bvec2 _15580 = bvec2((_42306 & 1u) != 0u);
                            uint _15586 = min(_15546, ((_42306 * 8u) + 8u));
                            uint _15588 = _42306 * 8u;
                            vec2 _42349;
                            float _42350;
                            uint _42351;
                            _42351 = _42308;
                            _42350 = _42307;
                            _42349 = vec2(_15580.x ? _15577.x : _15579.x, _15580.y ? _15577.y : _15579.y);
                            vec2 _48784;
                            float _48785;
                            uint _48786;
                            SPIRV_CROSS_LOOP
                            for (uint _42348 = _15588; _42348 < _15586; _42351 = _48786, _42350 = _48785, _42349 = _48784, _42348 += 2u)
                            {
                                vec4 _15798 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42303.x + int(_42348 >> 1u), _42303.y, 0).xy, 0);
                                uint _15601 = _15542 + _42348;
                                vec2 _15604 = _15798.xy;
                                vec2 _15806 = _42349 - _15604;
                                vec2 _15809 = _5872 - _15604;
                                vec2 _15822 = _15809 - (_15806 * clamp(dot(_15809, _15806) / max(dot(_15806, _15806), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _15825 = dot(_15822, _15822);
                                bool _15836 = (_15825 < _42350) || ((_15825 == _42350) && (_15601 < _42351));
                                uint _49436 = _15836 ? _15601 : _42351;
                                float _49437 = _15836 ? _15825 : _42350;
                                vec2 _15614 = _15798.xy;
                                if ((_42348 + 1u) < _15586)
                                {
                                    uint _15623 = (_15542 + _42348) + 1u;
                                    vec2 _15626 = _15798.zw;
                                    vec2 _15849 = _15614 - _15626;
                                    vec2 _15852 = _5872 - _15626;
                                    vec2 _15865 = _15852 - (_15849 * clamp(dot(_15852, _15849) / max(dot(_15849, _15849), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _15868 = dot(_15865, _15865);
                                    bool _15879 = (_15868 < _49437) || ((_15868 == _49437) && (_15623 < _49436));
                                    _48786 = _15879 ? _15623 : _49436;
                                    _48785 = _15879 ? _15868 : _49437;
                                    _48784 = _15798.zw;
                                }
                                else
                                {
                                    _48786 = _49436;
                                    _48785 = _49437;
                                    _48784 = _15614;
                                }
                            }
                            _48736 = _42351;
                            _48735 = _42350;
                        }
                        uint _15385 = esia_v1 + 1u;
                        uint _15891 = max(uint(_292.gConv.z), 1u);
                        float _42310;
                        ivec2 _42321;
                        _42321 = ivec2(int((_15385 % _15891) * 24u), int(_15385 / _15891));
                        _42310 = _42307;
                        ivec2 _42328;
                        float _48738;
                        uint _48768;
                        SPIRV_CROSS_LOOP
                        for (uint _42309 = 0u, _42322 = _42308; _42309 < _15342; _42322 = _48768, _42321 = _42328, _42310 = _48738, _42309++)
                        {
                            vec4 _15929 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42321.x + 20, _42321.y, 0).xy, 0);
                            float _15947 = ((sqrt(_42310) * 1.000003814697265625) + 0.03125) + ((abs(_5872.x) + abs(_5872.y)) * 3.814697265625e-06);
                            vec2 _15967 = max(max(_15929.xy - _5872, _5872 - _15929.zw), vec2(0.0));
                            if ((_42309 != _42304) && (!(dot(_15967, _15967) > (_15947 * _15947))))
                            {
                                uint _16008 = _42309 * 32u;
                                uint _16012 = min((_15339 - _16008), 32u);
                                vec4 _16139 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42321.x + 21, _42321.y, 0).xy, 0);
                                vec4 _16167 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42321.x + 22, _42321.y, 0).xy, 0);
                                float _42324;
                                uint _42325;
                                _42325 = _42322;
                                _42324 = _42310;
                                float _48769;
                                uint _48770;
                                SPIRV_CROSS_LOOP
                                for (uint _42323 = 0u; (_42323 * 8u) < _16012; _42325 = _48770, _42324 = _48769, _42323++)
                                {
                                    vec4 _16195 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42321.x + int(16u + _42323), _42321.y, 0).xy, 0);
                                    float _16213 = ((sqrt(_42324) * 1.000003814697265625) + 0.03125) + ((abs(_5872.x) + abs(_5872.y)) * 3.814697265625e-06);
                                    vec2 _16233 = max(max(_16195.xy - _5872, _5872 - _16195.zw), vec2(0.0));
                                    if (dot(_16233, _16233) > (_16213 * _16213))
                                    {
                                        _48770 = _42325;
                                        _48769 = _42324;
                                        continue;
                                    }
                                    bvec4 _16037 = bvec4(_42323 < 2u);
                                    vec4 _16038 = vec4(_16037.x ? _16139.x : _16167.x, _16037.y ? _16139.y : _16167.y, _16037.z ? _16139.z : _16167.z, _16037.w ? _16139.w : _16167.w);
                                    vec2 _16043 = _16038.zw;
                                    vec2 _16045 = _16038.xy;
                                    bvec2 _16046 = bvec2((_42323 & 1u) != 0u);
                                    uint _16052 = min(_16012, ((_42323 * 8u) + 8u));
                                    uint _16054 = _42323 * 8u;
                                    vec2 _42333;
                                    float _42334;
                                    uint _42335;
                                    _42335 = _42325;
                                    _42334 = _42324;
                                    _42333 = vec2(_16046.x ? _16043.x : _16045.x, _16046.y ? _16043.y : _16045.y);
                                    vec2 _48781;
                                    float _48782;
                                    uint _48783;
                                    SPIRV_CROSS_LOOP
                                    for (uint _42332 = _16054; _42332 < _16052; _42335 = _48783, _42334 = _48782, _42333 = _48781, _42332 += 2u)
                                    {
                                        vec4 _16264 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42321.x + int(_42332 >> 1u), _42321.y, 0).xy, 0);
                                        uint _16067 = _16008 + _42332;
                                        vec2 _16070 = _16264.xy;
                                        vec2 _16272 = _42333 - _16070;
                                        vec2 _16275 = _5872 - _16070;
                                        vec2 _16288 = _16275 - (_16272 * clamp(dot(_16275, _16272) / max(dot(_16272, _16272), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _16291 = dot(_16288, _16288);
                                        bool _16302 = (_16291 < _42334) || ((_16291 == _42334) && (_16067 < _42335));
                                        uint _49440 = _16302 ? _16067 : _42335;
                                        float _49441 = _16302 ? _16291 : _42334;
                                        vec2 _16080 = _16264.xy;
                                        if ((_42332 + 1u) < _16052)
                                        {
                                            uint _16089 = (_16008 + _42332) + 1u;
                                            vec2 _16092 = _16264.zw;
                                            vec2 _16315 = _16080 - _16092;
                                            vec2 _16318 = _5872 - _16092;
                                            vec2 _16331 = _16318 - (_16315 * clamp(dot(_16318, _16315) / max(dot(_16315, _16315), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _16334 = dot(_16331, _16331);
                                            bool _16345 = (_16334 < _49441) || ((_16334 == _49441) && (_16089 < _49440));
                                            _48783 = _16345 ? _16089 : _49440;
                                            _48782 = _16345 ? _16334 : _49441;
                                            _48781 = _16264.zw;
                                        }
                                        else
                                        {
                                            _48783 = _49440;
                                            _48782 = _49441;
                                            _48781 = _16080;
                                        }
                                    }
                                    _48770 = _42335;
                                    _48769 = _42334;
                                }
                                _48768 = _42325;
                                _48738 = _42324;
                            }
                            else
                            {
                                _48768 = _42322;
                                _48738 = _42310;
                            }
                            int _16355 = _42321.x + 24;
                            ivec2 _39690 = _42321;
                            _39690.x = _16355;
                            if (_16355 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                            {
                                _42328 = ivec2(0, _42321.y + 1);
                            }
                            else
                            {
                                _42328 = _39690;
                            }
                        }
                        _42313 = 1.0;
                        _42311 = _42310;
                    }
                    else
                    {
                        float _15177 = floor((esia_v3.z - 1.0) * 0.5);
                        float _16380 = floor(_15177 * 0.0625);
                        uint _16385 = (esia_v1 + 1u) + uint(_16380);
                        uint _16398 = max(uint(_292.gConv.z), 1u);
                        vec4 _16432 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_16385 % _16398) * 24u) + uint(_15177 - (_16380 * 16.0))), int(_16385 / _16398), 0).xy, 0);
                        vec2 _15189 = _16432.xy;
                        vec2 _15191 = _16432.zw;
                        bvec2 _15192 = bvec2(((esia_v3.z - 1.0) - (_15177 * 2.0)) < 0.5);
                        vec2 _42281;
                        if (_15155)
                        {
                            float _16440 = floor(0.0);
                            uint _16445 = (esia_v1 + 1u) + uint(_16440);
                            uint _16458 = max(uint(_292.gConv.z), 1u);
                            _42281 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_16445 % _16458) * 24u) + uint(_16440 * (-16.0))), int(_16445 / _16458), 0).xy, 0).xy;
                        }
                        else
                        {
                            _42281 = vec2(_15192.x ? _15189.x : _15191.x, _15192.y ? _15189.y : _15191.y);
                        }
                        vec2 _42280;
                        float _42283;
                        float _42286;
                        _42286 = 1.0;
                        _42283 = 1000000015047466219876688855040.0;
                        _42280 = _42281;
                        vec2 _42279;
                        float _42282;
                        float _42285;
                        SPIRV_CROSS_LOOP
                        for (float _42276 = 0.0; _42276 < 16.0; _42286 = _42285, _42283 = _42282, _42280 = _42279, _42276 += 1.0)
                        {
                            if (_42276 >= _15163)
                            {
                                break;
                            }
                            _42285 = _42286;
                            _42282 = _42283;
                            _42279 = _42280;
                            vec2 _48724;
                            float _48726;
                            float _48728;
                            SPIRV_CROSS_LOOP
                            for (float _42277 = 0.0; _42277 < 128.0; _42285 = _48728, _42282 = _48726, _42279 = _48724, _42277 += 1.0)
                            {
                                float _15219 = (_42276 * 128.0) + _42277;
                                if (_15219 >= _15159)
                                {
                                    break;
                                }
                                float _16500 = floor(_15219 * 0.0625);
                                uint _16505 = (esia_v1 + 1u) + uint(_16500);
                                uint _16518 = max(uint(_292.gConv.z), 1u);
                                vec4 _16552 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_16505 % _16518) * 24u) + uint(_15219 - (_16500 * 16.0))), int(_16505 / _16518), 0).xy, 0);
                                vec2 _15231 = _16552.xy;
                                vec2 _16563 = _42279 - _15231;
                                vec2 _16566 = _5872 - _15231;
                                vec2 _16579 = _16566 - (_16563 * clamp(dot(_16566, _16563) / max(dot(_16563, _16563), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _16582 = dot(_16579, _16579);
                                float _49444 = (_16582 < _42282) ? _16582 : _42282;
                                bool _16594 = _5872.y >= _16552.y;
                                bool _16599 = _5872.y < _42279.y;
                                bool _16610 = (_16563.x * _16566.y) > (_16563.y * _16566.x);
                                float _42291;
                                if (((_16594 && _16599) && _16610) || (((!_16594) && (!_16599)) && (!_16610)))
                                {
                                    _42291 = -_42285;
                                }
                                else
                                {
                                    _42291 = _42285;
                                }
                                vec2 _15241 = _16552.xy;
                                if (((2.0 * _15219) + 1.0) < esia_v3.z)
                                {
                                    vec2 _15250 = _16552.zw;
                                    vec2 _16639 = _15241 - _15250;
                                    vec2 _16642 = _5872 - _15250;
                                    vec2 _16655 = _16642 - (_16639 * clamp(dot(_16642, _16639) / max(dot(_16639, _16639), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _16658 = dot(_16655, _16655);
                                    bool _16670 = _5872.y >= _16552.w;
                                    bool _16675 = _5872.y < _16552.y;
                                    bool _16686 = (_16639.x * _16642.y) > (_16639.y * _16642.x);
                                    float _42295;
                                    if (((_16670 && _16675) && _16686) || (((!_16670) && (!_16675)) && (!_16686)))
                                    {
                                        _42295 = -_42291;
                                    }
                                    else
                                    {
                                        _42295 = _42291;
                                    }
                                    _48728 = _42295;
                                    _48726 = (_16658 < _49444) ? _16658 : _49444;
                                    _48724 = _16552.zw;
                                }
                                else
                                {
                                    _48728 = _42291;
                                    _48726 = _49444;
                                    _48724 = _15241;
                                }
                            }
                        }
                        _42313 = _42286;
                        _42311 = _42283;
                    }
                    float _15272 = sqrt(_42311);
                    if (_15155)
                    {
                        _42318 = _15272 - esia_v3.x;
                        break;
                    }
                    _42318 = (_42313 * _15272) - esia_v3.x;
                    break;
                } while(false);
                _42371 = _42318;
            }
            else
            {
                float _42372;
                if (esia_v7.z == 1u)
                {
                    vec2 _14898 = _5872 - _14864;
                    float _42275;
                    do
                    {
                        if (esia_v5.w >= 6.282185077667236328125)
                        {
                            _42275 = abs(length(_14898) - esia_v3.x) - esia_v3.y;
                            break;
                        }
                        float _16729 = esia_v5.z + (esia_v5.w * 0.5);
                        float _16731 = cos(_16729);
                        float _16733 = sin(_16729);
                        float _16742 = dot(_14898, vec2(-_16733, _16731));
                        float _16745 = dot(_14898, vec2(_16731, _16733));
                        vec2 _16746 = vec2(_16742, _16745);
                        float _16749 = abs(_16742);
                        _16746.x = _16749;
                        float _16752 = esia_v5.w * 0.5;
                        float _16754 = sin(_16752);
                        float _16756 = cos(_16752);
                        _42275 = (((_16756 * _16749) > (_16754 * _16745)) ? length(_16746 - (vec2(_16754, _16756) * esia_v3.x)) : abs(length(_16746) - esia_v3.x)) - esia_v3.y;
                        break;
                    } while(false);
                    _42372 = _42275;
                }
                else
                {
                    float _42373;
                    if (esia_v7.z == 2u)
                    {
                        vec2 _16792 = _5872 - _41468.xy;
                        vec2 _16795 = _41468.zw - _41468.xy;
                        _42373 = length(_16792 - (_16795 * clamp(dot(_16792, _16795) / max(dot(_16795, _16795), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                    }
                    else
                    {
                        vec2 _14925 = _5872 - _14864;
                        float _16860 = min(_14873.x, _14873.y);
                        float _16863 = min((_14925.x > 0.0) ? ((_14925.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_14925.y > 0.0) ? esia_v3.w : esia_v3.x), _16860);
                        float _16869 = _16863 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _42264;
                        float _42265;
                        if (_16869 > _16860)
                        {
                            float _16883 = esia_v5.y * clamp((_16860 - _16863) / max(0.60000002384185791015625 * _16863, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _42265 = _16883;
                            _42264 = _16863 * (1.0 + (0.60000002384185791015625 * _16883));
                        }
                        else
                        {
                            _42265 = esia_v5.y;
                            _42264 = _16869;
                        }
                        vec2 _16912 = (abs(_14925) - _14873) + vec2(_42264);
                        vec2 _16914 = max(_16912, vec2(0.0));
                        float _42266;
                        SPIRV_CROSS_BRANCH
                        if ((_16914.x > 0.0) && (_16914.y > 0.0))
                        {
                            float _42267;
                            if ((_42265 > 0.001000000047497451305389404296875) && (_42264 > 9.9999997473787516355514526367188e-05))
                            {
                                float _16931 = 2.0 + (2.0 * _42265);
                                vec2 _16936 = _16914 / vec2(max(_42264, 9.9999997473787516355514526367188e-05));
                                _42267 = pow(pow(_16936.x, _16931) + pow(_16936.y, _16931), 1.0 / _16931) * _42264;
                            }
                            else
                            {
                                _42267 = length(_16914);
                            }
                            _42266 = _42267;
                        }
                        else
                        {
                            _42266 = max(_16914.x, _16914.y);
                        }
                        float _16971 = (min(max(_16912.x, _16912.y), 0.0) + _42266) - _42264;
                        float _42374;
                        SPIRV_CROSS_BRANCH
                        if ((esia_v7.x & _2365) != 0u)
                        {
                            vec2 _14953 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                            vec2 _14956 = _5872 - ((_41468.xy + _41468.zw) * 0.5);
                            float _17019 = min(_14953.x, _14953.y);
                            float _17022 = min((_14956.x > 0.0) ? ((_14956.y > 0.0) ? _41466.x : _41466.x) : ((_14956.y > 0.0) ? _41466.x : _41466.x), _17019);
                            float _17028 = _17022 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                            float _42268;
                            float _42269;
                            if (_17028 > _17019)
                            {
                                float _17042 = esia_v5.y * clamp((_17019 - _17022) / max(0.60000002384185791015625 * _17022, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _42269 = _17042;
                                _42268 = _17022 * (1.0 + (0.60000002384185791015625 * _17042));
                            }
                            else
                            {
                                _42269 = esia_v5.y;
                                _42268 = _17028;
                            }
                            vec2 _17071 = (abs(_14956) - _14953) + vec2(_42268);
                            vec2 _17073 = max(_17071, vec2(0.0));
                            float _42270;
                            SPIRV_CROSS_BRANCH
                            if ((_17073.x > 0.0) && (_17073.y > 0.0))
                            {
                                float _42271;
                                if ((_42269 > 0.001000000047497451305389404296875) && (_42268 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _17090 = 2.0 + (2.0 * _42269);
                                    vec2 _17095 = _17073 / vec2(max(_42268, 9.9999997473787516355514526367188e-05));
                                    _42271 = pow(pow(_17095.x, _17090) + pow(_17095.y, _17090), 1.0 / _17090) * _42268;
                                }
                                else
                                {
                                    _42271 = length(_17073);
                                }
                                _42270 = _42271;
                            }
                            else
                            {
                                _42270 = max(_17073.x, _17073.y);
                            }
                            float _17130 = (min(max(_17071.x, _17071.y), 0.0) + _42270) - _42268;
                            float _17135 = max(_41466.y, 9.9999997473787516355514526367188e-05);
                            float _17144 = max(_17135 - abs(_16971 - _17130), 0.0) / _17135;
                            _42374 = min(_16971, _17130) - (((_17144 * _17144) * _17135) * 0.25);
                        }
                        else
                        {
                            _42374 = _16971;
                        }
                        _42373 = _42374;
                    }
                    _42372 = _42373;
                }
                _42371 = _42372;
            }
            _42370 = _42371;
        }
        float _5881 = (_42370 - _14766.y) / (max(_14766.x * 0.5, _5632 * 0.5) * 1.41421353816986083984375);
        float _17161 = sign(_5881);
        float _17163 = abs(_5881);
        float _17174 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_17163 * _17163))) * _17163)) * _17163);
        float _17177 = _17174 * _17174;
        float _17192 = clamp(_14725.w * (0.5 - (0.5 * (_17161 - (_17161 / (_17177 * _17177))))), 0.0, 1.0);
        _42513 = vec4(_14725.xyz * _17192, _17192);
    }
    else
    {
        _42513 = vec4(0.0);
    }
    vec4 _42613;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & _3969) != 0u) && (!_5842))
    {
        uint _17217 = max(uint(_292.gConv.z), 1u);
        vec4 _17251 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _17217) * 24u) + 9u), int(esia_v1 / _17217), 0).xy, 0);
        uint _17258 = max(uint(_292.gConv.z), 1u);
        vec4 _17292 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _17258) * 24u) + 10u), int(esia_v1 / _17258), 0).xy, 0);
        float _5909 = max(_17292.x, 0.001000000047497451305389404296875);
        float _42506;
        float _42509;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & _3998) != 0u)
        {
            uint _17299 = max(uint(_292.gConv.z), 1u);
            vec4 _17333 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _17299) * 24u) + 22u), int(esia_v1 / _17299), 0).xy, 0);
            vec2 _5925 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _5934 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            vec2 _5941 = esia_v2.xy - _17333.xy;
            vec2 _5948 = _17333.zw - esia_v2.zw;
            vec2 _5978 = clamp(vec2((esia_v0.x < _5925.x) ? _5941.x : _5948.x, (esia_v0.y < _5925.y) ? _5941.y : _5948.y) * vec2(0.58823525905609130859375), vec2(min(_5909, 1.5)), vec2(_5909));
            float _5983 = min(_5934.x, _5934.y);
            float _42504;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 0u)
            {
                _42504 = min(((esia_v0.x > _5925.x) ? ((esia_v0.y > _5925.y) ? esia_v3.z : esia_v3.y) : ((esia_v0.y > _5925.y) ? esia_v3.w : esia_v3.x)) * (1.0 + (0.60000002384185791015625 * esia_v5.y)), _5983);
            }
            else
            {
                _42504 = _5983;
            }
            vec2 _6032 = max(abs(esia_v0 - _5925) - (_5934 - vec2(_42504)), vec2(0.0));
            float _6034 = length(_6032);
            vec2 _6040 = _6032 / vec2(_6034);
            bvec2 _6041 = bvec2(_6034 > 9.9999997473787516355514526367188e-05);
            vec2 _6042 = vec2(_6041.x ? _6040.x : vec2(0.707099974155426025390625).x, _6041.y ? _6040.y : vec2(0.707099974155426025390625).y);
            vec2 _6057 = esia_v0 - _17333.xy;
            vec2 _6062 = _17333.zw - esia_v0;
            _42509 = clamp(min(min(_6057.x, _6057.y), min(_6062.x, _6062.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _42506 = inversesqrt(dot(_6042 * _6042, vec2(1.0) / (_5978 * _5978)));
        }
        else
        {
            _42509 = 1.0;
            _42506 = _5909;
        }
        float _6080 = max(_41577, 0.0) / _42506;
        float _17341 = clamp(_17251.w * clamp((exp(((-_6080) * _6080) * 2.2000000476837158203125) * _17292.y) * _42509, 0.0, 1.0), 0.0, 1.0);
        _42613 = vec4(_17251.xyz * _17341, _17341) + (_42513 * (1.0 - _17341));
    }
    else
    {
        _42613 = _42513;
    }
    vec4 _44704;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _4214) != 0u)
    {
        vec2 _6111 = _41461 * _292.gTarget.zw;
        float _17374 = clamp(log2(max(max(esia_v3.w / _5632, 4.0), 1.0)) - 1.0, 0.0, 5.0);
        int _17377 = int(floor(_17374));
        float _17381 = _17374 - float(_17377);
        bool _17386 = (_17381 > 0.0199999995529651641845703125) && (_17377 < 5);
        vec3 _42585;
        _42585 = vec3(0.0);
        vec3 _17415;
        SPIRV_CROSS_LOOP
        for (int _42515 = 0; _42515 < 2; _42585 = _17415, _42515++)
        {
            if ((_42515 > 0) && (!_17386))
            {
                break;
            }
            int _17401 = _17377 + _42515;
            int _17439 = clamp(_17401, 1, 5);
            vec2 _17508 = (_6111 * _292.gLevel[_17439].xy) - vec2(0.5);
            vec2 _17510 = floor(_17508);
            vec2 _17513 = _17508 - _17510;
            vec2 _17516 = _17513 * _17513;
            vec2 _17519 = _17516 * _17513;
            vec2 _17538 = (((_17519 * 3.0) - (_17516 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
            vec2 _17551 = _17519 * 0.16666667163372039794921875;
            vec2 _17554 = (((((-_17519) + (_17516 * 3.0)) - (_17513 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17538;
            vec2 _17558 = (((((_17519 * (-3.0)) + (_17516 * 3.0)) + (_17513 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17551;
            vec2 _17570 = ((_17510 - vec2(0.5)) + (_17538 / _17554)) * _292.gLevel[_17439].zw;
            vec2 _17582 = ((_17510 + vec2(1.5)) + (_17551 / _17558)) * _292.gLevel[_17439].zw;
            vec4 _42552;
            SPIRV_CROSS_BRANCH
            if (_17401 <= 0)
            {
                vec2 _42551;
                if (_292.gConv.x > 0.5)
                {
                    vec2 _39803 = _6111;
                    _39803.y = 1.0 - _6111.y;
                    _42551 = _39803;
                }
                else
                {
                    _42551 = _6111;
                }
                _42552 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _42551, 0.0);
            }
            else
            {
                vec4 _42553;
                if (_17401 == 1)
                {
                    vec2 _42544;
                    if (_292.gConv.x > 0.5)
                    {
                        vec2 _39808 = _17570;
                        _39808.y = 1.0 - _17570.y;
                        _42544 = _39808;
                    }
                    else
                    {
                        _42544 = _17570;
                    }
                    float _17627 = _17570.y;
                    vec2 _17628 = vec2(_17582.x, _17627);
                    vec2 _42545;
                    if (_292.gConv.x > 0.5)
                    {
                        vec2 _39815 = _17628;
                        _39815.y = 1.0 - _17627;
                        _42545 = _39815;
                    }
                    else
                    {
                        _42545 = _17628;
                    }
                    float _17645 = _17582.y;
                    vec2 _17646 = vec2(_17570.x, _17645);
                    vec2 _42547;
                    if (_292.gConv.x > 0.5)
                    {
                        vec2 _39822 = _17646;
                        _39822.y = 1.0 - _17645;
                        _42547 = _39822;
                    }
                    else
                    {
                        _42547 = _17646;
                    }
                    vec2 _42549;
                    if (_292.gConv.x > 0.5)
                    {
                        vec2 _39829 = _17582;
                        _39829.y = 1.0 - _17582.y;
                        _42549 = _39829;
                    }
                    else
                    {
                        _42549 = _17582;
                    }
                    _42553 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _42544, 0.0) * (_17554.x * _17554.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _42545, 0.0) * (_17558.x * _17554.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _42547, 0.0) * (_17554.x * _17558.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _42549, 0.0) * (_17558.x * _17558.y));
                }
                else
                {
                    vec4 _42554;
                    if (_17401 == 2)
                    {
                        vec2 _42537;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _39836 = _17570;
                            _39836.y = 1.0 - _17570.y;
                            _42537 = _39836;
                        }
                        else
                        {
                            _42537 = _17570;
                        }
                        float _17757 = _17570.y;
                        vec2 _17758 = vec2(_17582.x, _17757);
                        vec2 _42538;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _39843 = _17758;
                            _39843.y = 1.0 - _17757;
                            _42538 = _39843;
                        }
                        else
                        {
                            _42538 = _17758;
                        }
                        float _17775 = _17582.y;
                        vec2 _17776 = vec2(_17570.x, _17775);
                        vec2 _42540;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _39850 = _17776;
                            _39850.y = 1.0 - _17775;
                            _42540 = _39850;
                        }
                        else
                        {
                            _42540 = _17776;
                        }
                        vec2 _42542;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _39857 = _17582;
                            _39857.y = 1.0 - _17582.y;
                            _42542 = _39857;
                        }
                        else
                        {
                            _42542 = _17582;
                        }
                        _42554 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _42537, 0.0) * (_17554.x * _17554.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _42538, 0.0) * (_17558.x * _17554.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _42540, 0.0) * (_17554.x * _17558.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _42542, 0.0) * (_17558.x * _17558.y));
                    }
                    else
                    {
                        vec4 _42555;
                        if (_17401 == 3)
                        {
                            vec2 _42530;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _39864 = _17570;
                                _39864.y = 1.0 - _17570.y;
                                _42530 = _39864;
                            }
                            else
                            {
                                _42530 = _17570;
                            }
                            float _17887 = _17570.y;
                            vec2 _17888 = vec2(_17582.x, _17887);
                            vec2 _42531;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _39871 = _17888;
                                _39871.y = 1.0 - _17887;
                                _42531 = _39871;
                            }
                            else
                            {
                                _42531 = _17888;
                            }
                            float _17905 = _17582.y;
                            vec2 _17906 = vec2(_17570.x, _17905);
                            vec2 _42533;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _39878 = _17906;
                                _39878.y = 1.0 - _17905;
                                _42533 = _39878;
                            }
                            else
                            {
                                _42533 = _17906;
                            }
                            vec2 _42535;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _39885 = _17582;
                                _39885.y = 1.0 - _17582.y;
                                _42535 = _39885;
                            }
                            else
                            {
                                _42535 = _17582;
                            }
                            _42555 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _42530, 0.0) * (_17554.x * _17554.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _42531, 0.0) * (_17558.x * _17554.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _42533, 0.0) * (_17554.x * _17558.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _42535, 0.0) * (_17558.x * _17558.y));
                        }
                        else
                        {
                            vec4 _42556;
                            if (_17401 == 4)
                            {
                                vec2 _42523;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _39892 = _17570;
                                    _39892.y = 1.0 - _17570.y;
                                    _42523 = _39892;
                                }
                                else
                                {
                                    _42523 = _17570;
                                }
                                float _18017 = _17570.y;
                                vec2 _18018 = vec2(_17582.x, _18017);
                                vec2 _42524;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _39899 = _18018;
                                    _39899.y = 1.0 - _18017;
                                    _42524 = _39899;
                                }
                                else
                                {
                                    _42524 = _18018;
                                }
                                float _18035 = _17582.y;
                                vec2 _18036 = vec2(_17570.x, _18035);
                                vec2 _42526;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _39906 = _18036;
                                    _39906.y = 1.0 - _18035;
                                    _42526 = _39906;
                                }
                                else
                                {
                                    _42526 = _18036;
                                }
                                vec2 _42528;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _39913 = _17582;
                                    _39913.y = 1.0 - _17582.y;
                                    _42528 = _39913;
                                }
                                else
                                {
                                    _42528 = _17582;
                                }
                                _42556 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _42523, 0.0) * (_17554.x * _17554.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _42524, 0.0) * (_17558.x * _17554.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _42526, 0.0) * (_17554.x * _17558.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _42528, 0.0) * (_17558.x * _17558.y));
                            }
                            else
                            {
                                vec2 _42516;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _39920 = _17570;
                                    _39920.y = 1.0 - _17570.y;
                                    _42516 = _39920;
                                }
                                else
                                {
                                    _42516 = _17570;
                                }
                                float _18147 = _17570.y;
                                vec2 _18148 = vec2(_17582.x, _18147);
                                vec2 _42517;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _39927 = _18148;
                                    _39927.y = 1.0 - _18147;
                                    _42517 = _39927;
                                }
                                else
                                {
                                    _42517 = _18148;
                                }
                                float _18165 = _17582.y;
                                vec2 _18166 = vec2(_17570.x, _18165);
                                vec2 _42519;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _39934 = _18166;
                                    _39934.y = 1.0 - _18165;
                                    _42519 = _39934;
                                }
                                else
                                {
                                    _42519 = _18166;
                                }
                                vec2 _42521;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _39941 = _17582;
                                    _39941.y = 1.0 - _17582.y;
                                    _42521 = _39941;
                                }
                                else
                                {
                                    _42521 = _17582;
                                }
                                _42556 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _42516, 0.0) * (_17554.x * _17554.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _42517, 0.0) * (_17558.x * _17554.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _42519, 0.0) * (_17554.x * _17558.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _42521, 0.0) * (_17558.x * _17558.y));
                            }
                            _42555 = _42556;
                        }
                        _42554 = _42555;
                    }
                    _42553 = _42554;
                }
                _42552 = _42553;
            }
            _17415 = _42585 + (_42552.xyz * (_17386 ? ((_42515 == 0) ? (1.0 - _17381) : _17381) : 1.0));
        }
        bvec3 _6118 = bvec3(_292.gTime.z > 0.5);
        float _18251 = dot(esia_v4.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
        float _6141 = max(_41577, 0.0) / max(esia_v3.w, 0.001000000047497451305389404296875);
        float _6144 = (_18251 > 0.5) ? 0.0 : 1.0;
        float _18263 = clamp(0.85000002384185791015625 * ((clamp((0.5 - abs(_18251 - dot(vec3(_6118.x ? _42585.x : vec3(0.5).x, _6118.y ? _42585.y : vec3(0.5).y, _6118.z ? _42585.z : vec3(0.5).z), vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)))) * 4.0, 0.0, 1.0) * esia_v3.z) * exp(((-_6141) * _6141) * 2.2000000476837158203125)), 0.0, 1.0);
        _44704 = vec4(vec4(_6144, _6144, _6144, 0.85000002384185791015625).xyz * _18263, _18263) + (_42613 * (1.0 - _18263));
    }
    else
    {
        _44704 = _42613;
    }
    vec4 _46051;
    vec4 _46381;
    float _47370;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & _4290) != 0u) && (_42238 > 0.0))
    {
        uint _18288 = max(uint(_292.gConv.z), 1u);
        vec4 _18322 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _18288) * 24u) + 11u), int(esia_v1 / _18288), 0).xy, 0);
        uint _18329 = max(uint(_292.gConv.z), 1u);
        vec4 _18363 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _18329) * 24u) + 12u), int(esia_v1 / _18329), 0).xy, 0);
        uint _18370 = max(uint(_292.gConv.z), 1u);
        vec4 _18404 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _18370) * 24u) + 13u), int(esia_v1 / _18370), 0).xy, 0);
        uint _18411 = max(uint(_292.gConv.z), 1u);
        vec4 _18445 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _18411) * 24u) + 16u), int(esia_v1 / _18411), 0).xy, 0);
        float _18516 = _18322.x * _292.gDisplay.z;
        float _18518 = _18322.y;
        vec2 _18527 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(1.0));
        float _18535 = clamp(_18322.z, 0.001000000047497451305389404296875, min(_18527.x, _18527.y));
        float _18537 = _18322.w;
        float _18544 = clamp(1.0 - (max(-_41577, 0.0) / _18535), 0.0, 1.0);
        float _18550 = sqrt(clamp(1.0 - (_18544 * _18544), 0.0, 1.0));
        vec2 _43599;
        vec3 _44176;
        SPIRV_CROSS_BRANCH
        if (_18544 > 0.0)
        {
            vec2 _43598;
            do
            {
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & _2417) != 0u)
                {
                    vec2 _43597;
                    do
                    {
                        bool _19164 = esia_v3.w > 0.5;
                        float _19168 = floor((esia_v3.z + 1.0) * 0.5);
                        float _19172 = floor((_19168 + 127.0) * 0.0078125);
                        float _43588;
                        vec2 _43590;
                        float _43592;
                        SPIRV_CROSS_BRANCH
                        if (_19164)
                        {
                            uint _19348 = uint(esia_v3.z);
                            uint _19351 = (_19348 + 31u) >> 5u;
                            uint _19354 = esia_v1 + 1u;
                            uint _19437 = max(uint(_292.gConv.z), 1u);
                            ivec2 _19447 = ivec2(int((_19354 % _19437) * 24u), int(_19354 / _19437));
                            ivec2 _43577;
                            uint _43578;
                            ivec2 _48373;
                            _48373 = _19447;
                            _43578 = 0u;
                            _43577 = _19447;
                            float _49446;
                            uint _49447;
                            ivec2 _49449;
                            ivec2 _48376;
                            uint _43576 = 0u;
                            float _48374 = 3.0000000054977557577780399428115e+38;
                            SPIRV_CROSS_LOOP
                            for (; _43576 < _19351; _48374 = _49446, _48373 = _48376, _43578 = _49447, _43577 = _49449, _43576++)
                            {
                                vec4 _19475 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_48373.x + 20, _48373.y, 0).xy, 0);
                                vec2 _19488 = max(max(_19475.xy - esia_v0, esia_v0 - _19475.zw), vec2(0.0));
                                float _19491 = dot(_19488, _19488);
                                bool _19369 = _19491 < _48374;
                                _49446 = _19369 ? _19491 : _48374;
                                _49447 = _19369 ? _43576 : _43578;
                                bvec2 _49448 = bvec2(_19369);
                                _49449 = ivec2(_49448.x ? _48373.x : _43577.x, _49448.y ? _48373.y : _43577.y);
                                int _19496 = _48373.x + 24;
                                ivec2 _39964 = _48373;
                                _39964.x = _19496;
                                if (_19496 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                {
                                    _48376 = ivec2(0, _48373.y + 1);
                                }
                                else
                                {
                                    _48376 = _39964;
                                }
                            }
                            uint _19551 = _43578 * 32u;
                            uint _19555 = min((_19348 - _19551), 32u);
                            vec4 _19682 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43577.x + 21, _43577.y, 0).xy, 0);
                            vec4 _19710 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43577.x + 22, _43577.y, 0).xy, 0);
                            float _43582;
                            vec2 _43583;
                            uint _43584;
                            _43584 = 4294967295u;
                            _43583 = vec2(0.0, -1.0);
                            _43582 = 1000000015047466219876688855040.0;
                            float _49094;
                            vec2 _49095;
                            uint _49096;
                            SPIRV_CROSS_LOOP
                            for (uint _43581 = 0u; (_43581 * 8u) < _19555; _43584 = _49096, _43583 = _49095, _43582 = _49094, _43581++)
                            {
                                vec4 _19738 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43577.x + int(16u + _43581), _43577.y, 0).xy, 0);
                                float _19756 = ((sqrt(_43582) * 1.000003814697265625) + 0.03125) + ((abs(esia_v0.x) + abs(esia_v0.y)) * 3.814697265625e-06);
                                vec2 _19776 = max(max(_19738.xy - esia_v0, esia_v0 - _19738.zw), vec2(0.0));
                                if (dot(_19776, _19776) > (_19756 * _19756))
                                {
                                    _49096 = _43584;
                                    _49095 = _43583;
                                    _49094 = _43582;
                                    continue;
                                }
                                bvec4 _19580 = bvec4(_43581 < 2u);
                                vec4 _19581 = vec4(_19580.x ? _19682.x : _19710.x, _19580.y ? _19682.y : _19710.y, _19580.z ? _19682.z : _19710.z, _19580.w ? _19682.w : _19710.w);
                                vec2 _19586 = _19581.zw;
                                vec2 _19588 = _19581.xy;
                                bvec2 _19589 = bvec2((_43581 & 1u) != 0u);
                                uint _19595 = min(_19555, ((_43581 * 8u) + 8u));
                                uint _19597 = _43581 * 8u;
                                vec2 _48355;
                                float _48356;
                                vec2 _48357;
                                uint _48358;
                                _48358 = _43584;
                                _48357 = _43583;
                                _48356 = _43582;
                                _48355 = vec2(_19589.x ? _19586.x : _19588.x, _19589.y ? _19586.y : _19588.y);
                                vec2 _49384;
                                float _49385;
                                vec2 _49386;
                                uint _49387;
                                SPIRV_CROSS_LOOP
                                for (uint _48354 = _19597; _48354 < _19595; _48358 = _49387, _48357 = _49386, _48356 = _49385, _48355 = _49384, _48354 += 2u)
                                {
                                    vec4 _19807 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43577.x + int(_48354 >> 1u), _43577.y, 0).xy, 0);
                                    uint _19610 = _19551 + _48354;
                                    vec2 _19613 = _19807.xy;
                                    vec2 _19815 = _48355 - _19613;
                                    vec2 _19818 = esia_v0 - _19613;
                                    vec2 _19831 = _19818 - (_19815 * clamp(dot(_19818, _19815) / max(dot(_19815, _19815), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _19834 = dot(_19831, _19831);
                                    bool _19845 = (_19834 < _48356) || ((_19834 == _48356) && (_19610 < _48358));
                                    uint _49450 = _19845 ? _19610 : _48358;
                                    bvec2 _49451 = bvec2(_19845);
                                    vec2 _49452 = vec2(_49451.x ? _19831.x : _48357.x, _49451.y ? _19831.y : _48357.y);
                                    float _49453 = _19845 ? _19834 : _48356;
                                    vec2 _19623 = _19807.xy;
                                    if ((_48354 + 1u) < _19595)
                                    {
                                        uint _19632 = (_19551 + _48354) + 1u;
                                        vec2 _19635 = _19807.zw;
                                        vec2 _19858 = _19623 - _19635;
                                        vec2 _19861 = esia_v0 - _19635;
                                        vec2 _19874 = _19861 - (_19858 * clamp(dot(_19861, _19858) / max(dot(_19858, _19858), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _19877 = dot(_19874, _19874);
                                        bool _19888 = (_19877 < _49453) || ((_19877 == _49453) && (_19632 < _49450));
                                        bvec2 _49455 = bvec2(_19888);
                                        _49387 = _19888 ? _19632 : _49450;
                                        _49386 = vec2(_49455.x ? _19874.x : _49452.x, _49455.y ? _19874.y : _49452.y);
                                        _49385 = _19888 ? _19877 : _49453;
                                        _49384 = _19807.zw;
                                    }
                                    else
                                    {
                                        _49387 = _49450;
                                        _49386 = _49452;
                                        _49385 = _49453;
                                        _49384 = _19623;
                                    }
                                }
                                _49096 = _48358;
                                _49095 = _48357;
                                _49094 = _48356;
                            }
                            uint _19394 = esia_v1 + 1u;
                            uint _19900 = max(uint(_292.gConv.z), 1u);
                            float _43586;
                            vec2 _43587;
                            ivec2 _48323;
                            _48323 = ivec2(int((_19394 % _19900) * 24u), int(_19394 / _19900));
                            _43587 = _43583;
                            _43586 = _43582;
                            ivec2 _48331;
                            float _49098;
                            vec2 _49100;
                            uint _49366;
                            SPIRV_CROSS_LOOP
                            for (uint _43585 = 0u, _48324 = _43584; _43585 < _19351; _48324 = _49366, _48323 = _48331, _43587 = _49100, _43586 = _49098, _43585++)
                            {
                                vec4 _19938 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_48323.x + 20, _48323.y, 0).xy, 0);
                                float _19956 = ((sqrt(_43586) * 1.000003814697265625) + 0.03125) + ((abs(esia_v0.x) + abs(esia_v0.y)) * 3.814697265625e-06);
                                vec2 _19976 = max(max(_19938.xy - esia_v0, esia_v0 - _19938.zw), vec2(0.0));
                                if ((_43585 != _43578) && (!(dot(_19976, _19976) > (_19956 * _19956))))
                                {
                                    uint _20017 = _43585 * 32u;
                                    uint _20021 = min((_19348 - _20017), 32u);
                                    vec4 _20148 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_48323.x + 21, _48323.y, 0).xy, 0);
                                    vec4 _20176 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_48323.x + 22, _48323.y, 0).xy, 0);
                                    float _48326;
                                    vec2 _48327;
                                    uint _48328;
                                    _48328 = _48324;
                                    _48327 = _43587;
                                    _48326 = _43586;
                                    float _49367;
                                    vec2 _49368;
                                    uint _49369;
                                    SPIRV_CROSS_LOOP
                                    for (uint _48325 = 0u; (_48325 * 8u) < _20021; _48328 = _49369, _48327 = _49368, _48326 = _49367, _48325++)
                                    {
                                        vec4 _20204 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_48323.x + int(16u + _48325), _48323.y, 0).xy, 0);
                                        float _20222 = ((sqrt(_48326) * 1.000003814697265625) + 0.03125) + ((abs(esia_v0.x) + abs(esia_v0.y)) * 3.814697265625e-06);
                                        vec2 _20242 = max(max(_20204.xy - esia_v0, esia_v0 - _20204.zw), vec2(0.0));
                                        if (dot(_20242, _20242) > (_20222 * _20222))
                                        {
                                            _49369 = _48328;
                                            _49368 = _48327;
                                            _49367 = _48326;
                                            continue;
                                        }
                                        bvec4 _20046 = bvec4(_48325 < 2u);
                                        vec4 _20047 = vec4(_20046.x ? _20148.x : _20176.x, _20046.y ? _20148.y : _20176.y, _20046.z ? _20148.z : _20176.z, _20046.w ? _20148.w : _20176.w);
                                        vec2 _20052 = _20047.zw;
                                        vec2 _20054 = _20047.xy;
                                        bvec2 _20055 = bvec2((_48325 & 1u) != 0u);
                                        uint _20061 = min(_20021, ((_48325 * 8u) + 8u));
                                        uint _20063 = _48325 * 8u;
                                        vec2 _48336;
                                        float _48337;
                                        vec2 _48338;
                                        uint _48339;
                                        _48339 = _48328;
                                        _48338 = _48327;
                                        _48337 = _48326;
                                        _48336 = vec2(_20055.x ? _20052.x : _20054.x, _20055.y ? _20052.y : _20054.y);
                                        vec2 _49380;
                                        float _49381;
                                        vec2 _49382;
                                        uint _49383;
                                        SPIRV_CROSS_LOOP
                                        for (uint _48335 = _20063; _48335 < _20061; _48339 = _49383, _48338 = _49382, _48337 = _49381, _48336 = _49380, _48335 += 2u)
                                        {
                                            vec4 _20273 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_48323.x + int(_48335 >> 1u), _48323.y, 0).xy, 0);
                                            uint _20076 = _20017 + _48335;
                                            vec2 _20079 = _20273.xy;
                                            vec2 _20281 = _48336 - _20079;
                                            vec2 _20284 = esia_v0 - _20079;
                                            vec2 _20297 = _20284 - (_20281 * clamp(dot(_20284, _20281) / max(dot(_20281, _20281), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _20300 = dot(_20297, _20297);
                                            bool _20311 = (_20300 < _48337) || ((_20300 == _48337) && (_20076 < _48339));
                                            uint _49458 = _20311 ? _20076 : _48339;
                                            bvec2 _49459 = bvec2(_20311);
                                            vec2 _49460 = vec2(_49459.x ? _20297.x : _48338.x, _49459.y ? _20297.y : _48338.y);
                                            float _49461 = _20311 ? _20300 : _48337;
                                            vec2 _20089 = _20273.xy;
                                            if ((_48335 + 1u) < _20061)
                                            {
                                                uint _20098 = (_20017 + _48335) + 1u;
                                                vec2 _20101 = _20273.zw;
                                                vec2 _20324 = _20089 - _20101;
                                                vec2 _20327 = esia_v0 - _20101;
                                                vec2 _20340 = _20327 - (_20324 * clamp(dot(_20327, _20324) / max(dot(_20324, _20324), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _20343 = dot(_20340, _20340);
                                                bool _20354 = (_20343 < _49461) || ((_20343 == _49461) && (_20098 < _49458));
                                                bvec2 _49463 = bvec2(_20354);
                                                _49383 = _20354 ? _20098 : _49458;
                                                _49382 = vec2(_49463.x ? _20340.x : _49460.x, _49463.y ? _20340.y : _49460.y);
                                                _49381 = _20354 ? _20343 : _49461;
                                                _49380 = _20273.zw;
                                            }
                                            else
                                            {
                                                _49383 = _49458;
                                                _49382 = _49460;
                                                _49381 = _49461;
                                                _49380 = _20089;
                                            }
                                        }
                                        _49369 = _48339;
                                        _49368 = _48338;
                                        _49367 = _48337;
                                    }
                                    _49366 = _48328;
                                    _49100 = _48327;
                                    _49098 = _48326;
                                }
                                else
                                {
                                    _49366 = _48324;
                                    _49100 = _43587;
                                    _49098 = _43586;
                                }
                                int _20364 = _48323.x + 24;
                                ivec2 _40011 = _48323;
                                _40011.x = _20364;
                                if (_20364 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                {
                                    _48331 = ivec2(0, _48323.y + 1);
                                }
                                else
                                {
                                    _48331 = _40011;
                                }
                            }
                            _43592 = 1.0;
                            _43590 = _43587;
                            _43588 = _43586;
                        }
                        else
                        {
                            float _19186 = floor((esia_v3.z - 1.0) * 0.5);
                            float _20389 = floor(_19186 * 0.0625);
                            uint _20394 = (esia_v1 + 1u) + uint(_20389);
                            uint _20407 = max(uint(_292.gConv.z), 1u);
                            vec4 _20441 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_20394 % _20407) * 24u) + uint(_19186 - (_20389 * 16.0))), int(_20394 / _20407), 0).xy, 0);
                            vec2 _19198 = _20441.xy;
                            vec2 _19200 = _20441.zw;
                            bvec2 _19201 = bvec2(((esia_v3.z - 1.0) - (_19186 * 2.0)) < 0.5);
                            vec2 _43548;
                            if (_19164)
                            {
                                float _20449 = floor(0.0);
                                uint _20454 = (esia_v1 + 1u) + uint(_20449);
                                uint _20467 = max(uint(_292.gConv.z), 1u);
                                _43548 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_20454 % _20467) * 24u) + uint(_20449 * (-16.0))), int(_20454 / _20467), 0).xy, 0).xy;
                            }
                            else
                            {
                                _43548 = vec2(_19201.x ? _19198.x : _19200.x, _19201.y ? _19198.y : _19200.y);
                            }
                            vec2 _43547;
                            float _43550;
                            vec2 _43553;
                            float _43556;
                            _43556 = 1.0;
                            _43553 = vec2(0.0, -1.0);
                            _43550 = 1000000015047466219876688855040.0;
                            _43547 = _43548;
                            vec2 _43546;
                            float _43549;
                            vec2 _43552;
                            float _43555;
                            SPIRV_CROSS_LOOP
                            for (float _43543 = 0.0; _43543 < 16.0; _43556 = _43555, _43553 = _43552, _43550 = _43549, _43547 = _43546, _43543 += 1.0)
                            {
                                if (_43543 >= _19172)
                                {
                                    break;
                                }
                                _43555 = _43556;
                                _43552 = _43553;
                                _43549 = _43550;
                                _43546 = _43547;
                                vec2 _49079;
                                float _49081;
                                vec2 _49083;
                                float _49085;
                                SPIRV_CROSS_LOOP
                                for (float _43544 = 0.0; _43544 < 128.0; _43555 = _49085, _43552 = _49083, _43549 = _49081, _43546 = _49079, _43544 += 1.0)
                                {
                                    float _19228 = (_43543 * 128.0) + _43544;
                                    if (_19228 >= _19168)
                                    {
                                        break;
                                    }
                                    float _20509 = floor(_19228 * 0.0625);
                                    uint _20514 = (esia_v1 + 1u) + uint(_20509);
                                    uint _20527 = max(uint(_292.gConv.z), 1u);
                                    vec4 _20561 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_20514 % _20527) * 24u) + uint(_19228 - (_20509 * 16.0))), int(_20514 / _20527), 0).xy, 0);
                                    vec2 _19240 = _20561.xy;
                                    vec2 _20572 = _43546 - _19240;
                                    vec2 _20575 = esia_v0 - _19240;
                                    vec2 _20588 = _20575 - (_20572 * clamp(dot(_20575, _20572) / max(dot(_20572, _20572), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _20591 = dot(_20588, _20588);
                                    bool _20594 = _20591 < _43549;
                                    bvec2 _49466 = bvec2(_20594);
                                    vec2 _49467 = vec2(_49466.x ? _20588.x : _43552.x, _49466.y ? _20588.y : _43552.y);
                                    float _49468 = _20594 ? _20591 : _43549;
                                    bool _20603 = esia_v0.y >= _20561.y;
                                    bool _20608 = esia_v0.y < _43546.y;
                                    bool _20619 = (_20572.x * _20575.y) > (_20572.y * _20575.x);
                                    float _43563;
                                    if (((_20603 && _20608) && _20619) || (((!_20603) && (!_20608)) && (!_20619)))
                                    {
                                        _43563 = -_43555;
                                    }
                                    else
                                    {
                                        _43563 = _43555;
                                    }
                                    vec2 _19250 = _20561.xy;
                                    if (((2.0 * _19228) + 1.0) < esia_v3.z)
                                    {
                                        vec2 _19259 = _20561.zw;
                                        vec2 _20648 = _19250 - _19259;
                                        vec2 _20651 = esia_v0 - _19259;
                                        vec2 _20664 = _20651 - (_20648 * clamp(dot(_20651, _20648) / max(dot(_20648, _20648), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _20667 = dot(_20664, _20664);
                                        bool _20670 = _20667 < _49468;
                                        bvec2 _49469 = bvec2(_20670);
                                        bool _20679 = esia_v0.y >= _20561.w;
                                        bool _20684 = esia_v0.y < _20561.y;
                                        bool _20695 = (_20648.x * _20651.y) > (_20648.y * _20651.x);
                                        float _43569;
                                        if (((_20679 && _20684) && _20695) || (((!_20679) && (!_20684)) && (!_20695)))
                                        {
                                            _43569 = -_43563;
                                        }
                                        else
                                        {
                                            _43569 = _43563;
                                        }
                                        _49085 = _43569;
                                        _49083 = vec2(_49469.x ? _20664.x : _49467.x, _49469.y ? _20664.y : _49467.y);
                                        _49081 = _20670 ? _20667 : _49468;
                                        _49079 = _20561.zw;
                                    }
                                    else
                                    {
                                        _49085 = _43563;
                                        _49083 = _49467;
                                        _49081 = _49468;
                                        _49079 = _19250;
                                    }
                                }
                            }
                            _43592 = _43556;
                            _43590 = _43553;
                            _43588 = _43550;
                        }
                        float _19281 = sqrt(_43588);
                        vec2 _19287 = _43590 / vec2(_19281);
                        bvec2 _19288 = bvec2(_19281 > 9.9999999747524270787835121154785e-07);
                        vec2 _19289 = vec2(_19288.x ? _19287.x : vec2(0.0, -1.0).x, _19288.y ? _19287.y : vec2(0.0, -1.0).y);
                        if (_19164)
                        {
                            _43597 = _19289;
                            break;
                        }
                        _43597 = _19289 * _43592;
                        break;
                    } while(false);
                    _43598 = _43597;
                    break;
                }
                vec2 _43542;
                SPIRV_CROSS_BRANCH
                if ((((esia_v7.z == 0u) && (!((esia_v7.x & _2437) != 0u))) && (!((esia_v7.x & _2444) != 0u))) && all(equal(esia_v3.xyz, esia_v3.yzw)))
                {
                    vec2 _19032 = (esia_v2.xy + esia_v2.zw) * 0.5;
                    vec2 _19041 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _19044 = esia_v0 - _19032;
                    float _20744 = min(_19041.x, _19041.y);
                    float _20747 = min((_19044.x > 0.0) ? ((_19044.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_19044.y > 0.0) ? esia_v3.w : esia_v3.x), _20744);
                    float _20753 = _20747 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _43310;
                    float _43311;
                    if (_20753 > _20744)
                    {
                        float _20767 = esia_v5.y * clamp((_20744 - _20747) / max(0.60000002384185791015625 * _20747, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _43311 = _20767;
                        _43310 = _20747 * (1.0 + (0.60000002384185791015625 * _20767));
                    }
                    else
                    {
                        _43311 = esia_v5.y;
                        _43310 = _20753;
                    }
                    vec2 _20796 = (abs((esia_v0 + vec2(0.5, 0.0)) - _19032) - _19041) + vec2(_43310);
                    vec2 _20798 = max(_20796, vec2(0.0));
                    float _43312;
                    SPIRV_CROSS_BRANCH
                    if ((_20798.x > 0.0) && (_20798.y > 0.0))
                    {
                        float _43313;
                        if ((_43311 > 0.001000000047497451305389404296875) && (_43310 > 9.9999997473787516355514526367188e-05))
                        {
                            float _20815 = 2.0 + (2.0 * _43311);
                            vec2 _20820 = _20798 / vec2(max(_43310, 9.9999997473787516355514526367188e-05));
                            _43313 = pow(pow(_20820.x, _20815) + pow(_20820.y, _20815), 1.0 / _20815) * _43310;
                        }
                        else
                        {
                            _43313 = length(_20798);
                        }
                        _43312 = _43313;
                    }
                    else
                    {
                        _43312 = max(_20798.x, _20798.y);
                    }
                    vec2 _20875 = (abs((esia_v0 - vec2(0.5, 0.0)) - _19032) - _19041) + vec2(_43310);
                    vec2 _20877 = max(_20875, vec2(0.0));
                    float _43314;
                    SPIRV_CROSS_BRANCH
                    if ((_20877.x > 0.0) && (_20877.y > 0.0))
                    {
                        float _43315;
                        if ((_43311 > 0.001000000047497451305389404296875) && (_43310 > 9.9999997473787516355514526367188e-05))
                        {
                            float _20894 = 2.0 + (2.0 * _43311);
                            vec2 _20899 = _20877 / vec2(max(_43310, 9.9999997473787516355514526367188e-05));
                            _43315 = pow(pow(_20899.x, _20894) + pow(_20899.y, _20894), 1.0 / _20894) * _43310;
                        }
                        else
                        {
                            _43315 = length(_20877);
                        }
                        _43314 = _43315;
                    }
                    else
                    {
                        _43314 = max(_20877.x, _20877.y);
                    }
                    vec2 _20954 = (abs((esia_v0 + vec2(0.0, 0.5)) - _19032) - _19041) + vec2(_43310);
                    vec2 _20956 = max(_20954, vec2(0.0));
                    float _43534;
                    SPIRV_CROSS_BRANCH
                    if ((_20956.x > 0.0) && (_20956.y > 0.0))
                    {
                        float _43535;
                        if ((_43311 > 0.001000000047497451305389404296875) && (_43310 > 9.9999997473787516355514526367188e-05))
                        {
                            float _20973 = 2.0 + (2.0 * _43311);
                            vec2 _20978 = _20956 / vec2(max(_43310, 9.9999997473787516355514526367188e-05));
                            _43535 = pow(pow(_20978.x, _20973) + pow(_20978.y, _20973), 1.0 / _20973) * _43310;
                        }
                        else
                        {
                            _43535 = length(_20956);
                        }
                        _43534 = _43535;
                    }
                    else
                    {
                        _43534 = max(_20956.x, _20956.y);
                    }
                    vec2 _21033 = (abs((esia_v0 - vec2(0.0, 0.5)) - _19032) - _19041) + vec2(_43310);
                    vec2 _21035 = max(_21033, vec2(0.0));
                    float _43536;
                    SPIRV_CROSS_BRANCH
                    if ((_21035.x > 0.0) && (_21035.y > 0.0))
                    {
                        float _43537;
                        if ((_43311 > 0.001000000047497451305389404296875) && (_43310 > 9.9999997473787516355514526367188e-05))
                        {
                            float _21052 = 2.0 + (2.0 * _43311);
                            vec2 _21057 = _21035 / vec2(max(_43310, 9.9999997473787516355514526367188e-05));
                            _43537 = pow(pow(_21057.x, _21052) + pow(_21057.y, _21052), 1.0 / _21052) * _43310;
                        }
                        else
                        {
                            _43537 = length(_21035);
                        }
                        _43536 = _43537;
                    }
                    else
                    {
                        _43536 = max(_21035.x, _21035.y);
                    }
                    _43542 = vec2(((min(max(_20796.x, _20796.y), 0.0) + _43312) - _43310) - ((min(max(_20875.x, _20875.y), 0.0) + _43314) - _43310), ((min(max(_20954.x, _20954.y), 0.0) + _43534) - _43310) - ((min(max(_21033.x, _21033.y), 0.0) + _43536) - _43310));
                }
                else
                {
                    vec2 _19085 = esia_v0 + vec2(0.5, 0.0);
                    vec2 _21137 = (esia_v2.xy + esia_v2.zw) * 0.5;
                    vec2 _21146 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    float _42972;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _2279) != 0u)
                    {
                        float _21263 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
                        vec4 _21297 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_41462.xy, _41462.zw, (clamp(_19085, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
                        float _21321 = (_21297.w - 0.5) * esia_v3.y;
                        vec2 _21362 = abs(_19085 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _21263), vec2(0.0));
                        _42972 = max((-mix((max(min(_21297.x, _21297.y), min(max(_21297.x, _21297.y), _21297.z)) - 0.5) * esia_v3.x, _21321, clamp(abs(_21321) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _21263, length(max(_21362, vec2(0.0))) + min(max(_21362.x, _21362.y), 0.0)) - esia_v5.w;
                    }
                    else
                    {
                        float _42973;
                        if ((esia_v7.x & _2294) != 0u)
                        {
                            float _42920;
                            do
                            {
                                bool _21428 = esia_v3.w > 0.5;
                                float _21432 = floor((esia_v3.z + 1.0) * 0.5);
                                float _21436 = floor((_21432 + 127.0) * 0.0078125);
                                float _42913;
                                float _42915;
                                SPIRV_CROSS_BRANCH
                                if (_21428)
                                {
                                    uint _21612 = uint(esia_v3.z);
                                    uint _21615 = (_21612 + 31u) >> 5u;
                                    uint _21618 = esia_v1 + 1u;
                                    uint _21701 = max(uint(_292.gConv.z), 1u);
                                    ivec2 _21711 = ivec2(int((_21618 % _21701) * 24u), int(_21618 / _21701));
                                    ivec2 _42905;
                                    uint _42906;
                                    ivec2 _42966;
                                    _42966 = _21711;
                                    _42906 = 0u;
                                    _42905 = _21711;
                                    float _49472;
                                    uint _49473;
                                    ivec2 _49475;
                                    ivec2 _42969;
                                    uint _42904 = 0u;
                                    float _42967 = 3.0000000054977557577780399428115e+38;
                                    SPIRV_CROSS_LOOP
                                    for (; _42904 < _21615; _42967 = _49472, _42966 = _42969, _42906 = _49473, _42905 = _49475, _42904++)
                                    {
                                        vec4 _21739 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42966.x + 20, _42966.y, 0).xy, 0);
                                        vec2 _21752 = max(max(_21739.xy - _19085, _19085 - _21739.zw), vec2(0.0));
                                        float _21755 = dot(_21752, _21752);
                                        bool _21633 = _21755 < _42967;
                                        _49472 = _21633 ? _21755 : _42967;
                                        _49473 = _21633 ? _42904 : _42906;
                                        bvec2 _49474 = bvec2(_21633);
                                        _49475 = ivec2(_49474.x ? _42966.x : _42905.x, _49474.y ? _42966.y : _42905.y);
                                        int _21760 = _42966.x + 24;
                                        ivec2 _40115 = _42966;
                                        _40115.x = _21760;
                                        if (_21760 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                        {
                                            _42969 = ivec2(0, _42966.y + 1);
                                        }
                                        else
                                        {
                                            _42969 = _40115;
                                        }
                                    }
                                    uint _21815 = _42906 * 32u;
                                    uint _21819 = min((_21612 - _21815), 32u);
                                    vec4 _21946 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42905.x + 21, _42905.y, 0).xy, 0);
                                    vec4 _21974 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42905.x + 22, _42905.y, 0).xy, 0);
                                    float _42909;
                                    uint _42910;
                                    _42910 = 4294967295u;
                                    _42909 = 1000000015047466219876688855040.0;
                                    float _48806;
                                    uint _48807;
                                    SPIRV_CROSS_LOOP
                                    for (uint _42908 = 0u; (_42908 * 8u) < _21819; _42910 = _48807, _42909 = _48806, _42908++)
                                    {
                                        vec4 _22002 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42905.x + int(16u + _42908), _42905.y, 0).xy, 0);
                                        float _22020 = ((sqrt(_42909) * 1.000003814697265625) + 0.03125) + ((abs(_19085.x) + abs(_19085.y)) * 3.814697265625e-06);
                                        vec2 _22040 = max(max(_22002.xy - _19085, _19085 - _22002.zw), vec2(0.0));
                                        if (dot(_22040, _22040) > (_22020 * _22020))
                                        {
                                            _48807 = _42910;
                                            _48806 = _42909;
                                            continue;
                                        }
                                        bvec4 _21844 = bvec4(_42908 < 2u);
                                        vec4 _21845 = vec4(_21844.x ? _21946.x : _21974.x, _21844.y ? _21946.y : _21974.y, _21844.z ? _21946.z : _21974.z, _21844.w ? _21946.w : _21974.w);
                                        vec2 _21850 = _21845.zw;
                                        vec2 _21852 = _21845.xy;
                                        bvec2 _21853 = bvec2((_42908 & 1u) != 0u);
                                        uint _21859 = min(_21819, ((_42908 * 8u) + 8u));
                                        uint _21861 = _42908 * 8u;
                                        vec2 _42951;
                                        float _42952;
                                        uint _42953;
                                        _42953 = _42910;
                                        _42952 = _42909;
                                        _42951 = vec2(_21853.x ? _21850.x : _21852.x, _21853.y ? _21850.y : _21852.y);
                                        vec2 _48855;
                                        float _48856;
                                        uint _48857;
                                        SPIRV_CROSS_LOOP
                                        for (uint _42950 = _21861; _42950 < _21859; _42953 = _48857, _42952 = _48856, _42951 = _48855, _42950 += 2u)
                                        {
                                            vec4 _22071 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42905.x + int(_42950 >> 1u), _42905.y, 0).xy, 0);
                                            uint _21874 = _21815 + _42950;
                                            vec2 _21877 = _22071.xy;
                                            vec2 _22079 = _42951 - _21877;
                                            vec2 _22082 = _19085 - _21877;
                                            vec2 _22095 = _22082 - (_22079 * clamp(dot(_22082, _22079) / max(dot(_22079, _22079), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _22098 = dot(_22095, _22095);
                                            bool _22109 = (_22098 < _42952) || ((_22098 == _42952) && (_21874 < _42953));
                                            uint _49476 = _22109 ? _21874 : _42953;
                                            float _49477 = _22109 ? _22098 : _42952;
                                            vec2 _21887 = _22071.xy;
                                            if ((_42950 + 1u) < _21859)
                                            {
                                                uint _21896 = (_21815 + _42950) + 1u;
                                                vec2 _21899 = _22071.zw;
                                                vec2 _22122 = _21887 - _21899;
                                                vec2 _22125 = _19085 - _21899;
                                                vec2 _22138 = _22125 - (_22122 * clamp(dot(_22125, _22122) / max(dot(_22122, _22122), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _22141 = dot(_22138, _22138);
                                                bool _22152 = (_22141 < _49477) || ((_22141 == _49477) && (_21896 < _49476));
                                                _48857 = _22152 ? _21896 : _49476;
                                                _48856 = _22152 ? _22141 : _49477;
                                                _48855 = _22071.zw;
                                            }
                                            else
                                            {
                                                _48857 = _49476;
                                                _48856 = _49477;
                                                _48855 = _21887;
                                            }
                                        }
                                        _48807 = _42953;
                                        _48806 = _42952;
                                    }
                                    uint _21658 = esia_v1 + 1u;
                                    uint _22164 = max(uint(_292.gConv.z), 1u);
                                    float _42912;
                                    ivec2 _42923;
                                    _42923 = ivec2(int((_21658 % _22164) * 24u), int(_21658 / _22164));
                                    _42912 = _42909;
                                    ivec2 _42930;
                                    float _48809;
                                    uint _48839;
                                    SPIRV_CROSS_LOOP
                                    for (uint _42911 = 0u, _42924 = _42910; _42911 < _21615; _42924 = _48839, _42923 = _42930, _42912 = _48809, _42911++)
                                    {
                                        vec4 _22202 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42923.x + 20, _42923.y, 0).xy, 0);
                                        float _22220 = ((sqrt(_42912) * 1.000003814697265625) + 0.03125) + ((abs(_19085.x) + abs(_19085.y)) * 3.814697265625e-06);
                                        vec2 _22240 = max(max(_22202.xy - _19085, _19085 - _22202.zw), vec2(0.0));
                                        if ((_42911 != _42906) && (!(dot(_22240, _22240) > (_22220 * _22220))))
                                        {
                                            uint _22281 = _42911 * 32u;
                                            uint _22285 = min((_21612 - _22281), 32u);
                                            vec4 _22412 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42923.x + 21, _42923.y, 0).xy, 0);
                                            vec4 _22440 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42923.x + 22, _42923.y, 0).xy, 0);
                                            float _42926;
                                            uint _42927;
                                            _42927 = _42924;
                                            _42926 = _42912;
                                            float _48840;
                                            uint _48841;
                                            SPIRV_CROSS_LOOP
                                            for (uint _42925 = 0u; (_42925 * 8u) < _22285; _42927 = _48841, _42926 = _48840, _42925++)
                                            {
                                                vec4 _22468 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42923.x + int(16u + _42925), _42923.y, 0).xy, 0);
                                                float _22486 = ((sqrt(_42926) * 1.000003814697265625) + 0.03125) + ((abs(_19085.x) + abs(_19085.y)) * 3.814697265625e-06);
                                                vec2 _22506 = max(max(_22468.xy - _19085, _19085 - _22468.zw), vec2(0.0));
                                                if (dot(_22506, _22506) > (_22486 * _22486))
                                                {
                                                    _48841 = _42927;
                                                    _48840 = _42926;
                                                    continue;
                                                }
                                                bvec4 _22310 = bvec4(_42925 < 2u);
                                                vec4 _22311 = vec4(_22310.x ? _22412.x : _22440.x, _22310.y ? _22412.y : _22440.y, _22310.z ? _22412.z : _22440.z, _22310.w ? _22412.w : _22440.w);
                                                vec2 _22316 = _22311.zw;
                                                vec2 _22318 = _22311.xy;
                                                bvec2 _22319 = bvec2((_42925 & 1u) != 0u);
                                                uint _22325 = min(_22285, ((_42925 * 8u) + 8u));
                                                uint _22327 = _42925 * 8u;
                                                vec2 _42935;
                                                float _42936;
                                                uint _42937;
                                                _42937 = _42927;
                                                _42936 = _42926;
                                                _42935 = vec2(_22319.x ? _22316.x : _22318.x, _22319.y ? _22316.y : _22318.y);
                                                vec2 _48852;
                                                float _48853;
                                                uint _48854;
                                                SPIRV_CROSS_LOOP
                                                for (uint _42934 = _22327; _42934 < _22325; _42937 = _48854, _42936 = _48853, _42935 = _48852, _42934 += 2u)
                                                {
                                                    vec4 _22537 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_42923.x + int(_42934 >> 1u), _42923.y, 0).xy, 0);
                                                    uint _22340 = _22281 + _42934;
                                                    vec2 _22343 = _22537.xy;
                                                    vec2 _22545 = _42935 - _22343;
                                                    vec2 _22548 = _19085 - _22343;
                                                    vec2 _22561 = _22548 - (_22545 * clamp(dot(_22548, _22545) / max(dot(_22545, _22545), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                    float _22564 = dot(_22561, _22561);
                                                    bool _22575 = (_22564 < _42936) || ((_22564 == _42936) && (_22340 < _42937));
                                                    uint _49480 = _22575 ? _22340 : _42937;
                                                    float _49481 = _22575 ? _22564 : _42936;
                                                    vec2 _22353 = _22537.xy;
                                                    if ((_42934 + 1u) < _22325)
                                                    {
                                                        uint _22362 = (_22281 + _42934) + 1u;
                                                        vec2 _22365 = _22537.zw;
                                                        vec2 _22588 = _22353 - _22365;
                                                        vec2 _22591 = _19085 - _22365;
                                                        vec2 _22604 = _22591 - (_22588 * clamp(dot(_22591, _22588) / max(dot(_22588, _22588), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                        float _22607 = dot(_22604, _22604);
                                                        bool _22618 = (_22607 < _49481) || ((_22607 == _49481) && (_22362 < _49480));
                                                        _48854 = _22618 ? _22362 : _49480;
                                                        _48853 = _22618 ? _22607 : _49481;
                                                        _48852 = _22537.zw;
                                                    }
                                                    else
                                                    {
                                                        _48854 = _49480;
                                                        _48853 = _49481;
                                                        _48852 = _22353;
                                                    }
                                                }
                                                _48841 = _42937;
                                                _48840 = _42936;
                                            }
                                            _48839 = _42927;
                                            _48809 = _42926;
                                        }
                                        else
                                        {
                                            _48839 = _42924;
                                            _48809 = _42912;
                                        }
                                        int _22628 = _42923.x + 24;
                                        ivec2 _40162 = _42923;
                                        _40162.x = _22628;
                                        if (_22628 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                        {
                                            _42930 = ivec2(0, _42923.y + 1);
                                        }
                                        else
                                        {
                                            _42930 = _40162;
                                        }
                                    }
                                    _42915 = 1.0;
                                    _42913 = _42912;
                                }
                                else
                                {
                                    float _21450 = floor((esia_v3.z - 1.0) * 0.5);
                                    float _22653 = floor(_21450 * 0.0625);
                                    uint _22658 = (esia_v1 + 1u) + uint(_22653);
                                    uint _22671 = max(uint(_292.gConv.z), 1u);
                                    vec4 _22705 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_22658 % _22671) * 24u) + uint(_21450 - (_22653 * 16.0))), int(_22658 / _22671), 0).xy, 0);
                                    vec2 _21462 = _22705.xy;
                                    vec2 _21464 = _22705.zw;
                                    bvec2 _21465 = bvec2(((esia_v3.z - 1.0) - (_21450 * 2.0)) < 0.5);
                                    vec2 _42883;
                                    if (_21428)
                                    {
                                        float _22713 = floor(0.0);
                                        uint _22718 = (esia_v1 + 1u) + uint(_22713);
                                        uint _22731 = max(uint(_292.gConv.z), 1u);
                                        _42883 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_22718 % _22731) * 24u) + uint(_22713 * (-16.0))), int(_22718 / _22731), 0).xy, 0).xy;
                                    }
                                    else
                                    {
                                        _42883 = vec2(_21465.x ? _21462.x : _21464.x, _21465.y ? _21462.y : _21464.y);
                                    }
                                    vec2 _42882;
                                    float _42885;
                                    float _42888;
                                    _42888 = 1.0;
                                    _42885 = 1000000015047466219876688855040.0;
                                    _42882 = _42883;
                                    vec2 _42881;
                                    float _42884;
                                    float _42887;
                                    SPIRV_CROSS_LOOP
                                    for (float _42878 = 0.0; _42878 < 16.0; _42888 = _42887, _42885 = _42884, _42882 = _42881, _42878 += 1.0)
                                    {
                                        if (_42878 >= _21436)
                                        {
                                            break;
                                        }
                                        _42887 = _42888;
                                        _42884 = _42885;
                                        _42881 = _42882;
                                        vec2 _48795;
                                        float _48797;
                                        float _48799;
                                        SPIRV_CROSS_LOOP
                                        for (float _42879 = 0.0; _42879 < 128.0; _42887 = _48799, _42884 = _48797, _42881 = _48795, _42879 += 1.0)
                                        {
                                            float _21492 = (_42878 * 128.0) + _42879;
                                            if (_21492 >= _21432)
                                            {
                                                break;
                                            }
                                            float _22773 = floor(_21492 * 0.0625);
                                            uint _22778 = (esia_v1 + 1u) + uint(_22773);
                                            uint _22791 = max(uint(_292.gConv.z), 1u);
                                            vec4 _22825 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_22778 % _22791) * 24u) + uint(_21492 - (_22773 * 16.0))), int(_22778 / _22791), 0).xy, 0);
                                            vec2 _21504 = _22825.xy;
                                            vec2 _22836 = _42881 - _21504;
                                            vec2 _22839 = _19085 - _21504;
                                            vec2 _22852 = _22839 - (_22836 * clamp(dot(_22839, _22836) / max(dot(_22836, _22836), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _22855 = dot(_22852, _22852);
                                            float _49484 = (_22855 < _42884) ? _22855 : _42884;
                                            bool _22867 = _19085.y >= _22825.y;
                                            bool _22872 = _19085.y < _42881.y;
                                            bool _22883 = (_22836.x * _22839.y) > (_22836.y * _22839.x);
                                            float _42893;
                                            if (((_22867 && _22872) && _22883) || (((!_22867) && (!_22872)) && (!_22883)))
                                            {
                                                _42893 = -_42887;
                                            }
                                            else
                                            {
                                                _42893 = _42887;
                                            }
                                            vec2 _21514 = _22825.xy;
                                            if (((2.0 * _21492) + 1.0) < esia_v3.z)
                                            {
                                                vec2 _21523 = _22825.zw;
                                                vec2 _22912 = _21514 - _21523;
                                                vec2 _22915 = _19085 - _21523;
                                                vec2 _22928 = _22915 - (_22912 * clamp(dot(_22915, _22912) / max(dot(_22912, _22912), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _22931 = dot(_22928, _22928);
                                                bool _22943 = _19085.y >= _22825.w;
                                                bool _22948 = _19085.y < _22825.y;
                                                bool _22959 = (_22912.x * _22915.y) > (_22912.y * _22915.x);
                                                float _42897;
                                                if (((_22943 && _22948) && _22959) || (((!_22943) && (!_22948)) && (!_22959)))
                                                {
                                                    _42897 = -_42893;
                                                }
                                                else
                                                {
                                                    _42897 = _42893;
                                                }
                                                _48799 = _42897;
                                                _48797 = (_22931 < _49484) ? _22931 : _49484;
                                                _48795 = _22825.zw;
                                            }
                                            else
                                            {
                                                _48799 = _42893;
                                                _48797 = _49484;
                                                _48795 = _21514;
                                            }
                                        }
                                    }
                                    _42915 = _42888;
                                    _42913 = _42885;
                                }
                                float _21545 = sqrt(_42913);
                                if (_21428)
                                {
                                    _42920 = _21545 - esia_v3.x;
                                    break;
                                }
                                _42920 = (_42915 * _21545) - esia_v3.x;
                                break;
                            } while(false);
                            _42973 = _42920;
                        }
                        else
                        {
                            float _42974;
                            if (esia_v7.z == 1u)
                            {
                                vec2 _21171 = _19085 - _21137;
                                float _42877;
                                do
                                {
                                    if (esia_v5.w >= 6.282185077667236328125)
                                    {
                                        _42877 = abs(length(_21171) - esia_v3.x) - esia_v3.y;
                                        break;
                                    }
                                    float _23002 = esia_v5.z + (esia_v5.w * 0.5);
                                    float _23004 = cos(_23002);
                                    float _23006 = sin(_23002);
                                    float _23015 = dot(_21171, vec2(-_23006, _23004));
                                    float _23018 = dot(_21171, vec2(_23004, _23006));
                                    vec2 _23019 = vec2(_23015, _23018);
                                    float _23022 = abs(_23015);
                                    _23019.x = _23022;
                                    float _23025 = esia_v5.w * 0.5;
                                    float _23027 = sin(_23025);
                                    float _23029 = cos(_23025);
                                    _42877 = (((_23029 * _23022) > (_23027 * _23018)) ? length(_23019 - (vec2(_23027, _23029) * esia_v3.x)) : abs(length(_23019) - esia_v3.x)) - esia_v3.y;
                                    break;
                                } while(false);
                                _42974 = _42877;
                            }
                            else
                            {
                                float _42975;
                                if (esia_v7.z == 2u)
                                {
                                    vec2 _23065 = _19085 - _41468.xy;
                                    vec2 _23068 = _41468.zw - _41468.xy;
                                    _42975 = length(_23065 - (_23068 * clamp(dot(_23065, _23068) / max(dot(_23068, _23068), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                                }
                                else
                                {
                                    vec2 _21198 = _19085 - _21137;
                                    float _23133 = min(_21146.x, _21146.y);
                                    float _23136 = min((_21198.x > 0.0) ? ((_21198.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_21198.y > 0.0) ? esia_v3.w : esia_v3.x), _23133);
                                    float _23142 = _23136 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                    float _42866;
                                    float _42867;
                                    if (_23142 > _23133)
                                    {
                                        float _23156 = esia_v5.y * clamp((_23133 - _23136) / max(0.60000002384185791015625 * _23136, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _42867 = _23156;
                                        _42866 = _23136 * (1.0 + (0.60000002384185791015625 * _23156));
                                    }
                                    else
                                    {
                                        _42867 = esia_v5.y;
                                        _42866 = _23142;
                                    }
                                    vec2 _23185 = (abs(_21198) - _21146) + vec2(_42866);
                                    vec2 _23187 = max(_23185, vec2(0.0));
                                    float _42868;
                                    SPIRV_CROSS_BRANCH
                                    if ((_23187.x > 0.0) && (_23187.y > 0.0))
                                    {
                                        float _42869;
                                        if ((_42867 > 0.001000000047497451305389404296875) && (_42866 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _23204 = 2.0 + (2.0 * _42867);
                                            vec2 _23209 = _23187 / vec2(max(_42866, 9.9999997473787516355514526367188e-05));
                                            _42869 = pow(pow(_23209.x, _23204) + pow(_23209.y, _23204), 1.0 / _23204) * _42866;
                                        }
                                        else
                                        {
                                            _42869 = length(_23187);
                                        }
                                        _42868 = _42869;
                                    }
                                    else
                                    {
                                        _42868 = max(_23187.x, _23187.y);
                                    }
                                    float _23244 = (min(max(_23185.x, _23185.y), 0.0) + _42868) - _42866;
                                    float _42976;
                                    SPIRV_CROSS_BRANCH
                                    if ((esia_v7.x & _2365) != 0u)
                                    {
                                        vec2 _21226 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                                        vec2 _21229 = _19085 - ((_41468.xy + _41468.zw) * 0.5);
                                        float _23292 = min(_21226.x, _21226.y);
                                        float _23295 = min((_21229.x > 0.0) ? ((_21229.y > 0.0) ? _18445.x : _18445.x) : ((_21229.y > 0.0) ? _18445.x : _18445.x), _23292);
                                        float _23301 = _23295 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                        float _42870;
                                        float _42871;
                                        if (_23301 > _23292)
                                        {
                                            float _23315 = esia_v5.y * clamp((_23292 - _23295) / max(0.60000002384185791015625 * _23295, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _42871 = _23315;
                                            _42870 = _23295 * (1.0 + (0.60000002384185791015625 * _23315));
                                        }
                                        else
                                        {
                                            _42871 = esia_v5.y;
                                            _42870 = _23301;
                                        }
                                        vec2 _23344 = (abs(_21229) - _21226) + vec2(_42870);
                                        vec2 _23346 = max(_23344, vec2(0.0));
                                        float _42872;
                                        SPIRV_CROSS_BRANCH
                                        if ((_23346.x > 0.0) && (_23346.y > 0.0))
                                        {
                                            float _42873;
                                            if ((_42871 > 0.001000000047497451305389404296875) && (_42870 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _23363 = 2.0 + (2.0 * _42871);
                                                vec2 _23368 = _23346 / vec2(max(_42870, 9.9999997473787516355514526367188e-05));
                                                _42873 = pow(pow(_23368.x, _23363) + pow(_23368.y, _23363), 1.0 / _23363) * _42870;
                                            }
                                            else
                                            {
                                                _42873 = length(_23346);
                                            }
                                            _42872 = _42873;
                                        }
                                        else
                                        {
                                            _42872 = max(_23346.x, _23346.y);
                                        }
                                        float _23403 = (min(max(_23344.x, _23344.y), 0.0) + _42872) - _42870;
                                        float _23408 = max(_18445.y, 9.9999997473787516355514526367188e-05);
                                        float _23417 = max(_23408 - abs(_23244 - _23403), 0.0) / _23408;
                                        _42976 = min(_23244, _23403) - (((_23417 * _23417) * _23408) * 0.25);
                                    }
                                    else
                                    {
                                        _42976 = _23244;
                                    }
                                    _42975 = _42976;
                                }
                                _42974 = _42975;
                            }
                            _42973 = _42974;
                        }
                        _42972 = _42973;
                    }
                    vec2 _19089 = esia_v0 - vec2(0.5, 0.0);
                    vec2 _23472 = (esia_v2.xy + esia_v2.zw) * 0.5;
                    vec2 _23481 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    float _43083;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _2279) != 0u)
                    {
                        float _23598 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
                        vec4 _23632 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_41462.xy, _41462.zw, (clamp(_19089, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
                        float _23656 = (_23632.w - 0.5) * esia_v3.y;
                        vec2 _23697 = abs(_19089 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _23598), vec2(0.0));
                        _43083 = max((-mix((max(min(_23632.x, _23632.y), min(max(_23632.x, _23632.y), _23632.z)) - 0.5) * esia_v3.x, _23656, clamp(abs(_23656) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _23598, length(max(_23697, vec2(0.0))) + min(max(_23697.x, _23697.y), 0.0)) - esia_v5.w;
                    }
                    else
                    {
                        float _43084;
                        if ((esia_v7.x & _2294) != 0u)
                        {
                            float _43031;
                            do
                            {
                                bool _23763 = esia_v3.w > 0.5;
                                float _23767 = floor((esia_v3.z + 1.0) * 0.5);
                                float _23771 = floor((_23767 + 127.0) * 0.0078125);
                                float _43024;
                                float _43026;
                                SPIRV_CROSS_BRANCH
                                if (_23763)
                                {
                                    uint _23947 = uint(esia_v3.z);
                                    uint _23950 = (_23947 + 31u) >> 5u;
                                    uint _23953 = esia_v1 + 1u;
                                    uint _24036 = max(uint(_292.gConv.z), 1u);
                                    ivec2 _24046 = ivec2(int((_23953 % _24036) * 24u), int(_23953 / _24036));
                                    ivec2 _43016;
                                    uint _43017;
                                    ivec2 _43077;
                                    _43077 = _24046;
                                    _43017 = 0u;
                                    _43016 = _24046;
                                    float _49486;
                                    uint _49487;
                                    ivec2 _49489;
                                    ivec2 _43080;
                                    uint _43015 = 0u;
                                    float _43078 = 3.0000000054977557577780399428115e+38;
                                    SPIRV_CROSS_LOOP
                                    for (; _43015 < _23950; _43078 = _49486, _43077 = _43080, _43017 = _49487, _43016 = _49489, _43015++)
                                    {
                                        vec4 _24074 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43077.x + 20, _43077.y, 0).xy, 0);
                                        vec2 _24087 = max(max(_24074.xy - _19089, _19089 - _24074.zw), vec2(0.0));
                                        float _24090 = dot(_24087, _24087);
                                        bool _23968 = _24090 < _43078;
                                        _49486 = _23968 ? _24090 : _43078;
                                        _49487 = _23968 ? _43015 : _43017;
                                        bvec2 _49488 = bvec2(_23968);
                                        _49489 = ivec2(_49488.x ? _43077.x : _43016.x, _49488.y ? _43077.y : _43016.y);
                                        int _24095 = _43077.x + 24;
                                        ivec2 _40265 = _43077;
                                        _40265.x = _24095;
                                        if (_24095 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                        {
                                            _43080 = ivec2(0, _43077.y + 1);
                                        }
                                        else
                                        {
                                            _43080 = _40265;
                                        }
                                    }
                                    uint _24150 = _43017 * 32u;
                                    uint _24154 = min((_23947 - _24150), 32u);
                                    vec4 _24281 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43016.x + 21, _43016.y, 0).xy, 0);
                                    vec4 _24309 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43016.x + 22, _43016.y, 0).xy, 0);
                                    float _43020;
                                    uint _43021;
                                    _43021 = 4294967295u;
                                    _43020 = 1000000015047466219876688855040.0;
                                    float _48877;
                                    uint _48878;
                                    SPIRV_CROSS_LOOP
                                    for (uint _43019 = 0u; (_43019 * 8u) < _24154; _43021 = _48878, _43020 = _48877, _43019++)
                                    {
                                        vec4 _24337 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43016.x + int(16u + _43019), _43016.y, 0).xy, 0);
                                        float _24355 = ((sqrt(_43020) * 1.000003814697265625) + 0.03125) + ((abs(_19089.x) + abs(_19089.y)) * 3.814697265625e-06);
                                        vec2 _24375 = max(max(_24337.xy - _19089, _19089 - _24337.zw), vec2(0.0));
                                        if (dot(_24375, _24375) > (_24355 * _24355))
                                        {
                                            _48878 = _43021;
                                            _48877 = _43020;
                                            continue;
                                        }
                                        bvec4 _24179 = bvec4(_43019 < 2u);
                                        vec4 _24180 = vec4(_24179.x ? _24281.x : _24309.x, _24179.y ? _24281.y : _24309.y, _24179.z ? _24281.z : _24309.z, _24179.w ? _24281.w : _24309.w);
                                        vec2 _24185 = _24180.zw;
                                        vec2 _24187 = _24180.xy;
                                        bvec2 _24188 = bvec2((_43019 & 1u) != 0u);
                                        uint _24194 = min(_24154, ((_43019 * 8u) + 8u));
                                        uint _24196 = _43019 * 8u;
                                        vec2 _43062;
                                        float _43063;
                                        uint _43064;
                                        _43064 = _43021;
                                        _43063 = _43020;
                                        _43062 = vec2(_24188.x ? _24185.x : _24187.x, _24188.y ? _24185.y : _24187.y);
                                        vec2 _48926;
                                        float _48927;
                                        uint _48928;
                                        SPIRV_CROSS_LOOP
                                        for (uint _43061 = _24196; _43061 < _24194; _43064 = _48928, _43063 = _48927, _43062 = _48926, _43061 += 2u)
                                        {
                                            vec4 _24406 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43016.x + int(_43061 >> 1u), _43016.y, 0).xy, 0);
                                            uint _24209 = _24150 + _43061;
                                            vec2 _24212 = _24406.xy;
                                            vec2 _24414 = _43062 - _24212;
                                            vec2 _24417 = _19089 - _24212;
                                            vec2 _24430 = _24417 - (_24414 * clamp(dot(_24417, _24414) / max(dot(_24414, _24414), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _24433 = dot(_24430, _24430);
                                            bool _24444 = (_24433 < _43063) || ((_24433 == _43063) && (_24209 < _43064));
                                            uint _49490 = _24444 ? _24209 : _43064;
                                            float _49491 = _24444 ? _24433 : _43063;
                                            vec2 _24222 = _24406.xy;
                                            if ((_43061 + 1u) < _24194)
                                            {
                                                uint _24231 = (_24150 + _43061) + 1u;
                                                vec2 _24234 = _24406.zw;
                                                vec2 _24457 = _24222 - _24234;
                                                vec2 _24460 = _19089 - _24234;
                                                vec2 _24473 = _24460 - (_24457 * clamp(dot(_24460, _24457) / max(dot(_24457, _24457), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _24476 = dot(_24473, _24473);
                                                bool _24487 = (_24476 < _49491) || ((_24476 == _49491) && (_24231 < _49490));
                                                _48928 = _24487 ? _24231 : _49490;
                                                _48927 = _24487 ? _24476 : _49491;
                                                _48926 = _24406.zw;
                                            }
                                            else
                                            {
                                                _48928 = _49490;
                                                _48927 = _49491;
                                                _48926 = _24222;
                                            }
                                        }
                                        _48878 = _43064;
                                        _48877 = _43063;
                                    }
                                    uint _23993 = esia_v1 + 1u;
                                    uint _24499 = max(uint(_292.gConv.z), 1u);
                                    float _43023;
                                    ivec2 _43034;
                                    _43034 = ivec2(int((_23993 % _24499) * 24u), int(_23993 / _24499));
                                    _43023 = _43020;
                                    ivec2 _43041;
                                    float _48880;
                                    uint _48910;
                                    SPIRV_CROSS_LOOP
                                    for (uint _43022 = 0u, _43035 = _43021; _43022 < _23950; _43035 = _48910, _43034 = _43041, _43023 = _48880, _43022++)
                                    {
                                        vec4 _24537 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43034.x + 20, _43034.y, 0).xy, 0);
                                        float _24555 = ((sqrt(_43023) * 1.000003814697265625) + 0.03125) + ((abs(_19089.x) + abs(_19089.y)) * 3.814697265625e-06);
                                        vec2 _24575 = max(max(_24537.xy - _19089, _19089 - _24537.zw), vec2(0.0));
                                        if ((_43022 != _43017) && (!(dot(_24575, _24575) > (_24555 * _24555))))
                                        {
                                            uint _24616 = _43022 * 32u;
                                            uint _24620 = min((_23947 - _24616), 32u);
                                            vec4 _24747 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43034.x + 21, _43034.y, 0).xy, 0);
                                            vec4 _24775 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43034.x + 22, _43034.y, 0).xy, 0);
                                            float _43037;
                                            uint _43038;
                                            _43038 = _43035;
                                            _43037 = _43023;
                                            float _48911;
                                            uint _48912;
                                            SPIRV_CROSS_LOOP
                                            for (uint _43036 = 0u; (_43036 * 8u) < _24620; _43038 = _48912, _43037 = _48911, _43036++)
                                            {
                                                vec4 _24803 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43034.x + int(16u + _43036), _43034.y, 0).xy, 0);
                                                float _24821 = ((sqrt(_43037) * 1.000003814697265625) + 0.03125) + ((abs(_19089.x) + abs(_19089.y)) * 3.814697265625e-06);
                                                vec2 _24841 = max(max(_24803.xy - _19089, _19089 - _24803.zw), vec2(0.0));
                                                if (dot(_24841, _24841) > (_24821 * _24821))
                                                {
                                                    _48912 = _43038;
                                                    _48911 = _43037;
                                                    continue;
                                                }
                                                bvec4 _24645 = bvec4(_43036 < 2u);
                                                vec4 _24646 = vec4(_24645.x ? _24747.x : _24775.x, _24645.y ? _24747.y : _24775.y, _24645.z ? _24747.z : _24775.z, _24645.w ? _24747.w : _24775.w);
                                                vec2 _24651 = _24646.zw;
                                                vec2 _24653 = _24646.xy;
                                                bvec2 _24654 = bvec2((_43036 & 1u) != 0u);
                                                uint _24660 = min(_24620, ((_43036 * 8u) + 8u));
                                                uint _24662 = _43036 * 8u;
                                                vec2 _43046;
                                                float _43047;
                                                uint _43048;
                                                _43048 = _43038;
                                                _43047 = _43037;
                                                _43046 = vec2(_24654.x ? _24651.x : _24653.x, _24654.y ? _24651.y : _24653.y);
                                                vec2 _48923;
                                                float _48924;
                                                uint _48925;
                                                SPIRV_CROSS_LOOP
                                                for (uint _43045 = _24662; _43045 < _24660; _43048 = _48925, _43047 = _48924, _43046 = _48923, _43045 += 2u)
                                                {
                                                    vec4 _24872 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43034.x + int(_43045 >> 1u), _43034.y, 0).xy, 0);
                                                    uint _24675 = _24616 + _43045;
                                                    vec2 _24678 = _24872.xy;
                                                    vec2 _24880 = _43046 - _24678;
                                                    vec2 _24883 = _19089 - _24678;
                                                    vec2 _24896 = _24883 - (_24880 * clamp(dot(_24883, _24880) / max(dot(_24880, _24880), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                    float _24899 = dot(_24896, _24896);
                                                    bool _24910 = (_24899 < _43047) || ((_24899 == _43047) && (_24675 < _43048));
                                                    uint _49494 = _24910 ? _24675 : _43048;
                                                    float _49495 = _24910 ? _24899 : _43047;
                                                    vec2 _24688 = _24872.xy;
                                                    if ((_43045 + 1u) < _24660)
                                                    {
                                                        uint _24697 = (_24616 + _43045) + 1u;
                                                        vec2 _24700 = _24872.zw;
                                                        vec2 _24923 = _24688 - _24700;
                                                        vec2 _24926 = _19089 - _24700;
                                                        vec2 _24939 = _24926 - (_24923 * clamp(dot(_24926, _24923) / max(dot(_24923, _24923), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                        float _24942 = dot(_24939, _24939);
                                                        bool _24953 = (_24942 < _49495) || ((_24942 == _49495) && (_24697 < _49494));
                                                        _48925 = _24953 ? _24697 : _49494;
                                                        _48924 = _24953 ? _24942 : _49495;
                                                        _48923 = _24872.zw;
                                                    }
                                                    else
                                                    {
                                                        _48925 = _49494;
                                                        _48924 = _49495;
                                                        _48923 = _24688;
                                                    }
                                                }
                                                _48912 = _43048;
                                                _48911 = _43047;
                                            }
                                            _48910 = _43038;
                                            _48880 = _43037;
                                        }
                                        else
                                        {
                                            _48910 = _43035;
                                            _48880 = _43023;
                                        }
                                        int _24963 = _43034.x + 24;
                                        ivec2 _40312 = _43034;
                                        _40312.x = _24963;
                                        if (_24963 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                        {
                                            _43041 = ivec2(0, _43034.y + 1);
                                        }
                                        else
                                        {
                                            _43041 = _40312;
                                        }
                                    }
                                    _43026 = 1.0;
                                    _43024 = _43023;
                                }
                                else
                                {
                                    float _23785 = floor((esia_v3.z - 1.0) * 0.5);
                                    float _24988 = floor(_23785 * 0.0625);
                                    uint _24993 = (esia_v1 + 1u) + uint(_24988);
                                    uint _25006 = max(uint(_292.gConv.z), 1u);
                                    vec4 _25040 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_24993 % _25006) * 24u) + uint(_23785 - (_24988 * 16.0))), int(_24993 / _25006), 0).xy, 0);
                                    vec2 _23797 = _25040.xy;
                                    vec2 _23799 = _25040.zw;
                                    bvec2 _23800 = bvec2(((esia_v3.z - 1.0) - (_23785 * 2.0)) < 0.5);
                                    vec2 _42994;
                                    if (_23763)
                                    {
                                        float _25048 = floor(0.0);
                                        uint _25053 = (esia_v1 + 1u) + uint(_25048);
                                        uint _25066 = max(uint(_292.gConv.z), 1u);
                                        _42994 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_25053 % _25066) * 24u) + uint(_25048 * (-16.0))), int(_25053 / _25066), 0).xy, 0).xy;
                                    }
                                    else
                                    {
                                        _42994 = vec2(_23800.x ? _23797.x : _23799.x, _23800.y ? _23797.y : _23799.y);
                                    }
                                    vec2 _42993;
                                    float _42996;
                                    float _42999;
                                    _42999 = 1.0;
                                    _42996 = 1000000015047466219876688855040.0;
                                    _42993 = _42994;
                                    vec2 _42992;
                                    float _42995;
                                    float _42998;
                                    SPIRV_CROSS_LOOP
                                    for (float _42989 = 0.0; _42989 < 16.0; _42999 = _42998, _42996 = _42995, _42993 = _42992, _42989 += 1.0)
                                    {
                                        if (_42989 >= _23771)
                                        {
                                            break;
                                        }
                                        _42998 = _42999;
                                        _42995 = _42996;
                                        _42992 = _42993;
                                        vec2 _48866;
                                        float _48868;
                                        float _48870;
                                        SPIRV_CROSS_LOOP
                                        for (float _42990 = 0.0; _42990 < 128.0; _42998 = _48870, _42995 = _48868, _42992 = _48866, _42990 += 1.0)
                                        {
                                            float _23827 = (_42989 * 128.0) + _42990;
                                            if (_23827 >= _23767)
                                            {
                                                break;
                                            }
                                            float _25108 = floor(_23827 * 0.0625);
                                            uint _25113 = (esia_v1 + 1u) + uint(_25108);
                                            uint _25126 = max(uint(_292.gConv.z), 1u);
                                            vec4 _25160 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_25113 % _25126) * 24u) + uint(_23827 - (_25108 * 16.0))), int(_25113 / _25126), 0).xy, 0);
                                            vec2 _23839 = _25160.xy;
                                            vec2 _25171 = _42992 - _23839;
                                            vec2 _25174 = _19089 - _23839;
                                            vec2 _25187 = _25174 - (_25171 * clamp(dot(_25174, _25171) / max(dot(_25171, _25171), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _25190 = dot(_25187, _25187);
                                            float _49498 = (_25190 < _42995) ? _25190 : _42995;
                                            bool _25202 = _19089.y >= _25160.y;
                                            bool _25207 = _19089.y < _42992.y;
                                            bool _25218 = (_25171.x * _25174.y) > (_25171.y * _25174.x);
                                            float _43004;
                                            if (((_25202 && _25207) && _25218) || (((!_25202) && (!_25207)) && (!_25218)))
                                            {
                                                _43004 = -_42998;
                                            }
                                            else
                                            {
                                                _43004 = _42998;
                                            }
                                            vec2 _23849 = _25160.xy;
                                            if (((2.0 * _23827) + 1.0) < esia_v3.z)
                                            {
                                                vec2 _23858 = _25160.zw;
                                                vec2 _25247 = _23849 - _23858;
                                                vec2 _25250 = _19089 - _23858;
                                                vec2 _25263 = _25250 - (_25247 * clamp(dot(_25250, _25247) / max(dot(_25247, _25247), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _25266 = dot(_25263, _25263);
                                                bool _25278 = _19089.y >= _25160.w;
                                                bool _25283 = _19089.y < _25160.y;
                                                bool _25294 = (_25247.x * _25250.y) > (_25247.y * _25250.x);
                                                float _43008;
                                                if (((_25278 && _25283) && _25294) || (((!_25278) && (!_25283)) && (!_25294)))
                                                {
                                                    _43008 = -_43004;
                                                }
                                                else
                                                {
                                                    _43008 = _43004;
                                                }
                                                _48870 = _43008;
                                                _48868 = (_25266 < _49498) ? _25266 : _49498;
                                                _48866 = _25160.zw;
                                            }
                                            else
                                            {
                                                _48870 = _43004;
                                                _48868 = _49498;
                                                _48866 = _23849;
                                            }
                                        }
                                    }
                                    _43026 = _42999;
                                    _43024 = _42996;
                                }
                                float _23880 = sqrt(_43024);
                                if (_23763)
                                {
                                    _43031 = _23880 - esia_v3.x;
                                    break;
                                }
                                _43031 = (_43026 * _23880) - esia_v3.x;
                                break;
                            } while(false);
                            _43084 = _43031;
                        }
                        else
                        {
                            float _43085;
                            if (esia_v7.z == 1u)
                            {
                                vec2 _23506 = _19089 - _23472;
                                float _42988;
                                do
                                {
                                    if (esia_v5.w >= 6.282185077667236328125)
                                    {
                                        _42988 = abs(length(_23506) - esia_v3.x) - esia_v3.y;
                                        break;
                                    }
                                    float _25337 = esia_v5.z + (esia_v5.w * 0.5);
                                    float _25339 = cos(_25337);
                                    float _25341 = sin(_25337);
                                    float _25350 = dot(_23506, vec2(-_25341, _25339));
                                    float _25353 = dot(_23506, vec2(_25339, _25341));
                                    vec2 _25354 = vec2(_25350, _25353);
                                    float _25357 = abs(_25350);
                                    _25354.x = _25357;
                                    float _25360 = esia_v5.w * 0.5;
                                    float _25362 = sin(_25360);
                                    float _25364 = cos(_25360);
                                    _42988 = (((_25364 * _25357) > (_25362 * _25353)) ? length(_25354 - (vec2(_25362, _25364) * esia_v3.x)) : abs(length(_25354) - esia_v3.x)) - esia_v3.y;
                                    break;
                                } while(false);
                                _43085 = _42988;
                            }
                            else
                            {
                                float _43086;
                                if (esia_v7.z == 2u)
                                {
                                    vec2 _25400 = _19089 - _41468.xy;
                                    vec2 _25403 = _41468.zw - _41468.xy;
                                    _43086 = length(_25400 - (_25403 * clamp(dot(_25400, _25403) / max(dot(_25403, _25403), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                                }
                                else
                                {
                                    vec2 _23533 = _19089 - _23472;
                                    float _25468 = min(_23481.x, _23481.y);
                                    float _25471 = min((_23533.x > 0.0) ? ((_23533.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_23533.y > 0.0) ? esia_v3.w : esia_v3.x), _25468);
                                    float _25477 = _25471 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                    float _42977;
                                    float _42978;
                                    if (_25477 > _25468)
                                    {
                                        float _25491 = esia_v5.y * clamp((_25468 - _25471) / max(0.60000002384185791015625 * _25471, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _42978 = _25491;
                                        _42977 = _25471 * (1.0 + (0.60000002384185791015625 * _25491));
                                    }
                                    else
                                    {
                                        _42978 = esia_v5.y;
                                        _42977 = _25477;
                                    }
                                    vec2 _25520 = (abs(_23533) - _23481) + vec2(_42977);
                                    vec2 _25522 = max(_25520, vec2(0.0));
                                    float _42979;
                                    SPIRV_CROSS_BRANCH
                                    if ((_25522.x > 0.0) && (_25522.y > 0.0))
                                    {
                                        float _42980;
                                        if ((_42978 > 0.001000000047497451305389404296875) && (_42977 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _25539 = 2.0 + (2.0 * _42978);
                                            vec2 _25544 = _25522 / vec2(max(_42977, 9.9999997473787516355514526367188e-05));
                                            _42980 = pow(pow(_25544.x, _25539) + pow(_25544.y, _25539), 1.0 / _25539) * _42977;
                                        }
                                        else
                                        {
                                            _42980 = length(_25522);
                                        }
                                        _42979 = _42980;
                                    }
                                    else
                                    {
                                        _42979 = max(_25522.x, _25522.y);
                                    }
                                    float _25579 = (min(max(_25520.x, _25520.y), 0.0) + _42979) - _42977;
                                    float _43087;
                                    SPIRV_CROSS_BRANCH
                                    if ((esia_v7.x & _2365) != 0u)
                                    {
                                        vec2 _23561 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                                        vec2 _23564 = _19089 - ((_41468.xy + _41468.zw) * 0.5);
                                        float _25627 = min(_23561.x, _23561.y);
                                        float _25630 = min((_23564.x > 0.0) ? ((_23564.y > 0.0) ? _18445.x : _18445.x) : ((_23564.y > 0.0) ? _18445.x : _18445.x), _25627);
                                        float _25636 = _25630 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                        float _42981;
                                        float _42982;
                                        if (_25636 > _25627)
                                        {
                                            float _25650 = esia_v5.y * clamp((_25627 - _25630) / max(0.60000002384185791015625 * _25630, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _42982 = _25650;
                                            _42981 = _25630 * (1.0 + (0.60000002384185791015625 * _25650));
                                        }
                                        else
                                        {
                                            _42982 = esia_v5.y;
                                            _42981 = _25636;
                                        }
                                        vec2 _25679 = (abs(_23564) - _23561) + vec2(_42981);
                                        vec2 _25681 = max(_25679, vec2(0.0));
                                        float _42983;
                                        SPIRV_CROSS_BRANCH
                                        if ((_25681.x > 0.0) && (_25681.y > 0.0))
                                        {
                                            float _42984;
                                            if ((_42982 > 0.001000000047497451305389404296875) && (_42981 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _25698 = 2.0 + (2.0 * _42982);
                                                vec2 _25703 = _25681 / vec2(max(_42981, 9.9999997473787516355514526367188e-05));
                                                _42984 = pow(pow(_25703.x, _25698) + pow(_25703.y, _25698), 1.0 / _25698) * _42981;
                                            }
                                            else
                                            {
                                                _42984 = length(_25681);
                                            }
                                            _42983 = _42984;
                                        }
                                        else
                                        {
                                            _42983 = max(_25681.x, _25681.y);
                                        }
                                        float _25738 = (min(max(_25679.x, _25679.y), 0.0) + _42983) - _42981;
                                        float _25743 = max(_18445.y, 9.9999997473787516355514526367188e-05);
                                        float _25752 = max(_25743 - abs(_25579 - _25738), 0.0) / _25743;
                                        _43087 = min(_25579, _25738) - (((_25752 * _25752) * _25743) * 0.25);
                                    }
                                    else
                                    {
                                        _43087 = _25579;
                                    }
                                    _43086 = _43087;
                                }
                                _43085 = _43086;
                            }
                            _43084 = _43085;
                        }
                        _43083 = _43084;
                    }
                    vec2 _19094 = esia_v0 + vec2(0.0, 0.5);
                    vec2 _25807 = (esia_v2.xy + esia_v2.zw) * 0.5;
                    vec2 _25816 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    float _43194;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _2279) != 0u)
                    {
                        float _25933 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
                        vec4 _25967 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_41462.xy, _41462.zw, (clamp(_19094, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
                        float _25991 = (_25967.w - 0.5) * esia_v3.y;
                        vec2 _26032 = abs(_19094 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _25933), vec2(0.0));
                        _43194 = max((-mix((max(min(_25967.x, _25967.y), min(max(_25967.x, _25967.y), _25967.z)) - 0.5) * esia_v3.x, _25991, clamp(abs(_25991) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _25933, length(max(_26032, vec2(0.0))) + min(max(_26032.x, _26032.y), 0.0)) - esia_v5.w;
                    }
                    else
                    {
                        float _43195;
                        if ((esia_v7.x & _2294) != 0u)
                        {
                            float _43142;
                            do
                            {
                                bool _26098 = esia_v3.w > 0.5;
                                float _26102 = floor((esia_v3.z + 1.0) * 0.5);
                                float _26106 = floor((_26102 + 127.0) * 0.0078125);
                                float _43135;
                                float _43137;
                                SPIRV_CROSS_BRANCH
                                if (_26098)
                                {
                                    uint _26282 = uint(esia_v3.z);
                                    uint _26285 = (_26282 + 31u) >> 5u;
                                    uint _26288 = esia_v1 + 1u;
                                    uint _26371 = max(uint(_292.gConv.z), 1u);
                                    ivec2 _26381 = ivec2(int((_26288 % _26371) * 24u), int(_26288 / _26371));
                                    ivec2 _43127;
                                    uint _43128;
                                    ivec2 _43188;
                                    _43188 = _26381;
                                    _43128 = 0u;
                                    _43127 = _26381;
                                    float _49500;
                                    uint _49501;
                                    ivec2 _49503;
                                    ivec2 _43191;
                                    uint _43126 = 0u;
                                    float _43189 = 3.0000000054977557577780399428115e+38;
                                    SPIRV_CROSS_LOOP
                                    for (; _43126 < _26285; _43189 = _49500, _43188 = _43191, _43128 = _49501, _43127 = _49503, _43126++)
                                    {
                                        vec4 _26409 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43188.x + 20, _43188.y, 0).xy, 0);
                                        vec2 _26422 = max(max(_26409.xy - _19094, _19094 - _26409.zw), vec2(0.0));
                                        float _26425 = dot(_26422, _26422);
                                        bool _26303 = _26425 < _43189;
                                        _49500 = _26303 ? _26425 : _43189;
                                        _49501 = _26303 ? _43126 : _43128;
                                        bvec2 _49502 = bvec2(_26303);
                                        _49503 = ivec2(_49502.x ? _43188.x : _43127.x, _49502.y ? _43188.y : _43127.y);
                                        int _26430 = _43188.x + 24;
                                        ivec2 _40415 = _43188;
                                        _40415.x = _26430;
                                        if (_26430 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                        {
                                            _43191 = ivec2(0, _43188.y + 1);
                                        }
                                        else
                                        {
                                            _43191 = _40415;
                                        }
                                    }
                                    uint _26485 = _43128 * 32u;
                                    uint _26489 = min((_26282 - _26485), 32u);
                                    vec4 _26616 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43127.x + 21, _43127.y, 0).xy, 0);
                                    vec4 _26644 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43127.x + 22, _43127.y, 0).xy, 0);
                                    float _43131;
                                    uint _43132;
                                    _43132 = 4294967295u;
                                    _43131 = 1000000015047466219876688855040.0;
                                    float _48948;
                                    uint _48949;
                                    SPIRV_CROSS_LOOP
                                    for (uint _43130 = 0u; (_43130 * 8u) < _26489; _43132 = _48949, _43131 = _48948, _43130++)
                                    {
                                        vec4 _26672 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43127.x + int(16u + _43130), _43127.y, 0).xy, 0);
                                        float _26690 = ((sqrt(_43131) * 1.000003814697265625) + 0.03125) + ((abs(_19094.x) + abs(_19094.y)) * 3.814697265625e-06);
                                        vec2 _26710 = max(max(_26672.xy - _19094, _19094 - _26672.zw), vec2(0.0));
                                        if (dot(_26710, _26710) > (_26690 * _26690))
                                        {
                                            _48949 = _43132;
                                            _48948 = _43131;
                                            continue;
                                        }
                                        bvec4 _26514 = bvec4(_43130 < 2u);
                                        vec4 _26515 = vec4(_26514.x ? _26616.x : _26644.x, _26514.y ? _26616.y : _26644.y, _26514.z ? _26616.z : _26644.z, _26514.w ? _26616.w : _26644.w);
                                        vec2 _26520 = _26515.zw;
                                        vec2 _26522 = _26515.xy;
                                        bvec2 _26523 = bvec2((_43130 & 1u) != 0u);
                                        uint _26529 = min(_26489, ((_43130 * 8u) + 8u));
                                        uint _26531 = _43130 * 8u;
                                        vec2 _43173;
                                        float _43174;
                                        uint _43175;
                                        _43175 = _43132;
                                        _43174 = _43131;
                                        _43173 = vec2(_26523.x ? _26520.x : _26522.x, _26523.y ? _26520.y : _26522.y);
                                        vec2 _48997;
                                        float _48998;
                                        uint _48999;
                                        SPIRV_CROSS_LOOP
                                        for (uint _43172 = _26531; _43172 < _26529; _43175 = _48999, _43174 = _48998, _43173 = _48997, _43172 += 2u)
                                        {
                                            vec4 _26741 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43127.x + int(_43172 >> 1u), _43127.y, 0).xy, 0);
                                            uint _26544 = _26485 + _43172;
                                            vec2 _26547 = _26741.xy;
                                            vec2 _26749 = _43173 - _26547;
                                            vec2 _26752 = _19094 - _26547;
                                            vec2 _26765 = _26752 - (_26749 * clamp(dot(_26752, _26749) / max(dot(_26749, _26749), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _26768 = dot(_26765, _26765);
                                            bool _26779 = (_26768 < _43174) || ((_26768 == _43174) && (_26544 < _43175));
                                            uint _49504 = _26779 ? _26544 : _43175;
                                            float _49505 = _26779 ? _26768 : _43174;
                                            vec2 _26557 = _26741.xy;
                                            if ((_43172 + 1u) < _26529)
                                            {
                                                uint _26566 = (_26485 + _43172) + 1u;
                                                vec2 _26569 = _26741.zw;
                                                vec2 _26792 = _26557 - _26569;
                                                vec2 _26795 = _19094 - _26569;
                                                vec2 _26808 = _26795 - (_26792 * clamp(dot(_26795, _26792) / max(dot(_26792, _26792), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _26811 = dot(_26808, _26808);
                                                bool _26822 = (_26811 < _49505) || ((_26811 == _49505) && (_26566 < _49504));
                                                _48999 = _26822 ? _26566 : _49504;
                                                _48998 = _26822 ? _26811 : _49505;
                                                _48997 = _26741.zw;
                                            }
                                            else
                                            {
                                                _48999 = _49504;
                                                _48998 = _49505;
                                                _48997 = _26557;
                                            }
                                        }
                                        _48949 = _43175;
                                        _48948 = _43174;
                                    }
                                    uint _26328 = esia_v1 + 1u;
                                    uint _26834 = max(uint(_292.gConv.z), 1u);
                                    float _43134;
                                    ivec2 _43145;
                                    _43145 = ivec2(int((_26328 % _26834) * 24u), int(_26328 / _26834));
                                    _43134 = _43131;
                                    ivec2 _43152;
                                    float _48951;
                                    uint _48981;
                                    SPIRV_CROSS_LOOP
                                    for (uint _43133 = 0u, _43146 = _43132; _43133 < _26285; _43146 = _48981, _43145 = _43152, _43134 = _48951, _43133++)
                                    {
                                        vec4 _26872 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43145.x + 20, _43145.y, 0).xy, 0);
                                        float _26890 = ((sqrt(_43134) * 1.000003814697265625) + 0.03125) + ((abs(_19094.x) + abs(_19094.y)) * 3.814697265625e-06);
                                        vec2 _26910 = max(max(_26872.xy - _19094, _19094 - _26872.zw), vec2(0.0));
                                        if ((_43133 != _43128) && (!(dot(_26910, _26910) > (_26890 * _26890))))
                                        {
                                            uint _26951 = _43133 * 32u;
                                            uint _26955 = min((_26282 - _26951), 32u);
                                            vec4 _27082 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43145.x + 21, _43145.y, 0).xy, 0);
                                            vec4 _27110 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43145.x + 22, _43145.y, 0).xy, 0);
                                            float _43148;
                                            uint _43149;
                                            _43149 = _43146;
                                            _43148 = _43134;
                                            float _48982;
                                            uint _48983;
                                            SPIRV_CROSS_LOOP
                                            for (uint _43147 = 0u; (_43147 * 8u) < _26955; _43149 = _48983, _43148 = _48982, _43147++)
                                            {
                                                vec4 _27138 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43145.x + int(16u + _43147), _43145.y, 0).xy, 0);
                                                float _27156 = ((sqrt(_43148) * 1.000003814697265625) + 0.03125) + ((abs(_19094.x) + abs(_19094.y)) * 3.814697265625e-06);
                                                vec2 _27176 = max(max(_27138.xy - _19094, _19094 - _27138.zw), vec2(0.0));
                                                if (dot(_27176, _27176) > (_27156 * _27156))
                                                {
                                                    _48983 = _43149;
                                                    _48982 = _43148;
                                                    continue;
                                                }
                                                bvec4 _26980 = bvec4(_43147 < 2u);
                                                vec4 _26981 = vec4(_26980.x ? _27082.x : _27110.x, _26980.y ? _27082.y : _27110.y, _26980.z ? _27082.z : _27110.z, _26980.w ? _27082.w : _27110.w);
                                                vec2 _26986 = _26981.zw;
                                                vec2 _26988 = _26981.xy;
                                                bvec2 _26989 = bvec2((_43147 & 1u) != 0u);
                                                uint _26995 = min(_26955, ((_43147 * 8u) + 8u));
                                                uint _26997 = _43147 * 8u;
                                                vec2 _43157;
                                                float _43158;
                                                uint _43159;
                                                _43159 = _43149;
                                                _43158 = _43148;
                                                _43157 = vec2(_26989.x ? _26986.x : _26988.x, _26989.y ? _26986.y : _26988.y);
                                                vec2 _48994;
                                                float _48995;
                                                uint _48996;
                                                SPIRV_CROSS_LOOP
                                                for (uint _43156 = _26997; _43156 < _26995; _43159 = _48996, _43158 = _48995, _43157 = _48994, _43156 += 2u)
                                                {
                                                    vec4 _27207 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43145.x + int(_43156 >> 1u), _43145.y, 0).xy, 0);
                                                    uint _27010 = _26951 + _43156;
                                                    vec2 _27013 = _27207.xy;
                                                    vec2 _27215 = _43157 - _27013;
                                                    vec2 _27218 = _19094 - _27013;
                                                    vec2 _27231 = _27218 - (_27215 * clamp(dot(_27218, _27215) / max(dot(_27215, _27215), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                    float _27234 = dot(_27231, _27231);
                                                    bool _27245 = (_27234 < _43158) || ((_27234 == _43158) && (_27010 < _43159));
                                                    uint _49508 = _27245 ? _27010 : _43159;
                                                    float _49509 = _27245 ? _27234 : _43158;
                                                    vec2 _27023 = _27207.xy;
                                                    if ((_43156 + 1u) < _26995)
                                                    {
                                                        uint _27032 = (_26951 + _43156) + 1u;
                                                        vec2 _27035 = _27207.zw;
                                                        vec2 _27258 = _27023 - _27035;
                                                        vec2 _27261 = _19094 - _27035;
                                                        vec2 _27274 = _27261 - (_27258 * clamp(dot(_27261, _27258) / max(dot(_27258, _27258), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                        float _27277 = dot(_27274, _27274);
                                                        bool _27288 = (_27277 < _49509) || ((_27277 == _49509) && (_27032 < _49508));
                                                        _48996 = _27288 ? _27032 : _49508;
                                                        _48995 = _27288 ? _27277 : _49509;
                                                        _48994 = _27207.zw;
                                                    }
                                                    else
                                                    {
                                                        _48996 = _49508;
                                                        _48995 = _49509;
                                                        _48994 = _27023;
                                                    }
                                                }
                                                _48983 = _43159;
                                                _48982 = _43158;
                                            }
                                            _48981 = _43149;
                                            _48951 = _43148;
                                        }
                                        else
                                        {
                                            _48981 = _43146;
                                            _48951 = _43134;
                                        }
                                        int _27298 = _43145.x + 24;
                                        ivec2 _40462 = _43145;
                                        _40462.x = _27298;
                                        if (_27298 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                        {
                                            _43152 = ivec2(0, _43145.y + 1);
                                        }
                                        else
                                        {
                                            _43152 = _40462;
                                        }
                                    }
                                    _43137 = 1.0;
                                    _43135 = _43134;
                                }
                                else
                                {
                                    float _26120 = floor((esia_v3.z - 1.0) * 0.5);
                                    float _27323 = floor(_26120 * 0.0625);
                                    uint _27328 = (esia_v1 + 1u) + uint(_27323);
                                    uint _27341 = max(uint(_292.gConv.z), 1u);
                                    vec4 _27375 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_27328 % _27341) * 24u) + uint(_26120 - (_27323 * 16.0))), int(_27328 / _27341), 0).xy, 0);
                                    vec2 _26132 = _27375.xy;
                                    vec2 _26134 = _27375.zw;
                                    bvec2 _26135 = bvec2(((esia_v3.z - 1.0) - (_26120 * 2.0)) < 0.5);
                                    vec2 _43105;
                                    if (_26098)
                                    {
                                        float _27383 = floor(0.0);
                                        uint _27388 = (esia_v1 + 1u) + uint(_27383);
                                        uint _27401 = max(uint(_292.gConv.z), 1u);
                                        _43105 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_27388 % _27401) * 24u) + uint(_27383 * (-16.0))), int(_27388 / _27401), 0).xy, 0).xy;
                                    }
                                    else
                                    {
                                        _43105 = vec2(_26135.x ? _26132.x : _26134.x, _26135.y ? _26132.y : _26134.y);
                                    }
                                    vec2 _43104;
                                    float _43107;
                                    float _43110;
                                    _43110 = 1.0;
                                    _43107 = 1000000015047466219876688855040.0;
                                    _43104 = _43105;
                                    vec2 _43103;
                                    float _43106;
                                    float _43109;
                                    SPIRV_CROSS_LOOP
                                    for (float _43100 = 0.0; _43100 < 16.0; _43110 = _43109, _43107 = _43106, _43104 = _43103, _43100 += 1.0)
                                    {
                                        if (_43100 >= _26106)
                                        {
                                            break;
                                        }
                                        _43109 = _43110;
                                        _43106 = _43107;
                                        _43103 = _43104;
                                        vec2 _48937;
                                        float _48939;
                                        float _48941;
                                        SPIRV_CROSS_LOOP
                                        for (float _43101 = 0.0; _43101 < 128.0; _43109 = _48941, _43106 = _48939, _43103 = _48937, _43101 += 1.0)
                                        {
                                            float _26162 = (_43100 * 128.0) + _43101;
                                            if (_26162 >= _26102)
                                            {
                                                break;
                                            }
                                            float _27443 = floor(_26162 * 0.0625);
                                            uint _27448 = (esia_v1 + 1u) + uint(_27443);
                                            uint _27461 = max(uint(_292.gConv.z), 1u);
                                            vec4 _27495 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_27448 % _27461) * 24u) + uint(_26162 - (_27443 * 16.0))), int(_27448 / _27461), 0).xy, 0);
                                            vec2 _26174 = _27495.xy;
                                            vec2 _27506 = _43103 - _26174;
                                            vec2 _27509 = _19094 - _26174;
                                            vec2 _27522 = _27509 - (_27506 * clamp(dot(_27509, _27506) / max(dot(_27506, _27506), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _27525 = dot(_27522, _27522);
                                            float _49512 = (_27525 < _43106) ? _27525 : _43106;
                                            bool _27537 = _19094.y >= _27495.y;
                                            bool _27542 = _19094.y < _43103.y;
                                            bool _27553 = (_27506.x * _27509.y) > (_27506.y * _27509.x);
                                            float _43115;
                                            if (((_27537 && _27542) && _27553) || (((!_27537) && (!_27542)) && (!_27553)))
                                            {
                                                _43115 = -_43109;
                                            }
                                            else
                                            {
                                                _43115 = _43109;
                                            }
                                            vec2 _26184 = _27495.xy;
                                            if (((2.0 * _26162) + 1.0) < esia_v3.z)
                                            {
                                                vec2 _26193 = _27495.zw;
                                                vec2 _27582 = _26184 - _26193;
                                                vec2 _27585 = _19094 - _26193;
                                                vec2 _27598 = _27585 - (_27582 * clamp(dot(_27585, _27582) / max(dot(_27582, _27582), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _27601 = dot(_27598, _27598);
                                                bool _27613 = _19094.y >= _27495.w;
                                                bool _27618 = _19094.y < _27495.y;
                                                bool _27629 = (_27582.x * _27585.y) > (_27582.y * _27585.x);
                                                float _43119;
                                                if (((_27613 && _27618) && _27629) || (((!_27613) && (!_27618)) && (!_27629)))
                                                {
                                                    _43119 = -_43115;
                                                }
                                                else
                                                {
                                                    _43119 = _43115;
                                                }
                                                _48941 = _43119;
                                                _48939 = (_27601 < _49512) ? _27601 : _49512;
                                                _48937 = _27495.zw;
                                            }
                                            else
                                            {
                                                _48941 = _43115;
                                                _48939 = _49512;
                                                _48937 = _26184;
                                            }
                                        }
                                    }
                                    _43137 = _43110;
                                    _43135 = _43107;
                                }
                                float _26215 = sqrt(_43135);
                                if (_26098)
                                {
                                    _43142 = _26215 - esia_v3.x;
                                    break;
                                }
                                _43142 = (_43137 * _26215) - esia_v3.x;
                                break;
                            } while(false);
                            _43195 = _43142;
                        }
                        else
                        {
                            float _43196;
                            if (esia_v7.z == 1u)
                            {
                                vec2 _25841 = _19094 - _25807;
                                float _43099;
                                do
                                {
                                    if (esia_v5.w >= 6.282185077667236328125)
                                    {
                                        _43099 = abs(length(_25841) - esia_v3.x) - esia_v3.y;
                                        break;
                                    }
                                    float _27672 = esia_v5.z + (esia_v5.w * 0.5);
                                    float _27674 = cos(_27672);
                                    float _27676 = sin(_27672);
                                    float _27685 = dot(_25841, vec2(-_27676, _27674));
                                    float _27688 = dot(_25841, vec2(_27674, _27676));
                                    vec2 _27689 = vec2(_27685, _27688);
                                    float _27692 = abs(_27685);
                                    _27689.x = _27692;
                                    float _27695 = esia_v5.w * 0.5;
                                    float _27697 = sin(_27695);
                                    float _27699 = cos(_27695);
                                    _43099 = (((_27699 * _27692) > (_27697 * _27688)) ? length(_27689 - (vec2(_27697, _27699) * esia_v3.x)) : abs(length(_27689) - esia_v3.x)) - esia_v3.y;
                                    break;
                                } while(false);
                                _43196 = _43099;
                            }
                            else
                            {
                                float _43197;
                                if (esia_v7.z == 2u)
                                {
                                    vec2 _27735 = _19094 - _41468.xy;
                                    vec2 _27738 = _41468.zw - _41468.xy;
                                    _43197 = length(_27735 - (_27738 * clamp(dot(_27735, _27738) / max(dot(_27738, _27738), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                                }
                                else
                                {
                                    vec2 _25868 = _19094 - _25807;
                                    float _27803 = min(_25816.x, _25816.y);
                                    float _27806 = min((_25868.x > 0.0) ? ((_25868.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_25868.y > 0.0) ? esia_v3.w : esia_v3.x), _27803);
                                    float _27812 = _27806 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                    float _43088;
                                    float _43089;
                                    if (_27812 > _27803)
                                    {
                                        float _27826 = esia_v5.y * clamp((_27803 - _27806) / max(0.60000002384185791015625 * _27806, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _43089 = _27826;
                                        _43088 = _27806 * (1.0 + (0.60000002384185791015625 * _27826));
                                    }
                                    else
                                    {
                                        _43089 = esia_v5.y;
                                        _43088 = _27812;
                                    }
                                    vec2 _27855 = (abs(_25868) - _25816) + vec2(_43088);
                                    vec2 _27857 = max(_27855, vec2(0.0));
                                    float _43090;
                                    SPIRV_CROSS_BRANCH
                                    if ((_27857.x > 0.0) && (_27857.y > 0.0))
                                    {
                                        float _43091;
                                        if ((_43089 > 0.001000000047497451305389404296875) && (_43088 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _27874 = 2.0 + (2.0 * _43089);
                                            vec2 _27879 = _27857 / vec2(max(_43088, 9.9999997473787516355514526367188e-05));
                                            _43091 = pow(pow(_27879.x, _27874) + pow(_27879.y, _27874), 1.0 / _27874) * _43088;
                                        }
                                        else
                                        {
                                            _43091 = length(_27857);
                                        }
                                        _43090 = _43091;
                                    }
                                    else
                                    {
                                        _43090 = max(_27857.x, _27857.y);
                                    }
                                    float _27914 = (min(max(_27855.x, _27855.y), 0.0) + _43090) - _43088;
                                    float _43198;
                                    SPIRV_CROSS_BRANCH
                                    if ((esia_v7.x & _2365) != 0u)
                                    {
                                        vec2 _25896 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                                        vec2 _25899 = _19094 - ((_41468.xy + _41468.zw) * 0.5);
                                        float _27962 = min(_25896.x, _25896.y);
                                        float _27965 = min((_25899.x > 0.0) ? ((_25899.y > 0.0) ? _18445.x : _18445.x) : ((_25899.y > 0.0) ? _18445.x : _18445.x), _27962);
                                        float _27971 = _27965 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                        float _43092;
                                        float _43093;
                                        if (_27971 > _27962)
                                        {
                                            float _27985 = esia_v5.y * clamp((_27962 - _27965) / max(0.60000002384185791015625 * _27965, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _43093 = _27985;
                                            _43092 = _27965 * (1.0 + (0.60000002384185791015625 * _27985));
                                        }
                                        else
                                        {
                                            _43093 = esia_v5.y;
                                            _43092 = _27971;
                                        }
                                        vec2 _28014 = (abs(_25899) - _25896) + vec2(_43092);
                                        vec2 _28016 = max(_28014, vec2(0.0));
                                        float _43094;
                                        SPIRV_CROSS_BRANCH
                                        if ((_28016.x > 0.0) && (_28016.y > 0.0))
                                        {
                                            float _43095;
                                            if ((_43093 > 0.001000000047497451305389404296875) && (_43092 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _28033 = 2.0 + (2.0 * _43093);
                                                vec2 _28038 = _28016 / vec2(max(_43092, 9.9999997473787516355514526367188e-05));
                                                _43095 = pow(pow(_28038.x, _28033) + pow(_28038.y, _28033), 1.0 / _28033) * _43092;
                                            }
                                            else
                                            {
                                                _43095 = length(_28016);
                                            }
                                            _43094 = _43095;
                                        }
                                        else
                                        {
                                            _43094 = max(_28016.x, _28016.y);
                                        }
                                        float _28073 = (min(max(_28014.x, _28014.y), 0.0) + _43094) - _43092;
                                        float _28078 = max(_18445.y, 9.9999997473787516355514526367188e-05);
                                        float _28087 = max(_28078 - abs(_27914 - _28073), 0.0) / _28078;
                                        _43198 = min(_27914, _28073) - (((_28087 * _28087) * _28078) * 0.25);
                                    }
                                    else
                                    {
                                        _43198 = _27914;
                                    }
                                    _43197 = _43198;
                                }
                                _43196 = _43197;
                            }
                            _43195 = _43196;
                        }
                        _43194 = _43195;
                    }
                    vec2 _19098 = esia_v0 - vec2(0.0, 0.5);
                    vec2 _28142 = (esia_v2.xy + esia_v2.zw) * 0.5;
                    vec2 _28151 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    float _43305;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _2279) != 0u)
                    {
                        float _28268 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
                        vec4 _28302 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_41462.xy, _41462.zw, (clamp(_19098, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
                        float _28326 = (_28302.w - 0.5) * esia_v3.y;
                        vec2 _28367 = abs(_19098 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _28268), vec2(0.0));
                        _43305 = max((-mix((max(min(_28302.x, _28302.y), min(max(_28302.x, _28302.y), _28302.z)) - 0.5) * esia_v3.x, _28326, clamp(abs(_28326) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _28268, length(max(_28367, vec2(0.0))) + min(max(_28367.x, _28367.y), 0.0)) - esia_v5.w;
                    }
                    else
                    {
                        float _43306;
                        if ((esia_v7.x & _2294) != 0u)
                        {
                            float _43253;
                            do
                            {
                                bool _28433 = esia_v3.w > 0.5;
                                float _28437 = floor((esia_v3.z + 1.0) * 0.5);
                                float _28441 = floor((_28437 + 127.0) * 0.0078125);
                                float _43246;
                                float _43248;
                                SPIRV_CROSS_BRANCH
                                if (_28433)
                                {
                                    uint _28617 = uint(esia_v3.z);
                                    uint _28620 = (_28617 + 31u) >> 5u;
                                    uint _28623 = esia_v1 + 1u;
                                    uint _28706 = max(uint(_292.gConv.z), 1u);
                                    ivec2 _28716 = ivec2(int((_28623 % _28706) * 24u), int(_28623 / _28706));
                                    ivec2 _43238;
                                    uint _43239;
                                    ivec2 _43299;
                                    _43299 = _28716;
                                    _43239 = 0u;
                                    _43238 = _28716;
                                    float _49514;
                                    uint _49515;
                                    ivec2 _49517;
                                    ivec2 _43302;
                                    uint _43237 = 0u;
                                    float _43300 = 3.0000000054977557577780399428115e+38;
                                    SPIRV_CROSS_LOOP
                                    for (; _43237 < _28620; _43300 = _49514, _43299 = _43302, _43239 = _49515, _43238 = _49517, _43237++)
                                    {
                                        vec4 _28744 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43299.x + 20, _43299.y, 0).xy, 0);
                                        vec2 _28757 = max(max(_28744.xy - _19098, _19098 - _28744.zw), vec2(0.0));
                                        float _28760 = dot(_28757, _28757);
                                        bool _28638 = _28760 < _43300;
                                        _49514 = _28638 ? _28760 : _43300;
                                        _49515 = _28638 ? _43237 : _43239;
                                        bvec2 _49516 = bvec2(_28638);
                                        _49517 = ivec2(_49516.x ? _43299.x : _43238.x, _49516.y ? _43299.y : _43238.y);
                                        int _28765 = _43299.x + 24;
                                        ivec2 _40565 = _43299;
                                        _40565.x = _28765;
                                        if (_28765 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                        {
                                            _43302 = ivec2(0, _43299.y + 1);
                                        }
                                        else
                                        {
                                            _43302 = _40565;
                                        }
                                    }
                                    uint _28820 = _43239 * 32u;
                                    uint _28824 = min((_28617 - _28820), 32u);
                                    vec4 _28951 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43238.x + 21, _43238.y, 0).xy, 0);
                                    vec4 _28979 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43238.x + 22, _43238.y, 0).xy, 0);
                                    float _43242;
                                    uint _43243;
                                    _43243 = 4294967295u;
                                    _43242 = 1000000015047466219876688855040.0;
                                    float _49019;
                                    uint _49020;
                                    SPIRV_CROSS_LOOP
                                    for (uint _43241 = 0u; (_43241 * 8u) < _28824; _43243 = _49020, _43242 = _49019, _43241++)
                                    {
                                        vec4 _29007 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43238.x + int(16u + _43241), _43238.y, 0).xy, 0);
                                        float _29025 = ((sqrt(_43242) * 1.000003814697265625) + 0.03125) + ((abs(_19098.x) + abs(_19098.y)) * 3.814697265625e-06);
                                        vec2 _29045 = max(max(_29007.xy - _19098, _19098 - _29007.zw), vec2(0.0));
                                        if (dot(_29045, _29045) > (_29025 * _29025))
                                        {
                                            _49020 = _43243;
                                            _49019 = _43242;
                                            continue;
                                        }
                                        bvec4 _28849 = bvec4(_43241 < 2u);
                                        vec4 _28850 = vec4(_28849.x ? _28951.x : _28979.x, _28849.y ? _28951.y : _28979.y, _28849.z ? _28951.z : _28979.z, _28849.w ? _28951.w : _28979.w);
                                        vec2 _28855 = _28850.zw;
                                        vec2 _28857 = _28850.xy;
                                        bvec2 _28858 = bvec2((_43241 & 1u) != 0u);
                                        uint _28864 = min(_28824, ((_43241 * 8u) + 8u));
                                        uint _28866 = _43241 * 8u;
                                        vec2 _43284;
                                        float _43285;
                                        uint _43286;
                                        _43286 = _43243;
                                        _43285 = _43242;
                                        _43284 = vec2(_28858.x ? _28855.x : _28857.x, _28858.y ? _28855.y : _28857.y);
                                        vec2 _49068;
                                        float _49069;
                                        uint _49070;
                                        SPIRV_CROSS_LOOP
                                        for (uint _43283 = _28866; _43283 < _28864; _43286 = _49070, _43285 = _49069, _43284 = _49068, _43283 += 2u)
                                        {
                                            vec4 _29076 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43238.x + int(_43283 >> 1u), _43238.y, 0).xy, 0);
                                            uint _28879 = _28820 + _43283;
                                            vec2 _28882 = _29076.xy;
                                            vec2 _29084 = _43284 - _28882;
                                            vec2 _29087 = _19098 - _28882;
                                            vec2 _29100 = _29087 - (_29084 * clamp(dot(_29087, _29084) / max(dot(_29084, _29084), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _29103 = dot(_29100, _29100);
                                            bool _29114 = (_29103 < _43285) || ((_29103 == _43285) && (_28879 < _43286));
                                            uint _49518 = _29114 ? _28879 : _43286;
                                            float _49519 = _29114 ? _29103 : _43285;
                                            vec2 _28892 = _29076.xy;
                                            if ((_43283 + 1u) < _28864)
                                            {
                                                uint _28901 = (_28820 + _43283) + 1u;
                                                vec2 _28904 = _29076.zw;
                                                vec2 _29127 = _28892 - _28904;
                                                vec2 _29130 = _19098 - _28904;
                                                vec2 _29143 = _29130 - (_29127 * clamp(dot(_29130, _29127) / max(dot(_29127, _29127), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _29146 = dot(_29143, _29143);
                                                bool _29157 = (_29146 < _49519) || ((_29146 == _49519) && (_28901 < _49518));
                                                _49070 = _29157 ? _28901 : _49518;
                                                _49069 = _29157 ? _29146 : _49519;
                                                _49068 = _29076.zw;
                                            }
                                            else
                                            {
                                                _49070 = _49518;
                                                _49069 = _49519;
                                                _49068 = _28892;
                                            }
                                        }
                                        _49020 = _43286;
                                        _49019 = _43285;
                                    }
                                    uint _28663 = esia_v1 + 1u;
                                    uint _29169 = max(uint(_292.gConv.z), 1u);
                                    float _43245;
                                    ivec2 _43256;
                                    _43256 = ivec2(int((_28663 % _29169) * 24u), int(_28663 / _29169));
                                    _43245 = _43242;
                                    ivec2 _43263;
                                    float _49022;
                                    uint _49052;
                                    SPIRV_CROSS_LOOP
                                    for (uint _43244 = 0u, _43257 = _43243; _43244 < _28620; _43257 = _49052, _43256 = _43263, _43245 = _49022, _43244++)
                                    {
                                        vec4 _29207 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43256.x + 20, _43256.y, 0).xy, 0);
                                        float _29225 = ((sqrt(_43245) * 1.000003814697265625) + 0.03125) + ((abs(_19098.x) + abs(_19098.y)) * 3.814697265625e-06);
                                        vec2 _29245 = max(max(_29207.xy - _19098, _19098 - _29207.zw), vec2(0.0));
                                        if ((_43244 != _43239) && (!(dot(_29245, _29245) > (_29225 * _29225))))
                                        {
                                            uint _29286 = _43244 * 32u;
                                            uint _29290 = min((_28617 - _29286), 32u);
                                            vec4 _29417 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43256.x + 21, _43256.y, 0).xy, 0);
                                            vec4 _29445 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43256.x + 22, _43256.y, 0).xy, 0);
                                            float _43259;
                                            uint _43260;
                                            _43260 = _43257;
                                            _43259 = _43245;
                                            float _49053;
                                            uint _49054;
                                            SPIRV_CROSS_LOOP
                                            for (uint _43258 = 0u; (_43258 * 8u) < _29290; _43260 = _49054, _43259 = _49053, _43258++)
                                            {
                                                vec4 _29473 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43256.x + int(16u + _43258), _43256.y, 0).xy, 0);
                                                float _29491 = ((sqrt(_43259) * 1.000003814697265625) + 0.03125) + ((abs(_19098.x) + abs(_19098.y)) * 3.814697265625e-06);
                                                vec2 _29511 = max(max(_29473.xy - _19098, _19098 - _29473.zw), vec2(0.0));
                                                if (dot(_29511, _29511) > (_29491 * _29491))
                                                {
                                                    _49054 = _43260;
                                                    _49053 = _43259;
                                                    continue;
                                                }
                                                bvec4 _29315 = bvec4(_43258 < 2u);
                                                vec4 _29316 = vec4(_29315.x ? _29417.x : _29445.x, _29315.y ? _29417.y : _29445.y, _29315.z ? _29417.z : _29445.z, _29315.w ? _29417.w : _29445.w);
                                                vec2 _29321 = _29316.zw;
                                                vec2 _29323 = _29316.xy;
                                                bvec2 _29324 = bvec2((_43258 & 1u) != 0u);
                                                uint _29330 = min(_29290, ((_43258 * 8u) + 8u));
                                                uint _29332 = _43258 * 8u;
                                                vec2 _43268;
                                                float _43269;
                                                uint _43270;
                                                _43270 = _43260;
                                                _43269 = _43259;
                                                _43268 = vec2(_29324.x ? _29321.x : _29323.x, _29324.y ? _29321.y : _29323.y);
                                                vec2 _49065;
                                                float _49066;
                                                uint _49067;
                                                SPIRV_CROSS_LOOP
                                                for (uint _43267 = _29332; _43267 < _29330; _43270 = _49067, _43269 = _49066, _43268 = _49065, _43267 += 2u)
                                                {
                                                    vec4 _29542 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_43256.x + int(_43267 >> 1u), _43256.y, 0).xy, 0);
                                                    uint _29345 = _29286 + _43267;
                                                    vec2 _29348 = _29542.xy;
                                                    vec2 _29550 = _43268 - _29348;
                                                    vec2 _29553 = _19098 - _29348;
                                                    vec2 _29566 = _29553 - (_29550 * clamp(dot(_29553, _29550) / max(dot(_29550, _29550), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                    float _29569 = dot(_29566, _29566);
                                                    bool _29580 = (_29569 < _43269) || ((_29569 == _43269) && (_29345 < _43270));
                                                    uint _49522 = _29580 ? _29345 : _43270;
                                                    float _49523 = _29580 ? _29569 : _43269;
                                                    vec2 _29358 = _29542.xy;
                                                    if ((_43267 + 1u) < _29330)
                                                    {
                                                        uint _29367 = (_29286 + _43267) + 1u;
                                                        vec2 _29370 = _29542.zw;
                                                        vec2 _29593 = _29358 - _29370;
                                                        vec2 _29596 = _19098 - _29370;
                                                        vec2 _29609 = _29596 - (_29593 * clamp(dot(_29596, _29593) / max(dot(_29593, _29593), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                        float _29612 = dot(_29609, _29609);
                                                        bool _29623 = (_29612 < _49523) || ((_29612 == _49523) && (_29367 < _49522));
                                                        _49067 = _29623 ? _29367 : _49522;
                                                        _49066 = _29623 ? _29612 : _49523;
                                                        _49065 = _29542.zw;
                                                    }
                                                    else
                                                    {
                                                        _49067 = _49522;
                                                        _49066 = _49523;
                                                        _49065 = _29358;
                                                    }
                                                }
                                                _49054 = _43270;
                                                _49053 = _43269;
                                            }
                                            _49052 = _43260;
                                            _49022 = _43259;
                                        }
                                        else
                                        {
                                            _49052 = _43257;
                                            _49022 = _43245;
                                        }
                                        int _29633 = _43256.x + 24;
                                        ivec2 _40612 = _43256;
                                        _40612.x = _29633;
                                        if (_29633 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                                        {
                                            _43263 = ivec2(0, _43256.y + 1);
                                        }
                                        else
                                        {
                                            _43263 = _40612;
                                        }
                                    }
                                    _43248 = 1.0;
                                    _43246 = _43245;
                                }
                                else
                                {
                                    float _28455 = floor((esia_v3.z - 1.0) * 0.5);
                                    float _29658 = floor(_28455 * 0.0625);
                                    uint _29663 = (esia_v1 + 1u) + uint(_29658);
                                    uint _29676 = max(uint(_292.gConv.z), 1u);
                                    vec4 _29710 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_29663 % _29676) * 24u) + uint(_28455 - (_29658 * 16.0))), int(_29663 / _29676), 0).xy, 0);
                                    vec2 _28467 = _29710.xy;
                                    vec2 _28469 = _29710.zw;
                                    bvec2 _28470 = bvec2(((esia_v3.z - 1.0) - (_28455 * 2.0)) < 0.5);
                                    vec2 _43216;
                                    if (_28433)
                                    {
                                        float _29718 = floor(0.0);
                                        uint _29723 = (esia_v1 + 1u) + uint(_29718);
                                        uint _29736 = max(uint(_292.gConv.z), 1u);
                                        _43216 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_29723 % _29736) * 24u) + uint(_29718 * (-16.0))), int(_29723 / _29736), 0).xy, 0).xy;
                                    }
                                    else
                                    {
                                        _43216 = vec2(_28470.x ? _28467.x : _28469.x, _28470.y ? _28467.y : _28469.y);
                                    }
                                    vec2 _43215;
                                    float _43218;
                                    float _43221;
                                    _43221 = 1.0;
                                    _43218 = 1000000015047466219876688855040.0;
                                    _43215 = _43216;
                                    vec2 _43214;
                                    float _43217;
                                    float _43220;
                                    SPIRV_CROSS_LOOP
                                    for (float _43211 = 0.0; _43211 < 16.0; _43221 = _43220, _43218 = _43217, _43215 = _43214, _43211 += 1.0)
                                    {
                                        if (_43211 >= _28441)
                                        {
                                            break;
                                        }
                                        _43220 = _43221;
                                        _43217 = _43218;
                                        _43214 = _43215;
                                        vec2 _49008;
                                        float _49010;
                                        float _49012;
                                        SPIRV_CROSS_LOOP
                                        for (float _43212 = 0.0; _43212 < 128.0; _43220 = _49012, _43217 = _49010, _43214 = _49008, _43212 += 1.0)
                                        {
                                            float _28497 = (_43211 * 128.0) + _43212;
                                            if (_28497 >= _28437)
                                            {
                                                break;
                                            }
                                            float _29778 = floor(_28497 * 0.0625);
                                            uint _29783 = (esia_v1 + 1u) + uint(_29778);
                                            uint _29796 = max(uint(_292.gConv.z), 1u);
                                            vec4 _29830 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_29783 % _29796) * 24u) + uint(_28497 - (_29778 * 16.0))), int(_29783 / _29796), 0).xy, 0);
                                            vec2 _28509 = _29830.xy;
                                            vec2 _29841 = _43214 - _28509;
                                            vec2 _29844 = _19098 - _28509;
                                            vec2 _29857 = _29844 - (_29841 * clamp(dot(_29844, _29841) / max(dot(_29841, _29841), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _29860 = dot(_29857, _29857);
                                            float _49526 = (_29860 < _43217) ? _29860 : _43217;
                                            bool _29872 = _19098.y >= _29830.y;
                                            bool _29877 = _19098.y < _43214.y;
                                            bool _29888 = (_29841.x * _29844.y) > (_29841.y * _29844.x);
                                            float _43226;
                                            if (((_29872 && _29877) && _29888) || (((!_29872) && (!_29877)) && (!_29888)))
                                            {
                                                _43226 = -_43220;
                                            }
                                            else
                                            {
                                                _43226 = _43220;
                                            }
                                            vec2 _28519 = _29830.xy;
                                            if (((2.0 * _28497) + 1.0) < esia_v3.z)
                                            {
                                                vec2 _28528 = _29830.zw;
                                                vec2 _29917 = _28519 - _28528;
                                                vec2 _29920 = _19098 - _28528;
                                                vec2 _29933 = _29920 - (_29917 * clamp(dot(_29920, _29917) / max(dot(_29917, _29917), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                                float _29936 = dot(_29933, _29933);
                                                bool _29948 = _19098.y >= _29830.w;
                                                bool _29953 = _19098.y < _29830.y;
                                                bool _29964 = (_29917.x * _29920.y) > (_29917.y * _29920.x);
                                                float _43230;
                                                if (((_29948 && _29953) && _29964) || (((!_29948) && (!_29953)) && (!_29964)))
                                                {
                                                    _43230 = -_43226;
                                                }
                                                else
                                                {
                                                    _43230 = _43226;
                                                }
                                                _49012 = _43230;
                                                _49010 = (_29936 < _49526) ? _29936 : _49526;
                                                _49008 = _29830.zw;
                                            }
                                            else
                                            {
                                                _49012 = _43226;
                                                _49010 = _49526;
                                                _49008 = _28519;
                                            }
                                        }
                                    }
                                    _43248 = _43221;
                                    _43246 = _43218;
                                }
                                float _28550 = sqrt(_43246);
                                if (_28433)
                                {
                                    _43253 = _28550 - esia_v3.x;
                                    break;
                                }
                                _43253 = (_43248 * _28550) - esia_v3.x;
                                break;
                            } while(false);
                            _43306 = _43253;
                        }
                        else
                        {
                            float _43307;
                            if (esia_v7.z == 1u)
                            {
                                vec2 _28176 = _19098 - _28142;
                                float _43210;
                                do
                                {
                                    if (esia_v5.w >= 6.282185077667236328125)
                                    {
                                        _43210 = abs(length(_28176) - esia_v3.x) - esia_v3.y;
                                        break;
                                    }
                                    float _30007 = esia_v5.z + (esia_v5.w * 0.5);
                                    float _30009 = cos(_30007);
                                    float _30011 = sin(_30007);
                                    float _30020 = dot(_28176, vec2(-_30011, _30009));
                                    float _30023 = dot(_28176, vec2(_30009, _30011));
                                    vec2 _30024 = vec2(_30020, _30023);
                                    float _30027 = abs(_30020);
                                    _30024.x = _30027;
                                    float _30030 = esia_v5.w * 0.5;
                                    float _30032 = sin(_30030);
                                    float _30034 = cos(_30030);
                                    _43210 = (((_30034 * _30027) > (_30032 * _30023)) ? length(_30024 - (vec2(_30032, _30034) * esia_v3.x)) : abs(length(_30024) - esia_v3.x)) - esia_v3.y;
                                    break;
                                } while(false);
                                _43307 = _43210;
                            }
                            else
                            {
                                float _43308;
                                if (esia_v7.z == 2u)
                                {
                                    vec2 _30070 = _19098 - _41468.xy;
                                    vec2 _30073 = _41468.zw - _41468.xy;
                                    _43308 = length(_30070 - (_30073 * clamp(dot(_30070, _30073) / max(dot(_30073, _30073), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                                }
                                else
                                {
                                    vec2 _28203 = _19098 - _28142;
                                    float _30138 = min(_28151.x, _28151.y);
                                    float _30141 = min((_28203.x > 0.0) ? ((_28203.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_28203.y > 0.0) ? esia_v3.w : esia_v3.x), _30138);
                                    float _30147 = _30141 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                    float _43199;
                                    float _43200;
                                    if (_30147 > _30138)
                                    {
                                        float _30161 = esia_v5.y * clamp((_30138 - _30141) / max(0.60000002384185791015625 * _30141, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                        _43200 = _30161;
                                        _43199 = _30141 * (1.0 + (0.60000002384185791015625 * _30161));
                                    }
                                    else
                                    {
                                        _43200 = esia_v5.y;
                                        _43199 = _30147;
                                    }
                                    vec2 _30190 = (abs(_28203) - _28151) + vec2(_43199);
                                    vec2 _30192 = max(_30190, vec2(0.0));
                                    float _43201;
                                    SPIRV_CROSS_BRANCH
                                    if ((_30192.x > 0.0) && (_30192.y > 0.0))
                                    {
                                        float _43202;
                                        if ((_43200 > 0.001000000047497451305389404296875) && (_43199 > 9.9999997473787516355514526367188e-05))
                                        {
                                            float _30209 = 2.0 + (2.0 * _43200);
                                            vec2 _30214 = _30192 / vec2(max(_43199, 9.9999997473787516355514526367188e-05));
                                            _43202 = pow(pow(_30214.x, _30209) + pow(_30214.y, _30209), 1.0 / _30209) * _43199;
                                        }
                                        else
                                        {
                                            _43202 = length(_30192);
                                        }
                                        _43201 = _43202;
                                    }
                                    else
                                    {
                                        _43201 = max(_30192.x, _30192.y);
                                    }
                                    float _30249 = (min(max(_30190.x, _30190.y), 0.0) + _43201) - _43199;
                                    float _43309;
                                    SPIRV_CROSS_BRANCH
                                    if ((esia_v7.x & _2365) != 0u)
                                    {
                                        vec2 _28231 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                                        vec2 _28234 = _19098 - ((_41468.xy + _41468.zw) * 0.5);
                                        float _30297 = min(_28231.x, _28231.y);
                                        float _30300 = min((_28234.x > 0.0) ? ((_28234.y > 0.0) ? _18445.x : _18445.x) : ((_28234.y > 0.0) ? _18445.x : _18445.x), _30297);
                                        float _30306 = _30300 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                                        float _43203;
                                        float _43204;
                                        if (_30306 > _30297)
                                        {
                                            float _30320 = esia_v5.y * clamp((_30297 - _30300) / max(0.60000002384185791015625 * _30300, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                            _43204 = _30320;
                                            _43203 = _30300 * (1.0 + (0.60000002384185791015625 * _30320));
                                        }
                                        else
                                        {
                                            _43204 = esia_v5.y;
                                            _43203 = _30306;
                                        }
                                        vec2 _30349 = (abs(_28234) - _28231) + vec2(_43203);
                                        vec2 _30351 = max(_30349, vec2(0.0));
                                        float _43205;
                                        SPIRV_CROSS_BRANCH
                                        if ((_30351.x > 0.0) && (_30351.y > 0.0))
                                        {
                                            float _43206;
                                            if ((_43204 > 0.001000000047497451305389404296875) && (_43203 > 9.9999997473787516355514526367188e-05))
                                            {
                                                float _30368 = 2.0 + (2.0 * _43204);
                                                vec2 _30373 = _30351 / vec2(max(_43203, 9.9999997473787516355514526367188e-05));
                                                _43206 = pow(pow(_30373.x, _30368) + pow(_30373.y, _30368), 1.0 / _30368) * _43203;
                                            }
                                            else
                                            {
                                                _43206 = length(_30351);
                                            }
                                            _43205 = _43206;
                                        }
                                        else
                                        {
                                            _43205 = max(_30351.x, _30351.y);
                                        }
                                        float _30408 = (min(max(_30349.x, _30349.y), 0.0) + _43205) - _43203;
                                        float _30413 = max(_18445.y, 9.9999997473787516355514526367188e-05);
                                        float _30422 = max(_30413 - abs(_30249 - _30408), 0.0) / _30413;
                                        _43309 = min(_30249, _30408) - (((_30422 * _30422) * _30413) * 0.25);
                                    }
                                    else
                                    {
                                        _43309 = _30249;
                                    }
                                    _43308 = _43309;
                                }
                                _43307 = _43308;
                            }
                            _43306 = _43307;
                        }
                        _43305 = _43306;
                    }
                    _43542 = vec2(_42972 - _43083, _43194 - _43305);
                }
                float _19107 = length(_43542);
                vec2 _19113 = _43542 / vec2(_19107);
                bvec2 _19114 = bvec2(_19107 > 9.9999997473787516355514526367188e-06);
                _43598 = vec2(_19114.x ? _19113.x : vec2(0.0, -1.0).x, _19114.y ? _19113.y : vec2(0.0, -1.0).y);
                break;
            } while(false);
            _44176 = normalize(vec3(_43598 * min(_18544 / max(_18550, 0.001000000047497451305389404296875), 8.0), 1.0));
            _43599 = _43598;
        }
        else
        {
            _44176 = vec3(0.0, 0.0, 1.0);
            _43599 = vec2(0.0, -1.0);
        }
        vec2 _18576 = ((-_43599) * _18518) * (1.0 - _18550);
        float _18578 = _18445.z;
        vec2 _43600;
        SPIRV_CROSS_BRANCH
        if (_18578 > 0.0)
        {
            _43600 = (((esia_v2.xy + esia_v2.zw) * 0.5) - esia_v0) * (_18578 / (1.0 + _18578));
        }
        else
        {
            _43600 = vec2(0.0);
        }
        vec2 _18602 = _41461 * _292.gTarget.zw;
        vec2 _18609 = _292.gDisplay.zw * _292.gTarget.zw;
        vec2 _18614 = (_18576 + _43600) * _18609;
        vec2 _18617 = _18576 * _18609;
        vec3 _44058;
        float _44076;
        vec3 _44279;
        if (_292.gTime.z > 0.5)
        {
            vec2 _18624 = _18602 + _18614;
            float _30447 = clamp(log2(max(_18516, 1.0)) - 1.0, 0.0, 5.0);
            int _30450 = int(floor(_30447));
            float _30454 = _30447 - float(_30450);
            bool _30459 = (_30454 > 0.0199999995529651641845703125) && (_30450 < 5);
            vec3 _43671;
            _43671 = vec3(0.0);
            vec3 _30488;
            SPIRV_CROSS_LOOP
            for (int _43601 = 0; _43601 < 2; _43671 = _30488, _43601++)
            {
                if ((_43601 > 0) && (!_30459))
                {
                    break;
                }
                int _30474 = _30450 + _43601;
                int _30512 = clamp(_30474, 1, 5);
                vec2 _30581 = (_18624 * _292.gLevel[_30512].xy) - vec2(0.5);
                vec2 _30583 = floor(_30581);
                vec2 _30586 = _30581 - _30583;
                vec2 _30589 = _30586 * _30586;
                vec2 _30592 = _30589 * _30586;
                vec2 _30611 = (((_30592 * 3.0) - (_30589 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                vec2 _30624 = _30592 * 0.16666667163372039794921875;
                vec2 _30627 = (((((-_30592) + (_30589 * 3.0)) - (_30586 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _30611;
                vec2 _30631 = (((((_30592 * (-3.0)) + (_30589 * 3.0)) + (_30586 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _30624;
                vec2 _30643 = ((_30583 - vec2(0.5)) + (_30611 / _30627)) * _292.gLevel[_30512].zw;
                vec2 _30655 = ((_30583 + vec2(1.5)) + (_30624 / _30631)) * _292.gLevel[_30512].zw;
                vec4 _43638;
                SPIRV_CROSS_BRANCH
                if (_30474 <= 0)
                {
                    vec2 _43637;
                    if (_292.gConv.x > 0.5)
                    {
                        vec2 _40692 = _18624;
                        _40692.y = 1.0 - _18624.y;
                        _43637 = _40692;
                    }
                    else
                    {
                        _43637 = _18624;
                    }
                    _43638 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _43637, 0.0);
                }
                else
                {
                    vec4 _43639;
                    if (_30474 == 1)
                    {
                        vec2 _43630;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _40697 = _30643;
                            _40697.y = 1.0 - _30643.y;
                            _43630 = _40697;
                        }
                        else
                        {
                            _43630 = _30643;
                        }
                        float _30700 = _30643.y;
                        vec2 _30701 = vec2(_30655.x, _30700);
                        vec2 _43631;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _40704 = _30701;
                            _40704.y = 1.0 - _30700;
                            _43631 = _40704;
                        }
                        else
                        {
                            _43631 = _30701;
                        }
                        float _30718 = _30655.y;
                        vec2 _30719 = vec2(_30643.x, _30718);
                        vec2 _43633;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _40711 = _30719;
                            _40711.y = 1.0 - _30718;
                            _43633 = _40711;
                        }
                        else
                        {
                            _43633 = _30719;
                        }
                        vec2 _43635;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _40718 = _30655;
                            _40718.y = 1.0 - _30655.y;
                            _43635 = _40718;
                        }
                        else
                        {
                            _43635 = _30655;
                        }
                        _43639 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43630, 0.0) * (_30627.x * _30627.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43631, 0.0) * (_30631.x * _30627.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43633, 0.0) * (_30627.x * _30631.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43635, 0.0) * (_30631.x * _30631.y));
                    }
                    else
                    {
                        vec4 _43640;
                        if (_30474 == 2)
                        {
                            vec2 _43623;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40725 = _30643;
                                _40725.y = 1.0 - _30643.y;
                                _43623 = _40725;
                            }
                            else
                            {
                                _43623 = _30643;
                            }
                            float _30830 = _30643.y;
                            vec2 _30831 = vec2(_30655.x, _30830);
                            vec2 _43624;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40732 = _30831;
                                _40732.y = 1.0 - _30830;
                                _43624 = _40732;
                            }
                            else
                            {
                                _43624 = _30831;
                            }
                            float _30848 = _30655.y;
                            vec2 _30849 = vec2(_30643.x, _30848);
                            vec2 _43626;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40739 = _30849;
                                _40739.y = 1.0 - _30848;
                                _43626 = _40739;
                            }
                            else
                            {
                                _43626 = _30849;
                            }
                            vec2 _43628;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40746 = _30655;
                                _40746.y = 1.0 - _30655.y;
                                _43628 = _40746;
                            }
                            else
                            {
                                _43628 = _30655;
                            }
                            _43640 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43623, 0.0) * (_30627.x * _30627.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43624, 0.0) * (_30631.x * _30627.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43626, 0.0) * (_30627.x * _30631.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43628, 0.0) * (_30631.x * _30631.y));
                        }
                        else
                        {
                            vec4 _43641;
                            if (_30474 == 3)
                            {
                                vec2 _43616;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _40753 = _30643;
                                    _40753.y = 1.0 - _30643.y;
                                    _43616 = _40753;
                                }
                                else
                                {
                                    _43616 = _30643;
                                }
                                float _30960 = _30643.y;
                                vec2 _30961 = vec2(_30655.x, _30960);
                                vec2 _43617;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _40760 = _30961;
                                    _40760.y = 1.0 - _30960;
                                    _43617 = _40760;
                                }
                                else
                                {
                                    _43617 = _30961;
                                }
                                float _30978 = _30655.y;
                                vec2 _30979 = vec2(_30643.x, _30978);
                                vec2 _43619;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _40767 = _30979;
                                    _40767.y = 1.0 - _30978;
                                    _43619 = _40767;
                                }
                                else
                                {
                                    _43619 = _30979;
                                }
                                vec2 _43621;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _40774 = _30655;
                                    _40774.y = 1.0 - _30655.y;
                                    _43621 = _40774;
                                }
                                else
                                {
                                    _43621 = _30655;
                                }
                                _43641 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43616, 0.0) * (_30627.x * _30627.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43617, 0.0) * (_30631.x * _30627.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43619, 0.0) * (_30627.x * _30631.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43621, 0.0) * (_30631.x * _30631.y));
                            }
                            else
                            {
                                vec4 _43642;
                                if (_30474 == 4)
                                {
                                    vec2 _43609;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40781 = _30643;
                                        _40781.y = 1.0 - _30643.y;
                                        _43609 = _40781;
                                    }
                                    else
                                    {
                                        _43609 = _30643;
                                    }
                                    float _31090 = _30643.y;
                                    vec2 _31091 = vec2(_30655.x, _31090);
                                    vec2 _43610;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40788 = _31091;
                                        _40788.y = 1.0 - _31090;
                                        _43610 = _40788;
                                    }
                                    else
                                    {
                                        _43610 = _31091;
                                    }
                                    float _31108 = _30655.y;
                                    vec2 _31109 = vec2(_30643.x, _31108);
                                    vec2 _43612;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40795 = _31109;
                                        _40795.y = 1.0 - _31108;
                                        _43612 = _40795;
                                    }
                                    else
                                    {
                                        _43612 = _31109;
                                    }
                                    vec2 _43614;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40802 = _30655;
                                        _40802.y = 1.0 - _30655.y;
                                        _43614 = _40802;
                                    }
                                    else
                                    {
                                        _43614 = _30655;
                                    }
                                    _43642 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43609, 0.0) * (_30627.x * _30627.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43610, 0.0) * (_30631.x * _30627.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43612, 0.0) * (_30627.x * _30631.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43614, 0.0) * (_30631.x * _30631.y));
                                }
                                else
                                {
                                    vec2 _43602;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40809 = _30643;
                                        _40809.y = 1.0 - _30643.y;
                                        _43602 = _40809;
                                    }
                                    else
                                    {
                                        _43602 = _30643;
                                    }
                                    float _31220 = _30643.y;
                                    vec2 _31221 = vec2(_30655.x, _31220);
                                    vec2 _43603;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40816 = _31221;
                                        _40816.y = 1.0 - _31220;
                                        _43603 = _40816;
                                    }
                                    else
                                    {
                                        _43603 = _31221;
                                    }
                                    float _31238 = _30655.y;
                                    vec2 _31239 = vec2(_30643.x, _31238);
                                    vec2 _43605;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40823 = _31239;
                                        _40823.y = 1.0 - _31238;
                                        _43605 = _40823;
                                    }
                                    else
                                    {
                                        _43605 = _31239;
                                    }
                                    vec2 _43607;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40830 = _30655;
                                        _40830.y = 1.0 - _30655.y;
                                        _43607 = _40830;
                                    }
                                    else
                                    {
                                        _43607 = _30655;
                                    }
                                    _43642 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43602, 0.0) * (_30627.x * _30627.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43603, 0.0) * (_30631.x * _30627.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43605, 0.0) * (_30627.x * _30631.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43607, 0.0) * (_30631.x * _30631.y));
                                }
                                _43641 = _43642;
                            }
                            _43640 = _43641;
                        }
                        _43639 = _43640;
                    }
                    _43638 = _43639;
                }
                _30488 = _43671 + (_43638.xyz * (_30459 ? ((_43601 == 0) ? (1.0 - _30454) : _30454) : 1.0));
            }
            vec3 _44061;
            SPIRV_CROSS_BRANCH
            if (((_18537 > 0.001000000047497451305389404296875) && (_18544 > 0.0)) && ((((_18518 * 0.300000011920928955078125) * _18537) * _292.gDisplay.z) > (_18516 * 0.3499999940395355224609375)))
            {
                float _18644 = 0.300000011920928955078125 * _18537;
                vec2 _18651 = (_18602 + _18614) - (_18617 * _18644);
                float _31335 = clamp(log2(max(_18516, 1.0)) - 1.0, 0.0, 5.0);
                int _31338 = int(floor(_31335));
                float _31342 = _31335 - float(_31338);
                bool _31347 = (_31342 > 0.0199999995529651641845703125) && (_31338 < 5);
                vec3 _43767;
                _43767 = vec3(0.0);
                vec3 _31376;
                SPIRV_CROSS_LOOP
                for (int _43697 = 0; _43697 < 2; _43767 = _31376, _43697++)
                {
                    if ((_43697 > 0) && (!_31347))
                    {
                        break;
                    }
                    int _31362 = _31338 + _43697;
                    int _31400 = clamp(_31362, 1, 5);
                    vec2 _31469 = (_18651 * _292.gLevel[_31400].xy) - vec2(0.5);
                    vec2 _31471 = floor(_31469);
                    vec2 _31474 = _31469 - _31471;
                    vec2 _31477 = _31474 * _31474;
                    vec2 _31480 = _31477 * _31474;
                    vec2 _31499 = (((_31480 * 3.0) - (_31477 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                    vec2 _31512 = _31480 * 0.16666667163372039794921875;
                    vec2 _31515 = (((((-_31480) + (_31477 * 3.0)) - (_31474 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _31499;
                    vec2 _31519 = (((((_31480 * (-3.0)) + (_31477 * 3.0)) + (_31474 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _31512;
                    vec2 _31531 = ((_31471 - vec2(0.5)) + (_31499 / _31515)) * _292.gLevel[_31400].zw;
                    vec2 _31543 = ((_31471 + vec2(1.5)) + (_31512 / _31519)) * _292.gLevel[_31400].zw;
                    vec4 _43734;
                    SPIRV_CROSS_BRANCH
                    if (_31362 <= 0)
                    {
                        vec2 _43733;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _40835 = _18651;
                            _40835.y = 1.0 - _18651.y;
                            _43733 = _40835;
                        }
                        else
                        {
                            _43733 = _18651;
                        }
                        _43734 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _43733, 0.0);
                    }
                    else
                    {
                        vec4 _43735;
                        if (_31362 == 1)
                        {
                            vec2 _43726;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40840 = _31531;
                                _40840.y = 1.0 - _31531.y;
                                _43726 = _40840;
                            }
                            else
                            {
                                _43726 = _31531;
                            }
                            float _31588 = _31531.y;
                            vec2 _31589 = vec2(_31543.x, _31588);
                            vec2 _43727;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40847 = _31589;
                                _40847.y = 1.0 - _31588;
                                _43727 = _40847;
                            }
                            else
                            {
                                _43727 = _31589;
                            }
                            float _31606 = _31543.y;
                            vec2 _31607 = vec2(_31531.x, _31606);
                            vec2 _43729;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40854 = _31607;
                                _40854.y = 1.0 - _31606;
                                _43729 = _40854;
                            }
                            else
                            {
                                _43729 = _31607;
                            }
                            vec2 _43731;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40861 = _31543;
                                _40861.y = 1.0 - _31543.y;
                                _43731 = _40861;
                            }
                            else
                            {
                                _43731 = _31543;
                            }
                            _43735 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43726, 0.0) * (_31515.x * _31515.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43727, 0.0) * (_31519.x * _31515.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43729, 0.0) * (_31515.x * _31519.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43731, 0.0) * (_31519.x * _31519.y));
                        }
                        else
                        {
                            vec4 _43736;
                            if (_31362 == 2)
                            {
                                vec2 _43719;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _40868 = _31531;
                                    _40868.y = 1.0 - _31531.y;
                                    _43719 = _40868;
                                }
                                else
                                {
                                    _43719 = _31531;
                                }
                                float _31718 = _31531.y;
                                vec2 _31719 = vec2(_31543.x, _31718);
                                vec2 _43720;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _40875 = _31719;
                                    _40875.y = 1.0 - _31718;
                                    _43720 = _40875;
                                }
                                else
                                {
                                    _43720 = _31719;
                                }
                                float _31736 = _31543.y;
                                vec2 _31737 = vec2(_31531.x, _31736);
                                vec2 _43722;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _40882 = _31737;
                                    _40882.y = 1.0 - _31736;
                                    _43722 = _40882;
                                }
                                else
                                {
                                    _43722 = _31737;
                                }
                                vec2 _43724;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _40889 = _31543;
                                    _40889.y = 1.0 - _31543.y;
                                    _43724 = _40889;
                                }
                                else
                                {
                                    _43724 = _31543;
                                }
                                _43736 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43719, 0.0) * (_31515.x * _31515.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43720, 0.0) * (_31519.x * _31515.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43722, 0.0) * (_31515.x * _31519.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43724, 0.0) * (_31519.x * _31519.y));
                            }
                            else
                            {
                                vec4 _43737;
                                if (_31362 == 3)
                                {
                                    vec2 _43712;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40896 = _31531;
                                        _40896.y = 1.0 - _31531.y;
                                        _43712 = _40896;
                                    }
                                    else
                                    {
                                        _43712 = _31531;
                                    }
                                    float _31848 = _31531.y;
                                    vec2 _31849 = vec2(_31543.x, _31848);
                                    vec2 _43713;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40903 = _31849;
                                        _40903.y = 1.0 - _31848;
                                        _43713 = _40903;
                                    }
                                    else
                                    {
                                        _43713 = _31849;
                                    }
                                    float _31866 = _31543.y;
                                    vec2 _31867 = vec2(_31531.x, _31866);
                                    vec2 _43715;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40910 = _31867;
                                        _40910.y = 1.0 - _31866;
                                        _43715 = _40910;
                                    }
                                    else
                                    {
                                        _43715 = _31867;
                                    }
                                    vec2 _43717;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _40917 = _31543;
                                        _40917.y = 1.0 - _31543.y;
                                        _43717 = _40917;
                                    }
                                    else
                                    {
                                        _43717 = _31543;
                                    }
                                    _43737 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43712, 0.0) * (_31515.x * _31515.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43713, 0.0) * (_31519.x * _31515.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43715, 0.0) * (_31515.x * _31519.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43717, 0.0) * (_31519.x * _31519.y));
                                }
                                else
                                {
                                    vec4 _43738;
                                    if (_31362 == 4)
                                    {
                                        vec2 _43705;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _40924 = _31531;
                                            _40924.y = 1.0 - _31531.y;
                                            _43705 = _40924;
                                        }
                                        else
                                        {
                                            _43705 = _31531;
                                        }
                                        float _31978 = _31531.y;
                                        vec2 _31979 = vec2(_31543.x, _31978);
                                        vec2 _43706;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _40931 = _31979;
                                            _40931.y = 1.0 - _31978;
                                            _43706 = _40931;
                                        }
                                        else
                                        {
                                            _43706 = _31979;
                                        }
                                        float _31996 = _31543.y;
                                        vec2 _31997 = vec2(_31531.x, _31996);
                                        vec2 _43708;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _40938 = _31997;
                                            _40938.y = 1.0 - _31996;
                                            _43708 = _40938;
                                        }
                                        else
                                        {
                                            _43708 = _31997;
                                        }
                                        vec2 _43710;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _40945 = _31543;
                                            _40945.y = 1.0 - _31543.y;
                                            _43710 = _40945;
                                        }
                                        else
                                        {
                                            _43710 = _31543;
                                        }
                                        _43738 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43705, 0.0) * (_31515.x * _31515.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43706, 0.0) * (_31519.x * _31515.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43708, 0.0) * (_31515.x * _31519.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43710, 0.0) * (_31519.x * _31519.y));
                                    }
                                    else
                                    {
                                        vec2 _43698;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _40952 = _31531;
                                            _40952.y = 1.0 - _31531.y;
                                            _43698 = _40952;
                                        }
                                        else
                                        {
                                            _43698 = _31531;
                                        }
                                        float _32108 = _31531.y;
                                        vec2 _32109 = vec2(_31543.x, _32108);
                                        vec2 _43699;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _40959 = _32109;
                                            _40959.y = 1.0 - _32108;
                                            _43699 = _40959;
                                        }
                                        else
                                        {
                                            _43699 = _32109;
                                        }
                                        float _32126 = _31543.y;
                                        vec2 _32127 = vec2(_31531.x, _32126);
                                        vec2 _43701;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _40966 = _32127;
                                            _40966.y = 1.0 - _32126;
                                            _43701 = _40966;
                                        }
                                        else
                                        {
                                            _43701 = _32127;
                                        }
                                        vec2 _43703;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _40973 = _31543;
                                            _40973.y = 1.0 - _31543.y;
                                            _43703 = _40973;
                                        }
                                        else
                                        {
                                            _43703 = _31543;
                                        }
                                        _43738 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43698, 0.0) * (_31515.x * _31515.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43699, 0.0) * (_31519.x * _31515.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43701, 0.0) * (_31515.x * _31519.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43703, 0.0) * (_31519.x * _31519.y));
                                    }
                                    _43737 = _43738;
                                }
                                _43736 = _43737;
                            }
                            _43735 = _43736;
                        }
                        _43734 = _43735;
                    }
                    _31376 = _43767 + (_43734.xyz * (_31347 ? ((_43697 == 0) ? (1.0 - _31342) : _31342) : 1.0));
                }
                vec3 _40977 = _43671;
                _40977.x = _43767.x;
                vec2 _18662 = (_18602 + _18614) + (_18617 * _18644);
                float _32223 = clamp(log2(max(_18516, 1.0)) - 1.0, 0.0, 5.0);
                int _32226 = int(floor(_32223));
                float _32230 = _32223 - float(_32226);
                bool _32235 = (_32230 > 0.0199999995529651641845703125) && (_32226 < 5);
                vec3 _43891;
                _43891 = vec3(0.0);
                vec3 _32264;
                SPIRV_CROSS_LOOP
                for (int _43821 = 0; _43821 < 2; _43891 = _32264, _43821++)
                {
                    if ((_43821 > 0) && (!_32235))
                    {
                        break;
                    }
                    int _32250 = _32226 + _43821;
                    int _32288 = clamp(_32250, 1, 5);
                    vec2 _32357 = (_18662 * _292.gLevel[_32288].xy) - vec2(0.5);
                    vec2 _32359 = floor(_32357);
                    vec2 _32362 = _32357 - _32359;
                    vec2 _32365 = _32362 * _32362;
                    vec2 _32368 = _32365 * _32362;
                    vec2 _32387 = (((_32368 * 3.0) - (_32365 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                    vec2 _32400 = _32368 * 0.16666667163372039794921875;
                    vec2 _32403 = (((((-_32368) + (_32365 * 3.0)) - (_32362 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _32387;
                    vec2 _32407 = (((((_32368 * (-3.0)) + (_32365 * 3.0)) + (_32362 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _32400;
                    vec2 _32419 = ((_32359 - vec2(0.5)) + (_32387 / _32403)) * _292.gLevel[_32288].zw;
                    vec2 _32431 = ((_32359 + vec2(1.5)) + (_32400 / _32407)) * _292.gLevel[_32288].zw;
                    vec4 _43858;
                    SPIRV_CROSS_BRANCH
                    if (_32250 <= 0)
                    {
                        vec2 _43857;
                        if (_292.gConv.x > 0.5)
                        {
                            vec2 _40980 = _18662;
                            _40980.y = 1.0 - _18662.y;
                            _43857 = _40980;
                        }
                        else
                        {
                            _43857 = _18662;
                        }
                        _43858 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _43857, 0.0);
                    }
                    else
                    {
                        vec4 _43859;
                        if (_32250 == 1)
                        {
                            vec2 _43850;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40985 = _32419;
                                _40985.y = 1.0 - _32419.y;
                                _43850 = _40985;
                            }
                            else
                            {
                                _43850 = _32419;
                            }
                            float _32476 = _32419.y;
                            vec2 _32477 = vec2(_32431.x, _32476);
                            vec2 _43851;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40992 = _32477;
                                _40992.y = 1.0 - _32476;
                                _43851 = _40992;
                            }
                            else
                            {
                                _43851 = _32477;
                            }
                            float _32494 = _32431.y;
                            vec2 _32495 = vec2(_32419.x, _32494);
                            vec2 _43853;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _40999 = _32495;
                                _40999.y = 1.0 - _32494;
                                _43853 = _40999;
                            }
                            else
                            {
                                _43853 = _32495;
                            }
                            vec2 _43855;
                            if (_292.gConv.x > 0.5)
                            {
                                vec2 _41006 = _32431;
                                _41006.y = 1.0 - _32431.y;
                                _43855 = _41006;
                            }
                            else
                            {
                                _43855 = _32431;
                            }
                            _43859 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43850, 0.0) * (_32403.x * _32403.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43851, 0.0) * (_32407.x * _32403.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43853, 0.0) * (_32403.x * _32407.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _43855, 0.0) * (_32407.x * _32407.y));
                        }
                        else
                        {
                            vec4 _43860;
                            if (_32250 == 2)
                            {
                                vec2 _43843;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _41013 = _32419;
                                    _41013.y = 1.0 - _32419.y;
                                    _43843 = _41013;
                                }
                                else
                                {
                                    _43843 = _32419;
                                }
                                float _32606 = _32419.y;
                                vec2 _32607 = vec2(_32431.x, _32606);
                                vec2 _43844;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _41020 = _32607;
                                    _41020.y = 1.0 - _32606;
                                    _43844 = _41020;
                                }
                                else
                                {
                                    _43844 = _32607;
                                }
                                float _32624 = _32431.y;
                                vec2 _32625 = vec2(_32419.x, _32624);
                                vec2 _43846;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _41027 = _32625;
                                    _41027.y = 1.0 - _32624;
                                    _43846 = _41027;
                                }
                                else
                                {
                                    _43846 = _32625;
                                }
                                vec2 _43848;
                                if (_292.gConv.x > 0.5)
                                {
                                    vec2 _41034 = _32431;
                                    _41034.y = 1.0 - _32431.y;
                                    _43848 = _41034;
                                }
                                else
                                {
                                    _43848 = _32431;
                                }
                                _43860 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43843, 0.0) * (_32403.x * _32403.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43844, 0.0) * (_32407.x * _32403.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43846, 0.0) * (_32403.x * _32407.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _43848, 0.0) * (_32407.x * _32407.y));
                            }
                            else
                            {
                                vec4 _43861;
                                if (_32250 == 3)
                                {
                                    vec2 _43836;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _41041 = _32419;
                                        _41041.y = 1.0 - _32419.y;
                                        _43836 = _41041;
                                    }
                                    else
                                    {
                                        _43836 = _32419;
                                    }
                                    float _32736 = _32419.y;
                                    vec2 _32737 = vec2(_32431.x, _32736);
                                    vec2 _43837;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _41048 = _32737;
                                        _41048.y = 1.0 - _32736;
                                        _43837 = _41048;
                                    }
                                    else
                                    {
                                        _43837 = _32737;
                                    }
                                    float _32754 = _32431.y;
                                    vec2 _32755 = vec2(_32419.x, _32754);
                                    vec2 _43839;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _41055 = _32755;
                                        _41055.y = 1.0 - _32754;
                                        _43839 = _41055;
                                    }
                                    else
                                    {
                                        _43839 = _32755;
                                    }
                                    vec2 _43841;
                                    if (_292.gConv.x > 0.5)
                                    {
                                        vec2 _41062 = _32431;
                                        _41062.y = 1.0 - _32431.y;
                                        _43841 = _41062;
                                    }
                                    else
                                    {
                                        _43841 = _32431;
                                    }
                                    _43861 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43836, 0.0) * (_32403.x * _32403.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43837, 0.0) * (_32407.x * _32403.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43839, 0.0) * (_32403.x * _32407.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _43841, 0.0) * (_32407.x * _32407.y));
                                }
                                else
                                {
                                    vec4 _43862;
                                    if (_32250 == 4)
                                    {
                                        vec2 _43829;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _41069 = _32419;
                                            _41069.y = 1.0 - _32419.y;
                                            _43829 = _41069;
                                        }
                                        else
                                        {
                                            _43829 = _32419;
                                        }
                                        float _32866 = _32419.y;
                                        vec2 _32867 = vec2(_32431.x, _32866);
                                        vec2 _43830;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _41076 = _32867;
                                            _41076.y = 1.0 - _32866;
                                            _43830 = _41076;
                                        }
                                        else
                                        {
                                            _43830 = _32867;
                                        }
                                        float _32884 = _32431.y;
                                        vec2 _32885 = vec2(_32419.x, _32884);
                                        vec2 _43832;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _41083 = _32885;
                                            _41083.y = 1.0 - _32884;
                                            _43832 = _41083;
                                        }
                                        else
                                        {
                                            _43832 = _32885;
                                        }
                                        vec2 _43834;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _41090 = _32431;
                                            _41090.y = 1.0 - _32431.y;
                                            _43834 = _41090;
                                        }
                                        else
                                        {
                                            _43834 = _32431;
                                        }
                                        _43862 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43829, 0.0) * (_32403.x * _32403.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43830, 0.0) * (_32407.x * _32403.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43832, 0.0) * (_32403.x * _32407.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _43834, 0.0) * (_32407.x * _32407.y));
                                    }
                                    else
                                    {
                                        vec2 _43822;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _41097 = _32419;
                                            _41097.y = 1.0 - _32419.y;
                                            _43822 = _41097;
                                        }
                                        else
                                        {
                                            _43822 = _32419;
                                        }
                                        float _32996 = _32419.y;
                                        vec2 _32997 = vec2(_32431.x, _32996);
                                        vec2 _43823;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _41104 = _32997;
                                            _41104.y = 1.0 - _32996;
                                            _43823 = _41104;
                                        }
                                        else
                                        {
                                            _43823 = _32997;
                                        }
                                        float _33014 = _32431.y;
                                        vec2 _33015 = vec2(_32419.x, _33014);
                                        vec2 _43825;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _41111 = _33015;
                                            _41111.y = 1.0 - _33014;
                                            _43825 = _41111;
                                        }
                                        else
                                        {
                                            _43825 = _33015;
                                        }
                                        vec2 _43827;
                                        if (_292.gConv.x > 0.5)
                                        {
                                            vec2 _41118 = _32431;
                                            _41118.y = 1.0 - _32431.y;
                                            _43827 = _41118;
                                        }
                                        else
                                        {
                                            _43827 = _32431;
                                        }
                                        _43862 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43822, 0.0) * (_32403.x * _32403.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43823, 0.0) * (_32407.x * _32403.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43825, 0.0) * (_32403.x * _32407.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _43827, 0.0) * (_32407.x * _32407.y));
                                    }
                                    _43861 = _43862;
                                }
                                _43860 = _43861;
                            }
                            _43859 = _43860;
                        }
                        _43858 = _43859;
                    }
                    _32264 = _43891 + (_43858.xyz * (_32235 ? ((_43821 == 0) ? (1.0 - _32230) : _32230) : 1.0));
                }
                _40977.z = _43891.z;
                _44061 = _40977;
            }
            else
            {
                _44061 = _43671;
            }
            vec3 _44281;
            SPIRV_CROSS_BRANCH
            if (_18544 > 0.0)
            {
                vec2 _18685 = _18602 + (((_43599 * min(_18535 * 0.5, 16.0)) * _292.gDisplay.zw) * _292.gTarget.zw);
                float _33111 = clamp(log2(max(16.0 * _292.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _33114 = int(floor(_33111));
                float _33118 = _33111 - float(_33114);
                bool _33123 = (_33118 > 0.0199999995529651641845703125) && (_33114 < 5);
                vec3 _44046;
                _44046 = vec3(0.0);
                vec3 _33152;
                SPIRV_CROSS_LOOP
                for (int _44031 = 0; _44031 < 2; _44046 = _33152, _44031++)
                {
                    if ((_44031 > 0) && (!_33123))
                    {
                        break;
                    }
                    int _33138 = _33114 + _44031;
                    vec2 _44032;
                    if (_292.gConv.x > 0.5)
                    {
                        vec2 _41125 = _18685;
                        _41125.y = 1.0 - _18685.y;
                        _44032 = _41125;
                    }
                    else
                    {
                        _44032 = _18685;
                    }
                    vec4 _44033;
                    SPIRV_CROSS_BRANCH
                    if (_33138 <= 0)
                    {
                        _44033 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _44032, 0.0);
                    }
                    else
                    {
                        vec4 _44034;
                        if (_33138 == 1)
                        {
                            _44034 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _44032, 0.0);
                        }
                        else
                        {
                            vec4 _44035;
                            if (_33138 == 2)
                            {
                                _44035 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _44032, 0.0);
                            }
                            else
                            {
                                vec4 _44036;
                                if (_33138 == 3)
                                {
                                    _44036 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _44032, 0.0);
                                }
                                else
                                {
                                    vec4 _44037;
                                    if (_33138 == 4)
                                    {
                                        _44037 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _44032, 0.0);
                                    }
                                    else
                                    {
                                        _44037 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _44032, 0.0);
                                    }
                                    _44036 = _44037;
                                }
                                _44035 = _44036;
                            }
                            _44034 = _44035;
                        }
                        _44033 = _44034;
                    }
                    _33152 = _44046 + (_44033.xyz * (_33123 ? ((_44031 == 0) ? (1.0 - _33118) : _33118) : 1.0));
                }
                _44281 = _44046;
            }
            else
            {
                _44281 = vec3(0.5);
            }
            float _44077;
            SPIRV_CROSS_BRANCH
            if (esia_v5.x > 0.001000000047497451305389404296875)
            {
                int _33241 = clamp(int(roundEven(log2(36.0 * _292.gDisplay.z) - 1.0)), 1, 4);
                vec2 _44052;
                if (_292.gConv.x > 0.5)
                {
                    vec2 _41129 = _18602;
                    _41129.y = 1.0 - _18602.y;
                    _44052 = _41129;
                }
                else
                {
                    _44052 = _18602;
                }
                vec4 _44053;
                SPIRV_CROSS_BRANCH
                if (_33241 <= 0)
                {
                    _44053 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _44052, 0.0);
                }
                else
                {
                    vec4 _44054;
                    if (_33241 == 1)
                    {
                        _44054 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _44052, 0.0);
                    }
                    else
                    {
                        vec4 _44055;
                        if (_33241 == 2)
                        {
                            _44055 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _44052, 0.0);
                        }
                        else
                        {
                            vec4 _44056;
                            if (_33241 == 3)
                            {
                                _44056 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _44052, 0.0);
                            }
                            else
                            {
                                vec4 _44057;
                                if (_33241 == 4)
                                {
                                    _44057 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _44052, 0.0);
                                }
                                else
                                {
                                    _44057 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _44052, 0.0);
                                }
                                _44056 = _44057;
                            }
                            _44055 = _44056;
                        }
                        _44054 = _44055;
                    }
                    _44053 = _44054;
                }
                _44077 = dot(_44053.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _44077 = 0.5;
            }
            _44279 = _44281;
            _44076 = _44077;
            _44058 = _44061;
        }
        else
        {
            _44279 = vec3(0.5);
            _44076 = 0.5;
            _44058 = vec3(0.5);
        }
        vec3 _18713 = mix(vec3(dot(_44058, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _44058, vec3(_18404.x)) + vec3(_18404.y);
        vec3 _44172;
        SPIRV_CROSS_BRANCH
        if (esia_v5.x > 0.001000000047497451305389404296875)
        {
            float _18723 = clamp(max(_44076 + _18404.y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _18731 = mix(_18723, dot(_18363.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), esia_v5.x);
            vec3 _18739 = _18713 * (_18731 / _18723);
            vec3 _18749 = mix(_18713, vec3(1.0), vec3((_18731 - _18723) / max(1.0 - _18723, 0.001000000047497451305389404296875)));
            bvec3 _18750 = bvec3(_18731 < _18723);
            _44172 = vec3(_18750.x ? _18739.x : _18749.x, _18750.y ? _18739.y : _18749.y, _18750.z ? _18739.z : _18749.z);
        }
        else
        {
            _44172 = _18713;
        }
        vec2 _18769 = clamp((esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875)), vec2(0.0), vec2(1.0));
        vec2 _44392;
        SPIRV_CROSS_BRANCH
        if (_334.gDrawInfo.w > 0.5)
        {
            vec2 _33348 = _334.gDrawInfo.yz - esia_v0;
            bvec2 _33352 = bvec2(_334.gDrawInfo.w > 1.0);
            vec2 _33353 = vec2(_33352.x ? _33348.x : _334.gDrawInfo.yz.x, _33352.y ? _33348.y : _334.gDrawInfo.yz.y);
            _44392 = _33353 / vec2(max(length(_33353), 9.9999997473787516355514526367188e-05));
        }
        else
        {
            _44392 = vec2(cos(_18404.w), sin(_18404.w));
        }
        float _18825 = -_41577;
        float _18832 = _5632 * ((0.5 * abs(_43599.x)) + 0.0199999995529651641845703125);
        float _18839 = _5632 * ((0.5 * abs(_43599.y)) + 0.0199999995529651641845703125);
        float _44396;
        SPIRV_CROSS_BRANCH
        if ((((_18825 + _18832) + _18839) > 0.0) && (((_18825 - _18832) - _18839) < (0.5 * _18535)))
        {
            float _33372 = max((_18825 + _18832) + _18839, 0.0) / _18535;
            float _33376 = clamp(sqrt(2.0 * _33372), 0.12399999797344207763671875, 1.0);
            float _33378 = 1.0 - _33376;
            float _33381 = _33378 * _33378;
            float _33435 = max((_18825 + _18832) - _18839, 0.0) / _18535;
            float _33439 = clamp(sqrt(2.0 * _33435), 0.12399999797344207763671875, 1.0);
            float _33441 = 1.0 - _33439;
            float _33444 = _33441 * _33441;
            float _33498 = max((_18825 - _18832) + _18839, 0.0) / _18535;
            float _33502 = clamp(sqrt(2.0 * _33498), 0.12399999797344207763671875, 1.0);
            float _33504 = 1.0 - _33502;
            float _33507 = _33504 * _33504;
            float _33561 = max((_18825 - _18832) - _18839, 0.0) / _18535;
            float _33565 = clamp(sqrt(2.0 * _33561), 0.12399999797344207763671875, 1.0);
            float _33567 = 1.0 - _33565;
            float _33570 = _33567 * _33567;
            float _18887 = ((((((_33372 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _33372) * _33372) : ((_33372 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _33376) * _33376)) + ((((_33381 * _33381) * _33381) * _33378) * (0.02380952425301074981689453125 + (_33378 * ((-0.0386904776096343994140625) + (_33378 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_33372 - 0.5))))) * _18535) * _18535) - ((((_33435 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _33435) * _33435) : ((_33435 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _33439) * _33439)) + ((((_33444 * _33444) * _33444) * _33441) * (0.02380952425301074981689453125 + (_33441 * ((-0.0386904776096343994140625) + (_33441 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_33435 - 0.5))))) * _18535) * _18535)) - ((((_33498 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _33498) * _33498) : ((_33498 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _33502) * _33502)) + ((((_33507 * _33507) * _33507) * _33504) * (0.02380952425301074981689453125 + (_33504 * ((-0.0386904776096343994140625) + (_33504 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_33498 - 0.5))))) * _18535) * _18535)) + ((((_33561 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _33561) * _33561) : ((_33561 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _33565) * _33565)) + ((((_33570 * _33570) * _33570) * _33567) * (0.02380952425301074981689453125 + (_33567 * ((-0.0386904776096343994140625) + (_33567 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_33561 - 0.5))))) * _18535) * _18535);
            float _18895 = dot(_43599, _44392);
            _44396 = (((_18887 / ((4.0 * _18832) * _18839)) * (pow(clamp(_18895, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(clamp(-_18895, 0.0, 1.0), 1.5)))) * _18404.z) * 1.60000002384185791015625;
        }
        else
        {
            _44396 = 0.0;
        }
        float _18931 = clamp(0.5 + (0.5 * dot((esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5)) / _18527, _44392)), 0.0, 1.0);
        _47370 = _44396;
        _46381 = _18445;
        _46051 = vec4((mix(mix(_44172, _18363.xyz, vec3(clamp(_18363.w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _18550))) + (0.3499999940395355224609375 * ((dot(_18363.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _18769.y) : _18769.y))), 0.0, 1.0))), (_44279 * 1.10000002384185791015625) + vec3(0.07999999821186065673828125), vec3(pow(1.0 - _44176.z, 5.0) * 0.3499999940395355224609375)) + vec3((((0.039999999105930328369140625 * _18931) * _18931) + (0.0500000007450580596923828125 * (1.0 - _18550))) * _18404.z)) * _42238, _42238) + (_44704 * (1.0 - _42238));
    }
    else
    {
        _47370 = 0.0;
        _46381 = _41466;
        _46051 = _44704;
    }
    vec4 _46063;
    vec4 _46067;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _4354) != 0u)
    {
        vec4 _45000;
        vec4 _45515;
        SPIRV_CROSS_BRANCH
        if (esia_v7.y != 0u)
        {
            uint _33627 = max(uint(_292.gConv.z), 1u);
            uint _33668 = max(uint(_292.gConv.z), 1u);
            _45515 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _33627) * 24u) + 3u), int(esia_v1 / _33627), 0).xy, 0);
            _45000 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _33668) * 24u) + 4u), int(esia_v1 / _33668), 0).xy, 0);
        }
        else
        {
            _45515 = vec4(0.0);
            _45000 = vec4(0.0);
        }
        vec4 _46038;
        do
        {
            if (esia_v7.y == 0u)
            {
                _46038 = esia_v4;
                break;
            }
            vec2 _33740 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            vec2 _33750 = fwidth(esia_v0);
            float _33752 = max(length(_33750), 9.9999997473787516355514526367188e-05);
            float _46030;
            float _46034;
            if ((esia_v7.y == 1u) || (esia_v7.y == 4u))
            {
                float _33761 = cos(_45000.x);
                float _33764 = sin(_45000.x);
                float _33790 = ((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_33761, _33764)) / max(0.5 * ((abs(_33761) * _33740.x) + (abs(_33764) * _33740.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (esia_v7.y == 4u)
                {
                    float _33805 = clamp((_33790 - _45000.y) / max(_45000.z - _45000.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _46038 = vec4(clamp(abs((fract(vec3(_33805 * 0.800000011920928955078125) + vec3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - vec3(3.0)) - vec3(1.0), vec3(0.0), vec3(1.0)), esia_v4.w * pow(max(sin(_33805 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _46034 = -1.0;
                _46030 = _33790;
            }
            else
            {
                float _46031;
                float _46035;
                if (esia_v7.y == 2u)
                {
                    _46035 = -1.0;
                    _46031 = length(esia_v0 - (esia_v2.xy + (_45000.xy * _33740))) / max(_45000.z * max(_33740.x, _33740.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    vec2 _33873 = esia_v0 - (esia_v2.xy + (_45000.xy * _33740));
                    float _33884 = fract(((atan(_33873.y, _33873.x) - _45000.z) * 0.15915493667125701904296875) + 1.0);
                    float _46032;
                    float _46036;
                    if (_45000.w > 0.5)
                    {
                        _46036 = -1.0;
                        _46032 = 0.5 - (0.5 * cos(_33884 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _33904 = (((_33884 < 0.5) ? _33884 : (_33884 - 1.0)) * 6.283185482025146484375) * length(_33873);
                        float _46037;
                        if (abs(_33904) < _33752)
                        {
                            _46037 = clamp(((_33904 / _33752) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _46037 = -1.0;
                        }
                        _46036 = _46037;
                        _46032 = _33884;
                    }
                    _46035 = _46036;
                    _46031 = _46032;
                }
                _46034 = _46035;
                _46030 = _46031;
            }
            vec4 _33933 = vec4(esia_v4.xyz * esia_v4.w, esia_v4.w);
            vec4 _33945 = vec4(_45515.xyz * _45515.w, _45515.w);
            vec4 _33952 = mix(_33945, _33933, vec4(_46034));
            vec4 _33957 = mix(_33933, _33945, vec4(clamp(_46030, 0.0, 1.0)));
            bvec4 _33958 = bvec4(_46034 >= 0.0);
            vec4 _33959 = vec4(_33958.x ? _33952.x : _33957.x, _33958.y ? _33952.y : _33957.y, _33958.z ? _33952.z : _33957.z, _33958.w ? _33952.w : _33957.w);
            vec4 _33974 = vec4(_33959.xyz / vec3(_33959.w), _33959.w);
            bvec4 _33975 = bvec4(_33959.w > 9.9999997473787516355514526367188e-06);
            _46038 = vec4(_33975.x ? _33974.x : vec4(0.0).x, _33975.y ? _33974.y : vec4(0.0).y, _33975.z ? _33974.z : vec4(0.0).z, _33975.w ? _33974.w : vec4(0.0).w);
            break;
        } while(false);
        vec4 _46039;
        vec4 _46370;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & _4383) != 0u)
        {
            uint _33985 = max(uint(_292.gConv.z), 1u);
            vec4 _34019 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _33985) * 24u) + 18u), int(esia_v1 / _33985), 0).xy, 0);
            _46370 = _34019;
            _46039 = _46038 * texture(SPIRV_Cross_CombinedgTexgLinear, mix(_34019.xy, _34019.zw, (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875))));
        }
        else
        {
            _46370 = _41462;
            _46039 = _46038;
        }
        float _34027 = clamp(_46039.w * _42238, 0.0, 1.0);
        _46067 = _46370;
        _46063 = vec4(_46039.xyz * _34027, _34027) + (_46051 * (1.0 - _34027));
    }
    else
    {
        _46067 = _41462;
        _46063 = _46051;
    }
    vec4 _47322;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _4436) != 0u)
    {
        uint _34052 = max(uint(_292.gConv.z), 1u);
        vec4 _34086 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _34052) * 24u) + 21u), int(esia_v1 / _34052), 0).xy, 0);
        uint _34093 = max(uint(_292.gConv.z), 1u);
        vec4 _34127 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _34093) * 24u) + 3u), int(esia_v1 / _34093), 0).xy, 0);
        vec2 _6285 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        vec2 _6292 = (esia_v0 - esia_v2.xy) / _6285;
        float _6303 = ((_34127.x >= 0.0) ? _34127.x : _292.gTime.x) * _34086.z;
        float _6306 = _6292.x * 2.0;
        float _6307 = _6306 - 1.0;
        float _6312 = _6292.y * _6285.y;
        float _6315 = max(_34086.w, 0.001000000047497451305389404296875);
        float _6320 = clamp(1.0 - (_6307 * _6307), 0.0, 1.0);
        float _6322 = pow(_6320, 1.2999999523162841796875);
        float _6324 = pow(_6320, 0.699999988079071044921875);
        float _6327 = clamp(_6303 * 1.4285714626312255859375, 0.0, 1.0);
        float _6343 = (0.25 + (0.75 * ((_6327 * _6327) * (3.0 - (2.0 * _6327))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_6303 * 2.099999904632568359375)));
        float _6352 = ((_34086.y * _6285.y) * _6343) * _6322;
        float _6378 = (_6285.y * (0.5 + ((_34086.x * (0.5 - (_6307 * _6307))) * 0.5))) + (((0.14000000059604644775390625 * _6285.y) * _6322) * sin(((_6307 * 2.400000095367431640625) - (_6303 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _46057;
        float _46058;
        vec3 _46059;
        _46059 = vec3(0.0);
        _46058 = _6378;
        _46057 = _6378;
        vec3 _6450;
        float _49528;
        float _49529;
        SPIRV_CROSS_UNROLL
        for (int _46056 = 0; _46056 < 4; _46059 = _6450, _46058 = _49529, _46057 = _49528, _46056++)
        {
            float _6402 = _6378 + ((_6352 * _4598[_46056].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_6303 * 1.7000000476837158203125) + _4615[_46056].y))));
            _49528 = (_46056 == 0) ? _6402 : _46057;
            _49529 = (_46056 == 2) ? _6402 : _46058;
            float _6417 = _6315 * _4598[_46056].y;
            float _6422 = (_6312 - _6402) / _6417;
            float _6427 = _6315 * _4598[_46056].z;
            vec3 _48316;
            _48316 = vec3(0.0);
            SPIRV_CROSS_UNROLL
            for (int _48315 = 0; _48315 < 6; )
            {
                float _34150 = ((_6312 - _6402) - (_6427 * ((float(_48315) * 0.4000000059604644775390625) - 1.0))) / _6417;
                _48316 += (_2986[_48315] * exp((-_34150) * _34150));
                _48315++;
                continue;
            }
            _6450 = _46059 + (mix(_48316 * vec3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), vec3(exp((-_6422) * _6422)), vec3(_4615[_46056].x)) * (_4598[_46056].w * _6324));
        }
        float _6456 = _6315 * 1.5;
        float _6486 = clamp((_6312 - _46057) / max(_46058 - _46057, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _6514 = (_6312 - (_46058 - (_6315 * 3.0))) / (((_6285.y * 0.0900000035762786865234375) + (_6352 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _6517 = (_6306 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _6542 = ((_6312 - _46057) + (_6315 * 5.0)) / (_6315 * 7.0);
        vec3 _6568 = vec3(1.0) - exp((-((((_46059 + (mix(vec3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), vec3(1.0), vec3(_6486)) * ((((1.0 / (1.0 + exp((-((_6312 - _46057) - (_6315 * 2.0))) / _6456))) / (1.0 + exp((-(_46058 - _6312)) / _6456))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_6486, 2.5)))) * _6324))) + (vec3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_6514) * _6514) - (_6517 * _6517)) * (0.5 + (1.10000002384185791015625 * _6343))))) + (vec3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_6542) * _6542) * _6322) * 0.100000001490116119384765625))) * mix(vec3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), vec3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), vec3(0.5 + (0.5 * sin(_6303 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _6578 = (esia_v4.w * smoothstep(0.0, 0.119999997317790985107421875, _6292.y)) * smoothstep(1.0, 0.87999999523162841796875, _6292.y);
        float _6597 = (clamp(max(_6568.x, max(_6568.y, _6568.z)), 0.0, 1.0) * _6578) * _42238;
        _47322 = vec4((_6568 * _6578) * _42238, _6597) + (_46063 * (1.0 - _6597));
    }
    else
    {
        _47322 = _46063;
    }
    vec4 _47365;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & _4867) != 0u) && ((esia_v7.x & _4871) != 0u))
    {
        uint _34183 = max(uint(_292.gConv.z), 1u);
        vec4 _34217 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _34183) * 24u) + 7u), int(esia_v1 / _34183), 0).xy, 0);
        uint _34224 = max(uint(_292.gConv.z), 1u);
        vec4 _34258 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _34224) * 24u) + 8u), int(esia_v1 / _34224), 0).xy, 0);
        vec2 _46064;
        SPIRV_CROSS_BRANCH
        if (_334.gDrawInfo.w > 0.5)
        {
            vec2 _34300 = _334.gDrawInfo.yz - ((esia_v2.xy + esia_v2.zw) * 0.5);
            bvec2 _34304 = bvec2(_334.gDrawInfo.w > 1.0);
            vec2 _34305 = vec2(_34304.x ? _34300.x : _334.gDrawInfo.yz.x, _34304.y ? _34300.y : _334.gDrawInfo.yz.y);
            _46064 = (-(_34305 / vec2(max(length(_34305), 9.9999997473787516355514526367188e-05)))) * length(_34258.zw);
        }
        else
        {
            _46064 = _34258.zw;
        }
        vec2 _6630 = esia_v0 - _46064;
        vec2 _34356 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _34365 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _47185;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & _2279) != 0u)
        {
            float _34482 = 1.0 / max(esia_v5.z, 9.9999999747524270787835121154785e-07);
            vec4 _34516 = textureLod(SPIRV_Cross_CombinedgTexgLinear, mix(_46067.xy, _46067.zw, (clamp(_6630, _41468.xy, _41468.zw) - _41468.xy) / max(_41468.zw - _41468.xy, vec2(9.9999999747524270787835121154785e-07))), 0.0);
            float _34540 = (_34516.w - 0.5) * esia_v3.y;
            vec2 _34581 = abs(_6630 - ((_41468.xy + _41468.zw) * 0.5)) - max(((_41468.zw - _41468.xy) * 0.5) - vec2(((0.5 * esia_v3.y) + 1.0) * _34482), vec2(0.0));
            _47185 = max((-mix((max(min(_34516.x, _34516.y), min(max(_34516.x, _34516.y), _34516.z)) - 0.5) * esia_v3.x, _34540, clamp(abs(_34540) - ((0.5 * esia_v3.x) - 1.5), 0.0, 1.0))) * _34482, length(max(_34581, vec2(0.0))) + min(max(_34581.x, _34581.y), 0.0)) - esia_v5.w;
        }
        else
        {
            float _47186;
            if ((esia_v7.x & _2294) != 0u)
            {
                float _47133;
                do
                {
                    bool _34647 = esia_v3.w > 0.5;
                    float _34651 = floor((esia_v3.z + 1.0) * 0.5);
                    float _34655 = floor((_34651 + 127.0) * 0.0078125);
                    float _47126;
                    float _47128;
                    SPIRV_CROSS_BRANCH
                    if (_34647)
                    {
                        uint _34831 = uint(esia_v3.z);
                        uint _34834 = (_34831 + 31u) >> 5u;
                        uint _34837 = esia_v1 + 1u;
                        uint _34920 = max(uint(_292.gConv.z), 1u);
                        ivec2 _34930 = ivec2(int((_34837 % _34920) * 24u), int(_34837 / _34920));
                        ivec2 _47118;
                        uint _47119;
                        ivec2 _47179;
                        _47179 = _34930;
                        _47119 = 0u;
                        _47118 = _34930;
                        float _49530;
                        uint _49531;
                        ivec2 _49533;
                        ivec2 _47182;
                        uint _47117 = 0u;
                        float _47180 = 3.0000000054977557577780399428115e+38;
                        SPIRV_CROSS_LOOP
                        for (; _47117 < _34834; _47180 = _49530, _47179 = _47182, _47119 = _49531, _47118 = _49533, _47117++)
                        {
                            vec4 _34958 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47179.x + 20, _47179.y, 0).xy, 0);
                            vec2 _34971 = max(max(_34958.xy - _6630, _6630 - _34958.zw), vec2(0.0));
                            float _34974 = dot(_34971, _34971);
                            bool _34852 = _34974 < _47180;
                            _49530 = _34852 ? _34974 : _47180;
                            _49531 = _34852 ? _47117 : _47119;
                            bvec2 _49532 = bvec2(_34852);
                            _49533 = ivec2(_49532.x ? _47179.x : _47118.x, _49532.y ? _47179.y : _47118.y);
                            int _34979 = _47179.x + 24;
                            ivec2 _41221 = _47179;
                            _41221.x = _34979;
                            if (_34979 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                            {
                                _47182 = ivec2(0, _47179.y + 1);
                            }
                            else
                            {
                                _47182 = _41221;
                            }
                        }
                        uint _35034 = _47119 * 32u;
                        uint _35038 = min((_34831 - _35034), 32u);
                        vec4 _35165 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47118.x + 21, _47118.y, 0).xy, 0);
                        vec4 _35193 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47118.x + 22, _47118.y, 0).xy, 0);
                        float _47122;
                        uint _47123;
                        _47123 = 4294967295u;
                        _47122 = 1000000015047466219876688855040.0;
                        float _49259;
                        uint _49260;
                        SPIRV_CROSS_LOOP
                        for (uint _47121 = 0u; (_47121 * 8u) < _35038; _47123 = _49260, _47122 = _49259, _47121++)
                        {
                            vec4 _35221 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47118.x + int(16u + _47121), _47118.y, 0).xy, 0);
                            float _35239 = ((sqrt(_47122) * 1.000003814697265625) + 0.03125) + ((abs(_6630.x) + abs(_6630.y)) * 3.814697265625e-06);
                            vec2 _35259 = max(max(_35221.xy - _6630, _6630 - _35221.zw), vec2(0.0));
                            if (dot(_35259, _35259) > (_35239 * _35239))
                            {
                                _49260 = _47123;
                                _49259 = _47122;
                                continue;
                            }
                            bvec4 _35063 = bvec4(_47121 < 2u);
                            vec4 _35064 = vec4(_35063.x ? _35165.x : _35193.x, _35063.y ? _35165.y : _35193.y, _35063.z ? _35165.z : _35193.z, _35063.w ? _35165.w : _35193.w);
                            vec2 _35069 = _35064.zw;
                            vec2 _35071 = _35064.xy;
                            bvec2 _35072 = bvec2((_47121 & 1u) != 0u);
                            uint _35078 = min(_35038, ((_47121 * 8u) + 8u));
                            uint _35080 = _47121 * 8u;
                            vec2 _47164;
                            float _47165;
                            uint _47166;
                            _47166 = _47123;
                            _47165 = _47122;
                            _47164 = vec2(_35072.x ? _35069.x : _35071.x, _35072.y ? _35069.y : _35071.y);
                            vec2 _49308;
                            float _49309;
                            uint _49310;
                            SPIRV_CROSS_LOOP
                            for (uint _47163 = _35080; _47163 < _35078; _47166 = _49310, _47165 = _49309, _47164 = _49308, _47163 += 2u)
                            {
                                vec4 _35290 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47118.x + int(_47163 >> 1u), _47118.y, 0).xy, 0);
                                uint _35093 = _35034 + _47163;
                                vec2 _35096 = _35290.xy;
                                vec2 _35298 = _47164 - _35096;
                                vec2 _35301 = _6630 - _35096;
                                vec2 _35314 = _35301 - (_35298 * clamp(dot(_35301, _35298) / max(dot(_35298, _35298), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _35317 = dot(_35314, _35314);
                                bool _35328 = (_35317 < _47165) || ((_35317 == _47165) && (_35093 < _47166));
                                uint _49534 = _35328 ? _35093 : _47166;
                                float _49535 = _35328 ? _35317 : _47165;
                                vec2 _35106 = _35290.xy;
                                if ((_47163 + 1u) < _35078)
                                {
                                    uint _35115 = (_35034 + _47163) + 1u;
                                    vec2 _35118 = _35290.zw;
                                    vec2 _35341 = _35106 - _35118;
                                    vec2 _35344 = _6630 - _35118;
                                    vec2 _35357 = _35344 - (_35341 * clamp(dot(_35344, _35341) / max(dot(_35341, _35341), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _35360 = dot(_35357, _35357);
                                    bool _35371 = (_35360 < _49535) || ((_35360 == _49535) && (_35115 < _49534));
                                    _49310 = _35371 ? _35115 : _49534;
                                    _49309 = _35371 ? _35360 : _49535;
                                    _49308 = _35290.zw;
                                }
                                else
                                {
                                    _49310 = _49534;
                                    _49309 = _49535;
                                    _49308 = _35106;
                                }
                            }
                            _49260 = _47166;
                            _49259 = _47165;
                        }
                        uint _34877 = esia_v1 + 1u;
                        uint _35383 = max(uint(_292.gConv.z), 1u);
                        float _47125;
                        ivec2 _47136;
                        _47136 = ivec2(int((_34877 % _35383) * 24u), int(_34877 / _35383));
                        _47125 = _47122;
                        ivec2 _47143;
                        float _49262;
                        uint _49292;
                        SPIRV_CROSS_LOOP
                        for (uint _47124 = 0u, _47137 = _47123; _47124 < _34834; _47137 = _49292, _47136 = _47143, _47125 = _49262, _47124++)
                        {
                            vec4 _35421 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47136.x + 20, _47136.y, 0).xy, 0);
                            float _35439 = ((sqrt(_47125) * 1.000003814697265625) + 0.03125) + ((abs(_6630.x) + abs(_6630.y)) * 3.814697265625e-06);
                            vec2 _35459 = max(max(_35421.xy - _6630, _6630 - _35421.zw), vec2(0.0));
                            if ((_47124 != _47119) && (!(dot(_35459, _35459) > (_35439 * _35439))))
                            {
                                uint _35500 = _47124 * 32u;
                                uint _35504 = min((_34831 - _35500), 32u);
                                vec4 _35631 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47136.x + 21, _47136.y, 0).xy, 0);
                                vec4 _35659 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47136.x + 22, _47136.y, 0).xy, 0);
                                float _47139;
                                uint _47140;
                                _47140 = _47137;
                                _47139 = _47125;
                                float _49293;
                                uint _49294;
                                SPIRV_CROSS_LOOP
                                for (uint _47138 = 0u; (_47138 * 8u) < _35504; _47140 = _49294, _47139 = _49293, _47138++)
                                {
                                    vec4 _35687 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47136.x + int(16u + _47138), _47136.y, 0).xy, 0);
                                    float _35705 = ((sqrt(_47139) * 1.000003814697265625) + 0.03125) + ((abs(_6630.x) + abs(_6630.y)) * 3.814697265625e-06);
                                    vec2 _35725 = max(max(_35687.xy - _6630, _6630 - _35687.zw), vec2(0.0));
                                    if (dot(_35725, _35725) > (_35705 * _35705))
                                    {
                                        _49294 = _47140;
                                        _49293 = _47139;
                                        continue;
                                    }
                                    bvec4 _35529 = bvec4(_47138 < 2u);
                                    vec4 _35530 = vec4(_35529.x ? _35631.x : _35659.x, _35529.y ? _35631.y : _35659.y, _35529.z ? _35631.z : _35659.z, _35529.w ? _35631.w : _35659.w);
                                    vec2 _35535 = _35530.zw;
                                    vec2 _35537 = _35530.xy;
                                    bvec2 _35538 = bvec2((_47138 & 1u) != 0u);
                                    uint _35544 = min(_35504, ((_47138 * 8u) + 8u));
                                    uint _35546 = _47138 * 8u;
                                    vec2 _47148;
                                    float _47149;
                                    uint _47150;
                                    _47150 = _47140;
                                    _47149 = _47139;
                                    _47148 = vec2(_35538.x ? _35535.x : _35537.x, _35538.y ? _35535.y : _35537.y);
                                    vec2 _49305;
                                    float _49306;
                                    uint _49307;
                                    SPIRV_CROSS_LOOP
                                    for (uint _47147 = _35546; _47147 < _35544; _47150 = _49307, _47149 = _49306, _47148 = _49305, _47147 += 2u)
                                    {
                                        vec4 _35756 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(_47136.x + int(_47147 >> 1u), _47136.y, 0).xy, 0);
                                        uint _35559 = _35500 + _47147;
                                        vec2 _35562 = _35756.xy;
                                        vec2 _35764 = _47148 - _35562;
                                        vec2 _35767 = _6630 - _35562;
                                        vec2 _35780 = _35767 - (_35764 * clamp(dot(_35767, _35764) / max(dot(_35764, _35764), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                        float _35783 = dot(_35780, _35780);
                                        bool _35794 = (_35783 < _47149) || ((_35783 == _47149) && (_35559 < _47150));
                                        uint _49538 = _35794 ? _35559 : _47150;
                                        float _49539 = _35794 ? _35783 : _47149;
                                        vec2 _35572 = _35756.xy;
                                        if ((_47147 + 1u) < _35544)
                                        {
                                            uint _35581 = (_35500 + _47147) + 1u;
                                            vec2 _35584 = _35756.zw;
                                            vec2 _35807 = _35572 - _35584;
                                            vec2 _35810 = _6630 - _35584;
                                            vec2 _35823 = _35810 - (_35807 * clamp(dot(_35810, _35807) / max(dot(_35807, _35807), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                            float _35826 = dot(_35823, _35823);
                                            bool _35837 = (_35826 < _49539) || ((_35826 == _49539) && (_35581 < _49538));
                                            _49307 = _35837 ? _35581 : _49538;
                                            _49306 = _35837 ? _35826 : _49539;
                                            _49305 = _35756.zw;
                                        }
                                        else
                                        {
                                            _49307 = _49538;
                                            _49306 = _49539;
                                            _49305 = _35572;
                                        }
                                    }
                                    _49294 = _47150;
                                    _49293 = _47149;
                                }
                                _49292 = _47140;
                                _49262 = _47139;
                            }
                            else
                            {
                                _49292 = _47137;
                                _49262 = _47125;
                            }
                            int _35847 = _47136.x + 24;
                            ivec2 _41268 = _47136;
                            _41268.x = _35847;
                            if (_35847 >= int(max(uint(_292.gConv.z), 1u) * 24u))
                            {
                                _47143 = ivec2(0, _47136.y + 1);
                            }
                            else
                            {
                                _47143 = _41268;
                            }
                        }
                        _47128 = 1.0;
                        _47126 = _47125;
                    }
                    else
                    {
                        float _34669 = floor((esia_v3.z - 1.0) * 0.5);
                        float _35872 = floor(_34669 * 0.0625);
                        uint _35877 = (esia_v1 + 1u) + uint(_35872);
                        uint _35890 = max(uint(_292.gConv.z), 1u);
                        vec4 _35924 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_35877 % _35890) * 24u) + uint(_34669 - (_35872 * 16.0))), int(_35877 / _35890), 0).xy, 0);
                        vec2 _34681 = _35924.xy;
                        vec2 _34683 = _35924.zw;
                        bvec2 _34684 = bvec2(((esia_v3.z - 1.0) - (_34669 * 2.0)) < 0.5);
                        vec2 _47096;
                        if (_34647)
                        {
                            float _35932 = floor(0.0);
                            uint _35937 = (esia_v1 + 1u) + uint(_35932);
                            uint _35950 = max(uint(_292.gConv.z), 1u);
                            _47096 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_35937 % _35950) * 24u) + uint(_35932 * (-16.0))), int(_35937 / _35950), 0).xy, 0).xy;
                        }
                        else
                        {
                            _47096 = vec2(_34684.x ? _34681.x : _34683.x, _34684.y ? _34681.y : _34683.y);
                        }
                        vec2 _47095;
                        float _47098;
                        float _47101;
                        _47101 = 1.0;
                        _47098 = 1000000015047466219876688855040.0;
                        _47095 = _47096;
                        vec2 _47094;
                        float _47097;
                        float _47100;
                        SPIRV_CROSS_LOOP
                        for (float _47091 = 0.0; _47091 < 16.0; _47101 = _47100, _47098 = _47097, _47095 = _47094, _47091 += 1.0)
                        {
                            if (_47091 >= _34655)
                            {
                                break;
                            }
                            _47100 = _47101;
                            _47097 = _47098;
                            _47094 = _47095;
                            vec2 _49248;
                            float _49250;
                            float _49252;
                            SPIRV_CROSS_LOOP
                            for (float _47092 = 0.0; _47092 < 128.0; _47100 = _49252, _47097 = _49250, _47094 = _49248, _47092 += 1.0)
                            {
                                float _34711 = (_47091 * 128.0) + _47092;
                                if (_34711 >= _34651)
                                {
                                    break;
                                }
                                float _35992 = floor(_34711 * 0.0625);
                                uint _35997 = (esia_v1 + 1u) + uint(_35992);
                                uint _36010 = max(uint(_292.gConv.z), 1u);
                                vec4 _36044 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((_35997 % _36010) * 24u) + uint(_34711 - (_35992 * 16.0))), int(_35997 / _36010), 0).xy, 0);
                                vec2 _34723 = _36044.xy;
                                vec2 _36055 = _47094 - _34723;
                                vec2 _36058 = _6630 - _34723;
                                vec2 _36071 = _36058 - (_36055 * clamp(dot(_36058, _36055) / max(dot(_36055, _36055), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                float _36074 = dot(_36071, _36071);
                                float _49542 = (_36074 < _47097) ? _36074 : _47097;
                                bool _36086 = _6630.y >= _36044.y;
                                bool _36091 = _6630.y < _47094.y;
                                bool _36102 = (_36055.x * _36058.y) > (_36055.y * _36058.x);
                                float _47106;
                                if (((_36086 && _36091) && _36102) || (((!_36086) && (!_36091)) && (!_36102)))
                                {
                                    _47106 = -_47100;
                                }
                                else
                                {
                                    _47106 = _47100;
                                }
                                vec2 _34733 = _36044.xy;
                                if (((2.0 * _34711) + 1.0) < esia_v3.z)
                                {
                                    vec2 _34742 = _36044.zw;
                                    vec2 _36131 = _34733 - _34742;
                                    vec2 _36134 = _6630 - _34742;
                                    vec2 _36147 = _36134 - (_36131 * clamp(dot(_36134, _36131) / max(dot(_36131, _36131), 9.9999999600419720025001879548654e-13), 0.0, 1.0));
                                    float _36150 = dot(_36147, _36147);
                                    bool _36162 = _6630.y >= _36044.w;
                                    bool _36167 = _6630.y < _36044.y;
                                    bool _36178 = (_36131.x * _36134.y) > (_36131.y * _36134.x);
                                    float _47110;
                                    if (((_36162 && _36167) && _36178) || (((!_36162) && (!_36167)) && (!_36178)))
                                    {
                                        _47110 = -_47106;
                                    }
                                    else
                                    {
                                        _47110 = _47106;
                                    }
                                    _49252 = _47110;
                                    _49250 = (_36150 < _49542) ? _36150 : _49542;
                                    _49248 = _36044.zw;
                                }
                                else
                                {
                                    _49252 = _47106;
                                    _49250 = _49542;
                                    _49248 = _34733;
                                }
                            }
                        }
                        _47128 = _47101;
                        _47126 = _47098;
                    }
                    float _34764 = sqrt(_47126);
                    if (_34647)
                    {
                        _47133 = _34764 - esia_v3.x;
                        break;
                    }
                    _47133 = (_47128 * _34764) - esia_v3.x;
                    break;
                } while(false);
                _47186 = _47133;
            }
            else
            {
                float _47187;
                if (esia_v7.z == 1u)
                {
                    vec2 _34390 = _6630 - _34356;
                    float _47090;
                    do
                    {
                        if (esia_v5.w >= 6.282185077667236328125)
                        {
                            _47090 = abs(length(_34390) - esia_v3.x) - esia_v3.y;
                            break;
                        }
                        float _36221 = esia_v5.z + (esia_v5.w * 0.5);
                        float _36223 = cos(_36221);
                        float _36225 = sin(_36221);
                        float _36234 = dot(_34390, vec2(-_36225, _36223));
                        float _36237 = dot(_34390, vec2(_36223, _36225));
                        vec2 _36238 = vec2(_36234, _36237);
                        float _36241 = abs(_36234);
                        _36238.x = _36241;
                        float _36244 = esia_v5.w * 0.5;
                        float _36246 = sin(_36244);
                        float _36248 = cos(_36244);
                        _47090 = (((_36248 * _36241) > (_36246 * _36237)) ? length(_36238 - (vec2(_36246, _36248) * esia_v3.x)) : abs(length(_36238) - esia_v3.x)) - esia_v3.y;
                        break;
                    } while(false);
                    _47187 = _47090;
                }
                else
                {
                    float _47188;
                    if (esia_v7.z == 2u)
                    {
                        vec2 _36284 = _6630 - _41468.xy;
                        vec2 _36287 = _41468.zw - _41468.xy;
                        _47188 = length(_36284 - (_36287 * clamp(dot(_36284, _36287) / max(dot(_36287, _36287), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                    }
                    else
                    {
                        vec2 _34417 = _6630 - _34356;
                        float _36352 = min(_34365.x, _34365.y);
                        float _36355 = min((_34417.x > 0.0) ? ((_34417.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_34417.y > 0.0) ? esia_v3.w : esia_v3.x), _36352);
                        float _36361 = _36355 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _47079;
                        float _47080;
                        if (_36361 > _36352)
                        {
                            float _36375 = esia_v5.y * clamp((_36352 - _36355) / max(0.60000002384185791015625 * _36355, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _47080 = _36375;
                            _47079 = _36355 * (1.0 + (0.60000002384185791015625 * _36375));
                        }
                        else
                        {
                            _47080 = esia_v5.y;
                            _47079 = _36361;
                        }
                        vec2 _36404 = (abs(_34417) - _34365) + vec2(_47079);
                        vec2 _36406 = max(_36404, vec2(0.0));
                        float _47081;
                        SPIRV_CROSS_BRANCH
                        if ((_36406.x > 0.0) && (_36406.y > 0.0))
                        {
                            float _47082;
                            if ((_47080 > 0.001000000047497451305389404296875) && (_47079 > 9.9999997473787516355514526367188e-05))
                            {
                                float _36423 = 2.0 + (2.0 * _47080);
                                vec2 _36428 = _36406 / vec2(max(_47079, 9.9999997473787516355514526367188e-05));
                                _47082 = pow(pow(_36428.x, _36423) + pow(_36428.y, _36423), 1.0 / _36423) * _47079;
                            }
                            else
                            {
                                _47082 = length(_36406);
                            }
                            _47081 = _47082;
                        }
                        else
                        {
                            _47081 = max(_36406.x, _36406.y);
                        }
                        float _36463 = (min(max(_36404.x, _36404.y), 0.0) + _47081) - _47079;
                        float _47189;
                        SPIRV_CROSS_BRANCH
                        if ((esia_v7.x & _2365) != 0u)
                        {
                            vec2 _34445 = max((_41468.zw - _41468.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                            vec2 _34448 = _6630 - ((_41468.xy + _41468.zw) * 0.5);
                            float _36511 = min(_34445.x, _34445.y);
                            float _36514 = min((_34448.x > 0.0) ? ((_34448.y > 0.0) ? _46381.x : _46381.x) : ((_34448.y > 0.0) ? _46381.x : _46381.x), _36511);
                            float _36520 = _36514 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                            float _47083;
                            float _47084;
                            if (_36520 > _36511)
                            {
                                float _36534 = esia_v5.y * clamp((_36511 - _36514) / max(0.60000002384185791015625 * _36514, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                                _47084 = _36534;
                                _47083 = _36514 * (1.0 + (0.60000002384185791015625 * _36534));
                            }
                            else
                            {
                                _47084 = esia_v5.y;
                                _47083 = _36520;
                            }
                            vec2 _36563 = (abs(_34448) - _34445) + vec2(_47083);
                            vec2 _36565 = max(_36563, vec2(0.0));
                            float _47085;
                            SPIRV_CROSS_BRANCH
                            if ((_36565.x > 0.0) && (_36565.y > 0.0))
                            {
                                float _47086;
                                if ((_47084 > 0.001000000047497451305389404296875) && (_47083 > 9.9999997473787516355514526367188e-05))
                                {
                                    float _36582 = 2.0 + (2.0 * _47084);
                                    vec2 _36587 = _36565 / vec2(max(_47083, 9.9999997473787516355514526367188e-05));
                                    _47086 = pow(pow(_36587.x, _36582) + pow(_36587.y, _36582), 1.0 / _36582) * _47083;
                                }
                                else
                                {
                                    _47086 = length(_36565);
                                }
                                _47085 = _47086;
                            }
                            else
                            {
                                _47085 = max(_36565.x, _36565.y);
                            }
                            float _36622 = (min(max(_36563.x, _36563.y), 0.0) + _47085) - _47083;
                            float _36627 = max(_46381.y, 9.9999997473787516355514526367188e-05);
                            float _36636 = max(_36627 - abs(_36463 - _36622), 0.0) / _36627;
                            _47189 = min(_36463, _36622) - (((_36636 * _36636) * _36627) * 0.25);
                        }
                        else
                        {
                            _47189 = _36463;
                        }
                        _47188 = _47189;
                    }
                    _47187 = _47188;
                }
                _47186 = _47187;
            }
            _47185 = _47186;
        }
        float _6639 = (_47185 + _34258.y) / (max(_34258.x * 0.5, _5632 * 0.5) * 1.41421353816986083984375);
        float _36653 = sign(_6639);
        float _36655 = abs(_6639);
        float _36666 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_36655 * _36655))) * _36655)) * _36655);
        float _36669 = _36666 * _36666;
        float _36684 = clamp(_34217.w * ((0.5 + (0.5 * (_36653 - (_36653 / (_36669 * _36669))))) * _42238), 0.0, 1.0);
        vec3 _36688 = _34217.xyz * _36684;
        _47365 = vec4(_36688, _36684) + (_47322 * (1.0 - _36684));
    }
    else
    {
        _47365 = _47322;
    }
    vec4 _47422;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _4931) != 0u)
    {
        uint _36709 = max(uint(_292.gConv.z), 1u);
        vec4 _36743 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _36709) * 24u) + 9u), int(esia_v1 / _36709), 0).xy, 0);
        uint _36750 = max(uint(_292.gConv.z), 1u);
        vec4 _36784 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _36750) * 24u) + 10u), int(esia_v1 / _36750), 0).xy, 0);
        float _6671 = max(-_41577, 0.0) / max(_36784.z, 0.001000000047497451305389404296875);
        float _36792 = clamp(_36743.w * clamp((exp(((-_6671) * _6671) * 2.2000000476837158203125) * _36784.w) * _42238, 0.0, 1.0), 0.0, 1.0);
        _47422 = vec4(_36743.xyz * _36792, _36792) + (_47365 * (1.0 - _36792));
    }
    else
    {
        _47422 = _47365;
    }
    vec3 _6702 = _47422.xyz + vec3(_47370 * clamp(_47422.w / max(_42238, 0.001000000047497451305389404296875), 0.0, 1.0));
    vec4 _41354 = _47422;
    _41354.x = _6702.x;
    _41354.y = _6702.y;
    _41354.z = _6702.z;
    vec4 _47866;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _5000) != 0u)
    {
        uint _36817 = max(uint(_292.gConv.z), 1u);
        vec4 _36851 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _36817) * 24u) + 5u), int(esia_v1 / _36817), 0).xy, 0);
        uint _36858 = max(uint(_292.gConv.z), 1u);
        vec4 _36892 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _36858) * 24u) + 6u), int(esia_v1 / _36858), 0).xy, 0);
        float _6722 = _36892.x;
        float _6724 = _36892.y;
        vec4 _47862;
        if (_36892.z < 0.999000012874603271484375)
        {
            vec2 _6764 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            float _6775 = cos(_36892.w);
            float _6778 = sin(_36892.w);
            vec4 _41371 = _36851;
            _41371.w = _36851.w * mix(1.0, _36892.z, clamp(((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_6775, _6778)) / max(0.5 * ((abs(_6775) * _6764.x) + (abs(_6778) * _6764.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _47862 = _41371;
        }
        else
        {
            _47862 = _36851;
        }
        float _36900 = clamp(_47862.w * ((_47433 >= 0.0) ? _47433 : (clamp(0.5 - ((_41577 - (_6722 * _6724)) / _5632), 0.0, 1.0) - clamp(0.5 - ((_41577 + (_6722 * (1.0 - _6724))) / _5632), 0.0, 1.0))), 0.0, 1.0);
        _47866 = vec4(_47862.xyz * _36900, _36900) + (_41354 * (1.0 - _36900));
    }
    else
    {
        _47866 = _41354;
    }
    vec4 _47867;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _5133) != 0u)
    {
        vec2 _6839 = (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        float _6861 = exp(-pow((((_6839.x * 0.85000002384185791015625) + (_6839.y * 0.1500000059604644775390625)) - ((fract(_292.gTime.x * esia_v6.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _47867 = vec4(_47866.xyz + vec3(((_6861 * esia_v6.z) * _42238) * max(_47866.w, 0.3499999940395355224609375)), max(_47866.w, ((_6861 * esia_v6.z) * _42238) * 0.5));
    }
    else
    {
        _47867 = _47866;
    }
    vec4 _48310;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _5215) != 0u)
    {
        vec3 _36925 = fract(floor(_41461).xyx * 0.103100001811981201171875);
        vec3 _36934 = _36925 + vec3(dot(_36925, _36925.yzx + vec3(33.3300018310546875)));
        vec3 _6912 = _47867.xyz + vec3(((fract((_36934.x + _36934.y) * _36934.z) - 0.5) * esia_v6.y) * _47867.w);
        vec4 _41395 = _47867;
        _41395.x = _6912.x;
        _41395.y = _6912.y;
        _41395.z = _6912.z;
        _48310 = _41395;
    }
    else
    {
        _48310 = _47867;
    }
    float _48306;
    if (_334.gFade.z > 0.0)
    {
        _48306 = smoothstep(0.0, 1.0, clamp((_41461.y - _334.gFade.x) / _334.gFade.z, 0.0, 1.0));
    }
    else
    {
        _48306 = 1.0;
    }
    float _48307;
    if (_334.gFade.w > 0.0)
    {
        _48307 = _48306 * smoothstep(0.0, 1.0, clamp((_334.gFade.y - _41461.y) / _334.gFade.w, 0.0, 1.0));
    }
    else
    {
        _48307 = _48306;
    }
    vec4 _6929 = _48310 * ((esia_v6.x * _47879) * _48307);
    vec4 _48311;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & _5255) != 0u) || ((esia_v7.x & _5259) != 0u))
    {
        vec3 _36986 = fract((floor(_41461) + vec2(17.0)).xyx * 0.103100001811981201171875);
        vec3 _36995 = _36986 + vec3(dot(_36986, _36986.yzx + vec3(33.3300018310546875)));
        vec3 _6953 = _6929.xyz + vec3(((fract((_36995.x + _36995.y) * _36995.z) - 0.5) * 0.0039215688593685626983642578125) * clamp(_6929.w * 8.0, 0.0, 1.0));
        vec4 _41407 = _6929;
        _41407.x = _6953.x;
        _41407.y = _6953.y;
        _41407.z = _6953.z;
        _48311 = _41407;
    }
    else
    {
        _48311 = _6929;
    }
    vec3 _6963 = max(_48311.xyz, vec3(0.0));
    vec4 _41413 = _48311;
    _41413.x = _6963.x;
    _41413.y = _6963.y;
    _41413.z = _6963.z;
    vec4 _48312;
    if ((_292.gTime.w > 0.5) && (_48311.w > 9.9999997473787516355514526367188e-06))
    {
        vec3 _37039 = clamp(_41413.xyz / vec3(_48311.w), vec3(0.0), vec3(1.0));
        vec3 _37045 = pow((_37039 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _37048 = _37039 * vec3(0.077399380505084991455078125);
        bvec3 _37050 = lessThanEqual(_37039, vec3(0.040449999272823333740234375));
        vec3 _37025 = vec3(_37050.x ? _37048.x : _37045.x, _37050.y ? _37048.y : _37045.y, _37050.z ? _37048.z : _37045.z) * _48311.w;
        vec4 _41422 = _41413;
        _41422.x = _37025.x;
        _41422.y = _37025.y;
        _41422.z = _37025.z;
        _48312 = _41422;
    }
    else
    {
        _48312 = _41413;
    }
    _entryPointOutput = _48312;
}

