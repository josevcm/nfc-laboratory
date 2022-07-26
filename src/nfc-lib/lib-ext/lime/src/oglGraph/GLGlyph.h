#ifndef GLGlyph_H
#define GLGlyph_H

#include <vector>

struct GLGlyph
{
    std::vector<unsigned char> bitmap;
    unsigned short id;
    unsigned short width;
    unsigned short height;
    unsigned short u;
    unsigned short v;
    unsigned short offset_x;
    unsigned short offset_y;
    unsigned short advance_x;
};

#endif // GLGlyph_H

