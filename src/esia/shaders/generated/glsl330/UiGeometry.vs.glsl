#version 330

layout(std140) uniform WgtFrame
{
    vec4 gXform;
    vec4 gTarget;
    vec4 gDisplay;
    vec4 gTime;
    vec4 gLevel[6];
    vec4 gText;
    vec4 gConv;
} _28;

layout(location = 0) in vec2 v_pos;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec4 v_col;
out vec4 esia_v0;
out vec2 esia_v1;

void main()
{
    gl_Position = vec4((v_pos * _28.gXform.xy) + _28.gXform.zw, 0.0, 1.0);
    esia_v0 = v_col;
    esia_v1 = v_uv;
}

