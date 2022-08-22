#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<Vec3i> > faces_;
	std::vector<Vec2f> texture_;
	std::vector<Vec3f> norms_;
	TGAImage diffusemap_;
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	int ntex();
	int ntexface();
	Vec3f vert(int i);
	Vec3f vert(int nthface, int nthvert);
	Vec2f tex(int i);
	std::vector<int> face(int idx);
	std::vector<int> texface(int idx);
	Vec3f normal(int nthface, int nthvert);
	Vec2f texture(int nthface, int nthvert);
	void LoadTexture(std::string file_name);
	TGAColor diffuse(Vec2f uv);
};

#endif //__MODEL_H__