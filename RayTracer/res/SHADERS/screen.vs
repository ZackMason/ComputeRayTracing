#version 430 core
layout(location=0)in vec2 aPos;
layout(location=1)in vec2 aUV;

out vec2 voUV;
out vec2 voPos;

void main()
{
    voUV = aUV;
    voPos = aPos;
    gl_Position = vec4(aPos.x, aPos.y, .0, 1.);
}