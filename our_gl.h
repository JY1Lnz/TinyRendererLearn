#ifndef OUT_GL_H
#define OUT_GL_H

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#include <vector>

extern m4f ModelView;
extern m4f Viewport;
extern m4f Projection;

void GenModelView(Vec3f eye, Vec3f center, Vec3f up);
void GenViewport(int x, int y, int w, int h);
void GenProjection(float distance);

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f p);
Vec3f barycentric(Vec2i* pts, Vec2i P);

// »­Èý½Ç

struct IShader
{
    virtual ~IShader() = default;
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void DrawTriangle(Vec4f* pts, IShader& shader, TGAImage& image, std::vector<std::vector<float>>& z_buffer);

#endif