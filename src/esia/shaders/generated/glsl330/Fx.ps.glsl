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
const uint _1294 = (512u & kEsiaFxFeatures);
const vec3 _1758[6] = vec3[](vec3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), vec3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), vec3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), vec3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), vec3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), vec3(0.660000026226043701171875, 0.5, 1.0));
const uint _2394 = (512u & kEsiaFxFeatures);
const uint _2429 = (1024u & kEsiaFxFeatures);
const uint _2496 = (32u & kEsiaFxFeatures);
const uint _2504 = (4u & kEsiaFxFeatures);
const uint _2509 = (256u & kEsiaFxFeatures);
const uint _2574 = (8u & kEsiaFxFeatures);
const uint _2603 = (8192u & kEsiaFxFeatures);
const uint _2820 = (32u & kEsiaFxFeatures);
const uint _2884 = (1u & kEsiaFxFeatures);
const uint _2913 = (64u & kEsiaFxFeatures);
const uint _2968 = (16384u & kEsiaFxFeatures);
const vec4 _3132[4] = vec4[](vec4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), vec4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), vec4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), vec4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875));
const vec2 _3149[4] = vec2[](vec2(0.0), vec2(0.100000001490116119384765625, 1.2999999523162841796875), vec2(0.550000011920928955078125, 3.900000095367431640625), vec2(0.20000000298023223876953125, 5.19999980926513671875));
const uint _3401 = (4u & kEsiaFxFeatures);
const uint _3405 = (256u & kEsiaFxFeatures);
const uint _3465 = (16u & kEsiaFxFeatures);
const uint _3534 = (2u & kEsiaFxFeatures);
const uint _3664 = (128u & kEsiaFxFeatures);
const uint _3746 = (2048u & kEsiaFxFeatures);
const uint _3786 = (8u & kEsiaFxFeatures);
const uint _3790 = (4u & kEsiaFxFeatures);

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
    vec2 _5310 = gl_FragCoord.xy + _184.gConv.yy;
    vec2 _16732;
    if (_184.gConv.x > 0.5)
    {
        vec2 _15680 = _5310;
        _15680.y = _184.gTarget.y - _5310.y;
        _16732 = _15680;
    }
    else
    {
        _16732 = _5310;
    }
    float _4102 = dFdx(esia_v0.x);
    float _4106 = dFdy(esia_v0.x);
    float _4109 = max(abs(_4102) + abs(_4106), 9.9999997473787516355514526367188e-05);
    vec4 _16733;
    vec4 _16735;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.z == 2u) || ((esia_v7.x & _2394) != 0u))
    {
        uint _5329 = max(uint(_184.gConv.z), 1u);
        uint _5370 = max(uint(_184.gConv.z), 1u);
        _16735 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5329) * 24u) + 15u), int(esia_v1 / _5329), 0).xy, 0);
        _16733 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5370) * 24u) + 16u), int(esia_v1 / _5370), 0).xy, 0);
    }
    else
    {
        _16735 = vec4(0.0);
        _16733 = vec4(0.0);
    }
    vec2 _5443 = (esia_v2.xy + esia_v2.zw) * 0.5;
    vec2 _5452 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
    float _16753;
    SPIRV_CROSS_BRANCH
    if (esia_v7.z == 1u)
    {
        vec2 _5458 = esia_v0 - _5443;
        float _16752;
        do
        {
            if (esia_v5.w >= 6.282185077667236328125)
            {
                _16752 = abs(length(_5458) - esia_v3.x) - esia_v3.y;
                break;
            }
            float _5557 = esia_v5.z + (esia_v5.w * 0.5);
            float _5559 = cos(_5557);
            float _5561 = sin(_5557);
            float _5570 = dot(_5458, vec2(-_5561, _5559));
            float _5573 = dot(_5458, vec2(_5559, _5561));
            vec2 _5574 = vec2(_5570, _5573);
            float _5577 = abs(_5570);
            _5574.x = _5577;
            float _5580 = esia_v5.w * 0.5;
            float _5582 = sin(_5580);
            float _5584 = cos(_5580);
            _16752 = (((_5584 * _5577) > (_5582 * _5573)) ? length(_5574 - (vec2(_5582, _5584) * esia_v3.x)) : abs(length(_5574) - esia_v3.x)) - esia_v3.y;
            break;
        } while(false);
        _16753 = _16752;
    }
    else
    {
        float _16754;
        if (esia_v7.z == 2u)
        {
            vec2 _5620 = esia_v0 - _16735.xy;
            vec2 _5623 = _16735.zw - _16735.xy;
            _16754 = length(_5620 - (_5623 * clamp(dot(_5620, _5623) / max(dot(_5623, _5623), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
        }
        else
        {
            vec2 _5485 = esia_v0 - _5443;
            float _5676 = min(_5452.x, _5452.y);
            float _5679 = min((_5485.x > 0.0) ? ((_5485.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_5485.y > 0.0) ? esia_v3.w : esia_v3.x), _5676);
            float _5685 = _5679 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
            float _16737;
            float _16738;
            if (_5685 > _5676)
            {
                float _5699 = esia_v5.y * clamp((_5676 - _5679) / max(0.60000002384185791015625 * _5679, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _16738 = _5699;
                _16737 = _5679 * (1.0 + (0.60000002384185791015625 * _5699));
            }
            else
            {
                _16738 = esia_v5.y;
                _16737 = _5685;
            }
            vec2 _5712 = (abs(_5485) - _5452) + vec2(_16737);
            vec2 _5714 = max(_5712, vec2(0.0));
            float _16739;
            SPIRV_CROSS_BRANCH
            if ((_5714.x > 0.0) && (_5714.y > 0.0))
            {
                float _16740;
                if ((_16738 > 0.001000000047497451305389404296875) && (_16737 > 9.9999997473787516355514526367188e-05))
                {
                    float _5731 = 2.0 + (2.0 * _16738);
                    vec2 _5736 = _5714 / vec2(max(_16737, 9.9999997473787516355514526367188e-05));
                    _16740 = pow(pow(_5736.x, _5731) + pow(_5736.y, _5731), 1.0 / _5731) * _16737;
                }
                else
                {
                    _16740 = length(_5714);
                }
                _16739 = _16740;
            }
            else
            {
                _16739 = max(_5714.x, _5714.y);
            }
            float _5771 = (min(max(_5712.x, _5712.y), 0.0) + _16739) - _16737;
            float _16755;
            SPIRV_CROSS_BRANCH
            if ((esia_v7.x & _1294) != 0u)
            {
                vec2 _5513 = max((_16735.zw - _16735.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                vec2 _5516 = esia_v0 - ((_16735.xy + _16735.zw) * 0.5);
                float _5807 = min(_5513.x, _5513.y);
                float _5810 = min((_5516.x > 0.0) ? ((_5516.y > 0.0) ? _16733.x : _16733.x) : ((_5516.y > 0.0) ? _16733.x : _16733.x), _5807);
                float _5816 = _5810 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _16743;
                float _16744;
                if (_5816 > _5807)
                {
                    float _5830 = esia_v5.y * clamp((_5807 - _5810) / max(0.60000002384185791015625 * _5810, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _16744 = _5830;
                    _16743 = _5810 * (1.0 + (0.60000002384185791015625 * _5830));
                }
                else
                {
                    _16744 = esia_v5.y;
                    _16743 = _5816;
                }
                vec2 _5843 = (abs(_5516) - _5513) + vec2(_16743);
                vec2 _5845 = max(_5843, vec2(0.0));
                float _16745;
                SPIRV_CROSS_BRANCH
                if ((_5845.x > 0.0) && (_5845.y > 0.0))
                {
                    float _16746;
                    if ((_16744 > 0.001000000047497451305389404296875) && (_16743 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5862 = 2.0 + (2.0 * _16744);
                        vec2 _5867 = _5845 / vec2(max(_16743, 9.9999997473787516355514526367188e-05));
                        _16746 = pow(pow(_5867.x, _5862) + pow(_5867.y, _5862), 1.0 / _5862) * _16743;
                    }
                    else
                    {
                        _16746 = length(_5845);
                    }
                    _16745 = _16746;
                }
                else
                {
                    _16745 = max(_5845.x, _5845.y);
                }
                float _5902 = (min(max(_5843.x, _5843.y), 0.0) + _16745) - _16743;
                float _5907 = max(_16733.y, 9.9999997473787516355514526367188e-05);
                float _5916 = max(_5907 - abs(_5771 - _5902), 0.0) / _5907;
                _16755 = min(_5771, _5902) - (((_5916 * _5916) * _5907) * 0.25);
            }
            else
            {
                _16755 = _5771;
            }
            _16754 = _16755;
        }
        _16753 = _16754;
    }
    float _4134 = clamp(0.5 - (_16753 / _4109), 0.0, 1.0);
    float _18678;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _2429) != 0u)
    {
        uint _5933 = max(uint(_184.gConv.z), 1u);
        vec4 _5967 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5933) * 24u) + 19u), int(esia_v1 / _5933), 0).xy, 0);
        uint _5974 = max(uint(_184.gConv.z), 1u);
        vec4 _6008 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5974) * 24u) + 20u), int(esia_v1 / _5974), 0).xy, 0);
        vec2 _4163 = max((_5967.zw - _5967.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        vec2 _4166 = esia_v0 - ((_5967.xy + _5967.zw) * 0.5);
        float _4172 = _6008.y;
        float _6044 = min(_4163.x, _4163.y);
        float _6047 = min((_4166.x > 0.0) ? ((_4166.y > 0.0) ? _6008.x : _6008.x) : ((_4166.y > 0.0) ? _6008.x : _6008.x), _6044);
        float _6053 = _6047 * (1.0 + (0.60000002384185791015625 * _4172));
        float _16756;
        float _16757;
        if (_6053 > _6044)
        {
            float _6067 = _4172 * clamp((_6044 - _6047) / max(0.60000002384185791015625 * _6047, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _16757 = _6067;
            _16756 = _6047 * (1.0 + (0.60000002384185791015625 * _6067));
        }
        else
        {
            _16757 = _4172;
            _16756 = _6053;
        }
        vec2 _6080 = (abs(_4166) - _4163) + vec2(_16756);
        vec2 _6082 = max(_6080, vec2(0.0));
        float _16758;
        SPIRV_CROSS_BRANCH
        if ((_6082.x > 0.0) && (_6082.y > 0.0))
        {
            float _16759;
            if ((_16757 > 0.001000000047497451305389404296875) && (_16756 > 9.9999997473787516355514526367188e-05))
            {
                float _6099 = 2.0 + (2.0 * _16757);
                vec2 _6104 = _6082 / vec2(max(_16756, 9.9999997473787516355514526367188e-05));
                _16759 = pow(pow(_6104.x, _6099) + pow(_6104.y, _6099), 1.0 / _6099) * _16756;
            }
            else
            {
                _16759 = length(_6082);
            }
            _16758 = _16759;
        }
        else
        {
            _16758 = max(_6082.x, _6082.y);
        }
        float _4178 = clamp(0.5 - (((min(max(_6080.x, _6080.y), 0.0) + _16758) - _16756) / _4109), 0.0, 1.0);
        if (_4178 <= 0.0)
        {
            discard;
        }
        _18678 = _4178;
    }
    else
    {
        _18678 = 1.0;
    }
    bool _4189 = ((esia_v7.x & _2496) != 0u) && (_4134 >= 0.999000012874603271484375);
    vec4 _16848;
    SPIRV_CROSS_BRANCH
    if ((((esia_v7.x & _2504) != 0u) && (!((esia_v7.x & _2509) != 0u))) && (!_4189))
    {
        uint _6146 = max(uint(_184.gConv.z), 1u);
        vec4 _6180 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6146) * 24u) + 7u), int(esia_v1 / _6146), 0).xy, 0);
        uint _6187 = max(uint(_184.gConv.z), 1u);
        vec4 _6221 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6187) * 24u) + 8u), int(esia_v1 / _6187), 0).xy, 0);
        vec2 _4220 = esia_v0 - _6221.zw;
        vec2 _6260 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _6269 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _16806;
        SPIRV_CROSS_BRANCH
        if (esia_v7.z == 1u)
        {
            vec2 _6275 = _4220 - _6260;
            float _16805;
            do
            {
                if (esia_v5.w >= 6.282185077667236328125)
                {
                    _16805 = abs(length(_6275) - esia_v3.x) - esia_v3.y;
                    break;
                }
                float _6374 = esia_v5.z + (esia_v5.w * 0.5);
                float _6376 = cos(_6374);
                float _6378 = sin(_6374);
                float _6387 = dot(_6275, vec2(-_6378, _6376));
                float _6390 = dot(_6275, vec2(_6376, _6378));
                vec2 _6391 = vec2(_6387, _6390);
                float _6394 = abs(_6387);
                _6391.x = _6394;
                float _6397 = esia_v5.w * 0.5;
                float _6399 = sin(_6397);
                float _6401 = cos(_6397);
                _16805 = (((_6401 * _6394) > (_6399 * _6390)) ? length(_6391 - (vec2(_6399, _6401) * esia_v3.x)) : abs(length(_6391) - esia_v3.x)) - esia_v3.y;
                break;
            } while(false);
            _16806 = _16805;
        }
        else
        {
            float _16807;
            if (esia_v7.z == 2u)
            {
                vec2 _6437 = _4220 - _16735.xy;
                vec2 _6440 = _16735.zw - _16735.xy;
                _16807 = length(_6437 - (_6440 * clamp(dot(_6437, _6440) / max(dot(_6440, _6440), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
            }
            else
            {
                vec2 _6302 = _4220 - _6260;
                float _6493 = min(_6269.x, _6269.y);
                float _6496 = min((_6302.x > 0.0) ? ((_6302.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_6302.y > 0.0) ? esia_v3.w : esia_v3.x), _6493);
                float _6502 = _6496 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _16790;
                float _16791;
                if (_6502 > _6493)
                {
                    float _6516 = esia_v5.y * clamp((_6493 - _6496) / max(0.60000002384185791015625 * _6496, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _16791 = _6516;
                    _16790 = _6496 * (1.0 + (0.60000002384185791015625 * _6516));
                }
                else
                {
                    _16791 = esia_v5.y;
                    _16790 = _6502;
                }
                vec2 _6529 = (abs(_6302) - _6269) + vec2(_16790);
                vec2 _6531 = max(_6529, vec2(0.0));
                float _16792;
                SPIRV_CROSS_BRANCH
                if ((_6531.x > 0.0) && (_6531.y > 0.0))
                {
                    float _16793;
                    if ((_16791 > 0.001000000047497451305389404296875) && (_16790 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6548 = 2.0 + (2.0 * _16791);
                        vec2 _6553 = _6531 / vec2(max(_16790, 9.9999997473787516355514526367188e-05));
                        _16793 = pow(pow(_6553.x, _6548) + pow(_6553.y, _6548), 1.0 / _6548) * _16790;
                    }
                    else
                    {
                        _16793 = length(_6531);
                    }
                    _16792 = _16793;
                }
                else
                {
                    _16792 = max(_6531.x, _6531.y);
                }
                float _6588 = (min(max(_6529.x, _6529.y), 0.0) + _16792) - _16790;
                float _16808;
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & _1294) != 0u)
                {
                    vec2 _6330 = max((_16735.zw - _16735.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _6333 = _4220 - ((_16735.xy + _16735.zw) * 0.5);
                    float _6624 = min(_6330.x, _6330.y);
                    float _6627 = min((_6333.x > 0.0) ? ((_6333.y > 0.0) ? _16733.x : _16733.x) : ((_6333.y > 0.0) ? _16733.x : _16733.x), _6624);
                    float _6633 = _6627 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16796;
                    float _16797;
                    if (_6633 > _6624)
                    {
                        float _6647 = esia_v5.y * clamp((_6624 - _6627) / max(0.60000002384185791015625 * _6627, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16797 = _6647;
                        _16796 = _6627 * (1.0 + (0.60000002384185791015625 * _6647));
                    }
                    else
                    {
                        _16797 = esia_v5.y;
                        _16796 = _6633;
                    }
                    vec2 _6660 = (abs(_6333) - _6330) + vec2(_16796);
                    vec2 _6662 = max(_6660, vec2(0.0));
                    float _16798;
                    SPIRV_CROSS_BRANCH
                    if ((_6662.x > 0.0) && (_6662.y > 0.0))
                    {
                        float _16799;
                        if ((_16797 > 0.001000000047497451305389404296875) && (_16796 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6679 = 2.0 + (2.0 * _16797);
                            vec2 _6684 = _6662 / vec2(max(_16796, 9.9999997473787516355514526367188e-05));
                            _16799 = pow(pow(_6684.x, _6679) + pow(_6684.y, _6679), 1.0 / _6679) * _16796;
                        }
                        else
                        {
                            _16799 = length(_6662);
                        }
                        _16798 = _16799;
                    }
                    else
                    {
                        _16798 = max(_6662.x, _6662.y);
                    }
                    float _6719 = (min(max(_6660.x, _6660.y), 0.0) + _16798) - _16796;
                    float _6724 = max(_16733.y, 9.9999997473787516355514526367188e-05);
                    float _6733 = max(_6724 - abs(_6588 - _6719), 0.0) / _6724;
                    _16808 = min(_6588, _6719) - (((_6733 * _6733) * _6724) * 0.25);
                }
                else
                {
                    _16808 = _6588;
                }
                _16807 = _16808;
            }
            _16806 = _16807;
        }
        float _4229 = (_16806 - _6221.y) / (max(_6221.x * 0.5, _4109 * 0.5) * 1.41421353816986083984375);
        float _6750 = sign(_4229);
        float _6752 = abs(_4229);
        float _6763 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6752 * _6752))) * _6752)) * _6752);
        float _6766 = _6763 * _6763;
        float _6781 = clamp(_6180.w * (0.5 - (0.5 * (_6750 - (_6750 / (_6766 * _6766))))), 0.0, 1.0);
        _16848 = vec4(_6180.xyz * _6781, _6781);
    }
    else
    {
        _16848 = vec4(0.0);
    }
    vec4 _17739;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & _2574) != 0u) && (!_4189))
    {
        uint _6806 = max(uint(_184.gConv.z), 1u);
        vec4 _6840 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6806) * 24u) + 9u), int(esia_v1 / _6806), 0).xy, 0);
        uint _6847 = max(uint(_184.gConv.z), 1u);
        vec4 _6881 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6847) * 24u) + 10u), int(esia_v1 / _6847), 0).xy, 0);
        float _4257 = max(_6881.x, 0.001000000047497451305389404296875);
        float _16841;
        float _16844;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & _2603) != 0u)
        {
            uint _6888 = max(uint(_184.gConv.z), 1u);
            vec4 _6922 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6888) * 24u) + 22u), int(esia_v1 / _6888), 0).xy, 0);
            vec2 _4273 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _4282 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            vec2 _4289 = esia_v2.xy - _6922.xy;
            vec2 _4296 = _6922.zw - esia_v2.zw;
            vec2 _4326 = clamp(vec2((esia_v0.x < _4273.x) ? _4289.x : _4296.x, (esia_v0.y < _4273.y) ? _4289.y : _4296.y) * vec2(0.58823525905609130859375), vec2(min(_4257, 1.5)), vec2(_4257));
            float _4331 = min(_4282.x, _4282.y);
            float _16839;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 0u)
            {
                _16839 = min(((esia_v0.x > _4273.x) ? ((esia_v0.y > _4273.y) ? esia_v3.z : esia_v3.y) : ((esia_v0.y > _4273.y) ? esia_v3.w : esia_v3.x)) * (1.0 + (0.60000002384185791015625 * esia_v5.y)), _4331);
            }
            else
            {
                _16839 = _4331;
            }
            vec2 _4380 = max(abs(esia_v0 - _4273) - (_4282 - vec2(_16839)), vec2(0.0));
            float _4382 = length(_4380);
            vec2 _4388 = _4380 / vec2(_4382);
            bvec2 _4389 = bvec2(_4382 > 9.9999997473787516355514526367188e-05);
            vec2 _4390 = vec2(_4389.x ? _4388.x : vec2(0.707099974155426025390625).x, _4389.y ? _4388.y : vec2(0.707099974155426025390625).y);
            vec2 _4405 = esia_v0 - _6922.xy;
            vec2 _4410 = _6922.zw - esia_v0;
            _16844 = clamp(min(min(_4405.x, _4405.y), min(_4410.x, _4410.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _16841 = inversesqrt(dot(_4390 * _4390, vec2(1.0) / (_4326 * _4326)));
        }
        else
        {
            _16844 = 1.0;
            _16841 = _4257;
        }
        float _4428 = max(_16753, 0.0) / _16841;
        float _6930 = clamp(_6840.w * clamp((exp(((-_4428) * _4428) * 2.2000000476837158203125) * _6881.y) * _16844, 0.0, 1.0), 0.0, 1.0);
        _17739 = vec4(_6840.xyz * _6930, _6930) + (_16848 * (1.0 - _6930));
    }
    else
    {
        _17739 = _16848;
    }
    vec4 _18250;
    vec4 _18263;
    float _18644;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & _2820) != 0u) && (_4134 > 0.0))
    {
        uint _6955 = max(uint(_184.gConv.z), 1u);
        vec4 _6989 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6955) * 24u) + 11u), int(esia_v1 / _6955), 0).xy, 0);
        uint _6996 = max(uint(_184.gConv.z), 1u);
        vec4 _7030 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6996) * 24u) + 12u), int(esia_v1 / _6996), 0).xy, 0);
        uint _7037 = max(uint(_184.gConv.z), 1u);
        vec4 _7071 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _7037) * 24u) + 13u), int(esia_v1 / _7037), 0).xy, 0);
        uint _7078 = max(uint(_184.gConv.z), 1u);
        vec4 _7112 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _7078) * 24u) + 16u), int(esia_v1 / _7078), 0).xy, 0);
        float _7182 = _6989.x * _184.gDisplay.z;
        float _7184 = _6989.y;
        vec2 _7193 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(1.0));
        float _7201 = clamp(_6989.z, 0.001000000047497451305389404296875, min(_7193.x, _7193.y));
        float _7203 = _6989.w;
        float _7210 = clamp(1.0 - (max(-_16753, 0.0) / _7201), 0.0, 1.0);
        float _7216 = sqrt(clamp(1.0 - (_7210 * _7210), 0.0, 1.0));
        vec2 _16940;
        vec3 _17517;
        SPIRV_CROSS_BRANCH
        if (_7210 > 0.0)
        {
            vec2 _7623 = esia_v0 + vec2(0.5, 0.0);
            vec2 _7691 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _7700 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _16880;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _7706 = _7623 - _7691;
                float _16879;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _16879 = abs(length(_7706) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _7805 = esia_v5.z + (esia_v5.w * 0.5);
                    float _7807 = cos(_7805);
                    float _7809 = sin(_7805);
                    float _7818 = dot(_7706, vec2(-_7809, _7807));
                    float _7821 = dot(_7706, vec2(_7807, _7809));
                    vec2 _7822 = vec2(_7818, _7821);
                    float _7825 = abs(_7818);
                    _7822.x = _7825;
                    float _7828 = esia_v5.w * 0.5;
                    float _7830 = sin(_7828);
                    float _7832 = cos(_7828);
                    _16879 = (((_7832 * _7825) > (_7830 * _7821)) ? length(_7822 - (vec2(_7830, _7832) * esia_v3.x)) : abs(length(_7822) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _16880 = _16879;
            }
            else
            {
                float _16881;
                if (esia_v7.z == 2u)
                {
                    vec2 _7868 = _7623 - _16735.xy;
                    vec2 _7871 = _16735.zw - _16735.xy;
                    _16881 = length(_7868 - (_7871 * clamp(dot(_7868, _7871) / max(dot(_7871, _7871), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _7733 = _7623 - _7691;
                    float _7924 = min(_7700.x, _7700.y);
                    float _7927 = min((_7733.x > 0.0) ? ((_7733.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_7733.y > 0.0) ? esia_v3.w : esia_v3.x), _7924);
                    float _7933 = _7927 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16864;
                    float _16865;
                    if (_7933 > _7924)
                    {
                        float _7947 = esia_v5.y * clamp((_7924 - _7927) / max(0.60000002384185791015625 * _7927, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16865 = _7947;
                        _16864 = _7927 * (1.0 + (0.60000002384185791015625 * _7947));
                    }
                    else
                    {
                        _16865 = esia_v5.y;
                        _16864 = _7933;
                    }
                    vec2 _7960 = (abs(_7733) - _7700) + vec2(_16864);
                    vec2 _7962 = max(_7960, vec2(0.0));
                    float _16866;
                    SPIRV_CROSS_BRANCH
                    if ((_7962.x > 0.0) && (_7962.y > 0.0))
                    {
                        float _16867;
                        if ((_16865 > 0.001000000047497451305389404296875) && (_16864 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7979 = 2.0 + (2.0 * _16865);
                            vec2 _7984 = _7962 / vec2(max(_16864, 9.9999997473787516355514526367188e-05));
                            _16867 = pow(pow(_7984.x, _7979) + pow(_7984.y, _7979), 1.0 / _7979) * _16864;
                        }
                        else
                        {
                            _16867 = length(_7962);
                        }
                        _16866 = _16867;
                    }
                    else
                    {
                        _16866 = max(_7962.x, _7962.y);
                    }
                    float _8019 = (min(max(_7960.x, _7960.y), 0.0) + _16866) - _16864;
                    float _16882;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _1294) != 0u)
                    {
                        vec2 _7761 = max((_16735.zw - _16735.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _7764 = _7623 - ((_16735.xy + _16735.zw) * 0.5);
                        float _8055 = min(_7761.x, _7761.y);
                        float _8058 = min((_7764.x > 0.0) ? ((_7764.y > 0.0) ? _7112.x : _7112.x) : ((_7764.y > 0.0) ? _7112.x : _7112.x), _8055);
                        float _8064 = _8058 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _16870;
                        float _16871;
                        if (_8064 > _8055)
                        {
                            float _8078 = esia_v5.y * clamp((_8055 - _8058) / max(0.60000002384185791015625 * _8058, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16871 = _8078;
                            _16870 = _8058 * (1.0 + (0.60000002384185791015625 * _8078));
                        }
                        else
                        {
                            _16871 = esia_v5.y;
                            _16870 = _8064;
                        }
                        vec2 _8091 = (abs(_7764) - _7761) + vec2(_16870);
                        vec2 _8093 = max(_8091, vec2(0.0));
                        float _16872;
                        SPIRV_CROSS_BRANCH
                        if ((_8093.x > 0.0) && (_8093.y > 0.0))
                        {
                            float _16873;
                            if ((_16871 > 0.001000000047497451305389404296875) && (_16870 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8110 = 2.0 + (2.0 * _16871);
                                vec2 _8115 = _8093 / vec2(max(_16870, 9.9999997473787516355514526367188e-05));
                                _16873 = pow(pow(_8115.x, _8110) + pow(_8115.y, _8110), 1.0 / _8110) * _16870;
                            }
                            else
                            {
                                _16873 = length(_8093);
                            }
                            _16872 = _16873;
                        }
                        else
                        {
                            _16872 = max(_8093.x, _8093.y);
                        }
                        float _8150 = (min(max(_8091.x, _8091.y), 0.0) + _16872) - _16870;
                        float _8155 = max(_7112.y, 9.9999997473787516355514526367188e-05);
                        float _8164 = max(_8155 - abs(_8019 - _8150), 0.0) / _8155;
                        _16882 = min(_8019, _8150) - (((_8164 * _8164) * _8155) * 0.25);
                    }
                    else
                    {
                        _16882 = _8019;
                    }
                    _16881 = _16882;
                }
                _16880 = _16881;
            }
            vec2 _7627 = esia_v0 - vec2(0.5, 0.0);
            vec2 _8213 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _8222 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _16899;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _8228 = _7627 - _8213;
                float _16898;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _16898 = abs(length(_8228) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _8327 = esia_v5.z + (esia_v5.w * 0.5);
                    float _8329 = cos(_8327);
                    float _8331 = sin(_8327);
                    float _8340 = dot(_8228, vec2(-_8331, _8329));
                    float _8343 = dot(_8228, vec2(_8329, _8331));
                    vec2 _8344 = vec2(_8340, _8343);
                    float _8347 = abs(_8340);
                    _8344.x = _8347;
                    float _8350 = esia_v5.w * 0.5;
                    float _8352 = sin(_8350);
                    float _8354 = cos(_8350);
                    _16898 = (((_8354 * _8347) > (_8352 * _8343)) ? length(_8344 - (vec2(_8352, _8354) * esia_v3.x)) : abs(length(_8344) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _16899 = _16898;
            }
            else
            {
                float _16900;
                if (esia_v7.z == 2u)
                {
                    vec2 _8390 = _7627 - _16735.xy;
                    vec2 _8393 = _16735.zw - _16735.xy;
                    _16900 = length(_8390 - (_8393 * clamp(dot(_8390, _8393) / max(dot(_8393, _8393), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _8255 = _7627 - _8213;
                    float _8446 = min(_8222.x, _8222.y);
                    float _8449 = min((_8255.x > 0.0) ? ((_8255.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_8255.y > 0.0) ? esia_v3.w : esia_v3.x), _8446);
                    float _8455 = _8449 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16883;
                    float _16884;
                    if (_8455 > _8446)
                    {
                        float _8469 = esia_v5.y * clamp((_8446 - _8449) / max(0.60000002384185791015625 * _8449, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16884 = _8469;
                        _16883 = _8449 * (1.0 + (0.60000002384185791015625 * _8469));
                    }
                    else
                    {
                        _16884 = esia_v5.y;
                        _16883 = _8455;
                    }
                    vec2 _8482 = (abs(_8255) - _8222) + vec2(_16883);
                    vec2 _8484 = max(_8482, vec2(0.0));
                    float _16885;
                    SPIRV_CROSS_BRANCH
                    if ((_8484.x > 0.0) && (_8484.y > 0.0))
                    {
                        float _16886;
                        if ((_16884 > 0.001000000047497451305389404296875) && (_16883 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8501 = 2.0 + (2.0 * _16884);
                            vec2 _8506 = _8484 / vec2(max(_16883, 9.9999997473787516355514526367188e-05));
                            _16886 = pow(pow(_8506.x, _8501) + pow(_8506.y, _8501), 1.0 / _8501) * _16883;
                        }
                        else
                        {
                            _16886 = length(_8484);
                        }
                        _16885 = _16886;
                    }
                    else
                    {
                        _16885 = max(_8484.x, _8484.y);
                    }
                    float _8541 = (min(max(_8482.x, _8482.y), 0.0) + _16885) - _16883;
                    float _16901;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _1294) != 0u)
                    {
                        vec2 _8283 = max((_16735.zw - _16735.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _8286 = _7627 - ((_16735.xy + _16735.zw) * 0.5);
                        float _8577 = min(_8283.x, _8283.y);
                        float _8580 = min((_8286.x > 0.0) ? ((_8286.y > 0.0) ? _7112.x : _7112.x) : ((_8286.y > 0.0) ? _7112.x : _7112.x), _8577);
                        float _8586 = _8580 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _16889;
                        float _16890;
                        if (_8586 > _8577)
                        {
                            float _8600 = esia_v5.y * clamp((_8577 - _8580) / max(0.60000002384185791015625 * _8580, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16890 = _8600;
                            _16889 = _8580 * (1.0 + (0.60000002384185791015625 * _8600));
                        }
                        else
                        {
                            _16890 = esia_v5.y;
                            _16889 = _8586;
                        }
                        vec2 _8613 = (abs(_8286) - _8283) + vec2(_16889);
                        vec2 _8615 = max(_8613, vec2(0.0));
                        float _16891;
                        SPIRV_CROSS_BRANCH
                        if ((_8615.x > 0.0) && (_8615.y > 0.0))
                        {
                            float _16892;
                            if ((_16890 > 0.001000000047497451305389404296875) && (_16889 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8632 = 2.0 + (2.0 * _16890);
                                vec2 _8637 = _8615 / vec2(max(_16889, 9.9999997473787516355514526367188e-05));
                                _16892 = pow(pow(_8637.x, _8632) + pow(_8637.y, _8632), 1.0 / _8632) * _16889;
                            }
                            else
                            {
                                _16892 = length(_8615);
                            }
                            _16891 = _16892;
                        }
                        else
                        {
                            _16891 = max(_8615.x, _8615.y);
                        }
                        float _8672 = (min(max(_8613.x, _8613.y), 0.0) + _16891) - _16889;
                        float _8677 = max(_7112.y, 9.9999997473787516355514526367188e-05);
                        float _8686 = max(_8677 - abs(_8541 - _8672), 0.0) / _8677;
                        _16901 = min(_8541, _8672) - (((_8686 * _8686) * _8677) * 0.25);
                    }
                    else
                    {
                        _16901 = _8541;
                    }
                    _16900 = _16901;
                }
                _16899 = _16900;
            }
            vec2 _7632 = esia_v0 + vec2(0.0, 0.5);
            vec2 _8735 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _8744 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _16918;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _8750 = _7632 - _8735;
                float _16917;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _16917 = abs(length(_8750) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _8849 = esia_v5.z + (esia_v5.w * 0.5);
                    float _8851 = cos(_8849);
                    float _8853 = sin(_8849);
                    float _8862 = dot(_8750, vec2(-_8853, _8851));
                    float _8865 = dot(_8750, vec2(_8851, _8853));
                    vec2 _8866 = vec2(_8862, _8865);
                    float _8869 = abs(_8862);
                    _8866.x = _8869;
                    float _8872 = esia_v5.w * 0.5;
                    float _8874 = sin(_8872);
                    float _8876 = cos(_8872);
                    _16917 = (((_8876 * _8869) > (_8874 * _8865)) ? length(_8866 - (vec2(_8874, _8876) * esia_v3.x)) : abs(length(_8866) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _16918 = _16917;
            }
            else
            {
                float _16919;
                if (esia_v7.z == 2u)
                {
                    vec2 _8912 = _7632 - _16735.xy;
                    vec2 _8915 = _16735.zw - _16735.xy;
                    _16919 = length(_8912 - (_8915 * clamp(dot(_8912, _8915) / max(dot(_8915, _8915), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _8777 = _7632 - _8735;
                    float _8968 = min(_8744.x, _8744.y);
                    float _8971 = min((_8777.x > 0.0) ? ((_8777.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_8777.y > 0.0) ? esia_v3.w : esia_v3.x), _8968);
                    float _8977 = _8971 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16902;
                    float _16903;
                    if (_8977 > _8968)
                    {
                        float _8991 = esia_v5.y * clamp((_8968 - _8971) / max(0.60000002384185791015625 * _8971, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16903 = _8991;
                        _16902 = _8971 * (1.0 + (0.60000002384185791015625 * _8991));
                    }
                    else
                    {
                        _16903 = esia_v5.y;
                        _16902 = _8977;
                    }
                    vec2 _9004 = (abs(_8777) - _8744) + vec2(_16902);
                    vec2 _9006 = max(_9004, vec2(0.0));
                    float _16904;
                    SPIRV_CROSS_BRANCH
                    if ((_9006.x > 0.0) && (_9006.y > 0.0))
                    {
                        float _16905;
                        if ((_16903 > 0.001000000047497451305389404296875) && (_16902 > 9.9999997473787516355514526367188e-05))
                        {
                            float _9023 = 2.0 + (2.0 * _16903);
                            vec2 _9028 = _9006 / vec2(max(_16902, 9.9999997473787516355514526367188e-05));
                            _16905 = pow(pow(_9028.x, _9023) + pow(_9028.y, _9023), 1.0 / _9023) * _16902;
                        }
                        else
                        {
                            _16905 = length(_9006);
                        }
                        _16904 = _16905;
                    }
                    else
                    {
                        _16904 = max(_9006.x, _9006.y);
                    }
                    float _9063 = (min(max(_9004.x, _9004.y), 0.0) + _16904) - _16902;
                    float _16920;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _1294) != 0u)
                    {
                        vec2 _8805 = max((_16735.zw - _16735.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _8808 = _7632 - ((_16735.xy + _16735.zw) * 0.5);
                        float _9099 = min(_8805.x, _8805.y);
                        float _9102 = min((_8808.x > 0.0) ? ((_8808.y > 0.0) ? _7112.x : _7112.x) : ((_8808.y > 0.0) ? _7112.x : _7112.x), _9099);
                        float _9108 = _9102 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _16908;
                        float _16909;
                        if (_9108 > _9099)
                        {
                            float _9122 = esia_v5.y * clamp((_9099 - _9102) / max(0.60000002384185791015625 * _9102, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16909 = _9122;
                            _16908 = _9102 * (1.0 + (0.60000002384185791015625 * _9122));
                        }
                        else
                        {
                            _16909 = esia_v5.y;
                            _16908 = _9108;
                        }
                        vec2 _9135 = (abs(_8808) - _8805) + vec2(_16908);
                        vec2 _9137 = max(_9135, vec2(0.0));
                        float _16910;
                        SPIRV_CROSS_BRANCH
                        if ((_9137.x > 0.0) && (_9137.y > 0.0))
                        {
                            float _16911;
                            if ((_16909 > 0.001000000047497451305389404296875) && (_16908 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9154 = 2.0 + (2.0 * _16909);
                                vec2 _9159 = _9137 / vec2(max(_16908, 9.9999997473787516355514526367188e-05));
                                _16911 = pow(pow(_9159.x, _9154) + pow(_9159.y, _9154), 1.0 / _9154) * _16908;
                            }
                            else
                            {
                                _16911 = length(_9137);
                            }
                            _16910 = _16911;
                        }
                        else
                        {
                            _16910 = max(_9137.x, _9137.y);
                        }
                        float _9194 = (min(max(_9135.x, _9135.y), 0.0) + _16910) - _16908;
                        float _9199 = max(_7112.y, 9.9999997473787516355514526367188e-05);
                        float _9208 = max(_9199 - abs(_9063 - _9194), 0.0) / _9199;
                        _16920 = min(_9063, _9194) - (((_9208 * _9208) * _9199) * 0.25);
                    }
                    else
                    {
                        _16920 = _9063;
                    }
                    _16919 = _16920;
                }
                _16918 = _16919;
            }
            vec2 _7636 = esia_v0 - vec2(0.0, 0.5);
            vec2 _9257 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _9266 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _16937;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _9272 = _7636 - _9257;
                float _16936;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _16936 = abs(length(_9272) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _9371 = esia_v5.z + (esia_v5.w * 0.5);
                    float _9373 = cos(_9371);
                    float _9375 = sin(_9371);
                    float _9384 = dot(_9272, vec2(-_9375, _9373));
                    float _9387 = dot(_9272, vec2(_9373, _9375));
                    vec2 _9388 = vec2(_9384, _9387);
                    float _9391 = abs(_9384);
                    _9388.x = _9391;
                    float _9394 = esia_v5.w * 0.5;
                    float _9396 = sin(_9394);
                    float _9398 = cos(_9394);
                    _16936 = (((_9398 * _9391) > (_9396 * _9387)) ? length(_9388 - (vec2(_9396, _9398) * esia_v3.x)) : abs(length(_9388) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _16937 = _16936;
            }
            else
            {
                float _16938;
                if (esia_v7.z == 2u)
                {
                    vec2 _9434 = _7636 - _16735.xy;
                    vec2 _9437 = _16735.zw - _16735.xy;
                    _16938 = length(_9434 - (_9437 * clamp(dot(_9434, _9437) / max(dot(_9437, _9437), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _9299 = _7636 - _9257;
                    float _9490 = min(_9266.x, _9266.y);
                    float _9493 = min((_9299.x > 0.0) ? ((_9299.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_9299.y > 0.0) ? esia_v3.w : esia_v3.x), _9490);
                    float _9499 = _9493 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _16921;
                    float _16922;
                    if (_9499 > _9490)
                    {
                        float _9513 = esia_v5.y * clamp((_9490 - _9493) / max(0.60000002384185791015625 * _9493, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _16922 = _9513;
                        _16921 = _9493 * (1.0 + (0.60000002384185791015625 * _9513));
                    }
                    else
                    {
                        _16922 = esia_v5.y;
                        _16921 = _9499;
                    }
                    vec2 _9526 = (abs(_9299) - _9266) + vec2(_16921);
                    vec2 _9528 = max(_9526, vec2(0.0));
                    float _16923;
                    SPIRV_CROSS_BRANCH
                    if ((_9528.x > 0.0) && (_9528.y > 0.0))
                    {
                        float _16924;
                        if ((_16922 > 0.001000000047497451305389404296875) && (_16921 > 9.9999997473787516355514526367188e-05))
                        {
                            float _9545 = 2.0 + (2.0 * _16922);
                            vec2 _9550 = _9528 / vec2(max(_16921, 9.9999997473787516355514526367188e-05));
                            _16924 = pow(pow(_9550.x, _9545) + pow(_9550.y, _9545), 1.0 / _9545) * _16921;
                        }
                        else
                        {
                            _16924 = length(_9528);
                        }
                        _16923 = _16924;
                    }
                    else
                    {
                        _16923 = max(_9528.x, _9528.y);
                    }
                    float _9585 = (min(max(_9526.x, _9526.y), 0.0) + _16923) - _16921;
                    float _16939;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & _1294) != 0u)
                    {
                        vec2 _9327 = max((_16735.zw - _16735.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _9330 = _7636 - ((_16735.xy + _16735.zw) * 0.5);
                        float _9621 = min(_9327.x, _9327.y);
                        float _9624 = min((_9330.x > 0.0) ? ((_9330.y > 0.0) ? _7112.x : _7112.x) : ((_9330.y > 0.0) ? _7112.x : _7112.x), _9621);
                        float _9630 = _9624 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _16927;
                        float _16928;
                        if (_9630 > _9621)
                        {
                            float _9644 = esia_v5.y * clamp((_9621 - _9624) / max(0.60000002384185791015625 * _9624, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _16928 = _9644;
                            _16927 = _9624 * (1.0 + (0.60000002384185791015625 * _9644));
                        }
                        else
                        {
                            _16928 = esia_v5.y;
                            _16927 = _9630;
                        }
                        vec2 _9657 = (abs(_9330) - _9327) + vec2(_16927);
                        vec2 _9659 = max(_9657, vec2(0.0));
                        float _16929;
                        SPIRV_CROSS_BRANCH
                        if ((_9659.x > 0.0) && (_9659.y > 0.0))
                        {
                            float _16930;
                            if ((_16928 > 0.001000000047497451305389404296875) && (_16927 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9676 = 2.0 + (2.0 * _16928);
                                vec2 _9681 = _9659 / vec2(max(_16927, 9.9999997473787516355514526367188e-05));
                                _16930 = pow(pow(_9681.x, _9676) + pow(_9681.y, _9676), 1.0 / _9676) * _16927;
                            }
                            else
                            {
                                _16930 = length(_9659);
                            }
                            _16929 = _16930;
                        }
                        else
                        {
                            _16929 = max(_9659.x, _9659.y);
                        }
                        float _9716 = (min(max(_9657.x, _9657.y), 0.0) + _16929) - _16927;
                        float _9721 = max(_7112.y, 9.9999997473787516355514526367188e-05);
                        float _9730 = max(_9721 - abs(_9585 - _9716), 0.0) / _9721;
                        _16939 = min(_9585, _9716) - (((_9730 * _9730) * _9721) * 0.25);
                    }
                    else
                    {
                        _16939 = _9585;
                    }
                    _16938 = _16939;
                }
                _16937 = _16938;
            }
            vec2 _7642 = vec2(_16880 - _16899, _16918 - _16937);
            float _7644 = length(_7642);
            vec2 _7650 = _7642 / vec2(_7644);
            bvec2 _7651 = bvec2(_7644 > 9.9999997473787516355514526367188e-06);
            vec2 _7652 = vec2(_7651.x ? _7650.x : vec2(0.0, -1.0).x, _7651.y ? _7650.y : vec2(0.0, -1.0).y);
            _17517 = normalize(vec3(_7652 * min(_7210 / max(_7216, 0.001000000047497451305389404296875), 8.0), 1.0));
            _16940 = _7652;
        }
        else
        {
            _17517 = vec3(0.0, 0.0, 1.0);
            _16940 = vec2(0.0, -1.0);
        }
        vec2 _7242 = ((-_16940) * _7184) * (1.0 - _7216);
        float _7244 = _7112.z;
        vec2 _16941;
        SPIRV_CROSS_BRANCH
        if (_7244 > 0.0)
        {
            _16941 = (((esia_v2.xy + esia_v2.zw) * 0.5) - esia_v0) * (_7244 / (1.0 + _7244));
        }
        else
        {
            _16941 = vec2(0.0);
        }
        vec2 _7268 = _16732 * _184.gTarget.zw;
        vec2 _7275 = _184.gDisplay.zw * _184.gTarget.zw;
        vec2 _7280 = (_7242 + _16941) * _7275;
        vec2 _7283 = _7242 * _7275;
        vec3 _17399;
        float _17417;
        vec3 _17620;
        if (_184.gTime.z > 0.5)
        {
            vec2 _7290 = _7268 + _7280;
            float _9755 = clamp(log2(max(_7182, 1.0)) - 1.0, 0.0, 5.0);
            int _9758 = int(floor(_9755));
            float _9762 = _9755 - float(_9758);
            bool _9767 = (_9762 > 0.0199999995529651641845703125) && (_9758 < 5);
            vec3 _17012;
            _17012 = vec3(0.0);
            vec3 _9796;
            SPIRV_CROSS_LOOP
            for (int _16942 = 0; _16942 < 2; _17012 = _9796, _16942++)
            {
                if ((_16942 > 0) && (!_9767))
                {
                    break;
                }
                int _9782 = _9758 + _16942;
                int _9820 = clamp(_9782, 1, 5);
                vec2 _9889 = (_7290 * _184.gLevel[_9820].xy) - vec2(0.5);
                vec2 _9891 = floor(_9889);
                vec2 _9894 = _9889 - _9891;
                vec2 _9897 = _9894 * _9894;
                vec2 _9900 = _9897 * _9894;
                vec2 _9919 = (((_9900 * 3.0) - (_9897 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                vec2 _9932 = _9900 * 0.16666667163372039794921875;
                vec2 _9935 = (((((-_9900) + (_9897 * 3.0)) - (_9894 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9919;
                vec2 _9939 = (((((_9900 * (-3.0)) + (_9897 * 3.0)) + (_9894 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9932;
                vec2 _9951 = ((_9891 - vec2(0.5)) + (_9919 / _9935)) * _184.gLevel[_9820].zw;
                vec2 _9963 = ((_9891 + vec2(1.5)) + (_9932 / _9939)) * _184.gLevel[_9820].zw;
                vec4 _16979;
                SPIRV_CROSS_BRANCH
                if (_9782 <= 0)
                {
                    vec2 _16978;
                    if (_184.gConv.x > 0.5)
                    {
                        vec2 _16063 = _7290;
                        _16063.y = 1.0 - _7290.y;
                        _16978 = _16063;
                    }
                    else
                    {
                        _16978 = _7290;
                    }
                    _16979 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _16978, 0.0);
                }
                else
                {
                    vec4 _16980;
                    if (_9782 == 1)
                    {
                        vec2 _16971;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _16068 = _9951;
                            _16068.y = 1.0 - _9951.y;
                            _16971 = _16068;
                        }
                        else
                        {
                            _16971 = _9951;
                        }
                        float _10008 = _9951.y;
                        vec2 _10009 = vec2(_9963.x, _10008);
                        vec2 _16972;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _16075 = _10009;
                            _16075.y = 1.0 - _10008;
                            _16972 = _16075;
                        }
                        else
                        {
                            _16972 = _10009;
                        }
                        float _10026 = _9963.y;
                        vec2 _10027 = vec2(_9951.x, _10026);
                        vec2 _16974;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _16082 = _10027;
                            _16082.y = 1.0 - _10026;
                            _16974 = _16082;
                        }
                        else
                        {
                            _16974 = _10027;
                        }
                        vec2 _16976;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _16089 = _9963;
                            _16089.y = 1.0 - _9963.y;
                            _16976 = _16089;
                        }
                        else
                        {
                            _16976 = _9963;
                        }
                        _16980 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16971, 0.0) * (_9935.x * _9935.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16972, 0.0) * (_9939.x * _9935.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16974, 0.0) * (_9935.x * _9939.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _16976, 0.0) * (_9939.x * _9939.y));
                    }
                    else
                    {
                        vec4 _16981;
                        if (_9782 == 2)
                        {
                            vec2 _16964;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16096 = _9951;
                                _16096.y = 1.0 - _9951.y;
                                _16964 = _16096;
                            }
                            else
                            {
                                _16964 = _9951;
                            }
                            float _10138 = _9951.y;
                            vec2 _10139 = vec2(_9963.x, _10138);
                            vec2 _16965;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16103 = _10139;
                                _16103.y = 1.0 - _10138;
                                _16965 = _16103;
                            }
                            else
                            {
                                _16965 = _10139;
                            }
                            float _10156 = _9963.y;
                            vec2 _10157 = vec2(_9951.x, _10156);
                            vec2 _16967;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16110 = _10157;
                                _16110.y = 1.0 - _10156;
                                _16967 = _16110;
                            }
                            else
                            {
                                _16967 = _10157;
                            }
                            vec2 _16969;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16117 = _9963;
                                _16117.y = 1.0 - _9963.y;
                                _16969 = _16117;
                            }
                            else
                            {
                                _16969 = _9963;
                            }
                            _16981 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16964, 0.0) * (_9935.x * _9935.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16965, 0.0) * (_9939.x * _9935.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16967, 0.0) * (_9935.x * _9939.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _16969, 0.0) * (_9939.x * _9939.y));
                        }
                        else
                        {
                            vec4 _16982;
                            if (_9782 == 3)
                            {
                                vec2 _16957;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16124 = _9951;
                                    _16124.y = 1.0 - _9951.y;
                                    _16957 = _16124;
                                }
                                else
                                {
                                    _16957 = _9951;
                                }
                                float _10268 = _9951.y;
                                vec2 _10269 = vec2(_9963.x, _10268);
                                vec2 _16958;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16131 = _10269;
                                    _16131.y = 1.0 - _10268;
                                    _16958 = _16131;
                                }
                                else
                                {
                                    _16958 = _10269;
                                }
                                float _10286 = _9963.y;
                                vec2 _10287 = vec2(_9951.x, _10286);
                                vec2 _16960;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16138 = _10287;
                                    _16138.y = 1.0 - _10286;
                                    _16960 = _16138;
                                }
                                else
                                {
                                    _16960 = _10287;
                                }
                                vec2 _16962;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16145 = _9963;
                                    _16145.y = 1.0 - _9963.y;
                                    _16962 = _16145;
                                }
                                else
                                {
                                    _16962 = _9963;
                                }
                                _16982 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16957, 0.0) * (_9935.x * _9935.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16958, 0.0) * (_9939.x * _9935.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16960, 0.0) * (_9935.x * _9939.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _16962, 0.0) * (_9939.x * _9939.y));
                            }
                            else
                            {
                                vec4 _16983;
                                if (_9782 == 4)
                                {
                                    vec2 _16950;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16152 = _9951;
                                        _16152.y = 1.0 - _9951.y;
                                        _16950 = _16152;
                                    }
                                    else
                                    {
                                        _16950 = _9951;
                                    }
                                    float _10398 = _9951.y;
                                    vec2 _10399 = vec2(_9963.x, _10398);
                                    vec2 _16951;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16159 = _10399;
                                        _16159.y = 1.0 - _10398;
                                        _16951 = _16159;
                                    }
                                    else
                                    {
                                        _16951 = _10399;
                                    }
                                    float _10416 = _9963.y;
                                    vec2 _10417 = vec2(_9951.x, _10416);
                                    vec2 _16953;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16166 = _10417;
                                        _16166.y = 1.0 - _10416;
                                        _16953 = _16166;
                                    }
                                    else
                                    {
                                        _16953 = _10417;
                                    }
                                    vec2 _16955;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16173 = _9963;
                                        _16173.y = 1.0 - _9963.y;
                                        _16955 = _16173;
                                    }
                                    else
                                    {
                                        _16955 = _9963;
                                    }
                                    _16983 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16950, 0.0) * (_9935.x * _9935.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16951, 0.0) * (_9939.x * _9935.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16953, 0.0) * (_9935.x * _9939.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _16955, 0.0) * (_9939.x * _9939.y));
                                }
                                else
                                {
                                    vec2 _16943;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16180 = _9951;
                                        _16180.y = 1.0 - _9951.y;
                                        _16943 = _16180;
                                    }
                                    else
                                    {
                                        _16943 = _9951;
                                    }
                                    float _10528 = _9951.y;
                                    vec2 _10529 = vec2(_9963.x, _10528);
                                    vec2 _16944;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16187 = _10529;
                                        _16187.y = 1.0 - _10528;
                                        _16944 = _16187;
                                    }
                                    else
                                    {
                                        _16944 = _10529;
                                    }
                                    float _10546 = _9963.y;
                                    vec2 _10547 = vec2(_9951.x, _10546);
                                    vec2 _16946;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16194 = _10547;
                                        _16194.y = 1.0 - _10546;
                                        _16946 = _16194;
                                    }
                                    else
                                    {
                                        _16946 = _10547;
                                    }
                                    vec2 _16948;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16201 = _9963;
                                        _16201.y = 1.0 - _9963.y;
                                        _16948 = _16201;
                                    }
                                    else
                                    {
                                        _16948 = _9963;
                                    }
                                    _16983 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16943, 0.0) * (_9935.x * _9935.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16944, 0.0) * (_9939.x * _9935.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16946, 0.0) * (_9935.x * _9939.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _16948, 0.0) * (_9939.x * _9939.y));
                                }
                                _16982 = _16983;
                            }
                            _16981 = _16982;
                        }
                        _16980 = _16981;
                    }
                    _16979 = _16980;
                }
                _9796 = _17012 + (_16979.xyz * (_9767 ? ((_16942 == 0) ? (1.0 - _9762) : _9762) : 1.0));
            }
            vec3 _17402;
            SPIRV_CROSS_BRANCH
            if (((_7203 > 0.001000000047497451305389404296875) && (_7210 > 0.0)) && ((((_7184 * 0.300000011920928955078125) * _7203) * _184.gDisplay.z) > (_7182 * 0.3499999940395355224609375)))
            {
                float _7310 = 0.300000011920928955078125 * _7203;
                vec2 _7317 = (_7268 + _7280) - (_7283 * _7310);
                float _10643 = clamp(log2(max(_7182, 1.0)) - 1.0, 0.0, 5.0);
                int _10646 = int(floor(_10643));
                float _10650 = _10643 - float(_10646);
                bool _10655 = (_10650 > 0.0199999995529651641845703125) && (_10646 < 5);
                vec3 _17108;
                _17108 = vec3(0.0);
                vec3 _10684;
                SPIRV_CROSS_LOOP
                for (int _17038 = 0; _17038 < 2; _17108 = _10684, _17038++)
                {
                    if ((_17038 > 0) && (!_10655))
                    {
                        break;
                    }
                    int _10670 = _10646 + _17038;
                    int _10708 = clamp(_10670, 1, 5);
                    vec2 _10777 = (_7317 * _184.gLevel[_10708].xy) - vec2(0.5);
                    vec2 _10779 = floor(_10777);
                    vec2 _10782 = _10777 - _10779;
                    vec2 _10785 = _10782 * _10782;
                    vec2 _10788 = _10785 * _10782;
                    vec2 _10807 = (((_10788 * 3.0) - (_10785 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                    vec2 _10820 = _10788 * 0.16666667163372039794921875;
                    vec2 _10823 = (((((-_10788) + (_10785 * 3.0)) - (_10782 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10807;
                    vec2 _10827 = (((((_10788 * (-3.0)) + (_10785 * 3.0)) + (_10782 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10820;
                    vec2 _10839 = ((_10779 - vec2(0.5)) + (_10807 / _10823)) * _184.gLevel[_10708].zw;
                    vec2 _10851 = ((_10779 + vec2(1.5)) + (_10820 / _10827)) * _184.gLevel[_10708].zw;
                    vec4 _17075;
                    SPIRV_CROSS_BRANCH
                    if (_10670 <= 0)
                    {
                        vec2 _17074;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _16206 = _7317;
                            _16206.y = 1.0 - _7317.y;
                            _17074 = _16206;
                        }
                        else
                        {
                            _17074 = _7317;
                        }
                        _17075 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _17074, 0.0);
                    }
                    else
                    {
                        vec4 _17076;
                        if (_10670 == 1)
                        {
                            vec2 _17067;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16211 = _10839;
                                _16211.y = 1.0 - _10839.y;
                                _17067 = _16211;
                            }
                            else
                            {
                                _17067 = _10839;
                            }
                            float _10896 = _10839.y;
                            vec2 _10897 = vec2(_10851.x, _10896);
                            vec2 _17068;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16218 = _10897;
                                _16218.y = 1.0 - _10896;
                                _17068 = _16218;
                            }
                            else
                            {
                                _17068 = _10897;
                            }
                            float _10914 = _10851.y;
                            vec2 _10915 = vec2(_10839.x, _10914);
                            vec2 _17070;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16225 = _10915;
                                _16225.y = 1.0 - _10914;
                                _17070 = _16225;
                            }
                            else
                            {
                                _17070 = _10915;
                            }
                            vec2 _17072;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16232 = _10851;
                                _16232.y = 1.0 - _10851.y;
                                _17072 = _16232;
                            }
                            else
                            {
                                _17072 = _10851;
                            }
                            _17076 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17067, 0.0) * (_10823.x * _10823.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17068, 0.0) * (_10827.x * _10823.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17070, 0.0) * (_10823.x * _10827.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17072, 0.0) * (_10827.x * _10827.y));
                        }
                        else
                        {
                            vec4 _17077;
                            if (_10670 == 2)
                            {
                                vec2 _17060;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16239 = _10839;
                                    _16239.y = 1.0 - _10839.y;
                                    _17060 = _16239;
                                }
                                else
                                {
                                    _17060 = _10839;
                                }
                                float _11026 = _10839.y;
                                vec2 _11027 = vec2(_10851.x, _11026);
                                vec2 _17061;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16246 = _11027;
                                    _16246.y = 1.0 - _11026;
                                    _17061 = _16246;
                                }
                                else
                                {
                                    _17061 = _11027;
                                }
                                float _11044 = _10851.y;
                                vec2 _11045 = vec2(_10839.x, _11044);
                                vec2 _17063;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16253 = _11045;
                                    _16253.y = 1.0 - _11044;
                                    _17063 = _16253;
                                }
                                else
                                {
                                    _17063 = _11045;
                                }
                                vec2 _17065;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16260 = _10851;
                                    _16260.y = 1.0 - _10851.y;
                                    _17065 = _16260;
                                }
                                else
                                {
                                    _17065 = _10851;
                                }
                                _17077 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17060, 0.0) * (_10823.x * _10823.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17061, 0.0) * (_10827.x * _10823.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17063, 0.0) * (_10823.x * _10827.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17065, 0.0) * (_10827.x * _10827.y));
                            }
                            else
                            {
                                vec4 _17078;
                                if (_10670 == 3)
                                {
                                    vec2 _17053;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16267 = _10839;
                                        _16267.y = 1.0 - _10839.y;
                                        _17053 = _16267;
                                    }
                                    else
                                    {
                                        _17053 = _10839;
                                    }
                                    float _11156 = _10839.y;
                                    vec2 _11157 = vec2(_10851.x, _11156);
                                    vec2 _17054;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16274 = _11157;
                                        _16274.y = 1.0 - _11156;
                                        _17054 = _16274;
                                    }
                                    else
                                    {
                                        _17054 = _11157;
                                    }
                                    float _11174 = _10851.y;
                                    vec2 _11175 = vec2(_10839.x, _11174);
                                    vec2 _17056;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16281 = _11175;
                                        _16281.y = 1.0 - _11174;
                                        _17056 = _16281;
                                    }
                                    else
                                    {
                                        _17056 = _11175;
                                    }
                                    vec2 _17058;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16288 = _10851;
                                        _16288.y = 1.0 - _10851.y;
                                        _17058 = _16288;
                                    }
                                    else
                                    {
                                        _17058 = _10851;
                                    }
                                    _17078 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17053, 0.0) * (_10823.x * _10823.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17054, 0.0) * (_10827.x * _10823.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17056, 0.0) * (_10823.x * _10827.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17058, 0.0) * (_10827.x * _10827.y));
                                }
                                else
                                {
                                    vec4 _17079;
                                    if (_10670 == 4)
                                    {
                                        vec2 _17046;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16295 = _10839;
                                            _16295.y = 1.0 - _10839.y;
                                            _17046 = _16295;
                                        }
                                        else
                                        {
                                            _17046 = _10839;
                                        }
                                        float _11286 = _10839.y;
                                        vec2 _11287 = vec2(_10851.x, _11286);
                                        vec2 _17047;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16302 = _11287;
                                            _16302.y = 1.0 - _11286;
                                            _17047 = _16302;
                                        }
                                        else
                                        {
                                            _17047 = _11287;
                                        }
                                        float _11304 = _10851.y;
                                        vec2 _11305 = vec2(_10839.x, _11304);
                                        vec2 _17049;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16309 = _11305;
                                            _16309.y = 1.0 - _11304;
                                            _17049 = _16309;
                                        }
                                        else
                                        {
                                            _17049 = _11305;
                                        }
                                        vec2 _17051;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16316 = _10851;
                                            _16316.y = 1.0 - _10851.y;
                                            _17051 = _16316;
                                        }
                                        else
                                        {
                                            _17051 = _10851;
                                        }
                                        _17079 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17046, 0.0) * (_10823.x * _10823.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17047, 0.0) * (_10827.x * _10823.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17049, 0.0) * (_10823.x * _10827.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17051, 0.0) * (_10827.x * _10827.y));
                                    }
                                    else
                                    {
                                        vec2 _17039;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16323 = _10839;
                                            _16323.y = 1.0 - _10839.y;
                                            _17039 = _16323;
                                        }
                                        else
                                        {
                                            _17039 = _10839;
                                        }
                                        float _11416 = _10839.y;
                                        vec2 _11417 = vec2(_10851.x, _11416);
                                        vec2 _17040;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16330 = _11417;
                                            _16330.y = 1.0 - _11416;
                                            _17040 = _16330;
                                        }
                                        else
                                        {
                                            _17040 = _11417;
                                        }
                                        float _11434 = _10851.y;
                                        vec2 _11435 = vec2(_10839.x, _11434);
                                        vec2 _17042;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16337 = _11435;
                                            _16337.y = 1.0 - _11434;
                                            _17042 = _16337;
                                        }
                                        else
                                        {
                                            _17042 = _11435;
                                        }
                                        vec2 _17044;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16344 = _10851;
                                            _16344.y = 1.0 - _10851.y;
                                            _17044 = _16344;
                                        }
                                        else
                                        {
                                            _17044 = _10851;
                                        }
                                        _17079 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17039, 0.0) * (_10823.x * _10823.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17040, 0.0) * (_10827.x * _10823.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17042, 0.0) * (_10823.x * _10827.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17044, 0.0) * (_10827.x * _10827.y));
                                    }
                                    _17078 = _17079;
                                }
                                _17077 = _17078;
                            }
                            _17076 = _17077;
                        }
                        _17075 = _17076;
                    }
                    _10684 = _17108 + (_17075.xyz * (_10655 ? ((_17038 == 0) ? (1.0 - _10650) : _10650) : 1.0));
                }
                vec3 _16348 = _17012;
                _16348.x = _17108.x;
                vec2 _7328 = (_7268 + _7280) + (_7283 * _7310);
                float _11531 = clamp(log2(max(_7182, 1.0)) - 1.0, 0.0, 5.0);
                int _11534 = int(floor(_11531));
                float _11538 = _11531 - float(_11534);
                bool _11543 = (_11538 > 0.0199999995529651641845703125) && (_11534 < 5);
                vec3 _17232;
                _17232 = vec3(0.0);
                vec3 _11572;
                SPIRV_CROSS_LOOP
                for (int _17162 = 0; _17162 < 2; _17232 = _11572, _17162++)
                {
                    if ((_17162 > 0) && (!_11543))
                    {
                        break;
                    }
                    int _11558 = _11534 + _17162;
                    int _11596 = clamp(_11558, 1, 5);
                    vec2 _11665 = (_7328 * _184.gLevel[_11596].xy) - vec2(0.5);
                    vec2 _11667 = floor(_11665);
                    vec2 _11670 = _11665 - _11667;
                    vec2 _11673 = _11670 * _11670;
                    vec2 _11676 = _11673 * _11670;
                    vec2 _11695 = (((_11676 * 3.0) - (_11673 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                    vec2 _11708 = _11676 * 0.16666667163372039794921875;
                    vec2 _11711 = (((((-_11676) + (_11673 * 3.0)) - (_11670 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11695;
                    vec2 _11715 = (((((_11676 * (-3.0)) + (_11673 * 3.0)) + (_11670 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11708;
                    vec2 _11727 = ((_11667 - vec2(0.5)) + (_11695 / _11711)) * _184.gLevel[_11596].zw;
                    vec2 _11739 = ((_11667 + vec2(1.5)) + (_11708 / _11715)) * _184.gLevel[_11596].zw;
                    vec4 _17199;
                    SPIRV_CROSS_BRANCH
                    if (_11558 <= 0)
                    {
                        vec2 _17198;
                        if (_184.gConv.x > 0.5)
                        {
                            vec2 _16351 = _7328;
                            _16351.y = 1.0 - _7328.y;
                            _17198 = _16351;
                        }
                        else
                        {
                            _17198 = _7328;
                        }
                        _17199 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _17198, 0.0);
                    }
                    else
                    {
                        vec4 _17200;
                        if (_11558 == 1)
                        {
                            vec2 _17191;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16356 = _11727;
                                _16356.y = 1.0 - _11727.y;
                                _17191 = _16356;
                            }
                            else
                            {
                                _17191 = _11727;
                            }
                            float _11784 = _11727.y;
                            vec2 _11785 = vec2(_11739.x, _11784);
                            vec2 _17192;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16363 = _11785;
                                _16363.y = 1.0 - _11784;
                                _17192 = _16363;
                            }
                            else
                            {
                                _17192 = _11785;
                            }
                            float _11802 = _11739.y;
                            vec2 _11803 = vec2(_11727.x, _11802);
                            vec2 _17194;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16370 = _11803;
                                _16370.y = 1.0 - _11802;
                                _17194 = _16370;
                            }
                            else
                            {
                                _17194 = _11803;
                            }
                            vec2 _17196;
                            if (_184.gConv.x > 0.5)
                            {
                                vec2 _16377 = _11739;
                                _16377.y = 1.0 - _11739.y;
                                _17196 = _16377;
                            }
                            else
                            {
                                _17196 = _11739;
                            }
                            _17200 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17191, 0.0) * (_11711.x * _11711.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17192, 0.0) * (_11715.x * _11711.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17194, 0.0) * (_11711.x * _11715.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17196, 0.0) * (_11715.x * _11715.y));
                        }
                        else
                        {
                            vec4 _17201;
                            if (_11558 == 2)
                            {
                                vec2 _17184;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16384 = _11727;
                                    _16384.y = 1.0 - _11727.y;
                                    _17184 = _16384;
                                }
                                else
                                {
                                    _17184 = _11727;
                                }
                                float _11914 = _11727.y;
                                vec2 _11915 = vec2(_11739.x, _11914);
                                vec2 _17185;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16391 = _11915;
                                    _16391.y = 1.0 - _11914;
                                    _17185 = _16391;
                                }
                                else
                                {
                                    _17185 = _11915;
                                }
                                float _11932 = _11739.y;
                                vec2 _11933 = vec2(_11727.x, _11932);
                                vec2 _17187;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16398 = _11933;
                                    _16398.y = 1.0 - _11932;
                                    _17187 = _16398;
                                }
                                else
                                {
                                    _17187 = _11933;
                                }
                                vec2 _17189;
                                if (_184.gConv.x > 0.5)
                                {
                                    vec2 _16405 = _11739;
                                    _16405.y = 1.0 - _11739.y;
                                    _17189 = _16405;
                                }
                                else
                                {
                                    _17189 = _11739;
                                }
                                _17201 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17184, 0.0) * (_11711.x * _11711.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17185, 0.0) * (_11715.x * _11711.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17187, 0.0) * (_11711.x * _11715.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17189, 0.0) * (_11715.x * _11715.y));
                            }
                            else
                            {
                                vec4 _17202;
                                if (_11558 == 3)
                                {
                                    vec2 _17177;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16412 = _11727;
                                        _16412.y = 1.0 - _11727.y;
                                        _17177 = _16412;
                                    }
                                    else
                                    {
                                        _17177 = _11727;
                                    }
                                    float _12044 = _11727.y;
                                    vec2 _12045 = vec2(_11739.x, _12044);
                                    vec2 _17178;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16419 = _12045;
                                        _16419.y = 1.0 - _12044;
                                        _17178 = _16419;
                                    }
                                    else
                                    {
                                        _17178 = _12045;
                                    }
                                    float _12062 = _11739.y;
                                    vec2 _12063 = vec2(_11727.x, _12062);
                                    vec2 _17180;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16426 = _12063;
                                        _16426.y = 1.0 - _12062;
                                        _17180 = _16426;
                                    }
                                    else
                                    {
                                        _17180 = _12063;
                                    }
                                    vec2 _17182;
                                    if (_184.gConv.x > 0.5)
                                    {
                                        vec2 _16433 = _11739;
                                        _16433.y = 1.0 - _11739.y;
                                        _17182 = _16433;
                                    }
                                    else
                                    {
                                        _17182 = _11739;
                                    }
                                    _17202 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17177, 0.0) * (_11711.x * _11711.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17178, 0.0) * (_11715.x * _11711.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17180, 0.0) * (_11711.x * _11715.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17182, 0.0) * (_11715.x * _11715.y));
                                }
                                else
                                {
                                    vec4 _17203;
                                    if (_11558 == 4)
                                    {
                                        vec2 _17170;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16440 = _11727;
                                            _16440.y = 1.0 - _11727.y;
                                            _17170 = _16440;
                                        }
                                        else
                                        {
                                            _17170 = _11727;
                                        }
                                        float _12174 = _11727.y;
                                        vec2 _12175 = vec2(_11739.x, _12174);
                                        vec2 _17171;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16447 = _12175;
                                            _16447.y = 1.0 - _12174;
                                            _17171 = _16447;
                                        }
                                        else
                                        {
                                            _17171 = _12175;
                                        }
                                        float _12192 = _11739.y;
                                        vec2 _12193 = vec2(_11727.x, _12192);
                                        vec2 _17173;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16454 = _12193;
                                            _16454.y = 1.0 - _12192;
                                            _17173 = _16454;
                                        }
                                        else
                                        {
                                            _17173 = _12193;
                                        }
                                        vec2 _17175;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16461 = _11739;
                                            _16461.y = 1.0 - _11739.y;
                                            _17175 = _16461;
                                        }
                                        else
                                        {
                                            _17175 = _11739;
                                        }
                                        _17203 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17170, 0.0) * (_11711.x * _11711.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17171, 0.0) * (_11715.x * _11711.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17173, 0.0) * (_11711.x * _11715.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17175, 0.0) * (_11715.x * _11715.y));
                                    }
                                    else
                                    {
                                        vec2 _17163;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16468 = _11727;
                                            _16468.y = 1.0 - _11727.y;
                                            _17163 = _16468;
                                        }
                                        else
                                        {
                                            _17163 = _11727;
                                        }
                                        float _12304 = _11727.y;
                                        vec2 _12305 = vec2(_11739.x, _12304);
                                        vec2 _17164;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16475 = _12305;
                                            _16475.y = 1.0 - _12304;
                                            _17164 = _16475;
                                        }
                                        else
                                        {
                                            _17164 = _12305;
                                        }
                                        float _12322 = _11739.y;
                                        vec2 _12323 = vec2(_11727.x, _12322);
                                        vec2 _17166;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16482 = _12323;
                                            _16482.y = 1.0 - _12322;
                                            _17166 = _16482;
                                        }
                                        else
                                        {
                                            _17166 = _12323;
                                        }
                                        vec2 _17168;
                                        if (_184.gConv.x > 0.5)
                                        {
                                            vec2 _16489 = _11739;
                                            _16489.y = 1.0 - _11739.y;
                                            _17168 = _16489;
                                        }
                                        else
                                        {
                                            _17168 = _11739;
                                        }
                                        _17203 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17163, 0.0) * (_11711.x * _11711.y)) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17164, 0.0) * (_11715.x * _11711.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17166, 0.0) * (_11711.x * _11715.y))) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17168, 0.0) * (_11715.x * _11715.y));
                                    }
                                    _17202 = _17203;
                                }
                                _17201 = _17202;
                            }
                            _17200 = _17201;
                        }
                        _17199 = _17200;
                    }
                    _11572 = _17232 + (_17199.xyz * (_11543 ? ((_17162 == 0) ? (1.0 - _11538) : _11538) : 1.0));
                }
                _16348.z = _17232.z;
                _17402 = _16348;
            }
            else
            {
                _17402 = _17012;
            }
            vec3 _17622;
            SPIRV_CROSS_BRANCH
            if (_7210 > 0.0)
            {
                vec2 _7351 = _7268 + (((_16940 * min(_7201 * 0.5, 16.0)) * _184.gDisplay.zw) * _184.gTarget.zw);
                float _12419 = clamp(log2(max(16.0 * _184.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _12422 = int(floor(_12419));
                float _12426 = _12419 - float(_12422);
                bool _12431 = (_12426 > 0.0199999995529651641845703125) && (_12422 < 5);
                vec3 _17387;
                _17387 = vec3(0.0);
                vec3 _12460;
                SPIRV_CROSS_LOOP
                for (int _17372 = 0; _17372 < 2; _17387 = _12460, _17372++)
                {
                    if ((_17372 > 0) && (!_12431))
                    {
                        break;
                    }
                    int _12446 = _12422 + _17372;
                    vec2 _17373;
                    if (_184.gConv.x > 0.5)
                    {
                        vec2 _16496 = _7351;
                        _16496.y = 1.0 - _7351.y;
                        _17373 = _16496;
                    }
                    else
                    {
                        _17373 = _7351;
                    }
                    vec4 _17374;
                    SPIRV_CROSS_BRANCH
                    if (_12446 <= 0)
                    {
                        _17374 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _17373, 0.0);
                    }
                    else
                    {
                        vec4 _17375;
                        if (_12446 == 1)
                        {
                            _17375 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17373, 0.0);
                        }
                        else
                        {
                            vec4 _17376;
                            if (_12446 == 2)
                            {
                                _17376 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17373, 0.0);
                            }
                            else
                            {
                                vec4 _17377;
                                if (_12446 == 3)
                                {
                                    _17377 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17373, 0.0);
                                }
                                else
                                {
                                    vec4 _17378;
                                    if (_12446 == 4)
                                    {
                                        _17378 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17373, 0.0);
                                    }
                                    else
                                    {
                                        _17378 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17373, 0.0);
                                    }
                                    _17377 = _17378;
                                }
                                _17376 = _17377;
                            }
                            _17375 = _17376;
                        }
                        _17374 = _17375;
                    }
                    _12460 = _17387 + (_17374.xyz * (_12431 ? ((_17372 == 0) ? (1.0 - _12426) : _12426) : 1.0));
                }
                _17622 = _17387;
            }
            else
            {
                _17622 = vec3(0.5);
            }
            float _17418;
            SPIRV_CROSS_BRANCH
            if (esia_v5.x > 0.001000000047497451305389404296875)
            {
                int _12549 = clamp(int(roundEven(log2(36.0 * _184.gDisplay.z) - 1.0)), 1, 4);
                vec2 _17393;
                if (_184.gConv.x > 0.5)
                {
                    vec2 _16500 = _7268;
                    _16500.y = 1.0 - _7268.y;
                    _17393 = _16500;
                }
                else
                {
                    _17393 = _7268;
                }
                vec4 _17394;
                SPIRV_CROSS_BRANCH
                if (_12549 <= 0)
                {
                    _17394 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _17393, 0.0);
                }
                else
                {
                    vec4 _17395;
                    if (_12549 == 1)
                    {
                        _17395 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _17393, 0.0);
                    }
                    else
                    {
                        vec4 _17396;
                        if (_12549 == 2)
                        {
                            _17396 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _17393, 0.0);
                        }
                        else
                        {
                            vec4 _17397;
                            if (_12549 == 3)
                            {
                                _17397 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _17393, 0.0);
                            }
                            else
                            {
                                vec4 _17398;
                                if (_12549 == 4)
                                {
                                    _17398 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _17393, 0.0);
                                }
                                else
                                {
                                    _17398 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _17393, 0.0);
                                }
                                _17397 = _17398;
                            }
                            _17396 = _17397;
                        }
                        _17395 = _17396;
                    }
                    _17394 = _17395;
                }
                _17418 = dot(_17394.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _17418 = 0.5;
            }
            _17620 = _17622;
            _17417 = _17418;
            _17399 = _17402;
        }
        else
        {
            _17620 = vec3(0.5);
            _17417 = 0.5;
            _17399 = vec3(0.5);
        }
        vec3 _7379 = mix(vec3(dot(_17399, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _17399, vec3(_7071.x)) + vec3(_7071.y);
        vec3 _17513;
        SPIRV_CROSS_BRANCH
        if (esia_v5.x > 0.001000000047497451305389404296875)
        {
            float _7389 = clamp(max(_17417 + _7071.y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _7397 = mix(_7389, dot(_7030.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), esia_v5.x);
            vec3 _7405 = _7379 * (_7397 / _7389);
            vec3 _7415 = mix(_7379, vec3(1.0), vec3((_7397 - _7389) / max(1.0 - _7389, 0.001000000047497451305389404296875)));
            bvec3 _7416 = bvec3(_7397 < _7389);
            _17513 = vec3(_7416.x ? _7405.x : _7415.x, _7416.y ? _7405.y : _7415.y, _7416.z ? _7405.z : _7415.z);
        }
        else
        {
            _17513 = _7379;
        }
        vec2 _7435 = clamp((esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875)), vec2(0.0), vec2(1.0));
        vec2 _7483 = vec2(cos(_7071.w), sin(_7071.w));
        float _7486 = dot(_16940, _7483);
        float _7497 = -_16753;
        float _7504 = _4109 * ((0.5 * abs(_16940.x)) + 0.0199999995529651641845703125);
        float _7511 = _4109 * ((0.5 * abs(_16940.y)) + 0.0199999995529651641845703125);
        float _17732;
        SPIRV_CROSS_BRANCH
        if ((((_7497 + _7504) + _7511) > 0.0) && (((_7497 - _7504) - _7511) < (0.5 * _7201)))
        {
            float _12653 = max((_7497 + _7504) + _7511, 0.0) / _7201;
            float _12657 = clamp(sqrt(2.0 * _12653), 0.12399999797344207763671875, 1.0);
            float _12659 = 1.0 - _12657;
            float _12662 = _12659 * _12659;
            float _12716 = max((_7497 + _7504) - _7511, 0.0) / _7201;
            float _12720 = clamp(sqrt(2.0 * _12716), 0.12399999797344207763671875, 1.0);
            float _12722 = 1.0 - _12720;
            float _12725 = _12722 * _12722;
            float _12779 = max((_7497 - _7504) + _7511, 0.0) / _7201;
            float _12783 = clamp(sqrt(2.0 * _12779), 0.12399999797344207763671875, 1.0);
            float _12785 = 1.0 - _12783;
            float _12788 = _12785 * _12785;
            float _12842 = max((_7497 - _7504) - _7511, 0.0) / _7201;
            float _12846 = clamp(sqrt(2.0 * _12842), 0.12399999797344207763671875, 1.0);
            float _12848 = 1.0 - _12846;
            float _12851 = _12848 * _12848;
            float _7558 = ((((((_12653 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12653) * _12653) : ((_12653 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12657) * _12657)) + ((((_12662 * _12662) * _12662) * _12659) * (0.02380952425301074981689453125 + (_12659 * ((-0.0386904776096343994140625) + (_12659 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12653 - 0.5))))) * _7201) * _7201) - ((((_12716 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12716) * _12716) : ((_12716 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12720) * _12720)) + ((((_12725 * _12725) * _12725) * _12722) * (0.02380952425301074981689453125 + (_12722 * ((-0.0386904776096343994140625) + (_12722 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12716 - 0.5))))) * _7201) * _7201)) - ((((_12779 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12779) * _12779) : ((_12779 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12783) * _12783)) + ((((_12788 * _12788) * _12788) * _12785) * (0.02380952425301074981689453125 + (_12785 * ((-0.0386904776096343994140625) + (_12785 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12779 - 0.5))))) * _7201) * _7201)) + ((((_12842 < 0.007687999866902828216552734375) ? ((0.2579232752323150634765625 * _12842) * _12842) : ((_12842 < 0.5) ? (((-0.000989689142443239688873291015625) + ((0.0113648362457752227783203125 * _12846) * _12846)) + ((((_12851 * _12851) * _12851) * _12848) * (0.02380952425301074981689453125 + (_12848 * ((-0.0386904776096343994140625) + (_12848 * 0.01587301678955554962158203125)))))) : (0.01037514768540859222412109375 + (0.022729672491550445556640625 * (_12842 - 0.5))))) * _7201) * _7201);
            _17732 = _7558 / ((4.0 * _7504) * _7511);
        }
        else
        {
            _17732 = 0.0;
        }
        float _7590 = clamp(0.5 + (0.5 * dot((esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5)) / _7193, _7483)), 0.0, 1.0);
        _18644 = ((_17732 * (pow(clamp(_7486, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(clamp(-_7486, 0.0, 1.0), 1.5)))) * _7071.z) * 1.60000002384185791015625;
        _18263 = _7112;
        _18250 = vec4((mix(mix(_17513, _7030.xyz, vec3(clamp(_7030.w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _7216))) + (0.3499999940395355224609375 * ((dot(_7030.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _7435.y) : _7435.y))), 0.0, 1.0))), (_17620 * 1.10000002384185791015625) + vec3(0.07999999821186065673828125), vec3(pow(1.0 - _17517.z, 5.0) * 0.3499999940395355224609375)) + vec3((((0.039999999105930328369140625 * _7590) * _7590) + (0.0500000007450580596923828125 * (1.0 - _7216))) * _7071.z)) * _4134, _4134) + (_17739 * (1.0 - _4134));
    }
    else
    {
        _18644 = 0.0;
        _18263 = _16733;
        _18250 = _17739;
    }
    vec4 _18260;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _2884) != 0u)
    {
        vec4 _17881;
        vec4 _18059;
        SPIRV_CROSS_BRANCH
        if (esia_v7.y != 0u)
        {
            uint _12908 = max(uint(_184.gConv.z), 1u);
            uint _12949 = max(uint(_184.gConv.z), 1u);
            _18059 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _12908) * 24u) + 3u), int(esia_v1 / _12908), 0).xy, 0);
            _17881 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _12949) * 24u) + 4u), int(esia_v1 / _12949), 0).xy, 0);
        }
        else
        {
            _18059 = vec4(0.0);
            _17881 = vec4(0.0);
        }
        vec4 _18245;
        do
        {
            if (esia_v7.y == 0u)
            {
                _18245 = esia_v4;
                break;
            }
            vec2 _13021 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            vec2 _13031 = fwidth(esia_v0);
            float _13033 = max(length(_13031), 9.9999997473787516355514526367188e-05);
            float _18237;
            float _18241;
            if ((esia_v7.y == 1u) || (esia_v7.y == 4u))
            {
                float _13042 = cos(_17881.x);
                float _13045 = sin(_17881.x);
                float _13071 = ((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_13042, _13045)) / max(0.5 * ((abs(_13042) * _13021.x) + (abs(_13045) * _13021.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (esia_v7.y == 4u)
                {
                    float _13086 = clamp((_13071 - _17881.y) / max(_17881.z - _17881.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _18245 = vec4(clamp(abs((fract(vec3(_13086 * 0.800000011920928955078125) + vec3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - vec3(3.0)) - vec3(1.0), vec3(0.0), vec3(1.0)), esia_v4.w * pow(max(sin(_13086 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _18241 = -1.0;
                _18237 = _13071;
            }
            else
            {
                float _18238;
                float _18242;
                if (esia_v7.y == 2u)
                {
                    _18242 = -1.0;
                    _18238 = length(esia_v0 - (esia_v2.xy + (_17881.xy * _13021))) / max(_17881.z * max(_13021.x, _13021.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    vec2 _13154 = esia_v0 - (esia_v2.xy + (_17881.xy * _13021));
                    float _13165 = fract(((atan(_13154.y, _13154.x) - _17881.z) * 0.15915493667125701904296875) + 1.0);
                    float _18239;
                    float _18243;
                    if (_17881.w > 0.5)
                    {
                        _18243 = -1.0;
                        _18239 = 0.5 - (0.5 * cos(_13165 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _13185 = (((_13165 < 0.5) ? _13165 : (_13165 - 1.0)) * 6.283185482025146484375) * length(_13154);
                        float _18244;
                        if (abs(_13185) < _13033)
                        {
                            _18244 = clamp(((_13185 / _13033) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _18244 = -1.0;
                        }
                        _18243 = _18244;
                        _18239 = _13165;
                    }
                    _18242 = _18243;
                    _18238 = _18239;
                }
                _18241 = _18242;
                _18237 = _18238;
            }
            vec4 _13214 = vec4(esia_v4.xyz * esia_v4.w, esia_v4.w);
            vec4 _13226 = vec4(_18059.xyz * _18059.w, _18059.w);
            vec4 _13233 = mix(_13226, _13214, vec4(_18241));
            vec4 _13238 = mix(_13214, _13226, vec4(clamp(_18237, 0.0, 1.0)));
            bvec4 _13239 = bvec4(_18241 >= 0.0);
            vec4 _13240 = vec4(_13239.x ? _13233.x : _13238.x, _13239.y ? _13233.y : _13238.y, _13239.z ? _13233.z : _13238.z, _13239.w ? _13233.w : _13238.w);
            vec4 _13255 = vec4(_13240.xyz / vec3(_13240.w), _13240.w);
            bvec4 _13256 = bvec4(_13240.w > 9.9999997473787516355514526367188e-06);
            _18245 = vec4(_13256.x ? _13255.x : vec4(0.0).x, _13256.y ? _13255.y : vec4(0.0).y, _13256.z ? _13255.z : vec4(0.0).z, _13256.w ? _13255.w : vec4(0.0).w);
            break;
        } while(false);
        vec4 _18246;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & _2913) != 0u)
        {
            uint _13266 = max(uint(_184.gConv.z), 1u);
            vec4 _13300 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13266) * 24u) + 18u), int(esia_v1 / _13266), 0).xy, 0);
            _18246 = _18245 * texture(SPIRV_Cross_CombinedgTexgLinear, mix(_13300.xy, _13300.zw, (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875))));
        }
        else
        {
            _18246 = _18245;
        }
        float _13308 = clamp(_18246.w * _4134, 0.0, 1.0);
        _18260 = vec4(_18246.xyz * _13308, _13308) + (_18250 * (1.0 - _13308));
    }
    else
    {
        _18260 = _18250;
    }
    vec4 _18630;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _2968) != 0u)
    {
        uint _13333 = max(uint(_184.gConv.z), 1u);
        vec4 _13367 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13333) * 24u) + 21u), int(esia_v1 / _13333), 0).xy, 0);
        uint _13374 = max(uint(_184.gConv.z), 1u);
        vec4 _13408 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13374) * 24u) + 3u), int(esia_v1 / _13374), 0).xy, 0);
        vec2 _4574 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        vec2 _4581 = (esia_v0 - esia_v2.xy) / _4574;
        float _4592 = ((_13408.x >= 0.0) ? _13408.x : _184.gTime.x) * _13367.z;
        float _4595 = _4581.x * 2.0;
        float _4596 = _4595 - 1.0;
        float _4601 = _4581.y * _4574.y;
        float _4604 = max(_13367.w, 0.001000000047497451305389404296875);
        float _4609 = clamp(1.0 - (_4596 * _4596), 0.0, 1.0);
        float _4611 = pow(_4609, 1.2999999523162841796875);
        float _4613 = pow(_4609, 0.699999988079071044921875);
        float _4616 = clamp(_4592 * 1.4285714626312255859375, 0.0, 1.0);
        float _4632 = (0.25 + (0.75 * ((_4616 * _4616) * (3.0 - (2.0 * _4616))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4592 * 2.099999904632568359375)));
        float _4641 = ((_13367.y * _4574.y) * _4632) * _4611;
        float _4667 = (_4574.y * (0.5 + ((_13367.x * (0.5 - (_4596 * _4596))) * 0.5))) + (((0.14000000059604644775390625 * _4574.y) * _4611) * sin(((_4596 * 2.400000095367431640625) - (_4592 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _18256;
        float _18257;
        vec3 _18258;
        _18258 = vec3(0.0);
        _18257 = _4667;
        _18256 = _4667;
        vec3 _4739;
        float _18892;
        float _18893;
        SPIRV_CROSS_UNROLL
        for (int _18255 = 0; _18255 < 4; _18258 = _4739, _18257 = _18893, _18256 = _18892, _18255++)
        {
            float _4691 = _4667 + ((_4641 * _3132[_18255].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4592 * 1.7000000476837158203125) + _3149[_18255].y))));
            _18892 = (_18255 == 0) ? _4691 : _18256;
            _18893 = (_18255 == 2) ? _4691 : _18257;
            float _4706 = _4604 * _3132[_18255].y;
            float _4711 = (_4601 - _4691) / _4706;
            float _4716 = _4604 * _3132[_18255].z;
            vec3 _18866;
            _18866 = vec3(0.0);
            SPIRV_CROSS_UNROLL
            for (int _18865 = 0; _18865 < 6; )
            {
                float _13431 = ((_4601 - _4691) - (_4716 * ((float(_18865) * 0.4000000059604644775390625) - 1.0))) / _4706;
                _18866 += (_1758[_18865] * exp((-_13431) * _13431));
                _18865++;
                continue;
            }
            _4739 = _18258 + (mix(_18866 * vec3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), vec3(exp((-_4711) * _4711)), vec3(_3149[_18255].x)) * (_3132[_18255].w * _4613));
        }
        float _4745 = _4604 * 1.5;
        float _4775 = clamp((_4601 - _18256) / max(_18257 - _18256, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4803 = (_4601 - (_18257 - (_4604 * 3.0))) / (((_4574.y * 0.0900000035762786865234375) + (_4641 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4806 = (_4595 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4831 = ((_4601 - _18256) + (_4604 * 5.0)) / (_4604 * 7.0);
        vec3 _4857 = vec3(1.0) - exp((-((((_18258 + (mix(vec3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), vec3(1.0), vec3(_4775)) * ((((1.0 / (1.0 + exp((-((_4601 - _18256) - (_4604 * 2.0))) / _4745))) / (1.0 + exp((-(_18257 - _4601)) / _4745))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4775, 2.5)))) * _4613))) + (vec3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4803) * _4803) - (_4806 * _4806)) * (0.5 + (1.10000002384185791015625 * _4632))))) + (vec3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4831) * _4831) * _4611) * 0.100000001490116119384765625))) * mix(vec3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), vec3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), vec3(0.5 + (0.5 * sin(_4592 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4867 = (esia_v4.w * smoothstep(0.0, 0.119999997317790985107421875, _4581.y)) * smoothstep(1.0, 0.87999999523162841796875, _4581.y);
        float _4886 = (clamp(max(_4857.x, max(_4857.y, _4857.z)), 0.0, 1.0) * _4867) * _4134;
        _18630 = vec4((_4857 * _4867) * _4134, _4886) + (_18260 * (1.0 - _4886));
    }
    else
    {
        _18630 = _18260;
    }
    vec4 _18639;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & _3401) != 0u) && ((esia_v7.x & _3405) != 0u))
    {
        uint _13464 = max(uint(_184.gConv.z), 1u);
        vec4 _13498 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13464) * 24u) + 7u), int(esia_v1 / _13464), 0).xy, 0);
        uint _13505 = max(uint(_184.gConv.z), 1u);
        vec4 _13539 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _13505) * 24u) + 8u), int(esia_v1 / _13505), 0).xy, 0);
        vec2 _4920 = esia_v0 - _13539.zw;
        vec2 _13578 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _13587 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _18605;
        SPIRV_CROSS_BRANCH
        if (esia_v7.z == 1u)
        {
            vec2 _13593 = _4920 - _13578;
            float _18604;
            do
            {
                if (esia_v5.w >= 6.282185077667236328125)
                {
                    _18604 = abs(length(_13593) - esia_v3.x) - esia_v3.y;
                    break;
                }
                float _13692 = esia_v5.z + (esia_v5.w * 0.5);
                float _13694 = cos(_13692);
                float _13696 = sin(_13692);
                float _13705 = dot(_13593, vec2(-_13696, _13694));
                float _13708 = dot(_13593, vec2(_13694, _13696));
                vec2 _13709 = vec2(_13705, _13708);
                float _13712 = abs(_13705);
                _13709.x = _13712;
                float _13715 = esia_v5.w * 0.5;
                float _13717 = sin(_13715);
                float _13719 = cos(_13715);
                _18604 = (((_13719 * _13712) > (_13717 * _13708)) ? length(_13709 - (vec2(_13717, _13719) * esia_v3.x)) : abs(length(_13709) - esia_v3.x)) - esia_v3.y;
                break;
            } while(false);
            _18605 = _18604;
        }
        else
        {
            float _18606;
            if (esia_v7.z == 2u)
            {
                vec2 _13755 = _4920 - _16735.xy;
                vec2 _13758 = _16735.zw - _16735.xy;
                _18606 = length(_13755 - (_13758 * clamp(dot(_13755, _13758) / max(dot(_13758, _13758), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
            }
            else
            {
                vec2 _13620 = _4920 - _13578;
                float _13811 = min(_13587.x, _13587.y);
                float _13814 = min((_13620.x > 0.0) ? ((_13620.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_13620.y > 0.0) ? esia_v3.w : esia_v3.x), _13811);
                float _13820 = _13814 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _18589;
                float _18590;
                if (_13820 > _13811)
                {
                    float _13834 = esia_v5.y * clamp((_13811 - _13814) / max(0.60000002384185791015625 * _13814, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _18590 = _13834;
                    _18589 = _13814 * (1.0 + (0.60000002384185791015625 * _13834));
                }
                else
                {
                    _18590 = esia_v5.y;
                    _18589 = _13820;
                }
                vec2 _13847 = (abs(_13620) - _13587) + vec2(_18589);
                vec2 _13849 = max(_13847, vec2(0.0));
                float _18591;
                SPIRV_CROSS_BRANCH
                if ((_13849.x > 0.0) && (_13849.y > 0.0))
                {
                    float _18592;
                    if ((_18590 > 0.001000000047497451305389404296875) && (_18589 > 9.9999997473787516355514526367188e-05))
                    {
                        float _13866 = 2.0 + (2.0 * _18590);
                        vec2 _13871 = _13849 / vec2(max(_18589, 9.9999997473787516355514526367188e-05));
                        _18592 = pow(pow(_13871.x, _13866) + pow(_13871.y, _13866), 1.0 / _13866) * _18589;
                    }
                    else
                    {
                        _18592 = length(_13849);
                    }
                    _18591 = _18592;
                }
                else
                {
                    _18591 = max(_13849.x, _13849.y);
                }
                float _13906 = (min(max(_13847.x, _13847.y), 0.0) + _18591) - _18589;
                float _18607;
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & _1294) != 0u)
                {
                    vec2 _13648 = max((_16735.zw - _16735.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _13651 = _4920 - ((_16735.xy + _16735.zw) * 0.5);
                    float _13942 = min(_13648.x, _13648.y);
                    float _13945 = min((_13651.x > 0.0) ? ((_13651.y > 0.0) ? _18263.x : _18263.x) : ((_13651.y > 0.0) ? _18263.x : _18263.x), _13942);
                    float _13951 = _13945 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _18595;
                    float _18596;
                    if (_13951 > _13942)
                    {
                        float _13965 = esia_v5.y * clamp((_13942 - _13945) / max(0.60000002384185791015625 * _13945, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _18596 = _13965;
                        _18595 = _13945 * (1.0 + (0.60000002384185791015625 * _13965));
                    }
                    else
                    {
                        _18596 = esia_v5.y;
                        _18595 = _13951;
                    }
                    vec2 _13978 = (abs(_13651) - _13648) + vec2(_18595);
                    vec2 _13980 = max(_13978, vec2(0.0));
                    float _18597;
                    SPIRV_CROSS_BRANCH
                    if ((_13980.x > 0.0) && (_13980.y > 0.0))
                    {
                        float _18598;
                        if ((_18596 > 0.001000000047497451305389404296875) && (_18595 > 9.9999997473787516355514526367188e-05))
                        {
                            float _13997 = 2.0 + (2.0 * _18596);
                            vec2 _14002 = _13980 / vec2(max(_18595, 9.9999997473787516355514526367188e-05));
                            _18598 = pow(pow(_14002.x, _13997) + pow(_14002.y, _13997), 1.0 / _13997) * _18595;
                        }
                        else
                        {
                            _18598 = length(_13980);
                        }
                        _18597 = _18598;
                    }
                    else
                    {
                        _18597 = max(_13980.x, _13980.y);
                    }
                    float _14037 = (min(max(_13978.x, _13978.y), 0.0) + _18597) - _18595;
                    float _14042 = max(_18263.y, 9.9999997473787516355514526367188e-05);
                    float _14051 = max(_14042 - abs(_13906 - _14037), 0.0) / _14042;
                    _18607 = min(_13906, _14037) - (((_14051 * _14051) * _14042) * 0.25);
                }
                else
                {
                    _18607 = _13906;
                }
                _18606 = _18607;
            }
            _18605 = _18606;
        }
        float _4929 = (_18605 + _13539.y) / (max(_13539.x * 0.5, _4109 * 0.5) * 1.41421353816986083984375);
        float _14068 = sign(_4929);
        float _14070 = abs(_4929);
        float _14081 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_14070 * _14070))) * _14070)) * _14070);
        float _14084 = _14081 * _14081;
        float _14099 = clamp(_13498.w * ((0.5 + (0.5 * (_14068 - (_14068 / (_14084 * _14084))))) * _4134), 0.0, 1.0);
        _18639 = vec4(_13498.xyz * _14099, _14099) + (_18630 * (1.0 - _14099));
    }
    else
    {
        _18639 = _18630;
    }
    vec4 _18663;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _3465) != 0u)
    {
        uint _14124 = max(uint(_184.gConv.z), 1u);
        vec4 _14158 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _14124) * 24u) + 9u), int(esia_v1 / _14124), 0).xy, 0);
        uint _14165 = max(uint(_184.gConv.z), 1u);
        vec4 _14199 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _14165) * 24u) + 10u), int(esia_v1 / _14165), 0).xy, 0);
        float _4961 = max(-_16753, 0.0) / max(_14199.z, 0.001000000047497451305389404296875);
        float _14207 = clamp(_14158.w * clamp((exp(((-_4961) * _4961) * 2.2000000476837158203125) * _14199.w) * _4134, 0.0, 1.0), 0.0, 1.0);
        _18663 = vec4(_14158.xyz * _14207, _14207) + (_18639 * (1.0 - _14207));
    }
    else
    {
        _18663 = _18639;
    }
    vec3 _4992 = _18663.xyz + vec3(_18644 * clamp(_18663.w / max(_4134, 0.001000000047497451305389404296875), 0.0, 1.0));
    vec4 _16628 = _18663;
    _16628.x = _4992.x;
    _16628.y = _4992.y;
    _16628.z = _4992.z;
    vec4 _18666;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _3534) != 0u)
    {
        uint _14232 = max(uint(_184.gConv.z), 1u);
        vec4 _14266 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _14232) * 24u) + 5u), int(esia_v1 / _14232), 0).xy, 0);
        uint _14273 = max(uint(_184.gConv.z), 1u);
        vec4 _14307 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _14273) * 24u) + 6u), int(esia_v1 / _14273), 0).xy, 0);
        float _5012 = _14307.x;
        float _5014 = _14307.y;
        vec4 _18664;
        if (_14307.z < 0.999000012874603271484375)
        {
            vec2 _5050 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            float _5061 = cos(_14307.w);
            float _5064 = sin(_14307.w);
            vec4 _16645 = _14266;
            _16645.w = _14266.w * mix(1.0, _14307.z, clamp(((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_5061, _5064)) / max(0.5 * ((abs(_5061) * _5050.x) + (abs(_5064) * _5050.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _18664 = _16645;
        }
        else
        {
            _18664 = _14266;
        }
        float _14315 = clamp(_18664.w * (clamp(0.5 - ((_16753 - (_5012 * _5014)) / _4109), 0.0, 1.0) - clamp(0.5 - ((_16753 + (_5012 * (1.0 - _5014))) / _4109), 0.0, 1.0)), 0.0, 1.0);
        _18666 = vec4(_18664.xyz * _14315, _14315) + (_16628 * (1.0 - _14315));
    }
    else
    {
        _18666 = _16628;
    }
    vec4 _18667;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _3664) != 0u)
    {
        vec2 _5125 = (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        float _5147 = exp(-pow((((_5125.x * 0.85000002384185791015625) + (_5125.y * 0.1500000059604644775390625)) - ((fract(_184.gTime.x * esia_v6.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _18667 = vec4(_18666.xyz + vec3(((_5147 * esia_v6.z) * _4134) * max(_18666.w, 0.3499999940395355224609375)), max(_18666.w, ((_5147 * esia_v6.z) * _4134) * 0.5));
    }
    else
    {
        _18667 = _18666;
    }
    vec4 _18860;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & _3746) != 0u)
    {
        vec3 _14340 = fract(floor(_16732).xyx * 0.103100001811981201171875);
        vec3 _14349 = _14340 + vec3(dot(_14340, _14340.yzx + vec3(33.3300018310546875)));
        vec3 _5198 = _18667.xyz + vec3(((fract((_14349.x + _14349.y) * _14349.z) - 0.5) * esia_v6.y) * _18667.w);
        vec4 _16669 = _18667;
        _16669.x = _5198.x;
        _16669.y = _5198.y;
        _16669.z = _5198.z;
        _18860 = _16669;
    }
    else
    {
        _18860 = _18667;
    }
    float _18856;
    if (_227.gFade.z > 0.0)
    {
        _18856 = smoothstep(0.0, 1.0, clamp((_16732.y - _227.gFade.x) / _227.gFade.z, 0.0, 1.0));
    }
    else
    {
        _18856 = 1.0;
    }
    float _18857;
    if (_227.gFade.w > 0.0)
    {
        _18857 = _18856 * smoothstep(0.0, 1.0, clamp((_227.gFade.y - _16732.y) / _227.gFade.w, 0.0, 1.0));
    }
    else
    {
        _18857 = _18856;
    }
    vec4 _5215 = _18860 * ((esia_v6.x * _18678) * _18857);
    vec4 _18861;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & _3786) != 0u) || ((esia_v7.x & _3790) != 0u))
    {
        vec3 _14401 = fract((floor(_16732) + vec2(17.0)).xyx * 0.103100001811981201171875);
        vec3 _14410 = _14401 + vec3(dot(_14401, _14401.yzx + vec3(33.3300018310546875)));
        vec3 _5239 = _5215.xyz + vec3(((fract((_14410.x + _14410.y) * _14410.z) - 0.5) * 0.0039215688593685626983642578125) * clamp(_5215.w * 8.0, 0.0, 1.0));
        vec4 _16681 = _5215;
        _16681.x = _5239.x;
        _16681.y = _5239.y;
        _16681.z = _5239.z;
        _18861 = _16681;
    }
    else
    {
        _18861 = _5215;
    }
    vec3 _5249 = max(_18861.xyz, vec3(0.0));
    vec4 _16687 = _18861;
    _16687.x = _5249.x;
    _16687.y = _5249.y;
    _16687.z = _5249.z;
    vec4 _18862;
    if ((_184.gTime.w > 0.5) && (_18861.w > 9.9999997473787516355514526367188e-06))
    {
        vec3 _14454 = clamp(_16687.xyz / vec3(_18861.w), vec3(0.0), vec3(1.0));
        vec3 _14460 = pow((_14454 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _14463 = _14454 * vec3(0.077399380505084991455078125);
        bvec3 _14465 = lessThanEqual(_14454, vec3(0.040449999272823333740234375));
        vec3 _14440 = vec3(_14465.x ? _14463.x : _14460.x, _14465.y ? _14463.y : _14460.y, _14465.z ? _14463.z : _14460.z) * _18861.w;
        vec4 _16696 = _16687;
        _16696.x = _14440.x;
        _16696.y = _14440.y;
        _16696.z = _14440.z;
        _18862 = _16696;
    }
    else
    {
        _18862 = _16687;
    }
    _entryPointOutput = _18862;
}

