#include "GLFont.h"
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>
#include <vector>
using namespace std;

GLFont::GLFont() : m_texID(0), m_vboID(0), m_glyphCount(256)
{
    m_fontSize = 10;
    m_glyphs = new GLGlyph[m_glyphCount];
}

GLFont::~GLFont()
{
    delete []m_glyphs;
}

/**
    @brief loads opengl font file
    @param file font file to load

    Expected file structure:
    12 bytes - "OpenGL Font" literal file header
    1 byte - file version
    16 bytes - reserved
    1 byte - font size ( max line height in pixels )
    2 byte - glyph count
    glyphCount * 16 bytes - glyph definitions
        - 2 bytes (ushort)- glyph id
        - 2 bytes (ushort)- width
        - 2 bytes (ushort)- height
        - 2 bytes (ushort)- glyph x position in texture
        - 2 bytes (ushort)- glyph y position in texture
        - 2 bytes (short)- glyph x offset
        - 2 bytes (short)- glyph y offset (height above line)
        - 2 bytes (short)- glyph x advance (gap before glyph)
    2 bytes - texture width in pixels
    2 bytes - texture height in pixels
    1 byte - texture bits per pixel (currently hardcoded 8 bits per pixel)
    width*height*bpp - texture data
*/
bool GLFont::load(const char* file)
{
    glewInit();
    fstream in;
    in.open(file, ios::in | ios::binary);
    char header[13];
    header[12] = 0;
    in.read(header, 12);
    if( strcmp("OpenGL Font", header) != 0)
    {
        return false;
    }

    char fileVersion = 0;
    in.read(&fileVersion, 1);

    char reserved[16];
    in.read(reserved, 16);

    unsigned char fontSize = 0;
    in.read((char*)&fontSize, 1);
    m_fontSize = fontSize;

    unsigned short glyphCount = 0;
    in.read((char*)&glyphCount, 2);

    for(int i=0; i<m_glyphCount; ++i)
        m_glyphs[i].reset();
    GLGlyph tempGlyph;
    for(unsigned int i=0; i<glyphCount; ++i)
    {
        in.read((char*)&tempGlyph.id, sizeof(unsigned short));
        in.read((char*)&tempGlyph.width, sizeof(unsigned short));
        in.read((char*)&tempGlyph.height, sizeof(unsigned short));
        in.read((char*)&tempGlyph.u, sizeof(unsigned short));
        in.read((char*)&tempGlyph.v, sizeof(unsigned short));
        in.read((char*)&tempGlyph.offset_x, sizeof(short));
        in.read((char*)&tempGlyph.offset_y, sizeof(short));
        in.read((char*)&tempGlyph.advance_x, sizeof(short));
        tempGlyph.advance_x = tempGlyph.advance_x >> 6;
        m_glyphs[tempGlyph.id] = tempGlyph;
    }

    unsigned short texWidth = 0;
    unsigned short texHeight = 0;
    char bpp = 0;

    in.read((char*)&texWidth, sizeof(unsigned short));
    m_texwidth = texWidth;
    in.read((char*)&texHeight, sizeof(unsigned short));
    m_texheight = texHeight;
    in.read(&bpp, sizeof(char));

    unsigned char *bitmap = new unsigned char[texWidth*texHeight];
    in.read((char*)bitmap, texWidth*texHeight);
    in.close();

    glEnable( GL_TEXTURE_2D );
    glActiveTexture(GL_TEXTURE0);
    if( m_texID > 0)
        glDeleteTextures(1, &m_texID);
    glGenTextures(1, &m_texID);
    glBindTexture(GL_TEXTURE_2D, m_texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_ALPHA8,
        texWidth,
        texHeight,
        0,
        GL_ALPHA,
        GL_UNSIGNED_BYTE,
        bitmap
    );
    glBindTexture(GL_TEXTURE_2D, 0);

    if(!m_vboID && GLEW_VERSION_1_5)
    {
        glGenBuffers(1, &m_vboID);
        glBindBuffer( GL_ARRAY_BUFFER, m_vboID);
        glBufferData( GL_ARRAY_BUFFER, 16*sizeof(float), NULL, GL_DYNAMIC_DRAW );
        glBindBuffer( GL_ARRAY_BUFFER, 0);
    }

    return true;
}

