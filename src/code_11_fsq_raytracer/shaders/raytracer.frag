#version 330 core  
out vec4 color; 

in vec2 pos;


struct Ray {
    vec3 orig;
    vec3 dir;
};

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
};

struct HitInfo {
    float t;
    vec3 p;
    vec3 n;
    bool hit;
};

HitInfo hit_sphere(Ray r, Sphere s) {
    HitInfo hit_info;
    hit_info.t = 1e30;  // BIGT equivalent
    hit_info.hit = false;

    float A = dot(r.dir, r.dir);
    float B = dot(r.dir, r.orig - s.center) * 2.0;
    float C = dot(r.orig - s.center, r.orig - s.center) - s.radius * s.radius;

    float delta = B * B - 4.0 * A * C;

    if (delta < 0.0) {
        return hit_info;  // No hit, return default HitInfo
    }

    float t1 = (-B - sqrt(delta)) / (2.0 * A);
    float t2 = (-B + sqrt(delta)) / (2.0 * A);

    float t_min = min(t1, t2);

    if (t_min < 0.0) {
        t_min = max(t1, t2);
    }

    hit_info.t = t_min;
    hit_info.p = r.orig + r.dir * t_min;
    hit_info.n = hit_info.p - s.center;
    hit_info.n = normalize(hit_info.n);
    hit_info.hit = t_min > 0.0;

    return hit_info;
}


void main(void) 
{ 
    Sphere s0;
    s0.center = vec3(0, 0, -3);
    s0.radius = 1.f;

    Ray r;
    r.orig = vec3(0.0,0.0,0.0);
    r.dir = vec3(pos,-1);


    HitInfo hi = hit_sphere(r,s0);
    if(hi.hit)
        color = vec4(1.0,0.0,0.0,1.0);

} 