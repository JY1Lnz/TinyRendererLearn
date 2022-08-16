#include "../tgaimage.h"
#include "../model.h"
#include "../geometry.h"

#include <algorithm>
#include <iostream>
#include <math.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int width = 200;
const int height = 200;

Vec3f barycentric(Vec2i* pts, Vec2i P) {
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	// 测试
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = (x1 - x0);
	int dy = (y1 - y0);
	int derror = std::abs(dy);
	int error = 0;
	int y = y0;
	for (int x = x0; x <= x1; ++x)
	{
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		error += derror;
		if (error * 2 > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			error -= dx;
		}
	}
}

void triangle_2(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int y = t0.y; y <= t1.y; y++) {
		int segment_height = t1.y - t0.y + 1;
		float alpha = (float)(y - t0.y) / total_height;
		float beta = (float)(y - t0.y) / segment_height; // be careful with divisions by zero 
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = t0 + (t1 - t0) * beta;
		image.set(A.x, y, red);
		image.set(B.x, y, green);
	}
}

void triangle_3(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	// 这里是画上半部分的三角形，根据y轴一行一行遍历
	for (int y = t0.y; y <= t1.y; ++y)
	{
		int segment_height = t1.y - t0.y + 1;
		float alpha = (float)(y - t0.y) / total_height;
		float beta = (float)(y - t0.y) / segment_height;
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = t0 + (t1 - t0) * beta;
		if (A.x > B.x)
			std::swap(A, B);
		for (int j = A.x; j <= B.x; ++j)
		{
			image.set(j, y, color);
		}
	}
	// 下半部分
	for (int y = t1.y; y <= t2.y; ++y)
	{
		int segment_height = t2.y - t1.y + 1;
		float alpha = (float)(y - t0.y) / total_height;
		float beta = (float)(y - t1.y) / segment_height;
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = t1 + (t2 - t1) * beta;
		if (A.x > B.x)
			std::swap(A, B);
		for (int j = A.x; j <= B.x; ++j)
		{
			image.set(j, y, color);
		}
	}
}

void triangle_4(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int i = 0; i <= total_height; ++i)
	{
		int y = t0.y + i;
		bool second_half = (y >= t1.y || t0.y == t1.y);
		int sub_heght = second_half ? (t1.y - t0.y) : 0;
		int segment_height = second_half ? t2.y - t1.y + 1: t1.y - t0.y + 1;
		float alpha = (float)i / total_height;
		float beta = (float)(i - sub_heght) / segment_height;
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x)
			std::swap(A, B);
		for (int j = A.x; j <= B.x; ++j)
		{
			image.set(j, y, color);
		}
	}
}

void triangle_5(Vec2i* pts, TGAImage& image, TGAColor color)
{
	Vec2i bboxmin(image.width() - 1, image.height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.width() - 1, image.height() - 1);
	for (int i = 0; i < 3; i++) {
		bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			image.set(P.x, P.y, color);
		}
	}
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) {
	if (t0.y == t1.y && t0.y == t2.y) return; // I dont care about degenerate triangles 
	// sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here 
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++) {
			image.set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y 
		}
	}
}

int main()
{
	int x0, y0, x1, y1;

	Model* model = new Model("african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
	Vec3f light_dir = Vec3f(0, 0, -1);
	for (int i = 0; i < model->nfaces(); ++i)
	{
		std::vector<int> face = model->face(i);
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j < 3; ++j)
		{
			Vec3f p = model->vert(face[j]);
			world_coords[j] = p;
			screen_coords[j] = Vec2i((p.x + 1.0) * width / 2.0, (p.y + 1.0) * height / 2.0);
		}
		Vec3f normal = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		normal.normalize();
		float intensity = light_dir * normal;
		if (intensity > 0)
			triangle_5(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255));
	}

	//image.flip_vertically();
	image.write_tga_file("output.tga");
	delete model;

	return 0;
}