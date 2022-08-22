/*
 * @Descripttion: LinEngine
 * @version: 1.0.0
 * @Author: jy1lnz
 * @Date: 2022-08-20 22:09:47
 * @LastEditors: jy1lnz
 * @LastEditTime: 2022-08-20 22:42:16
 */
#include "../our_gl.h"
#include "../model.h"
#include "../tgaimage.h"

const int width  = 800;
const int height = 800;

TGAColor red(255, 0, 0);

Model *model = nullptr;
Vec3f light_dir(1, 1, 1);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

m4f v2m(Vec4f v)
{
    m4f m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = v.w;
    return m;
}

Vec4f m2v(m4f m)
{
    return Vec4f(m[0][0], m[1][0], m[2][0], m[3][0]);
}

struct GouraudShader : public IShader {
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = Vec4f(model->vert(iface, nthvert), 1); // read the vertex from .obj file
        return m2v(Viewport*Projection*ModelView*v2m(gl_Vertex)); // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        if (intensity > .85) intensity = 1;
        else if (intensity > .60) intensity = .80;
        else if (intensity > .45) intensity = .60;
        else if (intensity > .30) intensity = .45;
        else if (intensity > .15) intensity = .30;
        else intensity = 0;
        color = TGAColor(255, 155, 0)*intensity; // well duh
        return false;                              // no, we do not discard this pixel
    }
};

struct TextureShader : public IShader
{
    Vec3f varying_intensity;
    Vec2f tex[3];
    Vec4f vertex(int iface, int nthvert) override
    {
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);
        tex[nthvert] = model->texture(iface, nthvert);
        Vec4f gl_Vertex = Vec4f(model->vert(iface, nthvert), 1);
        return m2v(Viewport * Projection * ModelView * v2m(gl_Vertex));
    }

    bool fragment(Vec3f bar, TGAColor& color) override
    {
        float intensity = varying_intensity * bar;
        Vec2f uv = tex[0] * bar.x + tex[1] * bar.y + tex[2] * bar.z;
        color = model->diffuse(uv) * intensity;
        return false;
    }
};

float buffer[800 * 800];
int main()
{
    model = new Model("D:\\Project\\TinyRendererLearn\\african_head.obj");
    model->LoadTexture("D:\\Project\\TinyRendererLearn\\african_head_diffuse.tga");
    for (int i = 0; i < 800 * 800; ++i) buffer[i] = -100000;

    GenViewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    GenModelView(eye, center, up);
    GenProjection((eye-center).norm());
    light_dir.normalize();

    TGAImage image(width, height, TGAImage::RGB);
    std::vector<std::vector<float>> z_buffer(width, std::vector<float>(height, -100000));

    TextureShader shader;
    for (int i = 0; i < model->nfaces(); ++i)
    {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; ++j)
        {
            screen_coords[j] = shader.vertex(i, j);
        }
        DrawTriangle(screen_coords, shader, image, z_buffer);
    }

    //image.flip_vertically();
    //z_buffer.flip_vertically();
    image.write_tga_file("D:\\Project\\TinyRendererLearn\\output.tga");

    delete model;
    return 0;
}