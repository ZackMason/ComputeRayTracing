#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;
layout(rgba32f, binding = 1) uniform image2D ray_ori;
layout(rgba32f, binding = 2) uniform image2D ray_dir;

#define M_PI 3.14159

uniform float time;
uniform int depth;

struct material
{
  vec3 albedo;
  float fuzz;
  float fade;
  float ior;
};

struct sphere
{
  vec4 sph;
  material mat;
};

struct box
{
  vec3 min;
  vec3 max;
  material mat;
};

float rand(vec2 co) // rand float 0 - 1
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 rand3(vec2 co)
{
    return vec3(rand(co), rand(co*37.), rand(co*73.));
}

vec3 rand3n(vec2 co)
{
    return normalize(vec3(rand(co), rand(co*37.), rand(co*73.)));
}

vec2 get_sphere_uv(vec3 npos)
{
  float phi = atan(npos.z, npos.x);
  float theta = asin(npos.y);
  return vec2(1-(phi + M_PI) / (2.*M_PI), 
  (theta + M_PI/2.) / M_PI );
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

bool zrefract(vec3 v, vec3 n, float ni_over_nt, inout vec3 refracted) {
    vec3 uv = normalize(v);
    float dt = dot(uv, n);
    float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
    if (discriminant > 0) {
        refracted = ni_over_nt*(uv - n*dt) - n*sqrt(discriminant);
        return true;
    }
    else
        return false;
}

float schlick(float cosine, float ref_idx) 
{
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

bool scatter(vec3 ho, vec3 rd,vec3 hn, inout material mat, inout vec3 scatter_o, inout vec3 scatter_d)
{
  vec3 t = ho + hn + normalize(rand3(vec2(mod(time,303.) * gl_GlobalInvocationID.xy))*2. - 1.);
  scatter_o = ho;
  if (abs(1.-mat.ior) > 0.01 && mat.ior > 0.)
  {
    vec3 o_n;
    vec3 ref = reflect(rd,hn);
    float ni_over_nt;
    mat.albedo = vec3(1);
    float reflect_prob;
    float cosine;

    if (dot(rd,hn) > 0)
    {
      o_n = -hn;
      ni_over_nt = mat.ior;
      cosine = mat.ior * dot(rd,hn) / length(rd);
    }
    else
    {
      o_n = hn;
      ni_over_nt = 1. / mat.ior;
      cosine = -dot(rd,hn) / length(rd);

    }
    vec3 refracted;
    if (zrefract(rd,o_n,ni_over_nt,refracted))
    {
      reflect_prob = schlick(cosine, mat.ior);
    }
    else
    {
      reflect_prob = 1.0;
    }
    if (rand(gl_GlobalInvocationID.xy + time) < reflect_prob)
    {
      scatter_o = ho;
      scatter_d = normalize(reflect(rd,hn));
    }
    else
    {
      scatter_o = ho;
      scatter_d = normalize(refracted);
    }

  }
  else if ( mat.fuzz > 1.) // diffuse
  {
    if (mat.ior < 0.) // flag for lights
    {
      scatter_d = vec3(1337);
      //mat.albedo += mat.albedo;
    }
    else
    {
      scatter_d = normalize(t - ho);
      mat.albedo *= .5;
    }
  }
  else // metallic
  {
    scatter_d = reflect(normalize(rd),hn);
    scatter_d += mat.fuzz * normalize(rand3(vec2(time * gl_GlobalInvocationID.xy))*2. - 1.);
    scatter_d = normalize(scatter_d);
    mat.albedo *= .75;

    return (dot(scatter_d, hn) > 0);
  }
  return true;
}

float hit_sphere(sphere sph, vec3 ro, vec3 rd);
float hit_box(box bx, vec3 ro, vec3 rd);
vec3 hit_shapes(vec3 ro, vec3 rd, inout vec3 norm, inout material mat);

#define SPHERE_ARRAY_SIZE 8
sphere spheres[SPHERE_ARRAY_SIZE] = sphere[SPHERE_ARRAY_SIZE](
  sphere(vec4( 0.0, -.10, -12.0 , 2.), material(vec3(1.,1.,1.),.27600010,.65,1.)),
  sphere(vec4( 0.0, -1.10, -8.0 , .42), material(0.951*vec3(5.,6.4,14.),1.1336,.5,1.)),
  sphere(vec4( 2.0, -1.10, -8.5 , .5), material(vec3(1.,0.,1.),1.1,.5,1.5)),
  sphere(vec4( 2.0, -1.10, -8.5 , -.47), material(vec3(1.,0.,1.),1.1337,.5,1.5)),
  sphere(vec4( 2.0, -1.10, -8.5 , .125), material(2.*vec3(1.,2.,.5),1.1,.5,-1.)),
  sphere(vec4( 4.30,-.10, -10.0 , 2.), material(vec3(3.,.1,.1),1.1336,.8,-1.)),
  sphere(vec4(-4.30,-.10, -10.0 , 2.), material(vec3(0.,1.,.1),1.10,.5,1.5)),
  sphere(vec4(.0, -1002, 0.0 , 1000.), material(vec3(1.,1.,1.),1.1037,.5,1.))
);

#define BOX_ARRAY_SIZE 5
box boxes[BOX_ARRAY_SIZE] = box[BOX_ARRAY_SIZE](
  box(vec3( -3.,-2.20,  -10.),vec3( -2,-1.5,3),  material(vec3(1.,1.,1.),1.0010,.5,1.5)),
  box(vec3( 2.,-2.20,   -10.),vec3( 3,-1.5,3),   material(vec3(1.,1.,1.),1.0010,.8,1.5)),
  box(vec3( -.5,-2.20,   -7.),vec3( .5,-1.5,-6), material(vec3(1.,1.,.1),1.10,.5,1.)),
  box(vec3( -102.5,12.20,   -20.),vec3( 102.5,15.5,10), material(3.*vec3(1.,1.,.1),1.10,.5,1.)),
  box(vec3( -.15,-2.20,-6.65),vec3( .15,-1.25,-6.35), material(vec3(1.,1.,1.),1.1010,.5,1.5))
);

void main() 
{
  vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
  vec3 cur_color = imageLoad(img_output, pixel_coords).rgb;
  
  //cam
  vec3 lower_left_corner = vec3(0,0,-1);//vec3(-1.0, -1.0, -1.0);
  vec3 horizontal = vec3(1.0, 0.0, 0.0);
  vec3 vertical = vec3(0.0, 1.0, 0.0);
  vec3 origin = vec3(0.0, 0.0, 0.0);

  float max_x = 5.0;
  float max_y = 5.0;
  ivec2 dims = imageSize(img_output); 
  float x = (float(pixel_coords.x * 2 - dims.x) / dims.x);
  float y = (float(pixel_coords.y * 2 - dims.y) / dims.y);

  vec3 randoff = normalize(rand3(vec2(pixel_coords)+ time) * 2. - 1.) * .0020;
  randoff.z = 0;

  x += randoff.x;
  y += randoff.y;

  vec3 ray_o = vec3(x * max_x, y * max_y, 1.0);
  vec3 ray_d = vec3(0.0, 0.0, -1.0); 

 

  if( depth == 0)
  {
    cur_color = vec3(1);
    ray_o = vec3(0);
    ray_d = vec3(lower_left_corner + x*horizontal + y*vertical);
    ray_d = normalize(ray_d);
  }
  else
  {
    ray_o = imageLoad(ray_ori,pixel_coords).xyz;
    ray_d = imageLoad(ray_dir,pixel_coords).xyz;
    if (ray_d == vec3(1337) || ray_o == vec3(1337) )
      return;
  }

  vec3 n;

  material hitMat;
  
  vec3 p = hit_shapes(ray_o, ray_d, n, hitMat);

  if (p != vec3(1337))
  {
    vec3 scatter_o;
    vec3 scatter_d;
    if ( scatter(p,ray_d,n,hitMat,scatter_o,scatter_d))
    {
      pixel.rgb = hitMat.albedo;//* dot(n,normalize(vec3(1,1,1)));
      pixel.rgb = pow(pixel.rgb,vec3(2.2));

      imageStore(ray_ori, pixel_coords, vec4(scatter_o,1.));
      imageStore(ray_dir, pixel_coords, vec4(scatter_d,1.));
    }
    else
    {
      pixel.rgb = vec3(0.0);
      imageStore(ray_dir, pixel_coords, vec4(1337));
    }
  }
  else
  {
    vec3 unit_direction = normalize(ray_d);
    float t = 0.5 * (unit_direction.y + 1.0);
    pixel.rgb = (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
    pixel.rgb = pow(pixel.rgb,vec3(2.2));
    imageStore(ray_dir, pixel_coords, vec4(1337));
  }

  cur_color = max(vec3(0), cur_color);
  pixel.rgb = max(vec3(0), pixel.rgb);

  //pixel.rgb = pow(pixel.rgb, vec3(2.2));

  pixel.rgb = pixel.rgb * cur_color;


  // output to a specific pixel in the image
  imageStore(img_output, pixel_coords, pixel);
}

void swap(inout float a, inout float b)
{
  float t = a;
  a = b;
  b = t;
}

float hit_box(box bx, vec3 ro, vec3 rd)
{
  float tmin = (bx.min.x - ro.x) / rd.x;
  float tmax = (bx.max.x - ro.x) / rd.x;

  if (tmin > tmax) swap(tmin, tmax); 

  float tymin = (bx.min.y - ro.y) / rd.y; 
  float tymax = (bx.max.y - ro.y) / rd.y; 

  if (tymin > tymax) swap(tymin, tymax); 

  if ((tmin > tymax) || (tymin > tmax)) 
      return -1.; 

  if (tymin > tmin) 
      tmin = tymin; 

  if (tymax < tmax) 
      tmax = tymax; 

  float tzmin = (bx.min.z - ro.z) / rd.z; 
  float tzmax = (bx.max.z - ro.z) / rd.z; 

  if (tzmin > tzmax) swap(tzmin, tzmax); 

  if ((tmin > tzmax) || (tzmin > tmax)) 
      return -1.; 

  if (tzmin > tmin) 
      tmin = tzmin; 

  if (tzmax < tmax) 
      tmax = tzmax; 


  return tmin;
}


float hit_sphere(sphere sph, vec3 ro, vec3 rd)
{
  vec3 oc = ro - sph.sph.xyz;
  float a = dot(rd, rd);
	float b = 2.0 * dot(oc, rd);
	float c = dot(oc, oc) - sph.sph.w * sph.sph.w;
	float discriminant = b * b - 4 * a*c;
	if (discriminant < 0) 
  {
		return -1.0;
	}
	else 
  {
		return (-b - sqrt(discriminant)) / (2.0*a);
	}
}

vec3 hit_shapes(vec3 ro, vec3 rd, inout vec3 norm, inout material mat)
{
  float d = 13371337.;
  float min_d = d;
  vec3 p;
  for (int i = 0; i < SPHERE_ARRAY_SIZE; i++)
  {
    d = hit_sphere(spheres[i],ro,rd);
    if ( d >= 0.001 && d < min_d)
    {
      min_d = d;
      p = ro + d * rd;
      norm = normalize(p - spheres[i].sph.xyz);
      vec2 uv = get_sphere_uv(norm);
      float size = 120.;
      float sines = sin(uv.x * size) * sin(uv.y * size);
      if (spheres[i].mat.fuzz == 1.1336)
      {
        if (sines < 0.)
        {
          spheres[i].mat.albedo.rgb *= 0.;
        }
      }
      else if (spheres[i].mat.fuzz == 1.1337)
      {
        vec2 tile = truchetPattern(fract(uv*40.),rand(floor(uv*40.)));
        spheres[i].mat.albedo.rgb *= 
          smoothstep(tile.x-0.3,tile.x,tile.y)-
          smoothstep(tile.x,tile.x+0.3,tile.y);
      }
      mat = spheres[i].mat;
    }
  }
  for (int i = 0; i < BOX_ARRAY_SIZE; i++)
  {
    d = hit_box(boxes[i],ro,rd);
    if ( d >= 0.001 && d < min_d)
    {
      min_d = d;
      p = ro + d * rd;
      float e = 0.00001;
      if(abs(p.x - boxes[i].min.x) < e)
          norm = vec3(-1,0,0);
      else if(abs(p.x - boxes[i].max.x) < e)
          norm = vec3(1,0,0);
      else if(abs(p.y - boxes[i].min.y) < e)
          norm = vec3(0,-1,0);
      else if(abs(p.y - boxes[i].max.y) < e)
          norm = vec3(0,1,0);
      else if(abs(p.z - boxes[i].min.z) < e)
          norm = vec3(0,0,-1);
      else if(abs(p.z - boxes[i].max.z) < e)
          norm = vec3(0,0,1);
      mat = boxes[i].mat;
    }
  }
  if (min_d != 13371337)
      return p;
  return vec3(1337);
}