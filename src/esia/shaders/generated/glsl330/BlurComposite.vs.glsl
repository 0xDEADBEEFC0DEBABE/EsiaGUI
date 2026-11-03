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
} _45;

out vec2 esia_v0;

void main()
{
    vec2 _119 = vec2(((uint(gl_VertexID) & 1u) != 0u) ? 2.0 : 0.0, ((uint(gl_VertexID) & 2u) != 0u) ? 2.0 : 0.0);
    gl_Position = vec4((_119 * vec2(2.0, (_45.gXform.y < 0.0) ? (-2.0) : 2.0)) + vec2(-1.0, (_45.gXform.y < 0.0) ? 1.0 : (-1.0)), 0.0, 1.0);
    esia_v0 = _119;
}

