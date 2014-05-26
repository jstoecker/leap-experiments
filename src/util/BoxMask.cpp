#include "BoxMask.h"

using namespace std;
using namespace gl;

BoxMask::BoxMask(const Box box) : box_(box)
{
}

void BoxMask::center(const Vec3& center)
{
	box_.center(center);
}

Vec3 BoxMask::center() const
{
	return box_.center();
}

Geometry BoxMask::geometry() const
{
	return box_.lines();
}

VolumeEdit BoxMask::add(const Box& bounds, int width, int height, int depth) const
{
    Vec3 min = bounds.normalize(bounds.clamp(box_.min()));
	Vec3 max = bounds.normalize(bounds.clamp(box_.max()));;
    
	GLuint x1 = static_cast<GLuint>(min.x * width);
	GLuint y1 = static_cast<GLuint>(min.y * height);
	GLuint z1 = static_cast<GLuint>(min.z * depth);
    
	GLuint x2 = static_cast<GLuint>(max.x * width);
	GLuint y2 = static_cast<GLuint>(max.y * height);
	GLuint z2 = static_cast<GLuint>(max.z * depth);
    
	GLuint w = x2 - x1;
	GLuint h = y2 - y1;
	GLuint d = z2 - z1;
	GLuint total = w * h * d;
    
    vector<GLubyte> weights;
    weights.resize(total, static_cast<GLubyte>(0));
    
    return {x1, y1, z1, w, h, d, weights};
}

VolumeEdit BoxMask::sub(const Box& bounds, int width, int height, int depth) const
{
    Vec3 mmin = box_.min();
    Vec3 mmax = box_.max();
    Vec3 bmin = bounds.min();
    Vec3 bmax = bounds.max();
    
    GLuint x1 = clamp((int)((mmin.x - bmin.x) / (bmax.x - bmin.x) * width  - 0.5f), 0, width - 1);
    GLuint y1 = clamp((int)((mmin.y - bmin.y) / (bmax.y - bmin.y) * height - 0.5f), 0, height - 1);
    GLuint z1 = clamp((int)((mmin.z - bmin.z) / (bmax.z - bmin.z) * depth  - 0.5f), 0, depth - 1);

    GLuint x2 = clamp((int)round((mmax.x - bmin.x) / (bmax.x - bmin.x) * width  - 0.5f), 0, width - 1);
    GLuint y2 = clamp((int)round((mmax.y - bmin.y) / (bmax.y - bmin.y) * height - 0.5f), 0, height - 1);
    GLuint z2 = clamp((int)round((mmax.z - bmin.z) / (bmax.z - bmin.z) * depth  - 0.5f), 0, depth - 1);
    
	GLuint w = x2 - x1 + 1;
	GLuint h = y2 - y1 + 1;
	GLuint d = z2 - z1 + 1;
	GLuint total = w * h * d;
    
    vector<GLubyte> weights;
    weights.resize(total, static_cast<GLubyte>(255));
    
    return {x1, y1, z1, w, h, d, weights};
}