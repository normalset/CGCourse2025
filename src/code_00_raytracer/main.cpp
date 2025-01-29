#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;

/*
Simple class to implement an image saved in PPM format
https://netpbm.sourceforge.net/doc/ppm.html
Few applications load it.
One is IrfanView: https://www.irfanview.com/
*/
struct image {
	image(int _w, int _h) :w(_w), h(_h) { data.resize(w * h * 3, 255); }
	int w, h;

	std::vector<int>  data;

	template <class S>
	void set_pixel(int i, int j, S  r, S  g, S  b) {
		j = h - 1 - j;
		data[(j * w + i) * 3] = (unsigned char)r;
		data[(j * w + i) * 3 + 1] = (unsigned char)g;
		data[(j * w + i) * 3 + 2] = (unsigned char)b;
	}

	void save(const char* filename) {
		ofstream f;
		f.open(filename);
		f << "P3\n";
		f << w << " " << h << std::endl;

		f << *(std::max_element(data.begin(), data.end())) << std::endl;

		for (unsigned int i = 0; i < data.size() / 3; ++i)
			f << data[i * 3] << " " << data[i * 3 + 1] << " " << data[i * 3 + 2] << std::endl;
		f.close();
	}
};

struct p3 {
	p3() {}
	p3(float x, float y, float z):x(x),y(y),z(z){}
	float x, y, z;

	float dot(p3 p) { return x * p.x + y * p.y + z * p.z; }
	p3 operator +(p3 p) { return p3(x + p.x , y + p.y , z + p.z); }
	p3 operator -(p3 p) { return p3(x - p.x,  y - p.y,  z - p.z); }
	p3 operator *(float s) { return p3(x*s, y *s, z*s); }
};

struct ray {
	ray(p3 orig, p3 dir):orig(orig),dir(dir){}
	p3 orig, dir;
};

struct sphere {
	
	sphere(p3 center, float radius, p3 color ):center(center), radius(radius),color(color) {}

	p3 center,color;
	float radius;
};

#define BIGT 1000
struct hit_info {
	hit_info():t(BIGT),hit(false){}
	hit_info(float t, p3 p, p3 n,bool hit):t(t),p(p), n( n),hit(hit){}
	float t;
	p3 p, n;
	bool hit;
};

hit_info hit_sphere(ray r, sphere s) {
	float A = r.dir.dot(r.dir);
	float B = r.dir.dot(r.orig - s.center) * 2;
	float C = (r.orig - s.center).dot(r.orig - s.center) - s.radius * s.radius;

	
	float delta = B * B - 4 * A * C;

	if (delta < 0)
		return hit_info();
	
	float t1 = (-B - sqrt(delta)) / (2 * A);
	float t2 = (-B + sqrt(delta)) / (2 * A);

	float t_min = min(t1, t2);
	if (t_min < 0)
		t_min = max(t1, t2);

	p3 p = r.orig + r.dir * t_min;
	p3 n = p - s.center;
	return hit_info(t_min,p, n,t_min > 0.f);

}

void main(int args, char** argv) {
	int sx = 800;
	int sy = 800;
	image a(sx, sy);

	p3 l(2, 2, 0);
	

	std::vector<sphere> scene;
	scene.push_back(sphere(p3(0, 0, -3), 1.f ,p3(255,0,0 )));
	scene.push_back(sphere(p3(0, 2, -5), 0.5f,p3(0, 0, 255)));
	scene.push_back(sphere(p3(0.5, 0.5, -1.5), 0.3f, p3(0, 255, 0)));

	for (unsigned int i = 0; i < a.w; ++i)
		for (unsigned int j = 0; j < a.h; ++j)
			a.set_pixel(i, j, 100, 100, 100);

	for(unsigned int i=0; i < a.w; ++i)
		for (unsigned int j = 0; j < a.h; ++j) {
			ray r(p3(0, 0, 0), p3(-1 + i / float(a.w) * 2, -1 + j / float(a.h) * 2, -1));

			hit_info hit_closest;

			for (unsigned int is = 0; is < scene.size(); ++is) {

				sphere& s = scene[is];
				hit_info hi = hit_sphere(r, s);

				if (hi.t < hit_closest.t) {


					hit_closest = hi;
					p3 L = l - hi.p;
					L = L * (1.f / sqrt(L.dot(L)));
					
					ray shadow_ray(hi.p+L*0.000001, L);
					unsigned int iss;
					for( iss =0 ; iss < scene.size(); ++iss)
						if(hit_sphere(shadow_ray,scene[iss]).hit) 
							break;
					
					p3 col(0, 0, 0);
					if (iss == scene.size())
					{
						p3 N = hi.n;
						N = N * (1.f / sqrt(N.dot(N)));

						float cosalpha = max(0.f, L.dot(N));

						col = s.color * cosalpha;
					}
					a.set_pixel(i, j, col.x, col.y, col.z);
				}
			}
		}

	a.save("rendering.ppm");
}