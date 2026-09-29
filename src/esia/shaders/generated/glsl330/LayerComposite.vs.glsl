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
} _40;

out vec2 esia_v0;

void main()
{
    vec2 _116 = vec2(float((uint(gl_VertexID) << uint(1)) & 2u), float(uint(gl_VertexID) & 2u));
    gl_Position = vec4((_116 * vec2(2.0, (_40.gXform.y < 0.0) ? (-2.0) : 2.0)) + vec2(-1.0, (_40.gXform.y < 0.0) ? 1.0 : (-1.0)), 0.0, 1.0);
    esia_v0 = _116;
}