bool GLFont::loadFromArray(const char* array, unsigned int size)
{
    glewInit();
    stringstream in;
    in.str( string(array, size));
    char header[13];
    header[12] = 0;
    in.read(header, 12);
    if( strcmp("OpenGL Font", header) != 0)
    {
        return false;
    }

    char fileVersion = 0;
    in.read(&fileVersion, 1);

    char reserved[16];
    in.read(reserved, 16);

    unsigned char fontSize = 0;
    in.read((char*)&fontSize, 1);
    m_fontSize = fontSize;

    unsigned short glyphCount = 0;
    in.read((char*)&glyphCount, 2);

    for(int i=0; i<m_glyphCount; ++i)
        m_glyphs[i].reset();
    GLGlyph tempGlyph;
    for(unsigned int i=0; i<glyphCount; ++i)
    {
        in.read((char*)&tempGlyph.id, sizeof(unsigned short));
        in.read((char*)&tempGlyph.width, sizeof(unsigned short));
        in.read((char*)&tempGlyph.height, sizeof(unsigned short));
        in.read((char*)&tempGlyph.u, sizeof(unsigned short));
        in.read((char*)&tempGlyph.v, sizeof(unsigned short));
        in.read((char*)&tempGlyph.offset_x, sizeof(short));
        in.read((char*)&tempGlyph.offset_y, sizeof(short));
        in.read((char*)&tempGlyph.advance_x, sizeof(short));
        tempGlyph.advance_x = tempGlyph.advance_x >> 6;
        m_glyphs[tempGlyph.id] = tempGlyph;
    }

    unsigned short texWidth = 0;
    unsigned short texHeight = 0;
    char bpp = 0;

    in.read((char*)&texWidth, sizeof(unsigned short));
    m_texwidth = texWidth;
    in.read((char*)&texHeight, sizeof(unsigned short));
    m_texheight = texHeight;
    in.read(&bpp, sizeof(char));

    unsigned char *bitmap = new unsigned char[texWidth*texHeight];
    in.read((char*)bitmap, texWidth*texHeight);

    glEnable( GL_TEXTURE_2D );
    glActiveTexture(GL_TEXTURE0);
    if( m_texID > 0)
        glDeleteTextures(1, &m_texID);
    glGenTextures(1, &m_texID);
    glBindTexture(GL_TEXTURE_2D, m_texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_ALPHA8,
        texWidth,
        texHeight,
        0,
        GL_ALPHA,
        GL_UNSIGNED_BYTE,
        bitmap
    );
    glBindTexture(GL_TEXTURE_2D, 0);

    if(!m_vboID && GLEW_VERSION_1_5)
    {
        glGenBuffers(1, &m_vboID);
        glBindBuffer( GL_ARRAY_BUFFER, m_vboID);
        glBufferData( GL_ARRAY_BUFFER, 16*sizeof(float), NULL, GL_DYNAMIC_DRAW );
        glBindBuffer( GL_ARRAY_BUFFER, 0);
    }

    return true;
}

