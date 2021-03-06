#pragma once

#include <imagine/util/math/GLKitVec3.hh>
#include <imagine/util/math/GLKitVec4.hh>
#include <imagine/util/rectangle2.h>
#include <GLKit/GLKMathTypes.h>
#include <GLKit/GLKMatrix4.h>
#include <compare>

class GLKitMat4
{
public:
	class RowRef
	{
	public:
		constexpr RowRef(float &v0, float &v1, float &v2, float &v3):
			v{&v0, &v1, &v2, &v3}
		{}

		float &operator [](unsigned int i)
		{
			return *v[i];
		}

	private:
		float *v[4];
	};

	class ConstRowRef
	{
	public:
		constexpr ConstRowRef(const float &v0, const float &v1, const float &v2, const float &v3):
			v{&v0, &v1, &v2, &v3}
		{}

		const float &operator [](unsigned int i) const
		{
			return *v[i];
		}

	private:
		const float *v[4];
	};

	constexpr GLKitMat4() {}
	constexpr GLKitMat4(GLKMatrix4 m):
		m
		{{
			m.m[0],  m.m[1],  m.m[2],  m.m[3],
			m.m[4],  m.m[5],  m.m[6],  m.m[7],
			m.m[8],  m.m[9],  m.m[10], m.m[11],
			m.m[12], m.m[13], m.m[14], m.m[15]
		}}
	{}
	static GLKitMat4 makeTranslate(GLKitVec3 translation);
	static GLKitMat4 makePerspectiveFovRH(float fovy, float aspect, float znear, float zfar);
	GLKitMat4 translate(GLKitVec3 translation) const;
	GLKitMat4 scale(GLKitVec3 factors) const;
	GLKitMat4 rotate(float angle, GLKitVec3 axis) const;
	GLKitMat4 invert() const;
	GLKitMat4 mult(GLKitMat4 mat) const;
	GLKitVec4 mult(GLKitVec4 vec) const;
	GLKitVec3 project(IG::Rect2<int> viewport, GLKitVec3 obj) const;
	GLKitVec3 unproject(IG::Rect2<int> viewport, GLKitVec3 win, GLKitMat4 inverse) const;
	bool operator ==(GLKitMat4 const &rhs) const;

	RowRef operator[](int i)
	{
		i *= 4; // convert to row start
		return {m.m[i], m.m[i+1], m.m[i+2], m.m[i+3]};
	}

	constexpr ConstRowRef operator[](int i) const
	{
		i *= 4; // convert to row start
		return {m.m[i], m.m[i+1], m.m[i+2], m.m[i+3]};
	}

	// Convenience functions
	GLKitMat4 scale(float s) const { return scale({s, s, 1.}); }
	GLKitMat4 scale(IG::Point2D<float> p) const { return scale({p.x, p.y, 1.}); }
	GLKitMat4 pitchRotate(float t) const { return rotate(t, {1., 0., 0.}); }
	GLKitMat4 rollRotate(float t) const { return rotate(t, {0., 0., 1.}); }
	GLKitMat4 yawRotate(float t) const { return rotate(t, {0., 1., 0.}); }

protected:
	GLKMatrix4 m
	{{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	}};
};
