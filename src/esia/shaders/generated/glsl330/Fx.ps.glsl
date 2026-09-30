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

const vec3 _1756[6] = vec3[](vec3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), vec3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), vec3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), vec3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), vec3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), vec3(0.660000026226043701171875, 0.5, 1.0));
const vec4 _3038[4] = vec4[](vec4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), vec4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), vec4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), vec4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875));
const vec2 _3055[4] = vec2[](vec2(0.0), vec2(0.100000001490116119384765625, 1.2999999523162841796875), vec2(0.550000011920928955078125, 3.900000095367431640625), vec2(0.20000000298023223876953125, 5.19999980926513671875));

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _184;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _227;

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
    vec2 _5208 = gl_FragCoord.xy + _184.gConv.yy;
    vec2 _16367;
    if (_184.gConv.x > 0.5)
    {
        vec2 _15315 = _5208;
        _15315.y = _184.gTarget.y - _5208.y;
        _16367 = _15315;
    }
    else
    {
        _16367 = _5208;
    }
    float _4000 = dFdx(esia_v0.x);
    float _4004 = dFdy(esia_v0.x);
    float _4007 = max(abs(_4000) + abs(_4004), 9.9999997473787516355514526367188e-05);
    vec4 _16368;
    vec4 _16370;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.z == 2u) || ((esia_v7.x & 512u) != 0u))
    {
        uint _5227 = max(uint(_184.gConv.z), 1u);
        uint _5268 = max(uint(_184.gConv.z), 1u);
        _16370 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5227) * 24u) + 15u), int(esia_v1 / _5227), 0).xy, 0);
        _16368 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5268) * 24u) + 16u), int(esia_v1 / _5268), 0).xy, 0);
    }
    else
    {
        _16370 = vec4(0.0);
        _16368 = vec4(0.0);
    }
    vec2 _5341 = (esia_v2.xy + esia_v2.zw) * 0.5;
    vec2 _5350 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
    float _16388;
    SPIRV_CROSS_BRANCH
    if (esia_v7.z == 1u)
    {
        vec2 _5356 = esia_v0 - _5341;
        float _16387;
        do
        {
            if (esia_v5.w >= 6.282185077667236328125)
            {
                _16387 = abs(length(_5356) - esia_v3.x) - esia_v3.y;
                break;
            }
            float _5455 = esia_v5.z + (esia_v5.w * 0.5);
            float _5457 = cos(_5455);
            float _5459 = sin(_5455);
            float _5468 = dot(_5356, vec2(-_5459, _5457));
            float _5471 = dot(_5356, vec2(_5457, _5459));
            vec2 _5472 = vec2(_5468, _5471);
            float _5475 = abs(_5468);
            _5472.x = _5475;
            float _5478 = esia_v5.w * 0.5;
            float _5480 = sin(_5478);
            float _5482 = cos(_5478);
            _16387 = (((_5482 * _5475) > (_5480 * _5471)) ? length(_5472 - (vec2(_5480, _5482) * esia_v3.x)) : abs(length(_5472) - esia_v3.x)) - esia_v3.y;
            break;
        } while(false);
        _16388 = _16387;
    }
    else
    {
        float _16389;
        if (esia_v7.z == 2u)
        {
            vec2 _5518 = esia_v0 - _16370.xy;
            vec2 _5521 = _16370.zw - _16370.xy;
            _16389 = length(_5518 - (_5521 * clamp(dot(_5518, _5521) / max(dot(_5521, _5521), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
        }
        else
        {
            vec2 _5383 = esia_v0 - _5341;
            float _5574 = min(_5350.x, _5350.y);
            float _5577 = min((_5383.x > 0.0) ? ((_5383.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_5383.y > 0.0) ? esia_v3.w : esia_v3.x), _5574);
            float _5583 = _5577 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
            float _16372;
            float _16373;
            if (_5583 > _5574)
            {
                float _5597 = esia_v5.y * clamp((_5574 - _5577) / max(0.60000002384185791015625 * _5577, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _16373 = _5597;
                _16372 = _5577 * (1.0 + (0.60000002384185791015625 * _5597));
            }
            else
            {
                _16373 = esia_v5.y;
                _16372 = _5583;
            }
            vec2 _5610 = (abs(_5383) - _5350) + vec2(_16372);
            vec2 _5612 = max(_5610, vec2(0.0));
            float _16374;
            SPIRV_CROSS_BRANCH
            if ((_5612.x > 0.0) && (_5612.y > 0.0))
            {
                float _16375;
                if ((_16373 > 0.001000000047497451305389404296875) && (_16372 > 9.9999997473787516355514526367188e-05))
                {
                    float _5629 = 2.0 + (2.0 * _16373);
                    vec2 _5634 = _5612 / vec2(max(_16372, 9.9999997473787516355514526367188e-05));
                    _16375 = pow(pow(_5634.x, _5629) + pow(_5634.y, _5629), 1.0 / _5629) * _16372;
                }
                else
                {
                    _16375 = length(_5612);
                }
                _16374 = _16375;
            }
            else
            {
                _16374 = max(_5612.x, _5612.y);
            }
            float _5669 = (min(max(_5610.x, _5610.y), 0.0) + _16374) - _16372;
            float _16390;
            SPIRV_CROSS_BRANCH
            if ((esia_v7.x & 512u) != 0u)
            {
                vec2 _5411 = max((_16370.zw - _16370.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                vec2 _5414 = esia_v0 - ((_16370.xy + _16370.zw) * 0.5);
                float _5705 = min(_5411.x, _5411.y);
                float _5708 = min((_5414.x > 0.0) ? ((_5414.y > 0.0) ? _16368.x : _16368.x) : ((_5414.y > 0.0) ? _16368.x : _16368.x), _5705);
                float _5714 = _5708 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _16378;
                float _16379;
                if (_5714 > _5705)
                {
                    float _5728 = esia_v5.y * clamp((_5705 - _5708) / max(0.60000002384185791015625 * _5708, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _16379 = _5728;
                    _16378 = _5708 * (1.0 + (0.60000002384185791015625 * _5728));
                }
                else
                {
                    _16379 = esia_v5.y;
                    _16378 = _5714;
                }
                vec2 _5741 = (abs(_5414) - _5411) + vec2(_16378);
                vec2 _5743 = max(_5741, vec2(0.0));
                float _16380;
                SPIRV_CROSS_BRANCH
                if ((_5743.x > 0.0) && (_5743.y > 0.0))
                {
                    float _16381;
                    if ((_16379 > 0.001000000047497451305389404296875) && (_16378 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5760 = 2.0 + (2.0 * _16379);
                        vec2 _5765 = _5743 / vec2(max(_16378, 9.9999997473787516355514526367188e-05));
                        _16381 = pow(pow(_5765.x, _5760) + pow(_5765.y, _5760), 1.0 / _5760) * _16378;
                    }
                    else
                    {
                        _16381 = length(_5743);
                    }
                    _16380 = _16381;
                }
                else
                {
                    _16380 = max(_5743.x, _5743.y);
                }
                float _5800 = (min(max(_5741.x, _5741.y), 0.0) + _16380) - _16378;
                float _5805 = max(_16368.y, 9.9999997473787516355514526367188e-05);
                float _5814 = max(_5805 - abs(_5669 - _5800), 0.0) / _5805;
                _16390 = min(_5669, _5800) - (((_5814 * _5814) * _5805) * 0.25);
            }
            else
            {
                _16390 = _5669;
            }
            _16389 = _16390;
        }
        _16388 = _16389;
    }
    float _4032 = clamp(0.5 - (_16388 / _4007), 0.0, 1.0);
    float _18314;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 1024u) != 0u)
    {
        uint _5831 = max(uint(_184.gConv.z), 1u);
        vec4 _5865 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5831) * 24u) + 19u), int(esia_v1 / _5831), 0).xy, 0);
        uint _5872 = max(uint(_184.gConv.z), 1u);
        vec4 _5906 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5872) * 24u) + 20u), int(esia_v1 / _5872), 0).xy, 0);
        vec2 _4061 = max((_5865.zw - _5865.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        vec2 _4064 = esia_v0 - ((_5865.xy + _5865.zw) * 0.5);
        float _4070 = _5906.y;
        float _5942 = min(_4061.x, _4061.y);
        float _5945 = min((_4064.x > 0.0) ? ((_4064.y > 0.0) ? _5906.x : _5906.x) : ((_4064.y > 0.0) ? _5906.x : _5906.x), _5942);
        float _5951 = _5945 * (1.0 + (0.60000002384185791015625 * _4070));
        float _16391;
        float _16392;
        if (_5951 > _5942)
        {
            float _5965 = _4070 * clamp((_5942 - _5945) / max(0.60000002384185791015625 * _5945, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _16392 = _5965;
            _16391 = _5945 * (1.0 + (0.60000002384185791015625 * _5965));
        }
        else
        {
            _16392 = _4070;
            _16391 = _5951;
        }
        vec2 _5978 = (abs(_4064) - _4061) + vec2(_16391);
        vec2 _5980 = max(_5978, vec2(0.0));
        float _16393;
        SPIRV_CROSS_BRANCH
        if ((_5980.x > 0.0) && (_5980.y > 0.0))
        {
            float _16394;
            if ((_16392 > 0.001000000047497451305389404296875) && (_16391 > 9.9999997473787516355514526367188e-05))
            {
                float _5997 = 2.0 + (2.0 * _16392);
                vec2 _6002 = _5980 / vec2(max(_16391, 9.9999997473787516355514526367188e-05));
                _16394 = pow(pow(_6002.x, _5997) + pow(_6002.y, _5997), 1.0 / _5997) * _16391;
            }
            else
            {
                _16394 = length(_5980);
            }
            _16393 = _16394;
        }
        else
        {
            _16393 = max(_5980.x, _5980.y);
        }
        float _4076 = clamp(0.5 - (((min(max(_5978.x, _5978.y), 0.0) + _16393) - _16391) / _4007), 0.0, 1.0);
        if (_4076 <= 0.0)
        {
            discard;
        }
        _18314 = _4076;
    }
    else
    {
        _18314 = 1.0;
    }
    bool _4087 = ((esia_v7.x & 32u) != 0u) && (_4032 >= 0.999000012874603271484375);
    vec4 _16483;
    SPIRV_CROSS_BRANCH
    if ((((esia_v7.x & 4u) != 0u) && (!((esia_v7.x & 256u) != 0u))) && (!_4087))
    {
        uint _6044 = max(uint(_184.gConv.z), 1u);
        vec4 _6078 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6044) * 24u) + 7u), int(esia_v1 / _6044), 0).xy, 0);
        uint _6085 = max(uint(_184.gConv.z), 1u);
        vec4 _6119 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6085) * 24u) + 8u), int(esia_v1 / _6085), 0).xy, 0);
        vec2 _4118 = esia_v0 - _6119.zw;
        vec2 _6158 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _6167 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _16441;
        SPIRV_CROSS_BRANCH
        if (esia_v7.z == 1u)
        {
            vec2 _6173 = _4118 - _6158;
            float _16440;
            do
            {
                if (esia_v5.w >= 6.282185077667236328125)
                {
                    _16440 = abs(length(_6173) - esia_v3.x) - esia_v3.y;
                    break;
                }
                float _6272 = esia_v5.z + (esia_v5.w * 0.5);
                float _6274 = cos(_6272);
                float _6276 = sin(_6272);
                float _6285 = dot(_6173, vec2(-_6276, _6274));
                float _6288 = dot(_6173, vec2(_6274, _6276));
                vec2 _6289 = vec2(_6285, _6288);
                float _6292 = abs(_6285);
                _6289.x = _6292;
                float _6295 = esia_v5.w * 0.5;
                float _6297 = sin(_6295);
                float _6299 = cos(_6295);
                _16440 = (((_6299 * _6292) > (_6297 * _6288)) ? length(_6289 - (vec2(_6297, _6299) * esia_v3.x)) : abs(length(_6289) - esia_v3.x)) - esia_v3.y;
                break;
            } while(false);
            _16441 = _16440;
        }
        else
        {
            float _16442;
            if (esia_v7.z == 2u)
            {
                vec2 _6335 = _4118 - _16370.xy;
                vec2 _6338 = _16370.zw - _16370.xy;
                _16442 = length(_6335 - (_6338 * clamp(dot(_6335, _6338) / max(dot(_6338, _6338), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
            }
            else
            {
                vec2 _6200 = _4118 - _6158;
                float _6391 = min(_6167.x, _6167.y);
                float _6394 = min((_6200.x > 0.0) ? ((_6200.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_6200.y > 0.0) ? esia_v3.w : esia_v3.x), _6391);
                float _6400 = _6394 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _16425;
                float _16426;
                if (_6400 > _6391)
                {
                    float _6414 = esia_v5.y * clamp((_6391 - _6394) / max(0.60000002384185791015625 * _6394, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _16426 = _6414;
                    _16425 = _6394 * (1.0 + (0.60000002384185791015625 * _6414));
                }
                else
                {
                    _16426 = esia_v5.y;
                    _16425 = _6400;
                }
                vec2 _6427 = (abs(_6200) - _6167) + vec2(_16425);
                vec2 _6429 = max(_6427, vec2(0.0));
                float _16427;
                SPIRV_CROSS_BRANCH
                if ((_6429.x > 0.0) && (_6429.y > 0.0))
                {
                    float _16428;
                    if ((_16426 > 0.001000000047497451305389404296875) && (_16425 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6446 = 2.0 + (2.0 * _16426);
                        vec2 _6451 = _6429 / vec2(max(_16425, 9.9999997473787516355514526367188e-05));
                        _16428 = pow(pow(_6451.x, _6446) + pow(_6451.y, _6446), 1.0 / _6446) * _16425;
                    }
                    else
                    {
                        _16428 = length(_6429);
                    }
                    _16427 = _16428;
                }
                else
                {
                    _16427 = max(_6429.x, _6429.y);
                }
                float _6486 = (min(max(_6427.x, _6427.y), 0.0) + _16427) - _16425;
                float _16443;
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & 512u) != 0u)
                {
                    vec2 _6228 = max((_16370.zw - _16370.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _6231 = _4118 - ((_16370.xy + _16370.zw) * 0.5);
                    float _6522 = min(_6228.x, _6228.y);
                    float _6525 = min((_6231.x > 0.0) ? ((_6231.y > 0.0) ? _16368.x : _16368.x) : ((_6231.y > 0.0) ? _16368.x : _16368.x), _6522);
                    float _6531 = _6525 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16431;
                    float _16432;
                    if (_6531 > _6522)
                    {
                        float _6545 = esia_v5.y * clamp((_6522 - _6525) / max(0.60000002384185791015625 * _6525, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16432 = _6545;
                        _16431 = _6525 * (1.0 + (0.60000002384185791015625 * _6545));
                    }
                    else
                    {
                        _16432 = esia_v5.y;
                        _16431 = _6531;
                    }
                    vec2 _6558 = (abs(_6231) - _6228) + vec2(_16431);
                    vec2 _6560 = max(_6558, vec2(0.0));
                    float _16433;
                    SPIRV_CROSS_BRANCH
                    if ((_6560.x > 0.0) && (_6560.y > 0.0))
                    {
                        float _16434;
                        if ((_16432 > 0.001000000047497451305389404296875) && (_16431 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6577 = 2.0 + (2.0 * _16432);
                            vec2 _6582 = _6560 / vec2(max(_16431, 9.9999997473787516355514526367188e-05));
                            _16434 = pow(pow(_6582.x, _6577) + pow(_6582.y, _6577), 1.0 / _6577) * _16431;
                        }
                        else
                        {
                            _16434 = length(_6560);
                        }
                        _16433 = _16434;
                    }
                    else
                    {
                        _16433 = max(_6560.x, _6560.y);
                    }
                    float _6617 = (min(max(_6558.x, _6558.y), 0.0) + _16433) - _16431;
                    float _6622 = max(_16368.y, 9.9999997473787516355514526367188e-05);
                    float _6631 = max(_6622 - abs(_6486 - _6617), 0.0) / _6622;
                    _16443 = min(_6486, _6617) - (((_6631 * _6631) * _6622) * 0.25);
                }
                else
                {
                    _16443 = _6486;
                }
                _16442 = _16443;
            }
            _16441 = _16442;
        }
        float _4127 = (_16441 - _6119.y) / (max(_6119.x * 0.5, _4007 * 0.5) * 1.41421353816986083984375);
        float _6648 = sign(_4127);
        float _6650 = abs(_4127);
        float _6661 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6650 * _6650))) * _6650)) * _6650);
        float _6664 = _6661 * _6661;
        float _6679 = clamp(_6078.w * (0.5 - (0.5 * (_6648 - (_6648 / (_6664 * _6664))))), 0.0, 1.0);
        _16483 = vec4(_6078.xyz * _6679, _6679);
    }
    else
    {
        _16483 = vec4(0.0);
    }
    vec4 _17375;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 8u) != 0u) && (!_4087))
    {
        uint _6704 = max(uint(_184.gConv.z), 1u);
        vec4 _6738 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6704) * 24u) + 9u), int(esia_v1 / _6704), 0).xy, 0);
        uint _6745 = max(uint(_184.gConv.z), 1u);
        vec4 _6779 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6745) * 24u) + 10u), int(esia_v1 / _6745), 0).xy, 0);
        float _4155 = max(_6779.x, 0.001000000047497451305389404296875);
        float _16476;
        float _16479;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & 8192u) != 0u)
        {
            uint _6786 = max(uint(_184.gConv.z), 1u);
            vec4 _6820 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6786) * 24u) + 22u), int(esia_v1 / _6786), 0).xy, 0);
            vec2 _4171 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _4180 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            vec2 _4187 = esia_v2.xy - _6820.xy;
            vec2 _4194 = _6820.zw - esia_v2.zw;
            vec2 _4224 = clamp(vec2((esia_v0.x < _4171.x) ? _4187.x : _4194.x, (esia_v0.y < _4171.y) ? _4187.y : _4194.y) * vec2(0.58823525905609130859375), vec2(min(_4155, 1.5)), vec2(_4155));
            float _4229 = min(_4180.x, _4180.y);
            float _16474;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 0u)
            {
                _16474 = min(((esia_v0.x > _4171.x) ? ((esia_v0.y > _4171.y) ? esia_v3.z : esia_v3.y) : ((esia_v0.y > _4171.y) ? esia_v3.w : esia_v3.x)) * (1.0 + (0.60000002384185791015625 * esia_v5.y)), _4229);
            }
            else
            {
                _16474 = _4229;
            }
            vec2 _4278 = max(abs(esia_v0 - _4171) - (_4180 - vec2(_16474)), vec2(0.0));
            float _4280 = length(_4278);
            vec2 _4286 = _4278 / vec2(_4280);
            bvec2 _4287 = bvec2(_4280 > 9.9999997473787516355514526367188e-05);
            vec2 _4288 = vec2(_4287.x ? _4286.x : vec2(0.707099974155426025390625).x, _4287.y ? _4286.y : vec2(0.707099974155426025390625).y);
            vec2 _4303 = esia_v0 - _6820.xy;
            vec2 _4308 = _6820.zw - esia_v0;
            _16479 = clamp(min(min(_4303.x, _4303.y), min(_4308.x, _4308.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _16476 = inversesqrt(dot(_4288 * _4288, vec2(1.0) / (_4224 * _4224)));
        }
        else
        {
            _16479 = 1.0;
            _16476 = _4155;
        }
        float _4326 = max(_16388, 0.0) / _16476;
        float _6828 = clamp(_6738.w * clamp((exp(((-_4326) * _4326) * 2.2000000476837158203125) * _6779.y) * _16479, 0.0, 1.0), 0.0, 1.0);
        _17375 = vec4(_6738.xyz * _6828, _6828) + (_16483 * (1.0 - _6828));
    }
    else
    {
        _17375 = _16483;
    }
    vec4 _17886;
    vec4 _17899;
    float _18280;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 32u) != 0u) && (_4032 > 0.0))
    {
        uint _6853 = max(uint(_184.gConv.z), 1u);
        vec4 _6887 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6853) * 24u) + 11u), int(esia_v1 / _6853), 0).xy, 0);
        uint _6894 = max(uint(_184.gConv.z), 1u);
        vec4 _6928 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6894) * 24u) + 12u), int(esia_v1 / _6894), 0).xy, 0);
        uint _6935 = max(uint(_184.gConv.z), 1u);
        vec4 _6969 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6935) * 24u) + 13u), int(esia_v1 / _6935), 0).xy, 0);
        uint _6976 = max(uint(_184.gConv.z), 1u);
        vec4 _7010 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6976) * 24u) + 16u), int(esia_v1 / _6976), 0).xy, 0);
        float _7073 = _6887.x * _184.gDisplay.z;
        float _7075 = _6887.y;
        vec2 _7084 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(1.0));
        float _7092 = clamp(_6887.z, 0.001000000047497451305389404296875, min(_7084.x, _7084.y));
        float _7094 = _6887.w;
        float _7101 = clamp(1.0 - (max(-_16388, 0.0) / _7092), 0.0, 1.0);
        float _7107 = sqrt(clamp(1.0 - (_7101 * _7101), 0.0, 1.0));
        vec2 _16575;
        vec3 _17152;
        SPIRV_CROSS_BRANCH
        if (_7101 > 0.0)
        {
            vec2 _7481 = esia_v0 + vec2(0.5, 0.0);
            vec2 _7549 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _7558 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _16515;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _7564 = _7481 - _7549;
                float _16514;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _16514 = abs(length(_7564) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _7663 = esia_v5.z + (esia_v5.w * 0.5);
                    float _7665 = cos(_7663);
                    float _7667 = sin(_7663);
                    float _7676 = dot(_7564, vec2(-_7667, _7665));
                    float _7679 = dot(_7564, vec2(_7665, _7667));
                    vec2 _7680 = vec2(_7676, _7679);
                    float _7683 = abs(_7676);
                    _7680.x = _7683;
                    float _7686 = esia_v5.w * 0.5;
                    float _7688 = sin(_7686);
                    float _7690 = cos(_7686);
                    _16514 = (((_7690 * _7683) > (_7688 * _7679)) ? length(_7680 - (vec2(_7688, _7690) * esia_v3.x)) : abs(length(_7680) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _16515 = _16514;
            }
            else
            {
                float _16516;
                if (esia_v7.z == 2u)
                {
                    vec2 _7726 = _7481 - _16370.xy;
                    vec2 _7729 = _16370.zw - _16370.xy;
                    _16516 = length(_7726 - (_7729 * clamp(dot(_7726, _7729) / max(dot(_7729, _7729), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _7591 = _7481 - _7549;
                    float _7782 = min(_7558.x, _7558.y);
                    float _7785 = min((_7591.x > 0.0) ? ((_7591.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_7591.y > 0.0) ? esia_v3.w : esia_v3.x), _7782);
                    float _7791 = _7785 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16499;
                    float _16500;
                    if (_7791 > _7782)
                    {
                        float _7805 = esia_v5.y * clamp((_7782 - _7785) / max(0.60000002384185791015625 * _7785, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16500 = _7805;
                        _16499 = _7785 * (1.0 + (0.60000002384185791015625 * _7805));
                    }
                    else
                    {
                        _16500 = esia_v5.y;
                        _16499 = _7791;
                    }
                    vec2 _7818 = (abs(_7591) - _7558) + vec2(_16499);
                    vec2 _7820 = max(_7818, vec2(0.0));
                    float _16501;
                    SPIRV_CROSS_BRANCH
                    if ((_7820.x > 0.0) && (_7820.y > 0.0))
                    {
                        float _16502;
                        if ((_16500 > 0.001000000047497451305389404296875) && (_16499 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7837 = 2.0 + (2.0 * _16500);
                            vec2 _7842 = _7820 / vec2(max(_16499, 9.9999997473787516355514526367188e-05));
                            _16502 = pow(pow(_7842.x, _7837) + pow(_7842.y, _7837), 1.0 / _7837) * _16499;
                        }
                        else
                        {
                            _16502 = length(_7820);
                        }
                        _16501 = _16502;
                    }
                    else
                    {
                        _16501 = max(_7820.x, _7820.y);
                    }
                    float _7877 = (min(max(_7818.x, _7818.y), 0.0) + _16501) - _16499;
                    float _16517;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _7619 = max((_16370.zw - _16370.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _7622 = _7481 - ((_16370.xy + _16370.zw) * 0.5);
                        float _7913 = min(_7619.x, _7619.y);
                        float _7916 = min((_7622.x > 0.0) ? ((_7622.y > 0.0) ? _7010.x : _7010.x) : ((_7622.y > 0.0) ? _7010.x : _7010.x), _7913);
                        float _7922 = _7916 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _16505;
                        float _16506;
                        if (_7922 > _7913)
                        {
                            float _7936 = esia_v5.y * clamp((_7913 - _7916) / max(0.60000002384185791015625 * _7916, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16506 = _7936;
                            _16505 = _7916 * (1.0 + (0.60000002384185791015625 * _7936));
                        }
                        else
                        {
                            _16506 = esia_v5.y;
                            _16505 = _7922;
                        }
                        vec2 _7949 = (abs(_7622) - _7619) + vec2(_16505);
                        vec2 _7951 = max(_7949, vec2(0.0));
                        float _16507;
                        SPIRV_CROSS_BRANCH
                        if ((_7951.x > 0.0) && (_7951.y > 0.0))
                        {
                            float _16508;
                            if ((_16506 > 0.001000000047497451305389404296875) && (_16505 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7968 = 2.0 + (2.0 * _16506);
                                vec2 _7973 = _7951 / vec2(max(_16505, 9.9999997473787516355514526367188e-05));
                                _16508 = pow(pow(_7973.x, _7968) + pow(_7973.y, _7968), 1.0 / _7968) * _16505;
                            }
                            else
                            {
                                _16508 = length(_7951);
                            }
                            _16507 = _16508;
                        }
                        else
                        {
                            _16507 = max(_7951.x, _7951.y);
                        }
                        float _8008 = (min(max(_7949.x, _7949.y), 0.0) + _16507) - _16505;
                        float _8013 = max(_7010.y, 9.9999997473787516355514526367188e-05);
                        float _8022 = max(_8013 - abs(_7877 - _8008), 0.0) / _8013;
                        _16517 = min(_7877, _8008) - (((_8022 * _8022) * _8013) * 0.25);
                    }
                    else
                    {
                        _16517 = _7877;
                    }
                    _16516 = _16517;
                }
                _16515 = _16516;
            }
            vec2 _7485 = esia_v0 - vec2(0.5, 0.0);
            vec2 _8071 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _8080 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _16534;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _8086 = _7485 - _8071;
                float _16533;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _16533 = abs(length(_8086) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _8185 = esia_v5.z + (esia_v5.w * 0.5);
                    float _8187 = cos(_8185);
                    float _8189 = sin(_8185);
                    float _8198 = dot(_8086, vec2(-_8189, _8187));
                    float _8201 = dot(_8086, vec2(_8187, _8189));
                    vec2 _8202 = vec2(_8198, _8201);
                    float _8205 = abs(_8198);
                    _8202.x = _8205;
                    float _8208 = esia_v5.w * 0.5;
                    float _8210 = sin(_8208);
                    float _8212 = cos(_8208);
                    _16533 = (((_8212 * _8205) > (_8210 * _8201)) ? length(_8202 - (vec2(_8210, _8212) * esia_v3.x)) : abs(length(_8202) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _16534 = _16533;
            }
            else
            {
                float _16535;
                if (esia_v7.z == 2u)
                {
                    vec2 _8248 = _7485 - _16370.xy;
                    vec2 _8251 = _16370.zw - _16370.xy;
                    _16535 = length(_8248 - (_8251 * clamp(dot(_8248, _8251) / max(dot(_8251, _8251), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _8113 = _7485 - _8071;
                    float _8304 = min(_8080.x, _8080.y);
                    float _8307 = min((_8113.x > 0.0) ? ((_8113.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_8113.y > 0.0) ? esia_v3.w : esia_v3.x), _8304);
                    float _8313 = _8307 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16518;
                    float _16519;
                    if (_8313 > _8304)
                    {
                        float _8327 = esia_v5.y * clamp((_8304 - _8307) / max(0.60000002384185791015625 * _8307, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16519 = _8327;
                        _16518 = _8307 * (1.0 + (0.60000002384185791015625 * _8327));
                    }
                    else
                    {
                        _16519 = esia_v5.y;
                        _16518 = _8313;
                    }
                    vec2 _8340 = (abs(_8113) - _8080) + vec2(_16518);
                    vec2 _8342 = max(_8340, vec2(0.0));
                    float _16520;
                    SPIRV_CROSS_BRANCH
                    if ((_8342.x > 0.0) && (_8342.y > 0.0))
                    {
                        float _16521;
                        if ((_16519 > 0.001000000047497451305389404296875) && (_16518 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8359 = 2.0 + (2.0 * _16519);
                            vec2 _8364 = _8342 / vec2(max(_16518, 9.9999997473787516355514526367188e-05));
                            _16521 = pow(pow(_8364.x, _8359) + pow(_8364.y, _8359), 1.0 / _8359) * _16518;
                        }
                        else
                        {
                            _16521 = length(_8342);
                        }
                        _16520 = _16521;
                    }
                    else
                    {
                        _16520 = max(_8342.x, _8342.y);
                    }
                    float _8399 = (min(max(_8340.x, _8340.y), 0.0) + _16520) - _16518;
                    float _16536;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _8141 = max((_16370.zw - _16370.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _8144 = _7485 - ((_16370.xy + _16370.zw) * 0.5);
                        float _8435 = min(_8141.x, _8141.y);
                        float _8438 = min((_8144.x > 0.0) ? ((_8144.y > 0.0) ? _7010.x : _7010.x) : ((_8144.y > 0.0) ? _7010.x : _7010.x), _8435);
                        float _8444 = _8438 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _16524;
                        float _16525;
                        if (_8444 > _8435)
                        {
                            float _8458 = esia_v5.y * clamp((_8435 - _8438) / max(0.60000002384185791015625 * _8438, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16525 = _8458;
                            _16524 = _8438 * (1.0 + (0.60000002384185791015625 * _8458));
                        }
                        else
                        {
                            _16525 = esia_v5.y;
                            _16524 = _8444;
                        }
                        vec2 _8471 = (abs(_8144) - _8141) + vec2(_16524);
                        vec2 _8473 = max(_8471, vec2(0.0));
                        float _16526;
                        SPIRV_CROSS_BRANCH
                        if ((_8473.x > 0.0) && (_8473.y > 0.0))
                        {
                            float _16527;
                            if ((_16525 > 0.001000000047497451305389404296875) && (_16524 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8490 = 2.0 + (2.0 * _16525);
                                vec2 _8495 = _8473 / vec2(max(_16524, 9.9999997473787516355514526367188e-05));
                                _16527 = pow(pow(_8495.x, _8490) + pow(_8495.y, _8490), 1.0 / _8490) * _16524;
                            }
                            else
                            {
                                _16527 = length(_8473);
                            }
                            _16526 = _16527;
                        }
                        else
                        {
                            _16526 = max(_8473.x, _8473.y);
                        }
                        float _8530 = (min(max(_8471.x, _8471.y), 0.0) + _16526) - _16524;
                        float _8535 = max(_7010.y, 9.9999997473787516355514526367188e-05);
                        float _8544 = max(_8535 - abs(_8399 - _8530), 0.0) / _8535;
                        _16536 = min(_8399, _8530) - (((_8544 * _8544) * _8535) * 0.25);
                    }
                    else
                    {
                        _16536 = _8399;
                    }
                    _16535 = _16536;
                }
                _16534 = _16535;
            }
            vec2 _7490 = esia_v0 + vec2(0.0, 0.5);
            vec2 _8593 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _8602 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _16553;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _8608 = _7490 - _8593;
                float _16552;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _16552 = abs(length(_8608) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _8707 = esia_v5.z + (esia_v5.w * 0.5);
                    float _8709 = cos(_8707);
                    float _8711 = sin(_8707);
                    float _8720 = dot(_8608, vec2(-_8711, _8709));
                    float _8723 = dot(_8608, vec2(_8709, _8711));
                    vec2 _8724 = vec2(_8720, _8723);
                    float _8727 = abs(_8720);
                    _8724.x = _8727;
                    float _8730 = esia_v5.w * 0.5;
                    float _8732 = sin(_8730);
                    float _8734 = cos(_8730);
                    _16552 = (((_8734 * _8727) > (_8732 * _8723)) ? length(_8724 - (vec2(_8732, _8734) * esia_v3.x)) : abs(length(_8724) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _16553 = _16552;
            }
            else
            {
                float _16554;
                if (esia_v7.z == 2u)
                {
                    vec2 _8770 = _7490 - _16370.xy;
                    vec2 _8773 = _16370.zw - _16370.xy;
                    _16554 = length(_8770 - (_8773 * clamp(dot(_8770, _8773) / max(dot(_8773, _8773), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _8635 = _7490 - _8593;
                    float _8826 = min(_8602.x, _8602.y);
                    float _8829 = min((_8635.x > 0.0) ? ((_8635.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_8635.y > 0.0) ? esia_v3.w : esia_v3.x), _8826);
                    float _8835 = _8829 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16537;
                    float _16538;
                    if (_8835 > _8826)
                    {
                        float _8849 = esia_v5.y * clamp((_8826 - _8829) / max(0.60000002384185791015625 * _8829, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16538 = _8849;
                        _16537 = _8829 * (1.0 + (0.60000002384185791015625 * _8849));
                    }
                    else
                    {
                        _16538 = esia_v5.y;
                        _16537 = _8835;
                    }
                    vec2 _8862 = (abs(_8635) - _8602) + vec2(_16537);
                    vec2 _8864 = max(_8862, vec2(0.0));
                    float _16539;
                    SPIRV_CROSS_BRANCH
                    if ((_8864.x > 0.0) && (_8864.y > 0.0))
                    {
                        float _16540;
                        if ((_16538 > 0.001000000047497451305389404296875) && (_16537 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8881 = 2.0 + (2.0 * _16538);
                            vec2 _8886 = _8864 / vec2(max(_16537, 9.9999997473787516355514526367188e-05));
                            _16540 = pow(pow(_8886.x, _8881) + pow(_8886.y, _8881), 1.0 / _8881) * _16537;
                        }
                        else
                        {
                            _16540 = length(_8864);
                        }
                        _16539 = _16540;
                    }
                    else
                    {
                        _16539 = max(_8864.x, _8864.y);
                    }
                    float _8921 = (min(max(_8862.x, _8862.y), 0.0) + _16539) - _16537;
                    float _16555;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _8663 = max((_16370.zw - _16370.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _8666 = _7490 - ((_16370.xy + _16370.zw) * 0.5);
                        float _8957 = min(_8663.x, _8663.y);
                        float _8960 = min((_8666.x > 0.0) ? ((_8666.y > 0.0) ? _7010.x : _7010.x) : ((_8666.y > 0.0) ? _7010.x : _7010.x), _8957);
                        float _8966 = _8960 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _16543;
                        float _16544;
                        if (_8966 > _8957)
                        {
                            float _8980 = esia_v5.y * clamp((_8957 - _8960) / max(0.60000002384185791015625 * _8960, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16544 = _8980;
                            _16543 = _8960 * (1.0 + (0.60000002384185791015625 * _8980));
                        }
                        else
                        {
                            _16544 = esia_v5.y;
                            _16543 = _8966;
                        }
                        vec2 _8993 = (abs(_8666) - _8663) + vec2(_16543);
                        vec2 _8995 = max(_8993, vec2(0.0));
                        float _16545;
                        SPIRV_CROSS_BRANCH
                        if ((_8995.x > 0.0) && (_8995.y > 0.0))
                        {
                            float _16546;
                            if ((_16544 > 0.001000000047497451305389404296875) && (_16543 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9012 = 2.0 + (2.0 * _16544);
                                vec2 _9017 = _8995 / vec2(max(_16543, 9.9999997473787516355514526367188e-05));
                                _16546 = pow(pow(_9017.x, _9012) + pow(_9017.y, _9012), 1.0 / _9012) * _16543;
                            }
                            else
                            {
                                _16546 = length(_8995);
                            }
                            _16545 = _16546;
                        }
                        else
                        {
                            _16545 = max(_8995.x, _8995.y);
                        }
                        float _9052 = (min(max(_8993.x, _8993.y), 0.0) + _16545) - _16543;
                        float _9057 = max(_7010.y, 9.9999997473787516355514526367188e-05);
                        float _9066 = max(_9057 - abs(_8921 - _9052), 0.0) / _9057;
                        _16555 = min(_8921, _9052) - (((_9066 * _9066) * _9057) * 0.25);
                    }
                    else
                    {
                        _16555 = _8921;
                    }
                    _16554 = _16555;
                }
                _16553 = _16554;
            }
            vec2 _7494 = esia_v0 - vec2(0.0, 0.5);
            vec2 _9115 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _9124 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _16572;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _9130 = _7494 - _9115;
                float _16571;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _16571 = abs(length(_9130) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _9229 = esia_v5.z + (esia_v5.w * 0.5);
                    float _9231 = cos(_9229);
                    float _9233 = sin(_9229);
                    float _9242 = dot(_9130, vec2(-_9233, _9231));
                    float _9245 = dot(_9130, vec2(_9231, _9233));
                    vec2 _9246 = vec2(_9242, _9245);
                    float _9249 = abs(_9242);
                    _9246.x = _9249;
                    float _9252 = esia_v5.w * 0.5;
                    float _9254 = sin(_9252);
                    float _9256 = cos(_9252);
                    _16571 = (((_9256 * _9249) > (_9254 * _9245)) ? length(_9246 - (vec2(_9254, _9256) * esia_v3.x)) : abs(length(_9246) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _16572 = _16571;
            }
            else
            {
                float _16573;
                if (esia_v7.z == 2u)
                {
                    vec2 _9292 = _7494 - _16370.xy;
                    vec2 _9295 = _16370.zw - _16370.xy;
                    _16573 = length(_9292 - (_9295 * clamp(dot(_9292, _9295) / max(dot(_9295, _9295), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _9157 = _7494 - _9115;
                    float _9348 = min(_9124.x, _9124.y);
                    float _9351 = min((_9157.x > 0.0) ? ((_9157.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_9157.y > 0.0) ? esia_v3.w : esia_v3.x), _9348);
                    float _9357 = _9351 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16556;
                    float _16557;
                    if (_9357 > _9348)
                    {
                        float _9371 = esia_v5.y * clamp((_9348 - _9351) / max(0.60000002384185791015625 * _9351, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16557 = _9371;
                        _16556 = _9351 * (1.0 + (0.60000002384185791015625 * _9371));
                    }
                    else
                    {
                        _16557 = esia_v5.y;
                        _16556 = _9357;
                    }
                    vec2 _9384 = (abs(_9157) - _9124) + vec2(_16556);
                    vec2 _9386 = max(_9384, vec2(0.0));
                    float _16558;
                    SPIRV_CROSS_BRANCH
                    if ((_9386.x > 0.0) && (_9386.y > 0.0))
                    {
                        float _16559;
                        if ((_16557 > 0.001000000047497451305389404296875) && (_16556 > 9.9999997473787516355514526367188e-05))
                        {
                            float _9403 = 2.0 + (2.0 * _16557);
                            vec2 _9408 = _9386 / vec2(max(_16556, 9.9999997473787516355514526367188e-05));
                            _16559 = pow(pow(_9408.x, _9403) + pow(_9408.y, _9403), 1.0 / _9403) * _16556;
                        }
                        else
                        {
                            _16559 = length(_9386);
                        }
                        _16558 = _16559;
                    }
                    else
                    {
                        _16558 = max(_9386.x, _9386.y);
                    }
                    float _9443 = (min(max(_9384.x, _9384.y), 0.0) + _16558) - _16556;
                    float _16574;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _9185 = max((_16370.zw - _16370.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _9188 = _7494 - ((_16370.xy + _16370.zw) * 0.5);
                        float _9479 = min(_9185.x, _9185.y);
                        float _9482 = min((_9188.x > 0.0) ? ((_9188.y > 0.0) ? _7010.x : _7010.x) : ((_9188.y > 0.0) ? _7010.x : _7010.x), _9479);
                        float _9488 = _9482 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _16562;
                        float _16563;
                        if (_9488 > _9479)
                        {
                            float _9502 = esia_v5.y * clamp((_9479 - _9482) / max(0.60000002384185791015625 * _9482, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16563 = _9502;
                            _16562 = _9482 * (1.0 + (0.60000002384185791015625 * _9502));
                        }
                        else
                        {
                            _16563 = esia_v5.y;
                            _16562 = _9488;
                        }
                        vec2 _9515 = (abs(_9188) - _9185) + vec2(_16562);
                        vec2 _9517 = max(_9515, vec2(0.0));
                        float _16564;
                        SPIRV_CROSS_BRANCH
                        if ((_9517.x > 0.0) && (_9517.y > 0.0))
                        {
                            float _16565;
                            if ((_16563 > 0.001000000047497451305389404296875) && (_16562 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9534 = 2.0 + (2.0 * _16563);
                                vec2 _9539 = _9517 / vec2(max(_16562, 9.9999997473787516355514526367188e-05));
                                _16565 = pow(pow(_9539.x, _9534) + pow(_9539.y, _9534), 1.0 / _9534) * _16562;
                            }
                            else
                            {
                                _16565 = length(_9517);
                            }
                            _16564 = _16565;
                        }
                        else
                        {
                            _16564 = max(_9517.x, _9517.y);
                        }
                        float _9574 = (min(max(_9515.x, _9515.y), 0.0) + _16564) - _16562;
                        float _9579 = max(_7010.y, 9.9999997473787516355514526367188e-05);
                        float _9588 = max(_9579 - abs(_9443 - _9574), 0.0) / _9579;
                        _16574 = min(_9443, _9574) - (((_9588 * _9588) * _9579) * 0.25);
                    }
                    else
                    {
                        _16574 = _9443;
                    }
                    _16573 = _16574;
                }
                _16572 = _16573;
            }
            vec2 _7500 = vec2(_16515 - _16534, _16553 - _16572);
            float _7502 = length(_7500);
            vec2 _7508 = _7500 / vec2(_7502);
            bvec2 _7509 = bvec2(_7502 > 9.9999997473787516355514526367188e-06);
            vec2 _7510 = vec2(_7509.x ? _7508.x : vec2(0.0, -1.0).x, _7509.y ? _7508.y : vec2(0.0, -1.0).y);
            _17152 = normalize(vec3(_7510 * min(_7101 / max(_7107, 0.001000000047497451305389404296875), 8.0), 1.0));
            _16575 = _7510;
        }
        else
        {
            _17152 = vec3(0.0, 0.0, 1.0);
            _16575 = vec2(0.0, -1.0);
        }
        vec2 _7133 = ((-_16575) * _7075) * (1.0 - _7107);
        float _7135 = _7010.z;
        vec2 _16576;
        SPIRV_CROSS_BRANCH
        if (_7135 > 0.0)
        {
            _16576 = (((esia_v2.xy + esia_v2.zw) * 0.5) - esia_v0) * (_7135 / (1.0 + _7135));
        }
        else
        {
            _16576 = vec2(0.0);
        }
        vec2 _7159 = _16367 * _184.gTarget.zw;
        vec2 _7166 = _184.gDisplay.zw * _184.gTarget.zw;
        vec2 _7171 = (_7133 + _16576) * _7166;
        vec2 _7174 = _7133 * _7166;
        vec3 _17034;
        float _17052;
        vec3 _17255;
        if (_184.gTime.z > 0.5)
        {
            vec2 _7181 = _7159 + _7171;
            float _9613 = clamp(log2(max(_7073, 1.0)) - 1.0, 0.0, 5.0);
            int _9616 = int(floor(_9613));
            float _9620 = _9613 - float(_9616);
            bool _9625 = (_9620 > 0.0199999995529651641845703125) && (_9616 < 5);
            vec3 _16647;
            _16647 = vec3(0.0);
            vec3 _9654;
            SPIRV_CROSS_LOOP
            for (int _16577 = 0; _16577 < 2; _16647 = _9654, _16577++)
            {
                if ((_16577 > 0) && (!_9625))
                {
                    break;
                }
                int _9640 = _9616 + _16577;
                int _9678 = clamp(_9640, 1, 5);
                vec2 _9747 = (_7181 * _184.gLevel[_9678].xy) - vec2(0.5);
                vec2 _9749 = floor(_9747);
                vec2 _9752 = _9747 - _9749;
                vec2 _9755 = _9752 * _9752;
                vec2 _9758 = _9755 * _9752;
                vec2 _9777 = (((_9758 * 3.0) - (_9755 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                vec2 _9790 = _9758 * 0.16666667163372039794921875;
                vec2 _9793 = (((((-_9758) + (_9755 * 3.0)) - (_9752 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9777;
                vec2 _9797 = (((((_9758 * (-3.0)) + (_9755 * 3.0)) + (_9752 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9790;
                vec2 _9809 = ((_9749 - vec2(0.5)) + (_9777 / _9793)) * _184.gLevel[_9678].zw;
                vec2 _9821 = ((_9749 + vec2(1.5)) + (_9790 / _9797)) * _184.gLevel[_9678].zw;
                vec4 _16614;
                SPIRV_CROSS_BRANCH
                if (_9640 <= 0)
                {
                    vec2 _16613;
                    if (_184.gConv.x > 0.5)
                    {
                        vec2 _15698 = _7181;
                        _15698.y = 1.0 - _7181.y;
                        _16613 = _15698;
                    }
                    else
                    {
                        _16613 = _7181;
                    }
                    _16614 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _16613, 0.0);
                }
                else
                {
                    vec4 _16615;
                    if (_9640 == 1)
                    {
                        vec2 _16606;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _15703 = _9809;
                            _15703.y = 1.0 - _9809.y;
                            _16606 = _15703;
                        }
                        else
                        {
                            _16606 = _9809;
                        }
                        float _9866 = _9809.y;
                        vec2 _9867 = vec2(_9821.x, _9866);
                        vec2 _16607;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _15710 = _9867;
                            _15710.y = 1.0 - _9866;
                            _16607 = _15710;
                        }
                        else
                        {
                            _16607 = _9867;
                        }
                        float _9884 = _9821.y;
                        vec2 _9885 = vec2(_9809.x, _9884);
                        vec2 _16609;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _15717 = _9885;
                            _15717.y = 1.0 - _9884;
                            _16609 = _15717;
                        }
                        else
                        {
                            _16609 = _9885;
                        }
                        vec2 _16611;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _15724 = _9821;
                            _15724.y = 1.0 - _9821.y;
                            _16611 = _15724;
                        }
                        else
                        {
                            _16611 = _9821;
                        }
                        _16615 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16606, 0.0) * (_9793.x * _9793.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16607, 0.0) * (_9797.x * _9793.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16609, 0.0) * (_9793.x * _9797.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16611, 0.0) * (_9797.x * _9797.y));
                    }
                    else
                    {
                        vec4 _16616;
                        if (_9640 == 2)
                        {
                            vec2 _16599;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15731 = _9809;
                                _15731.y = 1.0 - _9809.y;
                                _16599 = _15731;
                            }
                            else
                            {
                                _16599 = _9809;
                            }
                            float _9996 = _9809.y;
                            vec2 _9997 = vec2(_9821.x, _9996);
                            vec2 _16600;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15738 = _9997;
                                _15738.y = 1.0 - _9996;
                                _16600 = _15738;
                            }
                            else
                            {
                                _16600 = _9997;
                            }
                            float _10014 = _9821.y;
                            vec2 _10015 = vec2(_9809.x, _10014);
                            vec2 _16602;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15745 = _10015;
                                _15745.y = 1.0 - _10014;
                                _16602 = _15745;
                            }
                            else
                            {
                                _16602 = _10015;
                            }
                            vec2 _16604;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15752 = _9821;
                                _15752.y = 1.0 - _9821.y;
                                _16604 = _15752;
                            }
                            else
                            {
                                _16604 = _9821;
                            }
                            _16616 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16599, 0.0) * (_9793.x * _9793.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16600, 0.0) * (_9797.x * _9793.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16602, 0.0) * (_9793.x * _9797.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16604, 0.0) * (_9797.x * _9797.y));
                        }
                        else
                        {
                            vec4 _16617;
                            if (_9640 == 3)
                            {
                                vec2 _16592;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _15759 = _9809;
                                    _15759.y = 1.0 - _9809.y;
                                    _16592 = _15759;
                                }
                                else
                                {
                                    _16592 = _9809;
                                }
                                float _10126 = _9809.y;
                                vec2 _10127 = vec2(_9821.x, _10126);
                                vec2 _16593;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _15766 = _10127;
                                    _15766.y = 1.0 - _10126;
                                    _16593 = _15766;
                                }
                                else
                                {
                                    _16593 = _10127;
                                }
                                float _10144 = _9821.y;
                                vec2 _10145 = vec2(_9809.x, _10144);
                                vec2 _16595;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _15773 = _10145;
                                    _15773.y = 1.0 - _10144;
                                    _16595 = _15773;
                                }
                                else
                                {
                                    _16595 = _10145;
                                }
                                vec2 _16597;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _15780 = _9821;
                                    _15780.y = 1.0 - _9821.y;
                                    _16597 = _15780;
                                }
                                else
                                {
                                    _16597 = _9821;
                                }
                                _16617 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16592, 0.0) * (_9793.x * _9793.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16593, 0.0) * (_9797.x * _9793.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16595, 0.0) * (_9793.x * _9797.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16597, 0.0) * (_9797.x * _9797.y));
                            }
                            else
                            {
                                vec4 _16618;
                                if (_9640 == 4)
                                {
                                    vec2 _16585;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15787 = _9809;
                                        _15787.y = 1.0 - _9809.y;
                                        _16585 = _15787;
                                    }
                                    else
                                    {
                                        _16585 = _9809;
                                    }
                                    float _10256 = _9809.y;
                                    vec2 _10257 = vec2(_9821.x, _10256);
                                    vec2 _16586;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15794 = _10257;
                                        _15794.y = 1.0 - _10256;
                                        _16586 = _15794;
                                    }
                                    else
                                    {
                                        _16586 = _10257;
                                    }
                                    float _10274 = _9821.y;
                                    vec2 _10275 = vec2(_9809.x, _10274);
                                    vec2 _16588;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15801 = _10275;
                                        _15801.y = 1.0 - _10274;
                                        _16588 = _15801;
                                    }
                                    else
                                    {
                                        _16588 = _10275;
                                    }
                                    vec2 _16590;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15808 = _9821;
                                        _15808.y = 1.0 - _9821.y;
                                        _16590 = _15808;
                                    }
                                    else
                                    {
                                        _16590 = _9821;
                                    }
                                    _16618 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16585, 0.0) * (_9793.x * _9793.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16586, 0.0) * (_9797.x * _9793.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16588, 0.0) * (_9793.x * _9797.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16590, 0.0) * (_9797.x * _9797.y));
                                }
                                else
                                {
                                    vec2 _16578;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15815 = _9809;
                                        _15815.y = 1.0 - _9809.y;
                                        _16578 = _15815;
                                    }
                                    else
                                    {
                                        _16578 = _9809;
                                    }
                                    float _10386 = _9809.y;
                                    vec2 _10387 = vec2(_9821.x, _10386);
                                    vec2 _16579;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15822 = _10387;
                                        _15822.y = 1.0 - _10386;
                                        _16579 = _15822;
                                    }
                                    else
                                    {
                                        _16579 = _10387;
                                    }
                                    float _10404 = _9821.y;
                                    vec2 _10405 = vec2(_9809.x, _10404);
                                    vec2 _16581;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15829 = _10405;
                                        _15829.y = 1.0 - _10404;
                                        _16581 = _15829;
                                    }
                                    else
                                    {
                                        _16581 = _10405;
                                    }
                                    vec2 _16583;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15836 = _9821;
                                        _15836.y = 1.0 - _9821.y;
                                        _16583 = _15836;
                                    }
                                    else
                                    {
                                        _16583 = _9821;
                                    }
                                    _16618 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16578, 0.0) * (_9793.x * _9793.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16579, 0.0) * (_9797.x * _9793.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16581, 0.0) * (_9793.x * _9797.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16583, 0.0) * (_9797.x * _9797.y));
                                }
                                _16617 = _16618;
                            }
                            _16616 = _16617;
                        }
                        _16615 = _16616;
                    }
                    _16614 = _16615;
                }
                _9654 = _16647 + (_16614.xyz * (_9625 ? ((_16577 == 0) ? (1.0 - _9620) : _9620) : 1.0));
            }
            vec3 _17037;
            SPIRV_CROSS_BRANCH
            if (((_7094 > 0.001000000047497451305389404296875) && (_7101 > 0.0)) && ((((_7075 * 0.300000011920928955078125) * _7094) * _184.gDisplay.z) > (_7073 * 0.3499999940395355224609375)))
            {
                float _7201 = 0.300000011920928955078125 * _7094;
                vec2 _7208 = (_7159 + _7171) - (_7174 * _7201);
                float _10501 = clamp(log2(max(_7073, 1.0)) - 1.0, 0.0, 5.0);
                int _10504 = int(floor(_10501));
                float _10508 = _10501 - float(_10504);
                bool _10513 = (_10508 > 0.0199999995529651641845703125) && (_10504 < 5);
                vec3 _16743;
                _16743 = vec3(0.0);
                vec3 _10542;
                SPIRV_CROSS_LOOP
                for (int _16673 = 0; _16673 < 2; _16743 = _10542, _16673++)
                {
                    if ((_16673 > 0) && (!_10513))
                    {
                        break;
                    }
                    int _10528 = _10504 + _16673;
                    int _10566 = clamp(_10528, 1, 5);
                    vec2 _10635 = (_7208 * _184.gLevel[_10566].xy) - vec2(0.5);
                    vec2 _10637 = floor(_10635);
                    vec2 _10640 = _10635 - _10637;
                    vec2 _10643 = _10640 * _10640;
                    vec2 _10646 = _10643 * _10640;
                    vec2 _10665 = (((_10646 * 3.0) - (_10643 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                    vec2 _10678 = _10646 * 0.16666667163372039794921875;
                    vec2 _10681 = (((((-_10646) + (_10643 * 3.0)) - (_10640 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10665;
                    vec2 _10685 = (((((_10646 * (-3.0)) + (_10643 * 3.0)) + (_10640 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10678;
                    vec2 _10697 = ((_10637 - vec2(0.5)) + (_10665 / _10681)) * _184.gLevel[_10566].zw;
                    vec2 _10709 = ((_10637 + vec2(1.5)) + (_10678 / _10685)) * _184.gLevel[_10566].zw;
                    vec4 _16710;
                    SPIRV_CROSS_BRANCH
                    if (_10528 <= 0)
                    {
                        vec2 _16709;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _15841 = _7208;
                            _15841.y = 1.0 - _7208.y;
                            _16709 = _15841;
                        }
                        else
                        {
                            _16709 = _7208;
                        }
                        _16710 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _16709, 0.0);
                    }
                    else
                    {
                        vec4 _16711;
                        if (_10528 == 1)
                        {
                            vec2 _16702;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15846 = _10697;
                                _15846.y = 1.0 - _10697.y;
                                _16702 = _15846;
                            }
                            else
                            {
                                _16702 = _10697;
                            }
                            float _10754 = _10697.y;
                            vec2 _10755 = vec2(_10709.x, _10754);
                            vec2 _16703;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15853 = _10755;
                                _15853.y = 1.0 - _10754;
                                _16703 = _15853;
                            }
                            else
                            {
                                _16703 = _10755;
                            }
                            float _10772 = _10709.y;
                            vec2 _10773 = vec2(_10697.x, _10772);
                            vec2 _16705;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15860 = _10773;
                                _15860.y = 1.0 - _10772;
                                _16705 = _15860;
                            }
                            else
                            {
                                _16705 = _10773;
                            }
                            vec2 _16707;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15867 = _10709;
                                _15867.y = 1.0 - _10709.y;
                                _16707 = _15867;
                            }
                            else
                            {
                                _16707 = _10709;
                            }
                            _16711 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16702, 0.0) * (_10681.x * _10681.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16703, 0.0) * (_10685.x * _10681.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16705, 0.0) * (_10681.x * _10685.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16707, 0.0) * (_10685.x * _10685.y));
                        }
                        else
                        {
                            vec4 _16712;
                            if (_10528 == 2)
                            {
                                vec2 _16695;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _15874 = _10697;
                                    _15874.y = 1.0 - _10697.y;
                                    _16695 = _15874;
                                }
                                else
                                {
                                    _16695 = _10697;
                                }
                                float _10884 = _10697.y;
                                vec2 _10885 = vec2(_10709.x, _10884);
                                vec2 _16696;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _15881 = _10885;
                                    _15881.y = 1.0 - _10884;
                                    _16696 = _15881;
                                }
                                else
                                {
                                    _16696 = _10885;
                                }
                                float _10902 = _10709.y;
                                vec2 _10903 = vec2(_10697.x, _10902);
                                vec2 _16698;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _15888 = _10903;
                                    _15888.y = 1.0 - _10902;
                                    _16698 = _15888;
                                }
                                else
                                {
                                    _16698 = _10903;
                                }
                                vec2 _16700;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _15895 = _10709;
                                    _15895.y = 1.0 - _10709.y;
                                    _16700 = _15895;
                                }
                                else
                                {
                                    _16700 = _10709;
                                }
                                _16712 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16695, 0.0) * (_10681.x * _10681.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16696, 0.0) * (_10685.x * _10681.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16698, 0.0) * (_10681.x * _10685.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16700, 0.0) * (_10685.x * _10685.y));
                            }
                            else
                            {
                                vec4 _16713;
                                if (_10528 == 3)
                                {
                                    vec2 _16688;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15902 = _10697;
                                        _15902.y = 1.0 - _10697.y;
                                        _16688 = _15902;
                                    }
                                    else
                                    {
                                        _16688 = _10697;
                                    }
                                    float _11014 = _10697.y;
                                    vec2 _11015 = vec2(_10709.x, _11014);
                                    vec2 _16689;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15909 = _11015;
                                        _15909.y = 1.0 - _11014;
                                        _16689 = _15909;
                                    }
                                    else
                                    {
                                        _16689 = _11015;
                                    }
                                    float _11032 = _10709.y;
                                    vec2 _11033 = vec2(_10697.x, _11032);
                                    vec2 _16691;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15916 = _11033;
                                        _15916.y = 1.0 - _11032;
                                        _16691 = _15916;
                                    }
                                    else
                                    {
                                        _16691 = _11033;
                                    }
                                    vec2 _16693;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _15923 = _10709;
                                        _15923.y = 1.0 - _10709.y;
                                        _16693 = _15923;
                                    }
                                    else
                                    {
                                        _16693 = _10709;
                                    }
                                    _16713 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16688, 0.0) * (_10681.x * _10681.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16689, 0.0) * (_10685.x * _10681.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16691, 0.0) * (_10681.x * _10685.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16693, 0.0) * (_10685.x * _10685.y));
                                }
                                else
                                {
                                    vec4 _16714;
                                    if (_10528 == 4)
                                    {
                                        vec2 _16681;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _15930 = _10697;
                                            _15930.y = 1.0 - _10697.y;
                                            _16681 = _15930;
                                        }
                                        else
                                        {
                                            _16681 = _10697;
                                        }
                                        float _11144 = _10697.y;
                                        vec2 _11145 = vec2(_10709.x, _11144);
                                        vec2 _16682;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _15937 = _11145;
                                            _15937.y = 1.0 - _11144;
                                            _16682 = _15937;
                                        }
                                        else
                                        {
                                            _16682 = _11145;
                                        }
                                        float _11162 = _10709.y;
                                        vec2 _11163 = vec2(_10697.x, _11162);
                                        vec2 _16684;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _15944 = _11163;
                                            _15944.y = 1.0 - _11162;
                                            _16684 = _15944;
                                        }
                                        else
                                        {
                                            _16684 = _11163;
                                        }
                                        vec2 _16686;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _15951 = _10709;
                                            _15951.y = 1.0 - _10709.y;
                                            _16686 = _15951;
                                        }
                                        else
                                        {
                                            _16686 = _10709;
                                        }
                                        _16714 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16681, 0.0) * (_10681.x * _10681.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16682, 0.0) * (_10685.x * _10681.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16684, 0.0) * (_10681.x * _10685.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16686, 0.0) * (_10685.x * _10685.y));
                                    }
                                    else
                                    {
                                        vec2 _16674;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _15958 = _10697;
                                            _15958.y = 1.0 - _10697.y;
                                            _16674 = _15958;
                                        }
                                        else
                                        {
                                            _16674 = _10697;
                                        }
                                        float _11274 = _10697.y;
                                        vec2 _11275 = vec2(_10709.x, _11274);
                                        vec2 _16675;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _15965 = _11275;
                                            _15965.y = 1.0 - _11274;
                                            _16675 = _15965;
                                        }
                                        else
                                        {
                                            _16675 = _11275;
                                        }
                                        float _11292 = _10709.y;
                                        vec2 _11293 = vec2(_10697.x, _11292);
                                        vec2 _16677;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _15972 = _11293;
                                            _15972.y = 1.0 - _11292;
                                            _16677 = _15972;
                                        }
                                        else
                                        {
                                            _16677 = _11293;
                                        }
                                        vec2 _16679;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _15979 = _10709;
                                            _15979.y = 1.0 - _10709.y;
                                            _16679 = _15979;
                                        }
                                        else
                                        {
                                            _16679 = _10709;
                                        }
                                        _16714 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16674, 0.0) * (_10681.x * _10681.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16675, 0.0) * (_10685.x * _10681.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16677, 0.0) * (_10681.x * _10685.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16679, 0.0) * (_10685.x * _10685.y));
                                    }
                                    _16713 = _16714;
                                }
                                _16712 = _16713;
                            }
                            _16711 = _16712;
                        }
                        _16710 = _16711;
                    }
                    _10542 = _16743 + (_16710.xyz * (_10513 ? ((_16673 == 0) ? (1.0 - _10508) : _10508) : 1.0));
                }
                vec3 _15983 = _16647;
                _15983.x = _16743.x;
                vec2 _7219 = (_7159 + _7171) + (_7174 * _7201);
                float _11389 = clamp(log2(max(_7073, 1.0)) - 1.0, 0.0, 5.0);
                int _11392 = int(floor(_11389));
                float _11396 = _11389 - float(_11392);
                bool _11401 = (_11396 > 0.0199999995529651641845703125) && (_11392 < 5);
                vec3 _16867;
                _16867 = vec3(0.0);
                vec3 _11430;
                SPIRV_CROSS_LOOP
                for (int _16797 = 0; _16797 < 2; _16867 = _11430, _16797++)
                {
                    if ((_16797 > 0) && (!_11401))
                    {
                        break;
                    }
                    int _11416 = _11392 + _16797;
                    int _11454 = clamp(_11416, 1, 5);
                    vec2 _11523 = (_7219 * _184.gLevel[_11454].xy) - vec2(0.5);
                    vec2 _11525 = floor(_11523);
                    vec2 _11528 = _11523 - _11525;
                    vec2 _11531 = _11528 * _11528;
                    vec2 _11534 = _11531 * _11528;
                    vec2 _11553 = (((_11534 * 3.0) - (_11531 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                    vec2 _11566 = _11534 * 0.16666667163372039794921875;
                    vec2 _11569 = (((((-_11534) + (_11531 * 3.0)) - (_11528 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11553;
                    vec2 _11573 = (((((_11534 * (-3.0)) + (_11531 * 3.0)) + (_11528 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11566;
                    vec2 _11585 = ((_11525 - vec2(0.5)) + (_11553 / _11569)) * _184.gLevel[_11454].zw;
                    vec2 _11597 = ((_11525 + vec2(1.5)) + (_11566 / _11573)) * _184.gLevel[_11454].zw;
                    vec4 _16834;
                    SPIRV_CROSS_BRANCH
                    if (_11416 <= 0)
                    {
                        vec2 _16833;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _15986 = _7219;
                            _15986.y = 1.0 - _7219.y;
                            _16833 = _15986;
                        }
                        else
                        {
                            _16833 = _7219;
                        }
                        _16834 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _16833, 0.0);
                    }
                    else
                    {
                        vec4 _16835;
                        if (_11416 == 1)
                        {
                            vec2 _16826;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15991 = _11585;
                                _15991.y = 1.0 - _11585.y;
                                _16826 = _15991;
                            }
                            else
                            {
                                _16826 = _11585;
                            }
                            float _11642 = _11585.y;
                            vec2 _11643 = vec2(_11597.x, _11642);
                            vec2 _16827;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _15998 = _11643;
                                _15998.y = 1.0 - _11642;
                                _16827 = _15998;
                            }
                            else
                            {
                                _16827 = _11643;
                            }
                            float _11660 = _11597.y;
                            vec2 _11661 = vec2(_11585.x, _11660);
                            vec2 _16829;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16005 = _11661;
                                _16005.y = 1.0 - _11660;
                                _16829 = _16005;
                            }
                            else
                            {
                                _16829 = _11661;
                            }
                            vec2 _16831;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16012 = _11597;
                                _16012.y = 1.0 - _11597.y;
                                _16831 = _16012;
                            }
                            else
                            {
                                _16831 = _11597;
                            }
                            _16835 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16826, 0.0) * (_11569.x * _11569.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16827, 0.0) * (_11573.x * _11569.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16829, 0.0) * (_11569.x * _11573.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16831, 0.0) * (_11573.x * _11573.y));
                        }
                        else
                        {
                            vec4 _16836;
                            if (_11416 == 2)
                            {
                                vec2 _16819;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16019 = _11585;
                                    _16019.y = 1.0 - _11585.y;
                                    _16819 = _16019;
                                }
                                else
                                {
                                    _16819 = _11585;
                                }
                                float _11772 = _11585.y;
                                vec2 _11773 = vec2(_11597.x, _11772);
                                vec2 _16820;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16026 = _11773;
                                    _16026.y = 1.0 - _11772;
                                    _16820 = _16026;
                                }
                                else
                                {
                                    _16820 = _11773;
                                }
                                float _11790 = _11597.y;
                                vec2 _11791 = vec2(_11585.x, _11790);
                                vec2 _16822;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16033 = _11791;
                                    _16033.y = 1.0 - _11790;
                                    _16822 = _16033;
                                }
                                else
                                {
                                    _16822 = _11791;
                                }
                                vec2 _16824;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16040 = _11597;
                                    _16040.y = 1.0 - _11597.y;
                                    _16824 = _16040;
                                }
                                else
                                {
                                    _16824 = _11597;
                                }
                                _16836 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16819, 0.0) * (_11569.x * _11569.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16820, 0.0) * (_11573.x * _11569.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16822, 0.0) * (_11569.x * _11573.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16824, 0.0) * (_11573.x * _11573.y));
                            }
                            else
                            {
                                vec4 _16837;
                                if (_11416 == 3)
                                {
                                    vec2 _16812;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16047 = _11585;
                                        _16047.y = 1.0 - _11585.y;
                                        _16812 = _16047;
                                    }
                                    else
                                    {
                                        _16812 = _11585;
                                    }
                                    float _11902 = _11585.y;
                                    vec2 _11903 = vec2(_11597.x, _11902);
                                    vec2 _16813;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16054 = _11903;
                                        _16054.y = 1.0 - _11902;
                                        _16813 = _16054;
                                    }
                                    else
                                    {
                                        _16813 = _11903;
                                    }
                                    float _11920 = _11597.y;
                                    vec2 _11921 = vec2(_11585.x, _11920);
                                    vec2 _16815;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16061 = _11921;
                                        _16061.y = 1.0 - _11920;
                                        _16815 = _16061;
                                    }
                                    else
                                    {
                                        _16815 = _11921;
                                    }
                                    vec2 _16817;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16068 = _11597;
                                        _16068.y = 1.0 - _11597.y;
                                        _16817 = _16068;
                                    }
                                    else
                                    {
                                        _16817 = _11597;
                                    }
                                    _16837 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16812, 0.0) * (_11569.x * _11569.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16813, 0.0) * (_11573.x * _11569.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16815, 0.0) * (_11569.x * _11573.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16817, 0.0) * (_11573.x * _11573.y));
                                }
                                else
                                {
                                    vec4 _16838;
                                    if (_11416 == 4)
                                    {
                                        vec2 _16805;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16075 = _11585;
                                            _16075.y = 1.0 - _11585.y;
                                            _16805 = _16075;
                                        }
                                        else
                                        {
                                            _16805 = _11585;
                                        }
                                        float _12032 = _11585.y;
                                        vec2 _12033 = vec2(_11597.x, _12032);
                                        vec2 _16806;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16082 = _12033;
                                            _16082.y = 1.0 - _12032;
                                            _16806 = _16082;
                                        }
                                        else
                                        {
                                            _16806 = _12033;
                                        }
                                        float _12050 = _11597.y;
                                        vec2 _12051 = vec2(_11585.x, _12050);
                                        vec2 _16808;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16089 = _12051;
                                            _16089.y = 1.0 - _12050;
                                            _16808 = _16089;
                                        }
                                        else
                                        {
                                            _16808 = _12051;
                                        }
                                        vec2 _16810;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16096 = _11597;
                                            _16096.y = 1.0 - _11597.y;
                                            _16810 = _16096;
                                        }
                                        else
                                        {
                                            _16810 = _11597;
                                        }
                                        _16838 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16805, 0.0) * (_11569.x * _11569.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16806, 0.0) * (_11573.x * _11569.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16808, 0.0) * (_11569.x * _11573.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16810, 0.0) * (_11573.x * _11573.y));
                                    }
                                    else
                                    {
                                        vec2 _16798;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16103 = _11585;
                                            _16103.y = 1.0 - _11585.y;
                                            _16798 = _16103;
                                        }
                                        else
                                        {
                                            _16798 = _11585;
                                        }
                                        float _12162 = _11585.y;
                                        vec2 _12163 = vec2(_11597.x, _12162);
                                        vec2 _16799;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16110 = _12163;
                                            _16110.y = 1.0 - _12162;
                                            _16799 = _16110;
                                        }
                                        else
                                        {
                                            _16799 = _12163;
                                        }
                                        float _12180 = _11597.y;
                                        vec2 _12181 = vec2(_11585.x, _12180);
                                        vec2 _16801;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16117 = _12181;
                                            _16117.y = 1.0 - _12180;
                                            _16801 = _16117;
                                        }
                                        else
                                        {
                                            _16801 = _12181;
                                        }
                                        vec2 _16803;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16124 = _11597;
                                            _16124.y = 1.0 - _11597.y;
                                            _16803 = _16124;
                                        }
                                        else
                                        {
                                            _16803 = _11597;
                                        }
                                        _16838 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16798, 0.0) * (_11569.x * _11569.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16799, 0.0) * (_11573.x * _11569.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16801, 0.0) * (_11569.x * _11573.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16803, 0.0) * (_11573.x * _11573.y));
                                    }
                                    _16837 = _16838;
                                }
                                _16836 = _16837;
                            }
                            _16835 = _16836;
                        }
                        _16834 = _16835;
                    }
                    _11430 = _16867 + (_16834.xyz * (_11401 ? ((_16797 == 0) ? (1.0 - _11396) : _11396) : 1.0));
                }
                _15983.z = _16867.z;
                _17037 = _15983;
            }
            else
            {
                _17037 = _16647;
            }
            vec3 _17257;
            SPIRV_CROSS_BRANCH
            if (_7101 > 0.0)
            {
                vec2 _7242 = _7159 + (((_16575 * min(_7092 * 0.5, 16.0)) * _184.gDisplay.zw) * _184.gTarget.zw);
                float _12277 = clamp(log2(max(16.0 * _184.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _12280 = int(floor(_12277));
                float _12284 = _12277 - float(_12280);
                bool _12289 = (_12284 > 0.0199999995529651641845703125) && (_12280 < 5);
                vec3 _17022;
                _17022 = vec3(0.0);
                vec3 _12318;
                SPIRV_CROSS_LOOP
                for (int _17007 = 0; _17007 < 2; _17022 = _12318, _17007++)
                {
                    if ((_17007 > 0) && (!_12289))
                    {
                        break;
                    }
                    int _12304 = _12280 + _17007;
                    vec2 _17008;
                    if (_184.gConv.x > 0.5)
                    {
                        vec2 _16131 = _7242;
                        _16131.y = 1.0 - _7242.y;
                        _17008 = _16131;
                    }
                    else
                    {
                        _17008 = _7242;
                    }
                    vec4 _17009;
                    SPIRV_CROSS_BRANCH
                    if (_12304 <= 0)
                    {
                        _17009 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _17008, 0.0);
                    }
                    else
                    {
                        vec4 _17010;
                        if (_12304 == 1)
                        {
                            _17010 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17008, 0.0);
                        }
                        else
                        {
                            vec4 _17011;
                            if (_12304 == 2)
                            {
                                _17011 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17008, 0.0);
                            }
                            else
                            {
                                vec4 _17012;
                                if (_12304 == 3)
                                {
                                    _17012 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17008, 0.0);
                                }
                                else
                                {
                                    vec4 _17013;
                                    if (_12304 == 4)
                                    {
                                        _17013 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17008, 0.0);
                                    }
                                    else
                                    {
                                        _17013 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17008, 0.0);
                                    }
                                    _17012 = _17013;
                                }
                                _17011 = _17012;
                            }
                            _17010 = _17011;
                        }
                        _17009 = _17010;
                    }
                    _12318 = _17022 + (_17009.xyz * (_12289 ? ((_17007 == 0) ? (1.0 - _12284) : _12284) : 1.0));
                }
                _17257 = _17022;
            }
            else
            {
                _17257 = vec3(0.5);
            }
            float _17053;
            SPIRV_CROSS_BRANCH
            if (esia_v5.x > 0.001000000047497451305389404296875)
            {
                int _12407 = clamp(int(roundEven(log2(36.0 * _184.gDisplay.z) - 1.0)), 1, 4);
                vec2 _17028;
                if (_184.gConv.x > 0.5)
                {
                    vec2 _16135 = _7159;
                    _16135.y = 1.0 - _7159.y;
                    _17028 = _16135;
                }
                else
                {
                    _17028 = _7159;
                }
                vec4 _17029;
                SPIRV_CROSS_BRANCH
                if (_12407 <= 0)
                {
                    _17029 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _17028, 0.0);
                }
                else
                {
                    vec4 _17030;
                    if (_12407 == 1)
                    {
                        _17030 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17028, 0.0);
                    }
                    else
                    {
                        vec4 _17031;
                        if (_12407 == 2)
                        {
                            _17031 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17028, 0.0);
                        }
                        else
                        {
                            vec4 _17032;
                            if (_12407 == 3)
                            {
                                _17032 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17028, 0.0);
                            }
                            else
                            {
                                vec4 _17033;
                                if (_12407 == 4)
                                {
                                    _17033 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17028, 0.0);
                                }
                                else
                                {
                                    _17033 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17028, 0.0);
                                }
                                _17032 = _17033;
                            }
                            _17031 = _17032;
                        }
                        _17030 = _17031;
                    }
                    _17029 = _17030;
                }
                _17053 = dot(_17029.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _17053 = 0.5;
            }
            _17255 = _17257;
            _17052 = _17053;
            _17034 = _17037;
        }
        else
        {
            _17255 = vec3(0.5);
            _17052 = 0.5;
            _17034 = vec3(0.5);
        }
        vec3 _7270 = mix(vec3(dot(_17034, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _17034, vec3(_6969.x)) + vec3(_6969.y);
        vec3 _17148;
        SPIRV_CROSS_BRANCH
        if (esia_v5.x > 0.001000000047497451305389404296875)
        {
            float _7280 = clamp(max(_17052 + _6969.y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _7288 = mix(_7280, dot(_6928.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), esia_v5.x);
            vec3 _7296 = _7270 * (_7288 / _7280);
            vec3 _7306 = mix(_7270, vec3(1.0), vec3((_7288 - _7280) / max(1.0 - _7280, 0.001000000047497451305389404296875)));
            bvec3 _7307 = bvec3(_7288 < _7280);
            _17148 = vec3(_7307.x ? _7296.x : _7306.x, _7307.y ? _7296.y : _7306.y, _7307.z ? _7296.z : _7306.z);
        }
        else
        {
            _17148 = _7270;
        }
        vec2 _7326 = clamp((esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875)), vec2(0.0), vec2(1.0));
        vec2 _7374 = vec2(cos(_6969.w), sin(_6969.w));
        float _7377 = dot(_16575, _7374);
        float _7395 = _4007 * (abs(_16575.x) + abs(_16575.y));
        float _17368;
        _17368 = 0.0;
        SPIRV_CROSS_UNROLL
        for (int _17367 = -2; _17367 <= 2; )
        {
            float _7412 = (-_16388) + ((float(_17367) * 0.4000000059604644775390625) * _7395);
            float _12508 = clamp(1.0 - (_7412 / _7092), 0.0, 1.0);
            float _12516 = 1.0 - max(sqrt(clamp(1.0 - (_12508 * _12508), 0.0, 1.0)), 0.12399999797344207763671875);
            float _12519 = _12516 * _12516;
            _17368 += ((3.0 - abs(float(_17367))) * ((_7412 < 0.0) ? 0.0 : ((_12519 * _12519) * _12516)));
            _17367++;
            continue;
        }
        float _7448 = clamp(0.5 + (0.5 * dot((esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5)) / _7084, _7374)), 0.0, 1.0);
        _18280 = (((_17368 * 0.111111111938953399658203125) * (pow(clamp(_7377, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(clamp(-_7377, 0.0, 1.0), 1.5)))) * _6969.z) * 1.60000002384185791015625;
        _17899 = _7010;
        _17886 = vec4((mix(mix(_17148, _6928.xyz, vec3(clamp(_6928.w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _7107))) + (0.3499999940395355224609375 * ((dot(_6928.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _7326.y) : _7326.y))), 0.0, 1.0))), (_17255 * 1.10000002384185791015625) + vec3(0.07999999821186065673828125), vec3(pow(1.0 - _17152.z, 5.0) * 0.3499999940395355224609375)) + vec3((((0.039999999105930328369140625 * _7448) * _7448) + (0.0500000007450580596923828125 * (1.0 - _7107))) * _6969.z)) * _4032, _4032) + (_17375 * (1.0 - _4032));
    }
    else
    {
        _18280 = 0.0;
        _17899 = _16368;
        _17886 = _17375;
    }
    vec4 _17896;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 1u) != 0u)
    {
        vec4 _17517;
        vec4 _17695;
        SPIRV_CROSS_BRANCH
        if (esia_v7.y != 0u)
        {
            uint _12543 = max(uint(_184.gConv.z), 1u);
            uint _12584 = max(uint(_184.gConv.z), 1u);
            _17695 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _12543) * 24u) + 3u), int(esia_v1 / _12543), 0).xy, 0);
            _17517 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _12584) * 24u) + 4u), int(esia_v1 / _12584), 0).xy, 0);
        }
        else
        {
            _17695 = vec4(0.0);
            _17517 = vec4(0.0);
        }
        vec4 _17881;
        do
        {
            if (esia_v7.y == 0u)
            {
                _17881 = esia_v4;
                break;
            }
            vec2 _12656 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            vec2 _12666 = fwidth(esia_v0);
            float _12668 = max(length(_12666), 9.9999997473787516355514526367188e-05);
            float _17873;
            float _17877;
            if ((esia_v7.y == 1u) || (esia_v7.y == 4u))
            {
                float _12677 = cos(_17517.x);
                float _12680 = sin(_17517.x);
                float _12706 = ((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_12677, _12680)) / max(0.5 * ((abs(_12677) * _12656.x) + (abs(_12680) * _12656.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (esia_v7.y == 4u)
                {
                    float _12721 = clamp((_12706 - _17517.y) / max(_17517.z - _17517.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _17881 = vec4(clamp(abs((fract(vec3(_12721 * 0.800000011920928955078125) + vec3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - vec3(3.0)) - vec3(1.0), vec3(0.0), vec3(1.0)), esia_v4.w * pow(max(sin(_12721 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _17877 = -1.0;
                _17873 = _12706;
            }
            else
            {
                float _17874;
                float _17878;
                if (esia_v7.y == 2u)
                {
                    _17878 = -1.0;
                    _17874 = length(esia_v0 - (esia_v2.xy + (_17517.xy * _12656))) / max(_17517.z * max(_12656.x, _12656.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    vec2 _12789 = esia_v0 - (esia_v2.xy + (_17517.xy * _12656));
                    float _12800 = fract(((atan(_12789.y, _12789.x) - _17517.z) * 0.15915493667125701904296875) + 1.0);
                    float _17875;
                    float _17879;
                    if (_17517.w > 0.5)
                    {
                        _17879 = -1.0;
                        _17875 = 0.5 - (0.5 * cos(_12800 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _12820 = (((_12800 < 0.5) ? _12800 : (_12800 - 1.0)) * 6.283185482025146484375) * length(_12789);
                        float _17880;
                        if (abs(_12820) < _12668)
                        {
                            _17880 = clamp(((_12820 / _12668) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _17880 = -1.0;
                        }
                        _17879 = _17880;
                        _17875 = _12800;
                    }
                    _17878 = _17879;
                    _17874 = _17875;
                }
                _17877 = _17878;
                _17873 = _17874;
            }
            vec4 _12849 = vec4(esia_v4.xyz * esia_v4.w, esia_v4.w);
            vec4 _12861 = vec4(_17695.xyz * _17695.w, _17695.w);
            vec4 _12868 = mix(_12861, _12849, vec4(_17877));
            vec4 _12873 = mix(_12849, _12861, vec4(clamp(_17873, 0.0, 1.0)));
            bvec4 _12874 = bvec4(_17877 >= 0.0);
            vec4 _12875 = vec4(_12874.x ? _12868.x : _12873.x, _12874.y ? _12868.y : _12873.y, _12874.z ? _12868.z : _12873.z, _12874.w ? _12868.w : _12873.w);
            vec4 _12890 = vec4(_12875.xyz / vec3(_12875.w), _12875.w);
            bvec4 _12891 = bvec4(_12875.w > 9.9999997473787516355514526367188e-06);
            _17881 = vec4(_12891.x ? _12890.x : vec4(0.0).x, _12891.y ? _12890.y : vec4(0.0).y, _12891.z ? _12890.z : vec4(0.0).z, _12891.w ? _12890.w : vec4(0.0).w);
            break;
        } while(false);
        vec4 _17882;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & 64u) != 0u)
        {
            uint _12901 = max(uint(_184.gConv.z), 1u);
            vec4 _12935 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _12901) * 24u) + 18u), int(esia_v1 / _12901), 0).xy, 0);
            _17882 = _17881 * texture(SPIRV_Cross_CombinedgTexgLinear, mix(_12935.xy, _12935.zw, (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875))));
        }
        else
        {
            _17882 = _17881;
        }
        float _12943 = clamp(_17882.w * _4032, 0.0, 1.0);
        _17896 = vec4(_17882.xyz * _12943, _12943) + (_17886 * (1.0 - _12943));
    }
    else
    {
        _17896 = _17886;
    }
    vec4 _18266;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 16384u) != 0u)
    {
        uint _12968 = max(uint(_184.gConv.z), 1u);
        vec4 _13002 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _12968) * 24u) + 21u), int(esia_v1 / _12968), 0).xy, 0);
        uint _13009 = max(uint(_184.gConv.z), 1u);
        vec4 _13043 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13009) * 24u) + 3u), int(esia_v1 / _13009), 0).xy, 0);
        vec2 _4472 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        vec2 _4479 = (esia_v0 - esia_v2.xy) / _4472;
        float _4490 = ((_13043.x >= 0.0) ? _13043.x : _184.gTime.x) * _13002.z;
        float _4493 = _4479.x * 2.0;
        float _4494 = _4493 - 1.0;
        float _4499 = _4479.y * _4472.y;
        float _4502 = max(_13002.w, 0.001000000047497451305389404296875);
        float _4507 = clamp(1.0 - (_4494 * _4494), 0.0, 1.0);
        float _4509 = pow(_4507, 1.2999999523162841796875);
        float _4511 = pow(_4507, 0.699999988079071044921875);
        float _4514 = clamp(_4490 * 1.4285714626312255859375, 0.0, 1.0);
        float _4530 = (0.25 + (0.75 * ((_4514 * _4514) * (3.0 - (2.0 * _4514))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4490 * 2.099999904632568359375)));
        float _4539 = ((_13002.y * _4472.y) * _4530) * _4509;
        float _4565 = (_4472.y * (0.5 + ((_13002.x * (0.5 - (_4494 * _4494))) * 0.5))) + (((0.14000000059604644775390625 * _4472.y) * _4509) * sin(((_4494 * 2.400000095367431640625) - (_4490 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _17892;
        float _17893;
        vec3 _17894;
        _17894 = vec3(0.0);
        _17893 = _4565;
        _17892 = _4565;
        vec3 _4637;
        float _18528;
        float _18529;
        SPIRV_CROSS_UNROLL
        for (int _17891 = 0; _17891 < 4; _17894 = _4637, _17893 = _18529, _17892 = _18528, _17891++)
        {
            float _4589 = _4565 + ((_4539 * _3038[_17891].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4490 * 1.7000000476837158203125) + _3055[_17891].y))));
            _18528 = (_17891 == 0) ? _4589 : _17892;
            _18529 = (_17891 == 2) ? _4589 : _17893;
            float _4604 = _4502 * _3038[_17891].y;
            float _4609 = (_4499 - _4589) / _4604;
            float _4614 = _4502 * _3038[_17891].z;
            vec3 _18502;
            _18502 = vec3(0.0);
            SPIRV_CROSS_UNROLL
            for (int _18501 = 0; _18501 < 6; )
            {
                float _13066 = ((_4499 - _4589) - (_4614 * ((float(_18501) * 0.4000000059604644775390625) - 1.0))) / _4604;
                _18502 += (_1756[_18501] * exp((-_13066) * _13066));
                _18501++;
                continue;
            }
            _4637 = _17894 + (mix(_18502 * vec3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), vec3(exp((-_4609) * _4609)), vec3(_3055[_17891].x)) * (_3038[_17891].w * _4511));
        }
        float _4643 = _4502 * 1.5;
        float _4673 = clamp((_4499 - _17892) / max(_17893 - _17892, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4701 = (_4499 - (_17893 - (_4502 * 3.0))) / (((_4472.y * 0.0900000035762786865234375) + (_4539 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4704 = (_4493 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4729 = ((_4499 - _17892) + (_4502 * 5.0)) / (_4502 * 7.0);
        vec3 _4755 = vec3(1.0) - exp((-((((_17894 + (mix(vec3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), vec3(1.0), vec3(_4673)) * ((((1.0 / (1.0 + exp((-((_4499 - _17892) - (_4502 * 2.0))) / _4643))) / (1.0 + exp((-(_17893 - _4499)) / _4643))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4673, 2.5)))) * _4511))) + (vec3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4701) * _4701) - (_4704 * _4704)) * (0.5 + (1.10000002384185791015625 * _4530))))) + (vec3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4729) * _4729) * _4509) * 0.100000001490116119384765625))) * mix(vec3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), vec3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), vec3(0.5 + (0.5 * sin(_4490 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4765 = (esia_v4.w * smoothstep(0.0, 0.119999997317790985107421875, _4479.y)) * smoothstep(1.0, 0.87999999523162841796875, _4479.y);
        float _4784 = (clamp(max(_4755.x, max(_4755.y, _4755.z)), 0.0, 1.0) * _4765) * _4032;
        _18266 = vec4((_4755 * _4765) * _4032, _4784) + (_17896 * (1.0 - _4784));
    }
    else
    {
        _18266 = _17896;
    }
    vec4 _18275;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 4u) != 0u) && ((esia_v7.x & 256u) != 0u))
    {
        uint _13099 = max(uint(_184.gConv.z), 1u);
        vec4 _13133 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13099) * 24u) + 7u), int(esia_v1 / _13099), 0).xy, 0);
        uint _13140 = max(uint(_184.gConv.z), 1u);
        vec4 _13174 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13140) * 24u) + 8u), int(esia_v1 / _13140), 0).xy, 0);
        vec2 _4818 = esia_v0 - _13174.zw;
        vec2 _13213 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _13222 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _18241;
        SPIRV_CROSS_BRANCH
        if (esia_v7.z == 1u)
        {
            vec2 _13228 = _4818 - _13213;
            float _18240;
            do
            {
                if (esia_v5.w >= 6.282185077667236328125)
                {
                    _18240 = abs(length(_13228) - esia_v3.x) - esia_v3.y;
                    break;
                }
                float _13327 = esia_v5.z + (esia_v5.w * 0.5);
                float _13329 = cos(_13327);
                float _13331 = sin(_13327);
                float _13340 = dot(_13228, vec2(-_13331, _13329));
                float _13343 = dot(_13228, vec2(_13329, _13331));
                vec2 _13344 = vec2(_13340, _13343);
                float _13347 = abs(_13340);
                _13344.x = _13347;
                float _13350 = esia_v5.w * 0.5;
                float _13352 = sin(_13350);
                float _13354 = cos(_13350);
                _18240 = (((_13354 * _13347) > (_13352 * _13343)) ? length(_13344 - (vec2(_13352, _13354) * esia_v3.x)) : abs(length(_13344) - esia_v3.x)) - esia_v3.y;
                break;
            } while(false);
            _18241 = _18240;
        }
        else
        {
            float _18242;
            if (esia_v7.z == 2u)
            {
                vec2 _13390 = _4818 - _16370.xy;
                vec2 _13393 = _16370.zw - _16370.xy;
                _18242 = length(_13390 - (_13393 * clamp(dot(_13390, _13393) / max(dot(_13393, _13393), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
            }
            else
            {
                vec2 _13255 = _4818 - _13213;
                float _13446 = min(_13222.x, _13222.y);
                float _13449 = min((_13255.x > 0.0) ? ((_13255.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_13255.y > 0.0) ? esia_v3.w : esia_v3.x), _13446);
                float _13455 = _13449 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _18225;
                float _18226;
                if (_13455 > _13446)
                {
                    float _13469 = esia_v5.y * clamp((_13446 - _13449) / max(0.60000002384185791015625 * _13449, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _18226 = _13469;
                    _18225 = _13449 * (1.0 + (0.60000002384185791015625 * _13469));
                }
                else
                {
                    _18226 = esia_v5.y;
                    _18225 = _13455;
                }
                vec2 _13482 = (abs(_13255) - _13222) + vec2(_18225);
                vec2 _13484 = max(_13482, vec2(0.0));
                float _18227;
                SPIRV_CROSS_BRANCH
                if ((_13484.x > 0.0) && (_13484.y > 0.0))
                {
                    float _18228;
                    if ((_18226 > 0.001000000047497451305389404296875) && (_18225 > 9.9999997473787516355514526367188e-05))
                    {
                        float _13501 = 2.0 + (2.0 * _18226);
                        vec2 _13506 = _13484 / vec2(max(_18225, 9.9999997473787516355514526367188e-05));
                        _18228 = pow(pow(_13506.x, _13501) + pow(_13506.y, _13501), 1.0 / _13501) * _18225;
                    }
                    else
                    {
                        _18228 = length(_13484);
                    }
                    _18227 = _18228;
                }
                else
                {
                    _18227 = max(_13484.x, _13484.y);
                }
                float _13541 = (min(max(_13482.x, _13482.y), 0.0) + _18227) - _18225;
                float _18243;
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & 512u) != 0u)
                {
                    vec2 _13283 = max((_16370.zw - _16370.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _13286 = _4818 - ((_16370.xy + _16370.zw) * 0.5);
                    float _13577 = min(_13283.x, _13283.y);
                    float _13580 = min((_13286.x > 0.0) ? ((_13286.y > 0.0) ? _17899.x : _17899.x) : ((_13286.y > 0.0) ? _17899.x : _17899.x), _13577);
                    float _13586 = _13580 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _18231;
                    float _18232;
                    if (_13586 > _13577)
                    {
                        float _13600 = esia_v5.y * clamp((_13577 - _13580) / max(0.60000002384185791015625 * _13580, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _18232 = _13600;
                        _18231 = _13580 * (1.0 + (0.60000002384185791015625 * _13600));
                    }
                    else
                    {
                        _18232 = esia_v5.y;
                        _18231 = _13586;
                    }
                    vec2 _13613 = (abs(_13286) - _13283) + vec2(_18231);
                    vec2 _13615 = max(_13613, vec2(0.0));
                    float _18233;
                    SPIRV_CROSS_BRANCH
                    if ((_13615.x > 0.0) && (_13615.y > 0.0))
                    {
                        float _18234;
                        if ((_18232 > 0.001000000047497451305389404296875) && (_18231 > 9.9999997473787516355514526367188e-05))
                        {
                            float _13632 = 2.0 + (2.0 * _18232);
                            vec2 _13637 = _13615 / vec2(max(_18231, 9.9999997473787516355514526367188e-05));
                            _18234 = pow(pow(_13637.x, _13632) + pow(_13637.y, _13632), 1.0 / _13632) * _18231;
                        }
                        else
                        {
                            _18234 = length(_13615);
                        }
                        _18233 = _18234;
                    }
                    else
                    {
                        _18233 = max(_13615.x, _13615.y);
                    }
                    float _13672 = (min(max(_13613.x, _13613.y), 0.0) + _18233) - _18231;
                    float _13677 = max(_17899.y, 9.9999997473787516355514526367188e-05);
                    float _13686 = max(_13677 - abs(_13541 - _13672), 0.0) / _13677;
                    _18243 = min(_13541, _13672) - (((_13686 * _13686) * _13677) * 0.25);
                }
                else
                {
                    _18243 = _13541;
                }
                _18242 = _18243;
            }
            _18241 = _18242;
        }
        float _4827 = (_18241 + _13174.y) / (max(_13174.x * 0.5, _4007 * 0.5) * 1.41421353816986083984375);
        float _13703 = sign(_4827);
        float _13705 = abs(_4827);
        float _13716 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_13705 * _13705))) * _13705)) * _13705);
        float _13719 = _13716 * _13716;
        float _13734 = clamp(_13133.w * ((0.5 + (0.5 * (_13703 - (_13703 / (_13719 * _13719))))) * _4032), 0.0, 1.0);
        _18275 = vec4(_13133.xyz * _13734, _13734) + (_18266 * (1.0 - _13734));
    }
    else
    {
        _18275 = _18266;
    }
    vec4 _18299;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 16u) != 0u)
    {
        uint _13759 = max(uint(_184.gConv.z), 1u);
        vec4 _13793 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13759) * 24u) + 9u), int(esia_v1 / _13759), 0).xy, 0);
        uint _13800 = max(uint(_184.gConv.z), 1u);
        vec4 _13834 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13800) * 24u) + 10u), int(esia_v1 / _13800), 0).xy, 0);
        float _4859 = max(-_16388, 0.0) / max(_13834.z, 0.001000000047497451305389404296875);
        float _13842 = clamp(_13793.w * clamp((exp(((-_4859) * _4859) * 2.2000000476837158203125) * _13834.w) * _4032, 0.0, 1.0), 0.0, 1.0);
        _18299 = vec4(_13793.xyz * _13842, _13842) + (_18275 * (1.0 - _13842));
    }
    else
    {
        _18299 = _18275;
    }
    vec3 _4890 = _18299.xyz + vec3(_18280 * clamp(_18299.w / max(_4032, 0.001000000047497451305389404296875), 0.0, 1.0));
    vec4 _16263 = _18299;
    _16263.x = _4890.x;
    _16263.y = _4890.y;
    _16263.z = _4890.z;
    vec4 _18302;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 2u) != 0u)
    {
        uint _13867 = max(uint(_184.gConv.z), 1u);
        vec4 _13901 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13867) * 24u) + 5u), int(esia_v1 / _13867), 0).xy, 0);
        uint _13908 = max(uint(_184.gConv.z), 1u);
        vec4 _13942 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13908) * 24u) + 6u), int(esia_v1 / _13908), 0).xy, 0);
        float _4910 = _13942.x;
        float _4912 = _13942.y;
        vec4 _18300;
        if (_13942.z < 0.999000012874603271484375)
        {
            vec2 _4948 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            float _4959 = cos(_13942.w);
            float _4962 = sin(_13942.w);
            vec4 _16280 = _13901;
            _16280.w = _13901.w * mix(1.0, _13942.z, clamp(((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_4959, _4962)) / max(0.5 * ((abs(_4959) * _4948.x) + (abs(_4962) * _4948.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _18300 = _16280;
        }
        else
        {
            _18300 = _13901;
        }
        float _13950 = clamp(_18300.w * (clamp(0.5 - ((_16388 - (_4910 * _4912)) / _4007), 0.0, 1.0) - clamp(0.5 - ((_16388 + (_4910 * (1.0 - _4912))) / _4007), 0.0, 1.0)), 0.0, 1.0);
        _18302 = vec4(_18300.xyz * _13950, _13950) + (_16263 * (1.0 - _13950));
    }
    else
    {
        _18302 = _16263;
    }
    vec4 _18303;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 128u) != 0u)
    {
        vec2 _5023 = (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        float _5045 = exp(-pow((((_5023.x * 0.85000002384185791015625) + (_5023.y * 0.1500000059604644775390625)) - ((fract(_184.gTime.x * esia_v6.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _18303 = vec4(_18302.xyz + vec3(((_5045 * esia_v6.z) * _4032) * max(_18302.w, 0.3499999940395355224609375)), max(_18302.w, ((_5045 * esia_v6.z) * _4032) * 0.5));
    }
    else
    {
        _18303 = _18302;
    }
    vec4 _18496;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 2048u) != 0u)
    {
        vec3 _13975 = fract(floor(_16367).xyx * 0.103100001811981201171875);
        vec3 _13984 = _13975 + vec3(dot(_13975, _13975.yzx + vec3(33.3300018310546875)));
        vec3 _5096 = _18303.xyz + vec3(((fract((_13984.x + _13984.y) * _13984.z) - 0.5) * esia_v6.y) * _18303.w);
        vec4 _16304 = _18303;
        _16304.x = _5096.x;
        _16304.y = _5096.y;
        _16304.z = _5096.z;
        _18496 = _16304;
    }
    else
    {
        _18496 = _18303;
    }
    float _18492;
    if (_227.gFade.z > 0.0)
    {
        _18492 = smoothstep(0.0, 1.0, clamp((_16367.y - _227.gFade.x) / _227.gFade.z, 0.0, 1.0));
    }
    else
    {
        _18492 = 1.0;
    }
    float _18493;
    if (_227.gFade.w > 0.0)
    {
        _18493 = _18492 * smoothstep(0.0, 1.0, clamp((_227.gFade.y - _16367.y) / _227.gFade.w, 0.0, 1.0));
    }
    else
    {
        _18493 = _18492;
    }
    vec4 _5113 = _18496 * ((esia_v6.x * _18314) * _18493);
    vec4 _18497;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 8u) != 0u) || ((esia_v7.x & 4u) != 0u))
    {
        vec3 _14036 = fract((floor(_16367) + vec2(17.0)).xyx * 0.103100001811981201171875);
        vec3 _14045 = _14036 + vec3(dot(_14036, _14036.yzx + vec3(33.3300018310546875)));
        vec3 _5137 = _5113.xyz + vec3(((fract((_14045.x + _14045.y) * _14045.z) - 0.5) * 0.0039215688593685626983642578125) * clamp(_5113.w * 8.0, 0.0, 1.0));
        vec4 _16316 = _5113;
        _16316.x = _5137.x;
        _16316.y = _5137.y;
        _16316.z = _5137.z;
        _18497 = _16316;
    }
    else
    {
        _18497 = _5113;
    }
    vec3 _5147 = max(_18497.xyz, vec3(0.0));
    vec4 _16322 = _18497;
    _16322.x = _5147.x;
    _16322.y = _5147.y;
    _16322.z = _5147.z;
    vec4 _18498;
    if ((_184.gTime.w > 0.5) && (_18497.w > 9.9999997473787516355514526367188e-06))
    {
        vec3 _14089 = clamp(_16322.xyz / vec3(_18497.w), vec3(0.0), vec3(1.0));
        vec3 _14095 = pow((_14089 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _14098 = _14089 * vec3(0.077399380505084991455078125);
        bvec3 _14100 = lessThanEqual(_14089, vec3(0.040449999272823333740234375));
        vec3 _14075 = vec3(_14100.x ? _14098.x : _14095.x, _14100.y ? _14098.y : _14095.y, _14100.z ? _14098.z : _14095.z) * _18497.w;
        vec4 _16331 = _16322;
        _16331.x = _14075.x;
        _16331.y = _14075.y;
        _16331.z = _14075.z;
        _18498 = _16331;
    }
    else
    {
        _18498 = _16322;
    }
    _entryPointOutput = _18498;
}

