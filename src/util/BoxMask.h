#ifndef __medleap_BoxMask__
#define __medleap_BoxMask__

#include "gl/geom/Box.h"

struct VolumeEdit
{
    GLuint x, y, z;
    GLuint w, h, d;
    std::vector<GLubyte> weights;
};

class BoxMask
{
public:
	BoxMask(const gl::Box box);
    
    VolumeEdit add(const gl::Box& bounds, int width, int height, int depth) const;
    VolumeEdit sub(const gl::Box& bounds, int width, int height, int depth) const;
    
	gl::Geometry geometry() const;
	void center(const gl::Vec3& center);
	gl::Vec3 center() const;

private:
	gl::Box box_;
};

#endif // __medleap_BoxMask__
