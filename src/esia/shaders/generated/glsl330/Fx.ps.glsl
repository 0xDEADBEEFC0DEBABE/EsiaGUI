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

const vec3 _1700[6] = vec3[](vec3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), vec3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), vec3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), vec3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), vec3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), vec3(0.660000026226043701171875, 0.5, 1.0));
const vec4 _2996[4] = vec4[](vec4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), vec4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), vec4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), vec4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875));
const vec2 _3013[4] = vec2[](vec2(0.0), vec2(0.100000001490116119384765625, 1.2999999523162841796875), vec2(0.550000011920928955078125, 3.900000095367431640625), vec2(0.20000000298023223876953125, 5.19999980926513671875));

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _178;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _221;

uniform sampler2D SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop0gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop1gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop2gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop3gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop4gLinear;
uniform sampler2D SPIRV_Cross_CombinedgBackdrop5gLinear;
uniform sampler2D SPIRV_Cross_CombinedgTexgLinear;

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
    vec2 _5171 = gl_FragCoord.xy + _178.gConv.yy;
    vec2 _23020;
    if (_178.gConv.x > 0.5)
    {
        vec2 _21328 = _5171;
        _21328.y = _178.gTarget.y - _5171.y;
        _23020 = _21328;
    }
    else
    {
        _23020 = _5171;
    }
    float _3963 = dFdx(esia_v0.x);
    float _3967 = dFdy(esia_v0.x);
    float _3970 = max(abs(_3963) + abs(_3967), 9.9999997473787516355514526367188e-05);
    vec4 _23021;
    vec4 _23023;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.z == 2u) || ((esia_v7.x & 512u) != 0u))
    {
        uint _5190 = max(uint(_178.gConv.z), 1u);
        uint _5231 = max(uint(_178.gConv.z), 1u);
        _23023 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5190) * 24u) + 15u), int(esia_v1 / _5190), 0).xy, 0);
        _23021 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5231) * 24u) + 16u), int(esia_v1 / _5231), 0).xy, 0);
    }
    else
    {
        _23023 = vec4(0.0);
        _23021 = vec4(0.0);
    }
    vec2 _5304 = (esia_v2.xy + esia_v2.zw) * 0.5;
    vec2 _5313 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
    float _23041;
    SPIRV_CROSS_BRANCH
    if (esia_v7.z == 1u)
    {
        vec2 _5319 = esia_v0 - _5304;
        float _23040;
        do
        {
            if (esia_v5.w >= 6.282185077667236328125)
            {
                _23040 = abs(length(_5319) - esia_v3.x) - esia_v3.y;
                break;
            }
            float _5418 = esia_v5.z + (esia_v5.w * 0.5);
            float _5420 = cos(_5418);
            float _5422 = sin(_5418);
            float _5431 = dot(_5319, vec2(-_5422, _5420));
            float _5434 = dot(_5319, vec2(_5420, _5422));
            vec2 _5435 = vec2(_5431, _5434);
            float _5438 = abs(_5431);
            _5435.x = _5438;
            float _5441 = esia_v5.w * 0.5;
            float _5443 = sin(_5441);
            float _5445 = cos(_5441);
            _23040 = (((_5445 * _5438) > (_5443 * _5434)) ? length(_5435 - (vec2(_5443, _5445) * esia_v3.x)) : abs(length(_5435) - esia_v3.x)) - esia_v3.y;
            break;
        } while(false);
        _23041 = _23040;
    }
    else
    {
        float _23042;
        if (esia_v7.z == 2u)
        {
            vec2 _5481 = esia_v0 - _23023.xy;
            vec2 _5484 = _23023.zw - _23023.xy;
            _23042 = length(_5481 - (_5484 * clamp(dot(_5481, _5484) / max(dot(_5484, _5484), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
        }
        else
        {
            vec2 _5346 = esia_v0 - _5304;
            float _5537 = min(_5313.x, _5313.y);
            float _5540 = min((_5346.x > 0.0) ? ((_5346.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_5346.y > 0.0) ? esia_v3.w : esia_v3.x), _5537);
            float _5546 = _5540 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
            float _23025;
            float _23026;
            if (_5546 > _5537)
            {
                float _5560 = esia_v5.y * clamp((_5537 - _5540) / max(0.60000002384185791015625 * _5540, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _23026 = _5560;
                _23025 = _5540 * (1.0 + (0.60000002384185791015625 * _5560));
            }
            else
            {
                _23026 = esia_v5.y;
                _23025 = _5546;
            }
            vec2 _5573 = (abs(_5346) - _5313) + vec2(_23025);
            vec2 _5575 = max(_5573, vec2(0.0));
            float _23027;
            SPIRV_CROSS_BRANCH
            if ((_5575.x > 0.0) && (_5575.y > 0.0))
            {
                float _23028;
                if ((_23026 > 0.001000000047497451305389404296875) && (_23025 > 9.9999997473787516355514526367188e-05))
                {
                    float _5592 = 2.0 + (2.0 * _23026);
                    vec2 _5597 = _5575 / vec2(max(_23025, 9.9999997473787516355514526367188e-05));
                    _23028 = pow(pow(_5597.x, _5592) + pow(_5597.y, _5592), 1.0 / _5592) * _23025;
                }
                else
                {
                    _23028 = length(_5575);
                }
                _23027 = _23028;
            }
            else
            {
                _23027 = max(_5575.x, _5575.y);
            }
            float _5632 = (min(max(_5573.x, _5573.y), 0.0) + _23027) - _23025;
            float _23043;
            SPIRV_CROSS_BRANCH
            if ((esia_v7.x & 512u) != 0u)
            {
                vec2 _5374 = max((_23023.zw - _23023.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                vec2 _5377 = esia_v0 - ((_23023.xy + _23023.zw) * 0.5);
                float _5668 = min(_5374.x, _5374.y);
                float _5671 = min((_5377.x > 0.0) ? ((_5377.y > 0.0) ? _23021.x : _23021.x) : ((_5377.y > 0.0) ? _23021.x : _23021.x), _5668);
                float _5677 = _5671 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _23031;
                float _23032;
                if (_5677 > _5668)
                {
                    float _5691 = esia_v5.y * clamp((_5668 - _5671) / max(0.60000002384185791015625 * _5671, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _23032 = _5691;
                    _23031 = _5671 * (1.0 + (0.60000002384185791015625 * _5691));
                }
                else
                {
                    _23032 = esia_v5.y;
                    _23031 = _5677;
                }
                vec2 _5704 = (abs(_5377) - _5374) + vec2(_23031);
                vec2 _5706 = max(_5704, vec2(0.0));
                float _23033;
                SPIRV_CROSS_BRANCH
                if ((_5706.x > 0.0) && (_5706.y > 0.0))
                {
                    float _23034;
                    if ((_23032 > 0.001000000047497451305389404296875) && (_23031 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5723 = 2.0 + (2.0 * _23032);
                        vec2 _5728 = _5706 / vec2(max(_23031, 9.9999997473787516355514526367188e-05));
                        _23034 = pow(pow(_5728.x, _5723) + pow(_5728.y, _5723), 1.0 / _5723) * _23031;
                    }
                    else
                    {
                        _23034 = length(_5706);
                    }
                    _23033 = _23034;
                }
                else
                {
                    _23033 = max(_5706.x, _5706.y);
                }
                float _5763 = (min(max(_5704.x, _5704.y), 0.0) + _23033) - _23031;
                float _5768 = max(_23021.y, 9.9999997473787516355514526367188e-05);
                float _5777 = max(_5768 - abs(_5632 - _5763), 0.0) / _5768;
                _23043 = min(_5632, _5763) - (((_5777 * _5777) * _5768) * 0.25);
            }
            else
            {
                _23043 = _5632;
            }
            _23042 = _23043;
        }
        _23041 = _23042;
    }
    float _3995 = clamp(0.5 - (_23041 / _3970), 0.0, 1.0);
    float _26183;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 1024u) != 0u)
    {
        uint _5794 = max(uint(_178.gConv.z), 1u);
        vec4 _5828 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5794) * 24u) + 19u), int(esia_v1 / _5794), 0).xy, 0);
        uint _5835 = max(uint(_178.gConv.z), 1u);
        vec4 _5869 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5835) * 24u) + 20u), int(esia_v1 / _5835), 0).xy, 0);
        vec2 _4024 = max((_5828.zw - _5828.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        vec2 _4027 = esia_v0 - ((_5828.xy + _5828.zw) * 0.5);
        float _4033 = _5869.y;
        float _5905 = min(_4024.x, _4024.y);
        float _5908 = min((_4027.x > 0.0) ? ((_4027.y > 0.0) ? _5869.x : _5869.x) : ((_4027.y > 0.0) ? _5869.x : _5869.x), _5905);
        float _5914 = _5908 * (1.0 + (0.60000002384185791015625 * _4033));
        float _23044;
        float _23045;
        if (_5914 > _5905)
        {
            float _5928 = _4033 * clamp((_5905 - _5908) / max(0.60000002384185791015625 * _5908, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _23045 = _5928;
            _23044 = _5908 * (1.0 + (0.60000002384185791015625 * _5928));
        }
        else
        {
            _23045 = _4033;
            _23044 = _5914;
        }
        vec2 _5941 = (abs(_4027) - _4024) + vec2(_23044);
        vec2 _5943 = max(_5941, vec2(0.0));
        float _23046;
        SPIRV_CROSS_BRANCH
        if ((_5943.x > 0.0) && (_5943.y > 0.0))
        {
            float _23047;
            if ((_23045 > 0.001000000047497451305389404296875) && (_23044 > 9.9999997473787516355514526367188e-05))
            {
                float _5960 = 2.0 + (2.0 * _23045);
                vec2 _5965 = _5943 / vec2(max(_23044, 9.9999997473787516355514526367188e-05));
                _23047 = pow(pow(_5965.x, _5960) + pow(_5965.y, _5960), 1.0 / _5960) * _23044;
            }
            else
            {
                _23047 = length(_5943);
            }
            _23046 = _23047;
        }
        else
        {
            _23046 = max(_5943.x, _5943.y);
        }
        float _4039 = clamp(0.5 - (((min(max(_5941.x, _5941.y), 0.0) + _23046) - _23044) / _3970), 0.0, 1.0);
        if (_4039 <= 0.0)
        {
            discard;
        }
        _26183 = _4039;
    }
    else
    {
        _26183 = 1.0;
    }
    bool _4050 = ((esia_v7.x & 32u) != 0u) && (_3995 >= 0.999000012874603271484375);
    vec4 _23136;
    SPIRV_CROSS_BRANCH
    if ((((esia_v7.x & 4u) != 0u) && (!((esia_v7.x & 256u) != 0u))) && (!_4050))
    {
        uint _6007 = max(uint(_178.gConv.z), 1u);
        vec4 _6041 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6007) * 24u) + 7u), int(esia_v1 / _6007), 0).xy, 0);
        uint _6048 = max(uint(_178.gConv.z), 1u);
        vec4 _6082 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6048) * 24u) + 8u), int(esia_v1 / _6048), 0).xy, 0);
        vec2 _4081 = esia_v0 - _6082.zw;
        vec2 _6121 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _6130 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _23094;
        SPIRV_CROSS_BRANCH
        if (esia_v7.z == 1u)
        {
            vec2 _6136 = _4081 - _6121;
            float _23093;
            do
            {
                if (esia_v5.w >= 6.282185077667236328125)
                {
                    _23093 = abs(length(_6136) - esia_v3.x) - esia_v3.y;
                    break;
                }
                float _6235 = esia_v5.z + (esia_v5.w * 0.5);
                float _6237 = cos(_6235);
                float _6239 = sin(_6235);
                float _6248 = dot(_6136, vec2(-_6239, _6237));
                float _6251 = dot(_6136, vec2(_6237, _6239));
                vec2 _6252 = vec2(_6248, _6251);
                float _6255 = abs(_6248);
                _6252.x = _6255;
                float _6258 = esia_v5.w * 0.5;
                float _6260 = sin(_6258);
                float _6262 = cos(_6258);
                _23093 = (((_6262 * _6255) > (_6260 * _6251)) ? length(_6252 - (vec2(_6260, _6262) * esia_v3.x)) : abs(length(_6252) - esia_v3.x)) - esia_v3.y;
                break;
            } while(false);
            _23094 = _23093;
        }
        else
        {
            float _23095;
            if (esia_v7.z == 2u)
            {
                vec2 _6298 = _4081 - _23023.xy;
                vec2 _6301 = _23023.zw - _23023.xy;
                _23095 = length(_6298 - (_6301 * clamp(dot(_6298, _6301) / max(dot(_6301, _6301), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
            }
            else
            {
                vec2 _6163 = _4081 - _6121;
                float _6354 = min(_6130.x, _6130.y);
                float _6357 = min((_6163.x > 0.0) ? ((_6163.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_6163.y > 0.0) ? esia_v3.w : esia_v3.x), _6354);
                float _6363 = _6357 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _23078;
                float _23079;
                if (_6363 > _6354)
                {
                    float _6377 = esia_v5.y * clamp((_6354 - _6357) / max(0.60000002384185791015625 * _6357, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _23079 = _6377;
                    _23078 = _6357 * (1.0 + (0.60000002384185791015625 * _6377));
                }
                else
                {
                    _23079 = esia_v5.y;
                    _23078 = _6363;
                }
                vec2 _6390 = (abs(_6163) - _6130) + vec2(_23078);
                vec2 _6392 = max(_6390, vec2(0.0));
                float _23080;
                SPIRV_CROSS_BRANCH
                if ((_6392.x > 0.0) && (_6392.y > 0.0))
                {
                    float _23081;
                    if ((_23079 > 0.001000000047497451305389404296875) && (_23078 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6409 = 2.0 + (2.0 * _23079);
                        vec2 _6414 = _6392 / vec2(max(_23078, 9.9999997473787516355514526367188e-05));
                        _23081 = pow(pow(_6414.x, _6409) + pow(_6414.y, _6409), 1.0 / _6409) * _23078;
                    }
                    else
                    {
                        _23081 = length(_6392);
                    }
                    _23080 = _23081;
                }
                else
                {
                    _23080 = max(_6392.x, _6392.y);
                }
                float _6449 = (min(max(_6390.x, _6390.y), 0.0) + _23080) - _23078;
                float _23096;
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & 512u) != 0u)
                {
                    vec2 _6191 = max((_23023.zw - _23023.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _6194 = _4081 - ((_23023.xy + _23023.zw) * 0.5);
                    float _6485 = min(_6191.x, _6191.y);
                    float _6488 = min((_6194.x > 0.0) ? ((_6194.y > 0.0) ? _23021.x : _23021.x) : ((_6194.y > 0.0) ? _23021.x : _23021.x), _6485);
                    float _6494 = _6488 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _23084;
                    float _23085;
                    if (_6494 > _6485)
                    {
                        float _6508 = esia_v5.y * clamp((_6485 - _6488) / max(0.60000002384185791015625 * _6488, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23085 = _6508;
                        _23084 = _6488 * (1.0 + (0.60000002384185791015625 * _6508));
                    }
                    else
                    {
                        _23085 = esia_v5.y;
                        _23084 = _6494;
                    }
                    vec2 _6521 = (abs(_6194) - _6191) + vec2(_23084);
                    vec2 _6523 = max(_6521, vec2(0.0));
                    float _23086;
                    SPIRV_CROSS_BRANCH
                    if ((_6523.x > 0.0) && (_6523.y > 0.0))
                    {
                        float _23087;
                        if ((_23085 > 0.001000000047497451305389404296875) && (_23084 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6540 = 2.0 + (2.0 * _23085);
                            vec2 _6545 = _6523 / vec2(max(_23084, 9.9999997473787516355514526367188e-05));
                            _23087 = pow(pow(_6545.x, _6540) + pow(_6545.y, _6540), 1.0 / _6540) * _23084;
                        }
                        else
                        {
                            _23087 = length(_6523);
                        }
                        _23086 = _23087;
                    }
                    else
                    {
                        _23086 = max(_6523.x, _6523.y);
                    }
                    float _6580 = (min(max(_6521.x, _6521.y), 0.0) + _23086) - _23084;
                    float _6585 = max(_23021.y, 9.9999997473787516355514526367188e-05);
                    float _6594 = max(_6585 - abs(_6449 - _6580), 0.0) / _6585;
                    _23096 = min(_6449, _6580) - (((_6594 * _6594) * _6585) * 0.25);
                }
                else
                {
                    _23096 = _6449;
                }
                _23095 = _23096;
            }
            _23094 = _23095;
        }
        float _4090 = (_23094 - _6082.y) / (max(_6082.x * 0.5, _3970 * 0.5) * 1.41421353816986083984375);
        float _6611 = sign(_4090);
        float _6613 = abs(_4090);
        float _6624 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6613 * _6613))) * _6613)) * _6613);
        float _6627 = _6624 * _6624;
        float _6642 = clamp(_6041.w * (0.5 - (0.5 * (_6611 - (_6611 / (_6627 * _6627))))), 0.0, 1.0);
        _23136 = vec4(_6041.xyz * _6642, _6642);
    }
    else
    {
        _23136 = vec4(0.0);
    }
    vec4 _24579;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 8u) != 0u) && (!_4050))
    {
        uint _6667 = max(uint(_178.gConv.z), 1u);
        vec4 _6701 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6667) * 24u) + 9u), int(esia_v1 / _6667), 0).xy, 0);
        uint _6708 = max(uint(_178.gConv.z), 1u);
        vec4 _6742 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6708) * 24u) + 10u), int(esia_v1 / _6708), 0).xy, 0);
        float _4118 = max(_6742.x, 0.001000000047497451305389404296875);
        float _23129;
        float _23132;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & 8192u) != 0u)
        {
            uint _6749 = max(uint(_178.gConv.z), 1u);
            vec4 _6783 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6749) * 24u) + 22u), int(esia_v1 / _6749), 0).xy, 0);
            vec2 _4134 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _4143 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            vec2 _4150 = esia_v2.xy - _6783.xy;
            vec2 _4157 = _6783.zw - esia_v2.zw;
            vec2 _4187 = clamp(vec2((esia_v0.x < _4134.x) ? _4150.x : _4157.x, (esia_v0.y < _4134.y) ? _4150.y : _4157.y) * vec2(0.58823525905609130859375), vec2(min(_4118, 1.5)), vec2(_4118));
            float _4192 = min(_4143.x, _4143.y);
            float _23127;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 0u)
            {
                _23127 = min(((esia_v0.x > _4134.x) ? ((esia_v0.y > _4134.y) ? esia_v3.z : esia_v3.y) : ((esia_v0.y > _4134.y) ? esia_v3.w : esia_v3.x)) * (1.0 + (0.60000002384185791015625 * esia_v5.y)), _4192);
            }
            else
            {
                _23127 = _4192;
            }
            vec2 _4241 = max(abs(esia_v0 - _4134) - (_4143 - vec2(_23127)), vec2(0.0));
            float _4243 = length(_4241);
            vec2 _4249 = _4241 / vec2(_4243);
            bvec2 _4250 = bvec2(_4243 > 9.9999997473787516355514526367188e-05);
            vec2 _4251 = vec2(_4250.x ? _4249.x : vec2(0.707099974155426025390625).x, _4250.y ? _4249.y : vec2(0.707099974155426025390625).y);
            vec2 _4266 = esia_v0 - _6783.xy;
            vec2 _4271 = _6783.zw - esia_v0;
            _23132 = clamp(min(min(_4266.x, _4266.y), min(_4271.x, _4271.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _23129 = inversesqrt(dot(_4251 * _4251, vec2(1.0) / (_4187 * _4187)));
        }
        else
        {
            _23132 = 1.0;
            _23129 = _4118;
        }
        float _4289 = max(_23041, 0.0) / _23129;
        float _6791 = clamp(_6701.w * clamp((exp(((-_4289) * _4289) * 2.2000000476837158203125) * _6742.y) * _23132, 0.0, 1.0), 0.0, 1.0);
        _24579 = vec4(_6701.xyz * _6791, _6791) + (_23136 * (1.0 - _6791));
    }
    else
    {
        _24579 = _23136;
    }
    vec4 _25489;
    vec4 _25502;
    float _26149;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 32u) != 0u) && (_3995 > 0.0))
    {
        uint _6816 = max(uint(_178.gConv.z), 1u);
        vec4 _6850 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6816) * 24u) + 11u), int(esia_v1 / _6816), 0).xy, 0);
        uint _6857 = max(uint(_178.gConv.z), 1u);
        vec4 _6891 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6857) * 24u) + 12u), int(esia_v1 / _6857), 0).xy, 0);
        uint _6898 = max(uint(_178.gConv.z), 1u);
        vec4 _6932 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6898) * 24u) + 13u), int(esia_v1 / _6898), 0).xy, 0);
        uint _6939 = max(uint(_178.gConv.z), 1u);
        vec4 _6973 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6939) * 24u) + 16u), int(esia_v1 / _6939), 0).xy, 0);
        float _7038 = _6850.x * _178.gDisplay.z;
        float _7040 = _6850.y;
        vec2 _7049 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(1.0));
        float _7057 = clamp(_6850.z, 0.001000000047497451305389404296875, min(_7049.x, _7049.y));
        float _7059 = _6850.w;
        float _7066 = clamp(1.0 - (max(-_23041, 0.0) / _7057), 0.0, 1.0);
        float _7072 = sqrt(clamp(1.0 - (_7066 * _7066), 0.0, 1.0));
        vec2 _23228;
        vec3 _24090;
        SPIRV_CROSS_BRANCH
        if (_7066 > 0.0)
        {
            vec2 _7454 = esia_v0 + vec2(0.5, 0.0);
            vec2 _7522 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _7531 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _23168;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _7537 = _7454 - _7522;
                float _23167;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _23167 = abs(length(_7537) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _7636 = esia_v5.z + (esia_v5.w * 0.5);
                    float _7638 = cos(_7636);
                    float _7640 = sin(_7636);
                    float _7649 = dot(_7537, vec2(-_7640, _7638));
                    float _7652 = dot(_7537, vec2(_7638, _7640));
                    vec2 _7653 = vec2(_7649, _7652);
                    float _7656 = abs(_7649);
                    _7653.x = _7656;
                    float _7659 = esia_v5.w * 0.5;
                    float _7661 = sin(_7659);
                    float _7663 = cos(_7659);
                    _23167 = (((_7663 * _7656) > (_7661 * _7652)) ? length(_7653 - (vec2(_7661, _7663) * esia_v3.x)) : abs(length(_7653) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _23168 = _23167;
            }
            else
            {
                float _23169;
                if (esia_v7.z == 2u)
                {
                    vec2 _7699 = _7454 - _23023.xy;
                    vec2 _7702 = _23023.zw - _23023.xy;
                    _23169 = length(_7699 - (_7702 * clamp(dot(_7699, _7702) / max(dot(_7702, _7702), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _7564 = _7454 - _7522;
                    float _7755 = min(_7531.x, _7531.y);
                    float _7758 = min((_7564.x > 0.0) ? ((_7564.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_7564.y > 0.0) ? esia_v3.w : esia_v3.x), _7755);
                    float _7764 = _7758 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _23152;
                    float _23153;
                    if (_7764 > _7755)
                    {
                        float _7778 = esia_v5.y * clamp((_7755 - _7758) / max(0.60000002384185791015625 * _7758, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23153 = _7778;
                        _23152 = _7758 * (1.0 + (0.60000002384185791015625 * _7778));
                    }
                    else
                    {
                        _23153 = esia_v5.y;
                        _23152 = _7764;
                    }
                    vec2 _7791 = (abs(_7564) - _7531) + vec2(_23152);
                    vec2 _7793 = max(_7791, vec2(0.0));
                    float _23154;
                    SPIRV_CROSS_BRANCH
                    if ((_7793.x > 0.0) && (_7793.y > 0.0))
                    {
                        float _23155;
                        if ((_23153 > 0.001000000047497451305389404296875) && (_23152 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7810 = 2.0 + (2.0 * _23153);
                            vec2 _7815 = _7793 / vec2(max(_23152, 9.9999997473787516355514526367188e-05));
                            _23155 = pow(pow(_7815.x, _7810) + pow(_7815.y, _7810), 1.0 / _7810) * _23152;
                        }
                        else
                        {
                            _23155 = length(_7793);
                        }
                        _23154 = _23155;
                    }
                    else
                    {
                        _23154 = max(_7793.x, _7793.y);
                    }
                    float _7850 = (min(max(_7791.x, _7791.y), 0.0) + _23154) - _23152;
                    float _23170;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _7592 = max((_23023.zw - _23023.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _7595 = _7454 - ((_23023.xy + _23023.zw) * 0.5);
                        float _7886 = min(_7592.x, _7592.y);
                        float _7889 = min((_7595.x > 0.0) ? ((_7595.y > 0.0) ? _6973.x : _6973.x) : ((_7595.y > 0.0) ? _6973.x : _6973.x), _7886);
                        float _7895 = _7889 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _23158;
                        float _23159;
                        if (_7895 > _7886)
                        {
                            float _7909 = esia_v5.y * clamp((_7886 - _7889) / max(0.60000002384185791015625 * _7889, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _23159 = _7909;
                            _23158 = _7889 * (1.0 + (0.60000002384185791015625 * _7909));
                        }
                        else
                        {
                            _23159 = esia_v5.y;
                            _23158 = _7895;
                        }
                        vec2 _7922 = (abs(_7595) - _7592) + vec2(_23158);
                        vec2 _7924 = max(_7922, vec2(0.0));
                        float _23160;
                        SPIRV_CROSS_BRANCH
                        if ((_7924.x > 0.0) && (_7924.y > 0.0))
                        {
                            float _23161;
                            if ((_23159 > 0.001000000047497451305389404296875) && (_23158 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7941 = 2.0 + (2.0 * _23159);
                                vec2 _7946 = _7924 / vec2(max(_23158, 9.9999997473787516355514526367188e-05));
                                _23161 = pow(pow(_7946.x, _7941) + pow(_7946.y, _7941), 1.0 / _7941) * _23158;
                            }
                            else
                            {
                                _23161 = length(_7924);
                            }
                            _23160 = _23161;
                        }
                        else
                        {
                            _23160 = max(_7924.x, _7924.y);
                        }
                        float _7981 = (min(max(_7922.x, _7922.y), 0.0) + _23160) - _23158;
                        float _7986 = max(_6973.y, 9.9999997473787516355514526367188e-05);
                        float _7995 = max(_7986 - abs(_7850 - _7981), 0.0) / _7986;
                        _23170 = min(_7850, _7981) - (((_7995 * _7995) * _7986) * 0.25);
                    }
                    else
                    {
                        _23170 = _7850;
                    }
                    _23169 = _23170;
                }
                _23168 = _23169;
            }
            vec2 _7458 = esia_v0 - vec2(0.5, 0.0);
            vec2 _8044 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _8053 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _23187;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _8059 = _7458 - _8044;
                float _23186;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _23186 = abs(length(_8059) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _8158 = esia_v5.z + (esia_v5.w * 0.5);
                    float _8160 = cos(_8158);
                    float _8162 = sin(_8158);
                    float _8171 = dot(_8059, vec2(-_8162, _8160));
                    float _8174 = dot(_8059, vec2(_8160, _8162));
                    vec2 _8175 = vec2(_8171, _8174);
                    float _8178 = abs(_8171);
                    _8175.x = _8178;
                    float _8181 = esia_v5.w * 0.5;
                    float _8183 = sin(_8181);
                    float _8185 = cos(_8181);
                    _23186 = (((_8185 * _8178) > (_8183 * _8174)) ? length(_8175 - (vec2(_8183, _8185) * esia_v3.x)) : abs(length(_8175) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _23187 = _23186;
            }
            else
            {
                float _23188;
                if (esia_v7.z == 2u)
                {
                    vec2 _8221 = _7458 - _23023.xy;
                    vec2 _8224 = _23023.zw - _23023.xy;
                    _23188 = length(_8221 - (_8224 * clamp(dot(_8221, _8224) / max(dot(_8224, _8224), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _8086 = _7458 - _8044;
                    float _8277 = min(_8053.x, _8053.y);
                    float _8280 = min((_8086.x > 0.0) ? ((_8086.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_8086.y > 0.0) ? esia_v3.w : esia_v3.x), _8277);
                    float _8286 = _8280 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _23171;
                    float _23172;
                    if (_8286 > _8277)
                    {
                        float _8300 = esia_v5.y * clamp((_8277 - _8280) / max(0.60000002384185791015625 * _8280, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23172 = _8300;
                        _23171 = _8280 * (1.0 + (0.60000002384185791015625 * _8300));
                    }
                    else
                    {
                        _23172 = esia_v5.y;
                        _23171 = _8286;
                    }
                    vec2 _8313 = (abs(_8086) - _8053) + vec2(_23171);
                    vec2 _8315 = max(_8313, vec2(0.0));
                    float _23173;
                    SPIRV_CROSS_BRANCH
                    if ((_8315.x > 0.0) && (_8315.y > 0.0))
                    {
                        float _23174;
                        if ((_23172 > 0.001000000047497451305389404296875) && (_23171 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8332 = 2.0 + (2.0 * _23172);
                            vec2 _8337 = _8315 / vec2(max(_23171, 9.9999997473787516355514526367188e-05));
                            _23174 = pow(pow(_8337.x, _8332) + pow(_8337.y, _8332), 1.0 / _8332) * _23171;
                        }
                        else
                        {
                            _23174 = length(_8315);
                        }
                        _23173 = _23174;
                    }
                    else
                    {
                        _23173 = max(_8315.x, _8315.y);
                    }
                    float _8372 = (min(max(_8313.x, _8313.y), 0.0) + _23173) - _23171;
                    float _23189;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _8114 = max((_23023.zw - _23023.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _8117 = _7458 - ((_23023.xy + _23023.zw) * 0.5);
                        float _8408 = min(_8114.x, _8114.y);
                        float _8411 = min((_8117.x > 0.0) ? ((_8117.y > 0.0) ? _6973.x : _6973.x) : ((_8117.y > 0.0) ? _6973.x : _6973.x), _8408);
                        float _8417 = _8411 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _23177;
                        float _23178;
                        if (_8417 > _8408)
                        {
                            float _8431 = esia_v5.y * clamp((_8408 - _8411) / max(0.60000002384185791015625 * _8411, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _23178 = _8431;
                            _23177 = _8411 * (1.0 + (0.60000002384185791015625 * _8431));
                        }
                        else
                        {
                            _23178 = esia_v5.y;
                            _23177 = _8417;
                        }
                        vec2 _8444 = (abs(_8117) - _8114) + vec2(_23177);
                        vec2 _8446 = max(_8444, vec2(0.0));
                        float _23179;
                        SPIRV_CROSS_BRANCH
                        if ((_8446.x > 0.0) && (_8446.y > 0.0))
                        {
                            float _23180;
                            if ((_23178 > 0.001000000047497451305389404296875) && (_23177 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8463 = 2.0 + (2.0 * _23178);
                                vec2 _8468 = _8446 / vec2(max(_23177, 9.9999997473787516355514526367188e-05));
                                _23180 = pow(pow(_8468.x, _8463) + pow(_8468.y, _8463), 1.0 / _8463) * _23177;
                            }
                            else
                            {
                                _23180 = length(_8446);
                            }
                            _23179 = _23180;
                        }
                        else
                        {
                            _23179 = max(_8446.x, _8446.y);
                        }
                        float _8503 = (min(max(_8444.x, _8444.y), 0.0) + _23179) - _23177;
                        float _8508 = max(_6973.y, 9.9999997473787516355514526367188e-05);
                        float _8517 = max(_8508 - abs(_8372 - _8503), 0.0) / _8508;
                        _23189 = min(_8372, _8503) - (((_8517 * _8517) * _8508) * 0.25);
                    }
                    else
                    {
                        _23189 = _8372;
                    }
                    _23188 = _23189;
                }
                _23187 = _23188;
            }
            vec2 _7463 = esia_v0 + vec2(0.0, 0.5);
            vec2 _8566 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _8575 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _23206;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _8581 = _7463 - _8566;
                float _23205;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _23205 = abs(length(_8581) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _8680 = esia_v5.z + (esia_v5.w * 0.5);
                    float _8682 = cos(_8680);
                    float _8684 = sin(_8680);
                    float _8693 = dot(_8581, vec2(-_8684, _8682));
                    float _8696 = dot(_8581, vec2(_8682, _8684));
                    vec2 _8697 = vec2(_8693, _8696);
                    float _8700 = abs(_8693);
                    _8697.x = _8700;
                    float _8703 = esia_v5.w * 0.5;
                    float _8705 = sin(_8703);
                    float _8707 = cos(_8703);
                    _23205 = (((_8707 * _8700) > (_8705 * _8696)) ? length(_8697 - (vec2(_8705, _8707) * esia_v3.x)) : abs(length(_8697) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _23206 = _23205;
            }
            else
            {
                float _23207;
                if (esia_v7.z == 2u)
                {
                    vec2 _8743 = _7463 - _23023.xy;
                    vec2 _8746 = _23023.zw - _23023.xy;
                    _23207 = length(_8743 - (_8746 * clamp(dot(_8743, _8746) / max(dot(_8746, _8746), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _8608 = _7463 - _8566;
                    float _8799 = min(_8575.x, _8575.y);
                    float _8802 = min((_8608.x > 0.0) ? ((_8608.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_8608.y > 0.0) ? esia_v3.w : esia_v3.x), _8799);
                    float _8808 = _8802 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _23190;
                    float _23191;
                    if (_8808 > _8799)
                    {
                        float _8822 = esia_v5.y * clamp((_8799 - _8802) / max(0.60000002384185791015625 * _8802, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23191 = _8822;
                        _23190 = _8802 * (1.0 + (0.60000002384185791015625 * _8822));
                    }
                    else
                    {
                        _23191 = esia_v5.y;
                        _23190 = _8808;
                    }
                    vec2 _8835 = (abs(_8608) - _8575) + vec2(_23190);
                    vec2 _8837 = max(_8835, vec2(0.0));
                    float _23192;
                    SPIRV_CROSS_BRANCH
                    if ((_8837.x > 0.0) && (_8837.y > 0.0))
                    {
                        float _23193;
                        if ((_23191 > 0.001000000047497451305389404296875) && (_23190 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8854 = 2.0 + (2.0 * _23191);
                            vec2 _8859 = _8837 / vec2(max(_23190, 9.9999997473787516355514526367188e-05));
                            _23193 = pow(pow(_8859.x, _8854) + pow(_8859.y, _8854), 1.0 / _8854) * _23190;
                        }
                        else
                        {
                            _23193 = length(_8837);
                        }
                        _23192 = _23193;
                    }
                    else
                    {
                        _23192 = max(_8837.x, _8837.y);
                    }
                    float _8894 = (min(max(_8835.x, _8835.y), 0.0) + _23192) - _23190;
                    float _23208;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _8636 = max((_23023.zw - _23023.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _8639 = _7463 - ((_23023.xy + _23023.zw) * 0.5);
                        float _8930 = min(_8636.x, _8636.y);
                        float _8933 = min((_8639.x > 0.0) ? ((_8639.y > 0.0) ? _6973.x : _6973.x) : ((_8639.y > 0.0) ? _6973.x : _6973.x), _8930);
                        float _8939 = _8933 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _23196;
                        float _23197;
                        if (_8939 > _8930)
                        {
                            float _8953 = esia_v5.y * clamp((_8930 - _8933) / max(0.60000002384185791015625 * _8933, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _23197 = _8953;
                            _23196 = _8933 * (1.0 + (0.60000002384185791015625 * _8953));
                        }
                        else
                        {
                            _23197 = esia_v5.y;
                            _23196 = _8939;
                        }
                        vec2 _8966 = (abs(_8639) - _8636) + vec2(_23196);
                        vec2 _8968 = max(_8966, vec2(0.0));
                        float _23198;
                        SPIRV_CROSS_BRANCH
                        if ((_8968.x > 0.0) && (_8968.y > 0.0))
                        {
                            float _23199;
                            if ((_23197 > 0.001000000047497451305389404296875) && (_23196 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8985 = 2.0 + (2.0 * _23197);
                                vec2 _8990 = _8968 / vec2(max(_23196, 9.9999997473787516355514526367188e-05));
                                _23199 = pow(pow(_8990.x, _8985) + pow(_8990.y, _8985), 1.0 / _8985) * _23196;
                            }
                            else
                            {
                                _23199 = length(_8968);
                            }
                            _23198 = _23199;
                        }
                        else
                        {
                            _23198 = max(_8968.x, _8968.y);
                        }
                        float _9025 = (min(max(_8966.x, _8966.y), 0.0) + _23198) - _23196;
                        float _9030 = max(_6973.y, 9.9999997473787516355514526367188e-05);
                        float _9039 = max(_9030 - abs(_8894 - _9025), 0.0) / _9030;
                        _23208 = min(_8894, _9025) - (((_9039 * _9039) * _9030) * 0.25);
                    }
                    else
                    {
                        _23208 = _8894;
                    }
                    _23207 = _23208;
                }
                _23206 = _23207;
            }
            vec2 _7467 = esia_v0 - vec2(0.0, 0.5);
            vec2 _9088 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _9097 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _23225;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _9103 = _7467 - _9088;
                float _23224;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _23224 = abs(length(_9103) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _9202 = esia_v5.z + (esia_v5.w * 0.5);
                    float _9204 = cos(_9202);
                    float _9206 = sin(_9202);
                    float _9215 = dot(_9103, vec2(-_9206, _9204));
                    float _9218 = dot(_9103, vec2(_9204, _9206));
                    vec2 _9219 = vec2(_9215, _9218);
                    float _9222 = abs(_9215);
                    _9219.x = _9222;
                    float _9225 = esia_v5.w * 0.5;
                    float _9227 = sin(_9225);
                    float _9229 = cos(_9225);
                    _23224 = (((_9229 * _9222) > (_9227 * _9218)) ? length(_9219 - (vec2(_9227, _9229) * esia_v3.x)) : abs(length(_9219) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _23225 = _23224;
            }
            else
            {
                float _23226;
                if (esia_v7.z == 2u)
                {
                    vec2 _9265 = _7467 - _23023.xy;
                    vec2 _9268 = _23023.zw - _23023.xy;
                    _23226 = length(_9265 - (_9268 * clamp(dot(_9265, _9268) / max(dot(_9268, _9268), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _9130 = _7467 - _9088;
                    float _9321 = min(_9097.x, _9097.y);
                    float _9324 = min((_9130.x > 0.0) ? ((_9130.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_9130.y > 0.0) ? esia_v3.w : esia_v3.x), _9321);
                    float _9330 = _9324 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _23209;
                    float _23210;
                    if (_9330 > _9321)
                    {
                        float _9344 = esia_v5.y * clamp((_9321 - _9324) / max(0.60000002384185791015625 * _9324, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23210 = _9344;
                        _23209 = _9324 * (1.0 + (0.60000002384185791015625 * _9344));
                    }
                    else
                    {
                        _23210 = esia_v5.y;
                        _23209 = _9330;
                    }
                    vec2 _9357 = (abs(_9130) - _9097) + vec2(_23209);
                    vec2 _9359 = max(_9357, vec2(0.0));
                    float _23211;
                    SPIRV_CROSS_BRANCH
                    if ((_9359.x > 0.0) && (_9359.y > 0.0))
                    {
                        float _23212;
                        if ((_23210 > 0.001000000047497451305389404296875) && (_23209 > 9.9999997473787516355514526367188e-05))
                        {
                            float _9376 = 2.0 + (2.0 * _23210);
                            vec2 _9381 = _9359 / vec2(max(_23209, 9.9999997473787516355514526367188e-05));
                            _23212 = pow(pow(_9381.x, _9376) + pow(_9381.y, _9376), 1.0 / _9376) * _23209;
                        }
                        else
                        {
                            _23212 = length(_9359);
                        }
                        _23211 = _23212;
                    }
                    else
                    {
                        _23211 = max(_9359.x, _9359.y);
                    }
                    float _9416 = (min(max(_9357.x, _9357.y), 0.0) + _23211) - _23209;
                    float _23227;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _9158 = max((_23023.zw - _23023.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _9161 = _7467 - ((_23023.xy + _23023.zw) * 0.5);
                        float _9452 = min(_9158.x, _9158.y);
                        float _9455 = min((_9161.x > 0.0) ? ((_9161.y > 0.0) ? _6973.x : _6973.x) : ((_9161.y > 0.0) ? _6973.x : _6973.x), _9452);
                        float _9461 = _9455 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _23215;
                        float _23216;
                        if (_9461 > _9452)
                        {
                            float _9475 = esia_v5.y * clamp((_9452 - _9455) / max(0.60000002384185791015625 * _9455, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _23216 = _9475;
                            _23215 = _9455 * (1.0 + (0.60000002384185791015625 * _9475));
                        }
                        else
                        {
                            _23216 = esia_v5.y;
                            _23215 = _9461;
                        }
                        vec2 _9488 = (abs(_9161) - _9158) + vec2(_23215);
                        vec2 _9490 = max(_9488, vec2(0.0));
                        float _23217;
                        SPIRV_CROSS_BRANCH
                        if ((_9490.x > 0.0) && (_9490.y > 0.0))
                        {
                            float _23218;
                            if ((_23216 > 0.001000000047497451305389404296875) && (_23215 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9507 = 2.0 + (2.0 * _23216);
                                vec2 _9512 = _9490 / vec2(max(_23215, 9.9999997473787516355514526367188e-05));
                                _23218 = pow(pow(_9512.x, _9507) + pow(_9512.y, _9507), 1.0 / _9507) * _23215;
                            }
                            else
                            {
                                _23218 = length(_9490);
                            }
                            _23217 = _23218;
                        }
                        else
                        {
                            _23217 = max(_9490.x, _9490.y);
                        }
                        float _9547 = (min(max(_9488.x, _9488.y), 0.0) + _23217) - _23215;
                        float _9552 = max(_6973.y, 9.9999997473787516355514526367188e-05);
                        float _9561 = max(_9552 - abs(_9416 - _9547), 0.0) / _9552;
                        _23227 = min(_9416, _9547) - (((_9561 * _9561) * _9552) * 0.25);
                    }
                    else
                    {
                        _23227 = _9416;
                    }
                    _23226 = _23227;
                }
                _23225 = _23226;
            }
            vec2 _7473 = vec2(_23168 - _23187, _23206 - _23225);
            float _7475 = length(_7473);
            vec2 _7481 = _7473 / vec2(_7475);
            bvec2 _7482 = bvec2(_7475 > 9.9999997473787516355514526367188e-06);
            vec2 _7483 = vec2(_7482.x ? _7481.x : vec2(0.0, -1.0).x, _7482.y ? _7481.y : vec2(0.0, -1.0).y);
            _24090 = normalize(vec3(_7483 * min(_7066 / max(_7072, 0.001000000047497451305389404296875), 8.0), 1.0));
            _23228 = _7483;
        }
        else
        {
            _24090 = vec3(0.0, 0.0, 1.0);
            _23228 = vec2(0.0, -1.0);
        }
        vec2 _7098 = ((-_23228) * _7040) * (1.0 - _7072);
        float _7100 = _6973.z;
        vec2 _23229;
        SPIRV_CROSS_BRANCH
        if (_7100 > 0.0)
        {
            _23229 = (((esia_v2.xy + esia_v2.zw) * 0.5) - esia_v0) * (_7100 / (1.0 + _7100));
        }
        else
        {
            _23229 = vec2(0.0);
        }
        vec2 _7124 = _23020 * _178.gTarget.zw;
        vec2 _7131 = _178.gDisplay.zw * _178.gTarget.zw;
        vec2 _7136 = (_7098 + _23229) * _7131;
        vec2 _7139 = _7098 * _7131;
        vec3 _23834;
        float _23857;
        vec3 _24326;
        if (_178.gTime.z > 0.5)
        {
            vec3 _23837;
            SPIRV_CROSS_BRANCH
            if (((_7059 > 0.001000000047497451305389404296875) && (_7066 > 0.0)) && ((((_7040 * 0.300000011920928955078125) * _7059) * _178.gDisplay.z) > (_7038 * 0.3499999940395355224609375)))
            {
                float _7161 = 0.300000011920928955078125 * _7059;
                vec2 _7168 = (_7124 + _7136) - (_7139 * _7161);
                float _9586 = clamp(log2(max(_7038, 1.0)) - 1.0, 0.0, 5.0);
                int _9589 = int(floor(_9586));
                float _9593 = _9586 - float(_9589);
                vec4 _23304;
                SPIRV_CROSS_BRANCH
                if (_9589 <= 0)
                {
                    vec2 _23303;
                    if (_178.gConv.x > 0.5)
                    {
                        vec2 _21711 = _7168;
                        _21711.y = 1.0 - _7168.y;
                        _23303 = _21711;
                    }
                    else
                    {
                        _23303 = _7168;
                    }
                    _23304 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23303, 0.0);
                }
                else
                {
                    vec4 _23305;
                    if (_9589 == 1)
                    {
                        vec2 _9728 = (_7168 * _178.gLevel[1].xy) - vec2(0.5);
                        vec2 _9730 = floor(_9728);
                        vec2 _9733 = _9728 - _9730;
                        vec2 _9736 = _9733 * _9733;
                        vec2 _9739 = _9736 * _9733;
                        vec2 _9758 = (((_9739 * 3.0) - (_9736 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                        vec2 _9771 = _9739 * 0.16666667163372039794921875;
                        vec2 _9774 = (((((-_9739) + (_9736 * 3.0)) - (_9733 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9758;
                        vec2 _9777 = (((((_9739 * (-3.0)) + (_9736 * 3.0)) + (_9733 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9771;
                        vec2 _9787 = ((_9730 - vec2(0.5)) + (_9758 / _9774)) * _178.gLevel[1].zw;
                        vec2 _9797 = ((_9730 + vec2(1.5)) + (_9771 / _9777)) * _178.gLevel[1].zw;
                        vec2 _23299;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _21716 = _9787;
                            _21716.y = 1.0 - _9787.y;
                            _23299 = _21716;
                        }
                        else
                        {
                            _23299 = _9787;
                        }
                        float _9817 = _9787.y;
                        vec2 _9818 = vec2(_9797.x, _9817);
                        vec2 _23300;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _21722 = _9818;
                            _21722.y = 1.0 - _9817;
                            _23300 = _21722;
                        }
                        else
                        {
                            _23300 = _9818;
                        }
                        float _9834 = _9797.y;
                        vec2 _9835 = vec2(_9787.x, _9834);
                        vec2 _23301;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _21729 = _9835;
                            _21729.y = 1.0 - _9834;
                            _23301 = _21729;
                        }
                        else
                        {
                            _23301 = _9835;
                        }
                        vec2 _23302;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _21735 = _9797;
                            _21735.y = 1.0 - _9797.y;
                            _23302 = _21735;
                        }
                        else
                        {
                            _23302 = _9797;
                        }
                        _23305 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23299, 0.0) * _9774.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23300, 0.0) * _9777.x)) * _9774.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23301, 0.0) * _9774.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23302, 0.0) * _9777.x)) * _9777.y);
                    }
                    else
                    {
                        vec4 _23306;
                        if (_9589 == 2)
                        {
                            vec2 _9935 = (_7168 * _178.gLevel[2].xy) - vec2(0.5);
                            vec2 _9937 = floor(_9935);
                            vec2 _9940 = _9935 - _9937;
                            vec2 _9943 = _9940 * _9940;
                            vec2 _9946 = _9943 * _9940;
                            vec2 _9965 = (((_9946 * 3.0) - (_9943 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _9978 = _9946 * 0.16666667163372039794921875;
                            vec2 _9981 = (((((-_9946) + (_9943 * 3.0)) - (_9940 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9965;
                            vec2 _9984 = (((((_9946 * (-3.0)) + (_9943 * 3.0)) + (_9940 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9978;
                            vec2 _9994 = ((_9937 - vec2(0.5)) + (_9965 / _9981)) * _178.gLevel[2].zw;
                            vec2 _10004 = ((_9937 + vec2(1.5)) + (_9978 / _9984)) * _178.gLevel[2].zw;
                            vec2 _23295;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _21742 = _9994;
                                _21742.y = 1.0 - _9994.y;
                                _23295 = _21742;
                            }
                            else
                            {
                                _23295 = _9994;
                            }
                            float _10024 = _9994.y;
                            vec2 _10025 = vec2(_10004.x, _10024);
                            vec2 _23296;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _21748 = _10025;
                                _21748.y = 1.0 - _10024;
                                _23296 = _21748;
                            }
                            else
                            {
                                _23296 = _10025;
                            }
                            float _10041 = _10004.y;
                            vec2 _10042 = vec2(_9994.x, _10041);
                            vec2 _23297;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _21755 = _10042;
                                _21755.y = 1.0 - _10041;
                                _23297 = _21755;
                            }
                            else
                            {
                                _23297 = _10042;
                            }
                            vec2 _23298;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _21761 = _10004;
                                _21761.y = 1.0 - _10004.y;
                                _23298 = _21761;
                            }
                            else
                            {
                                _23298 = _10004;
                            }
                            _23306 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23295, 0.0) * _9981.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23296, 0.0) * _9984.x)) * _9981.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23297, 0.0) * _9981.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23298, 0.0) * _9984.x)) * _9984.y);
                        }
                        else
                        {
                            vec4 _23307;
                            if (_9589 == 3)
                            {
                                vec2 _10142 = (_7168 * _178.gLevel[3].xy) - vec2(0.5);
                                vec2 _10144 = floor(_10142);
                                vec2 _10147 = _10142 - _10144;
                                vec2 _10150 = _10147 * _10147;
                                vec2 _10153 = _10150 * _10147;
                                vec2 _10172 = (((_10153 * 3.0) - (_10150 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _10185 = _10153 * 0.16666667163372039794921875;
                                vec2 _10188 = (((((-_10153) + (_10150 * 3.0)) - (_10147 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10172;
                                vec2 _10191 = (((((_10153 * (-3.0)) + (_10150 * 3.0)) + (_10147 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10185;
                                vec2 _10201 = ((_10144 - vec2(0.5)) + (_10172 / _10188)) * _178.gLevel[3].zw;
                                vec2 _10211 = ((_10144 + vec2(1.5)) + (_10185 / _10191)) * _178.gLevel[3].zw;
                                vec2 _23291;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _21768 = _10201;
                                    _21768.y = 1.0 - _10201.y;
                                    _23291 = _21768;
                                }
                                else
                                {
                                    _23291 = _10201;
                                }
                                float _10231 = _10201.y;
                                vec2 _10232 = vec2(_10211.x, _10231);
                                vec2 _23292;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _21774 = _10232;
                                    _21774.y = 1.0 - _10231;
                                    _23292 = _21774;
                                }
                                else
                                {
                                    _23292 = _10232;
                                }
                                float _10248 = _10211.y;
                                vec2 _10249 = vec2(_10201.x, _10248);
                                vec2 _23293;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _21781 = _10249;
                                    _21781.y = 1.0 - _10248;
                                    _23293 = _21781;
                                }
                                else
                                {
                                    _23293 = _10249;
                                }
                                vec2 _23294;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _21787 = _10211;
                                    _21787.y = 1.0 - _10211.y;
                                    _23294 = _21787;
                                }
                                else
                                {
                                    _23294 = _10211;
                                }
                                _23307 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23291, 0.0) * _10188.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23292, 0.0) * _10191.x)) * _10188.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23293, 0.0) * _10188.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23294, 0.0) * _10191.x)) * _10191.y);
                            }
                            else
                            {
                                vec4 _23308;
                                if (_9589 == 4)
                                {
                                    vec2 _10349 = (_7168 * _178.gLevel[4].xy) - vec2(0.5);
                                    vec2 _10351 = floor(_10349);
                                    vec2 _10354 = _10349 - _10351;
                                    vec2 _10357 = _10354 * _10354;
                                    vec2 _10360 = _10357 * _10354;
                                    vec2 _10379 = (((_10360 * 3.0) - (_10357 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _10392 = _10360 * 0.16666667163372039794921875;
                                    vec2 _10395 = (((((-_10360) + (_10357 * 3.0)) - (_10354 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10379;
                                    vec2 _10398 = (((((_10360 * (-3.0)) + (_10357 * 3.0)) + (_10354 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10392;
                                    vec2 _10408 = ((_10351 - vec2(0.5)) + (_10379 / _10395)) * _178.gLevel[4].zw;
                                    vec2 _10418 = ((_10351 + vec2(1.5)) + (_10392 / _10398)) * _178.gLevel[4].zw;
                                    vec2 _23287;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21794 = _10408;
                                        _21794.y = 1.0 - _10408.y;
                                        _23287 = _21794;
                                    }
                                    else
                                    {
                                        _23287 = _10408;
                                    }
                                    float _10438 = _10408.y;
                                    vec2 _10439 = vec2(_10418.x, _10438);
                                    vec2 _23288;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21800 = _10439;
                                        _21800.y = 1.0 - _10438;
                                        _23288 = _21800;
                                    }
                                    else
                                    {
                                        _23288 = _10439;
                                    }
                                    float _10455 = _10418.y;
                                    vec2 _10456 = vec2(_10408.x, _10455);
                                    vec2 _23289;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21807 = _10456;
                                        _21807.y = 1.0 - _10455;
                                        _23289 = _21807;
                                    }
                                    else
                                    {
                                        _23289 = _10456;
                                    }
                                    vec2 _23290;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21813 = _10418;
                                        _21813.y = 1.0 - _10418.y;
                                        _23290 = _21813;
                                    }
                                    else
                                    {
                                        _23290 = _10418;
                                    }
                                    _23308 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23287, 0.0) * _10395.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23288, 0.0) * _10398.x)) * _10395.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23289, 0.0) * _10395.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23290, 0.0) * _10398.x)) * _10398.y);
                                }
                                else
                                {
                                    vec2 _10556 = (_7168 * _178.gLevel[5].xy) - vec2(0.5);
                                    vec2 _10558 = floor(_10556);
                                    vec2 _10561 = _10556 - _10558;
                                    vec2 _10564 = _10561 * _10561;
                                    vec2 _10567 = _10564 * _10561;
                                    vec2 _10586 = (((_10567 * 3.0) - (_10564 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _10599 = _10567 * 0.16666667163372039794921875;
                                    vec2 _10602 = (((((-_10567) + (_10564 * 3.0)) - (_10561 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10586;
                                    vec2 _10605 = (((((_10567 * (-3.0)) + (_10564 * 3.0)) + (_10561 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10599;
                                    vec2 _10615 = ((_10558 - vec2(0.5)) + (_10586 / _10602)) * _178.gLevel[5].zw;
                                    vec2 _10625 = ((_10558 + vec2(1.5)) + (_10599 / _10605)) * _178.gLevel[5].zw;
                                    vec2 _23283;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21820 = _10615;
                                        _21820.y = 1.0 - _10615.y;
                                        _23283 = _21820;
                                    }
                                    else
                                    {
                                        _23283 = _10615;
                                    }
                                    float _10645 = _10615.y;
                                    vec2 _10646 = vec2(_10625.x, _10645);
                                    vec2 _23284;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21826 = _10646;
                                        _21826.y = 1.0 - _10645;
                                        _23284 = _21826;
                                    }
                                    else
                                    {
                                        _23284 = _10646;
                                    }
                                    float _10662 = _10625.y;
                                    vec2 _10663 = vec2(_10615.x, _10662);
                                    vec2 _23285;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21833 = _10663;
                                        _21833.y = 1.0 - _10662;
                                        _23285 = _21833;
                                    }
                                    else
                                    {
                                        _23285 = _10663;
                                    }
                                    vec2 _23286;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21839 = _10625;
                                        _21839.y = 1.0 - _10625.y;
                                        _23286 = _21839;
                                    }
                                    else
                                    {
                                        _23286 = _10625;
                                    }
                                    _23308 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23283, 0.0) * _10602.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23284, 0.0) * _10605.x)) * _10602.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23285, 0.0) * _10602.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23286, 0.0) * _10605.x)) * _10605.y);
                                }
                                _23307 = _23308;
                            }
                            _23306 = _23307;
                        }
                        _23305 = _23306;
                    }
                    _23304 = _23305;
                }
                vec3 _23335;
                SPIRV_CROSS_BRANCH
                if ((_9593 > 0.0199999995529651641845703125) && (_9589 < 5))
                {
                    int _9606 = _9589 + 1;
                    vec4 _23330;
                    SPIRV_CROSS_BRANCH
                    if (_9606 <= 0)
                    {
                        vec2 _23329;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _21844 = _7168;
                            _21844.y = 1.0 - _7168.y;
                            _23329 = _21844;
                        }
                        else
                        {
                            _23329 = _7168;
                        }
                        _23330 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23329, 0.0);
                    }
                    else
                    {
                        vec4 _23331;
                        if (_9606 == 1)
                        {
                            vec2 _10852 = (_7168 * _178.gLevel[1].xy) - vec2(0.5);
                            vec2 _10854 = floor(_10852);
                            vec2 _10857 = _10852 - _10854;
                            vec2 _10860 = _10857 * _10857;
                            vec2 _10863 = _10860 * _10857;
                            vec2 _10882 = (((_10863 * 3.0) - (_10860 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _10895 = _10863 * 0.16666667163372039794921875;
                            vec2 _10898 = (((((-_10863) + (_10860 * 3.0)) - (_10857 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10882;
                            vec2 _10901 = (((((_10863 * (-3.0)) + (_10860 * 3.0)) + (_10857 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10895;
                            vec2 _10911 = ((_10854 - vec2(0.5)) + (_10882 / _10898)) * _178.gLevel[1].zw;
                            vec2 _10921 = ((_10854 + vec2(1.5)) + (_10895 / _10901)) * _178.gLevel[1].zw;
                            vec2 _23325;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _21849 = _10911;
                                _21849.y = 1.0 - _10911.y;
                                _23325 = _21849;
                            }
                            else
                            {
                                _23325 = _10911;
                            }
                            float _10941 = _10911.y;
                            vec2 _10942 = vec2(_10921.x, _10941);
                            vec2 _23326;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _21855 = _10942;
                                _21855.y = 1.0 - _10941;
                                _23326 = _21855;
                            }
                            else
                            {
                                _23326 = _10942;
                            }
                            float _10958 = _10921.y;
                            vec2 _10959 = vec2(_10911.x, _10958);
                            vec2 _23327;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _21862 = _10959;
                                _21862.y = 1.0 - _10958;
                                _23327 = _21862;
                            }
                            else
                            {
                                _23327 = _10959;
                            }
                            vec2 _23328;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _21868 = _10921;
                                _21868.y = 1.0 - _10921.y;
                                _23328 = _21868;
                            }
                            else
                            {
                                _23328 = _10921;
                            }
                            _23331 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23325, 0.0) * _10898.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23326, 0.0) * _10901.x)) * _10898.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23327, 0.0) * _10898.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23328, 0.0) * _10901.x)) * _10901.y);
                        }
                        else
                        {
                            vec4 _23332;
                            if (_9606 == 2)
                            {
                                vec2 _11059 = (_7168 * _178.gLevel[2].xy) - vec2(0.5);
                                vec2 _11061 = floor(_11059);
                                vec2 _11064 = _11059 - _11061;
                                vec2 _11067 = _11064 * _11064;
                                vec2 _11070 = _11067 * _11064;
                                vec2 _11089 = (((_11070 * 3.0) - (_11067 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _11102 = _11070 * 0.16666667163372039794921875;
                                vec2 _11105 = (((((-_11070) + (_11067 * 3.0)) - (_11064 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11089;
                                vec2 _11108 = (((((_11070 * (-3.0)) + (_11067 * 3.0)) + (_11064 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11102;
                                vec2 _11118 = ((_11061 - vec2(0.5)) + (_11089 / _11105)) * _178.gLevel[2].zw;
                                vec2 _11128 = ((_11061 + vec2(1.5)) + (_11102 / _11108)) * _178.gLevel[2].zw;
                                vec2 _23321;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _21875 = _11118;
                                    _21875.y = 1.0 - _11118.y;
                                    _23321 = _21875;
                                }
                                else
                                {
                                    _23321 = _11118;
                                }
                                float _11148 = _11118.y;
                                vec2 _11149 = vec2(_11128.x, _11148);
                                vec2 _23322;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _21881 = _11149;
                                    _21881.y = 1.0 - _11148;
                                    _23322 = _21881;
                                }
                                else
                                {
                                    _23322 = _11149;
                                }
                                float _11165 = _11128.y;
                                vec2 _11166 = vec2(_11118.x, _11165);
                                vec2 _23323;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _21888 = _11166;
                                    _21888.y = 1.0 - _11165;
                                    _23323 = _21888;
                                }
                                else
                                {
                                    _23323 = _11166;
                                }
                                vec2 _23324;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _21894 = _11128;
                                    _21894.y = 1.0 - _11128.y;
                                    _23324 = _21894;
                                }
                                else
                                {
                                    _23324 = _11128;
                                }
                                _23332 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23321, 0.0) * _11105.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23322, 0.0) * _11108.x)) * _11105.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23323, 0.0) * _11105.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23324, 0.0) * _11108.x)) * _11108.y);
                            }
                            else
                            {
                                vec4 _23333;
                                if (_9606 == 3)
                                {
                                    vec2 _11266 = (_7168 * _178.gLevel[3].xy) - vec2(0.5);
                                    vec2 _11268 = floor(_11266);
                                    vec2 _11271 = _11266 - _11268;
                                    vec2 _11274 = _11271 * _11271;
                                    vec2 _11277 = _11274 * _11271;
                                    vec2 _11296 = (((_11277 * 3.0) - (_11274 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _11309 = _11277 * 0.16666667163372039794921875;
                                    vec2 _11312 = (((((-_11277) + (_11274 * 3.0)) - (_11271 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11296;
                                    vec2 _11315 = (((((_11277 * (-3.0)) + (_11274 * 3.0)) + (_11271 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11309;
                                    vec2 _11325 = ((_11268 - vec2(0.5)) + (_11296 / _11312)) * _178.gLevel[3].zw;
                                    vec2 _11335 = ((_11268 + vec2(1.5)) + (_11309 / _11315)) * _178.gLevel[3].zw;
                                    vec2 _23317;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21901 = _11325;
                                        _21901.y = 1.0 - _11325.y;
                                        _23317 = _21901;
                                    }
                                    else
                                    {
                                        _23317 = _11325;
                                    }
                                    float _11355 = _11325.y;
                                    vec2 _11356 = vec2(_11335.x, _11355);
                                    vec2 _23318;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21907 = _11356;
                                        _21907.y = 1.0 - _11355;
                                        _23318 = _21907;
                                    }
                                    else
                                    {
                                        _23318 = _11356;
                                    }
                                    float _11372 = _11335.y;
                                    vec2 _11373 = vec2(_11325.x, _11372);
                                    vec2 _23319;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21914 = _11373;
                                        _21914.y = 1.0 - _11372;
                                        _23319 = _21914;
                                    }
                                    else
                                    {
                                        _23319 = _11373;
                                    }
                                    vec2 _23320;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _21920 = _11335;
                                        _21920.y = 1.0 - _11335.y;
                                        _23320 = _21920;
                                    }
                                    else
                                    {
                                        _23320 = _11335;
                                    }
                                    _23333 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23317, 0.0) * _11312.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23318, 0.0) * _11315.x)) * _11312.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23319, 0.0) * _11312.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23320, 0.0) * _11315.x)) * _11315.y);
                                }
                                else
                                {
                                    vec4 _23334;
                                    if (_9606 == 4)
                                    {
                                        vec2 _11473 = (_7168 * _178.gLevel[4].xy) - vec2(0.5);
                                        vec2 _11475 = floor(_11473);
                                        vec2 _11478 = _11473 - _11475;
                                        vec2 _11481 = _11478 * _11478;
                                        vec2 _11484 = _11481 * _11478;
                                        vec2 _11503 = (((_11484 * 3.0) - (_11481 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _11516 = _11484 * 0.16666667163372039794921875;
                                        vec2 _11519 = (((((-_11484) + (_11481 * 3.0)) - (_11478 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11503;
                                        vec2 _11522 = (((((_11484 * (-3.0)) + (_11481 * 3.0)) + (_11478 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11516;
                                        vec2 _11532 = ((_11475 - vec2(0.5)) + (_11503 / _11519)) * _178.gLevel[4].zw;
                                        vec2 _11542 = ((_11475 + vec2(1.5)) + (_11516 / _11522)) * _178.gLevel[4].zw;
                                        vec2 _23313;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _21927 = _11532;
                                            _21927.y = 1.0 - _11532.y;
                                            _23313 = _21927;
                                        }
                                        else
                                        {
                                            _23313 = _11532;
                                        }
                                        float _11562 = _11532.y;
                                        vec2 _11563 = vec2(_11542.x, _11562);
                                        vec2 _23314;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _21933 = _11563;
                                            _21933.y = 1.0 - _11562;
                                            _23314 = _21933;
                                        }
                                        else
                                        {
                                            _23314 = _11563;
                                        }
                                        float _11579 = _11542.y;
                                        vec2 _11580 = vec2(_11532.x, _11579);
                                        vec2 _23315;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _21940 = _11580;
                                            _21940.y = 1.0 - _11579;
                                            _23315 = _21940;
                                        }
                                        else
                                        {
                                            _23315 = _11580;
                                        }
                                        vec2 _23316;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _21946 = _11542;
                                            _21946.y = 1.0 - _11542.y;
                                            _23316 = _21946;
                                        }
                                        else
                                        {
                                            _23316 = _11542;
                                        }
                                        _23334 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23313, 0.0) * _11519.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23314, 0.0) * _11522.x)) * _11519.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23315, 0.0) * _11519.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23316, 0.0) * _11522.x)) * _11522.y);
                                    }
                                    else
                                    {
                                        vec2 _11680 = (_7168 * _178.gLevel[5].xy) - vec2(0.5);
                                        vec2 _11682 = floor(_11680);
                                        vec2 _11685 = _11680 - _11682;
                                        vec2 _11688 = _11685 * _11685;
                                        vec2 _11691 = _11688 * _11685;
                                        vec2 _11710 = (((_11691 * 3.0) - (_11688 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _11723 = _11691 * 0.16666667163372039794921875;
                                        vec2 _11726 = (((((-_11691) + (_11688 * 3.0)) - (_11685 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11710;
                                        vec2 _11729 = (((((_11691 * (-3.0)) + (_11688 * 3.0)) + (_11685 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11723;
                                        vec2 _11739 = ((_11682 - vec2(0.5)) + (_11710 / _11726)) * _178.gLevel[5].zw;
                                        vec2 _11749 = ((_11682 + vec2(1.5)) + (_11723 / _11729)) * _178.gLevel[5].zw;
                                        vec2 _23309;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _21953 = _11739;
                                            _21953.y = 1.0 - _11739.y;
                                            _23309 = _21953;
                                        }
                                        else
                                        {
                                            _23309 = _11739;
                                        }
                                        float _11769 = _11739.y;
                                        vec2 _11770 = vec2(_11749.x, _11769);
                                        vec2 _23310;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _21959 = _11770;
                                            _21959.y = 1.0 - _11769;
                                            _23310 = _21959;
                                        }
                                        else
                                        {
                                            _23310 = _11770;
                                        }
                                        float _11786 = _11749.y;
                                        vec2 _11787 = vec2(_11739.x, _11786);
                                        vec2 _23311;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _21966 = _11787;
                                            _21966.y = 1.0 - _11786;
                                            _23311 = _21966;
                                        }
                                        else
                                        {
                                            _23311 = _11787;
                                        }
                                        vec2 _23312;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _21972 = _11749;
                                            _21972.y = 1.0 - _11749.y;
                                            _23312 = _21972;
                                        }
                                        else
                                        {
                                            _23312 = _11749;
                                        }
                                        _23334 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23309, 0.0) * _11726.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23310, 0.0) * _11729.x)) * _11726.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23311, 0.0) * _11726.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23312, 0.0) * _11729.x)) * _11729.y);
                                    }
                                    _23333 = _23334;
                                }
                                _23332 = _23333;
                            }
                            _23331 = _23332;
                        }
                        _23330 = _23331;
                    }
                    _23335 = mix(_23304.xyz, _23330.xyz, vec3(_9593));
                }
                else
                {
                    _23335 = _23304.xyz;
                }
                vec2 _7175 = _7124 + _7136;
                float _11877 = clamp(log2(max(_7038, 1.0)) - 1.0, 0.0, 5.0);
                int _11880 = int(floor(_11877));
                float _11884 = _11877 - float(_11880);
                vec4 _23410;
                SPIRV_CROSS_BRANCH
                if (_11880 <= 0)
                {
                    vec2 _23409;
                    if (_178.gConv.x > 0.5)
                    {
                        vec2 _21979 = _7175;
                        _21979.y = 1.0 - _7175.y;
                        _23409 = _21979;
                    }
                    else
                    {
                        _23409 = _7175;
                    }
                    _23410 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23409, 0.0);
                }
                else
                {
                    vec4 _23411;
                    if (_11880 == 1)
                    {
                        vec2 _12019 = (_7175 * _178.gLevel[1].xy) - vec2(0.5);
                        vec2 _12021 = floor(_12019);
                        vec2 _12024 = _12019 - _12021;
                        vec2 _12027 = _12024 * _12024;
                        vec2 _12030 = _12027 * _12024;
                        vec2 _12049 = (((_12030 * 3.0) - (_12027 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                        vec2 _12062 = _12030 * 0.16666667163372039794921875;
                        vec2 _12065 = (((((-_12030) + (_12027 * 3.0)) - (_12024 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12049;
                        vec2 _12068 = (((((_12030 * (-3.0)) + (_12027 * 3.0)) + (_12024 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12062;
                        vec2 _12078 = ((_12021 - vec2(0.5)) + (_12049 / _12065)) * _178.gLevel[1].zw;
                        vec2 _12088 = ((_12021 + vec2(1.5)) + (_12062 / _12068)) * _178.gLevel[1].zw;
                        vec2 _23405;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _21984 = _12078;
                            _21984.y = 1.0 - _12078.y;
                            _23405 = _21984;
                        }
                        else
                        {
                            _23405 = _12078;
                        }
                        vec2 _12109 = vec2(_12088.x, _12078.y);
                        vec2 _23406;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _21990 = _12109;
                            _21990.y = 1.0 - _12078.y;
                            _23406 = _21990;
                        }
                        else
                        {
                            _23406 = _12109;
                        }
                        float _12125 = _12088.y;
                        vec2 _12126 = vec2(_12078.x, _12125);
                        vec2 _23407;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _21997 = _12126;
                            _21997.y = 1.0 - _12125;
                            _23407 = _21997;
                        }
                        else
                        {
                            _23407 = _12126;
                        }
                        vec2 _23408;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22003 = _12088;
                            _22003.y = 1.0 - _12088.y;
                            _23408 = _22003;
                        }
                        else
                        {
                            _23408 = _12088;
                        }
                        _23411 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23405, 0.0) * _12065.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23406, 0.0) * _12068.x)) * _12065.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23407, 0.0) * _12065.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23408, 0.0) * _12068.x)) * _12068.y);
                    }
                    else
                    {
                        vec4 _23412;
                        if (_11880 == 2)
                        {
                            vec2 _12226 = (_7175 * _178.gLevel[2].xy) - vec2(0.5);
                            vec2 _12228 = floor(_12226);
                            vec2 _12231 = _12226 - _12228;
                            vec2 _12234 = _12231 * _12231;
                            vec2 _12237 = _12234 * _12231;
                            vec2 _12256 = (((_12237 * 3.0) - (_12234 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _12269 = _12237 * 0.16666667163372039794921875;
                            vec2 _12272 = (((((-_12237) + (_12234 * 3.0)) - (_12231 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12256;
                            vec2 _12275 = (((((_12237 * (-3.0)) + (_12234 * 3.0)) + (_12231 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12269;
                            vec2 _12285 = ((_12228 - vec2(0.5)) + (_12256 / _12272)) * _178.gLevel[2].zw;
                            vec2 _12295 = ((_12228 + vec2(1.5)) + (_12269 / _12275)) * _178.gLevel[2].zw;
                            vec2 _23401;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22010 = _12285;
                                _22010.y = 1.0 - _12285.y;
                                _23401 = _22010;
                            }
                            else
                            {
                                _23401 = _12285;
                            }
                            vec2 _12316 = vec2(_12295.x, _12285.y);
                            vec2 _23402;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22016 = _12316;
                                _22016.y = 1.0 - _12285.y;
                                _23402 = _22016;
                            }
                            else
                            {
                                _23402 = _12316;
                            }
                            float _12332 = _12295.y;
                            vec2 _12333 = vec2(_12285.x, _12332);
                            vec2 _23403;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22023 = _12333;
                                _22023.y = 1.0 - _12332;
                                _23403 = _22023;
                            }
                            else
                            {
                                _23403 = _12333;
                            }
                            vec2 _23404;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22029 = _12295;
                                _22029.y = 1.0 - _12295.y;
                                _23404 = _22029;
                            }
                            else
                            {
                                _23404 = _12295;
                            }
                            _23412 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23401, 0.0) * _12272.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23402, 0.0) * _12275.x)) * _12272.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23403, 0.0) * _12272.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23404, 0.0) * _12275.x)) * _12275.y);
                        }
                        else
                        {
                            vec4 _23413;
                            if (_11880 == 3)
                            {
                                vec2 _12433 = (_7175 * _178.gLevel[3].xy) - vec2(0.5);
                                vec2 _12435 = floor(_12433);
                                vec2 _12438 = _12433 - _12435;
                                vec2 _12441 = _12438 * _12438;
                                vec2 _12444 = _12441 * _12438;
                                vec2 _12463 = (((_12444 * 3.0) - (_12441 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _12476 = _12444 * 0.16666667163372039794921875;
                                vec2 _12479 = (((((-_12444) + (_12441 * 3.0)) - (_12438 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12463;
                                vec2 _12482 = (((((_12444 * (-3.0)) + (_12441 * 3.0)) + (_12438 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12476;
                                vec2 _12492 = ((_12435 - vec2(0.5)) + (_12463 / _12479)) * _178.gLevel[3].zw;
                                vec2 _12502 = ((_12435 + vec2(1.5)) + (_12476 / _12482)) * _178.gLevel[3].zw;
                                vec2 _23397;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22036 = _12492;
                                    _22036.y = 1.0 - _12492.y;
                                    _23397 = _22036;
                                }
                                else
                                {
                                    _23397 = _12492;
                                }
                                vec2 _12523 = vec2(_12502.x, _12492.y);
                                vec2 _23398;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22042 = _12523;
                                    _22042.y = 1.0 - _12492.y;
                                    _23398 = _22042;
                                }
                                else
                                {
                                    _23398 = _12523;
                                }
                                float _12539 = _12502.y;
                                vec2 _12540 = vec2(_12492.x, _12539);
                                vec2 _23399;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22049 = _12540;
                                    _22049.y = 1.0 - _12539;
                                    _23399 = _22049;
                                }
                                else
                                {
                                    _23399 = _12540;
                                }
                                vec2 _23400;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22055 = _12502;
                                    _22055.y = 1.0 - _12502.y;
                                    _23400 = _22055;
                                }
                                else
                                {
                                    _23400 = _12502;
                                }
                                _23413 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23397, 0.0) * _12479.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23398, 0.0) * _12482.x)) * _12479.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23399, 0.0) * _12479.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23400, 0.0) * _12482.x)) * _12482.y);
                            }
                            else
                            {
                                vec4 _23414;
                                if (_11880 == 4)
                                {
                                    vec2 _12640 = (_7175 * _178.gLevel[4].xy) - vec2(0.5);
                                    vec2 _12642 = floor(_12640);
                                    vec2 _12645 = _12640 - _12642;
                                    vec2 _12648 = _12645 * _12645;
                                    vec2 _12651 = _12648 * _12645;
                                    vec2 _12670 = (((_12651 * 3.0) - (_12648 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _12683 = _12651 * 0.16666667163372039794921875;
                                    vec2 _12686 = (((((-_12651) + (_12648 * 3.0)) - (_12645 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12670;
                                    vec2 _12689 = (((((_12651 * (-3.0)) + (_12648 * 3.0)) + (_12645 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12683;
                                    vec2 _12699 = ((_12642 - vec2(0.5)) + (_12670 / _12686)) * _178.gLevel[4].zw;
                                    vec2 _12709 = ((_12642 + vec2(1.5)) + (_12683 / _12689)) * _178.gLevel[4].zw;
                                    vec2 _23393;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22062 = _12699;
                                        _22062.y = 1.0 - _12699.y;
                                        _23393 = _22062;
                                    }
                                    else
                                    {
                                        _23393 = _12699;
                                    }
                                    vec2 _12730 = vec2(_12709.x, _12699.y);
                                    vec2 _23394;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22068 = _12730;
                                        _22068.y = 1.0 - _12699.y;
                                        _23394 = _22068;
                                    }
                                    else
                                    {
                                        _23394 = _12730;
                                    }
                                    float _12746 = _12709.y;
                                    vec2 _12747 = vec2(_12699.x, _12746);
                                    vec2 _23395;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22075 = _12747;
                                        _22075.y = 1.0 - _12746;
                                        _23395 = _22075;
                                    }
                                    else
                                    {
                                        _23395 = _12747;
                                    }
                                    vec2 _23396;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22081 = _12709;
                                        _22081.y = 1.0 - _12709.y;
                                        _23396 = _22081;
                                    }
                                    else
                                    {
                                        _23396 = _12709;
                                    }
                                    _23414 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23393, 0.0) * _12686.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23394, 0.0) * _12689.x)) * _12686.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23395, 0.0) * _12686.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23396, 0.0) * _12689.x)) * _12689.y);
                                }
                                else
                                {
                                    vec2 _12847 = (_7175 * _178.gLevel[5].xy) - vec2(0.5);
                                    vec2 _12849 = floor(_12847);
                                    vec2 _12852 = _12847 - _12849;
                                    vec2 _12855 = _12852 * _12852;
                                    vec2 _12858 = _12855 * _12852;
                                    vec2 _12877 = (((_12858 * 3.0) - (_12855 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _12890 = _12858 * 0.16666667163372039794921875;
                                    vec2 _12893 = (((((-_12858) + (_12855 * 3.0)) - (_12852 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12877;
                                    vec2 _12896 = (((((_12858 * (-3.0)) + (_12855 * 3.0)) + (_12852 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12890;
                                    vec2 _12906 = ((_12849 - vec2(0.5)) + (_12877 / _12893)) * _178.gLevel[5].zw;
                                    vec2 _12916 = ((_12849 + vec2(1.5)) + (_12890 / _12896)) * _178.gLevel[5].zw;
                                    vec2 _23389;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22088 = _12906;
                                        _22088.y = 1.0 - _12906.y;
                                        _23389 = _22088;
                                    }
                                    else
                                    {
                                        _23389 = _12906;
                                    }
                                    vec2 _12937 = vec2(_12916.x, _12906.y);
                                    vec2 _23390;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22094 = _12937;
                                        _22094.y = 1.0 - _12906.y;
                                        _23390 = _22094;
                                    }
                                    else
                                    {
                                        _23390 = _12937;
                                    }
                                    float _12953 = _12916.y;
                                    vec2 _12954 = vec2(_12906.x, _12953);
                                    vec2 _23391;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22101 = _12954;
                                        _22101.y = 1.0 - _12953;
                                        _23391 = _22101;
                                    }
                                    else
                                    {
                                        _23391 = _12954;
                                    }
                                    vec2 _23392;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22107 = _12916;
                                        _22107.y = 1.0 - _12916.y;
                                        _23392 = _22107;
                                    }
                                    else
                                    {
                                        _23392 = _12916;
                                    }
                                    _23414 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23389, 0.0) * _12893.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23390, 0.0) * _12896.x)) * _12893.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23391, 0.0) * _12893.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23392, 0.0) * _12896.x)) * _12896.y);
                                }
                                _23413 = _23414;
                            }
                            _23412 = _23413;
                        }
                        _23411 = _23412;
                    }
                    _23410 = _23411;
                }
                vec3 _23441;
                SPIRV_CROSS_BRANCH
                if ((_11884 > 0.0199999995529651641845703125) && (_11880 < 5))
                {
                    int _11897 = _11880 + 1;
                    vec4 _23436;
                    SPIRV_CROSS_BRANCH
                    if (_11897 <= 0)
                    {
                        vec2 _23435;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22112 = _7175;
                            _22112.y = 1.0 - _7175.y;
                            _23435 = _22112;
                        }
                        else
                        {
                            _23435 = _7175;
                        }
                        _23436 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23435, 0.0);
                    }
                    else
                    {
                        vec4 _23437;
                        if (_11897 == 1)
                        {
                            vec2 _13143 = (_7175 * _178.gLevel[1].xy) - vec2(0.5);
                            vec2 _13145 = floor(_13143);
                            vec2 _13148 = _13143 - _13145;
                            vec2 _13151 = _13148 * _13148;
                            vec2 _13154 = _13151 * _13148;
                            vec2 _13173 = (((_13154 * 3.0) - (_13151 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _13186 = _13154 * 0.16666667163372039794921875;
                            vec2 _13189 = (((((-_13154) + (_13151 * 3.0)) - (_13148 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13173;
                            vec2 _13192 = (((((_13154 * (-3.0)) + (_13151 * 3.0)) + (_13148 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13186;
                            vec2 _13202 = ((_13145 - vec2(0.5)) + (_13173 / _13189)) * _178.gLevel[1].zw;
                            vec2 _13212 = ((_13145 + vec2(1.5)) + (_13186 / _13192)) * _178.gLevel[1].zw;
                            vec2 _23431;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22117 = _13202;
                                _22117.y = 1.0 - _13202.y;
                                _23431 = _22117;
                            }
                            else
                            {
                                _23431 = _13202;
                            }
                            vec2 _13233 = vec2(_13212.x, _13202.y);
                            vec2 _23432;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22123 = _13233;
                                _22123.y = 1.0 - _13202.y;
                                _23432 = _22123;
                            }
                            else
                            {
                                _23432 = _13233;
                            }
                            float _13249 = _13212.y;
                            vec2 _13250 = vec2(_13202.x, _13249);
                            vec2 _23433;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22130 = _13250;
                                _22130.y = 1.0 - _13249;
                                _23433 = _22130;
                            }
                            else
                            {
                                _23433 = _13250;
                            }
                            vec2 _23434;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22136 = _13212;
                                _22136.y = 1.0 - _13212.y;
                                _23434 = _22136;
                            }
                            else
                            {
                                _23434 = _13212;
                            }
                            _23437 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23431, 0.0) * _13189.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23432, 0.0) * _13192.x)) * _13189.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23433, 0.0) * _13189.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23434, 0.0) * _13192.x)) * _13192.y);
                        }
                        else
                        {
                            vec4 _23438;
                            if (_11897 == 2)
                            {
                                vec2 _13350 = (_7175 * _178.gLevel[2].xy) - vec2(0.5);
                                vec2 _13352 = floor(_13350);
                                vec2 _13355 = _13350 - _13352;
                                vec2 _13358 = _13355 * _13355;
                                vec2 _13361 = _13358 * _13355;
                                vec2 _13380 = (((_13361 * 3.0) - (_13358 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _13393 = _13361 * 0.16666667163372039794921875;
                                vec2 _13396 = (((((-_13361) + (_13358 * 3.0)) - (_13355 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13380;
                                vec2 _13399 = (((((_13361 * (-3.0)) + (_13358 * 3.0)) + (_13355 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13393;
                                vec2 _13409 = ((_13352 - vec2(0.5)) + (_13380 / _13396)) * _178.gLevel[2].zw;
                                vec2 _13419 = ((_13352 + vec2(1.5)) + (_13393 / _13399)) * _178.gLevel[2].zw;
                                vec2 _23427;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22143 = _13409;
                                    _22143.y = 1.0 - _13409.y;
                                    _23427 = _22143;
                                }
                                else
                                {
                                    _23427 = _13409;
                                }
                                vec2 _13440 = vec2(_13419.x, _13409.y);
                                vec2 _23428;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22149 = _13440;
                                    _22149.y = 1.0 - _13409.y;
                                    _23428 = _22149;
                                }
                                else
                                {
                                    _23428 = _13440;
                                }
                                float _13456 = _13419.y;
                                vec2 _13457 = vec2(_13409.x, _13456);
                                vec2 _23429;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22156 = _13457;
                                    _22156.y = 1.0 - _13456;
                                    _23429 = _22156;
                                }
                                else
                                {
                                    _23429 = _13457;
                                }
                                vec2 _23430;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22162 = _13419;
                                    _22162.y = 1.0 - _13419.y;
                                    _23430 = _22162;
                                }
                                else
                                {
                                    _23430 = _13419;
                                }
                                _23438 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23427, 0.0) * _13396.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23428, 0.0) * _13399.x)) * _13396.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23429, 0.0) * _13396.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23430, 0.0) * _13399.x)) * _13399.y);
                            }
                            else
                            {
                                vec4 _23439;
                                if (_11897 == 3)
                                {
                                    vec2 _13557 = (_7175 * _178.gLevel[3].xy) - vec2(0.5);
                                    vec2 _13559 = floor(_13557);
                                    vec2 _13562 = _13557 - _13559;
                                    vec2 _13565 = _13562 * _13562;
                                    vec2 _13568 = _13565 * _13562;
                                    vec2 _13587 = (((_13568 * 3.0) - (_13565 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _13600 = _13568 * 0.16666667163372039794921875;
                                    vec2 _13603 = (((((-_13568) + (_13565 * 3.0)) - (_13562 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13587;
                                    vec2 _13606 = (((((_13568 * (-3.0)) + (_13565 * 3.0)) + (_13562 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13600;
                                    vec2 _13616 = ((_13559 - vec2(0.5)) + (_13587 / _13603)) * _178.gLevel[3].zw;
                                    vec2 _13626 = ((_13559 + vec2(1.5)) + (_13600 / _13606)) * _178.gLevel[3].zw;
                                    vec2 _23423;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22169 = _13616;
                                        _22169.y = 1.0 - _13616.y;
                                        _23423 = _22169;
                                    }
                                    else
                                    {
                                        _23423 = _13616;
                                    }
                                    vec2 _13647 = vec2(_13626.x, _13616.y);
                                    vec2 _23424;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22175 = _13647;
                                        _22175.y = 1.0 - _13616.y;
                                        _23424 = _22175;
                                    }
                                    else
                                    {
                                        _23424 = _13647;
                                    }
                                    float _13663 = _13626.y;
                                    vec2 _13664 = vec2(_13616.x, _13663);
                                    vec2 _23425;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22182 = _13664;
                                        _22182.y = 1.0 - _13663;
                                        _23425 = _22182;
                                    }
                                    else
                                    {
                                        _23425 = _13664;
                                    }
                                    vec2 _23426;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22188 = _13626;
                                        _22188.y = 1.0 - _13626.y;
                                        _23426 = _22188;
                                    }
                                    else
                                    {
                                        _23426 = _13626;
                                    }
                                    _23439 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23423, 0.0) * _13603.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23424, 0.0) * _13606.x)) * _13603.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23425, 0.0) * _13603.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23426, 0.0) * _13606.x)) * _13606.y);
                                }
                                else
                                {
                                    vec4 _23440;
                                    if (_11897 == 4)
                                    {
                                        vec2 _13764 = (_7175 * _178.gLevel[4].xy) - vec2(0.5);
                                        vec2 _13766 = floor(_13764);
                                        vec2 _13769 = _13764 - _13766;
                                        vec2 _13772 = _13769 * _13769;
                                        vec2 _13775 = _13772 * _13769;
                                        vec2 _13794 = (((_13775 * 3.0) - (_13772 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _13807 = _13775 * 0.16666667163372039794921875;
                                        vec2 _13810 = (((((-_13775) + (_13772 * 3.0)) - (_13769 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13794;
                                        vec2 _13813 = (((((_13775 * (-3.0)) + (_13772 * 3.0)) + (_13769 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13807;
                                        vec2 _13823 = ((_13766 - vec2(0.5)) + (_13794 / _13810)) * _178.gLevel[4].zw;
                                        vec2 _13833 = ((_13766 + vec2(1.5)) + (_13807 / _13813)) * _178.gLevel[4].zw;
                                        vec2 _23419;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22195 = _13823;
                                            _22195.y = 1.0 - _13823.y;
                                            _23419 = _22195;
                                        }
                                        else
                                        {
                                            _23419 = _13823;
                                        }
                                        vec2 _13854 = vec2(_13833.x, _13823.y);
                                        vec2 _23420;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22201 = _13854;
                                            _22201.y = 1.0 - _13823.y;
                                            _23420 = _22201;
                                        }
                                        else
                                        {
                                            _23420 = _13854;
                                        }
                                        float _13870 = _13833.y;
                                        vec2 _13871 = vec2(_13823.x, _13870);
                                        vec2 _23421;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22208 = _13871;
                                            _22208.y = 1.0 - _13870;
                                            _23421 = _22208;
                                        }
                                        else
                                        {
                                            _23421 = _13871;
                                        }
                                        vec2 _23422;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22214 = _13833;
                                            _22214.y = 1.0 - _13833.y;
                                            _23422 = _22214;
                                        }
                                        else
                                        {
                                            _23422 = _13833;
                                        }
                                        _23440 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23419, 0.0) * _13810.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23420, 0.0) * _13813.x)) * _13810.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23421, 0.0) * _13810.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23422, 0.0) * _13813.x)) * _13813.y);
                                    }
                                    else
                                    {
                                        vec2 _13971 = (_7175 * _178.gLevel[5].xy) - vec2(0.5);
                                        vec2 _13973 = floor(_13971);
                                        vec2 _13976 = _13971 - _13973;
                                        vec2 _13979 = _13976 * _13976;
                                        vec2 _13982 = _13979 * _13976;
                                        vec2 _14001 = (((_13982 * 3.0) - (_13979 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _14014 = _13982 * 0.16666667163372039794921875;
                                        vec2 _14017 = (((((-_13982) + (_13979 * 3.0)) - (_13976 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14001;
                                        vec2 _14020 = (((((_13982 * (-3.0)) + (_13979 * 3.0)) + (_13976 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14014;
                                        vec2 _14030 = ((_13973 - vec2(0.5)) + (_14001 / _14017)) * _178.gLevel[5].zw;
                                        vec2 _14040 = ((_13973 + vec2(1.5)) + (_14014 / _14020)) * _178.gLevel[5].zw;
                                        vec2 _23415;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22221 = _14030;
                                            _22221.y = 1.0 - _14030.y;
                                            _23415 = _22221;
                                        }
                                        else
                                        {
                                            _23415 = _14030;
                                        }
                                        vec2 _14061 = vec2(_14040.x, _14030.y);
                                        vec2 _23416;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22227 = _14061;
                                            _22227.y = 1.0 - _14030.y;
                                            _23416 = _22227;
                                        }
                                        else
                                        {
                                            _23416 = _14061;
                                        }
                                        float _14077 = _14040.y;
                                        vec2 _14078 = vec2(_14030.x, _14077);
                                        vec2 _23417;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22234 = _14078;
                                            _22234.y = 1.0 - _14077;
                                            _23417 = _22234;
                                        }
                                        else
                                        {
                                            _23417 = _14078;
                                        }
                                        vec2 _23418;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22240 = _14040;
                                            _22240.y = 1.0 - _14040.y;
                                            _23418 = _22240;
                                        }
                                        else
                                        {
                                            _23418 = _14040;
                                        }
                                        _23440 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23415, 0.0) * _14017.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23416, 0.0) * _14020.x)) * _14017.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23417, 0.0) * _14017.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23418, 0.0) * _14020.x)) * _14020.y);
                                    }
                                    _23439 = _23440;
                                }
                                _23438 = _23439;
                            }
                            _23437 = _23438;
                        }
                        _23436 = _23437;
                    }
                    _23441 = mix(_23410.xyz, _23436.xyz, vec3(_11884));
                }
                else
                {
                    _23441 = _23410.xyz;
                }
                vec2 _7186 = (_7124 + _7136) + (_7139 * _7161);
                float _14168 = clamp(log2(max(_7038, 1.0)) - 1.0, 0.0, 5.0);
                int _14171 = int(floor(_14168));
                float _14175 = _14168 - float(_14171);
                vec4 _23516;
                SPIRV_CROSS_BRANCH
                if (_14171 <= 0)
                {
                    vec2 _23515;
                    if (_178.gConv.x > 0.5)
                    {
                        vec2 _22247 = _7186;
                        _22247.y = 1.0 - _7186.y;
                        _23515 = _22247;
                    }
                    else
                    {
                        _23515 = _7186;
                    }
                    _23516 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23515, 0.0);
                }
                else
                {
                    vec4 _23517;
                    if (_14171 == 1)
                    {
                        vec2 _14310 = (_7186 * _178.gLevel[1].xy) - vec2(0.5);
                        vec2 _14312 = floor(_14310);
                        vec2 _14315 = _14310 - _14312;
                        vec2 _14318 = _14315 * _14315;
                        vec2 _14321 = _14318 * _14315;
                        vec2 _14340 = (((_14321 * 3.0) - (_14318 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                        vec2 _14353 = _14321 * 0.16666667163372039794921875;
                        vec2 _14356 = (((((-_14321) + (_14318 * 3.0)) - (_14315 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14340;
                        vec2 _14359 = (((((_14321 * (-3.0)) + (_14318 * 3.0)) + (_14315 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14353;
                        vec2 _14369 = ((_14312 - vec2(0.5)) + (_14340 / _14356)) * _178.gLevel[1].zw;
                        vec2 _14379 = ((_14312 + vec2(1.5)) + (_14353 / _14359)) * _178.gLevel[1].zw;
                        vec2 _23511;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22252 = _14369;
                            _22252.y = 1.0 - _14369.y;
                            _23511 = _22252;
                        }
                        else
                        {
                            _23511 = _14369;
                        }
                        float _14399 = _14369.y;
                        vec2 _14400 = vec2(_14379.x, _14399);
                        vec2 _23512;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22258 = _14400;
                            _22258.y = 1.0 - _14399;
                            _23512 = _22258;
                        }
                        else
                        {
                            _23512 = _14400;
                        }
                        float _14416 = _14379.y;
                        vec2 _14417 = vec2(_14369.x, _14416);
                        vec2 _23513;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22265 = _14417;
                            _22265.y = 1.0 - _14416;
                            _23513 = _22265;
                        }
                        else
                        {
                            _23513 = _14417;
                        }
                        vec2 _23514;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22271 = _14379;
                            _22271.y = 1.0 - _14379.y;
                            _23514 = _22271;
                        }
                        else
                        {
                            _23514 = _14379;
                        }
                        _23517 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23511, 0.0) * _14356.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23512, 0.0) * _14359.x)) * _14356.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23513, 0.0) * _14356.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23514, 0.0) * _14359.x)) * _14359.y);
                    }
                    else
                    {
                        vec4 _23518;
                        if (_14171 == 2)
                        {
                            vec2 _14517 = (_7186 * _178.gLevel[2].xy) - vec2(0.5);
                            vec2 _14519 = floor(_14517);
                            vec2 _14522 = _14517 - _14519;
                            vec2 _14525 = _14522 * _14522;
                            vec2 _14528 = _14525 * _14522;
                            vec2 _14547 = (((_14528 * 3.0) - (_14525 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _14560 = _14528 * 0.16666667163372039794921875;
                            vec2 _14563 = (((((-_14528) + (_14525 * 3.0)) - (_14522 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14547;
                            vec2 _14566 = (((((_14528 * (-3.0)) + (_14525 * 3.0)) + (_14522 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14560;
                            vec2 _14576 = ((_14519 - vec2(0.5)) + (_14547 / _14563)) * _178.gLevel[2].zw;
                            vec2 _14586 = ((_14519 + vec2(1.5)) + (_14560 / _14566)) * _178.gLevel[2].zw;
                            vec2 _23507;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22278 = _14576;
                                _22278.y = 1.0 - _14576.y;
                                _23507 = _22278;
                            }
                            else
                            {
                                _23507 = _14576;
                            }
                            float _14606 = _14576.y;
                            vec2 _14607 = vec2(_14586.x, _14606);
                            vec2 _23508;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22284 = _14607;
                                _22284.y = 1.0 - _14606;
                                _23508 = _22284;
                            }
                            else
                            {
                                _23508 = _14607;
                            }
                            float _14623 = _14586.y;
                            vec2 _14624 = vec2(_14576.x, _14623);
                            vec2 _23509;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22291 = _14624;
                                _22291.y = 1.0 - _14623;
                                _23509 = _22291;
                            }
                            else
                            {
                                _23509 = _14624;
                            }
                            vec2 _23510;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22297 = _14586;
                                _22297.y = 1.0 - _14586.y;
                                _23510 = _22297;
                            }
                            else
                            {
                                _23510 = _14586;
                            }
                            _23518 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23507, 0.0) * _14563.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23508, 0.0) * _14566.x)) * _14563.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23509, 0.0) * _14563.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23510, 0.0) * _14566.x)) * _14566.y);
                        }
                        else
                        {
                            vec4 _23519;
                            if (_14171 == 3)
                            {
                                vec2 _14724 = (_7186 * _178.gLevel[3].xy) - vec2(0.5);
                                vec2 _14726 = floor(_14724);
                                vec2 _14729 = _14724 - _14726;
                                vec2 _14732 = _14729 * _14729;
                                vec2 _14735 = _14732 * _14729;
                                vec2 _14754 = (((_14735 * 3.0) - (_14732 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _14767 = _14735 * 0.16666667163372039794921875;
                                vec2 _14770 = (((((-_14735) + (_14732 * 3.0)) - (_14729 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14754;
                                vec2 _14773 = (((((_14735 * (-3.0)) + (_14732 * 3.0)) + (_14729 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14767;
                                vec2 _14783 = ((_14726 - vec2(0.5)) + (_14754 / _14770)) * _178.gLevel[3].zw;
                                vec2 _14793 = ((_14726 + vec2(1.5)) + (_14767 / _14773)) * _178.gLevel[3].zw;
                                vec2 _23503;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22304 = _14783;
                                    _22304.y = 1.0 - _14783.y;
                                    _23503 = _22304;
                                }
                                else
                                {
                                    _23503 = _14783;
                                }
                                float _14813 = _14783.y;
                                vec2 _14814 = vec2(_14793.x, _14813);
                                vec2 _23504;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22310 = _14814;
                                    _22310.y = 1.0 - _14813;
                                    _23504 = _22310;
                                }
                                else
                                {
                                    _23504 = _14814;
                                }
                                float _14830 = _14793.y;
                                vec2 _14831 = vec2(_14783.x, _14830);
                                vec2 _23505;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22317 = _14831;
                                    _22317.y = 1.0 - _14830;
                                    _23505 = _22317;
                                }
                                else
                                {
                                    _23505 = _14831;
                                }
                                vec2 _23506;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22323 = _14793;
                                    _22323.y = 1.0 - _14793.y;
                                    _23506 = _22323;
                                }
                                else
                                {
                                    _23506 = _14793;
                                }
                                _23519 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23503, 0.0) * _14770.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23504, 0.0) * _14773.x)) * _14770.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23505, 0.0) * _14770.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23506, 0.0) * _14773.x)) * _14773.y);
                            }
                            else
                            {
                                vec4 _23520;
                                if (_14171 == 4)
                                {
                                    vec2 _14931 = (_7186 * _178.gLevel[4].xy) - vec2(0.5);
                                    vec2 _14933 = floor(_14931);
                                    vec2 _14936 = _14931 - _14933;
                                    vec2 _14939 = _14936 * _14936;
                                    vec2 _14942 = _14939 * _14936;
                                    vec2 _14961 = (((_14942 * 3.0) - (_14939 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _14974 = _14942 * 0.16666667163372039794921875;
                                    vec2 _14977 = (((((-_14942) + (_14939 * 3.0)) - (_14936 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14961;
                                    vec2 _14980 = (((((_14942 * (-3.0)) + (_14939 * 3.0)) + (_14936 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14974;
                                    vec2 _14990 = ((_14933 - vec2(0.5)) + (_14961 / _14977)) * _178.gLevel[4].zw;
                                    vec2 _15000 = ((_14933 + vec2(1.5)) + (_14974 / _14980)) * _178.gLevel[4].zw;
                                    vec2 _23499;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22330 = _14990;
                                        _22330.y = 1.0 - _14990.y;
                                        _23499 = _22330;
                                    }
                                    else
                                    {
                                        _23499 = _14990;
                                    }
                                    float _15020 = _14990.y;
                                    vec2 _15021 = vec2(_15000.x, _15020);
                                    vec2 _23500;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22336 = _15021;
                                        _22336.y = 1.0 - _15020;
                                        _23500 = _22336;
                                    }
                                    else
                                    {
                                        _23500 = _15021;
                                    }
                                    float _15037 = _15000.y;
                                    vec2 _15038 = vec2(_14990.x, _15037);
                                    vec2 _23501;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22343 = _15038;
                                        _22343.y = 1.0 - _15037;
                                        _23501 = _22343;
                                    }
                                    else
                                    {
                                        _23501 = _15038;
                                    }
                                    vec2 _23502;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22349 = _15000;
                                        _22349.y = 1.0 - _15000.y;
                                        _23502 = _22349;
                                    }
                                    else
                                    {
                                        _23502 = _15000;
                                    }
                                    _23520 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23499, 0.0) * _14977.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23500, 0.0) * _14980.x)) * _14977.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23501, 0.0) * _14977.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23502, 0.0) * _14980.x)) * _14980.y);
                                }
                                else
                                {
                                    vec2 _15138 = (_7186 * _178.gLevel[5].xy) - vec2(0.5);
                                    vec2 _15140 = floor(_15138);
                                    vec2 _15143 = _15138 - _15140;
                                    vec2 _15146 = _15143 * _15143;
                                    vec2 _15149 = _15146 * _15143;
                                    vec2 _15168 = (((_15149 * 3.0) - (_15146 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _15181 = _15149 * 0.16666667163372039794921875;
                                    vec2 _15184 = (((((-_15149) + (_15146 * 3.0)) - (_15143 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15168;
                                    vec2 _15187 = (((((_15149 * (-3.0)) + (_15146 * 3.0)) + (_15143 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15181;
                                    vec2 _15197 = ((_15140 - vec2(0.5)) + (_15168 / _15184)) * _178.gLevel[5].zw;
                                    vec2 _15207 = ((_15140 + vec2(1.5)) + (_15181 / _15187)) * _178.gLevel[5].zw;
                                    vec2 _23495;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22356 = _15197;
                                        _22356.y = 1.0 - _15197.y;
                                        _23495 = _22356;
                                    }
                                    else
                                    {
                                        _23495 = _15197;
                                    }
                                    float _15227 = _15197.y;
                                    vec2 _15228 = vec2(_15207.x, _15227);
                                    vec2 _23496;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22362 = _15228;
                                        _22362.y = 1.0 - _15227;
                                        _23496 = _22362;
                                    }
                                    else
                                    {
                                        _23496 = _15228;
                                    }
                                    float _15244 = _15207.y;
                                    vec2 _15245 = vec2(_15197.x, _15244);
                                    vec2 _23497;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22369 = _15245;
                                        _22369.y = 1.0 - _15244;
                                        _23497 = _22369;
                                    }
                                    else
                                    {
                                        _23497 = _15245;
                                    }
                                    vec2 _23498;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22375 = _15207;
                                        _22375.y = 1.0 - _15207.y;
                                        _23498 = _22375;
                                    }
                                    else
                                    {
                                        _23498 = _15207;
                                    }
                                    _23520 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23495, 0.0) * _15184.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23496, 0.0) * _15187.x)) * _15184.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23497, 0.0) * _15184.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23498, 0.0) * _15187.x)) * _15187.y);
                                }
                                _23519 = _23520;
                            }
                            _23518 = _23519;
                        }
                        _23517 = _23518;
                    }
                    _23516 = _23517;
                }
                vec3 _23547;
                SPIRV_CROSS_BRANCH
                if ((_14175 > 0.0199999995529651641845703125) && (_14171 < 5))
                {
                    int _14188 = _14171 + 1;
                    vec4 _23542;
                    SPIRV_CROSS_BRANCH
                    if (_14188 <= 0)
                    {
                        vec2 _23541;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22380 = _7186;
                            _22380.y = 1.0 - _7186.y;
                            _23541 = _22380;
                        }
                        else
                        {
                            _23541 = _7186;
                        }
                        _23542 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23541, 0.0);
                    }
                    else
                    {
                        vec4 _23543;
                        if (_14188 == 1)
                        {
                            vec2 _15434 = (_7186 * _178.gLevel[1].xy) - vec2(0.5);
                            vec2 _15436 = floor(_15434);
                            vec2 _15439 = _15434 - _15436;
                            vec2 _15442 = _15439 * _15439;
                            vec2 _15445 = _15442 * _15439;
                            vec2 _15464 = (((_15445 * 3.0) - (_15442 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _15477 = _15445 * 0.16666667163372039794921875;
                            vec2 _15480 = (((((-_15445) + (_15442 * 3.0)) - (_15439 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15464;
                            vec2 _15483 = (((((_15445 * (-3.0)) + (_15442 * 3.0)) + (_15439 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15477;
                            vec2 _15493 = ((_15436 - vec2(0.5)) + (_15464 / _15480)) * _178.gLevel[1].zw;
                            vec2 _15503 = ((_15436 + vec2(1.5)) + (_15477 / _15483)) * _178.gLevel[1].zw;
                            vec2 _23537;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22385 = _15493;
                                _22385.y = 1.0 - _15493.y;
                                _23537 = _22385;
                            }
                            else
                            {
                                _23537 = _15493;
                            }
                            float _15523 = _15493.y;
                            vec2 _15524 = vec2(_15503.x, _15523);
                            vec2 _23538;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22391 = _15524;
                                _22391.y = 1.0 - _15523;
                                _23538 = _22391;
                            }
                            else
                            {
                                _23538 = _15524;
                            }
                            float _15540 = _15503.y;
                            vec2 _15541 = vec2(_15493.x, _15540);
                            vec2 _23539;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22398 = _15541;
                                _22398.y = 1.0 - _15540;
                                _23539 = _22398;
                            }
                            else
                            {
                                _23539 = _15541;
                            }
                            vec2 _23540;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22404 = _15503;
                                _22404.y = 1.0 - _15503.y;
                                _23540 = _22404;
                            }
                            else
                            {
                                _23540 = _15503;
                            }
                            _23543 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23537, 0.0) * _15480.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23538, 0.0) * _15483.x)) * _15480.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23539, 0.0) * _15480.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23540, 0.0) * _15483.x)) * _15483.y);
                        }
                        else
                        {
                            vec4 _23544;
                            if (_14188 == 2)
                            {
                                vec2 _15641 = (_7186 * _178.gLevel[2].xy) - vec2(0.5);
                                vec2 _15643 = floor(_15641);
                                vec2 _15646 = _15641 - _15643;
                                vec2 _15649 = _15646 * _15646;
                                vec2 _15652 = _15649 * _15646;
                                vec2 _15671 = (((_15652 * 3.0) - (_15649 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _15684 = _15652 * 0.16666667163372039794921875;
                                vec2 _15687 = (((((-_15652) + (_15649 * 3.0)) - (_15646 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15671;
                                vec2 _15690 = (((((_15652 * (-3.0)) + (_15649 * 3.0)) + (_15646 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15684;
                                vec2 _15700 = ((_15643 - vec2(0.5)) + (_15671 / _15687)) * _178.gLevel[2].zw;
                                vec2 _15710 = ((_15643 + vec2(1.5)) + (_15684 / _15690)) * _178.gLevel[2].zw;
                                vec2 _23533;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22411 = _15700;
                                    _22411.y = 1.0 - _15700.y;
                                    _23533 = _22411;
                                }
                                else
                                {
                                    _23533 = _15700;
                                }
                                float _15730 = _15700.y;
                                vec2 _15731 = vec2(_15710.x, _15730);
                                vec2 _23534;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22417 = _15731;
                                    _22417.y = 1.0 - _15730;
                                    _23534 = _22417;
                                }
                                else
                                {
                                    _23534 = _15731;
                                }
                                float _15747 = _15710.y;
                                vec2 _15748 = vec2(_15700.x, _15747);
                                vec2 _23535;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22424 = _15748;
                                    _22424.y = 1.0 - _15747;
                                    _23535 = _22424;
                                }
                                else
                                {
                                    _23535 = _15748;
                                }
                                vec2 _23536;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22430 = _15710;
                                    _22430.y = 1.0 - _15710.y;
                                    _23536 = _22430;
                                }
                                else
                                {
                                    _23536 = _15710;
                                }
                                _23544 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23533, 0.0) * _15687.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23534, 0.0) * _15690.x)) * _15687.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23535, 0.0) * _15687.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23536, 0.0) * _15690.x)) * _15690.y);
                            }
                            else
                            {
                                vec4 _23545;
                                if (_14188 == 3)
                                {
                                    vec2 _15848 = (_7186 * _178.gLevel[3].xy) - vec2(0.5);
                                    vec2 _15850 = floor(_15848);
                                    vec2 _15853 = _15848 - _15850;
                                    vec2 _15856 = _15853 * _15853;
                                    vec2 _15859 = _15856 * _15853;
                                    vec2 _15878 = (((_15859 * 3.0) - (_15856 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _15891 = _15859 * 0.16666667163372039794921875;
                                    vec2 _15894 = (((((-_15859) + (_15856 * 3.0)) - (_15853 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15878;
                                    vec2 _15897 = (((((_15859 * (-3.0)) + (_15856 * 3.0)) + (_15853 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15891;
                                    vec2 _15907 = ((_15850 - vec2(0.5)) + (_15878 / _15894)) * _178.gLevel[3].zw;
                                    vec2 _15917 = ((_15850 + vec2(1.5)) + (_15891 / _15897)) * _178.gLevel[3].zw;
                                    vec2 _23529;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22437 = _15907;
                                        _22437.y = 1.0 - _15907.y;
                                        _23529 = _22437;
                                    }
                                    else
                                    {
                                        _23529 = _15907;
                                    }
                                    float _15937 = _15907.y;
                                    vec2 _15938 = vec2(_15917.x, _15937);
                                    vec2 _23530;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22443 = _15938;
                                        _22443.y = 1.0 - _15937;
                                        _23530 = _22443;
                                    }
                                    else
                                    {
                                        _23530 = _15938;
                                    }
                                    float _15954 = _15917.y;
                                    vec2 _15955 = vec2(_15907.x, _15954);
                                    vec2 _23531;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22450 = _15955;
                                        _22450.y = 1.0 - _15954;
                                        _23531 = _22450;
                                    }
                                    else
                                    {
                                        _23531 = _15955;
                                    }
                                    vec2 _23532;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22456 = _15917;
                                        _22456.y = 1.0 - _15917.y;
                                        _23532 = _22456;
                                    }
                                    else
                                    {
                                        _23532 = _15917;
                                    }
                                    _23545 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23529, 0.0) * _15894.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23530, 0.0) * _15897.x)) * _15894.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23531, 0.0) * _15894.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23532, 0.0) * _15897.x)) * _15897.y);
                                }
                                else
                                {
                                    vec4 _23546;
                                    if (_14188 == 4)
                                    {
                                        vec2 _16055 = (_7186 * _178.gLevel[4].xy) - vec2(0.5);
                                        vec2 _16057 = floor(_16055);
                                        vec2 _16060 = _16055 - _16057;
                                        vec2 _16063 = _16060 * _16060;
                                        vec2 _16066 = _16063 * _16060;
                                        vec2 _16085 = (((_16066 * 3.0) - (_16063 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _16098 = _16066 * 0.16666667163372039794921875;
                                        vec2 _16101 = (((((-_16066) + (_16063 * 3.0)) - (_16060 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16085;
                                        vec2 _16104 = (((((_16066 * (-3.0)) + (_16063 * 3.0)) + (_16060 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16098;
                                        vec2 _16114 = ((_16057 - vec2(0.5)) + (_16085 / _16101)) * _178.gLevel[4].zw;
                                        vec2 _16124 = ((_16057 + vec2(1.5)) + (_16098 / _16104)) * _178.gLevel[4].zw;
                                        vec2 _23525;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22463 = _16114;
                                            _22463.y = 1.0 - _16114.y;
                                            _23525 = _22463;
                                        }
                                        else
                                        {
                                            _23525 = _16114;
                                        }
                                        float _16144 = _16114.y;
                                        vec2 _16145 = vec2(_16124.x, _16144);
                                        vec2 _23526;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22469 = _16145;
                                            _22469.y = 1.0 - _16144;
                                            _23526 = _22469;
                                        }
                                        else
                                        {
                                            _23526 = _16145;
                                        }
                                        float _16161 = _16124.y;
                                        vec2 _16162 = vec2(_16114.x, _16161);
                                        vec2 _23527;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22476 = _16162;
                                            _22476.y = 1.0 - _16161;
                                            _23527 = _22476;
                                        }
                                        else
                                        {
                                            _23527 = _16162;
                                        }
                                        vec2 _23528;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22482 = _16124;
                                            _22482.y = 1.0 - _16124.y;
                                            _23528 = _22482;
                                        }
                                        else
                                        {
                                            _23528 = _16124;
                                        }
                                        _23546 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23525, 0.0) * _16101.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23526, 0.0) * _16104.x)) * _16101.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23527, 0.0) * _16101.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23528, 0.0) * _16104.x)) * _16104.y);
                                    }
                                    else
                                    {
                                        vec2 _16262 = (_7186 * _178.gLevel[5].xy) - vec2(0.5);
                                        vec2 _16264 = floor(_16262);
                                        vec2 _16267 = _16262 - _16264;
                                        vec2 _16270 = _16267 * _16267;
                                        vec2 _16273 = _16270 * _16267;
                                        vec2 _16292 = (((_16273 * 3.0) - (_16270 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _16305 = _16273 * 0.16666667163372039794921875;
                                        vec2 _16308 = (((((-_16273) + (_16270 * 3.0)) - (_16267 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16292;
                                        vec2 _16311 = (((((_16273 * (-3.0)) + (_16270 * 3.0)) + (_16267 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16305;
                                        vec2 _16321 = ((_16264 - vec2(0.5)) + (_16292 / _16308)) * _178.gLevel[5].zw;
                                        vec2 _16331 = ((_16264 + vec2(1.5)) + (_16305 / _16311)) * _178.gLevel[5].zw;
                                        vec2 _23521;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22489 = _16321;
                                            _22489.y = 1.0 - _16321.y;
                                            _23521 = _22489;
                                        }
                                        else
                                        {
                                            _23521 = _16321;
                                        }
                                        float _16351 = _16321.y;
                                        vec2 _16352 = vec2(_16331.x, _16351);
                                        vec2 _23522;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22495 = _16352;
                                            _22495.y = 1.0 - _16351;
                                            _23522 = _22495;
                                        }
                                        else
                                        {
                                            _23522 = _16352;
                                        }
                                        float _16368 = _16331.y;
                                        vec2 _16369 = vec2(_16321.x, _16368);
                                        vec2 _23523;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22502 = _16369;
                                            _22502.y = 1.0 - _16368;
                                            _23523 = _22502;
                                        }
                                        else
                                        {
                                            _23523 = _16369;
                                        }
                                        vec2 _23524;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22508 = _16331;
                                            _22508.y = 1.0 - _16331.y;
                                            _23524 = _22508;
                                        }
                                        else
                                        {
                                            _23524 = _16331;
                                        }
                                        _23546 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23521, 0.0) * _16308.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23522, 0.0) * _16311.x)) * _16308.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23523, 0.0) * _16308.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23524, 0.0) * _16311.x)) * _16311.y);
                                    }
                                    _23545 = _23546;
                                }
                                _23544 = _23545;
                            }
                            _23543 = _23544;
                        }
                        _23542 = _23543;
                    }
                    _23547 = mix(_23516.xyz, _23542.xyz, vec3(_14175));
                }
                else
                {
                    _23547 = _23516.xyz;
                }
                _23837 = vec3(_23335.x, _23441.y, _23547.z);
            }
            else
            {
                vec2 _7194 = _7124 + _7136;
                float _16459 = clamp(log2(max(_7038, 1.0)) - 1.0, 0.0, 5.0);
                int _16462 = int(floor(_16459));
                float _16466 = _16459 - float(_16462);
                vec4 _23251;
                SPIRV_CROSS_BRANCH
                if (_16462 <= 0)
                {
                    vec2 _23250;
                    if (_178.gConv.x > 0.5)
                    {
                        vec2 _22515 = _7194;
                        _22515.y = 1.0 - _7194.y;
                        _23250 = _22515;
                    }
                    else
                    {
                        _23250 = _7194;
                    }
                    _23251 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23250, 0.0);
                }
                else
                {
                    vec4 _23252;
                    if (_16462 == 1)
                    {
                        vec2 _16601 = (_7194 * _178.gLevel[1].xy) - vec2(0.5);
                        vec2 _16603 = floor(_16601);
                        vec2 _16606 = _16601 - _16603;
                        vec2 _16609 = _16606 * _16606;
                        vec2 _16612 = _16609 * _16606;
                        vec2 _16631 = (((_16612 * 3.0) - (_16609 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                        vec2 _16644 = _16612 * 0.16666667163372039794921875;
                        vec2 _16647 = (((((-_16612) + (_16609 * 3.0)) - (_16606 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16631;
                        vec2 _16650 = (((((_16612 * (-3.0)) + (_16609 * 3.0)) + (_16606 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16644;
                        vec2 _16660 = ((_16603 - vec2(0.5)) + (_16631 / _16647)) * _178.gLevel[1].zw;
                        vec2 _16670 = ((_16603 + vec2(1.5)) + (_16644 / _16650)) * _178.gLevel[1].zw;
                        vec2 _23246;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22520 = _16660;
                            _22520.y = 1.0 - _16660.y;
                            _23246 = _22520;
                        }
                        else
                        {
                            _23246 = _16660;
                        }
                        vec2 _16691 = vec2(_16670.x, _16660.y);
                        vec2 _23247;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22526 = _16691;
                            _22526.y = 1.0 - _16660.y;
                            _23247 = _22526;
                        }
                        else
                        {
                            _23247 = _16691;
                        }
                        float _16707 = _16670.y;
                        vec2 _16708 = vec2(_16660.x, _16707);
                        vec2 _23248;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22533 = _16708;
                            _22533.y = 1.0 - _16707;
                            _23248 = _22533;
                        }
                        else
                        {
                            _23248 = _16708;
                        }
                        vec2 _23249;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22539 = _16670;
                            _22539.y = 1.0 - _16670.y;
                            _23249 = _22539;
                        }
                        else
                        {
                            _23249 = _16670;
                        }
                        _23252 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23246, 0.0) * _16647.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23247, 0.0) * _16650.x)) * _16647.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23248, 0.0) * _16647.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23249, 0.0) * _16650.x)) * _16650.y);
                    }
                    else
                    {
                        vec4 _23253;
                        if (_16462 == 2)
                        {
                            vec2 _16808 = (_7194 * _178.gLevel[2].xy) - vec2(0.5);
                            vec2 _16810 = floor(_16808);
                            vec2 _16813 = _16808 - _16810;
                            vec2 _16816 = _16813 * _16813;
                            vec2 _16819 = _16816 * _16813;
                            vec2 _16838 = (((_16819 * 3.0) - (_16816 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _16851 = _16819 * 0.16666667163372039794921875;
                            vec2 _16854 = (((((-_16819) + (_16816 * 3.0)) - (_16813 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16838;
                            vec2 _16857 = (((((_16819 * (-3.0)) + (_16816 * 3.0)) + (_16813 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16851;
                            vec2 _16867 = ((_16810 - vec2(0.5)) + (_16838 / _16854)) * _178.gLevel[2].zw;
                            vec2 _16877 = ((_16810 + vec2(1.5)) + (_16851 / _16857)) * _178.gLevel[2].zw;
                            vec2 _23242;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22546 = _16867;
                                _22546.y = 1.0 - _16867.y;
                                _23242 = _22546;
                            }
                            else
                            {
                                _23242 = _16867;
                            }
                            vec2 _16898 = vec2(_16877.x, _16867.y);
                            vec2 _23243;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22552 = _16898;
                                _22552.y = 1.0 - _16867.y;
                                _23243 = _22552;
                            }
                            else
                            {
                                _23243 = _16898;
                            }
                            float _16914 = _16877.y;
                            vec2 _16915 = vec2(_16867.x, _16914);
                            vec2 _23244;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22559 = _16915;
                                _22559.y = 1.0 - _16914;
                                _23244 = _22559;
                            }
                            else
                            {
                                _23244 = _16915;
                            }
                            vec2 _23245;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22565 = _16877;
                                _22565.y = 1.0 - _16877.y;
                                _23245 = _22565;
                            }
                            else
                            {
                                _23245 = _16877;
                            }
                            _23253 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23242, 0.0) * _16854.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23243, 0.0) * _16857.x)) * _16854.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23244, 0.0) * _16854.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23245, 0.0) * _16857.x)) * _16857.y);
                        }
                        else
                        {
                            vec4 _23254;
                            if (_16462 == 3)
                            {
                                vec2 _17015 = (_7194 * _178.gLevel[3].xy) - vec2(0.5);
                                vec2 _17017 = floor(_17015);
                                vec2 _17020 = _17015 - _17017;
                                vec2 _17023 = _17020 * _17020;
                                vec2 _17026 = _17023 * _17020;
                                vec2 _17045 = (((_17026 * 3.0) - (_17023 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _17058 = _17026 * 0.16666667163372039794921875;
                                vec2 _17061 = (((((-_17026) + (_17023 * 3.0)) - (_17020 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17045;
                                vec2 _17064 = (((((_17026 * (-3.0)) + (_17023 * 3.0)) + (_17020 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17058;
                                vec2 _17074 = ((_17017 - vec2(0.5)) + (_17045 / _17061)) * _178.gLevel[3].zw;
                                vec2 _17084 = ((_17017 + vec2(1.5)) + (_17058 / _17064)) * _178.gLevel[3].zw;
                                vec2 _23238;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22572 = _17074;
                                    _22572.y = 1.0 - _17074.y;
                                    _23238 = _22572;
                                }
                                else
                                {
                                    _23238 = _17074;
                                }
                                vec2 _17105 = vec2(_17084.x, _17074.y);
                                vec2 _23239;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22578 = _17105;
                                    _22578.y = 1.0 - _17074.y;
                                    _23239 = _22578;
                                }
                                else
                                {
                                    _23239 = _17105;
                                }
                                float _17121 = _17084.y;
                                vec2 _17122 = vec2(_17074.x, _17121);
                                vec2 _23240;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22585 = _17122;
                                    _22585.y = 1.0 - _17121;
                                    _23240 = _22585;
                                }
                                else
                                {
                                    _23240 = _17122;
                                }
                                vec2 _23241;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22591 = _17084;
                                    _22591.y = 1.0 - _17084.y;
                                    _23241 = _22591;
                                }
                                else
                                {
                                    _23241 = _17084;
                                }
                                _23254 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23238, 0.0) * _17061.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23239, 0.0) * _17064.x)) * _17061.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23240, 0.0) * _17061.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23241, 0.0) * _17064.x)) * _17064.y);
                            }
                            else
                            {
                                vec4 _23255;
                                if (_16462 == 4)
                                {
                                    vec2 _17222 = (_7194 * _178.gLevel[4].xy) - vec2(0.5);
                                    vec2 _17224 = floor(_17222);
                                    vec2 _17227 = _17222 - _17224;
                                    vec2 _17230 = _17227 * _17227;
                                    vec2 _17233 = _17230 * _17227;
                                    vec2 _17252 = (((_17233 * 3.0) - (_17230 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _17265 = _17233 * 0.16666667163372039794921875;
                                    vec2 _17268 = (((((-_17233) + (_17230 * 3.0)) - (_17227 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17252;
                                    vec2 _17271 = (((((_17233 * (-3.0)) + (_17230 * 3.0)) + (_17227 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17265;
                                    vec2 _17281 = ((_17224 - vec2(0.5)) + (_17252 / _17268)) * _178.gLevel[4].zw;
                                    vec2 _17291 = ((_17224 + vec2(1.5)) + (_17265 / _17271)) * _178.gLevel[4].zw;
                                    vec2 _23234;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22598 = _17281;
                                        _22598.y = 1.0 - _17281.y;
                                        _23234 = _22598;
                                    }
                                    else
                                    {
                                        _23234 = _17281;
                                    }
                                    vec2 _17312 = vec2(_17291.x, _17281.y);
                                    vec2 _23235;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22604 = _17312;
                                        _22604.y = 1.0 - _17281.y;
                                        _23235 = _22604;
                                    }
                                    else
                                    {
                                        _23235 = _17312;
                                    }
                                    float _17328 = _17291.y;
                                    vec2 _17329 = vec2(_17281.x, _17328);
                                    vec2 _23236;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22611 = _17329;
                                        _22611.y = 1.0 - _17328;
                                        _23236 = _22611;
                                    }
                                    else
                                    {
                                        _23236 = _17329;
                                    }
                                    vec2 _23237;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22617 = _17291;
                                        _22617.y = 1.0 - _17291.y;
                                        _23237 = _22617;
                                    }
                                    else
                                    {
                                        _23237 = _17291;
                                    }
                                    _23255 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23234, 0.0) * _17268.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23235, 0.0) * _17271.x)) * _17268.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23236, 0.0) * _17268.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23237, 0.0) * _17271.x)) * _17271.y);
                                }
                                else
                                {
                                    vec2 _17429 = (_7194 * _178.gLevel[5].xy) - vec2(0.5);
                                    vec2 _17431 = floor(_17429);
                                    vec2 _17434 = _17429 - _17431;
                                    vec2 _17437 = _17434 * _17434;
                                    vec2 _17440 = _17437 * _17434;
                                    vec2 _17459 = (((_17440 * 3.0) - (_17437 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _17472 = _17440 * 0.16666667163372039794921875;
                                    vec2 _17475 = (((((-_17440) + (_17437 * 3.0)) - (_17434 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17459;
                                    vec2 _17478 = (((((_17440 * (-3.0)) + (_17437 * 3.0)) + (_17434 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17472;
                                    vec2 _17488 = ((_17431 - vec2(0.5)) + (_17459 / _17475)) * _178.gLevel[5].zw;
                                    vec2 _17498 = ((_17431 + vec2(1.5)) + (_17472 / _17478)) * _178.gLevel[5].zw;
                                    vec2 _23230;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22624 = _17488;
                                        _22624.y = 1.0 - _17488.y;
                                        _23230 = _22624;
                                    }
                                    else
                                    {
                                        _23230 = _17488;
                                    }
                                    vec2 _17519 = vec2(_17498.x, _17488.y);
                                    vec2 _23231;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22630 = _17519;
                                        _22630.y = 1.0 - _17488.y;
                                        _23231 = _22630;
                                    }
                                    else
                                    {
                                        _23231 = _17519;
                                    }
                                    float _17535 = _17498.y;
                                    vec2 _17536 = vec2(_17488.x, _17535);
                                    vec2 _23232;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22637 = _17536;
                                        _22637.y = 1.0 - _17535;
                                        _23232 = _22637;
                                    }
                                    else
                                    {
                                        _23232 = _17536;
                                    }
                                    vec2 _23233;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22643 = _17498;
                                        _22643.y = 1.0 - _17498.y;
                                        _23233 = _22643;
                                    }
                                    else
                                    {
                                        _23233 = _17498;
                                    }
                                    _23255 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23230, 0.0) * _17475.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23231, 0.0) * _17478.x)) * _17475.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23232, 0.0) * _17475.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23233, 0.0) * _17478.x)) * _17478.y);
                                }
                                _23254 = _23255;
                            }
                            _23253 = _23254;
                        }
                        _23252 = _23253;
                    }
                    _23251 = _23252;
                }
                vec3 _23282;
                SPIRV_CROSS_BRANCH
                if ((_16466 > 0.0199999995529651641845703125) && (_16462 < 5))
                {
                    int _16479 = _16462 + 1;
                    vec4 _23277;
                    SPIRV_CROSS_BRANCH
                    if (_16479 <= 0)
                    {
                        vec2 _23276;
                        if (_178.gConv.x > 0.5)
                        {
                            vec2 _22648 = _7194;
                            _22648.y = 1.0 - _7194.y;
                            _23276 = _22648;
                        }
                        else
                        {
                            _23276 = _7194;
                        }
                        _23277 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23276, 0.0);
                    }
                    else
                    {
                        vec4 _23278;
                        if (_16479 == 1)
                        {
                            vec2 _17725 = (_7194 * _178.gLevel[1].xy) - vec2(0.5);
                            vec2 _17727 = floor(_17725);
                            vec2 _17730 = _17725 - _17727;
                            vec2 _17733 = _17730 * _17730;
                            vec2 _17736 = _17733 * _17730;
                            vec2 _17755 = (((_17736 * 3.0) - (_17733 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _17768 = _17736 * 0.16666667163372039794921875;
                            vec2 _17771 = (((((-_17736) + (_17733 * 3.0)) - (_17730 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17755;
                            vec2 _17774 = (((((_17736 * (-3.0)) + (_17733 * 3.0)) + (_17730 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17768;
                            vec2 _17784 = ((_17727 - vec2(0.5)) + (_17755 / _17771)) * _178.gLevel[1].zw;
                            vec2 _17794 = ((_17727 + vec2(1.5)) + (_17768 / _17774)) * _178.gLevel[1].zw;
                            vec2 _23272;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22653 = _17784;
                                _22653.y = 1.0 - _17784.y;
                                _23272 = _22653;
                            }
                            else
                            {
                                _23272 = _17784;
                            }
                            vec2 _17815 = vec2(_17794.x, _17784.y);
                            vec2 _23273;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22659 = _17815;
                                _22659.y = 1.0 - _17784.y;
                                _23273 = _22659;
                            }
                            else
                            {
                                _23273 = _17815;
                            }
                            float _17831 = _17794.y;
                            vec2 _17832 = vec2(_17784.x, _17831);
                            vec2 _23274;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22666 = _17832;
                                _22666.y = 1.0 - _17831;
                                _23274 = _22666;
                            }
                            else
                            {
                                _23274 = _17832;
                            }
                            vec2 _23275;
                            if (_178.gConv.x > 0.5)
                            {
                                vec2 _22672 = _17794;
                                _22672.y = 1.0 - _17794.y;
                                _23275 = _22672;
                            }
                            else
                            {
                                _23275 = _17794;
                            }
                            _23278 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23272, 0.0) * _17771.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23273, 0.0) * _17774.x)) * _17771.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23274, 0.0) * _17771.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23275, 0.0) * _17774.x)) * _17774.y);
                        }
                        else
                        {
                            vec4 _23279;
                            if (_16479 == 2)
                            {
                                vec2 _17932 = (_7194 * _178.gLevel[2].xy) - vec2(0.5);
                                vec2 _17934 = floor(_17932);
                                vec2 _17937 = _17932 - _17934;
                                vec2 _17940 = _17937 * _17937;
                                vec2 _17943 = _17940 * _17937;
                                vec2 _17962 = (((_17943 * 3.0) - (_17940 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _17975 = _17943 * 0.16666667163372039794921875;
                                vec2 _17978 = (((((-_17943) + (_17940 * 3.0)) - (_17937 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17962;
                                vec2 _17981 = (((((_17943 * (-3.0)) + (_17940 * 3.0)) + (_17937 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17975;
                                vec2 _17991 = ((_17934 - vec2(0.5)) + (_17962 / _17978)) * _178.gLevel[2].zw;
                                vec2 _18001 = ((_17934 + vec2(1.5)) + (_17975 / _17981)) * _178.gLevel[2].zw;
                                vec2 _23268;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22679 = _17991;
                                    _22679.y = 1.0 - _17991.y;
                                    _23268 = _22679;
                                }
                                else
                                {
                                    _23268 = _17991;
                                }
                                vec2 _18022 = vec2(_18001.x, _17991.y);
                                vec2 _23269;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22685 = _18022;
                                    _22685.y = 1.0 - _17991.y;
                                    _23269 = _22685;
                                }
                                else
                                {
                                    _23269 = _18022;
                                }
                                float _18038 = _18001.y;
                                vec2 _18039 = vec2(_17991.x, _18038);
                                vec2 _23270;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22692 = _18039;
                                    _22692.y = 1.0 - _18038;
                                    _23270 = _22692;
                                }
                                else
                                {
                                    _23270 = _18039;
                                }
                                vec2 _23271;
                                if (_178.gConv.x > 0.5)
                                {
                                    vec2 _22698 = _18001;
                                    _22698.y = 1.0 - _18001.y;
                                    _23271 = _22698;
                                }
                                else
                                {
                                    _23271 = _18001;
                                }
                                _23279 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23268, 0.0) * _17978.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23269, 0.0) * _17981.x)) * _17978.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23270, 0.0) * _17978.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23271, 0.0) * _17981.x)) * _17981.y);
                            }
                            else
                            {
                                vec4 _23280;
                                if (_16479 == 3)
                                {
                                    vec2 _18139 = (_7194 * _178.gLevel[3].xy) - vec2(0.5);
                                    vec2 _18141 = floor(_18139);
                                    vec2 _18144 = _18139 - _18141;
                                    vec2 _18147 = _18144 * _18144;
                                    vec2 _18150 = _18147 * _18144;
                                    vec2 _18169 = (((_18150 * 3.0) - (_18147 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _18182 = _18150 * 0.16666667163372039794921875;
                                    vec2 _18185 = (((((-_18150) + (_18147 * 3.0)) - (_18144 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18169;
                                    vec2 _18188 = (((((_18150 * (-3.0)) + (_18147 * 3.0)) + (_18144 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18182;
                                    vec2 _18198 = ((_18141 - vec2(0.5)) + (_18169 / _18185)) * _178.gLevel[3].zw;
                                    vec2 _18208 = ((_18141 + vec2(1.5)) + (_18182 / _18188)) * _178.gLevel[3].zw;
                                    vec2 _23264;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22705 = _18198;
                                        _22705.y = 1.0 - _18198.y;
                                        _23264 = _22705;
                                    }
                                    else
                                    {
                                        _23264 = _18198;
                                    }
                                    vec2 _18229 = vec2(_18208.x, _18198.y);
                                    vec2 _23265;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22711 = _18229;
                                        _22711.y = 1.0 - _18198.y;
                                        _23265 = _22711;
                                    }
                                    else
                                    {
                                        _23265 = _18229;
                                    }
                                    float _18245 = _18208.y;
                                    vec2 _18246 = vec2(_18198.x, _18245);
                                    vec2 _23266;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22718 = _18246;
                                        _22718.y = 1.0 - _18245;
                                        _23266 = _22718;
                                    }
                                    else
                                    {
                                        _23266 = _18246;
                                    }
                                    vec2 _23267;
                                    if (_178.gConv.x > 0.5)
                                    {
                                        vec2 _22724 = _18208;
                                        _22724.y = 1.0 - _18208.y;
                                        _23267 = _22724;
                                    }
                                    else
                                    {
                                        _23267 = _18208;
                                    }
                                    _23280 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23264, 0.0) * _18185.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23265, 0.0) * _18188.x)) * _18185.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23266, 0.0) * _18185.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23267, 0.0) * _18188.x)) * _18188.y);
                                }
                                else
                                {
                                    vec4 _23281;
                                    if (_16479 == 4)
                                    {
                                        vec2 _18346 = (_7194 * _178.gLevel[4].xy) - vec2(0.5);
                                        vec2 _18348 = floor(_18346);
                                        vec2 _18351 = _18346 - _18348;
                                        vec2 _18354 = _18351 * _18351;
                                        vec2 _18357 = _18354 * _18351;
                                        vec2 _18376 = (((_18357 * 3.0) - (_18354 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _18389 = _18357 * 0.16666667163372039794921875;
                                        vec2 _18392 = (((((-_18357) + (_18354 * 3.0)) - (_18351 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18376;
                                        vec2 _18395 = (((((_18357 * (-3.0)) + (_18354 * 3.0)) + (_18351 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18389;
                                        vec2 _18405 = ((_18348 - vec2(0.5)) + (_18376 / _18392)) * _178.gLevel[4].zw;
                                        vec2 _18415 = ((_18348 + vec2(1.5)) + (_18389 / _18395)) * _178.gLevel[4].zw;
                                        vec2 _23260;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22731 = _18405;
                                            _22731.y = 1.0 - _18405.y;
                                            _23260 = _22731;
                                        }
                                        else
                                        {
                                            _23260 = _18405;
                                        }
                                        vec2 _18436 = vec2(_18415.x, _18405.y);
                                        vec2 _23261;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22737 = _18436;
                                            _22737.y = 1.0 - _18405.y;
                                            _23261 = _22737;
                                        }
                                        else
                                        {
                                            _23261 = _18436;
                                        }
                                        float _18452 = _18415.y;
                                        vec2 _18453 = vec2(_18405.x, _18452);
                                        vec2 _23262;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22744 = _18453;
                                            _22744.y = 1.0 - _18452;
                                            _23262 = _22744;
                                        }
                                        else
                                        {
                                            _23262 = _18453;
                                        }
                                        vec2 _23263;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22750 = _18415;
                                            _22750.y = 1.0 - _18415.y;
                                            _23263 = _22750;
                                        }
                                        else
                                        {
                                            _23263 = _18415;
                                        }
                                        _23281 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23260, 0.0) * _18392.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23261, 0.0) * _18395.x)) * _18392.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23262, 0.0) * _18392.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23263, 0.0) * _18395.x)) * _18395.y);
                                    }
                                    else
                                    {
                                        vec2 _18553 = (_7194 * _178.gLevel[5].xy) - vec2(0.5);
                                        vec2 _18555 = floor(_18553);
                                        vec2 _18558 = _18553 - _18555;
                                        vec2 _18561 = _18558 * _18558;
                                        vec2 _18564 = _18561 * _18558;
                                        vec2 _18583 = (((_18564 * 3.0) - (_18561 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _18596 = _18564 * 0.16666667163372039794921875;
                                        vec2 _18599 = (((((-_18564) + (_18561 * 3.0)) - (_18558 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18583;
                                        vec2 _18602 = (((((_18564 * (-3.0)) + (_18561 * 3.0)) + (_18558 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18596;
                                        vec2 _18612 = ((_18555 - vec2(0.5)) + (_18583 / _18599)) * _178.gLevel[5].zw;
                                        vec2 _18622 = ((_18555 + vec2(1.5)) + (_18596 / _18602)) * _178.gLevel[5].zw;
                                        vec2 _23256;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22757 = _18612;
                                            _22757.y = 1.0 - _18612.y;
                                            _23256 = _22757;
                                        }
                                        else
                                        {
                                            _23256 = _18612;
                                        }
                                        vec2 _18643 = vec2(_18622.x, _18612.y);
                                        vec2 _23257;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22763 = _18643;
                                            _22763.y = 1.0 - _18612.y;
                                            _23257 = _22763;
                                        }
                                        else
                                        {
                                            _23257 = _18643;
                                        }
                                        float _18659 = _18622.y;
                                        vec2 _18660 = vec2(_18612.x, _18659);
                                        vec2 _23258;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22770 = _18660;
                                            _22770.y = 1.0 - _18659;
                                            _23258 = _22770;
                                        }
                                        else
                                        {
                                            _23258 = _18660;
                                        }
                                        vec2 _23259;
                                        if (_178.gConv.x > 0.5)
                                        {
                                            vec2 _22776 = _18622;
                                            _22776.y = 1.0 - _18622.y;
                                            _23259 = _22776;
                                        }
                                        else
                                        {
                                            _23259 = _18622;
                                        }
                                        _23281 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23256, 0.0) * _18599.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23257, 0.0) * _18602.x)) * _18599.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23258, 0.0) * _18599.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23259, 0.0) * _18602.x)) * _18602.y);
                                    }
                                    _23280 = _23281;
                                }
                                _23279 = _23280;
                            }
                            _23278 = _23279;
                        }
                        _23277 = _23278;
                    }
                    _23282 = mix(_23251.xyz, _23277.xyz, vec3(_16466));
                }
                else
                {
                    _23282 = _23251.xyz;
                }
                _23837 = _23282;
            }
            vec3 _24328;
            SPIRV_CROSS_BRANCH
            if (_7066 > 0.0)
            {
                vec2 _7215 = _7124 + (((_23228 * min(_7057 * 0.5, 16.0)) * _178.gDisplay.zw) * _178.gTarget.zw);
                float _18750 = clamp(log2(max(16.0 * _178.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _18753 = int(floor(_18750));
                float _18757 = _18750 - float(_18753);
                vec2 _23815;
                if (_178.gConv.x > 0.5)
                {
                    vec2 _22781 = _7215;
                    _22781.y = 1.0 - _7215.y;
                    _23815 = _22781;
                }
                else
                {
                    _23815 = _7215;
                }
                vec4 _23816;
                SPIRV_CROSS_BRANCH
                if (_18753 <= 0)
                {
                    _23816 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23815, 0.0);
                }
                else
                {
                    vec4 _23817;
                    if (_18753 == 1)
                    {
                        _23817 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23815, 0.0);
                    }
                    else
                    {
                        vec4 _23818;
                        if (_18753 == 2)
                        {
                            _23818 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23815, 0.0);
                        }
                        else
                        {
                            vec4 _23819;
                            if (_18753 == 3)
                            {
                                _23819 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23815, 0.0);
                            }
                            else
                            {
                                vec4 _23820;
                                if (_18753 == 4)
                                {
                                    _23820 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23815, 0.0);
                                }
                                else
                                {
                                    _23820 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23815, 0.0);
                                }
                                _23819 = _23820;
                            }
                            _23818 = _23819;
                        }
                        _23817 = _23818;
                    }
                    _23816 = _23817;
                }
                vec3 _23827;
                SPIRV_CROSS_BRANCH
                if ((_18757 > 0.0199999995529651641845703125) && (_18753 < 5))
                {
                    int _18770 = _18753 + 1;
                    vec2 _23821;
                    if (_178.gConv.x > 0.5)
                    {
                        vec2 _22784 = _7215;
                        _22784.y = 1.0 - _7215.y;
                        _23821 = _22784;
                    }
                    else
                    {
                        _23821 = _7215;
                    }
                    vec4 _23822;
                    SPIRV_CROSS_BRANCH
                    if (_18770 <= 0)
                    {
                        _23822 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23821, 0.0);
                    }
                    else
                    {
                        vec4 _23823;
                        if (_18770 == 1)
                        {
                            _23823 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23821, 0.0);
                        }
                        else
                        {
                            vec4 _23824;
                            if (_18770 == 2)
                            {
                                _23824 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23821, 0.0);
                            }
                            else
                            {
                                vec4 _23825;
                                if (_18770 == 3)
                                {
                                    _23825 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23821, 0.0);
                                }
                                else
                                {
                                    vec4 _23826;
                                    if (_18770 == 4)
                                    {
                                        _23826 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23821, 0.0);
                                    }
                                    else
                                    {
                                        _23826 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23821, 0.0);
                                    }
                                    _23825 = _23826;
                                }
                                _23824 = _23825;
                            }
                            _23823 = _23824;
                        }
                        _23822 = _23823;
                    }
                    _23827 = mix(_23816.xyz, _23822.xyz, vec3(_18757));
                }
                else
                {
                    _23827 = _23816.xyz;
                }
                _24328 = _23827;
            }
            else
            {
                _24328 = vec3(0.5);
            }
            float _23858;
            SPIRV_CROSS_BRANCH
            if (esia_v5.x > 0.001000000047497451305389404296875)
            {
                int _18937 = clamp(int(roundEven(log2(36.0 * _178.gDisplay.z) - 1.0)), 1, 4);
                vec2 _23828;
                if (_178.gConv.x > 0.5)
                {
                    vec2 _22788 = _7124;
                    _22788.y = 1.0 - _7124.y;
                    _23828 = _22788;
                }
                else
                {
                    _23828 = _7124;
                }
                vec4 _23829;
                SPIRV_CROSS_BRANCH
                if (_18937 <= 0)
                {
                    _23829 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23828, 0.0);
                }
                else
                {
                    vec4 _23830;
                    if (_18937 == 1)
                    {
                        _23830 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23828, 0.0);
                    }
                    else
                    {
                        vec4 _23831;
                        if (_18937 == 2)
                        {
                            _23831 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23828, 0.0);
                        }
                        else
                        {
                            vec4 _23832;
                            if (_18937 == 3)
                            {
                                _23832 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23828, 0.0);
                            }
                            else
                            {
                                vec4 _23833;
                                if (_18937 == 4)
                                {
                                    _23833 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23828, 0.0);
                                }
                                else
                                {
                                    _23833 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23828, 0.0);
                                }
                                _23832 = _23833;
                            }
                            _23831 = _23832;
                        }
                        _23830 = _23831;
                    }
                    _23829 = _23830;
                }
                _23858 = dot(_23829.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _23858 = 0.5;
            }
            _24326 = _24328;
            _23857 = _23858;
            _23834 = _23837;
        }
        else
        {
            _24326 = vec3(0.5);
            _23857 = 0.5;
            _23834 = vec3(0.5);
        }
        vec3 _7243 = mix(vec3(dot(_23834, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _23834, vec3(_6932.x)) + vec3(_6932.y);
        vec3 _24086;
        SPIRV_CROSS_BRANCH
        if (esia_v5.x > 0.001000000047497451305389404296875)
        {
            float _7253 = clamp(max(_23857 + _6932.y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _7261 = mix(_7253, dot(_6891.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), esia_v5.x);
            vec3 _7269 = _7243 * (_7261 / _7253);
            vec3 _7279 = mix(_7243, vec3(1.0), vec3((_7261 - _7253) / max(1.0 - _7253, 0.001000000047497451305389404296875)));
            bvec3 _7280 = bvec3(_7261 < _7253);
            _24086 = vec3(_7280.x ? _7269.x : _7279.x, _7280.y ? _7269.y : _7279.y, _7280.z ? _7269.z : _7279.z);
        }
        else
        {
            _24086 = _7243;
        }
        vec2 _7299 = clamp((esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875)), vec2(0.0), vec2(1.0));
        vec2 _7347 = vec2(cos(_6932.w), sin(_6932.w));
        float _7350 = dot(_23228, _7347);
        float _7368 = _3970 * (abs(_23228.x) + abs(_23228.y));
        float _24572;
        _24572 = 0.0;
        float _7390;
        SPIRV_CROSS_UNROLL
        for (int _24571 = -2; _24571 <= 2; _24572 = _7390, _24571++)
        {
            float _7385 = (-_23041) + ((float(_24571) * 0.4000000059604644775390625) * _7368);
            float _26509;
            do
            {
                if (_7385 < 0.0)
                {
                    _26509 = 0.0;
                    break;
                }
                float _19045 = clamp(1.0 - (_7385 / _7057), 0.0, 1.0);
                float _19056 = min(_19045 / max(sqrt(clamp(1.0 - (_19045 * _19045), 0.0, 1.0)), 0.001000000047497451305389404296875), 8.0);
                _26509 = pow(1.0 - inversesqrt(1.0 + (_19056 * _19056)), 5.0);
                break;
            } while(false);
            _7390 = _24572 + ((3.0 - abs(float(_24571))) * _26509);
        }
        float _7421 = clamp(0.5 + (0.5 * dot((esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5)) / _7049, _7347)), 0.0, 1.0);
        _26149 = (((_24572 * 0.111111111938953399658203125) * (pow(clamp(_7350, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(clamp(-_7350, 0.0, 1.0), 1.5)))) * _6932.z) * 1.60000002384185791015625;
        _25502 = _6973;
        _25489 = vec4((mix(mix(_24086, _6891.xyz, vec3(clamp(_6891.w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _7072))) + (0.3499999940395355224609375 * ((dot(_6891.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _7299.y) : _7299.y))), 0.0, 1.0))), (_24326 * 1.10000002384185791015625) + vec3(0.07999999821186065673828125), vec3(pow(1.0 - _24090.z, 5.0) * 0.3499999940395355224609375)) + vec3((((0.039999999105930328369140625 * _7421) * _7421) + (0.0500000007450580596923828125 * (1.0 - _7072))) * _6932.z)) * _3995, _3995) + (_24579 * (1.0 - _3995));
    }
    else
    {
        _26149 = 0.0;
        _25502 = _23021;
        _25489 = _24579;
    }
    vec4 _25499;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 1u) != 0u)
    {
        vec4 _24854;
        vec4 _25165;
        SPIRV_CROSS_BRANCH
        if (esia_v7.y != 0u)
        {
            uint _19081 = max(uint(_178.gConv.z), 1u);
            uint _19122 = max(uint(_178.gConv.z), 1u);
            _25165 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19081) * 24u) + 3u), int(esia_v1 / _19081), 0).xy, 0);
            _24854 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19122) * 24u) + 4u), int(esia_v1 / _19122), 0).xy, 0);
        }
        else
        {
            _25165 = vec4(0.0);
            _24854 = vec4(0.0);
        }
        vec4 _25484;
        do
        {
            if (esia_v7.y == 0u)
            {
                _25484 = esia_v4;
                break;
            }
            vec2 _19194 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            vec2 _19204 = fwidth(esia_v0);
            float _19206 = max(length(_19204), 9.9999997473787516355514526367188e-05);
            float _25476;
            float _25480;
            if ((esia_v7.y == 1u) || (esia_v7.y == 4u))
            {
                float _19215 = cos(_24854.x);
                float _19218 = sin(_24854.x);
                float _19244 = ((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_19215, _19218)) / max(0.5 * ((abs(_19215) * _19194.x) + (abs(_19218) * _19194.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (esia_v7.y == 4u)
                {
                    float _19259 = clamp((_19244 - _24854.y) / max(_24854.z - _24854.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _25484 = vec4(clamp(abs((fract(vec3(_19259 * 0.800000011920928955078125) + vec3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - vec3(3.0)) - vec3(1.0), vec3(0.0), vec3(1.0)), esia_v4.w * pow(max(sin(_19259 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _25480 = -1.0;
                _25476 = _19244;
            }
            else
            {
                float _25477;
                float _25481;
                if (esia_v7.y == 2u)
                {
                    _25481 = -1.0;
                    _25477 = length(esia_v0 - (esia_v2.xy + (_24854.xy * _19194))) / max(_24854.z * max(_19194.x, _19194.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    vec2 _19327 = esia_v0 - (esia_v2.xy + (_24854.xy * _19194));
                    float _19338 = fract(((atan(_19327.y, _19327.x) - _24854.z) * 0.15915493667125701904296875) + 1.0);
                    float _25478;
                    float _25482;
                    if (_24854.w > 0.5)
                    {
                        _25482 = -1.0;
                        _25478 = 0.5 - (0.5 * cos(_19338 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _19358 = (((_19338 < 0.5) ? _19338 : (_19338 - 1.0)) * 6.283185482025146484375) * length(_19327);
                        float _25483;
                        if (abs(_19358) < _19206)
                        {
                            _25483 = clamp(((_19358 / _19206) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _25483 = -1.0;
                        }
                        _25482 = _25483;
                        _25478 = _19338;
                    }
                    _25481 = _25482;
                    _25477 = _25478;
                }
                _25480 = _25481;
                _25476 = _25477;
            }
            vec4 _19387 = vec4(esia_v4.xyz * esia_v4.w, esia_v4.w);
            vec4 _19399 = vec4(_25165.xyz * _25165.w, _25165.w);
            vec4 _19406 = mix(_19399, _19387, vec4(_25480));
            vec4 _19411 = mix(_19387, _19399, vec4(clamp(_25476, 0.0, 1.0)));
            bvec4 _19412 = bvec4(_25480 >= 0.0);
            vec4 _19413 = vec4(_19412.x ? _19406.x : _19411.x, _19412.y ? _19406.y : _19411.y, _19412.z ? _19406.z : _19411.z, _19412.w ? _19406.w : _19411.w);
            vec4 _19428 = vec4(_19413.xyz / vec3(_19413.w), _19413.w);
            bvec4 _19429 = bvec4(_19413.w > 9.9999997473787516355514526367188e-06);
            _25484 = vec4(_19429.x ? _19428.x : vec4(0.0).x, _19429.y ? _19428.y : vec4(0.0).y, _19429.z ? _19428.z : vec4(0.0).z, _19429.w ? _19428.w : vec4(0.0).w);
            break;
        } while(false);
        vec4 _25485;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & 64u) != 0u)
        {
            uint _19439 = max(uint(_178.gConv.z), 1u);
            vec4 _19473 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19439) * 24u) + 18u), int(esia_v1 / _19439), 0).xy, 0);
            _25485 = _25484 * texture(SPIRV_Cross_CombinedgTexgLinear, mix(_19473.xy, _19473.zw, (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875))));
        }
        else
        {
            _25485 = _25484;
        }
        float _19481 = clamp(_25485.w * _3995, 0.0, 1.0);
        _25499 = vec4(_25485.xyz * _19481, _19481) + (_25489 * (1.0 - _19481));
    }
    else
    {
        _25499 = _25489;
    }
    vec4 _26135;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 16384u) != 0u)
    {
        uint _19506 = max(uint(_178.gConv.z), 1u);
        vec4 _19540 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19506) * 24u) + 21u), int(esia_v1 / _19506), 0).xy, 0);
        uint _19547 = max(uint(_178.gConv.z), 1u);
        vec4 _19581 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19547) * 24u) + 3u), int(esia_v1 / _19547), 0).xy, 0);
        vec2 _4435 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        vec2 _4442 = (esia_v0 - esia_v2.xy) / _4435;
        float _4453 = ((_19581.x >= 0.0) ? _19581.x : _178.gTime.x) * _19540.z;
        float _4456 = _4442.x * 2.0;
        float _4457 = _4456 - 1.0;
        float _4462 = _4442.y * _4435.y;
        float _4465 = max(_19540.w, 0.001000000047497451305389404296875);
        float _4470 = clamp(1.0 - (_4457 * _4457), 0.0, 1.0);
        float _4472 = pow(_4470, 1.2999999523162841796875);
        float _4474 = pow(_4470, 0.699999988079071044921875);
        float _4477 = clamp(_4453 * 1.4285714626312255859375, 0.0, 1.0);
        float _4493 = (0.25 + (0.75 * ((_4477 * _4477) * (3.0 - (2.0 * _4477))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4453 * 2.099999904632568359375)));
        float _4502 = ((_19540.y * _4435.y) * _4493) * _4472;
        float _4528 = (_4435.y * (0.5 + ((_19540.x * (0.5 - (_4457 * _4457))) * 0.5))) + (((0.14000000059604644775390625 * _4435.y) * _4472) * sin(((_4457 * 2.400000095367431640625) - (_4453 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _25495;
        float _25496;
        vec3 _25497;
        _25497 = vec3(0.0);
        _25496 = _4528;
        _25495 = _4528;
        vec3 _4600;
        float _26540;
        float _26541;
        SPIRV_CROSS_UNROLL
        for (int _25494 = 0; _25494 < 4; _25497 = _4600, _25496 = _26541, _25495 = _26540, _25494++)
        {
            float _4552 = _4528 + ((_4502 * _2996[_25494].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4453 * 1.7000000476837158203125) + _3013[_25494].y))));
            _26540 = (_25494 == 0) ? _4552 : _25495;
            _26541 = (_25494 == 2) ? _4552 : _25496;
            float _4567 = _4465 * _2996[_25494].y;
            float _4572 = (_4462 - _4552) / _4567;
            float _4577 = _4465 * _2996[_25494].z;
            vec3 _26504;
            _26504 = vec3(0.0);
            SPIRV_CROSS_UNROLL
            for (int _26503 = 0; _26503 < 6; )
            {
                float _19604 = ((_4462 - _4552) - (_4577 * ((float(_26503) * 0.4000000059604644775390625) - 1.0))) / _4567;
                _26504 += (_1700[_26503] * exp((-_19604) * _19604));
                _26503++;
                continue;
            }
            _4600 = _25497 + (mix(_26504 * vec3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), vec3(exp((-_4572) * _4572)), vec3(_3013[_25494].x)) * (_2996[_25494].w * _4474));
        }
        float _4606 = _4465 * 1.5;
        float _4636 = clamp((_4462 - _25495) / max(_25496 - _25495, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4664 = (_4462 - (_25496 - (_4465 * 3.0))) / (((_4435.y * 0.0900000035762786865234375) + (_4502 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4667 = (_4456 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4692 = ((_4462 - _25495) + (_4465 * 5.0)) / (_4465 * 7.0);
        vec3 _4718 = vec3(1.0) - exp((-((((_25497 + (mix(vec3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), vec3(1.0), vec3(_4636)) * ((((1.0 / (1.0 + exp((-((_4462 - _25495) - (_4465 * 2.0))) / _4606))) / (1.0 + exp((-(_25496 - _4462)) / _4606))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4636, 2.5)))) * _4474))) + (vec3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4664) * _4664) - (_4667 * _4667)) * (0.5 + (1.10000002384185791015625 * _4493))))) + (vec3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4692) * _4692) * _4472) * 0.100000001490116119384765625))) * mix(vec3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), vec3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), vec3(0.5 + (0.5 * sin(_4453 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4728 = (esia_v4.w * smoothstep(0.0, 0.119999997317790985107421875, _4442.y)) * smoothstep(1.0, 0.87999999523162841796875, _4442.y);
        float _4747 = (clamp(max(_4718.x, max(_4718.y, _4718.z)), 0.0, 1.0) * _4728) * _3995;
        _26135 = vec4((_4718 * _4728) * _3995, _4747) + (_25499 * (1.0 - _4747));
    }
    else
    {
        _26135 = _25499;
    }
    vec4 _26144;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 4u) != 0u) && ((esia_v7.x & 256u) != 0u))
    {
        uint _19637 = max(uint(_178.gConv.z), 1u);
        vec4 _19671 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19637) * 24u) + 7u), int(esia_v1 / _19637), 0).xy, 0);
        uint _19678 = max(uint(_178.gConv.z), 1u);
        vec4 _19712 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19678) * 24u) + 8u), int(esia_v1 / _19678), 0).xy, 0);
        vec2 _4781 = esia_v0 - _19712.zw;
        vec2 _19751 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _19760 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _26110;
        SPIRV_CROSS_BRANCH
        if (esia_v7.z == 1u)
        {
            vec2 _19766 = _4781 - _19751;
            float _26109;
            do
            {
                if (esia_v5.w >= 6.282185077667236328125)
                {
                    _26109 = abs(length(_19766) - esia_v3.x) - esia_v3.y;
                    break;
                }
                float _19865 = esia_v5.z + (esia_v5.w * 0.5);
                float _19867 = cos(_19865);
                float _19869 = sin(_19865);
                float _19878 = dot(_19766, vec2(-_19869, _19867));
                float _19881 = dot(_19766, vec2(_19867, _19869));
                vec2 _19882 = vec2(_19878, _19881);
                float _19885 = abs(_19878);
                _19882.x = _19885;
                float _19888 = esia_v5.w * 0.5;
                float _19890 = sin(_19888);
                float _19892 = cos(_19888);
                _26109 = (((_19892 * _19885) > (_19890 * _19881)) ? length(_19882 - (vec2(_19890, _19892) * esia_v3.x)) : abs(length(_19882) - esia_v3.x)) - esia_v3.y;
                break;
            } while(false);
            _26110 = _26109;
        }
        else
        {
            float _26111;
            if (esia_v7.z == 2u)
            {
                vec2 _19928 = _4781 - _23023.xy;
                vec2 _19931 = _23023.zw - _23023.xy;
                _26111 = length(_19928 - (_19931 * clamp(dot(_19928, _19931) / max(dot(_19931, _19931), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
            }
            else
            {
                vec2 _19793 = _4781 - _19751;
                float _19984 = min(_19760.x, _19760.y);
                float _19987 = min((_19793.x > 0.0) ? ((_19793.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_19793.y > 0.0) ? esia_v3.w : esia_v3.x), _19984);
                float _19993 = _19987 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _26094;
                float _26095;
                if (_19993 > _19984)
                {
                    float _20007 = esia_v5.y * clamp((_19984 - _19987) / max(0.60000002384185791015625 * _19987, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _26095 = _20007;
                    _26094 = _19987 * (1.0 + (0.60000002384185791015625 * _20007));
                }
                else
                {
                    _26095 = esia_v5.y;
                    _26094 = _19993;
                }
                vec2 _20020 = (abs(_19793) - _19760) + vec2(_26094);
                vec2 _20022 = max(_20020, vec2(0.0));
                float _26096;
                SPIRV_CROSS_BRANCH
                if ((_20022.x > 0.0) && (_20022.y > 0.0))
                {
                    float _26097;
                    if ((_26095 > 0.001000000047497451305389404296875) && (_26094 > 9.9999997473787516355514526367188e-05))
                    {
                        float _20039 = 2.0 + (2.0 * _26095);
                        vec2 _20044 = _20022 / vec2(max(_26094, 9.9999997473787516355514526367188e-05));
                        _26097 = pow(pow(_20044.x, _20039) + pow(_20044.y, _20039), 1.0 / _20039) * _26094;
                    }
                    else
                    {
                        _26097 = length(_20022);
                    }
                    _26096 = _26097;
                }
                else
                {
                    _26096 = max(_20022.x, _20022.y);
                }
                float _20079 = (min(max(_20020.x, _20020.y), 0.0) + _26096) - _26094;
                float _26112;
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & 512u) != 0u)
                {
                    vec2 _19821 = max((_23023.zw - _23023.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _19824 = _4781 - ((_23023.xy + _23023.zw) * 0.5);
                    float _20115 = min(_19821.x, _19821.y);
                    float _20118 = min((_19824.x > 0.0) ? ((_19824.y > 0.0) ? _25502.x : _25502.x) : ((_19824.y > 0.0) ? _25502.x : _25502.x), _20115);
                    float _20124 = _20118 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _26100;
                    float _26101;
                    if (_20124 > _20115)
                    {
                        float _20138 = esia_v5.y * clamp((_20115 - _20118) / max(0.60000002384185791015625 * _20118, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _26101 = _20138;
                        _26100 = _20118 * (1.0 + (0.60000002384185791015625 * _20138));
                    }
                    else
                    {
                        _26101 = esia_v5.y;
                        _26100 = _20124;
                    }
                    vec2 _20151 = (abs(_19824) - _19821) + vec2(_26100);
                    vec2 _20153 = max(_20151, vec2(0.0));
                    float _26102;
                    SPIRV_CROSS_BRANCH
                    if ((_20153.x > 0.0) && (_20153.y > 0.0))
                    {
                        float _26103;
                        if ((_26101 > 0.001000000047497451305389404296875) && (_26100 > 9.9999997473787516355514526367188e-05))
                        {
                            float _20170 = 2.0 + (2.0 * _26101);
                            vec2 _20175 = _20153 / vec2(max(_26100, 9.9999997473787516355514526367188e-05));
                            _26103 = pow(pow(_20175.x, _20170) + pow(_20175.y, _20170), 1.0 / _20170) * _26100;
                        }
                        else
                        {
                            _26103 = length(_20153);
                        }
                        _26102 = _26103;
                    }
                    else
                    {
                        _26102 = max(_20153.x, _20153.y);
                    }
                    float _20210 = (min(max(_20151.x, _20151.y), 0.0) + _26102) - _26100;
                    float _20215 = max(_25502.y, 9.9999997473787516355514526367188e-05);
                    float _20224 = max(_20215 - abs(_20079 - _20210), 0.0) / _20215;
                    _26112 = min(_20079, _20210) - (((_20224 * _20224) * _20215) * 0.25);
                }
                else
                {
                    _26112 = _20079;
                }
                _26111 = _26112;
            }
            _26110 = _26111;
        }
        float _4790 = (_26110 + _19712.y) / (max(_19712.x * 0.5, _3970 * 0.5) * 1.41421353816986083984375);
        float _20241 = sign(_4790);
        float _20243 = abs(_4790);
        float _20254 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_20243 * _20243))) * _20243)) * _20243);
        float _20257 = _20254 * _20254;
        float _20272 = clamp(_19671.w * ((0.5 + (0.5 * (_20241 - (_20241 / (_20257 * _20257))))) * _3995), 0.0, 1.0);
        _26144 = vec4(_19671.xyz * _20272, _20272) + (_26135 * (1.0 - _20272));
    }
    else
    {
        _26144 = _26135;
    }
    vec4 _26168;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 16u) != 0u)
    {
        uint _20297 = max(uint(_178.gConv.z), 1u);
        vec4 _20331 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _20297) * 24u) + 9u), int(esia_v1 / _20297), 0).xy, 0);
        uint _20338 = max(uint(_178.gConv.z), 1u);
        vec4 _20372 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _20338) * 24u) + 10u), int(esia_v1 / _20338), 0).xy, 0);
        float _4822 = max(-_23041, 0.0) / max(_20372.z, 0.001000000047497451305389404296875);
        float _20380 = clamp(_20331.w * clamp((exp(((-_4822) * _4822) * 2.2000000476837158203125) * _20372.w) * _3995, 0.0, 1.0), 0.0, 1.0);
        _26168 = vec4(_20331.xyz * _20380, _20380) + (_26144 * (1.0 - _20380));
    }
    else
    {
        _26168 = _26144;
    }
    vec3 _4853 = _26168.xyz + vec3(_26149 * clamp(_26168.w / max(_3995, 0.001000000047497451305389404296875), 0.0, 1.0));
    vec4 _22916 = _26168;
    _22916.x = _4853.x;
    _22916.y = _4853.y;
    _22916.z = _4853.z;
    vec4 _26171;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 2u) != 0u)
    {
        uint _20405 = max(uint(_178.gConv.z), 1u);
        vec4 _20439 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _20405) * 24u) + 5u), int(esia_v1 / _20405), 0).xy, 0);
        uint _20446 = max(uint(_178.gConv.z), 1u);
        vec4 _20480 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _20446) * 24u) + 6u), int(esia_v1 / _20446), 0).xy, 0);
        float _4873 = _20480.x;
        float _4875 = _20480.y;
        vec4 _26169;
        if (_20480.z < 0.999000012874603271484375)
        {
            vec2 _4911 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            float _4922 = cos(_20480.w);
            float _4925 = sin(_20480.w);
            vec4 _22933 = _20439;
            _22933.w = _20439.w * mix(1.0, _20480.z, clamp(((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_4922, _4925)) / max(0.5 * ((abs(_4922) * _4911.x) + (abs(_4925) * _4911.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _26169 = _22933;
        }
        else
        {
            _26169 = _20439;
        }
        float _20488 = clamp(_26169.w * (clamp(0.5 - ((_23041 - (_4873 * _4875)) / _3970), 0.0, 1.0) - clamp(0.5 - ((_23041 + (_4873 * (1.0 - _4875))) / _3970), 0.0, 1.0)), 0.0, 1.0);
        _26171 = vec4(_26169.xyz * _20488, _20488) + (_22916 * (1.0 - _20488));
    }
    else
    {
        _26171 = _22916;
    }
    vec4 _26172;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 128u) != 0u)
    {
        vec2 _4986 = (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        float _5008 = exp(-pow((((_4986.x * 0.85000002384185791015625) + (_4986.y * 0.1500000059604644775390625)) - ((fract(_178.gTime.x * esia_v6.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _26172 = vec4(_26171.xyz + vec3(((_5008 * esia_v6.z) * _3995) * max(_26171.w, 0.3499999940395355224609375)), max(_26171.w, ((_5008 * esia_v6.z) * _3995) * 0.5));
    }
    else
    {
        _26172 = _26171;
    }
    vec4 _26498;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 2048u) != 0u)
    {
        vec3 _20513 = fract(floor(_23020).xyx * 0.103100001811981201171875);
        vec3 _20522 = _20513 + vec3(dot(_20513, _20513.yzx + vec3(33.3300018310546875)));
        vec3 _5059 = _26172.xyz + vec3(((fract((_20522.x + _20522.y) * _20522.z) - 0.5) * esia_v6.y) * _26172.w);
        vec4 _22957 = _26172;
        _22957.x = _5059.x;
        _22957.y = _5059.y;
        _22957.z = _5059.z;
        _26498 = _22957;
    }
    else
    {
        _26498 = _26172;
    }
    float _26494;
    if (_221.gFade.z > 0.0)
    {
        _26494 = smoothstep(0.0, 1.0, clamp((_23020.y - _221.gFade.x) / _221.gFade.z, 0.0, 1.0));
    }
    else
    {
        _26494 = 1.0;
    }
    float _26495;
    if (_221.gFade.w > 0.0)
    {
        _26495 = _26494 * smoothstep(0.0, 1.0, clamp((_221.gFade.y - _23020.y) / _221.gFade.w, 0.0, 1.0));
    }
    else
    {
        _26495 = _26494;
    }
    vec4 _5076 = _26498 * ((esia_v6.x * _26183) * _26495);
    vec4 _26499;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 8u) != 0u) || ((esia_v7.x & 4u) != 0u))
    {
        vec3 _20574 = fract((floor(_23020) + vec2(17.0)).xyx * 0.103100001811981201171875);
        vec3 _20583 = _20574 + vec3(dot(_20574, _20574.yzx + vec3(33.3300018310546875)));
        vec3 _5100 = _5076.xyz + vec3(((fract((_20583.x + _20583.y) * _20583.z) - 0.5) * 0.0039215688593685626983642578125) * clamp(_5076.w * 8.0, 0.0, 1.0));
        vec4 _22969 = _5076;
        _22969.x = _5100.x;
        _22969.y = _5100.y;
        _22969.z = _5100.z;
        _26499 = _22969;
    }
    else
    {
        _26499 = _5076;
    }
    vec3 _5110 = max(_26499.xyz, vec3(0.0));
    vec4 _22975 = _26499;
    _22975.x = _5110.x;
    _22975.y = _5110.y;
    _22975.z = _5110.z;
    vec4 _26500;
    if ((_178.gTime.w > 0.5) && (_26499.w > 9.9999997473787516355514526367188e-06))
    {
        vec3 _20627 = clamp(_22975.xyz / vec3(_26499.w), vec3(0.0), vec3(1.0));
        vec3 _20633 = pow((_20627 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _20636 = _20627 * vec3(0.077399380505084991455078125);
        bvec3 _20638 = lessThanEqual(_20627, vec3(0.040449999272823333740234375));
        vec3 _20613 = vec3(_20638.x ? _20636.x : _20633.x, _20638.y ? _20636.y : _20633.y, _20638.z ? _20636.z : _20633.z) * _26499.w;
        vec4 _22984 = _22975;
        _22984.x = _20613.x;
        _22984.y = _20613.y;
        _22984.z = _20613.z;
        _26500 = _22984;
    }
    else
    {
        _26500 = _22975;
    }
    _entryPointOutput = _26500;
}

