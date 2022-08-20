#include "our_gl.h"
#include "tgaimage.h"
#include <algorithm>

m4f ModelView;
m4f Viewport;
m4f Projection;

void GenModelView(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
	Vec3f x = (up ^ z).normalize();
	Vec3f y = (z ^ x).normalize();
	m4f min_v = m4f::identity(4);
	m4f tr = m4f::identity(4);

	for (int i = 0;i < 3; ++i)
	{
		// 旋转
		min_v[0][i] = x[i];
		min_v[1][i] = y[i];
		min_v[2][i] = z[i];
		// 平移
		tr[i][3] = -eye[i];
	}
	ModelView = min_v * tr;
}

void GenViewport(int x, int y, int w, int h)
{
	m4f m = m4f::identity(4);
	m[0][3] = x + w / 2.0f;
	m[1][3] = x + h / 2.0f;
	m[2][3] = 255 / 2.0f;

	m[0][0] = w / 2.0f;
	m[1][1] = h / 2.0f;
	m[2][2] = 255 / 2.0f;
    Viewport = m;
}

void GenProjection(float distance)
{
    Projection = m4f::identity(4);
    Projection[3][2] = -1.0 / distance;
}


Vec3f barycentric(Vec2i* pts, Vec2i P) {
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);

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
	
    if (std::abs(u[2])>1e-2) 
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1);
}

void DrawTriangle(Vec4f *pts, IShader& shader, TGAImage& image, TGAImage& z_buffer)
{
    Vec2f box_min(1000,  1000);
    Vec2f box_max(-1000, -1000);
    for (int i = 0;i < 3; ++i)
    {
        for (int j = 0;j < 2; ++j)
        {
            box_min[j] = std::min(box_max[j], pts[i][j] / pts[i][3]);
            box_max[j] = std::max(box_max[j], pts[i][j] / pts[i][3]);
        }
    }
    box_min.x = int(box_min.x);
    box_min.y = int(box_min.y);
    box_max.x = int(box_max.x + 1.0);
    box_max.y = int(box_max.y + 1.0);
    Vec2i p;
    TGAColor color;
    for (p.x = box_min.x;p.x <= box_max.y; ++p.x)
    {
        for (p.y = box_min.y;p.y <= box_max.y; ++p.y)
        {
            Vec3f c = barycentric(Vec3f(pts[0]), Vec3f(pts[1]), Vec3f(pts[2]), Vec3f(p[0], p[1], 0));
            float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
            float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
            int frag_depth = std::max(0, std::min(255, int(z/w + 0.5)));
            if (c.x < 0 || c.y < 0 || c.z < 0 || z_buffer.get(p.x, p.y)[0] > frag_depth) continue;
            bool discard = shader.fragment(c, color);
            if (!discard)
            {
                z_buffer.set(p.x, p.y, TGAColor(frag_depth));
                image.set(p.x, p.y, color);
            }
        }
    }
}