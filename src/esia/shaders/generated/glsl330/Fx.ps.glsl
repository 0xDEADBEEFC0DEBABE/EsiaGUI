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

const vec3 _1694[6] = vec3[](vec3(1.0, 0.4199999868869781494140625, 0.2199999988079071044921875), vec3(1.0, 0.699999988079071044921875, 0.300000011920928955078125), vec3(0.800000011920928955078125, 0.920000016689300537109375, 0.4000000059604644775390625), vec3(0.3499999940395355224609375, 0.89999997615814208984375, 0.699999988079071044921875), vec3(0.4000000059604644775390625, 0.62000000476837158203125, 1.0), vec3(0.660000026226043701171875, 0.5, 1.0));
const vec4 _2912[4] = vec4[](vec4(-1.0, 1.0, 3.400000095367431640625, 2.599999904632568359375), vec4(-0.550000011920928955078125, 0.800000011920928955078125, 2.0, 0.800000011920928955078125), vec4(0.300000011920928955078125, 1.0, 1.2000000476837158203125, 1.2999999523162841796875), vec4(0.62000000476837158203125, 0.800000011920928955078125, 1.60000002384185791015625, 0.449999988079071044921875));
const vec2 _2929[4] = vec2[](vec2(0.0), vec2(0.100000001490116119384765625, 1.2999999523162841796875), vec2(0.550000011920928955078125, 3.900000095367431640625), vec2(0.20000000298023223876953125, 5.19999980926513671875));

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _172;

