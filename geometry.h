#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <iostream>
#include <assert.h>

template <class t> struct Vec2 {
	union {
		struct { t u, v; };
		struct { t x, y; };
		t raw[2];
	};
	Vec2() : u(0), v(0) {}
	Vec2(t _u, t _v) : u(_u), v(_v) {}
	inline Vec2<t> operator +(const Vec2<t>& V) const { return Vec2<t>(u + V.u, v + V.v); }
	inline Vec2<t> operator -(const Vec2<t>& V) const { return Vec2<t>(u - V.u, v - V.v); }
	inline Vec2<t> operator *(float f)          const { return Vec2<t>(u * f, v * f); }
	t& operator[](int x) { return raw[x]; }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
	union {
		struct { t x, y, z; };
		struct { t ivert, iuv, inorm; };
		t raw[3];
	};
	Vec3() : x(0), y(0), z(0) {}
	Vec3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
	inline Vec3<t> operator ^(const Vec3<t>& v) const { return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
	inline Vec3<t> operator +(const Vec3<t>& v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
	inline Vec3<t> operator -(const Vec3<t>& v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
	inline Vec3<t> operator *(float f)          const { return Vec3<t>(x * f, y * f, z * f); }
	inline t       operator *(const Vec3<t>& v) const { return x * v.x + y * v.y + z * v.z; }
	t& operator[](int x) { return raw[x]; }
	float norm() const { return std::sqrt(x * x + y * y + z * z); }
	Vec3<t>& normalize(t l = 1) { *this = (*this) * (l / norm()); return *this; }
	template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

template <class t> struct Vec4 {
	union {
		struct { t x, y, z, w; };
		t raw[4];
	};
	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(t _x, t _y, t _z, t _w): x(_x), y(_y), z(_z), w(_w) { }
	Vec4(const Vec3<t>& v, int _w) : x(v.x), y(v.y), z(v.z), w(_w) { }
	Vec4<t> operator +(const Vec4<t>& v) const {
		return Vec4<t>(x + v.x, y + v.y, z + v.z, w + v.w);
	}
	Vec4<t> operator -(const Vec4<t>& v) const {
		return Vec4<t>(x - v.x, y - v.y, z - v.z, w + v.w);
	}	
	Vec4<t> operator -(float t) const {
		return Vec3<t>(x * f, y * f, z * f, w * f);	
	}
	Vec4<t> operator /(float t) const {
		return Vec4<t>(x / t, y / t, z / t, w / t);
	}
	t& operator[](int x) { return raw[x]; }
	const t& operator[](int x) const { return raw[x]; }
};

template<class T>
Vec3<T> toVec3(Vec4<T> v)
{
	return Vec3<T>(v.x / v.w, v.y / v.w, v.z / v.w);
}

template<class T>
Vec2<T> toVec2(Vec4<T> v)
{
	return Vec2<T>(v.x / v.w, v.y / v.w);
}

typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;
typedef Vec4<float> Vec4f;

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}


const int DEFAULT_ALLOC = 4;

template<class T>
class Matrix 
{
	std::vector<std::vector<T>> m;
	int rows, cols;
public:
	Matrix(int r = DEFAULT_ALLOC, int c = DEFAULT_ALLOC):
		m(std::vector<std::vector<T>>(r, std::vector<T>(c, 0))), rows(r), cols(c)
	{ }
	int nrows() { return rows; }
	int ncols() { return cols; }

	static Matrix<T> identity(int dimensions)
	{
		Matrix<T> E(dimensions, dimensions);
		for (int i = 0;i < dimensions; ++i)
			for (int j = 0;j < dimensions; ++j)
				E[i][j] = (i == j ? 1 : 0);
		return E;
	}
	std::vector<T>& operator[](const int i)
	{
		assert(i >= 0 && i < rows);
		return m[i];
	}
	const std::vector<T>& operator[](const int i) const
	{
		assert(i >= 0 && i < rows);
		return m[i];
	}
	Matrix<T> operator*(const Matrix<T>& rhs)
	{
		assert(cols == rhs.rows);
		Matrix<T> result(rows, rhs.cols);
		for (int i = 0;i < rows; ++i)
		{
			for (int j = 0;j < rhs.cols; ++j)
			{
				result[i][j] = 0;
				for (int k = 0;k < cols; ++k)
				{
					result[i][j] += m[i][k] * rhs[k][j];
				}
			}
		}
		return result;
	}
	Matrix<T> transpose()
	{
		Matrix<T> result(cols, rows);
		for(int i=0; i<rows; i++)
			for(int j=0; j<cols; j++)
				result[j][i] = m[i][j];
		return result;
	}
	Matrix<T> inverse()
	{
		assert(rows==cols);
		// augmenting the square matrix with the identity matrix of the same dimensions a => [ai]
		Matrix<T> result(rows, cols*2);
		for(int i=0; i<rows; i++)
			for(int j=0; j<cols; j++)
				result[i][j] = m[i][j];
		for(int i=0; i<rows; i++)
			result[i][i+cols] = 1;
		// first pass
		for (int i=0; i<rows-1; i++) {
			// normalize the first row
			for(int j=result.cols-1; j>=0; j--)
				result[i][j] /= result[i][i];
			for (int k=i+1; k<rows; k++) {
				float coeff = result[k][i];
				for (int j=0; j<result.cols; j++) {
					result[k][j] -= result[i][j]*coeff;
				}
			}
		}
		// normalize the last row
		for(int j=result.cols-1; j>=rows-1; j--)
			result[rows-1][j] /= result[rows-1][rows-1];
		// second pass
		for (int i=rows-1; i>0; i--) {
			for (int k=i-1; k>=0; k--) {
				float coeff = result[k][i];
				for (int j=0; j<result.cols; j++) {
					result[k][j] -= result[i][j]*coeff;
				}
			}
		}
		// cut the identity matrix back
		Matrix<T> truncate(rows, cols);
		for(int i=0; i<rows; i++)
			for(int j=0; j<cols; j++)
				truncate[i][j] = result[i][j+cols];
		return truncate;
	}

	friend std::ostream& operator<<(std::ostream& s, Matrix& m)
	{
		for (int i=0; i<m.nrows(); i++)  {
			for (int j=0; j<m.ncols(); j++) {
				s << m[i][j];
				if (j<m.ncols()-1) s << "\t";
			}
        s << "\n";
		}
		return s;
	}

};

typedef Matrix<float> m4f;

#endif