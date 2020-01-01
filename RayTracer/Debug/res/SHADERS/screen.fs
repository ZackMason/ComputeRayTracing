#version 430 core
out vec4 FragColor;

in vec2 voUV;
in vec2 voPos;

uniform sampler2D uTexture1;
uniform sampler2D uTexture2;

vec3 hash3(float n){return fract(sin(vec3(n,n+1.,n+2.))*43758.5453123);}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 rand3(vec2 co)
{
    return vec3(rand(co), rand(co+1), rand(co+2));
}

void main()
{
    vec3 color = texture(uTexture1,voUV).rgb;
    //if (voUV.x < 1./4. && voUV.y < 1./4.)
        //color = texture(uTexture2,voUV/4.).rgb;

    // HDR tonemapping
    #if 0
    color = pow(color, vec3(2.2));
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));
    #endif
    FragColor = vec4(color,1);
    //FragColor = vec4(color,1);
}