#include "../tgaimage.h"
#include <algorithm>
#include <iostream>
#include <math.h>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

void draw_line_1(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	for (float t = 0.0; t < 1.0; t += 0.01)
	{
		int x = x0 + (x1 - x0) * t;
		int y = y0 + (y1 - y0) * t;
		image.set(x, y, color);
	}
}

void draw_line_2(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	for (int x = x0; x <= x1; ++x)
	{
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1.0 - t) + y1 * t;
		image.set(x, y, color);
	}
}

void draw_line_3(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
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
	for (int x = x0; x <= x1; ++x)
	{
		float t = (x - x0) / (float)(x1 - x0);
		int y = y0 * (1.0 - t) + y1 * t;
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
	}
}

void draw_line_4(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = (x1 - x0);
	int dy = (y1 - y0);
	float derror = std::abs(dy / float(dx));
	std::cout << derror << std::endl;
	float error = 0;
	int y = y0;
	for (int x = x0; x <= x1; ++x)
	{
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		error += derror;
		/*
			这里的优化相当于对于一条线段，我们从（x0, y0), (x1, y1)的连线是连接像素中点
			因为最开始的位置在0.5，所以线段上点大于0.5之后，就代表要移动像素
		*/
		if (error > 0.5)
		{
			y += (y1 > y0 ? 1: -1);
			error -= 1.0;
		}
	}
}

void draw_line_5(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	// 浮点替换优化
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = (x1 - x0);
	int dy = (y1 - y0);
	float derror = std::abs(dy / float(dx));
	std::cout << derror << std::endl;
	float error = 0;
	int y = y0;
	for (int x = x0; x <= x1; ++x)
	{
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		error += derror;
		/*
			这里的优化相当于对于一条线段，我们从（x0, y0), (x1, y1)的连线是连接像素中点
			因为最开始的位置在0.5，所以线段上点大于0.5之后，就代表要移动像素
		*/
		if (error > 0.5)
		{
			y += (y1 > y0 ? 1 : -1);
			error -= 1.0;
		}
	}
}

int main()
{
	int x0, y0, x1, y1;
	TGAImage image(100, 100, TGAImage::RGB);

	//draw_line_3(1, 20, 80, 30, image, white);
	//draw_line_3(20, 1, 30, 80, image, red);
	draw_line_4(1, 1, 3, 8, image, red);

	//image.flip_vertically();
	image.write_tga_file("output.tga");

	return 0;
}