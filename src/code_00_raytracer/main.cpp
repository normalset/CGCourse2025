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

	void save(char* filename) {
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
	sphere(p3 center, float radius):center(center), radius(radius) {}

	p3 center;
	float radius;
};


struct hit_info {
	hit_info(p3 p, p3 n,bool hit):p(p), n( n),hit(hit){}
	p3 p, n;
	bool hit;
};

hit_info hit_sphere(ray r, sphere s) {
	float A = r.dir.dot(r.dir);
	float B = r.dir.dot(r.orig - s.center) * 2;
	float C = (r.orig - s.center).dot(r.orig - s.center) - s.radius * s.radius;

	
	float delta = B * B - 4 * A * C;

	if (delta < 0)
		return hit_info(p3(0,0,0),p3(0,0,0),false);

	float t1 = (-B - sqrt(delta)) / (2 * A);
	float t2 = (-B + sqrt(delta)) / (2 * A);

	float t_min = min(t1, t2);

	p3 p = r.orig + r.dir * t_min;
	p3 n = p - s.center;
	return hit_info(p, n,true);

}

void main(int args, char** argv) {
	int sx = 800;
	int sy = 800;
	image a(sx, sy);

	p3 l(2, 2, 0);
	sphere s(p3(0, 0, -3), 1.f);

	for (unsigned int i = 0; i < a.w; ++i)
		for (unsigned int j = 0; j < a.h; ++j)
			a.set_pixel(i, j, 0, 0, 0);

	for(unsigned int i=0; i < a.w; ++i)
		for (unsigned int j = 0; j < a.h; ++j) {
			ray r(p3(0, 0, 0), p3(-1 + i / float(a.w) * 2, -1 + j / float(a.h) * 2, -1));

			hit_info hi = hit_sphere(r, s);
			if (hi.hit) {
				p3 L = l - hi.p;
				L = L *(1.f/ sqrt(L.dot(L)));
				p3 N = hi.n;
				N = N * (1.f / sqrt(N.dot(N)));

				float cosalpha = max(0.f, L.dot(N));

				a.set_pixel(i, j, int(255*cosalpha), 0, 0);
			}
		}

	a.save("rendering.ppm");
}