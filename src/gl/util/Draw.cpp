#include "Draw.h"
#include "gl/math/Math.h"

using namespace gl;

static const char* vSrc = "\
#version 150\n\
uniform mat4 modelViewProj;\n\
in  vec4 vs_position;\n\
in  vec4 vs_color;\n\
out vec4 fs_color;\n\
void main()\n\
{\n\
    gl_Position = modelViewProj * vs_position;\n\
    fs_color = vs_color;\n\
}";

static const char* fSrc = "\
#version 150\n\
in vec4 fs_color;\n\
out vec4 display_color;\n\
void main()\n\
{\n\
    display_color = fs_color;\n\
}";

Draw::Draw()
{
}

void Draw::begin(GLenum mode)
{
	if (lineShader.id() == 0 || lineBuffer.id() == 0) {
		lineShader = Program::createFromSrc(vSrc, fSrc);
		lineBuffer.generateVBO(GL_DYNAMIC_DRAW);
	}
	this->mode = mode;
	vertices.clear();
}

void Draw::end()
{
	if (vertices.empty()) {
		return;
	}

	lineBuffer.bind();
	lineBuffer.data(&vertices[0], vertices.size() * sizeof(GLfloat));
}

void Draw::draw()
{
	lineShader.enable();
	lineBuffer.bind();

	GLint loc = lineShader.getAttribute("vs_position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, false, 7 * sizeof(GLfloat), 0);

	loc = lineShader.getAttribute("vs_color");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, false, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	loc = lineShader.getUniform("modelViewProj");
	glUniformMatrix4fv(loc, 1, false, modelViewProjection);

	glDrawArrays(mode, 0, static_cast<GLsizei>(vertices.size() / 7));
}

void Draw::setModelViewProj(const Mat4& mvp)
{
	this->modelViewProjection = mvp;
}

void Draw::color(float r, float g, float b, float a)
{
	currentColor.x = r;
	currentColor.y = g;
	currentColor.z = b;
	currentColor.w = a;
}

void Draw::vertex(float x, float y, float z)
{
	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(z);
	vertices.push_back(currentColor.x);
	vertices.push_back(currentColor.y);
	vertices.push_back(currentColor.z);
	vertices.push_back(currentColor.w);
}

void Draw::line(float x1, float y1, float x2, float y2)
{
	vertex(x1, y1);
	vertex(x2, y2);
}

void Draw::circle(float x, float y, float radius, int numSegments)
{
	float angle = 0.0f;
	float step = two_pi / numSegments;
	for (int i = 0; i < numSegments; ++i) {
		vertex(x + std::cos(angle) * radius, y + std::sin(angle) * radius);
		vertex(x + std::cos(angle + step) * radius, y + std::sin(angle + step) * radius);
		angle += step;
	}
}

void Draw::geometry(const Geometry& g)
{
	for (int i = 0; i < g.indices.size(); i++) {
		const Vec3& p = g.vertices[g.indices[i]];
		vertex(p.x, p.y, p.z);
	}
}