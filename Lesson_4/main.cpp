#include "../tgaimage.h"
#include "../model.h"
#include "../geometry.h"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <direct.h>
#include <numeric>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width = 800;
const int height = 800;
const int depth = 255;
float z_buffer[height*width];
using namespace std;

typedef Matrix<float> m4f;

Vec3f m2v(m4f m)
{
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

m4f v2m(Vec3f v)
{
	m4f m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

m4f viewport(int x, int y, int w, int h)
{
	m4f m = m4f::identity(4);
	m[0][3] = x + w / 2.0f;
	m[1][3] = x + h / 2.0f;
	m[2][3] = depth / 2.0f;

	m[0][0] = w / 2.0f;
	m[1][1] = h / 2.0f;
	m[2][2] = depth / 2.0f;

	return m;

}

Vec3f barycentric(Vec2i* pts, Vec2i P) {
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = s[0] ^ s[1];
	
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, float *z_buffer, TGAImage& image, 
	Vec2f* tex_coords, TGAImage& texture, float intensity)
{
	Vec2f box_min(image.width()-1, image.height());
	Vec2f box_max(-1, -1);
	Vec2f clamp(image.width() - 1, image.height() - 1);
	for (int i = 0;i < 3; ++i)
	{
		for (int j = 0;j < 2; ++j)
		{
			box_min[j] = std::max(0.0f, 		(float)std::min(box_min[j], pts[i][j]));
			box_max[j] = std::min(clamp[j], std::max(box_max[j], pts[i][j]));
		}
	}
	// 这里是为了遍历像素的时候在中间
	box_min[0] = (int)box_min[0] + .5;
	box_min[1] = (int)box_min[1] + .5;
	box_max[0] = (int)box_max[0] + .5;
	box_max[1] = (int)box_max[1] + .5;

	Vec3f p;
	for (p.x = box_min.x;p.x <= box_max.x; ++p.x)
	{
		for (p.y = box_min.y;p.y <= box_max.y; ++p.y)
		{
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], p);
			if (bc_screen.x <= 0 || bc_screen.y <= 0 || bc_screen.z <= 0)
			{
				continue;
			}
			TGAColor tex;
			Vec2f pos;
			p.z = 0;
			for (int i = 0; i < 3; ++i)
			{
				p.z += bc_screen[i] * pts[i].z;
				tex = tex + texture.get(tex_coords[i].x, tex_coords[i].y) * bc_screen[i];
				pos = pos + tex_coords[i] * bc_screen[i];
			}
			tex = texture.get(pos.x, pos.y);
			if (z_buffer[int(p.x + p.y * width)] <= p.z)
			{
				z_buffer[int(p.x + p.y * width)] = p.z;
				image.set(p.x, p.y, tex * intensity);
			}

		}
	}
}

int main()
{
	int x0, y0, x1, y1;
	for (int i = 0;i < width*height; ++i) z_buffer[i] = -std::numeric_limits<float>::max();

	Model* model = new Model("D:\\Project\\TinyRendererLearn\\african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage texture;
	texture.read_tga_file("D:\\Project\\TinyRendererLearn\\african_head_diffuse.tga");
	Vec3f light_dir = Vec3f(0, 0, -1);

	float distance = 1;
	m4f proj = m4f::identity(4);
	proj[3][2] = -1.0 / distance;


	for (int i = 0; i < model->nfaces(); ++i)
	{
		std::vector<int> face = model->face(i);
		std::vector<int> tex_face = model->texface(i);
		assert(tex_face.size() == 3);
		for (auto v : tex_face)
			assert(v >= 0);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		Vec3f proj_coords[3];
		Vec2f tex_coords[3];
		for (int j = 0; j < 3; ++j)
		{
			Vec3f p = model->vert(face[j]);
			Vec2f tex = model->tex(tex_face[j]);
			tex_coords[j] = Vec2f(tex.x * texture.width(), tex.y * texture.height());
			world_coords[j] = p;
			proj_coords[j] = m2v((proj * v2m(p)));
			screen_coords[j] = Vec3f((proj_coords[j].x + 1.0) * width / 2.0 ,
									 (proj_coords[j].y + 1.0) * height / 2.0 , proj_coords[j].z );
		}
		Vec3f normal = (world_coords[2] - world_coords[0]) ^
			(world_coords[1] - world_coords[0]);
		normal.normalize();
		float intensity = light_dir * normal;
		if (intensity > 0)
			triangle(screen_coords, z_buffer, image, tex_coords, texture, 
				intensity);
	}
	//image.flip_vertically();
	image.write_tga_file("D:\\Project\\TinyRendererLearn\\output.tga");
	delete model;

	return 0;
}

// void triangle(Vec3f* pts, float* z_buffer, TGAImage& image, TGAColor color)