layout(std140) uniform WgtDraw
{
    vec4 gFade;
    vec4 gDrawInfo;
} _215;

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
    vec2 _5076 = gl_FragCoord.xy + _172.gConv.yy;
    vec2 _22844;
    if (_172.gConv.x > 0.5)
    {
        vec2 _21155 = _5076;
        _21155.y = _172.gTarget.y - _5076.y;
        _22844 = _21155;
    }
    else
    {
        _22844 = _5076;
    }
    float _3871 = dFdx(esia_v0.x);
    float _3875 = dFdy(esia_v0.x);
    float _3878 = max(abs(_3871) + abs(_3875), 9.9999997473787516355514526367188e-05);
    vec4 _22845;
    vec4 _22847;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.z == 2u) || ((esia_v7.x & 512u) != 0u))
    {
        uint _5095 = max(uint(_172.gConv.z), 1u);
        uint _5136 = max(uint(_172.gConv.z), 1u);
        _22847 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5095) * 24u) + 15u), int(esia_v1 / _5095), 0).xy, 0);
        _22845 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5136) * 24u) + 16u), int(esia_v1 / _5136), 0).xy, 0);
    }
    else
    {
        _22847 = vec4(0.0);
        _22845 = vec4(0.0);
    }
    vec2 _5209 = (esia_v2.xy + esia_v2.zw) * 0.5;
    vec2 _5218 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
    float _22865;
    SPIRV_CROSS_BRANCH
    if (esia_v7.z == 1u)
    {
        vec2 _5224 = esia_v0 - _5209;
        float _22864;
        do
        {
            if (esia_v5.w >= 6.282185077667236328125)
            {
                _22864 = abs(length(_5224) - esia_v3.x) - esia_v3.y;
                break;
            }
            float _5323 = esia_v5.z + (esia_v5.w * 0.5);
            float _5325 = cos(_5323);
            float _5327 = sin(_5323);
            float _5336 = dot(_5224, vec2(-_5327, _5325));
            float _5339 = dot(_5224, vec2(_5325, _5327));
            vec2 _5340 = vec2(_5336, _5339);
            float _5343 = abs(_5336);
            _5340.x = _5343;
            float _5346 = esia_v5.w * 0.5;
            float _5348 = sin(_5346);
            float _5350 = cos(_5346);
            _22864 = (((_5350 * _5343) > (_5348 * _5339)) ? length(_5340 - (vec2(_5348, _5350) * esia_v3.x)) : abs(length(_5340) - esia_v3.x)) - esia_v3.y;
            break;
        } while(false);
        _22865 = _22864;
    }
    else
    {
        float _22866;
        if (esia_v7.z == 2u)
        {
            vec2 _5386 = esia_v0 - _22847.xy;
            vec2 _5389 = _22847.zw - _22847.xy;
            _22866 = length(_5386 - (_5389 * clamp(dot(_5386, _5389) / max(dot(_5389, _5389), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
        }
        else
        {
            vec2 _5251 = esia_v0 - _5209;
            float _5442 = min(_5218.x, _5218.y);
            float _5445 = min((_5251.x > 0.0) ? ((_5251.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_5251.y > 0.0) ? esia_v3.w : esia_v3.x), _5442);
            float _5451 = _5445 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
            float _22849;
            float _22850;
            if (_5451 > _5442)
            {
                float _5465 = esia_v5.y * clamp((_5442 - _5445) / max(0.60000002384185791015625 * _5445, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                _22850 = _5465;
                _22849 = _5445 * (1.0 + (0.60000002384185791015625 * _5465));
            }
            else
            {
                _22850 = esia_v5.y;
                _22849 = _5451;
            }
            vec2 _5478 = (abs(_5251) - _5218) + vec2(_22849);
            vec2 _5480 = max(_5478, vec2(0.0));
            float _22851;
            SPIRV_CROSS_BRANCH
            if ((_5480.x > 0.0) && (_5480.y > 0.0))
            {
                float _22852;
                if ((_22850 > 0.001000000047497451305389404296875) && (_22849 > 9.9999997473787516355514526367188e-05))
                {
                    float _5497 = 2.0 + (2.0 * _22850);
                    vec2 _5502 = _5480 / vec2(max(_22849, 9.9999997473787516355514526367188e-05));
                    _22852 = pow(pow(_5502.x, _5497) + pow(_5502.y, _5497), 1.0 / _5497) * _22849;
                }
                else
                {
                    _22852 = length(_5480);
                }
                _22851 = _22852;
            }
            else
            {
                _22851 = max(_5480.x, _5480.y);
            }
            float _5537 = (min(max(_5478.x, _5478.y), 0.0) + _22851) - _22849;
            float _22867;
            SPIRV_CROSS_BRANCH
            if ((esia_v7.x & 512u) != 0u)
            {
                vec2 _5279 = max((_22847.zw - _22847.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                vec2 _5282 = esia_v0 - ((_22847.xy + _22847.zw) * 0.5);
                float _5573 = min(_5279.x, _5279.y);
                float _5576 = min((_5282.x > 0.0) ? ((_5282.y > 0.0) ? _22845.x : _22845.x) : ((_5282.y > 0.0) ? _22845.x : _22845.x), _5573);
                float _5582 = _5576 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _22855;
                float _22856;
                if (_5582 > _5573)
                {
                    float _5596 = esia_v5.y * clamp((_5573 - _5576) / max(0.60000002384185791015625 * _5576, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _22856 = _5596;
                    _22855 = _5576 * (1.0 + (0.60000002384185791015625 * _5596));
                }
                else
                {
                    _22856 = esia_v5.y;
                    _22855 = _5582;
                }
                vec2 _5609 = (abs(_5282) - _5279) + vec2(_22855);
                vec2 _5611 = max(_5609, vec2(0.0));
                float _22857;
                SPIRV_CROSS_BRANCH
                if ((_5611.x > 0.0) && (_5611.y > 0.0))
                {
                    float _22858;
                    if ((_22856 > 0.001000000047497451305389404296875) && (_22855 > 9.9999997473787516355514526367188e-05))
                    {
                        float _5628 = 2.0 + (2.0 * _22856);
                        vec2 _5633 = _5611 / vec2(max(_22855, 9.9999997473787516355514526367188e-05));
                        _22858 = pow(pow(_5633.x, _5628) + pow(_5633.y, _5628), 1.0 / _5628) * _22855;
                    }
                    else
                    {
                        _22858 = length(_5611);
                    }
                    _22857 = _22858;
                }
                else
                {
                    _22857 = max(_5611.x, _5611.y);
                }
                float _5668 = (min(max(_5609.x, _5609.y), 0.0) + _22857) - _22855;
                float _5673 = max(_22845.y, 9.9999997473787516355514526367188e-05);
                float _5682 = max(_5673 - abs(_5537 - _5668), 0.0) / _5673;
                _22867 = min(_5537, _5668) - (((_5682 * _5682) * _5673) * 0.25);
            }
            else
            {
                _22867 = _5537;
            }
            _22866 = _22867;
        }
        _22865 = _22866;
    }
    float _3903 = clamp(0.5 - (_22865 / _3878), 0.0, 1.0);
    float _25999;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 1024u) != 0u)
    {
        uint _5699 = max(uint(_172.gConv.z), 1u);
        vec4 _5733 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5699) * 24u) + 19u), int(esia_v1 / _5699), 0).xy, 0);
        uint _5740 = max(uint(_172.gConv.z), 1u);
        vec4 _5774 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5740) * 24u) + 20u), int(esia_v1 / _5740), 0).xy, 0);
        vec2 _3932 = max((_5733.zw - _5733.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        vec2 _3935 = esia_v0 - ((_5733.xy + _5733.zw) * 0.5);
        float _3941 = _5774.y;
        float _5810 = min(_3932.x, _3932.y);
        float _5813 = min((_3935.x > 0.0) ? ((_3935.y > 0.0) ? _5774.x : _5774.x) : ((_3935.y > 0.0) ? _5774.x : _5774.x), _5810);
        float _5819 = _5813 * (1.0 + (0.60000002384185791015625 * _3941));
        float _22868;
        float _22869;
        if (_5819 > _5810)
        {
            float _5833 = _3941 * clamp((_5810 - _5813) / max(0.60000002384185791015625 * _5813, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
            _22869 = _5833;
            _22868 = _5813 * (1.0 + (0.60000002384185791015625 * _5833));
        }
        else
        {
            _22869 = _3941;
            _22868 = _5819;
        }
        vec2 _5846 = (abs(_3935) - _3932) + vec2(_22868);
        vec2 _5848 = max(_5846, vec2(0.0));
        float _22870;
        SPIRV_CROSS_BRANCH
        if ((_5848.x > 0.0) && (_5848.y > 0.0))
        {
            float _22871;
            if ((_22869 > 0.001000000047497451305389404296875) && (_22868 > 9.9999997473787516355514526367188e-05))
            {
                float _5865 = 2.0 + (2.0 * _22869);
                vec2 _5870 = _5848 / vec2(max(_22868, 9.9999997473787516355514526367188e-05));
                _22871 = pow(pow(_5870.x, _5865) + pow(_5870.y, _5865), 1.0 / _5865) * _22868;
            }
            else
            {
                _22871 = length(_5848);
            }
            _22870 = _22871;
        }
        else
        {
            _22870 = max(_5848.x, _5848.y);
        }
        float _3947 = clamp(0.5 - (((min(max(_5846.x, _5846.y), 0.0) + _22870) - _22868) / _3878), 0.0, 1.0);
        if (_3947 <= 0.0)
        {
            discard;
        }
        _25999 = _3947;
    }
    else
    {
        _25999 = 1.0;
    }
    bool _3958 = ((esia_v7.x & 32u) != 0u) && (_3903 >= 0.999000012874603271484375);
    vec4 _22960;
    SPIRV_CROSS_BRANCH
    if ((((esia_v7.x & 4u) != 0u) && (!((esia_v7.x & 256u) != 0u))) && (!_3958))
    {
        uint _5912 = max(uint(_172.gConv.z), 1u);
        vec4 _5946 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5912) * 24u) + 7u), int(esia_v1 / _5912), 0).xy, 0);
        uint _5953 = max(uint(_172.gConv.z), 1u);
        vec4 _5987 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _5953) * 24u) + 8u), int(esia_v1 / _5953), 0).xy, 0);
        vec2 _3989 = esia_v0 - _5987.zw;
        vec2 _6026 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _6035 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _22918;
        SPIRV_CROSS_BRANCH
        if (esia_v7.z == 1u)
        {
            vec2 _6041 = _3989 - _6026;
            float _22917;
            do
            {
                if (esia_v5.w >= 6.282185077667236328125)
                {
                    _22917 = abs(length(_6041) - esia_v3.x) - esia_v3.y;
                    break;
                }
                float _6140 = esia_v5.z + (esia_v5.w * 0.5);
                float _6142 = cos(_6140);
                float _6144 = sin(_6140);
                float _6153 = dot(_6041, vec2(-_6144, _6142));
                float _6156 = dot(_6041, vec2(_6142, _6144));
                vec2 _6157 = vec2(_6153, _6156);
                float _6160 = abs(_6153);
                _6157.x = _6160;
                float _6163 = esia_v5.w * 0.5;
                float _6165 = sin(_6163);
                float _6167 = cos(_6163);
                _22917 = (((_6167 * _6160) > (_6165 * _6156)) ? length(_6157 - (vec2(_6165, _6167) * esia_v3.x)) : abs(length(_6157) - esia_v3.x)) - esia_v3.y;
                break;
            } while(false);
            _22918 = _22917;
        }
        else
        {
            float _22919;
            if (esia_v7.z == 2u)
            {
                vec2 _6203 = _3989 - _22847.xy;
                vec2 _6206 = _22847.zw - _22847.xy;
                _22919 = length(_6203 - (_6206 * clamp(dot(_6203, _6206) / max(dot(_6206, _6206), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
            }
            else
            {
                vec2 _6068 = _3989 - _6026;
                float _6259 = min(_6035.x, _6035.y);
                float _6262 = min((_6068.x > 0.0) ? ((_6068.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_6068.y > 0.0) ? esia_v3.w : esia_v3.x), _6259);
                float _6268 = _6262 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _22902;
                float _22903;
                if (_6268 > _6259)
                {
                    float _6282 = esia_v5.y * clamp((_6259 - _6262) / max(0.60000002384185791015625 * _6262, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _22903 = _6282;
                    _22902 = _6262 * (1.0 + (0.60000002384185791015625 * _6282));
                }
                else
                {
                    _22903 = esia_v5.y;
                    _22902 = _6268;
                }
                vec2 _6295 = (abs(_6068) - _6035) + vec2(_22902);
                vec2 _6297 = max(_6295, vec2(0.0));
                float _22904;
                SPIRV_CROSS_BRANCH
                if ((_6297.x > 0.0) && (_6297.y > 0.0))
                {
                    float _22905;
                    if ((_22903 > 0.001000000047497451305389404296875) && (_22902 > 9.9999997473787516355514526367188e-05))
                    {
                        float _6314 = 2.0 + (2.0 * _22903);
                        vec2 _6319 = _6297 / vec2(max(_22902, 9.9999997473787516355514526367188e-05));
                        _22905 = pow(pow(_6319.x, _6314) + pow(_6319.y, _6314), 1.0 / _6314) * _22902;
                    }
                    else
                    {
                        _22905 = length(_6297);
                    }
                    _22904 = _22905;
                }
                else
                {
                    _22904 = max(_6297.x, _6297.y);
                }
                float _6354 = (min(max(_6295.x, _6295.y), 0.0) + _22904) - _22902;
                float _22920;
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & 512u) != 0u)
                {
                    vec2 _6096 = max((_22847.zw - _22847.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _6099 = _3989 - ((_22847.xy + _22847.zw) * 0.5);
                    float _6390 = min(_6096.x, _6096.y);
                    float _6393 = min((_6099.x > 0.0) ? ((_6099.y > 0.0) ? _22845.x : _22845.x) : ((_6099.y > 0.0) ? _22845.x : _22845.x), _6390);
                    float _6399 = _6393 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _22908;
                    float _22909;
                    if (_6399 > _6390)
                    {
                        float _6413 = esia_v5.y * clamp((_6390 - _6393) / max(0.60000002384185791015625 * _6393, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22909 = _6413;
                        _22908 = _6393 * (1.0 + (0.60000002384185791015625 * _6413));
                    }
                    else
                    {
                        _22909 = esia_v5.y;
                        _22908 = _6399;
                    }
                    vec2 _6426 = (abs(_6099) - _6096) + vec2(_22908);
                    vec2 _6428 = max(_6426, vec2(0.0));
                    float _22910;
                    SPIRV_CROSS_BRANCH
                    if ((_6428.x > 0.0) && (_6428.y > 0.0))
                    {
                        float _22911;
                        if ((_22909 > 0.001000000047497451305389404296875) && (_22908 > 9.9999997473787516355514526367188e-05))
                        {
                            float _6445 = 2.0 + (2.0 * _22909);
                            vec2 _6450 = _6428 / vec2(max(_22908, 9.9999997473787516355514526367188e-05));
                            _22911 = pow(pow(_6450.x, _6445) + pow(_6450.y, _6445), 1.0 / _6445) * _22908;
                        }
                        else
                        {
                            _22911 = length(_6428);
                        }
                        _22910 = _22911;
                    }
                    else
                    {
                        _22910 = max(_6428.x, _6428.y);
                    }
                    float _6485 = (min(max(_6426.x, _6426.y), 0.0) + _22910) - _22908;
                    float _6490 = max(_22845.y, 9.9999997473787516355514526367188e-05);
                    float _6499 = max(_6490 - abs(_6354 - _6485), 0.0) / _6490;
                    _22920 = min(_6354, _6485) - (((_6499 * _6499) * _6490) * 0.25);
                }
                else
                {
                    _22920 = _6354;
                }
                _22919 = _22920;
            }
            _22918 = _22919;
        }
        float _3998 = (_22918 - _5987.y) / (max(_5987.x * 0.5, _3878 * 0.5) * 1.41421353816986083984375);
        float _6516 = sign(_3998);
        float _6518 = abs(_3998);
        float _6529 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_6518 * _6518))) * _6518)) * _6518);
        float _6532 = _6529 * _6529;
        float _6547 = clamp(_5946.w * (0.5 - (0.5 * (_6516 - (_6516 / (_6532 * _6532))))), 0.0, 1.0);
        _22960 = vec4(_5946.xyz * _6547, _6547);
    }
    else
    {
        _22960 = vec4(0.0);
    }
    vec4 _24399;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 8u) != 0u) && (!_3958))
    {
        uint _6572 = max(uint(_172.gConv.z), 1u);
        vec4 _6606 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6572) * 24u) + 9u), int(esia_v1 / _6572), 0).xy, 0);
        uint _6613 = max(uint(_172.gConv.z), 1u);
        vec4 _6647 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6613) * 24u) + 10u), int(esia_v1 / _6613), 0).xy, 0);
        float _4026 = max(_6647.x, 0.001000000047497451305389404296875);
        float _22953;
        float _22956;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & 8192u) != 0u)
        {
            uint _6654 = max(uint(_172.gConv.z), 1u);
            vec4 _6688 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6654) * 24u) + 22u), int(esia_v1 / _6654), 0).xy, 0);
            vec2 _4042 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _4051 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            vec2 _4058 = esia_v2.xy - _6688.xy;
            vec2 _4065 = _6688.zw - esia_v2.zw;
            vec2 _4095 = clamp(vec2((esia_v0.x < _4042.x) ? _4058.x : _4065.x, (esia_v0.y < _4042.y) ? _4058.y : _4065.y) * vec2(0.58823525905609130859375), vec2(min(_4026, 1.5)), vec2(_4026));
            float _4100 = min(_4051.x, _4051.y);
            float _22951;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 0u)
            {
                _22951 = min(((esia_v0.x > _4042.x) ? ((esia_v0.y > _4042.y) ? esia_v3.z : esia_v3.y) : ((esia_v0.y > _4042.y) ? esia_v3.w : esia_v3.x)) * (1.0 + (0.60000002384185791015625 * esia_v5.y)), _4100);
            }
            else
            {
                _22951 = _4100;
            }
            vec2 _4149 = max(abs(esia_v0 - _4042) - (_4051 - vec2(_22951)), vec2(0.0));
            float _4151 = length(_4149);
            vec2 _4157 = _4149 / vec2(_4151);
            bvec2 _4158 = bvec2(_4151 > 9.9999997473787516355514526367188e-05);
            vec2 _4159 = vec2(_4158.x ? _4157.x : vec2(0.707099974155426025390625).x, _4158.y ? _4157.y : vec2(0.707099974155426025390625).y);
            vec2 _4174 = esia_v0 - _6688.xy;
            vec2 _4179 = _6688.zw - esia_v0;
            _22956 = clamp(min(min(_4174.x, _4174.y), min(_4179.x, _4179.y)) * 0.666666686534881591796875, 0.0, 1.0);
            _22953 = inversesqrt(dot(_4159 * _4159, vec2(1.0) / (_4095 * _4095)));
        }
        else
        {
            _22956 = 1.0;
            _22953 = _4026;
        }
        float _4197 = max(_22865, 0.0) / _22953;
        float _6696 = clamp(_6606.w * clamp((exp(((-_4197) * _4197) * 2.2000000476837158203125) * _6647.y) * _22956, 0.0, 1.0), 0.0, 1.0);
        _24399 = vec4(_6606.xyz * _6696, _6696) + (_22960 * (1.0 - _6696));
    }
    else
    {
        _24399 = _22960;
    }
    vec4 _25307;
    vec4 _25320;
    float _25965;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 32u) != 0u) && (_3903 > 0.0))
    {
        uint _6721 = max(uint(_172.gConv.z), 1u);
        vec4 _6755 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6721) * 24u) + 11u), int(esia_v1 / _6721), 0).xy, 0);
        uint _6762 = max(uint(_172.gConv.z), 1u);
        vec4 _6796 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6762) * 24u) + 12u), int(esia_v1 / _6762), 0).xy, 0);
        uint _6803 = max(uint(_172.gConv.z), 1u);
        vec4 _6837 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6803) * 24u) + 13u), int(esia_v1 / _6803), 0).xy, 0);
        uint _6844 = max(uint(_172.gConv.z), 1u);
        vec4 _6878 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _6844) * 24u) + 16u), int(esia_v1 / _6844), 0).xy, 0);
        float _6938 = _6755.x * _172.gDisplay.z;
        float _6940 = _6755.y;
        vec2 _6949 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(1.0));
        float _6957 = clamp(_6755.z, 0.001000000047497451305389404296875, min(_6949.x, _6949.y));
        float _6959 = _6755.w;
        float _6966 = clamp(1.0 - (max(-_22865, 0.0) / _6957), 0.0, 1.0);
        float _6972 = sqrt(clamp(1.0 - (_6966 * _6966), 0.0, 1.0));
        vec2 _23052;
        vec3 _23914;
        SPIRV_CROSS_BRANCH
        if (_6966 > 0.0)
        {
            vec2 _7318 = esia_v0 + vec2(0.5, 0.0);
            vec2 _7386 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _7395 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _22992;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _7401 = _7318 - _7386;
                float _22991;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _22991 = abs(length(_7401) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _7500 = esia_v5.z + (esia_v5.w * 0.5);
                    float _7502 = cos(_7500);
                    float _7504 = sin(_7500);
                    float _7513 = dot(_7401, vec2(-_7504, _7502));
                    float _7516 = dot(_7401, vec2(_7502, _7504));
                    vec2 _7517 = vec2(_7513, _7516);
                    float _7520 = abs(_7513);
                    _7517.x = _7520;
                    float _7523 = esia_v5.w * 0.5;
                    float _7525 = sin(_7523);
                    float _7527 = cos(_7523);
                    _22991 = (((_7527 * _7520) > (_7525 * _7516)) ? length(_7517 - (vec2(_7525, _7527) * esia_v3.x)) : abs(length(_7517) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _22992 = _22991;
            }
            else
            {
                float _22993;
                if (esia_v7.z == 2u)
                {
                    vec2 _7563 = _7318 - _22847.xy;
                    vec2 _7566 = _22847.zw - _22847.xy;
                    _22993 = length(_7563 - (_7566 * clamp(dot(_7563, _7566) / max(dot(_7566, _7566), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _7428 = _7318 - _7386;
                    float _7619 = min(_7395.x, _7395.y);
                    float _7622 = min((_7428.x > 0.0) ? ((_7428.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_7428.y > 0.0) ? esia_v3.w : esia_v3.x), _7619);
                    float _7628 = _7622 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _22976;
                    float _22977;
                    if (_7628 > _7619)
                    {
                        float _7642 = esia_v5.y * clamp((_7619 - _7622) / max(0.60000002384185791015625 * _7622, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22977 = _7642;
                        _22976 = _7622 * (1.0 + (0.60000002384185791015625 * _7642));
                    }
                    else
                    {
                        _22977 = esia_v5.y;
                        _22976 = _7628;
                    }
                    vec2 _7655 = (abs(_7428) - _7395) + vec2(_22976);
                    vec2 _7657 = max(_7655, vec2(0.0));
                    float _22978;
                    SPIRV_CROSS_BRANCH
                    if ((_7657.x > 0.0) && (_7657.y > 0.0))
                    {
                        float _22979;
                        if ((_22977 > 0.001000000047497451305389404296875) && (_22976 > 9.9999997473787516355514526367188e-05))
                        {
                            float _7674 = 2.0 + (2.0 * _22977);
                            vec2 _7679 = _7657 / vec2(max(_22976, 9.9999997473787516355514526367188e-05));
                            _22979 = pow(pow(_7679.x, _7674) + pow(_7679.y, _7674), 1.0 / _7674) * _22976;
                        }
                        else
                        {
                            _22979 = length(_7657);
                        }
                        _22978 = _22979;
                    }
                    else
                    {
                        _22978 = max(_7657.x, _7657.y);
                    }
                    float _7714 = (min(max(_7655.x, _7655.y), 0.0) + _22978) - _22976;
                    float _22994;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _7456 = max((_22847.zw - _22847.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _7459 = _7318 - ((_22847.xy + _22847.zw) * 0.5);
                        float _7750 = min(_7456.x, _7456.y);
                        float _7753 = min((_7459.x > 0.0) ? ((_7459.y > 0.0) ? _6878.x : _6878.x) : ((_7459.y > 0.0) ? _6878.x : _6878.x), _7750);
                        float _7759 = _7753 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _22982;
                        float _22983;
                        if (_7759 > _7750)
                        {
                            float _7773 = esia_v5.y * clamp((_7750 - _7753) / max(0.60000002384185791015625 * _7753, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _22983 = _7773;
                            _22982 = _7753 * (1.0 + (0.60000002384185791015625 * _7773));
                        }
                        else
                        {
                            _22983 = esia_v5.y;
                            _22982 = _7759;
                        }
                        vec2 _7786 = (abs(_7459) - _7456) + vec2(_22982);
                        vec2 _7788 = max(_7786, vec2(0.0));
                        float _22984;
                        SPIRV_CROSS_BRANCH
                        if ((_7788.x > 0.0) && (_7788.y > 0.0))
                        {
                            float _22985;
                            if ((_22983 > 0.001000000047497451305389404296875) && (_22982 > 9.9999997473787516355514526367188e-05))
                            {
                                float _7805 = 2.0 + (2.0 * _22983);
                                vec2 _7810 = _7788 / vec2(max(_22982, 9.9999997473787516355514526367188e-05));
                                _22985 = pow(pow(_7810.x, _7805) + pow(_7810.y, _7805), 1.0 / _7805) * _22982;
                            }
                            else
                            {
                                _22985 = length(_7788);
                            }
                            _22984 = _22985;
                        }
                        else
                        {
                            _22984 = max(_7788.x, _7788.y);
                        }
                        float _7845 = (min(max(_7786.x, _7786.y), 0.0) + _22984) - _22982;
                        float _7850 = max(_6878.y, 9.9999997473787516355514526367188e-05);
                        float _7859 = max(_7850 - abs(_7714 - _7845), 0.0) / _7850;
                        _22994 = min(_7714, _7845) - (((_7859 * _7859) * _7850) * 0.25);
                    }
                    else
                    {
                        _22994 = _7714;
                    }
                    _22993 = _22994;
                }
                _22992 = _22993;
            }
            vec2 _7322 = esia_v0 - vec2(0.5, 0.0);
            vec2 _7908 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _7917 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _23011;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _7923 = _7322 - _7908;
                float _23010;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _23010 = abs(length(_7923) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _8022 = esia_v5.z + (esia_v5.w * 0.5);
                    float _8024 = cos(_8022);
                    float _8026 = sin(_8022);
                    float _8035 = dot(_7923, vec2(-_8026, _8024));
                    float _8038 = dot(_7923, vec2(_8024, _8026));
                    vec2 _8039 = vec2(_8035, _8038);
                    float _8042 = abs(_8035);
                    _8039.x = _8042;
                    float _8045 = esia_v5.w * 0.5;
                    float _8047 = sin(_8045);
                    float _8049 = cos(_8045);
                    _23010 = (((_8049 * _8042) > (_8047 * _8038)) ? length(_8039 - (vec2(_8047, _8049) * esia_v3.x)) : abs(length(_8039) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _23011 = _23010;
            }
            else
            {
                float _23012;
                if (esia_v7.z == 2u)
                {
                    vec2 _8085 = _7322 - _22847.xy;
                    vec2 _8088 = _22847.zw - _22847.xy;
                    _23012 = length(_8085 - (_8088 * clamp(dot(_8085, _8088) / max(dot(_8088, _8088), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _7950 = _7322 - _7908;
                    float _8141 = min(_7917.x, _7917.y);
                    float _8144 = min((_7950.x > 0.0) ? ((_7950.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_7950.y > 0.0) ? esia_v3.w : esia_v3.x), _8141);
                    float _8150 = _8144 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _22995;
                    float _22996;
                    if (_8150 > _8141)
                    {
                        float _8164 = esia_v5.y * clamp((_8141 - _8144) / max(0.60000002384185791015625 * _8144, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _22996 = _8164;
                        _22995 = _8144 * (1.0 + (0.60000002384185791015625 * _8164));
                    }
                    else
                    {
                        _22996 = esia_v5.y;
                        _22995 = _8150;
                    }
                    vec2 _8177 = (abs(_7950) - _7917) + vec2(_22995);
                    vec2 _8179 = max(_8177, vec2(0.0));
                    float _22997;
                    SPIRV_CROSS_BRANCH
                    if ((_8179.x > 0.0) && (_8179.y > 0.0))
                    {
                        float _22998;
                        if ((_22996 > 0.001000000047497451305389404296875) && (_22995 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8196 = 2.0 + (2.0 * _22996);
                            vec2 _8201 = _8179 / vec2(max(_22995, 9.9999997473787516355514526367188e-05));
                            _22998 = pow(pow(_8201.x, _8196) + pow(_8201.y, _8196), 1.0 / _8196) * _22995;
                        }
                        else
                        {
                            _22998 = length(_8179);
                        }
                        _22997 = _22998;
                    }
                    else
                    {
                        _22997 = max(_8179.x, _8179.y);
                    }
                    float _8236 = (min(max(_8177.x, _8177.y), 0.0) + _22997) - _22995;
                    float _23013;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _7978 = max((_22847.zw - _22847.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _7981 = _7322 - ((_22847.xy + _22847.zw) * 0.5);
                        float _8272 = min(_7978.x, _7978.y);
                        float _8275 = min((_7981.x > 0.0) ? ((_7981.y > 0.0) ? _6878.x : _6878.x) : ((_7981.y > 0.0) ? _6878.x : _6878.x), _8272);
                        float _8281 = _8275 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _23001;
                        float _23002;
                        if (_8281 > _8272)
                        {
                            float _8295 = esia_v5.y * clamp((_8272 - _8275) / max(0.60000002384185791015625 * _8275, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _23002 = _8295;
                            _23001 = _8275 * (1.0 + (0.60000002384185791015625 * _8295));
                        }
                        else
                        {
                            _23002 = esia_v5.y;
                            _23001 = _8281;
                        }
                        vec2 _8308 = (abs(_7981) - _7978) + vec2(_23001);
                        vec2 _8310 = max(_8308, vec2(0.0));
                        float _23003;
                        SPIRV_CROSS_BRANCH
                        if ((_8310.x > 0.0) && (_8310.y > 0.0))
                        {
                            float _23004;
                            if ((_23002 > 0.001000000047497451305389404296875) && (_23001 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8327 = 2.0 + (2.0 * _23002);
                                vec2 _8332 = _8310 / vec2(max(_23001, 9.9999997473787516355514526367188e-05));
                                _23004 = pow(pow(_8332.x, _8327) + pow(_8332.y, _8327), 1.0 / _8327) * _23001;
                            }
                            else
                            {
                                _23004 = length(_8310);
                            }
                            _23003 = _23004;
                        }
                        else
                        {
                            _23003 = max(_8310.x, _8310.y);
                        }
                        float _8367 = (min(max(_8308.x, _8308.y), 0.0) + _23003) - _23001;
                        float _8372 = max(_6878.y, 9.9999997473787516355514526367188e-05);
                        float _8381 = max(_8372 - abs(_8236 - _8367), 0.0) / _8372;
                        _23013 = min(_8236, _8367) - (((_8381 * _8381) * _8372) * 0.25);
                    }
                    else
                    {
                        _23013 = _8236;
                    }
                    _23012 = _23013;
                }
                _23011 = _23012;
            }
            vec2 _7327 = esia_v0 + vec2(0.0, 0.5);
            vec2 _8430 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _8439 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _23030;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _8445 = _7327 - _8430;
                float _23029;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _23029 = abs(length(_8445) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _8544 = esia_v5.z + (esia_v5.w * 0.5);
                    float _8546 = cos(_8544);
                    float _8548 = sin(_8544);
                    float _8557 = dot(_8445, vec2(-_8548, _8546));
                    float _8560 = dot(_8445, vec2(_8546, _8548));
                    vec2 _8561 = vec2(_8557, _8560);
                    float _8564 = abs(_8557);
                    _8561.x = _8564;
                    float _8567 = esia_v5.w * 0.5;
                    float _8569 = sin(_8567);
                    float _8571 = cos(_8567);
                    _23029 = (((_8571 * _8564) > (_8569 * _8560)) ? length(_8561 - (vec2(_8569, _8571) * esia_v3.x)) : abs(length(_8561) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _23030 = _23029;
            }
            else
            {
                float _23031;
                if (esia_v7.z == 2u)
                {
                    vec2 _8607 = _7327 - _22847.xy;
                    vec2 _8610 = _22847.zw - _22847.xy;
                    _23031 = length(_8607 - (_8610 * clamp(dot(_8607, _8610) / max(dot(_8610, _8610), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _8472 = _7327 - _8430;
                    float _8663 = min(_8439.x, _8439.y);
                    float _8666 = min((_8472.x > 0.0) ? ((_8472.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_8472.y > 0.0) ? esia_v3.w : esia_v3.x), _8663);
                    float _8672 = _8666 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _23014;
                    float _23015;
                    if (_8672 > _8663)
                    {
                        float _8686 = esia_v5.y * clamp((_8663 - _8666) / max(0.60000002384185791015625 * _8666, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23015 = _8686;
                        _23014 = _8666 * (1.0 + (0.60000002384185791015625 * _8686));
                    }
                    else
                    {
                        _23015 = esia_v5.y;
                        _23014 = _8672;
                    }
                    vec2 _8699 = (abs(_8472) - _8439) + vec2(_23014);
                    vec2 _8701 = max(_8699, vec2(0.0));
                    float _23016;
                    SPIRV_CROSS_BRANCH
                    if ((_8701.x > 0.0) && (_8701.y > 0.0))
                    {
                        float _23017;
                        if ((_23015 > 0.001000000047497451305389404296875) && (_23014 > 9.9999997473787516355514526367188e-05))
                        {
                            float _8718 = 2.0 + (2.0 * _23015);
                            vec2 _8723 = _8701 / vec2(max(_23014, 9.9999997473787516355514526367188e-05));
                            _23017 = pow(pow(_8723.x, _8718) + pow(_8723.y, _8718), 1.0 / _8718) * _23014;
                        }
                        else
                        {
                            _23017 = length(_8701);
                        }
                        _23016 = _23017;
                    }
                    else
                    {
                        _23016 = max(_8701.x, _8701.y);
                    }
                    float _8758 = (min(max(_8699.x, _8699.y), 0.0) + _23016) - _23014;
                    float _23032;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _8500 = max((_22847.zw - _22847.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _8503 = _7327 - ((_22847.xy + _22847.zw) * 0.5);
                        float _8794 = min(_8500.x, _8500.y);
                        float _8797 = min((_8503.x > 0.0) ? ((_8503.y > 0.0) ? _6878.x : _6878.x) : ((_8503.y > 0.0) ? _6878.x : _6878.x), _8794);
                        float _8803 = _8797 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _23020;
                        float _23021;
                        if (_8803 > _8794)
                        {
                            float _8817 = esia_v5.y * clamp((_8794 - _8797) / max(0.60000002384185791015625 * _8797, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _23021 = _8817;
                            _23020 = _8797 * (1.0 + (0.60000002384185791015625 * _8817));
                        }
                        else
                        {
                            _23021 = esia_v5.y;
                            _23020 = _8803;
                        }
                        vec2 _8830 = (abs(_8503) - _8500) + vec2(_23020);
                        vec2 _8832 = max(_8830, vec2(0.0));
                        float _23022;
                        SPIRV_CROSS_BRANCH
                        if ((_8832.x > 0.0) && (_8832.y > 0.0))
                        {
                            float _23023;
                            if ((_23021 > 0.001000000047497451305389404296875) && (_23020 > 9.9999997473787516355514526367188e-05))
                            {
                                float _8849 = 2.0 + (2.0 * _23021);
                                vec2 _8854 = _8832 / vec2(max(_23020, 9.9999997473787516355514526367188e-05));
                                _23023 = pow(pow(_8854.x, _8849) + pow(_8854.y, _8849), 1.0 / _8849) * _23020;
                            }
                            else
                            {
                                _23023 = length(_8832);
                            }
                            _23022 = _23023;
                        }
                        else
                        {
                            _23022 = max(_8832.x, _8832.y);
                        }
                        float _8889 = (min(max(_8830.x, _8830.y), 0.0) + _23022) - _23020;
                        float _8894 = max(_6878.y, 9.9999997473787516355514526367188e-05);
                        float _8903 = max(_8894 - abs(_8758 - _8889), 0.0) / _8894;
                        _23032 = min(_8758, _8889) - (((_8903 * _8903) * _8894) * 0.25);
                    }
                    else
                    {
                        _23032 = _8758;
                    }
                    _23031 = _23032;
                }
                _23030 = _23031;
            }
            vec2 _7331 = esia_v0 - vec2(0.0, 0.5);
            vec2 _8952 = (esia_v2.xy + esia_v2.zw) * 0.5;
            vec2 _8961 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
            float _23049;
            SPIRV_CROSS_BRANCH
            if (esia_v7.z == 1u)
            {
                vec2 _8967 = _7331 - _8952;
                float _23048;
                do
                {
                    if (esia_v5.w >= 6.282185077667236328125)
                    {
                        _23048 = abs(length(_8967) - esia_v3.x) - esia_v3.y;
                        break;
                    }
                    float _9066 = esia_v5.z + (esia_v5.w * 0.5);
                    float _9068 = cos(_9066);
                    float _9070 = sin(_9066);
                    float _9079 = dot(_8967, vec2(-_9070, _9068));
                    float _9082 = dot(_8967, vec2(_9068, _9070));
                    vec2 _9083 = vec2(_9079, _9082);
                    float _9086 = abs(_9079);
                    _9083.x = _9086;
                    float _9089 = esia_v5.w * 0.5;
                    float _9091 = sin(_9089);
                    float _9093 = cos(_9089);
                    _23048 = (((_9093 * _9086) > (_9091 * _9082)) ? length(_9083 - (vec2(_9091, _9093) * esia_v3.x)) : abs(length(_9083) - esia_v3.x)) - esia_v3.y;
                    break;
                } while(false);
                _23049 = _23048;
            }
            else
            {
                float _23050;
                if (esia_v7.z == 2u)
                {
                    vec2 _9129 = _7331 - _22847.xy;
                    vec2 _9132 = _22847.zw - _22847.xy;
                    _23050 = length(_9129 - (_9132 * clamp(dot(_9129, _9132) / max(dot(_9132, _9132), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
                }
                else
                {
                    vec2 _8994 = _7331 - _8952;
                    float _9185 = min(_8961.x, _8961.y);
                    float _9188 = min((_8994.x > 0.0) ? ((_8994.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_8994.y > 0.0) ? esia_v3.w : esia_v3.x), _9185);
                    float _9194 = _9188 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _23033;
                    float _23034;
                    if (_9194 > _9185)
                    {
                        float _9208 = esia_v5.y * clamp((_9185 - _9188) / max(0.60000002384185791015625 * _9188, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _23034 = _9208;
                        _23033 = _9188 * (1.0 + (0.60000002384185791015625 * _9208));
                    }
                    else
                    {
                        _23034 = esia_v5.y;
                        _23033 = _9194;
                    }
                    vec2 _9221 = (abs(_8994) - _8961) + vec2(_23033);
                    vec2 _9223 = max(_9221, vec2(0.0));
                    float _23035;
                    SPIRV_CROSS_BRANCH
                    if ((_9223.x > 0.0) && (_9223.y > 0.0))
                    {
                        float _23036;
                        if ((_23034 > 0.001000000047497451305389404296875) && (_23033 > 9.9999997473787516355514526367188e-05))
                        {
                            float _9240 = 2.0 + (2.0 * _23034);
                            vec2 _9245 = _9223 / vec2(max(_23033, 9.9999997473787516355514526367188e-05));
                            _23036 = pow(pow(_9245.x, _9240) + pow(_9245.y, _9240), 1.0 / _9240) * _23033;
                        }
                        else
                        {
                            _23036 = length(_9223);
                        }
                        _23035 = _23036;
                    }
                    else
                    {
                        _23035 = max(_9223.x, _9223.y);
                    }
                    float _9280 = (min(max(_9221.x, _9221.y), 0.0) + _23035) - _23033;
                    float _23051;
                    SPIRV_CROSS_BRANCH
                    if ((esia_v7.x & 512u) != 0u)
                    {
                        vec2 _9022 = max((_22847.zw - _22847.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                        vec2 _9025 = _7331 - ((_22847.xy + _22847.zw) * 0.5);
                        float _9316 = min(_9022.x, _9022.y);
                        float _9319 = min((_9025.x > 0.0) ? ((_9025.y > 0.0) ? _6878.x : _6878.x) : ((_9025.y > 0.0) ? _6878.x : _6878.x), _9316);
                        float _9325 = _9319 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                        float _23039;
                        float _23040;
                        if (_9325 > _9316)
                        {
                            float _9339 = esia_v5.y * clamp((_9316 - _9319) / max(0.60000002384185791015625 * _9319, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                            _23040 = _9339;
                            _23039 = _9319 * (1.0 + (0.60000002384185791015625 * _9339));
                        }
                        else
                        {
                            _23040 = esia_v5.y;
                            _23039 = _9325;
                        }
                        vec2 _9352 = (abs(_9025) - _9022) + vec2(_23039);
                        vec2 _9354 = max(_9352, vec2(0.0));
                        float _23041;
                        SPIRV_CROSS_BRANCH
                        if ((_9354.x > 0.0) && (_9354.y > 0.0))
                        {
                            float _23042;
                            if ((_23040 > 0.001000000047497451305389404296875) && (_23039 > 9.9999997473787516355514526367188e-05))
                            {
                                float _9371 = 2.0 + (2.0 * _23040);
                                vec2 _9376 = _9354 / vec2(max(_23039, 9.9999997473787516355514526367188e-05));
                                _23042 = pow(pow(_9376.x, _9371) + pow(_9376.y, _9371), 1.0 / _9371) * _23039;
                            }
                            else
                            {
                                _23042 = length(_9354);
                            }
                            _23041 = _23042;
                        }
                        else
                        {
                            _23041 = max(_9354.x, _9354.y);
                        }
                        float _9411 = (min(max(_9352.x, _9352.y), 0.0) + _23041) - _23039;
                        float _9416 = max(_6878.y, 9.9999997473787516355514526367188e-05);
                        float _9425 = max(_9416 - abs(_9280 - _9411), 0.0) / _9416;
                        _23051 = min(_9280, _9411) - (((_9425 * _9425) * _9416) * 0.25);
                    }
                    else
                    {
                        _23051 = _9280;
                    }
                    _23050 = _23051;
                }
                _23049 = _23050;
            }
            vec2 _7337 = vec2(_22992 - _23011, _23030 - _23049);
            float _7339 = length(_7337);
            vec2 _7345 = _7337 / vec2(_7339);
            bvec2 _7346 = bvec2(_7339 > 9.9999997473787516355514526367188e-06);
            vec2 _7347 = vec2(_7346.x ? _7345.x : vec2(0.0, -1.0).x, _7346.y ? _7345.y : vec2(0.0, -1.0).y);
            _23914 = normalize(vec3(_7347 * min(_6966 / max(_6972, 0.001000000047497451305389404296875), 8.0), 1.0));
            _23052 = _7347;
        }
        else
        {
            _23914 = vec3(0.0, 0.0, 1.0);
            _23052 = vec2(0.0, -1.0);
        }
        vec2 _6998 = ((-_23052) * _6940) * (1.0 - _6972);
        float _7000 = _6878.z;
        vec2 _23053;
        SPIRV_CROSS_BRANCH
        if (_7000 > 0.0)
        {
            _23053 = (((esia_v2.xy + esia_v2.zw) * 0.5) - esia_v0) * (_7000 / (1.0 + _7000));
        }
        else
        {
            _23053 = vec2(0.0);
        }
        vec2 _7024 = _22844 * _172.gTarget.zw;
        vec2 _7031 = _172.gDisplay.zw * _172.gTarget.zw;
        vec2 _7036 = (_6998 + _23053) * _7031;
        vec2 _7039 = _6998 * _7031;
        vec3 _23658;
        float _23681;
        vec3 _24150;
        if (_172.gTime.z > 0.5)
        {
            vec3 _23661;
            SPIRV_CROSS_BRANCH
            if (((_6959 > 0.001000000047497451305389404296875) && (_6966 > 0.0)) && ((((_6940 * 0.300000011920928955078125) * _6959) * _172.gDisplay.z) > (_6938 * 0.3499999940395355224609375)))
            {
                float _7061 = 0.300000011920928955078125 * _6959;
                vec2 _7068 = (_7024 + _7036) - (_7039 * _7061);
                float _9450 = clamp(log2(max(_6938, 1.0)) - 1.0, 0.0, 5.0);
                int _9453 = int(floor(_9450));
                float _9457 = _9450 - float(_9453);
                vec4 _23128;
                SPIRV_CROSS_BRANCH
                if (_9453 <= 0)
                {
                    vec2 _23127;
                    if (_172.gConv.x > 0.5)
                    {
                        vec2 _21538 = _7068;
                        _21538.y = 1.0 - _7068.y;
                        _23127 = _21538;
                    }
                    else
                    {
                        _23127 = _7068;
                    }
                    _23128 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23127, 0.0);
                }
                else
                {
                    vec4 _23129;
                    if (_9453 == 1)
                    {
                        vec2 _9592 = (_7068 * _172.gLevel[1].xy) - vec2(0.5);
                        vec2 _9594 = floor(_9592);
                        vec2 _9597 = _9592 - _9594;
                        vec2 _9600 = _9597 * _9597;
                        vec2 _9603 = _9600 * _9597;
                        vec2 _9622 = (((_9603 * 3.0) - (_9600 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                        vec2 _9635 = _9603 * 0.16666667163372039794921875;
                        vec2 _9638 = (((((-_9603) + (_9600 * 3.0)) - (_9597 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9622;
                        vec2 _9641 = (((((_9603 * (-3.0)) + (_9600 * 3.0)) + (_9597 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9635;
                        vec2 _9651 = ((_9594 - vec2(0.5)) + (_9622 / _9638)) * _172.gLevel[1].zw;
                        vec2 _9661 = ((_9594 + vec2(1.5)) + (_9635 / _9641)) * _172.gLevel[1].zw;
                        vec2 _23123;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21543 = _9651;
                            _21543.y = 1.0 - _9651.y;
                            _23123 = _21543;
                        }
                        else
                        {
                            _23123 = _9651;
                        }
                        float _9681 = _9651.y;
                        vec2 _9682 = vec2(_9661.x, _9681);
                        vec2 _23124;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21549 = _9682;
                            _21549.y = 1.0 - _9681;
                            _23124 = _21549;
                        }
                        else
                        {
                            _23124 = _9682;
                        }
                        float _9698 = _9661.y;
                        vec2 _9699 = vec2(_9651.x, _9698);
                        vec2 _23125;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21556 = _9699;
                            _21556.y = 1.0 - _9698;
                            _23125 = _21556;
                        }
                        else
                        {
                            _23125 = _9699;
                        }
                        vec2 _23126;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21562 = _9661;
                            _21562.y = 1.0 - _9661.y;
                            _23126 = _21562;
                        }
                        else
                        {
                            _23126 = _9661;
                        }
                        _23129 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23123, 0.0) * _9638.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23124, 0.0) * _9641.x)) * _9638.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23125, 0.0) * _9638.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23126, 0.0) * _9641.x)) * _9641.y);
                    }
                    else
                    {
                        vec4 _23130;
                        if (_9453 == 2)
                        {
                            vec2 _9799 = (_7068 * _172.gLevel[2].xy) - vec2(0.5);
                            vec2 _9801 = floor(_9799);
                            vec2 _9804 = _9799 - _9801;
                            vec2 _9807 = _9804 * _9804;
                            vec2 _9810 = _9807 * _9804;
                            vec2 _9829 = (((_9810 * 3.0) - (_9807 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _9842 = _9810 * 0.16666667163372039794921875;
                            vec2 _9845 = (((((-_9810) + (_9807 * 3.0)) - (_9804 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9829;
                            vec2 _9848 = (((((_9810 * (-3.0)) + (_9807 * 3.0)) + (_9804 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _9842;
                            vec2 _9858 = ((_9801 - vec2(0.5)) + (_9829 / _9845)) * _172.gLevel[2].zw;
                            vec2 _9868 = ((_9801 + vec2(1.5)) + (_9842 / _9848)) * _172.gLevel[2].zw;
                            vec2 _23119;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21569 = _9858;
                                _21569.y = 1.0 - _9858.y;
                                _23119 = _21569;
                            }
                            else
                            {
                                _23119 = _9858;
                            }
                            float _9888 = _9858.y;
                            vec2 _9889 = vec2(_9868.x, _9888);
                            vec2 _23120;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21575 = _9889;
                                _21575.y = 1.0 - _9888;
                                _23120 = _21575;
                            }
                            else
                            {
                                _23120 = _9889;
                            }
                            float _9905 = _9868.y;
                            vec2 _9906 = vec2(_9858.x, _9905);
                            vec2 _23121;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21582 = _9906;
                                _21582.y = 1.0 - _9905;
                                _23121 = _21582;
                            }
                            else
                            {
                                _23121 = _9906;
                            }
                            vec2 _23122;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21588 = _9868;
                                _21588.y = 1.0 - _9868.y;
                                _23122 = _21588;
                            }
                            else
                            {
                                _23122 = _9868;
                            }
                            _23130 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23119, 0.0) * _9845.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23120, 0.0) * _9848.x)) * _9845.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23121, 0.0) * _9845.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23122, 0.0) * _9848.x)) * _9848.y);
                        }
                        else
                        {
                            vec4 _23131;
                            if (_9453 == 3)
                            {
                                vec2 _10006 = (_7068 * _172.gLevel[3].xy) - vec2(0.5);
                                vec2 _10008 = floor(_10006);
                                vec2 _10011 = _10006 - _10008;
                                vec2 _10014 = _10011 * _10011;
                                vec2 _10017 = _10014 * _10011;
                                vec2 _10036 = (((_10017 * 3.0) - (_10014 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _10049 = _10017 * 0.16666667163372039794921875;
                                vec2 _10052 = (((((-_10017) + (_10014 * 3.0)) - (_10011 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10036;
                                vec2 _10055 = (((((_10017 * (-3.0)) + (_10014 * 3.0)) + (_10011 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10049;
                                vec2 _10065 = ((_10008 - vec2(0.5)) + (_10036 / _10052)) * _172.gLevel[3].zw;
                                vec2 _10075 = ((_10008 + vec2(1.5)) + (_10049 / _10055)) * _172.gLevel[3].zw;
                                vec2 _23115;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21595 = _10065;
                                    _21595.y = 1.0 - _10065.y;
                                    _23115 = _21595;
                                }
                                else
                                {
                                    _23115 = _10065;
                                }
                                float _10095 = _10065.y;
                                vec2 _10096 = vec2(_10075.x, _10095);
                                vec2 _23116;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21601 = _10096;
                                    _21601.y = 1.0 - _10095;
                                    _23116 = _21601;
                                }
                                else
                                {
                                    _23116 = _10096;
                                }
                                float _10112 = _10075.y;
                                vec2 _10113 = vec2(_10065.x, _10112);
                                vec2 _23117;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21608 = _10113;
                                    _21608.y = 1.0 - _10112;
                                    _23117 = _21608;
                                }
                                else
                                {
                                    _23117 = _10113;
                                }
                                vec2 _23118;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21614 = _10075;
                                    _21614.y = 1.0 - _10075.y;
                                    _23118 = _21614;
                                }
                                else
                                {
                                    _23118 = _10075;
                                }
                                _23131 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23115, 0.0) * _10052.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23116, 0.0) * _10055.x)) * _10052.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23117, 0.0) * _10052.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23118, 0.0) * _10055.x)) * _10055.y);
                            }
                            else
                            {
                                vec4 _23132;
                                if (_9453 == 4)
                                {
                                    vec2 _10213 = (_7068 * _172.gLevel[4].xy) - vec2(0.5);
                                    vec2 _10215 = floor(_10213);
                                    vec2 _10218 = _10213 - _10215;
                                    vec2 _10221 = _10218 * _10218;
                                    vec2 _10224 = _10221 * _10218;
                                    vec2 _10243 = (((_10224 * 3.0) - (_10221 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _10256 = _10224 * 0.16666667163372039794921875;
                                    vec2 _10259 = (((((-_10224) + (_10221 * 3.0)) - (_10218 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10243;
                                    vec2 _10262 = (((((_10224 * (-3.0)) + (_10221 * 3.0)) + (_10218 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10256;
                                    vec2 _10272 = ((_10215 - vec2(0.5)) + (_10243 / _10259)) * _172.gLevel[4].zw;
                                    vec2 _10282 = ((_10215 + vec2(1.5)) + (_10256 / _10262)) * _172.gLevel[4].zw;
                                    vec2 _23111;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21621 = _10272;
                                        _21621.y = 1.0 - _10272.y;
                                        _23111 = _21621;
                                    }
                                    else
                                    {
                                        _23111 = _10272;
                                    }
                                    float _10302 = _10272.y;
                                    vec2 _10303 = vec2(_10282.x, _10302);
                                    vec2 _23112;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21627 = _10303;
                                        _21627.y = 1.0 - _10302;
                                        _23112 = _21627;
                                    }
                                    else
                                    {
                                        _23112 = _10303;
                                    }
                                    float _10319 = _10282.y;
                                    vec2 _10320 = vec2(_10272.x, _10319);
                                    vec2 _23113;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21634 = _10320;
                                        _21634.y = 1.0 - _10319;
                                        _23113 = _21634;
                                    }
                                    else
                                    {
                                        _23113 = _10320;
                                    }
                                    vec2 _23114;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21640 = _10282;
                                        _21640.y = 1.0 - _10282.y;
                                        _23114 = _21640;
                                    }
                                    else
                                    {
                                        _23114 = _10282;
                                    }
                                    _23132 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23111, 0.0) * _10259.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23112, 0.0) * _10262.x)) * _10259.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23113, 0.0) * _10259.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23114, 0.0) * _10262.x)) * _10262.y);
                                }
                                else
                                {
                                    vec2 _10420 = (_7068 * _172.gLevel[5].xy) - vec2(0.5);
                                    vec2 _10422 = floor(_10420);
                                    vec2 _10425 = _10420 - _10422;
                                    vec2 _10428 = _10425 * _10425;
                                    vec2 _10431 = _10428 * _10425;
                                    vec2 _10450 = (((_10431 * 3.0) - (_10428 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _10463 = _10431 * 0.16666667163372039794921875;
                                    vec2 _10466 = (((((-_10431) + (_10428 * 3.0)) - (_10425 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10450;
                                    vec2 _10469 = (((((_10431 * (-3.0)) + (_10428 * 3.0)) + (_10425 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10463;
                                    vec2 _10479 = ((_10422 - vec2(0.5)) + (_10450 / _10466)) * _172.gLevel[5].zw;
                                    vec2 _10489 = ((_10422 + vec2(1.5)) + (_10463 / _10469)) * _172.gLevel[5].zw;
                                    vec2 _23107;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21647 = _10479;
                                        _21647.y = 1.0 - _10479.y;
                                        _23107 = _21647;
                                    }
                                    else
                                    {
                                        _23107 = _10479;
                                    }
                                    float _10509 = _10479.y;
                                    vec2 _10510 = vec2(_10489.x, _10509);
                                    vec2 _23108;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21653 = _10510;
                                        _21653.y = 1.0 - _10509;
                                        _23108 = _21653;
                                    }
                                    else
                                    {
                                        _23108 = _10510;
                                    }
                                    float _10526 = _10489.y;
                                    vec2 _10527 = vec2(_10479.x, _10526);
                                    vec2 _23109;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21660 = _10527;
                                        _21660.y = 1.0 - _10526;
                                        _23109 = _21660;
                                    }
                                    else
                                    {
                                        _23109 = _10527;
                                    }
                                    vec2 _23110;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21666 = _10489;
                                        _21666.y = 1.0 - _10489.y;
                                        _23110 = _21666;
                                    }
                                    else
                                    {
                                        _23110 = _10489;
                                    }
                                    _23132 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23107, 0.0) * _10466.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23108, 0.0) * _10469.x)) * _10466.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23109, 0.0) * _10466.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23110, 0.0) * _10469.x)) * _10469.y);
                                }
                                _23131 = _23132;
                            }
                            _23130 = _23131;
                        }
                        _23129 = _23130;
                    }
                    _23128 = _23129;
                }
                vec3 _23159;
                SPIRV_CROSS_BRANCH
                if ((_9457 > 0.0199999995529651641845703125) && (_9453 < 5))
                {
                    int _9470 = _9453 + 1;
                    vec4 _23154;
                    SPIRV_CROSS_BRANCH
                    if (_9470 <= 0)
                    {
                        vec2 _23153;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21671 = _7068;
                            _21671.y = 1.0 - _7068.y;
                            _23153 = _21671;
                        }
                        else
                        {
                            _23153 = _7068;
                        }
                        _23154 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23153, 0.0);
                    }
                    else
                    {
                        vec4 _23155;
                        if (_9470 == 1)
                        {
                            vec2 _10716 = (_7068 * _172.gLevel[1].xy) - vec2(0.5);
                            vec2 _10718 = floor(_10716);
                            vec2 _10721 = _10716 - _10718;
                            vec2 _10724 = _10721 * _10721;
                            vec2 _10727 = _10724 * _10721;
                            vec2 _10746 = (((_10727 * 3.0) - (_10724 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _10759 = _10727 * 0.16666667163372039794921875;
                            vec2 _10762 = (((((-_10727) + (_10724 * 3.0)) - (_10721 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10746;
                            vec2 _10765 = (((((_10727 * (-3.0)) + (_10724 * 3.0)) + (_10721 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10759;
                            vec2 _10775 = ((_10718 - vec2(0.5)) + (_10746 / _10762)) * _172.gLevel[1].zw;
                            vec2 _10785 = ((_10718 + vec2(1.5)) + (_10759 / _10765)) * _172.gLevel[1].zw;
                            vec2 _23149;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21676 = _10775;
                                _21676.y = 1.0 - _10775.y;
                                _23149 = _21676;
                            }
                            else
                            {
                                _23149 = _10775;
                            }
                            float _10805 = _10775.y;
                            vec2 _10806 = vec2(_10785.x, _10805);
                            vec2 _23150;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21682 = _10806;
                                _21682.y = 1.0 - _10805;
                                _23150 = _21682;
                            }
                            else
                            {
                                _23150 = _10806;
                            }
                            float _10822 = _10785.y;
                            vec2 _10823 = vec2(_10775.x, _10822);
                            vec2 _23151;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21689 = _10823;
                                _21689.y = 1.0 - _10822;
                                _23151 = _21689;
                            }
                            else
                            {
                                _23151 = _10823;
                            }
                            vec2 _23152;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21695 = _10785;
                                _21695.y = 1.0 - _10785.y;
                                _23152 = _21695;
                            }
                            else
                            {
                                _23152 = _10785;
                            }
                            _23155 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23149, 0.0) * _10762.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23150, 0.0) * _10765.x)) * _10762.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23151, 0.0) * _10762.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23152, 0.0) * _10765.x)) * _10765.y);
                        }
                        else
                        {
                            vec4 _23156;
                            if (_9470 == 2)
                            {
                                vec2 _10923 = (_7068 * _172.gLevel[2].xy) - vec2(0.5);
                                vec2 _10925 = floor(_10923);
                                vec2 _10928 = _10923 - _10925;
                                vec2 _10931 = _10928 * _10928;
                                vec2 _10934 = _10931 * _10928;
                                vec2 _10953 = (((_10934 * 3.0) - (_10931 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _10966 = _10934 * 0.16666667163372039794921875;
                                vec2 _10969 = (((((-_10934) + (_10931 * 3.0)) - (_10928 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10953;
                                vec2 _10972 = (((((_10934 * (-3.0)) + (_10931 * 3.0)) + (_10928 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _10966;
                                vec2 _10982 = ((_10925 - vec2(0.5)) + (_10953 / _10969)) * _172.gLevel[2].zw;
                                vec2 _10992 = ((_10925 + vec2(1.5)) + (_10966 / _10972)) * _172.gLevel[2].zw;
                                vec2 _23145;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21702 = _10982;
                                    _21702.y = 1.0 - _10982.y;
                                    _23145 = _21702;
                                }
                                else
                                {
                                    _23145 = _10982;
                                }
                                float _11012 = _10982.y;
                                vec2 _11013 = vec2(_10992.x, _11012);
                                vec2 _23146;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21708 = _11013;
                                    _21708.y = 1.0 - _11012;
                                    _23146 = _21708;
                                }
                                else
                                {
                                    _23146 = _11013;
                                }
                                float _11029 = _10992.y;
                                vec2 _11030 = vec2(_10982.x, _11029);
                                vec2 _23147;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21715 = _11030;
                                    _21715.y = 1.0 - _11029;
                                    _23147 = _21715;
                                }
                                else
                                {
                                    _23147 = _11030;
                                }
                                vec2 _23148;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21721 = _10992;
                                    _21721.y = 1.0 - _10992.y;
                                    _23148 = _21721;
                                }
                                else
                                {
                                    _23148 = _10992;
                                }
                                _23156 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23145, 0.0) * _10969.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23146, 0.0) * _10972.x)) * _10969.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23147, 0.0) * _10969.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23148, 0.0) * _10972.x)) * _10972.y);
                            }
                            else
                            {
                                vec4 _23157;
                                if (_9470 == 3)
                                {
                                    vec2 _11130 = (_7068 * _172.gLevel[3].xy) - vec2(0.5);
                                    vec2 _11132 = floor(_11130);
                                    vec2 _11135 = _11130 - _11132;
                                    vec2 _11138 = _11135 * _11135;
                                    vec2 _11141 = _11138 * _11135;
                                    vec2 _11160 = (((_11141 * 3.0) - (_11138 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _11173 = _11141 * 0.16666667163372039794921875;
                                    vec2 _11176 = (((((-_11141) + (_11138 * 3.0)) - (_11135 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11160;
                                    vec2 _11179 = (((((_11141 * (-3.0)) + (_11138 * 3.0)) + (_11135 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11173;
                                    vec2 _11189 = ((_11132 - vec2(0.5)) + (_11160 / _11176)) * _172.gLevel[3].zw;
                                    vec2 _11199 = ((_11132 + vec2(1.5)) + (_11173 / _11179)) * _172.gLevel[3].zw;
                                    vec2 _23141;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21728 = _11189;
                                        _21728.y = 1.0 - _11189.y;
                                        _23141 = _21728;
                                    }
                                    else
                                    {
                                        _23141 = _11189;
                                    }
                                    float _11219 = _11189.y;
                                    vec2 _11220 = vec2(_11199.x, _11219);
                                    vec2 _23142;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21734 = _11220;
                                        _21734.y = 1.0 - _11219;
                                        _23142 = _21734;
                                    }
                                    else
                                    {
                                        _23142 = _11220;
                                    }
                                    float _11236 = _11199.y;
                                    vec2 _11237 = vec2(_11189.x, _11236);
                                    vec2 _23143;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21741 = _11237;
                                        _21741.y = 1.0 - _11236;
                                        _23143 = _21741;
                                    }
                                    else
                                    {
                                        _23143 = _11237;
                                    }
                                    vec2 _23144;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21747 = _11199;
                                        _21747.y = 1.0 - _11199.y;
                                        _23144 = _21747;
                                    }
                                    else
                                    {
                                        _23144 = _11199;
                                    }
                                    _23157 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23141, 0.0) * _11176.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23142, 0.0) * _11179.x)) * _11176.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23143, 0.0) * _11176.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23144, 0.0) * _11179.x)) * _11179.y);
                                }
                                else
                                {
                                    vec4 _23158;
                                    if (_9470 == 4)
                                    {
                                        vec2 _11337 = (_7068 * _172.gLevel[4].xy) - vec2(0.5);
                                        vec2 _11339 = floor(_11337);
                                        vec2 _11342 = _11337 - _11339;
                                        vec2 _11345 = _11342 * _11342;
                                        vec2 _11348 = _11345 * _11342;
                                        vec2 _11367 = (((_11348 * 3.0) - (_11345 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _11380 = _11348 * 0.16666667163372039794921875;
                                        vec2 _11383 = (((((-_11348) + (_11345 * 3.0)) - (_11342 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11367;
                                        vec2 _11386 = (((((_11348 * (-3.0)) + (_11345 * 3.0)) + (_11342 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11380;
                                        vec2 _11396 = ((_11339 - vec2(0.5)) + (_11367 / _11383)) * _172.gLevel[4].zw;
                                        vec2 _11406 = ((_11339 + vec2(1.5)) + (_11380 / _11386)) * _172.gLevel[4].zw;
                                        vec2 _23137;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _21754 = _11396;
                                            _21754.y = 1.0 - _11396.y;
                                            _23137 = _21754;
                                        }
                                        else
                                        {
                                            _23137 = _11396;
                                        }
                                        float _11426 = _11396.y;
                                        vec2 _11427 = vec2(_11406.x, _11426);
                                        vec2 _23138;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _21760 = _11427;
                                            _21760.y = 1.0 - _11426;
                                            _23138 = _21760;
                                        }
                                        else
                                        {
                                            _23138 = _11427;
                                        }
                                        float _11443 = _11406.y;
                                        vec2 _11444 = vec2(_11396.x, _11443);
                                        vec2 _23139;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _21767 = _11444;
                                            _21767.y = 1.0 - _11443;
                                            _23139 = _21767;
                                        }
                                        else
                                        {
                                            _23139 = _11444;
                                        }
                                        vec2 _23140;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _21773 = _11406;
                                            _21773.y = 1.0 - _11406.y;
                                            _23140 = _21773;
                                        }
                                        else
                                        {
                                            _23140 = _11406;
                                        }
                                        _23158 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23137, 0.0) * _11383.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23138, 0.0) * _11386.x)) * _11383.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23139, 0.0) * _11383.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23140, 0.0) * _11386.x)) * _11386.y);
                                    }
                                    else
                                    {
                                        vec2 _11544 = (_7068 * _172.gLevel[5].xy) - vec2(0.5);
                                        vec2 _11546 = floor(_11544);
                                        vec2 _11549 = _11544 - _11546;
                                        vec2 _11552 = _11549 * _11549;
                                        vec2 _11555 = _11552 * _11549;
                                        vec2 _11574 = (((_11555 * 3.0) - (_11552 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _11587 = _11555 * 0.16666667163372039794921875;
                                        vec2 _11590 = (((((-_11555) + (_11552 * 3.0)) - (_11549 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11574;
                                        vec2 _11593 = (((((_11555 * (-3.0)) + (_11552 * 3.0)) + (_11549 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11587;
                                        vec2 _11603 = ((_11546 - vec2(0.5)) + (_11574 / _11590)) * _172.gLevel[5].zw;
                                        vec2 _11613 = ((_11546 + vec2(1.5)) + (_11587 / _11593)) * _172.gLevel[5].zw;
                                        vec2 _23133;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _21780 = _11603;
                                            _21780.y = 1.0 - _11603.y;
                                            _23133 = _21780;
                                        }
                                        else
                                        {
                                            _23133 = _11603;
                                        }
                                        float _11633 = _11603.y;
                                        vec2 _11634 = vec2(_11613.x, _11633);
                                        vec2 _23134;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _21786 = _11634;
                                            _21786.y = 1.0 - _11633;
                                            _23134 = _21786;
                                        }
                                        else
                                        {
                                            _23134 = _11634;
                                        }
                                        float _11650 = _11613.y;
                                        vec2 _11651 = vec2(_11603.x, _11650);
                                        vec2 _23135;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _21793 = _11651;
                                            _21793.y = 1.0 - _11650;
                                            _23135 = _21793;
                                        }
                                        else
                                        {
                                            _23135 = _11651;
                                        }
                                        vec2 _23136;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _21799 = _11613;
                                            _21799.y = 1.0 - _11613.y;
                                            _23136 = _21799;
                                        }
                                        else
                                        {
                                            _23136 = _11613;
                                        }
                                        _23158 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23133, 0.0) * _11590.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23134, 0.0) * _11593.x)) * _11590.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23135, 0.0) * _11590.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23136, 0.0) * _11593.x)) * _11593.y);
                                    }
                                    _23157 = _23158;
                                }
                                _23156 = _23157;
                            }
                            _23155 = _23156;
                        }
                        _23154 = _23155;
                    }
                    _23159 = mix(_23128.xyz, _23154.xyz, vec3(_9457));
                }
                else
                {
                    _23159 = _23128.xyz;
                }
                vec2 _7075 = _7024 + _7036;
                float _11741 = clamp(log2(max(_6938, 1.0)) - 1.0, 0.0, 5.0);
                int _11744 = int(floor(_11741));
                float _11748 = _11741 - float(_11744);
                vec4 _23234;
                SPIRV_CROSS_BRANCH
                if (_11744 <= 0)
                {
                    vec2 _23233;
                    if (_172.gConv.x > 0.5)
                    {
                        vec2 _21806 = _7075;
                        _21806.y = 1.0 - _7075.y;
                        _23233 = _21806;
                    }
                    else
                    {
                        _23233 = _7075;
                    }
                    _23234 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23233, 0.0);
                }
                else
                {
                    vec4 _23235;
                    if (_11744 == 1)
                    {
                        vec2 _11883 = (_7075 * _172.gLevel[1].xy) - vec2(0.5);
                        vec2 _11885 = floor(_11883);
                        vec2 _11888 = _11883 - _11885;
                        vec2 _11891 = _11888 * _11888;
                        vec2 _11894 = _11891 * _11888;
                        vec2 _11913 = (((_11894 * 3.0) - (_11891 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                        vec2 _11926 = _11894 * 0.16666667163372039794921875;
                        vec2 _11929 = (((((-_11894) + (_11891 * 3.0)) - (_11888 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11913;
                        vec2 _11932 = (((((_11894 * (-3.0)) + (_11891 * 3.0)) + (_11888 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _11926;
                        vec2 _11942 = ((_11885 - vec2(0.5)) + (_11913 / _11929)) * _172.gLevel[1].zw;
                        vec2 _11952 = ((_11885 + vec2(1.5)) + (_11926 / _11932)) * _172.gLevel[1].zw;
                        vec2 _23229;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21811 = _11942;
                            _21811.y = 1.0 - _11942.y;
                            _23229 = _21811;
                        }
                        else
                        {
                            _23229 = _11942;
                        }
                        vec2 _11973 = vec2(_11952.x, _11942.y);
                        vec2 _23230;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21817 = _11973;
                            _21817.y = 1.0 - _11942.y;
                            _23230 = _21817;
                        }
                        else
                        {
                            _23230 = _11973;
                        }
                        float _11989 = _11952.y;
                        vec2 _11990 = vec2(_11942.x, _11989);
                        vec2 _23231;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21824 = _11990;
                            _21824.y = 1.0 - _11989;
                            _23231 = _21824;
                        }
                        else
                        {
                            _23231 = _11990;
                        }
                        vec2 _23232;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21830 = _11952;
                            _21830.y = 1.0 - _11952.y;
                            _23232 = _21830;
                        }
                        else
                        {
                            _23232 = _11952;
                        }
                        _23235 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23229, 0.0) * _11929.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23230, 0.0) * _11932.x)) * _11929.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23231, 0.0) * _11929.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23232, 0.0) * _11932.x)) * _11932.y);
                    }
                    else
                    {
                        vec4 _23236;
                        if (_11744 == 2)
                        {
                            vec2 _12090 = (_7075 * _172.gLevel[2].xy) - vec2(0.5);
                            vec2 _12092 = floor(_12090);
                            vec2 _12095 = _12090 - _12092;
                            vec2 _12098 = _12095 * _12095;
                            vec2 _12101 = _12098 * _12095;
                            vec2 _12120 = (((_12101 * 3.0) - (_12098 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _12133 = _12101 * 0.16666667163372039794921875;
                            vec2 _12136 = (((((-_12101) + (_12098 * 3.0)) - (_12095 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12120;
                            vec2 _12139 = (((((_12101 * (-3.0)) + (_12098 * 3.0)) + (_12095 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12133;
                            vec2 _12149 = ((_12092 - vec2(0.5)) + (_12120 / _12136)) * _172.gLevel[2].zw;
                            vec2 _12159 = ((_12092 + vec2(1.5)) + (_12133 / _12139)) * _172.gLevel[2].zw;
                            vec2 _23225;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21837 = _12149;
                                _21837.y = 1.0 - _12149.y;
                                _23225 = _21837;
                            }
                            else
                            {
                                _23225 = _12149;
                            }
                            vec2 _12180 = vec2(_12159.x, _12149.y);
                            vec2 _23226;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21843 = _12180;
                                _21843.y = 1.0 - _12149.y;
                                _23226 = _21843;
                            }
                            else
                            {
                                _23226 = _12180;
                            }
                            float _12196 = _12159.y;
                            vec2 _12197 = vec2(_12149.x, _12196);
                            vec2 _23227;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21850 = _12197;
                                _21850.y = 1.0 - _12196;
                                _23227 = _21850;
                            }
                            else
                            {
                                _23227 = _12197;
                            }
                            vec2 _23228;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21856 = _12159;
                                _21856.y = 1.0 - _12159.y;
                                _23228 = _21856;
                            }
                            else
                            {
                                _23228 = _12159;
                            }
                            _23236 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23225, 0.0) * _12136.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23226, 0.0) * _12139.x)) * _12136.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23227, 0.0) * _12136.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23228, 0.0) * _12139.x)) * _12139.y);
                        }
                        else
                        {
                            vec4 _23237;
                            if (_11744 == 3)
                            {
                                vec2 _12297 = (_7075 * _172.gLevel[3].xy) - vec2(0.5);
                                vec2 _12299 = floor(_12297);
                                vec2 _12302 = _12297 - _12299;
                                vec2 _12305 = _12302 * _12302;
                                vec2 _12308 = _12305 * _12302;
                                vec2 _12327 = (((_12308 * 3.0) - (_12305 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _12340 = _12308 * 0.16666667163372039794921875;
                                vec2 _12343 = (((((-_12308) + (_12305 * 3.0)) - (_12302 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12327;
                                vec2 _12346 = (((((_12308 * (-3.0)) + (_12305 * 3.0)) + (_12302 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12340;
                                vec2 _12356 = ((_12299 - vec2(0.5)) + (_12327 / _12343)) * _172.gLevel[3].zw;
                                vec2 _12366 = ((_12299 + vec2(1.5)) + (_12340 / _12346)) * _172.gLevel[3].zw;
                                vec2 _23221;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21863 = _12356;
                                    _21863.y = 1.0 - _12356.y;
                                    _23221 = _21863;
                                }
                                else
                                {
                                    _23221 = _12356;
                                }
                                vec2 _12387 = vec2(_12366.x, _12356.y);
                                vec2 _23222;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21869 = _12387;
                                    _21869.y = 1.0 - _12356.y;
                                    _23222 = _21869;
                                }
                                else
                                {
                                    _23222 = _12387;
                                }
                                float _12403 = _12366.y;
                                vec2 _12404 = vec2(_12356.x, _12403);
                                vec2 _23223;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21876 = _12404;
                                    _21876.y = 1.0 - _12403;
                                    _23223 = _21876;
                                }
                                else
                                {
                                    _23223 = _12404;
                                }
                                vec2 _23224;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21882 = _12366;
                                    _21882.y = 1.0 - _12366.y;
                                    _23224 = _21882;
                                }
                                else
                                {
                                    _23224 = _12366;
                                }
                                _23237 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23221, 0.0) * _12343.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23222, 0.0) * _12346.x)) * _12343.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23223, 0.0) * _12343.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23224, 0.0) * _12346.x)) * _12346.y);
                            }
                            else
                            {
                                vec4 _23238;
                                if (_11744 == 4)
                                {
                                    vec2 _12504 = (_7075 * _172.gLevel[4].xy) - vec2(0.5);
                                    vec2 _12506 = floor(_12504);
                                    vec2 _12509 = _12504 - _12506;
                                    vec2 _12512 = _12509 * _12509;
                                    vec2 _12515 = _12512 * _12509;
                                    vec2 _12534 = (((_12515 * 3.0) - (_12512 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _12547 = _12515 * 0.16666667163372039794921875;
                                    vec2 _12550 = (((((-_12515) + (_12512 * 3.0)) - (_12509 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12534;
                                    vec2 _12553 = (((((_12515 * (-3.0)) + (_12512 * 3.0)) + (_12509 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12547;
                                    vec2 _12563 = ((_12506 - vec2(0.5)) + (_12534 / _12550)) * _172.gLevel[4].zw;
                                    vec2 _12573 = ((_12506 + vec2(1.5)) + (_12547 / _12553)) * _172.gLevel[4].zw;
                                    vec2 _23217;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21889 = _12563;
                                        _21889.y = 1.0 - _12563.y;
                                        _23217 = _21889;
                                    }
                                    else
                                    {
                                        _23217 = _12563;
                                    }
                                    vec2 _12594 = vec2(_12573.x, _12563.y);
                                    vec2 _23218;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21895 = _12594;
                                        _21895.y = 1.0 - _12563.y;
                                        _23218 = _21895;
                                    }
                                    else
                                    {
                                        _23218 = _12594;
                                    }
                                    float _12610 = _12573.y;
                                    vec2 _12611 = vec2(_12563.x, _12610);
                                    vec2 _23219;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21902 = _12611;
                                        _21902.y = 1.0 - _12610;
                                        _23219 = _21902;
                                    }
                                    else
                                    {
                                        _23219 = _12611;
                                    }
                                    vec2 _23220;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21908 = _12573;
                                        _21908.y = 1.0 - _12573.y;
                                        _23220 = _21908;
                                    }
                                    else
                                    {
                                        _23220 = _12573;
                                    }
                                    _23238 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23217, 0.0) * _12550.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23218, 0.0) * _12553.x)) * _12550.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23219, 0.0) * _12550.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23220, 0.0) * _12553.x)) * _12553.y);
                                }
                                else
                                {
                                    vec2 _12711 = (_7075 * _172.gLevel[5].xy) - vec2(0.5);
                                    vec2 _12713 = floor(_12711);
                                    vec2 _12716 = _12711 - _12713;
                                    vec2 _12719 = _12716 * _12716;
                                    vec2 _12722 = _12719 * _12716;
                                    vec2 _12741 = (((_12722 * 3.0) - (_12719 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _12754 = _12722 * 0.16666667163372039794921875;
                                    vec2 _12757 = (((((-_12722) + (_12719 * 3.0)) - (_12716 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12741;
                                    vec2 _12760 = (((((_12722 * (-3.0)) + (_12719 * 3.0)) + (_12716 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _12754;
                                    vec2 _12770 = ((_12713 - vec2(0.5)) + (_12741 / _12757)) * _172.gLevel[5].zw;
                                    vec2 _12780 = ((_12713 + vec2(1.5)) + (_12754 / _12760)) * _172.gLevel[5].zw;
                                    vec2 _23213;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21915 = _12770;
                                        _21915.y = 1.0 - _12770.y;
                                        _23213 = _21915;
                                    }
                                    else
                                    {
                                        _23213 = _12770;
                                    }
                                    vec2 _12801 = vec2(_12780.x, _12770.y);
                                    vec2 _23214;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21921 = _12801;
                                        _21921.y = 1.0 - _12770.y;
                                        _23214 = _21921;
                                    }
                                    else
                                    {
                                        _23214 = _12801;
                                    }
                                    float _12817 = _12780.y;
                                    vec2 _12818 = vec2(_12770.x, _12817);
                                    vec2 _23215;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21928 = _12818;
                                        _21928.y = 1.0 - _12817;
                                        _23215 = _21928;
                                    }
                                    else
                                    {
                                        _23215 = _12818;
                                    }
                                    vec2 _23216;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21934 = _12780;
                                        _21934.y = 1.0 - _12780.y;
                                        _23216 = _21934;
                                    }
                                    else
                                    {
                                        _23216 = _12780;
                                    }
                                    _23238 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23213, 0.0) * _12757.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23214, 0.0) * _12760.x)) * _12757.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23215, 0.0) * _12757.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23216, 0.0) * _12760.x)) * _12760.y);
                                }
                                _23237 = _23238;
                            }
                            _23236 = _23237;
                        }
                        _23235 = _23236;
                    }
                    _23234 = _23235;
                }
                vec3 _23265;
                SPIRV_CROSS_BRANCH
                if ((_11748 > 0.0199999995529651641845703125) && (_11744 < 5))
                {
                    int _11761 = _11744 + 1;
                    vec4 _23260;
                    SPIRV_CROSS_BRANCH
                    if (_11761 <= 0)
                    {
                        vec2 _23259;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _21939 = _7075;
                            _21939.y = 1.0 - _7075.y;
                            _23259 = _21939;
                        }
                        else
                        {
                            _23259 = _7075;
                        }
                        _23260 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23259, 0.0);
                    }
                    else
                    {
                        vec4 _23261;
                        if (_11761 == 1)
                        {
                            vec2 _13007 = (_7075 * _172.gLevel[1].xy) - vec2(0.5);
                            vec2 _13009 = floor(_13007);
                            vec2 _13012 = _13007 - _13009;
                            vec2 _13015 = _13012 * _13012;
                            vec2 _13018 = _13015 * _13012;
                            vec2 _13037 = (((_13018 * 3.0) - (_13015 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _13050 = _13018 * 0.16666667163372039794921875;
                            vec2 _13053 = (((((-_13018) + (_13015 * 3.0)) - (_13012 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13037;
                            vec2 _13056 = (((((_13018 * (-3.0)) + (_13015 * 3.0)) + (_13012 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13050;
                            vec2 _13066 = ((_13009 - vec2(0.5)) + (_13037 / _13053)) * _172.gLevel[1].zw;
                            vec2 _13076 = ((_13009 + vec2(1.5)) + (_13050 / _13056)) * _172.gLevel[1].zw;
                            vec2 _23255;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21944 = _13066;
                                _21944.y = 1.0 - _13066.y;
                                _23255 = _21944;
                            }
                            else
                            {
                                _23255 = _13066;
                            }
                            vec2 _13097 = vec2(_13076.x, _13066.y);
                            vec2 _23256;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21950 = _13097;
                                _21950.y = 1.0 - _13066.y;
                                _23256 = _21950;
                            }
                            else
                            {
                                _23256 = _13097;
                            }
                            float _13113 = _13076.y;
                            vec2 _13114 = vec2(_13066.x, _13113);
                            vec2 _23257;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21957 = _13114;
                                _21957.y = 1.0 - _13113;
                                _23257 = _21957;
                            }
                            else
                            {
                                _23257 = _13114;
                            }
                            vec2 _23258;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _21963 = _13076;
                                _21963.y = 1.0 - _13076.y;
                                _23258 = _21963;
                            }
                            else
                            {
                                _23258 = _13076;
                            }
                            _23261 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23255, 0.0) * _13053.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23256, 0.0) * _13056.x)) * _13053.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23257, 0.0) * _13053.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23258, 0.0) * _13056.x)) * _13056.y);
                        }
                        else
                        {
                            vec4 _23262;
                            if (_11761 == 2)
                            {
                                vec2 _13214 = (_7075 * _172.gLevel[2].xy) - vec2(0.5);
                                vec2 _13216 = floor(_13214);
                                vec2 _13219 = _13214 - _13216;
                                vec2 _13222 = _13219 * _13219;
                                vec2 _13225 = _13222 * _13219;
                                vec2 _13244 = (((_13225 * 3.0) - (_13222 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _13257 = _13225 * 0.16666667163372039794921875;
                                vec2 _13260 = (((((-_13225) + (_13222 * 3.0)) - (_13219 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13244;
                                vec2 _13263 = (((((_13225 * (-3.0)) + (_13222 * 3.0)) + (_13219 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13257;
                                vec2 _13273 = ((_13216 - vec2(0.5)) + (_13244 / _13260)) * _172.gLevel[2].zw;
                                vec2 _13283 = ((_13216 + vec2(1.5)) + (_13257 / _13263)) * _172.gLevel[2].zw;
                                vec2 _23251;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21970 = _13273;
                                    _21970.y = 1.0 - _13273.y;
                                    _23251 = _21970;
                                }
                                else
                                {
                                    _23251 = _13273;
                                }
                                vec2 _13304 = vec2(_13283.x, _13273.y);
                                vec2 _23252;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21976 = _13304;
                                    _21976.y = 1.0 - _13273.y;
                                    _23252 = _21976;
                                }
                                else
                                {
                                    _23252 = _13304;
                                }
                                float _13320 = _13283.y;
                                vec2 _13321 = vec2(_13273.x, _13320);
                                vec2 _23253;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21983 = _13321;
                                    _21983.y = 1.0 - _13320;
                                    _23253 = _21983;
                                }
                                else
                                {
                                    _23253 = _13321;
                                }
                                vec2 _23254;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _21989 = _13283;
                                    _21989.y = 1.0 - _13283.y;
                                    _23254 = _21989;
                                }
                                else
                                {
                                    _23254 = _13283;
                                }
                                _23262 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23251, 0.0) * _13260.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23252, 0.0) * _13263.x)) * _13260.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23253, 0.0) * _13260.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23254, 0.0) * _13263.x)) * _13263.y);
                            }
                            else
                            {
                                vec4 _23263;
                                if (_11761 == 3)
                                {
                                    vec2 _13421 = (_7075 * _172.gLevel[3].xy) - vec2(0.5);
                                    vec2 _13423 = floor(_13421);
                                    vec2 _13426 = _13421 - _13423;
                                    vec2 _13429 = _13426 * _13426;
                                    vec2 _13432 = _13429 * _13426;
                                    vec2 _13451 = (((_13432 * 3.0) - (_13429 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _13464 = _13432 * 0.16666667163372039794921875;
                                    vec2 _13467 = (((((-_13432) + (_13429 * 3.0)) - (_13426 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13451;
                                    vec2 _13470 = (((((_13432 * (-3.0)) + (_13429 * 3.0)) + (_13426 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13464;
                                    vec2 _13480 = ((_13423 - vec2(0.5)) + (_13451 / _13467)) * _172.gLevel[3].zw;
                                    vec2 _13490 = ((_13423 + vec2(1.5)) + (_13464 / _13470)) * _172.gLevel[3].zw;
                                    vec2 _23247;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _21996 = _13480;
                                        _21996.y = 1.0 - _13480.y;
                                        _23247 = _21996;
                                    }
                                    else
                                    {
                                        _23247 = _13480;
                                    }
                                    vec2 _13511 = vec2(_13490.x, _13480.y);
                                    vec2 _23248;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22002 = _13511;
                                        _22002.y = 1.0 - _13480.y;
                                        _23248 = _22002;
                                    }
                                    else
                                    {
                                        _23248 = _13511;
                                    }
                                    float _13527 = _13490.y;
                                    vec2 _13528 = vec2(_13480.x, _13527);
                                    vec2 _23249;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22009 = _13528;
                                        _22009.y = 1.0 - _13527;
                                        _23249 = _22009;
                                    }
                                    else
                                    {
                                        _23249 = _13528;
                                    }
                                    vec2 _23250;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22015 = _13490;
                                        _22015.y = 1.0 - _13490.y;
                                        _23250 = _22015;
                                    }
                                    else
                                    {
                                        _23250 = _13490;
                                    }
                                    _23263 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23247, 0.0) * _13467.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23248, 0.0) * _13470.x)) * _13467.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23249, 0.0) * _13467.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23250, 0.0) * _13470.x)) * _13470.y);
                                }
                                else
                                {
                                    vec4 _23264;
                                    if (_11761 == 4)
                                    {
                                        vec2 _13628 = (_7075 * _172.gLevel[4].xy) - vec2(0.5);
                                        vec2 _13630 = floor(_13628);
                                        vec2 _13633 = _13628 - _13630;
                                        vec2 _13636 = _13633 * _13633;
                                        vec2 _13639 = _13636 * _13633;
                                        vec2 _13658 = (((_13639 * 3.0) - (_13636 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _13671 = _13639 * 0.16666667163372039794921875;
                                        vec2 _13674 = (((((-_13639) + (_13636 * 3.0)) - (_13633 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13658;
                                        vec2 _13677 = (((((_13639 * (-3.0)) + (_13636 * 3.0)) + (_13633 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13671;
                                        vec2 _13687 = ((_13630 - vec2(0.5)) + (_13658 / _13674)) * _172.gLevel[4].zw;
                                        vec2 _13697 = ((_13630 + vec2(1.5)) + (_13671 / _13677)) * _172.gLevel[4].zw;
                                        vec2 _23243;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22022 = _13687;
                                            _22022.y = 1.0 - _13687.y;
                                            _23243 = _22022;
                                        }
                                        else
                                        {
                                            _23243 = _13687;
                                        }
                                        vec2 _13718 = vec2(_13697.x, _13687.y);
                                        vec2 _23244;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22028 = _13718;
                                            _22028.y = 1.0 - _13687.y;
                                            _23244 = _22028;
                                        }
                                        else
                                        {
                                            _23244 = _13718;
                                        }
                                        float _13734 = _13697.y;
                                        vec2 _13735 = vec2(_13687.x, _13734);
                                        vec2 _23245;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22035 = _13735;
                                            _22035.y = 1.0 - _13734;
                                            _23245 = _22035;
                                        }
                                        else
                                        {
                                            _23245 = _13735;
                                        }
                                        vec2 _23246;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22041 = _13697;
                                            _22041.y = 1.0 - _13697.y;
                                            _23246 = _22041;
                                        }
                                        else
                                        {
                                            _23246 = _13697;
                                        }
                                        _23264 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23243, 0.0) * _13674.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23244, 0.0) * _13677.x)) * _13674.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23245, 0.0) * _13674.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23246, 0.0) * _13677.x)) * _13677.y);
                                    }
                                    else
                                    {
                                        vec2 _13835 = (_7075 * _172.gLevel[5].xy) - vec2(0.5);
                                        vec2 _13837 = floor(_13835);
                                        vec2 _13840 = _13835 - _13837;
                                        vec2 _13843 = _13840 * _13840;
                                        vec2 _13846 = _13843 * _13840;
                                        vec2 _13865 = (((_13846 * 3.0) - (_13843 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _13878 = _13846 * 0.16666667163372039794921875;
                                        vec2 _13881 = (((((-_13846) + (_13843 * 3.0)) - (_13840 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13865;
                                        vec2 _13884 = (((((_13846 * (-3.0)) + (_13843 * 3.0)) + (_13840 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _13878;
                                        vec2 _13894 = ((_13837 - vec2(0.5)) + (_13865 / _13881)) * _172.gLevel[5].zw;
                                        vec2 _13904 = ((_13837 + vec2(1.5)) + (_13878 / _13884)) * _172.gLevel[5].zw;
                                        vec2 _23239;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22048 = _13894;
                                            _22048.y = 1.0 - _13894.y;
                                            _23239 = _22048;
                                        }
                                        else
                                        {
                                            _23239 = _13894;
                                        }
                                        vec2 _13925 = vec2(_13904.x, _13894.y);
                                        vec2 _23240;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22054 = _13925;
                                            _22054.y = 1.0 - _13894.y;
                                            _23240 = _22054;
                                        }
                                        else
                                        {
                                            _23240 = _13925;
                                        }
                                        float _13941 = _13904.y;
                                        vec2 _13942 = vec2(_13894.x, _13941);
                                        vec2 _23241;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22061 = _13942;
                                            _22061.y = 1.0 - _13941;
                                            _23241 = _22061;
                                        }
                                        else
                                        {
                                            _23241 = _13942;
                                        }
                                        vec2 _23242;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22067 = _13904;
                                            _22067.y = 1.0 - _13904.y;
                                            _23242 = _22067;
                                        }
                                        else
                                        {
                                            _23242 = _13904;
                                        }
                                        _23264 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23239, 0.0) * _13881.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23240, 0.0) * _13884.x)) * _13881.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23241, 0.0) * _13881.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23242, 0.0) * _13884.x)) * _13884.y);
                                    }
                                    _23263 = _23264;
                                }
                                _23262 = _23263;
                            }
                            _23261 = _23262;
                        }
                        _23260 = _23261;
                    }
                    _23265 = mix(_23234.xyz, _23260.xyz, vec3(_11748));
                }
                else
                {
                    _23265 = _23234.xyz;
                }
                vec2 _7086 = (_7024 + _7036) + (_7039 * _7061);
                float _14032 = clamp(log2(max(_6938, 1.0)) - 1.0, 0.0, 5.0);
                int _14035 = int(floor(_14032));
                float _14039 = _14032 - float(_14035);
                vec4 _23340;
                SPIRV_CROSS_BRANCH
                if (_14035 <= 0)
                {
                    vec2 _23339;
                    if (_172.gConv.x > 0.5)
                    {
                        vec2 _22074 = _7086;
                        _22074.y = 1.0 - _7086.y;
                        _23339 = _22074;
                    }
                    else
                    {
                        _23339 = _7086;
                    }
                    _23340 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23339, 0.0);
                }
                else
                {
                    vec4 _23341;
                    if (_14035 == 1)
                    {
                        vec2 _14174 = (_7086 * _172.gLevel[1].xy) - vec2(0.5);
                        vec2 _14176 = floor(_14174);
                        vec2 _14179 = _14174 - _14176;
                        vec2 _14182 = _14179 * _14179;
                        vec2 _14185 = _14182 * _14179;
                        vec2 _14204 = (((_14185 * 3.0) - (_14182 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                        vec2 _14217 = _14185 * 0.16666667163372039794921875;
                        vec2 _14220 = (((((-_14185) + (_14182 * 3.0)) - (_14179 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14204;
                        vec2 _14223 = (((((_14185 * (-3.0)) + (_14182 * 3.0)) + (_14179 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14217;
                        vec2 _14233 = ((_14176 - vec2(0.5)) + (_14204 / _14220)) * _172.gLevel[1].zw;
                        vec2 _14243 = ((_14176 + vec2(1.5)) + (_14217 / _14223)) * _172.gLevel[1].zw;
                        vec2 _23335;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22079 = _14233;
                            _22079.y = 1.0 - _14233.y;
                            _23335 = _22079;
                        }
                        else
                        {
                            _23335 = _14233;
                        }
                        float _14263 = _14233.y;
                        vec2 _14264 = vec2(_14243.x, _14263);
                        vec2 _23336;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22085 = _14264;
                            _22085.y = 1.0 - _14263;
                            _23336 = _22085;
                        }
                        else
                        {
                            _23336 = _14264;
                        }
                        float _14280 = _14243.y;
                        vec2 _14281 = vec2(_14233.x, _14280);
                        vec2 _23337;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22092 = _14281;
                            _22092.y = 1.0 - _14280;
                            _23337 = _22092;
                        }
                        else
                        {
                            _23337 = _14281;
                        }
                        vec2 _23338;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22098 = _14243;
                            _22098.y = 1.0 - _14243.y;
                            _23338 = _22098;
                        }
                        else
                        {
                            _23338 = _14243;
                        }
                        _23341 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23335, 0.0) * _14220.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23336, 0.0) * _14223.x)) * _14220.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23337, 0.0) * _14220.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23338, 0.0) * _14223.x)) * _14223.y);
                    }
                    else
                    {
                        vec4 _23342;
                        if (_14035 == 2)
                        {
                            vec2 _14381 = (_7086 * _172.gLevel[2].xy) - vec2(0.5);
                            vec2 _14383 = floor(_14381);
                            vec2 _14386 = _14381 - _14383;
                            vec2 _14389 = _14386 * _14386;
                            vec2 _14392 = _14389 * _14386;
                            vec2 _14411 = (((_14392 * 3.0) - (_14389 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _14424 = _14392 * 0.16666667163372039794921875;
                            vec2 _14427 = (((((-_14392) + (_14389 * 3.0)) - (_14386 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14411;
                            vec2 _14430 = (((((_14392 * (-3.0)) + (_14389 * 3.0)) + (_14386 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14424;
                            vec2 _14440 = ((_14383 - vec2(0.5)) + (_14411 / _14427)) * _172.gLevel[2].zw;
                            vec2 _14450 = ((_14383 + vec2(1.5)) + (_14424 / _14430)) * _172.gLevel[2].zw;
                            vec2 _23331;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22105 = _14440;
                                _22105.y = 1.0 - _14440.y;
                                _23331 = _22105;
                            }
                            else
                            {
                                _23331 = _14440;
                            }
                            float _14470 = _14440.y;
                            vec2 _14471 = vec2(_14450.x, _14470);
                            vec2 _23332;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22111 = _14471;
                                _22111.y = 1.0 - _14470;
                                _23332 = _22111;
                            }
                            else
                            {
                                _23332 = _14471;
                            }
                            float _14487 = _14450.y;
                            vec2 _14488 = vec2(_14440.x, _14487);
                            vec2 _23333;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22118 = _14488;
                                _22118.y = 1.0 - _14487;
                                _23333 = _22118;
                            }
                            else
                            {
                                _23333 = _14488;
                            }
                            vec2 _23334;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22124 = _14450;
                                _22124.y = 1.0 - _14450.y;
                                _23334 = _22124;
                            }
                            else
                            {
                                _23334 = _14450;
                            }
                            _23342 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23331, 0.0) * _14427.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23332, 0.0) * _14430.x)) * _14427.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23333, 0.0) * _14427.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23334, 0.0) * _14430.x)) * _14430.y);
                        }
                        else
                        {
                            vec4 _23343;
                            if (_14035 == 3)
                            {
                                vec2 _14588 = (_7086 * _172.gLevel[3].xy) - vec2(0.5);
                                vec2 _14590 = floor(_14588);
                                vec2 _14593 = _14588 - _14590;
                                vec2 _14596 = _14593 * _14593;
                                vec2 _14599 = _14596 * _14593;
                                vec2 _14618 = (((_14599 * 3.0) - (_14596 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _14631 = _14599 * 0.16666667163372039794921875;
                                vec2 _14634 = (((((-_14599) + (_14596 * 3.0)) - (_14593 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14618;
                                vec2 _14637 = (((((_14599 * (-3.0)) + (_14596 * 3.0)) + (_14593 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14631;
                                vec2 _14647 = ((_14590 - vec2(0.5)) + (_14618 / _14634)) * _172.gLevel[3].zw;
                                vec2 _14657 = ((_14590 + vec2(1.5)) + (_14631 / _14637)) * _172.gLevel[3].zw;
                                vec2 _23327;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22131 = _14647;
                                    _22131.y = 1.0 - _14647.y;
                                    _23327 = _22131;
                                }
                                else
                                {
                                    _23327 = _14647;
                                }
                                float _14677 = _14647.y;
                                vec2 _14678 = vec2(_14657.x, _14677);
                                vec2 _23328;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22137 = _14678;
                                    _22137.y = 1.0 - _14677;
                                    _23328 = _22137;
                                }
                                else
                                {
                                    _23328 = _14678;
                                }
                                float _14694 = _14657.y;
                                vec2 _14695 = vec2(_14647.x, _14694);
                                vec2 _23329;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22144 = _14695;
                                    _22144.y = 1.0 - _14694;
                                    _23329 = _22144;
                                }
                                else
                                {
                                    _23329 = _14695;
                                }
                                vec2 _23330;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22150 = _14657;
                                    _22150.y = 1.0 - _14657.y;
                                    _23330 = _22150;
                                }
                                else
                                {
                                    _23330 = _14657;
                                }
                                _23343 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23327, 0.0) * _14634.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23328, 0.0) * _14637.x)) * _14634.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23329, 0.0) * _14634.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23330, 0.0) * _14637.x)) * _14637.y);
                            }
                            else
                            {
                                vec4 _23344;
                                if (_14035 == 4)
                                {
                                    vec2 _14795 = (_7086 * _172.gLevel[4].xy) - vec2(0.5);
                                    vec2 _14797 = floor(_14795);
                                    vec2 _14800 = _14795 - _14797;
                                    vec2 _14803 = _14800 * _14800;
                                    vec2 _14806 = _14803 * _14800;
                                    vec2 _14825 = (((_14806 * 3.0) - (_14803 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _14838 = _14806 * 0.16666667163372039794921875;
                                    vec2 _14841 = (((((-_14806) + (_14803 * 3.0)) - (_14800 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14825;
                                    vec2 _14844 = (((((_14806 * (-3.0)) + (_14803 * 3.0)) + (_14800 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _14838;
                                    vec2 _14854 = ((_14797 - vec2(0.5)) + (_14825 / _14841)) * _172.gLevel[4].zw;
                                    vec2 _14864 = ((_14797 + vec2(1.5)) + (_14838 / _14844)) * _172.gLevel[4].zw;
                                    vec2 _23323;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22157 = _14854;
                                        _22157.y = 1.0 - _14854.y;
                                        _23323 = _22157;
                                    }
                                    else
                                    {
                                        _23323 = _14854;
                                    }
                                    float _14884 = _14854.y;
                                    vec2 _14885 = vec2(_14864.x, _14884);
                                    vec2 _23324;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22163 = _14885;
                                        _22163.y = 1.0 - _14884;
                                        _23324 = _22163;
                                    }
                                    else
                                    {
                                        _23324 = _14885;
                                    }
                                    float _14901 = _14864.y;
                                    vec2 _14902 = vec2(_14854.x, _14901);
                                    vec2 _23325;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22170 = _14902;
                                        _22170.y = 1.0 - _14901;
                                        _23325 = _22170;
                                    }
                                    else
                                    {
                                        _23325 = _14902;
                                    }
                                    vec2 _23326;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22176 = _14864;
                                        _22176.y = 1.0 - _14864.y;
                                        _23326 = _22176;
                                    }
                                    else
                                    {
                                        _23326 = _14864;
                                    }
                                    _23344 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23323, 0.0) * _14841.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23324, 0.0) * _14844.x)) * _14841.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23325, 0.0) * _14841.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23326, 0.0) * _14844.x)) * _14844.y);
                                }
                                else
                                {
                                    vec2 _15002 = (_7086 * _172.gLevel[5].xy) - vec2(0.5);
                                    vec2 _15004 = floor(_15002);
                                    vec2 _15007 = _15002 - _15004;
                                    vec2 _15010 = _15007 * _15007;
                                    vec2 _15013 = _15010 * _15007;
                                    vec2 _15032 = (((_15013 * 3.0) - (_15010 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _15045 = _15013 * 0.16666667163372039794921875;
                                    vec2 _15048 = (((((-_15013) + (_15010 * 3.0)) - (_15007 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15032;
                                    vec2 _15051 = (((((_15013 * (-3.0)) + (_15010 * 3.0)) + (_15007 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15045;
                                    vec2 _15061 = ((_15004 - vec2(0.5)) + (_15032 / _15048)) * _172.gLevel[5].zw;
                                    vec2 _15071 = ((_15004 + vec2(1.5)) + (_15045 / _15051)) * _172.gLevel[5].zw;
                                    vec2 _23319;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22183 = _15061;
                                        _22183.y = 1.0 - _15061.y;
                                        _23319 = _22183;
                                    }
                                    else
                                    {
                                        _23319 = _15061;
                                    }
                                    float _15091 = _15061.y;
                                    vec2 _15092 = vec2(_15071.x, _15091);
                                    vec2 _23320;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22189 = _15092;
                                        _22189.y = 1.0 - _15091;
                                        _23320 = _22189;
                                    }
                                    else
                                    {
                                        _23320 = _15092;
                                    }
                                    float _15108 = _15071.y;
                                    vec2 _15109 = vec2(_15061.x, _15108);
                                    vec2 _23321;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22196 = _15109;
                                        _22196.y = 1.0 - _15108;
                                        _23321 = _22196;
                                    }
                                    else
                                    {
                                        _23321 = _15109;
                                    }
                                    vec2 _23322;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22202 = _15071;
                                        _22202.y = 1.0 - _15071.y;
                                        _23322 = _22202;
                                    }
                                    else
                                    {
                                        _23322 = _15071;
                                    }
                                    _23344 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23319, 0.0) * _15048.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23320, 0.0) * _15051.x)) * _15048.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23321, 0.0) * _15048.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23322, 0.0) * _15051.x)) * _15051.y);
                                }
                                _23343 = _23344;
                            }
                            _23342 = _23343;
                        }
                        _23341 = _23342;
                    }
                    _23340 = _23341;
                }
                vec3 _23371;
                SPIRV_CROSS_BRANCH
                if ((_14039 > 0.0199999995529651641845703125) && (_14035 < 5))
                {
                    int _14052 = _14035 + 1;
                    vec4 _23366;
                    SPIRV_CROSS_BRANCH
                    if (_14052 <= 0)
                    {
                        vec2 _23365;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22207 = _7086;
                            _22207.y = 1.0 - _7086.y;
                            _23365 = _22207;
                        }
                        else
                        {
                            _23365 = _7086;
                        }
                        _23366 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23365, 0.0);
                    }
                    else
                    {
                        vec4 _23367;
                        if (_14052 == 1)
                        {
                            vec2 _15298 = (_7086 * _172.gLevel[1].xy) - vec2(0.5);
                            vec2 _15300 = floor(_15298);
                            vec2 _15303 = _15298 - _15300;
                            vec2 _15306 = _15303 * _15303;
                            vec2 _15309 = _15306 * _15303;
                            vec2 _15328 = (((_15309 * 3.0) - (_15306 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _15341 = _15309 * 0.16666667163372039794921875;
                            vec2 _15344 = (((((-_15309) + (_15306 * 3.0)) - (_15303 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15328;
                            vec2 _15347 = (((((_15309 * (-3.0)) + (_15306 * 3.0)) + (_15303 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15341;
                            vec2 _15357 = ((_15300 - vec2(0.5)) + (_15328 / _15344)) * _172.gLevel[1].zw;
                            vec2 _15367 = ((_15300 + vec2(1.5)) + (_15341 / _15347)) * _172.gLevel[1].zw;
                            vec2 _23361;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22212 = _15357;
                                _22212.y = 1.0 - _15357.y;
                                _23361 = _22212;
                            }
                            else
                            {
                                _23361 = _15357;
                            }
                            float _15387 = _15357.y;
                            vec2 _15388 = vec2(_15367.x, _15387);
                            vec2 _23362;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22218 = _15388;
                                _22218.y = 1.0 - _15387;
                                _23362 = _22218;
                            }
                            else
                            {
                                _23362 = _15388;
                            }
                            float _15404 = _15367.y;
                            vec2 _15405 = vec2(_15357.x, _15404);
                            vec2 _23363;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22225 = _15405;
                                _22225.y = 1.0 - _15404;
                                _23363 = _22225;
                            }
                            else
                            {
                                _23363 = _15405;
                            }
                            vec2 _23364;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22231 = _15367;
                                _22231.y = 1.0 - _15367.y;
                                _23364 = _22231;
                            }
                            else
                            {
                                _23364 = _15367;
                            }
                            _23367 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23361, 0.0) * _15344.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23362, 0.0) * _15347.x)) * _15344.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23363, 0.0) * _15344.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23364, 0.0) * _15347.x)) * _15347.y);
                        }
                        else
                        {
                            vec4 _23368;
                            if (_14052 == 2)
                            {
                                vec2 _15505 = (_7086 * _172.gLevel[2].xy) - vec2(0.5);
                                vec2 _15507 = floor(_15505);
                                vec2 _15510 = _15505 - _15507;
                                vec2 _15513 = _15510 * _15510;
                                vec2 _15516 = _15513 * _15510;
                                vec2 _15535 = (((_15516 * 3.0) - (_15513 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _15548 = _15516 * 0.16666667163372039794921875;
                                vec2 _15551 = (((((-_15516) + (_15513 * 3.0)) - (_15510 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15535;
                                vec2 _15554 = (((((_15516 * (-3.0)) + (_15513 * 3.0)) + (_15510 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15548;
                                vec2 _15564 = ((_15507 - vec2(0.5)) + (_15535 / _15551)) * _172.gLevel[2].zw;
                                vec2 _15574 = ((_15507 + vec2(1.5)) + (_15548 / _15554)) * _172.gLevel[2].zw;
                                vec2 _23357;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22238 = _15564;
                                    _22238.y = 1.0 - _15564.y;
                                    _23357 = _22238;
                                }
                                else
                                {
                                    _23357 = _15564;
                                }
                                float _15594 = _15564.y;
                                vec2 _15595 = vec2(_15574.x, _15594);
                                vec2 _23358;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22244 = _15595;
                                    _22244.y = 1.0 - _15594;
                                    _23358 = _22244;
                                }
                                else
                                {
                                    _23358 = _15595;
                                }
                                float _15611 = _15574.y;
                                vec2 _15612 = vec2(_15564.x, _15611);
                                vec2 _23359;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22251 = _15612;
                                    _22251.y = 1.0 - _15611;
                                    _23359 = _22251;
                                }
                                else
                                {
                                    _23359 = _15612;
                                }
                                vec2 _23360;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22257 = _15574;
                                    _22257.y = 1.0 - _15574.y;
                                    _23360 = _22257;
                                }
                                else
                                {
                                    _23360 = _15574;
                                }
                                _23368 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23357, 0.0) * _15551.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23358, 0.0) * _15554.x)) * _15551.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23359, 0.0) * _15551.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23360, 0.0) * _15554.x)) * _15554.y);
                            }
                            else
                            {
                                vec4 _23369;
                                if (_14052 == 3)
                                {
                                    vec2 _15712 = (_7086 * _172.gLevel[3].xy) - vec2(0.5);
                                    vec2 _15714 = floor(_15712);
                                    vec2 _15717 = _15712 - _15714;
                                    vec2 _15720 = _15717 * _15717;
                                    vec2 _15723 = _15720 * _15717;
                                    vec2 _15742 = (((_15723 * 3.0) - (_15720 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _15755 = _15723 * 0.16666667163372039794921875;
                                    vec2 _15758 = (((((-_15723) + (_15720 * 3.0)) - (_15717 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15742;
                                    vec2 _15761 = (((((_15723 * (-3.0)) + (_15720 * 3.0)) + (_15717 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15755;
                                    vec2 _15771 = ((_15714 - vec2(0.5)) + (_15742 / _15758)) * _172.gLevel[3].zw;
                                    vec2 _15781 = ((_15714 + vec2(1.5)) + (_15755 / _15761)) * _172.gLevel[3].zw;
                                    vec2 _23353;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22264 = _15771;
                                        _22264.y = 1.0 - _15771.y;
                                        _23353 = _22264;
                                    }
                                    else
                                    {
                                        _23353 = _15771;
                                    }
                                    float _15801 = _15771.y;
                                    vec2 _15802 = vec2(_15781.x, _15801);
                                    vec2 _23354;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22270 = _15802;
                                        _22270.y = 1.0 - _15801;
                                        _23354 = _22270;
                                    }
                                    else
                                    {
                                        _23354 = _15802;
                                    }
                                    float _15818 = _15781.y;
                                    vec2 _15819 = vec2(_15771.x, _15818);
                                    vec2 _23355;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22277 = _15819;
                                        _22277.y = 1.0 - _15818;
                                        _23355 = _22277;
                                    }
                                    else
                                    {
                                        _23355 = _15819;
                                    }
                                    vec2 _23356;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22283 = _15781;
                                        _22283.y = 1.0 - _15781.y;
                                        _23356 = _22283;
                                    }
                                    else
                                    {
                                        _23356 = _15781;
                                    }
                                    _23369 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23353, 0.0) * _15758.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23354, 0.0) * _15761.x)) * _15758.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23355, 0.0) * _15758.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23356, 0.0) * _15761.x)) * _15761.y);
                                }
                                else
                                {
                                    vec4 _23370;
                                    if (_14052 == 4)
                                    {
                                        vec2 _15919 = (_7086 * _172.gLevel[4].xy) - vec2(0.5);
                                        vec2 _15921 = floor(_15919);
                                        vec2 _15924 = _15919 - _15921;
                                        vec2 _15927 = _15924 * _15924;
                                        vec2 _15930 = _15927 * _15924;
                                        vec2 _15949 = (((_15930 * 3.0) - (_15927 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _15962 = _15930 * 0.16666667163372039794921875;
                                        vec2 _15965 = (((((-_15930) + (_15927 * 3.0)) - (_15924 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15949;
                                        vec2 _15968 = (((((_15930 * (-3.0)) + (_15927 * 3.0)) + (_15924 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _15962;
                                        vec2 _15978 = ((_15921 - vec2(0.5)) + (_15949 / _15965)) * _172.gLevel[4].zw;
                                        vec2 _15988 = ((_15921 + vec2(1.5)) + (_15962 / _15968)) * _172.gLevel[4].zw;
                                        vec2 _23349;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22290 = _15978;
                                            _22290.y = 1.0 - _15978.y;
                                            _23349 = _22290;
                                        }
                                        else
                                        {
                                            _23349 = _15978;
                                        }
                                        float _16008 = _15978.y;
                                        vec2 _16009 = vec2(_15988.x, _16008);
                                        vec2 _23350;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22296 = _16009;
                                            _22296.y = 1.0 - _16008;
                                            _23350 = _22296;
                                        }
                                        else
                                        {
                                            _23350 = _16009;
                                        }
                                        float _16025 = _15988.y;
                                        vec2 _16026 = vec2(_15978.x, _16025);
                                        vec2 _23351;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22303 = _16026;
                                            _22303.y = 1.0 - _16025;
                                            _23351 = _22303;
                                        }
                                        else
                                        {
                                            _23351 = _16026;
                                        }
                                        vec2 _23352;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22309 = _15988;
                                            _22309.y = 1.0 - _15988.y;
                                            _23352 = _22309;
                                        }
                                        else
                                        {
                                            _23352 = _15988;
                                        }
                                        _23370 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23349, 0.0) * _15965.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23350, 0.0) * _15968.x)) * _15965.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23351, 0.0) * _15965.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23352, 0.0) * _15968.x)) * _15968.y);
                                    }
                                    else
                                    {
                                        vec2 _16126 = (_7086 * _172.gLevel[5].xy) - vec2(0.5);
                                        vec2 _16128 = floor(_16126);
                                        vec2 _16131 = _16126 - _16128;
                                        vec2 _16134 = _16131 * _16131;
                                        vec2 _16137 = _16134 * _16131;
                                        vec2 _16156 = (((_16137 * 3.0) - (_16134 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _16169 = _16137 * 0.16666667163372039794921875;
                                        vec2 _16172 = (((((-_16137) + (_16134 * 3.0)) - (_16131 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16156;
                                        vec2 _16175 = (((((_16137 * (-3.0)) + (_16134 * 3.0)) + (_16131 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16169;
                                        vec2 _16185 = ((_16128 - vec2(0.5)) + (_16156 / _16172)) * _172.gLevel[5].zw;
                                        vec2 _16195 = ((_16128 + vec2(1.5)) + (_16169 / _16175)) * _172.gLevel[5].zw;
                                        vec2 _23345;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22316 = _16185;
                                            _22316.y = 1.0 - _16185.y;
                                            _23345 = _22316;
                                        }
                                        else
                                        {
                                            _23345 = _16185;
                                        }
                                        float _16215 = _16185.y;
                                        vec2 _16216 = vec2(_16195.x, _16215);
                                        vec2 _23346;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22322 = _16216;
                                            _22322.y = 1.0 - _16215;
                                            _23346 = _22322;
                                        }
                                        else
                                        {
                                            _23346 = _16216;
                                        }
                                        float _16232 = _16195.y;
                                        vec2 _16233 = vec2(_16185.x, _16232);
                                        vec2 _23347;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22329 = _16233;
                                            _22329.y = 1.0 - _16232;
                                            _23347 = _22329;
                                        }
                                        else
                                        {
                                            _23347 = _16233;
                                        }
                                        vec2 _23348;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22335 = _16195;
                                            _22335.y = 1.0 - _16195.y;
                                            _23348 = _22335;
                                        }
                                        else
                                        {
                                            _23348 = _16195;
                                        }
                                        _23370 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23345, 0.0) * _16172.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23346, 0.0) * _16175.x)) * _16172.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23347, 0.0) * _16172.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23348, 0.0) * _16175.x)) * _16175.y);
                                    }
                                    _23369 = _23370;
                                }
                                _23368 = _23369;
                            }
                            _23367 = _23368;
                        }
                        _23366 = _23367;
                    }
                    _23371 = mix(_23340.xyz, _23366.xyz, vec3(_14039));
                }
                else
                {
                    _23371 = _23340.xyz;
                }
                _23661 = vec3(_23159.x, _23265.y, _23371.z);
            }
            else
            {
                vec2 _7094 = _7024 + _7036;
                float _16323 = clamp(log2(max(_6938, 1.0)) - 1.0, 0.0, 5.0);
                int _16326 = int(floor(_16323));
                float _16330 = _16323 - float(_16326);
                vec4 _23075;
                SPIRV_CROSS_BRANCH
                if (_16326 <= 0)
                {
                    vec2 _23074;
                    if (_172.gConv.x > 0.5)
                    {
                        vec2 _22342 = _7094;
                        _22342.y = 1.0 - _7094.y;
                        _23074 = _22342;
                    }
                    else
                    {
                        _23074 = _7094;
                    }
                    _23075 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23074, 0.0);
                }
                else
                {
                    vec4 _23076;
                    if (_16326 == 1)
                    {
                        vec2 _16465 = (_7094 * _172.gLevel[1].xy) - vec2(0.5);
                        vec2 _16467 = floor(_16465);
                        vec2 _16470 = _16465 - _16467;
                        vec2 _16473 = _16470 * _16470;
                        vec2 _16476 = _16473 * _16470;
                        vec2 _16495 = (((_16476 * 3.0) - (_16473 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                        vec2 _16508 = _16476 * 0.16666667163372039794921875;
                        vec2 _16511 = (((((-_16476) + (_16473 * 3.0)) - (_16470 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16495;
                        vec2 _16514 = (((((_16476 * (-3.0)) + (_16473 * 3.0)) + (_16470 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16508;
                        vec2 _16524 = ((_16467 - vec2(0.5)) + (_16495 / _16511)) * _172.gLevel[1].zw;
                        vec2 _16534 = ((_16467 + vec2(1.5)) + (_16508 / _16514)) * _172.gLevel[1].zw;
                        vec2 _23070;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22347 = _16524;
                            _22347.y = 1.0 - _16524.y;
                            _23070 = _22347;
                        }
                        else
                        {
                            _23070 = _16524;
                        }
                        vec2 _16555 = vec2(_16534.x, _16524.y);
                        vec2 _23071;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22353 = _16555;
                            _22353.y = 1.0 - _16524.y;
                            _23071 = _22353;
                        }
                        else
                        {
                            _23071 = _16555;
                        }
                        float _16571 = _16534.y;
                        vec2 _16572 = vec2(_16524.x, _16571);
                        vec2 _23072;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22360 = _16572;
                            _22360.y = 1.0 - _16571;
                            _23072 = _22360;
                        }
                        else
                        {
                            _23072 = _16572;
                        }
                        vec2 _23073;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22366 = _16534;
                            _22366.y = 1.0 - _16534.y;
                            _23073 = _22366;
                        }
                        else
                        {
                            _23073 = _16534;
                        }
                        _23076 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23070, 0.0) * _16511.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23071, 0.0) * _16514.x)) * _16511.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23072, 0.0) * _16511.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23073, 0.0) * _16514.x)) * _16514.y);
                    }
                    else
                    {
                        vec4 _23077;
                        if (_16326 == 2)
                        {
                            vec2 _16672 = (_7094 * _172.gLevel[2].xy) - vec2(0.5);
                            vec2 _16674 = floor(_16672);
                            vec2 _16677 = _16672 - _16674;
                            vec2 _16680 = _16677 * _16677;
                            vec2 _16683 = _16680 * _16677;
                            vec2 _16702 = (((_16683 * 3.0) - (_16680 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _16715 = _16683 * 0.16666667163372039794921875;
                            vec2 _16718 = (((((-_16683) + (_16680 * 3.0)) - (_16677 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16702;
                            vec2 _16721 = (((((_16683 * (-3.0)) + (_16680 * 3.0)) + (_16677 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16715;
                            vec2 _16731 = ((_16674 - vec2(0.5)) + (_16702 / _16718)) * _172.gLevel[2].zw;
                            vec2 _16741 = ((_16674 + vec2(1.5)) + (_16715 / _16721)) * _172.gLevel[2].zw;
                            vec2 _23066;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22373 = _16731;
                                _22373.y = 1.0 - _16731.y;
                                _23066 = _22373;
                            }
                            else
                            {
                                _23066 = _16731;
                            }
                            vec2 _16762 = vec2(_16741.x, _16731.y);
                            vec2 _23067;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22379 = _16762;
                                _22379.y = 1.0 - _16731.y;
                                _23067 = _22379;
                            }
                            else
                            {
                                _23067 = _16762;
                            }
                            float _16778 = _16741.y;
                            vec2 _16779 = vec2(_16731.x, _16778);
                            vec2 _23068;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22386 = _16779;
                                _22386.y = 1.0 - _16778;
                                _23068 = _22386;
                            }
                            else
                            {
                                _23068 = _16779;
                            }
                            vec2 _23069;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22392 = _16741;
                                _22392.y = 1.0 - _16741.y;
                                _23069 = _22392;
                            }
                            else
                            {
                                _23069 = _16741;
                            }
                            _23077 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23066, 0.0) * _16718.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23067, 0.0) * _16721.x)) * _16718.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23068, 0.0) * _16718.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23069, 0.0) * _16721.x)) * _16721.y);
                        }
                        else
                        {
                            vec4 _23078;
                            if (_16326 == 3)
                            {
                                vec2 _16879 = (_7094 * _172.gLevel[3].xy) - vec2(0.5);
                                vec2 _16881 = floor(_16879);
                                vec2 _16884 = _16879 - _16881;
                                vec2 _16887 = _16884 * _16884;
                                vec2 _16890 = _16887 * _16884;
                                vec2 _16909 = (((_16890 * 3.0) - (_16887 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _16922 = _16890 * 0.16666667163372039794921875;
                                vec2 _16925 = (((((-_16890) + (_16887 * 3.0)) - (_16884 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16909;
                                vec2 _16928 = (((((_16890 * (-3.0)) + (_16887 * 3.0)) + (_16884 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _16922;
                                vec2 _16938 = ((_16881 - vec2(0.5)) + (_16909 / _16925)) * _172.gLevel[3].zw;
                                vec2 _16948 = ((_16881 + vec2(1.5)) + (_16922 / _16928)) * _172.gLevel[3].zw;
                                vec2 _23062;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22399 = _16938;
                                    _22399.y = 1.0 - _16938.y;
                                    _23062 = _22399;
                                }
                                else
                                {
                                    _23062 = _16938;
                                }
                                vec2 _16969 = vec2(_16948.x, _16938.y);
                                vec2 _23063;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22405 = _16969;
                                    _22405.y = 1.0 - _16938.y;
                                    _23063 = _22405;
                                }
                                else
                                {
                                    _23063 = _16969;
                                }
                                float _16985 = _16948.y;
                                vec2 _16986 = vec2(_16938.x, _16985);
                                vec2 _23064;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22412 = _16986;
                                    _22412.y = 1.0 - _16985;
                                    _23064 = _22412;
                                }
                                else
                                {
                                    _23064 = _16986;
                                }
                                vec2 _23065;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22418 = _16948;
                                    _22418.y = 1.0 - _16948.y;
                                    _23065 = _22418;
                                }
                                else
                                {
                                    _23065 = _16948;
                                }
                                _23078 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23062, 0.0) * _16925.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23063, 0.0) * _16928.x)) * _16925.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23064, 0.0) * _16925.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23065, 0.0) * _16928.x)) * _16928.y);
                            }
                            else
                            {
                                vec4 _23079;
                                if (_16326 == 4)
                                {
                                    vec2 _17086 = (_7094 * _172.gLevel[4].xy) - vec2(0.5);
                                    vec2 _17088 = floor(_17086);
                                    vec2 _17091 = _17086 - _17088;
                                    vec2 _17094 = _17091 * _17091;
                                    vec2 _17097 = _17094 * _17091;
                                    vec2 _17116 = (((_17097 * 3.0) - (_17094 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _17129 = _17097 * 0.16666667163372039794921875;
                                    vec2 _17132 = (((((-_17097) + (_17094 * 3.0)) - (_17091 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17116;
                                    vec2 _17135 = (((((_17097 * (-3.0)) + (_17094 * 3.0)) + (_17091 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17129;
                                    vec2 _17145 = ((_17088 - vec2(0.5)) + (_17116 / _17132)) * _172.gLevel[4].zw;
                                    vec2 _17155 = ((_17088 + vec2(1.5)) + (_17129 / _17135)) * _172.gLevel[4].zw;
                                    vec2 _23058;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22425 = _17145;
                                        _22425.y = 1.0 - _17145.y;
                                        _23058 = _22425;
                                    }
                                    else
                                    {
                                        _23058 = _17145;
                                    }
                                    vec2 _17176 = vec2(_17155.x, _17145.y);
                                    vec2 _23059;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22431 = _17176;
                                        _22431.y = 1.0 - _17145.y;
                                        _23059 = _22431;
                                    }
                                    else
                                    {
                                        _23059 = _17176;
                                    }
                                    float _17192 = _17155.y;
                                    vec2 _17193 = vec2(_17145.x, _17192);
                                    vec2 _23060;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22438 = _17193;
                                        _22438.y = 1.0 - _17192;
                                        _23060 = _22438;
                                    }
                                    else
                                    {
                                        _23060 = _17193;
                                    }
                                    vec2 _23061;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22444 = _17155;
                                        _22444.y = 1.0 - _17155.y;
                                        _23061 = _22444;
                                    }
                                    else
                                    {
                                        _23061 = _17155;
                                    }
                                    _23079 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23058, 0.0) * _17132.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23059, 0.0) * _17135.x)) * _17132.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23060, 0.0) * _17132.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23061, 0.0) * _17135.x)) * _17135.y);
                                }
                                else
                                {
                                    vec2 _17293 = (_7094 * _172.gLevel[5].xy) - vec2(0.5);
                                    vec2 _17295 = floor(_17293);
                                    vec2 _17298 = _17293 - _17295;
                                    vec2 _17301 = _17298 * _17298;
                                    vec2 _17304 = _17301 * _17298;
                                    vec2 _17323 = (((_17304 * 3.0) - (_17301 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _17336 = _17304 * 0.16666667163372039794921875;
                                    vec2 _17339 = (((((-_17304) + (_17301 * 3.0)) - (_17298 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17323;
                                    vec2 _17342 = (((((_17304 * (-3.0)) + (_17301 * 3.0)) + (_17298 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17336;
                                    vec2 _17352 = ((_17295 - vec2(0.5)) + (_17323 / _17339)) * _172.gLevel[5].zw;
                                    vec2 _17362 = ((_17295 + vec2(1.5)) + (_17336 / _17342)) * _172.gLevel[5].zw;
                                    vec2 _23054;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22451 = _17352;
                                        _22451.y = 1.0 - _17352.y;
                                        _23054 = _22451;
                                    }
                                    else
                                    {
                                        _23054 = _17352;
                                    }
                                    vec2 _17383 = vec2(_17362.x, _17352.y);
                                    vec2 _23055;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22457 = _17383;
                                        _22457.y = 1.0 - _17352.y;
                                        _23055 = _22457;
                                    }
                                    else
                                    {
                                        _23055 = _17383;
                                    }
                                    float _17399 = _17362.y;
                                    vec2 _17400 = vec2(_17352.x, _17399);
                                    vec2 _23056;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22464 = _17400;
                                        _22464.y = 1.0 - _17399;
                                        _23056 = _22464;
                                    }
                                    else
                                    {
                                        _23056 = _17400;
                                    }
                                    vec2 _23057;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22470 = _17362;
                                        _22470.y = 1.0 - _17362.y;
                                        _23057 = _22470;
                                    }
                                    else
                                    {
                                        _23057 = _17362;
                                    }
                                    _23079 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23054, 0.0) * _17339.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23055, 0.0) * _17342.x)) * _17339.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23056, 0.0) * _17339.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23057, 0.0) * _17342.x)) * _17342.y);
                                }
                                _23078 = _23079;
                            }
                            _23077 = _23078;
                        }
                        _23076 = _23077;
                    }
                    _23075 = _23076;
                }
                vec3 _23106;
                SPIRV_CROSS_BRANCH
                if ((_16330 > 0.0199999995529651641845703125) && (_16326 < 5))
                {
                    int _16343 = _16326 + 1;
                    vec4 _23101;
                    SPIRV_CROSS_BRANCH
                    if (_16343 <= 0)
                    {
                        vec2 _23100;
                        if (_172.gConv.x > 0.5)
                        {
                            vec2 _22475 = _7094;
                            _22475.y = 1.0 - _7094.y;
                            _23100 = _22475;
                        }
                        else
                        {
                            _23100 = _7094;
                        }
                        _23101 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23100, 0.0);
                    }
                    else
                    {
                        vec4 _23102;
                        if (_16343 == 1)
                        {
                            vec2 _17589 = (_7094 * _172.gLevel[1].xy) - vec2(0.5);
                            vec2 _17591 = floor(_17589);
                            vec2 _17594 = _17589 - _17591;
                            vec2 _17597 = _17594 * _17594;
                            vec2 _17600 = _17597 * _17594;
                            vec2 _17619 = (((_17600 * 3.0) - (_17597 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                            vec2 _17632 = _17600 * 0.16666667163372039794921875;
                            vec2 _17635 = (((((-_17600) + (_17597 * 3.0)) - (_17594 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17619;
                            vec2 _17638 = (((((_17600 * (-3.0)) + (_17597 * 3.0)) + (_17594 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17632;
                            vec2 _17648 = ((_17591 - vec2(0.5)) + (_17619 / _17635)) * _172.gLevel[1].zw;
                            vec2 _17658 = ((_17591 + vec2(1.5)) + (_17632 / _17638)) * _172.gLevel[1].zw;
                            vec2 _23096;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22480 = _17648;
                                _22480.y = 1.0 - _17648.y;
                                _23096 = _22480;
                            }
                            else
                            {
                                _23096 = _17648;
                            }
                            vec2 _17679 = vec2(_17658.x, _17648.y);
                            vec2 _23097;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22486 = _17679;
                                _22486.y = 1.0 - _17648.y;
                                _23097 = _22486;
                            }
                            else
                            {
                                _23097 = _17679;
                            }
                            float _17695 = _17658.y;
                            vec2 _17696 = vec2(_17648.x, _17695);
                            vec2 _23098;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22493 = _17696;
                                _22493.y = 1.0 - _17695;
                                _23098 = _22493;
                            }
                            else
                            {
                                _23098 = _17696;
                            }
                            vec2 _23099;
                            if (_172.gConv.x > 0.5)
                            {
                                vec2 _22499 = _17658;
                                _22499.y = 1.0 - _17658.y;
                                _23099 = _22499;
                            }
                            else
                            {
                                _23099 = _17658;
                            }
                            _23102 = (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23096, 0.0) * _17635.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23097, 0.0) * _17638.x)) * _17635.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23098, 0.0) * _17635.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23099, 0.0) * _17638.x)) * _17638.y);
                        }
                        else
                        {
                            vec4 _23103;
                            if (_16343 == 2)
                            {
                                vec2 _17796 = (_7094 * _172.gLevel[2].xy) - vec2(0.5);
                                vec2 _17798 = floor(_17796);
                                vec2 _17801 = _17796 - _17798;
                                vec2 _17804 = _17801 * _17801;
                                vec2 _17807 = _17804 * _17801;
                                vec2 _17826 = (((_17807 * 3.0) - (_17804 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                vec2 _17839 = _17807 * 0.16666667163372039794921875;
                                vec2 _17842 = (((((-_17807) + (_17804 * 3.0)) - (_17801 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17826;
                                vec2 _17845 = (((((_17807 * (-3.0)) + (_17804 * 3.0)) + (_17801 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _17839;
                                vec2 _17855 = ((_17798 - vec2(0.5)) + (_17826 / _17842)) * _172.gLevel[2].zw;
                                vec2 _17865 = ((_17798 + vec2(1.5)) + (_17839 / _17845)) * _172.gLevel[2].zw;
                                vec2 _23092;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22506 = _17855;
                                    _22506.y = 1.0 - _17855.y;
                                    _23092 = _22506;
                                }
                                else
                                {
                                    _23092 = _17855;
                                }
                                vec2 _17886 = vec2(_17865.x, _17855.y);
                                vec2 _23093;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22512 = _17886;
                                    _22512.y = 1.0 - _17855.y;
                                    _23093 = _22512;
                                }
                                else
                                {
                                    _23093 = _17886;
                                }
                                float _17902 = _17865.y;
                                vec2 _17903 = vec2(_17855.x, _17902);
                                vec2 _23094;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22519 = _17903;
                                    _22519.y = 1.0 - _17902;
                                    _23094 = _22519;
                                }
                                else
                                {
                                    _23094 = _17903;
                                }
                                vec2 _23095;
                                if (_172.gConv.x > 0.5)
                                {
                                    vec2 _22525 = _17865;
                                    _22525.y = 1.0 - _17865.y;
                                    _23095 = _22525;
                                }
                                else
                                {
                                    _23095 = _17865;
                                }
                                _23103 = (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23092, 0.0) * _17842.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23093, 0.0) * _17845.x)) * _17842.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23094, 0.0) * _17842.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23095, 0.0) * _17845.x)) * _17845.y);
                            }
                            else
                            {
                                vec4 _23104;
                                if (_16343 == 3)
                                {
                                    vec2 _18003 = (_7094 * _172.gLevel[3].xy) - vec2(0.5);
                                    vec2 _18005 = floor(_18003);
                                    vec2 _18008 = _18003 - _18005;
                                    vec2 _18011 = _18008 * _18008;
                                    vec2 _18014 = _18011 * _18008;
                                    vec2 _18033 = (((_18014 * 3.0) - (_18011 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                    vec2 _18046 = _18014 * 0.16666667163372039794921875;
                                    vec2 _18049 = (((((-_18014) + (_18011 * 3.0)) - (_18008 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18033;
                                    vec2 _18052 = (((((_18014 * (-3.0)) + (_18011 * 3.0)) + (_18008 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18046;
                                    vec2 _18062 = ((_18005 - vec2(0.5)) + (_18033 / _18049)) * _172.gLevel[3].zw;
                                    vec2 _18072 = ((_18005 + vec2(1.5)) + (_18046 / _18052)) * _172.gLevel[3].zw;
                                    vec2 _23088;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22532 = _18062;
                                        _22532.y = 1.0 - _18062.y;
                                        _23088 = _22532;
                                    }
                                    else
                                    {
                                        _23088 = _18062;
                                    }
                                    vec2 _18093 = vec2(_18072.x, _18062.y);
                                    vec2 _23089;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22538 = _18093;
                                        _22538.y = 1.0 - _18062.y;
                                        _23089 = _22538;
                                    }
                                    else
                                    {
                                        _23089 = _18093;
                                    }
                                    float _18109 = _18072.y;
                                    vec2 _18110 = vec2(_18062.x, _18109);
                                    vec2 _23090;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22545 = _18110;
                                        _22545.y = 1.0 - _18109;
                                        _23090 = _22545;
                                    }
                                    else
                                    {
                                        _23090 = _18110;
                                    }
                                    vec2 _23091;
                                    if (_172.gConv.x > 0.5)
                                    {
                                        vec2 _22551 = _18072;
                                        _22551.y = 1.0 - _18072.y;
                                        _23091 = _22551;
                                    }
                                    else
                                    {
                                        _23091 = _18072;
                                    }
                                    _23104 = (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23088, 0.0) * _18049.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23089, 0.0) * _18052.x)) * _18049.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23090, 0.0) * _18049.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23091, 0.0) * _18052.x)) * _18052.y);
                                }
                                else
                                {
                                    vec4 _23105;
                                    if (_16343 == 4)
                                    {
                                        vec2 _18210 = (_7094 * _172.gLevel[4].xy) - vec2(0.5);
                                        vec2 _18212 = floor(_18210);
                                        vec2 _18215 = _18210 - _18212;
                                        vec2 _18218 = _18215 * _18215;
                                        vec2 _18221 = _18218 * _18215;
                                        vec2 _18240 = (((_18221 * 3.0) - (_18218 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _18253 = _18221 * 0.16666667163372039794921875;
                                        vec2 _18256 = (((((-_18221) + (_18218 * 3.0)) - (_18215 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18240;
                                        vec2 _18259 = (((((_18221 * (-3.0)) + (_18218 * 3.0)) + (_18215 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18253;
                                        vec2 _18269 = ((_18212 - vec2(0.5)) + (_18240 / _18256)) * _172.gLevel[4].zw;
                                        vec2 _18279 = ((_18212 + vec2(1.5)) + (_18253 / _18259)) * _172.gLevel[4].zw;
                                        vec2 _23084;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22558 = _18269;
                                            _22558.y = 1.0 - _18269.y;
                                            _23084 = _22558;
                                        }
                                        else
                                        {
                                            _23084 = _18269;
                                        }
                                        vec2 _18300 = vec2(_18279.x, _18269.y);
                                        vec2 _23085;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22564 = _18300;
                                            _22564.y = 1.0 - _18269.y;
                                            _23085 = _22564;
                                        }
                                        else
                                        {
                                            _23085 = _18300;
                                        }
                                        float _18316 = _18279.y;
                                        vec2 _18317 = vec2(_18269.x, _18316);
                                        vec2 _23086;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22571 = _18317;
                                            _22571.y = 1.0 - _18316;
                                            _23086 = _22571;
                                        }
                                        else
                                        {
                                            _23086 = _18317;
                                        }
                                        vec2 _23087;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22577 = _18279;
                                            _22577.y = 1.0 - _18279.y;
                                            _23087 = _22577;
                                        }
                                        else
                                        {
                                            _23087 = _18279;
                                        }
                                        _23105 = (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23084, 0.0) * _18256.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23085, 0.0) * _18259.x)) * _18256.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23086, 0.0) * _18256.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23087, 0.0) * _18259.x)) * _18259.y);
                                    }
                                    else
                                    {
                                        vec2 _18417 = (_7094 * _172.gLevel[5].xy) - vec2(0.5);
                                        vec2 _18419 = floor(_18417);
                                        vec2 _18422 = _18417 - _18419;
                                        vec2 _18425 = _18422 * _18422;
                                        vec2 _18428 = _18425 * _18422;
                                        vec2 _18447 = (((_18428 * 3.0) - (_18425 * 6.0)) + vec2(4.0)) * 0.16666667163372039794921875;
                                        vec2 _18460 = _18428 * 0.16666667163372039794921875;
                                        vec2 _18463 = (((((-_18428) + (_18425 * 3.0)) - (_18422 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18447;
                                        vec2 _18466 = (((((_18428 * (-3.0)) + (_18425 * 3.0)) + (_18422 * 3.0)) + vec2(1.0)) * 0.16666667163372039794921875) + _18460;
                                        vec2 _18476 = ((_18419 - vec2(0.5)) + (_18447 / _18463)) * _172.gLevel[5].zw;
                                        vec2 _18486 = ((_18419 + vec2(1.5)) + (_18460 / _18466)) * _172.gLevel[5].zw;
                                        vec2 _23080;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22584 = _18476;
                                            _22584.y = 1.0 - _18476.y;
                                            _23080 = _22584;
                                        }
                                        else
                                        {
                                            _23080 = _18476;
                                        }
                                        vec2 _18507 = vec2(_18486.x, _18476.y);
                                        vec2 _23081;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22590 = _18507;
                                            _22590.y = 1.0 - _18476.y;
                                            _23081 = _22590;
                                        }
                                        else
                                        {
                                            _23081 = _18507;
                                        }
                                        float _18523 = _18486.y;
                                        vec2 _18524 = vec2(_18476.x, _18523);
                                        vec2 _23082;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22597 = _18524;
                                            _22597.y = 1.0 - _18523;
                                            _23082 = _22597;
                                        }
                                        else
                                        {
                                            _23082 = _18524;
                                        }
                                        vec2 _23083;
                                        if (_172.gConv.x > 0.5)
                                        {
                                            vec2 _22603 = _18486;
                                            _22603.y = 1.0 - _18486.y;
                                            _23083 = _22603;
                                        }
                                        else
                                        {
                                            _23083 = _18486;
                                        }
                                        _23105 = (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23080, 0.0) * _18463.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23081, 0.0) * _18466.x)) * _18463.y) + (((textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23082, 0.0) * _18463.x) + (textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23083, 0.0) * _18466.x)) * _18466.y);
                                    }
                                    _23104 = _23105;
                                }
                                _23103 = _23104;
                            }
                            _23102 = _23103;
                        }
                        _23101 = _23102;
                    }
                    _23106 = mix(_23075.xyz, _23101.xyz, vec3(_16330));
                }
                else
                {
                    _23106 = _23075.xyz;
                }
                _23661 = _23106;
            }
            vec3 _24152;
            SPIRV_CROSS_BRANCH
            if (_6966 > 0.0)
            {
                vec2 _7115 = _7024 + (((_23052 * min(_6957 * 0.5, 16.0)) * _172.gDisplay.zw) * _172.gTarget.zw);
                float _18614 = clamp(log2(max(16.0 * _172.gDisplay.z, 1.0)) - 1.0, 0.0, 5.0);
                int _18617 = int(floor(_18614));
                float _18621 = _18614 - float(_18617);
                vec2 _23639;
                if (_172.gConv.x > 0.5)
                {
                    vec2 _22608 = _7115;
                    _22608.y = 1.0 - _7115.y;
                    _23639 = _22608;
                }
                else
                {
                    _23639 = _7115;
                }
                vec4 _23640;
                SPIRV_CROSS_BRANCH
                if (_18617 <= 0)
                {
                    _23640 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23639, 0.0);
                }
                else
                {
                    vec4 _23641;
                    if (_18617 == 1)
                    {
                        _23641 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23639, 0.0);
                    }
                    else
                    {
                        vec4 _23642;
                        if (_18617 == 2)
                        {
                            _23642 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23639, 0.0);
                        }
                        else
                        {
                            vec4 _23643;
                            if (_18617 == 3)
                            {
                                _23643 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23639, 0.0);
                            }
                            else
                            {
                                vec4 _23644;
                                if (_18617 == 4)
                                {
                                    _23644 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23639, 0.0);
                                }
                                else
                                {
                                    _23644 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23639, 0.0);
                                }
                                _23643 = _23644;
                            }
                            _23642 = _23643;
                        }
                        _23641 = _23642;
                    }
                    _23640 = _23641;
                }
                vec3 _23651;
                SPIRV_CROSS_BRANCH
                if ((_18621 > 0.0199999995529651641845703125) && (_18617 < 5))
                {
                    int _18634 = _18617 + 1;
                    vec2 _23645;
                    if (_172.gConv.x > 0.5)
                    {
                        vec2 _22611 = _7115;
                        _22611.y = 1.0 - _7115.y;
                        _23645 = _22611;
                    }
                    else
                    {
                        _23645 = _7115;
                    }
                    vec4 _23646;
                    SPIRV_CROSS_BRANCH
                    if (_18634 <= 0)
                    {
                        _23646 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23645, 0.0);
                    }
                    else
                    {
                        vec4 _23647;
                        if (_18634 == 1)
                        {
                            _23647 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23645, 0.0);
                        }
                        else
                        {
                            vec4 _23648;
                            if (_18634 == 2)
                            {
                                _23648 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23645, 0.0);
                            }
                            else
                            {
                                vec4 _23649;
                                if (_18634 == 3)
                                {
                                    _23649 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23645, 0.0);
                                }
                                else
                                {
                                    vec4 _23650;
                                    if (_18634 == 4)
                                    {
                                        _23650 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23645, 0.0);
                                    }
                                    else
                                    {
                                        _23650 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23645, 0.0);
                                    }
                                    _23649 = _23650;
                                }
                                _23648 = _23649;
                            }
                            _23647 = _23648;
                        }
                        _23646 = _23647;
                    }
                    _23651 = mix(_23640.xyz, _23646.xyz, vec3(_18621));
                }
                else
                {
                    _23651 = _23640.xyz;
                }
                _24152 = _23651;
            }
            else
            {
                _24152 = vec3(0.5);
            }
            float _23682;
            SPIRV_CROSS_BRANCH
            if (esia_v5.x > 0.001000000047497451305389404296875)
            {
                int _18801 = clamp(int(roundEven(log2(36.0 * _172.gDisplay.z) - 1.0)), 1, 4);
                vec2 _23652;
                if (_172.gConv.x > 0.5)
                {
                    vec2 _22615 = _7024;
                    _22615.y = 1.0 - _7024.y;
                    _23652 = _22615;
                }
                else
                {
                    _23652 = _7024;
                }
                vec4 _23653;
                SPIRV_CROSS_BRANCH
                if (_18801 <= 0)
                {
                    _23653 = textureLod(SPIRV_Cross_CombinedgBackdrop0gLinear, _23652, 0.0);
                }
                else
                {
                    vec4 _23654;
                    if (_18801 == 1)
                    {
                        _23654 = textureLod(SPIRV_Cross_CombinedgBackdrop1gLinear, _23652, 0.0);
                    }
                    else
                    {
                        vec4 _23655;
                        if (_18801 == 2)
                        {
                            _23655 = textureLod(SPIRV_Cross_CombinedgBackdrop2gLinear, _23652, 0.0);
                        }
                        else
                        {
                            vec4 _23656;
                            if (_18801 == 3)
                            {
                                _23656 = textureLod(SPIRV_Cross_CombinedgBackdrop3gLinear, _23652, 0.0);
                            }
                            else
                            {
                                vec4 _23657;
                                if (_18801 == 4)
                                {
                                    _23657 = textureLod(SPIRV_Cross_CombinedgBackdrop4gLinear, _23652, 0.0);
                                }
                                else
                                {
                                    _23657 = textureLod(SPIRV_Cross_CombinedgBackdrop5gLinear, _23652, 0.0);
                                }
                                _23656 = _23657;
                            }
                            _23655 = _23656;
                        }
                        _23654 = _23655;
                    }
                    _23653 = _23654;
                }
                _23682 = dot(_23653.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
            }
            else
            {
                _23682 = 0.5;
            }
            _24150 = _24152;
            _23681 = _23682;
            _23658 = _23661;
        }
        else
        {
            _24150 = vec3(0.5);
            _23681 = 0.5;
            _23658 = vec3(0.5);
        }
        vec3 _7143 = mix(vec3(dot(_23658, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875))), _23658, vec3(_6837.x)) + vec3(_6837.y);
        vec3 _23910;
        SPIRV_CROSS_BRANCH
        if (esia_v5.x > 0.001000000047497451305389404296875)
        {
            float _7153 = clamp(max(_23681 + _6837.y, 0.001000000047497451305389404296875), 0.0, 1.0);
            float _7161 = mix(_7153, dot(_6796.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)), esia_v5.x);
            vec3 _7169 = _7143 * (_7161 / _7153);
            vec3 _7179 = mix(_7143, vec3(1.0), vec3((_7161 - _7153) / max(1.0 - _7153, 0.001000000047497451305389404296875)));
            bvec3 _7180 = bvec3(_7161 < _7153);
            _23910 = vec3(_7180.x ? _7169.x : _7179.x, _7180.y ? _7169.y : _7179.y, _7180.z ? _7169.z : _7179.z);
        }
        else
        {
            _23910 = _7143;
        }
        vec2 _7199 = clamp((esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875)), vec2(0.0), vec2(1.0));
        float _7231 = pow(1.0 - _23914.z, 5.0);
        vec2 _7247 = vec2(cos(_6837.w), sin(_6837.w));
        float _7250 = dot(_23052, _7247);
        float _7285 = clamp(0.5 + (0.5 * dot((esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5)) / _6949, _7247)), 0.0, 1.0);
        _25965 = ((_7231 * (pow(clamp(_7250, 0.0, 1.0), 1.5) + (0.4000000059604644775390625 * pow(clamp(-_7250, 0.0, 1.0), 1.5)))) * _6837.z) * 1.60000002384185791015625;
        _25320 = _6878;
        _25307 = vec4((mix(mix(_23910, _6796.xyz, vec3(clamp(_6796.w * ((0.7200000286102294921875 + (0.550000011920928955078125 * (1.0 - _6972))) + (0.3499999940395355224609375 * ((dot(_6796.xyz, vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875)) > 0.5) ? (1.0 - _7199.y) : _7199.y))), 0.0, 1.0))), (_24150 * 1.10000002384185791015625) + vec3(0.07999999821186065673828125), vec3(_7231 * 0.3499999940395355224609375)) + vec3((((0.039999999105930328369140625 * _7285) * _7285) + (0.0500000007450580596923828125 * (1.0 - _6972))) * _6837.z)) * _3903, _3903) + (_24399 * (1.0 - _3903));
    }
    else
    {
        _25965 = 0.0;
        _25320 = _22845;
        _25307 = _24399;
    }
    vec4 _25317;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 1u) != 0u)
    {
        vec4 _24674;
        vec4 _24984;
        SPIRV_CROSS_BRANCH
        if (esia_v7.y != 0u)
        {
            uint _18908 = max(uint(_172.gConv.z), 1u);
            uint _18949 = max(uint(_172.gConv.z), 1u);
            _24984 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _18908) * 24u) + 3u), int(esia_v1 / _18908), 0).xy, 0);
            _24674 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _18949) * 24u) + 4u), int(esia_v1 / _18949), 0).xy, 0);
        }
        else
        {
            _24984 = vec4(0.0);
            _24674 = vec4(0.0);
        }
        vec4 _25302;
        do
        {
            if (esia_v7.y == 0u)
            {
                _25302 = esia_v4;
                break;
            }
            vec2 _19021 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            vec2 _19031 = fwidth(esia_v0);
            float _19033 = max(length(_19031), 9.9999997473787516355514526367188e-05);
            float _25294;
            float _25298;
            if ((esia_v7.y == 1u) || (esia_v7.y == 4u))
            {
                float _19042 = cos(_24674.x);
                float _19045 = sin(_24674.x);
                float _19071 = ((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_19042, _19045)) / max(0.5 * ((abs(_19042) * _19021.x) + (abs(_19045) * _19021.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5;
                if (esia_v7.y == 4u)
                {
                    float _19086 = clamp((_19071 - _24674.y) / max(_24674.z - _24674.y, 0.001000000047497451305389404296875), 0.0, 1.0);
                    _25302 = vec4(clamp(abs((fract(vec3(_19086 * 0.800000011920928955078125) + vec3(1.0, 0.66670000553131103515625, 0.33329999446868896484375)) * 6.0) - vec3(3.0)) - vec3(1.0), vec3(0.0), vec3(1.0)), esia_v4.w * pow(max(sin(_19086 * 3.1415927410125732421875), 0.0), 0.60000002384185791015625));
                    break;
                }
                _25298 = -1.0;
                _25294 = _19071;
            }
            else
            {
                float _25295;
                float _25299;
                if (esia_v7.y == 2u)
                {
                    _25299 = -1.0;
                    _25295 = length(esia_v0 - (esia_v2.xy + (_24674.xy * _19021))) / max(_24674.z * max(_19021.x, _19021.y), 0.001000000047497451305389404296875);
                }
                else
                {
                    vec2 _19154 = esia_v0 - (esia_v2.xy + (_24674.xy * _19021));
                    float _19165 = fract(((atan(_19154.y, _19154.x) - _24674.z) * 0.15915493667125701904296875) + 1.0);
                    float _25296;
                    float _25300;
                    if (_24674.w > 0.5)
                    {
                        _25300 = -1.0;
                        _25296 = 0.5 - (0.5 * cos(_19165 * 6.283185482025146484375));
                    }
                    else
                    {
                        float _19185 = (((_19165 < 0.5) ? _19165 : (_19165 - 1.0)) * 6.283185482025146484375) * length(_19154);
                        float _25301;
                        if (abs(_19185) < _19033)
                        {
                            _25301 = clamp(((_19185 / _19033) * 0.5) + 0.5, 0.0, 1.0);
                        }
                        else
                        {
                            _25301 = -1.0;
                        }
                        _25300 = _25301;
                        _25296 = _19165;
                    }
                    _25299 = _25300;
                    _25295 = _25296;
                }
                _25298 = _25299;
                _25294 = _25295;
            }
            vec4 _19214 = vec4(esia_v4.xyz * esia_v4.w, esia_v4.w);
            vec4 _19226 = vec4(_24984.xyz * _24984.w, _24984.w);
            vec4 _19233 = mix(_19226, _19214, vec4(_25298));
            vec4 _19238 = mix(_19214, _19226, vec4(clamp(_25294, 0.0, 1.0)));
            bvec4 _19239 = bvec4(_25298 >= 0.0);
            vec4 _19240 = vec4(_19239.x ? _19233.x : _19238.x, _19239.y ? _19233.y : _19238.y, _19239.z ? _19233.z : _19238.z, _19239.w ? _19233.w : _19238.w);
            vec4 _19255 = vec4(_19240.xyz / vec3(_19240.w), _19240.w);
            bvec4 _19256 = bvec4(_19240.w > 9.9999997473787516355514526367188e-06);
            _25302 = vec4(_19256.x ? _19255.x : vec4(0.0).x, _19256.y ? _19255.y : vec4(0.0).y, _19256.z ? _19255.z : vec4(0.0).z, _19256.w ? _19255.w : vec4(0.0).w);
            break;
        } while(false);
        vec4 _25303;
        SPIRV_CROSS_BRANCH
        if ((esia_v7.x & 64u) != 0u)
        {
            uint _19266 = max(uint(_172.gConv.z), 1u);
            vec4 _19300 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19266) * 24u) + 18u), int(esia_v1 / _19266), 0).xy, 0);
            _25303 = _25302 * texture(SPIRV_Cross_CombinedgTexgLinear, mix(_19300.xy, _19300.zw, (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875))));
        }
        else
        {
            _25303 = _25302;
        }
        float _19308 = clamp(_25303.w * _3903, 0.0, 1.0);
        _25317 = vec4(_25303.xyz * _19308, _19308) + (_25307 * (1.0 - _19308));
    }
    else
    {
        _25317 = _25307;
    }
    vec4 _25951;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 16384u) != 0u)
    {
        uint _19333 = max(uint(_172.gConv.z), 1u);
        vec4 _19367 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19333) * 24u) + 21u), int(esia_v1 / _19333), 0).xy, 0);
        uint _19374 = max(uint(_172.gConv.z), 1u);
        vec4 _19408 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19374) * 24u) + 3u), int(esia_v1 / _19374), 0).xy, 0);
        vec2 _4342 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        vec2 _4349 = (esia_v0 - esia_v2.xy) / _4342;
        float _4360 = ((_19408.x >= 0.0) ? _19408.x : _172.gTime.x) * _19367.z;
        float _4363 = _4349.x * 2.0;
        float _4364 = _4363 - 1.0;
        float _4369 = _4349.y * _4342.y;
        float _4372 = max(_19367.w, 0.001000000047497451305389404296875);
        float _4377 = clamp(1.0 - (_4364 * _4364), 0.0, 1.0);
        float _4379 = pow(_4377, 1.2999999523162841796875);
        float _4381 = pow(_4377, 0.699999988079071044921875);
        float _4384 = clamp(_4360 * 1.4285714626312255859375, 0.0, 1.0);
        float _4400 = (0.25 + (0.75 * ((_4384 * _4384) * (3.0 - (2.0 * _4384))))) * (0.85000002384185791015625 + (0.1500000059604644775390625 * sin(_4360 * 2.099999904632568359375)));
        float _4409 = ((_19367.y * _4342.y) * _4400) * _4379;
        float _4435 = (_4342.y * (0.5 + ((_19367.x * (0.5 - (_4364 * _4364))) * 0.5))) + (((0.14000000059604644775390625 * _4342.y) * _4379) * sin(((_4364 * 2.400000095367431640625) - (_4360 * 1.2000000476837158203125)) + 0.60000002384185791015625));
        float _25313;
        float _25314;
        vec3 _25315;
        _25315 = vec3(0.0);
        _25314 = _4435;
        _25313 = _4435;
        vec3 _4507;
        float _26345;
        float _26346;
        SPIRV_CROSS_UNROLL
        for (int _25312 = 0; _25312 < 4; _25315 = _4507, _25314 = _26346, _25313 = _26345, _25312++)
        {
            float _4459 = _4435 + ((_4409 * _2912[_25312].x) * (0.800000011920928955078125 + (0.20000000298023223876953125 * sin((_4360 * 1.7000000476837158203125) + _2929[_25312].y))));
            _26345 = (_25312 == 0) ? _4459 : _25313;
            _26346 = (_25312 == 2) ? _4459 : _25314;
            float _4474 = _4372 * _2912[_25312].y;
            float _4479 = (_4369 - _4459) / _4474;
            float _4484 = _4372 * _2912[_25312].z;
            vec3 _26319;
            _26319 = vec3(0.0);
            SPIRV_CROSS_UNROLL
            for (int _26318 = 0; _26318 < 6; )
            {
                float _19431 = ((_4369 - _4459) - (_4484 * ((float(_26318) * 0.4000000059604644775390625) - 1.0))) / _4474;
                _26319 += (_1694[_26318] * exp((-_19431) * _19431));
                _26318++;
                continue;
            }
            _4507 = _25315 + (mix(_26319 * vec3(0.237529695034027099609375, 0.24630542099475860595703125, 0.27624309062957763671875), vec3(exp((-_4479) * _4479)), vec3(_2929[_25312].x)) * (_2912[_25312].w * _4381));
        }
        float _4513 = _4372 * 1.5;
        float _4543 = clamp((_4369 - _25313) / max(_25314 - _25313, 0.001000000047497451305389404296875), 0.0, 1.0);
        float _4571 = (_4369 - (_25314 - (_4372 * 3.0))) / (((_4342.y * 0.0900000035762786865234375) + (_4409 * 0.20000000298023223876953125)) + 0.001000000047497451305389404296875);
        float _4574 = (_4363 - 1.0499999523162841796875) * 2.77777767181396484375;
        float _4599 = ((_4369 - _25313) + (_4372 * 5.0)) / (_4372 * 7.0);
        vec3 _4625 = vec3(1.0) - exp((-((((_25315 + (mix(vec3(0.7799999713897705078125, 0.800000011920928955078125, 1.0), vec3(1.0), vec3(_4543)) * ((((1.0 / (1.0 + exp((-((_4369 - _25313) - (_4372 * 2.0))) / _4513))) / (1.0 + exp((-(_25314 - _4369)) / _4513))) * (0.0599999986588954925537109375 + (0.3499999940395355224609375 * pow(_4543, 2.5)))) * _4381))) + (vec3(1.0, 0.980000019073486328125, 0.949999988079071044921875) * (exp(((-_4571) * _4571) - (_4574 * _4574)) * (0.5 + (1.10000002384185791015625 * _4400))))) + (vec3(1.0, 0.680000007152557373046875, 0.4199999868869781494140625) * ((exp((-_4599) * _4599) * _4379) * 0.100000001490116119384765625))) * mix(vec3(1.0, 0.9700000286102294921875, 0.939999997615814208984375), vec3(0.939999997615814208984375, 0.9700000286102294921875, 1.0), vec3(0.5 + (0.5 * sin(_4360 * 0.800000011920928955078125)))))) * 1.39999997615814208984375);
        float _4635 = (esia_v4.w * smoothstep(0.0, 0.119999997317790985107421875, _4349.y)) * smoothstep(1.0, 0.87999999523162841796875, _4349.y);
        float _4654 = (clamp(max(_4625.x, max(_4625.y, _4625.z)), 0.0, 1.0) * _4635) * _3903;
        _25951 = vec4((_4625 * _4635) * _3903, _4654) + (_25317 * (1.0 - _4654));
    }
    else
    {
        _25951 = _25317;
    }
    vec4 _25960;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 4u) != 0u) && ((esia_v7.x & 256u) != 0u))
    {
        uint _19464 = max(uint(_172.gConv.z), 1u);
        vec4 _19498 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19464) * 24u) + 7u), int(esia_v1 / _19464), 0).xy, 0);
        uint _19505 = max(uint(_172.gConv.z), 1u);
        vec4 _19539 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _19505) * 24u) + 8u), int(esia_v1 / _19505), 0).xy, 0);
        vec2 _4688 = esia_v0 - _19539.zw;
        vec2 _19578 = (esia_v2.xy + esia_v2.zw) * 0.5;
        vec2 _19587 = max((esia_v2.zw - esia_v2.xy) * 0.5, vec2(0.001000000047497451305389404296875));
        float _25926;
        SPIRV_CROSS_BRANCH
        if (esia_v7.z == 1u)
        {
            vec2 _19593 = _4688 - _19578;
            float _25925;
            do
            {
                if (esia_v5.w >= 6.282185077667236328125)
                {
                    _25925 = abs(length(_19593) - esia_v3.x) - esia_v3.y;
                    break;
                }
                float _19692 = esia_v5.z + (esia_v5.w * 0.5);
                float _19694 = cos(_19692);
                float _19696 = sin(_19692);
                float _19705 = dot(_19593, vec2(-_19696, _19694));
                float _19708 = dot(_19593, vec2(_19694, _19696));
                vec2 _19709 = vec2(_19705, _19708);
                float _19712 = abs(_19705);
                _19709.x = _19712;
                float _19715 = esia_v5.w * 0.5;
                float _19717 = sin(_19715);
                float _19719 = cos(_19715);
                _25925 = (((_19719 * _19712) > (_19717 * _19708)) ? length(_19709 - (vec2(_19717, _19719) * esia_v3.x)) : abs(length(_19709) - esia_v3.x)) - esia_v3.y;
                break;
            } while(false);
            _25926 = _25925;
        }
        else
        {
            float _25927;
            if (esia_v7.z == 2u)
            {
                vec2 _19755 = _4688 - _22847.xy;
                vec2 _19758 = _22847.zw - _22847.xy;
                _25927 = length(_19755 - (_19758 * clamp(dot(_19755, _19758) / max(dot(_19758, _19758), 9.9999999747524270787835121154785e-07), 0.0, 1.0))) - esia_v3.x;
            }
            else
            {
                vec2 _19620 = _4688 - _19578;
                float _19811 = min(_19587.x, _19587.y);
                float _19814 = min((_19620.x > 0.0) ? ((_19620.y > 0.0) ? esia_v3.z : esia_v3.y) : ((_19620.y > 0.0) ? esia_v3.w : esia_v3.x), _19811);
                float _19820 = _19814 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                float _25910;
                float _25911;
                if (_19820 > _19811)
                {
                    float _19834 = esia_v5.y * clamp((_19811 - _19814) / max(0.60000002384185791015625 * _19814, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                    _25911 = _19834;
                    _25910 = _19814 * (1.0 + (0.60000002384185791015625 * _19834));
                }
                else
                {
                    _25911 = esia_v5.y;
                    _25910 = _19820;
                }
                vec2 _19847 = (abs(_19620) - _19587) + vec2(_25910);
                vec2 _19849 = max(_19847, vec2(0.0));
                float _25912;
                SPIRV_CROSS_BRANCH
                if ((_19849.x > 0.0) && (_19849.y > 0.0))
                {
                    float _25913;
                    if ((_25911 > 0.001000000047497451305389404296875) && (_25910 > 9.9999997473787516355514526367188e-05))
                    {
                        float _19866 = 2.0 + (2.0 * _25911);
                        vec2 _19871 = _19849 / vec2(max(_25910, 9.9999997473787516355514526367188e-05));
                        _25913 = pow(pow(_19871.x, _19866) + pow(_19871.y, _19866), 1.0 / _19866) * _25910;
                    }
                    else
                    {
                        _25913 = length(_19849);
                    }
                    _25912 = _25913;
                }
                else
                {
                    _25912 = max(_19849.x, _19849.y);
                }
                float _19906 = (min(max(_19847.x, _19847.y), 0.0) + _25912) - _25910;
                float _25928;
                SPIRV_CROSS_BRANCH
                if ((esia_v7.x & 512u) != 0u)
                {
                    vec2 _19648 = max((_22847.zw - _22847.xy) * 0.5, vec2(0.001000000047497451305389404296875));
                    vec2 _19651 = _4688 - ((_22847.xy + _22847.zw) * 0.5);
                    float _19942 = min(_19648.x, _19648.y);
                    float _19945 = min((_19651.x > 0.0) ? ((_19651.y > 0.0) ? _25320.x : _25320.x) : ((_19651.y > 0.0) ? _25320.x : _25320.x), _19942);
                    float _19951 = _19945 * (1.0 + (0.60000002384185791015625 * esia_v5.y));
                    float _25916;
                    float _25917;
                    if (_19951 > _19942)
                    {
                        float _19965 = esia_v5.y * clamp((_19942 - _19945) / max(0.60000002384185791015625 * _19945, 9.9999997473787516355514526367188e-05), 0.0, 1.0);
                        _25917 = _19965;
                        _25916 = _19945 * (1.0 + (0.60000002384185791015625 * _19965));
                    }
                    else
                    {
                        _25917 = esia_v5.y;
                        _25916 = _19951;
                    }
                    vec2 _19978 = (abs(_19651) - _19648) + vec2(_25916);
                    vec2 _19980 = max(_19978, vec2(0.0));
                    float _25918;
                    SPIRV_CROSS_BRANCH
                    if ((_19980.x > 0.0) && (_19980.y > 0.0))
                    {
                        float _25919;
                        if ((_25917 > 0.001000000047497451305389404296875) && (_25916 > 9.9999997473787516355514526367188e-05))
                        {
                            float _19997 = 2.0 + (2.0 * _25917);
                            vec2 _20002 = _19980 / vec2(max(_25916, 9.9999997473787516355514526367188e-05));
                            _25919 = pow(pow(_20002.x, _19997) + pow(_20002.y, _19997), 1.0 / _19997) * _25916;
                        }
                        else
                        {
                            _25919 = length(_19980);
                        }
                        _25918 = _25919;
                    }
                    else
                    {
                        _25918 = max(_19980.x, _19980.y);
                    }
                    float _20037 = (min(max(_19978.x, _19978.y), 0.0) + _25918) - _25916;
                    float _20042 = max(_25320.y, 9.9999997473787516355514526367188e-05);
                    float _20051 = max(_20042 - abs(_19906 - _20037), 0.0) / _20042;
                    _25928 = min(_19906, _20037) - (((_20051 * _20051) * _20042) * 0.25);
                }
                else
                {
                    _25928 = _19906;
                }
                _25927 = _25928;
            }
            _25926 = _25927;
        }
        float _4697 = (_25926 + _19539.y) / (max(_19539.x * 0.5, _3878 * 0.5) * 1.41421353816986083984375);
        float _20068 = sign(_4697);
        float _20070 = abs(_4697);
        float _20081 = 1.0 + ((0.2783930003643035888671875 + ((0.23038899898529052734375 + (0.07810799777507781982421875 * (_20070 * _20070))) * _20070)) * _20070);
        float _20084 = _20081 * _20081;
        float _20099 = clamp(_19498.w * ((0.5 + (0.5 * (_20068 - (_20068 / (_20084 * _20084))))) * _3903), 0.0, 1.0);
        _25960 = vec4(_19498.xyz * _20099, _20099) + (_25951 * (1.0 - _20099));
    }
    else
    {
        _25960 = _25951;
    }
    vec4 _25984;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 16u) != 0u)
    {
        uint _20124 = max(uint(_172.gConv.z), 1u);
        vec4 _20158 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _20124) * 24u) + 9u), int(esia_v1 / _20124), 0).xy, 0);
        uint _20165 = max(uint(_172.gConv.z), 1u);
        vec4 _20199 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _20165) * 24u) + 10u), int(esia_v1 / _20165), 0).xy, 0);
        float _4729 = max(-_22865, 0.0) / max(_20199.z, 0.001000000047497451305389404296875);
        float _20207 = clamp(_20158.w * clamp((exp(((-_4729) * _4729) * 2.2000000476837158203125) * _20199.w) * _3903, 0.0, 1.0), 0.0, 1.0);
        _25984 = vec4(_20158.xyz * _20207, _20207) + (_25960 * (1.0 - _20207));
    }
    else
    {
        _25984 = _25960;
    }
    vec3 _4758 = _25984.xyz + vec3((_25965 * _3903) * _25984.w);
    vec4 _22741 = _25984;
    _22741.x = _4758.x;
    _22741.y = _4758.y;
    _22741.z = _4758.z;
    vec4 _25987;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 2u) != 0u)
    {
        uint _20232 = max(uint(_172.gConv.z), 1u);
        vec4 _20266 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _20232) * 24u) + 5u), int(esia_v1 / _20232), 0).xy, 0);
        uint _20273 = max(uint(_172.gConv.z), 1u);
        vec4 _20307 = texelFetch(SPIRV_Cross_CombinedgFxDataSPIRV_Cross_DummySampler, ivec3(int(((esia_v1 % _20273) * 24u) + 6u), int(esia_v1 / _20273), 0).xy, 0);
        float _4778 = _20307.x;
        float _4780 = _20307.y;
        vec4 _25985;
        if (_20307.z < 0.999000012874603271484375)
        {
            vec2 _4816 = max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
            float _4827 = cos(_20307.w);
            float _4830 = sin(_20307.w);
            vec4 _22758 = _20266;
            _22758.w = _20266.w * mix(1.0, _20307.z, clamp(((dot(esia_v0 - ((esia_v2.xy + esia_v2.zw) * 0.5), vec2(_4827, _4830)) / max(0.5 * ((abs(_4827) * _4816.x) + (abs(_4830) * _4816.y)), 0.001000000047497451305389404296875)) * 0.5) + 0.5, 0.0, 1.0));
            _25985 = _22758;
        }
        else
        {
            _25985 = _20266;
        }
        float _20315 = clamp(_25985.w * (clamp(0.5 - ((_22865 - (_4778 * _4780)) / _3878), 0.0, 1.0) - clamp(0.5 - ((_22865 + (_4778 * (1.0 - _4780))) / _3878), 0.0, 1.0)), 0.0, 1.0);
        _25987 = vec4(_25985.xyz * _20315, _20315) + (_22741 * (1.0 - _20315));
    }
    else
    {
        _25987 = _22741;
    }
    vec4 _25988;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 128u) != 0u)
    {
        vec2 _4891 = (esia_v0 - esia_v2.xy) / max(esia_v2.zw - esia_v2.xy, vec2(0.001000000047497451305389404296875));
        float _4913 = exp(-pow((((_4891.x * 0.85000002384185791015625) + (_4891.y * 0.1500000059604644775390625)) - ((fract(_172.gTime.x * esia_v6.w) * 1.7999999523162841796875) - 0.4000000059604644775390625)) * 9.09090900421142578125, 2.0));
        _25988 = vec4(_25987.xyz + vec3(((_4913 * esia_v6.z) * _3903) * max(_25987.w, 0.3499999940395355224609375)), max(_25987.w, ((_4913 * esia_v6.z) * _3903) * 0.5));
    }
    else
    {
        _25988 = _25987;
    }
    vec4 _26313;
    SPIRV_CROSS_BRANCH
    if ((esia_v7.x & 2048u) != 0u)
    {
        vec3 _20340 = fract(floor(_22844).xyx * 0.103100001811981201171875);
        vec3 _20349 = _20340 + vec3(dot(_20340, _20340.yzx + vec3(33.3300018310546875)));
        vec3 _4964 = _25988.xyz + vec3(((fract((_20349.x + _20349.y) * _20349.z) - 0.5) * esia_v6.y) * _25988.w);
        vec4 _22782 = _25988;
        _22782.x = _4964.x;
        _22782.y = _4964.y;
        _22782.z = _4964.z;
        _26313 = _22782;
    }
    else
    {
        _26313 = _25988;
    }
    float _26309;
    if (_215.gFade.z > 0.0)
    {
        _26309 = smoothstep(0.0, 1.0, clamp((_22844.y - _215.gFade.x) / _215.gFade.z, 0.0, 1.0));
    }
    else
    {
        _26309 = 1.0;
    }
    float _26310;
    if (_215.gFade.w > 0.0)
    {
        _26310 = _26309 * smoothstep(0.0, 1.0, clamp((_215.gFade.y - _22844.y) / _215.gFade.w, 0.0, 1.0));
    }
    else
    {
        _26310 = _26309;
    }
    vec4 _4981 = _26313 * ((esia_v6.x * _25999) * _26310);
    vec4 _26314;
    SPIRV_CROSS_BRANCH
    if (((esia_v7.x & 8u) != 0u) || ((esia_v7.x & 4u) != 0u))
    {
        vec3 _20401 = fract((floor(_22844) + vec2(17.0)).xyx * 0.103100001811981201171875);
        vec3 _20410 = _20401 + vec3(dot(_20401, _20401.yzx + vec3(33.3300018310546875)));
        vec3 _5005 = _4981.xyz + vec3(((fract((_20410.x + _20410.y) * _20410.z) - 0.5) * 0.0039215688593685626983642578125) * clamp(_4981.w * 8.0, 0.0, 1.0));
        vec4 _22794 = _4981;
        _22794.x = _5005.x;
        _22794.y = _5005.y;
        _22794.z = _5005.z;
        _26314 = _22794;
    }
    else
    {
        _26314 = _4981;
    }
    vec3 _5015 = max(_26314.xyz, vec3(0.0));
    vec4 _22800 = _26314;
    _22800.x = _5015.x;
    _22800.y = _5015.y;
    _22800.z = _5015.z;
    vec4 _26315;
    if ((_172.gTime.w > 0.5) && (_26314.w > 9.9999997473787516355514526367188e-06))
    {
        vec3 _20454 = clamp(_22800.xyz / vec3(_26314.w), vec3(0.0), vec3(1.0));
        vec3 _20460 = pow((_20454 + vec3(0.054999999701976776123046875)) * vec3(0.947867333889007568359375), vec3(2.400000095367431640625));
        vec3 _20463 = _20454 * vec3(0.077399380505084991455078125);
        bvec3 _20465 = lessThanEqual(_20454, vec3(0.040449999272823333740234375));
        vec3 _20440 = vec3(_20465.x ? _20463.x : _20460.x, _20465.y ? _20463.y : _20460.y, _20465.z ? _20463.z : _20460.z) * _26314.w;
        vec4 _22809 = _22800;
        _22809.x = _20440.x;
        _22809.y = _20440.y;
        _22809.z = _20440.z;
        _26315 = _22809;
    }
    else
    {
        _26315 = _22800;
    }
    _entryPointOutput = _26315;
}