void GLFont::render_textWorldSpace(const char *text, float x, float y, float fontSize, unsigned int rgba)
{
    glEnable( GL_TEXTURE_2D );
    glAlphaFunc(GL_GEQUAL, 0.05);
    glEnable( GL_ALPHA_TEST);
    glEnable( GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor3ub( (rgba >> 24)&0xff , (rgba >> 16)&0xFF, (rgba >> 8)&0xFF);

    float vbodata[16]; //interleaved x,y,u,v

    float scale = fontSize/m_fontSize;
    float startx = x;
    GLGlyph *g = NULL;
    glBindTexture( GL_TEXTURE_2D, m_texID );

    if(GLEW_VERSION_1_5)
        glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glVertexPointer( 2, GL_FLOAT, 4*sizeof(float), (char*)0 );
    glTexCoordPointer( 2, GL_FLOAT, 4*sizeof(float), (char*)0+2*sizeof(float) );
    const char *p;
    for(p = text; *p; ++p)
    {
        g = &m_glyphs[(unsigned char)*p];
        if((char)*p == '\n')
        {
            x = startx;
            y = y - lineHeight()*scale;
            continue;
        }
        float w = g->width * scale;
        float h = g->height * scale;
        vbodata[0] = x + g->offset_x * scale;
        vbodata[1] = y - (g->height-g->offset_y) * scale;
        vbodata[2] = g->u*1.0/m_texwidth;
        vbodata[3] = g->v*1.0/m_texheight;

        vbodata[4] = vbodata[0] + w;
        vbodata[5] = vbodata[1];
        vbodata[6] = g->u*1.0/m_texwidth + g->width*1.0/m_texwidth;
        vbodata[7] = vbodata[3];

        vbodata[8] = vbodata[0];
        vbodata[9] = vbodata[1] + h;
        vbodata[10] = vbodata[2];
        vbodata[11] = g->v*1.0/m_texheight + (g->height)*1.0/m_texheight;

        vbodata[12] = vbodata[4];
        vbodata[13] = vbodata[9];
        vbodata[14] = vbodata[6];
        vbodata[15] = vbodata[11];

//        if(GLEW_VERSION_1_5)
//        {
//            glBufferData( GL_ARRAY_BUFFER, 16*sizeof(float), NULL, GL_DYNAMIC_DRAW );
//            glBufferData( GL_ARRAY_BUFFER, 16*sizeof(float), vbodata, GL_DYNAMIC_DRAW );
//            glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
//            glFlush();
//        }
//        else
        {
            glBegin( GL_TRIANGLE_STRIP );
            glTexCoord2f( vbodata[2], vbodata[3]);
            glVertex3d(vbodata[0], vbodata[1], 10);
            glTexCoord2f( vbodata[6], vbodata[7]);
            glVertex3d(vbodata[4], vbodata[5], 10);
            glTexCoord2f( vbodata[10], vbodata[11]);
            glVertex3d(vbodata[8], vbodata[9], 10);
            glTexCoord2f( vbodata[14], vbodata[15]);
            glVertex3d(vbodata[12], vbodata[13], 10);
            glEnd();
            glFlush();
        }

        x += (g->advance_x ) * scale;
    }
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}

void GLFont::render_textScreenSpace(const char *text, float x, float y, float fontSize, unsigned int rgba)
{
    glEnable( GL_TEXTURE_2D );
    glAlphaFunc(GL_GEQUAL, 0.05);
    glEnable( GL_ALPHA_TEST);
    glEnable( GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor3ub( (rgba >> 24)&0xff , (rgba >> 16)&0xFF, (rgba >> 8)&0xFF);

    float vbodata[16]; //interleaved x,y,u,v

    float scale = fontSize/m_fontSize;
    float startx = x;
    GLGlyph *g = NULL;
    glBindTexture( GL_TEXTURE_2D, m_texID );

    if(GLEW_VERSION_1_5)
        glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glVertexPointer( 2, GL_FLOAT, 4*sizeof(float), (char*)0 );
    glTexCoordPointer( 2, GL_FLOAT, 4*sizeof(float), (char*)0+2*sizeof(float) );
    const char *p;
    for(p = text; *p; ++p)
    {
        g = &m_glyphs[(unsigned char)*p];
        if((char)*p == '\n')
        {
            x = startx;
            y = y + lineHeight()*scale;
            continue;
        }
        float w = g->width * scale;
        //float h = g->height * scale;
        vbodata[0] = x + g->offset_x * scale;
        vbodata[1] = y + (lineHeight()+(g->height-g->offset_y)) * scale;
        vbodata[2] = g->u*1.0/m_texwidth;
        vbodata[3] = g->v*1.0/m_texheight;

        vbodata[4] = vbodata[0] + w;
        vbodata[5] = vbodata[1];
        vbodata[6] = g->u*1.0/m_texwidth + g->width*1.0/m_texwidth;
        vbodata[7] = vbodata[3];

        vbodata[8] = vbodata[0];
        vbodata[9] = y + (lineHeight()-g->offset_y) * scale;;
        vbodata[10] = vbodata[2];
        vbodata[11] = g->v*1.0/m_texheight + (g->height)*1.0/m_texheight;

        vbodata[12] = vbodata[4];
        vbodata[13] = vbodata[9];
        vbodata[14] = vbodata[6];
        vbodata[15] = vbodata[11];

//        if(GLEW_VERSION_1_5)
//        {
//            glBufferData( GL_ARRAY_BUFFER, 16*sizeof(float), (char*)0, GL_DYNAMIC_DRAW );
//            glBufferData( GL_ARRAY_BUFFER, 16*sizeof(float), vbodata, GL_DYNAMIC_DRAW );
//            glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
//            glFlush();
//        }
//        else
        {
            glBegin( GL_TRIANGLE_STRIP );
            glTexCoord2f( vbodata[2], vbodata[3]);
            glVertex3d(vbodata[0], vbodata[1], 1);
            glTexCoord2f( vbodata[6], vbodata[7]);
            glVertex3d(vbodata[4], vbodata[5], 1);
            glTexCoord2f( vbodata[10], vbodata[11]);
            glVertex3d(vbodata[8], vbodata[9], 1);
            glTexCoord2f( vbodata[14], vbodata[15]);
            glVertex3d(vbodata[12], vbodata[13], 1);
            glEnd();
            //glFlush();
        }

        x += (g->advance_x ) * scale;
    }

    glFlush();
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
}

unsigned int GLFont::lineHeight() const
{
    return m_fontSize;
}

unsigned int GLFont::char_width(const char c) const
{
    return m_glyphs[(unsigned char)c].advance_x;
}

unsigned int GLFont::string_width(const char *str) const
{
    unsigned int width = 0;
    for(unsigned int i=0; i<strlen(str); ++i)
        width += m_glyphs[(unsigned char)str[i]].advance_x;
    return width;
}

void GLFont::getTextSize(const char* str, int& width, int &height, float fontSize)
{
    float scale = fontSize/m_fontSize;
    width = 0;

    int glyphH = 0;
    height =0;
    int len = strlen(str);
    for(int i=0; i<len; ++i)
    {
        if(i>len-1)
            width += m_glyphs[(unsigned char)str[i]].width;
        else
            width += m_glyphs[(unsigned char)str[i]].advance_x;
        glyphH = m_glyphs[(unsigned char)str[i]].height;
        if (glyphH> height)
            height = glyphH;
    }
    width = width*scale;
    height = height*scale;
}
