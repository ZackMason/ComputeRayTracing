#version 430 core
out vec4 FragColor;

in vec2 voUV;
in vec2 voPos;

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;
uniform float window_size;

float rand(vec2 co) // rand float 0 - 1
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 rand3(vec2 co)
{
    return vec3(rand(co), rand(co*37.), rand(co*73.));
}

vec2 truchetPattern(in vec2 _st, in float _index){
    _index = fract(((_index-0.5)*2.0));
    if (_index > 0.75) {
        _st = vec2(1.0) - _st;
    } else if (_index > 0.5) {
        _st = vec2(1.0-_st.x,_st.y);
    } else if (_index > 0.25) {
        _st = 1.0-vec2(1.0-_st.x,_st.y);
    }
    return _st;
}

void main()
{
    vec3 color = texture(uTexture1,voUV).rgb;
    
    // HDR tonemapping
    #if 1
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));
    #endif

    if (voUV.x < 1./window_size && voUV.y < 1./window_size)
        color = (texture(uTexture2,voUV/window_size).rgb);


    FragColor = vec4(color,1);
    //FragColor = vec4(color,1);
}