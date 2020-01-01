#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 4) uniform image2D final_image;
layout(rgba32f, binding = 0) uniform image2D frame;

uniform int iterations;

void main() 
{
  ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
  vec4 color = imageLoad(frame, uv);
  
  vec3 cur_color = imageLoad(final_image, uv).rgb;

  color.rgb = cur_color + ((color.rgb - cur_color) / float(iterations));

  imageStore(final_image, uv, color);
}