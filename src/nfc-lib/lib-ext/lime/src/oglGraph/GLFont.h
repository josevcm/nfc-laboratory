#ifndef GL_FONT_H
#define GL_FONT_H

#include "glew/GL/glew.h"
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <string>
#include <vector>

struct GLGlyph
{
    GLGlyph() : id(-1), width(0), height(0), u(0), v(0), offset_x(0), offset_y(0), advance_x(0) {};
    void reset() { id = -1; width=0; height=0; u=0; v=0; offset_x=0, offset_y=0, advance_x=0; bitmap.clear();};
    std::vector<unsigned char> bitmap;
    unsigned short id;
    unsigned short width;
    unsigned short height;
    unsigned short u;
    unsigned short v;
    short offset_x;
    short offset_y;
    short advance_x;
};

class GLFont
{
public:
    GLFont();
    ~GLFont();

    bool load(const char* file);
    bool loadFromArray(const char* array, unsigned int size);
    void render_textWorldSpace(const char *text, float x, float y, float scale, unsigned int rgba = 0x00000000);
    void render_textScreenSpace(const char *text, float x, float y, float scale, unsigned int rgba = 0x00000000);

    unsigned int lineHeight() const;
    unsigned int char_width(const char c) const;
    unsigned int string_width(const char *str) const;
    void getTextSize(const char* str, int& width, int &height, float fontSize);

    unsigned int getTextureID() {return m_texID;};

    int m_texwidth;
    int m_texheight;
    GLuint m_texID;
    GLuint m_vboID;
    GLGlyph *m_glyphs;
    const unsigned short m_glyphCount;
    unsigned char m_fontSize;
};

#endif
