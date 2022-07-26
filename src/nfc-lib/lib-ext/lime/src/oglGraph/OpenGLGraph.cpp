/**
@file 	OpenGLGraph.cpp
@author Lime Microsystems
@brief	OpenGL based basic chart drawing
*/

#include "OpenGLGraph.h"
#include <stdio.h>
#include <stdarg.h>
#include <cmath>
#include <iostream>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include "GLFont.h"
#include "StaticFontHeader.h"
#include "dlgMarkers.h"
using namespace std;

#define OGL_REDRAW_ENABLED 1
#define OGL_INVERT_MOUSE_Y 1
const unsigned int OpenGLGraph::mMarkerColors[] = {0x000000FF, 0x0000FFFF, 0xFF0000FF, 0xFF7F00FF, 0x007FFFFF, 0xFF00FFFF, 0x007F00FF, 0x00007FFF, 0x7F0000FF, 0x00FF00FF};
const long markers_timer_id = wxNewId();

const int OpenGLGraph::GLCanvasAttributes[8] = {
    WX_GL_RGBA,
    WX_GL_DOUBLEBUFFER,
    WX_GL_DEPTH_SIZE, 16,
    WX_GL_STENCIL_SIZE, 0,
    0, 0 } ;

GLG_settings::GLG_settings() :
	title(""), titleXaxis(""), titleYaxis(""),
	xUnits(""), yUnits(""),
	drawGridX(true), drawGridY(true),
	drawTitle(true), drawTitleX(true), drawTitleY(true),
	windowWidth(100),	windowHeight(100),
	dataViewWidth(95), dataViewHeight(95),
	marginTop(0), marginBottom(10), marginLeft(10), marginRight(0),
	useVBO(true),
	backgroundColor(0xffffffff),titlesColor(0x000000ff),
	dataViewBackgroundColor(0xfdfdfdff), dataViewPerimeterColor(0x00ff00ff),
	gridColor(0xD0D0D0ff), graphType(GLG_LINE),
	visibleArea(-100, 100, -100, 100),
	gridXstart(0), gridYstart(0),
	gridXspacing(1), gridYspacing(1),
	gridXlines(4), gridYlines(4),
	gridXprec(0), gridYprec(0),
	gridValuesHeight(14),
	pointsSize(3), fontSize(1),
	staticGrid(false),
	lock_aspect(false), markersEnabled(false),
	gridXoffset(0)
	{}

OpenGLGraph::OpenGLGraph(wxWindow* parent,  wxWindowID id = -1,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style=0, const wxString& name,
                    const int* args)
    : wxGLCanvas(parent, id, args, pos, size, wxNO_FULL_REPAINT_ON_RESIZE),
oglOk(true),
initialDisplayArea(-100, 100, -100, 100),
m_MouseCoord(0, 0, 0, 0)
{
    m_font = NULL;
    m_glContext = new wxGLContext(this);
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    int w, h;
    this->GetSize(&w, &h);
    setupViewport(w, h);

	m_selectedMarker = -1;
	m_maxMarkers = 10;
	m_actionState = OGLG_IDLE;
	initialized = false;
	//Initialize(w, h);
	m_popmenu.Append( OGLG_SHOW_MARKERS_MENU, _("Markers menu.."), _("Shows markers settings"));
	m_popmenu.Append( OGLG_SEARCH_PEAK, _("Add marker on peak value"), _("Adds marker to peak value"));
	m_popmenu.Append( OGLG_ADD_MARK, _("Add marker"), _("Adds measuring marker"));
	m_popmenu.Append( OGLG_REMOVE_MARKER, _("Remove marker"), _("Removes measuring marker"));
    m_popmenu.Append( OGLG_FIT, _("Fit"),_("Set plot view to show all items"));
    m_popmenu.Append( OGLG_RESET, _("Reset"),_("Resets view to initial state"));
    m_popmenu.AppendCheckItem( OGLG_LOCKASPECT, _("Lock aspect"), _("Lock horizontal and vertical zoom aspect."));
    wxMenuItem *chkbox = m_popmenu.FindItem(OGLG_LOCKASPECT);
    chkbox->Check(settings.lock_aspect);
    m_popmenu.Append( OGLG_HELP_MOUSE, _("Show mouse commands..."), _("Show help about the mouse commands."));
    for(size_t i=0; i<m_maxMarkers; ++i)
    {
        markers.push_back(OGLMarker());
        markers[i].color = mMarkerColors[i];
        markers[i].id = i;
    }

    mMarkersDlg = new dlgMarkers(this);
    mMarkersDlg->parent_graph = this;
    for(size_t i=0; i<m_maxMarkers; ++i)
        mMarkersDlg->AddMarker(i);
    mMarkersDlg->AddDeltas();

    for(int i=0; i<3; ++i)
    {
        info_msg.push_back("");
        info_msg_toDisplay.push_back("");
    }

    m_timer = new wxTimer(this, markers_timer_id);
    m_timer->Start(500);
}


OpenGLGraph::~OpenGLGraph()
{
  if(m_timer->IsRunning()) {
      m_timer->Stop();
  }
  if(m_font) {
    delete m_font;
  }
}

bool OpenGLGraph::Initialize(int width, int height)
{
    char tempc[256];
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		sprintf(tempc, "GLEW ERROR %s", glewGetErrorString(err));
		cout << tempc << endl;
	}

	char userOGLversion[255];
	strcpy(userOGLversion, (const char*)glGetString(GL_VERSION));

    oglOk = glewIsSupported("GL_VERSION_2_0");

    if(!oglOk)
    {
        wxMessageBox(wxString::Format("Your OpenGL version is %s, required version is 2.0\nPlease update your graphics card drivers",
                                      wxString(userOGLversion)),_("WARNING"),wxOK,this);
        return false;
    }

    glEnable( GL_TEXTURE_2D );
	glAlphaFunc(GL_GEQUAL, 0.3);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(settings.backgroundColor.red, settings.backgroundColor.green,
				settings.backgroundColor.blue, settings.backgroundColor.alpha);
	glClear(GL_COLOR_BUFFER_BIT);

	Resize(width, height);
	settings.fontSize = 10;

	viewChanged = true;
	initialized = true;

	m_font = new GLFont();
	m_font->loadFromArray((const char *)standardStaticFont, sizeof(standardStaticFont));
	return true;
}


/**
	@brief Changes graph view size.
	@param w window width
	@param h window height
*/
void OpenGLGraph::Resize(int w, int h)
{
  if(w <= 0 || h <=0 )
    return;
  settings.windowWidth = w * GetContentScaleFactor();
  settings.windowHeight = h * GetContentScaleFactor();
  settings.dataViewHeight = settings.windowHeight-settings.marginTop-settings.marginBottom;
  if(settings.dataViewHeight <= 0)
    settings.dataViewHeight = 1;
  settings.dataViewWidth = settings.windowWidth-settings.marginLeft-settings.marginRight;
  if(settings.dataViewWidth <= 0)
    settings.dataViewWidth = 1;
  SettingsChanged();
}

/**
	@brief Adds data serie to graph
*/
void OpenGLGraph::AddSerie(cDataSerie* serie)
{
    if(serie != NULL)
        series.push_back(serie);
}

/**
	@brief Removes data serie from graph
	@param i index of data serie
*/
void OpenGLGraph::RemoveSeries(unsigned int i)
{
	if(i < series.size())
	    series.erase (series.begin()+i);
}

/**
	@brief Sets initialy displayed data area, when resetting zoom it will be reset to this area
	@param minx minimal x value
	@param maxx maximum x value
	@param miny minimum y value
	@param maxy maximum y value
*/
void OpenGLGraph::SetInitialDisplayArea(float minx, float maxx, float miny, float maxy)
{
	initialDisplayArea.set(minx, maxx, miny, maxy);
	SetDisplayArea( minx, maxx, miny, maxy);
}

/**
	@brief Sets visible area
	@param minx minimal x value
	@param maxx maximum x value
	@param miny minimum y value
	@param maxy maximum y value
*/
void OpenGLGraph::SetDisplayArea(float minx, float maxx, float miny, float maxy)
{
  if((minx == maxx) || (miny == maxy))
    return;
  settings.visibleArea.set(minx, maxx, miny, maxy);
  SettingsChanged();
#ifdef OGL_REDRAW_ENABLED
  Refresh();
#endif
}

/**
	@brief Changes Y axis center and span values
	@param centerY Y axis value that will be in the center of the window
	@param spanY Y axis span around center value
*/
void OpenGLGraph::ZoomY( float centerY, float spanY)
{
	SetDisplayArea( settings.visibleArea.x1, settings.visibleArea.x2, centerY - spanY/2, centerY + spanY/2);
#ifdef OGL_REDRAW_ENABLED
    Refresh();
#endif
}

/**
	@brief Changes X axis center and span values
	@param centerX X axis value that will be in the center of the window
	@param spanX X axis span around center value
*/
void OpenGLGraph::ZoomX( float centerX, float spanX)
{
	SetDisplayArea(centerX - spanX/2, centerX + spanX/2, settings.visibleArea.y1, settings.visibleArea.y2);
#ifdef OGL_REDRAW_ENABLED
    Refresh();
#endif
}

/**
	@brief Changes X and Y axis center and span values
	@param centerX X axis value that will be in the center of the window
	@param centerY Y axis value that will be in the center of the window
	@param spanX X axis span around center value
	@param spanY Y axis span around center value
*/
void OpenGLGraph::Zoom( float centerX, float centerY, float spanX, float spanY)
{
    if(spanX > 1E-03 && spanY > 1E-03)
    {
        if(settings.lock_aspect)
        {
            if(spanX < spanY)
                spanX = spanY;
            else
                spanY = spanX;
        }
        SetDisplayArea(centerX - spanX/2, centerX + spanX/2, centerY - spanY/2, centerY + spanY/2);
    }
#ifdef OGL_REDRAW_ENABLED
    Refresh();
#endif
}

/**
	@brief Zooms in area selected by window pixels
	@param x1 first corner x pixel coordinate
	@param x2 second corner x pixel coordinate
	@param y1 first corner y pixel coordinate
	@param y2 second corner y pixel coordinate

	Coordinates (0;0) are at the lower left corner
*/
void OpenGLGraph::ZoomRect( int x1, int x2, int y1, int y2)
{
    float minx, maxx, miny, maxy;
    float temp;
    dataViewPixelToValue(x1 - settings.marginLeft, y1 - settings.marginBottom, minx, miny);
    dataViewPixelToValue(x2 - settings.marginLeft, y2 - settings.marginBottom, maxx, maxy);
    if( minx > maxx)
    {
        temp = maxx;
        maxx = minx;
        minx = temp;
	}
    if( miny > maxy)
    {
        temp = maxy;
        maxy = miny;
        miny = temp;
    }
    if(maxx - minx > 1E-03 && maxy - miny > 1E-03)
    {
        SetDisplayArea( minx, maxx, miny, maxy );
    }

#ifdef OGL_REDRAW_ENABLED
    Refresh();
#endif
}

/**
	@brief Moves visible data area by set amount of pixels
	@param dx number of pixels to move on the x axis
	@param dy number of pixels to move on the y axis
*/
void OpenGLGraph::Pan( float dx, float dy)
{
	float deltaX = dx * (settings.visibleArea.x2 - settings.visibleArea.x1) / settings.dataViewWidth;
	float deltaY = dy * (settings.visibleArea.y2 - settings.visibleArea.y1) / settings.dataViewHeight;
    SetDisplayArea( settings.visibleArea.x1 + deltaX,
                    settings.visibleArea.x2 + deltaX,
                    settings.visibleArea.y1 + deltaY,
                    settings.visibleArea.y2 + deltaY);
#ifdef OGL_REDRAW_ENABLED
    Refresh();
#endif
}

/**
	@brief Sets drawing mode ( GLG_POINTS, GLG_LINE )
*/
void OpenGLGraph::SetDrawingMode( eDrawingMode mode )
{
	settings.graphType = mode;
}

/**
	@brief Draw static elements (background colors, titles, grid)
*/
void OpenGLGraph::DrawStaticElements()
{
	switchToWindowView();
	//draw data view background
	glBegin(GL_QUADS);
		glColor4f(settings.dataViewBackgroundColor.red, settings.dataViewBackgroundColor.green,	settings.dataViewBackgroundColor.blue, settings.dataViewBackgroundColor.alpha);
		glVertex3f( 0, 0, -5);
		glVertex3f( settings.windowWidth, 0, -5);
		glVertex3f( settings.windowWidth, settings.windowHeight, -5);
		glVertex3f( 0, settings.windowHeight, -5);
	glEnd();
	glFlush();

	//draw title
	int tw = 0, th = 0;
	float fheight = 32;

	glColor4f(settings.titlesColor.red, settings.titlesColor.green,	settings.titlesColor.blue, settings.titlesColor.alpha);
	m_font->getTextSize(settings.title.c_str(), tw, th, fheight);
	glRenderText(settings.windowWidth/2+settings.marginLeft - tw/2, settings.windowHeight-th-1, 0, fheight, 0, "%s", settings.title.c_str() );

	//draw axis titles and units
	glColor4f(settings.titlesColor.red, settings.titlesColor.green,	settings.titlesColor.blue, settings.titlesColor.alpha);
	m_font->getTextSize(settings.titleXaxis.c_str(), tw, th, fheight);
	glRenderText(settings.windowWidth-tw-settings.marginRight-1, settings.marginBottom+1, 0, fheight, 0, settings.titleXaxis.c_str());
	m_font->getTextSize(settings.titleYaxis.c_str(), tw, th, fheight);
	glRenderText(settings.marginLeft+1, settings.windowHeight-th-1, 0, fheight, 0, settings.titleYaxis.c_str());

	double pixelXvalue = 0;
	double pixelYvalue = 0;
	pixelXvalue =(settings.visibleArea.x2 - settings.visibleArea.x1) / settings.dataViewWidth;
	pixelYvalue = (settings.visibleArea.y2 - settings.visibleArea.y1)/  settings.dataViewHeight;

	if(viewChanged)
	{
		CalculateGrid();
		viewChanged = false;
	}

	char text[32];
	//draw axis lines
	glBegin(GL_LINES);
        glColor3f(0.0,0.0,0.0);
        glVertex2d(settings.marginLeft, settings.windowHeight);
        glVertex2d(settings.marginLeft, 0);
        glVertex2d(0, settings.marginBottom);
        glVertex2d(settings.windowWidth, settings.marginBottom);
	glEnd();

	//print X axis
	int posX = 0;
	int posY = 0;
	double value = 0;
	double gridXstart = settings.gridXstart;

	char format[10];
	if(settings.staticGrid)
          sprintf(format, "%%.3f %%s");
	else
          sprintf(format, "%%.%if %%s", settings.gridXprec);

	float numbersH = 16;
	// X axis grid lines
	posY = settings.marginBottom;
	posX = 0;
	float gridpx = gridXstart;

	for(int i=0; i< settings.windowWidth && posX < settings.windowWidth-settings.marginRight; ++i)
	{
		posX = settings.marginLeft + ((gridpx-settings.visibleArea.x1)/pixelXvalue);
		if(posX > settings.marginLeft && posX < settings.windowWidth-settings.marginRight)
		{
		    if(settings.staticGrid)
                value = gridpx/1000000.0+settings.gridXoffset;
            else
                value = gridpx+settings.gridXoffset;
			sprintf(text, format, value, settings.xUnits.c_str());
			m_font->getTextSize(text, tw, th, numbersH);
			glRenderText(posX - tw/2, posY-th-2, 0, numbersH, 0, "%s", text);
		}
		gridpx += settings.gridXspacing;
	}

	//Y axis grid
	//print Y axis
	posX = 0;
	posY = 0;
	value = 0;
	double gridYstart = settings.gridYstart;

	sprintf(format, "%%.%if %%s", settings.gridYprec);

	// Y axis grid lines
	posY = settings.marginBottom;
	posX = settings.marginLeft;
	float gridpy = gridYstart;

	for(int i=0; i< settings.windowHeight && posY < settings.windowHeight-settings.marginTop; ++i)
	{
		posY = settings.marginBottom + ((gridpy-settings.visibleArea.y1)/pixelYvalue);
		if(posY > settings.marginBottom && posY < settings.windowHeight-settings.marginTop)
		{
			value = gridpy;
			sprintf(text, format, value, settings.yUnits.c_str());
			m_font->getTextSize(text, tw, th, numbersH);
			glRenderText(posX - tw-2, posY - th/2, 0, numbersH, 0, "%s", text);
		}
		gridpy += settings.gridYspacing;
	}


	//change to data view and start drawing grid
	switchToDataView();

	glBegin(GL_LINES);
	glColor4f(settings.gridColor.red, settings.gridColor.green, settings.gridColor.blue, settings.gridColor.alpha);

	//draw x grid lines
	float linePos = settings.gridXstart;
	for(int i = 0; linePos <= settings.visibleArea.x2; ++i)
	{
		glVertex3d(linePos, settings.visibleArea.y1, -2);
		glVertex3d(linePos, settings.visibleArea.y2, -2);
		linePos += settings.gridXspacing;
	}

	//draw y grid lines
	linePos = settings.gridYstart;
	for(int i = 0; linePos <= settings.visibleArea.y2; ++i)
	{
		glVertex3d(settings.visibleArea.x1, linePos, -2);
		glVertex3d(settings.visibleArea.x2, linePos, -2);
		linePos += settings.gridYspacing;
	}

	glEnd();
	glFlush();

}

/**
	@brief Calculates grid position and spacing values
*/
void OpenGLGraph::CalculateGrid()
{
    if (settings.staticGrid)
    {
        settings.gridXlines = 10;
        settings.gridYlines = 10;
        //settings.gridXprec = 1;
        //settings.gridYprec = 1;
        settings.gridXstart = settings.visibleArea.x1;
        settings.gridXspacing = (settings.visibleArea.x2 - settings.visibleArea.x1) / settings.gridXlines;
        settings.gridYstart = settings.visibleArea.y1;
        settings.gridYspacing = (settings.visibleArea.y2 - settings.visibleArea.y1) / settings.gridYlines;

        return;
    }
    const int mult[11] = { 1, 1, 2, 2, 4, 5, 5, 5, 5, 5, 5 };

    float interval = settings.visibleArea.x2 - settings.visibleArea.x1; // span of x axis
    if (settings.gridXlines > 0)
    {
        //estimate text width
        const float cWidth = 15; //~double actual width
        float absx1 = std::fabs(settings.visibleArea.x1);
        float absx2 = std::fabs(settings.visibleArea.x2);
        int valLen = 3 + std::fabs(std::log10(absx1 > absx2 ? absx2 : absx2)); //some extra spacing for separation between values, and for decimal point
        int tmpLines = 1+settings.dataViewWidth / (valLen * cWidth);
        float divisor = 1; // current grid spacing
        int xlines = settings.gridXlines > tmpLines ? tmpLines : settings.gridXlines;
        settings.gridXprec = 0;
        //determine if grid spacing needs to be decreased
        while (interval / divisor < xlines)
        {
            divisor /= 10;
            settings.gridXprec++;
            tmpLines = settings.dataViewWidth / (++valLen * cWidth);
            xlines = xlines > tmpLines ? tmpLines : xlines;
        }
        while (interval / divisor > 10 * xlines)
            divisor *= 10;
        settings.gridXspacing = divisor * mult[int(0.5f + interval / xlines / divisor)];
        settings.gridXstart = (int)(settings.visibleArea.x1 / settings.gridXspacing) * settings.gridXspacing;
    }
    else
        settings.gridXspacing = interval;

    interval = settings.visibleArea.y2 - settings.visibleArea.y1; // span of y axis
    if (settings.gridYlines > 0)
    {
        int tmpLines = 1+settings.dataViewHeight / (2 * settings.gridValuesHeight); //2x to be safe in double scaling steps
        float divisor = 1; // current grid spacing
        int ylines = settings.gridYlines > tmpLines ? tmpLines : settings.gridYlines;
        settings.gridYprec = 0;
        while (interval / divisor < ylines)
        {
            divisor /= 10;
            settings.gridYprec++;
        }
        while (interval / divisor > 10 * ylines)
            divisor *= 10;
        settings.gridYspacing = divisor * mult[int(0.5f + interval / ylines / divisor)];
        settings.gridYstart = (int)(settings.visibleArea.y1 / settings.gridYspacing) * settings.gridYspacing;
    }
    else
        settings.gridYspacing = interval;
    SettingsChanged();
}

/**
	@brief Draw graph data series
*/
void OpenGLGraph::Draw()
{
	if(!IsShownOnScreen())
    {
        return;
    }
    SetCurrent(*m_glContext);
    int w, h;
    if(oglOk == false)
    {
        GetSize(&w, &h);
        setupViewport(w, h);
        glClearColor(settings.backgroundColor.red, settings.backgroundColor.green,
				settings.backgroundColor.blue, settings.backgroundColor.alpha);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SwapBuffers();
        return;
    }
    if(!initialized)
    {
        int w, h;
        this->GetSize(&w, &h);
        if(Initialize(w, h) == false)
            return;
    }
    GetSize(&w, &h);
    setupViewport(w, h);
    glLoadIdentity();

	switchToWindowView();
	glLoadIdentity();
	glClearColor(settings.backgroundColor.red, settings.backgroundColor.green,
				settings.backgroundColor.blue, settings.backgroundColor.alpha);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw static view elements
	DrawStaticElements();
	switchToWindowView();
	//while zomming in draw selection rectangle
	if(m_actionState == OGLG_ZOOMIN)
	{
		glColor3f(1.0, 0, 0);
		glBegin(GL_LINE_STRIP);
			glVertex2d(m_MouseCoord.x1, m_MouseCoord.y1);
			glVertex2d(m_MouseCoord.x2, m_MouseCoord.y1);
			glVertex2d(m_MouseCoord.x2, m_MouseCoord.y2);
			glVertex2d(m_MouseCoord.x1, m_MouseCoord.y2);
			glVertex2d(m_MouseCoord.x1, m_MouseCoord.y1);
		glEnd();
		glFlush();
	}
	//draw series data

	switchToDataView();
	if(settings.useVBO && GLEW_VERSION_1_5)
	{
		for(unsigned int i=0; i<series.size(); i++)
		{
			glColor3f(series[i]->color.red, series[i]->color.green, series[i]->color.blue);
			if(series[i]->size > 0 && series[i]->visible)
			{
				if( series[i]->vboIndex == 0) //check if data series buffer is initialized
				{
					glGenBuffersARB(1, &series[i]->vboIndex);
				}
				//bind buffer for filling
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, series[i]->vboIndex);
				if(series[i]->modified) //check if buffer needs to be modified
				{
					glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float)*series[i]->size*2, NULL, GL_DYNAMIC_DRAW_ARB);
					glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float)*series[i]->size*2, series[i]->values, GL_DYNAMIC_DRAW_ARB);
					series[i]->modified = false;
				}
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_COLOR);
				glVertexPointer(2, GL_FLOAT, 0, 0);
				if(settings.graphType == GLG_POINTS)
				{
					glPointSize(settings.pointsSize);
					glDrawArrays(GL_POINTS, 0, series[i]->size);
				}
				else
				{
					glPointSize(1);
					glDrawArrays(GL_LINE_STRIP, 0, series[i]->size);
				}
				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_COLOR);
				glBindBufferARB(GL_ARRAY_BUFFER, 0);
			}
		}
	}
	else  //backup case if VBO is not supported
	{
		for(unsigned int i=0; i<series.size(); i++)
		{
			glColor3f(series[i]->color.red, series[i]->color.green, series[i]->color.blue);
			if(series[i]->size > 0 && series[i]->visible)
			{
				if(settings.graphType == GLG_POINTS)
				{
					glPointSize(settings.pointsSize);
					glBegin(GL_POINTS);
				}
				else
				{
					glPointSize(1);
					glBegin(GL_LINE_STRIP);
				}
				for(unsigned int j=0; j<series[i]->size; j++)
				{
					glVertex3f( series[i]->values[2*j], series[i]->values[2*j+1], 1.0);
				}
				glEnd();
			}
		}
		glFlush();
	}
	//draw measuring markers
	DrawMarkers();
	switchToWindowView();
	int fontSz = 16;
	unsigned int clrs[] {0xFF000000, 0x0000FF00, 0x00FF0000};
	for(size_t i=0; i<info_msg_toDisplay.size(); ++i)
    {
        glRenderText(settings.marginLeft, settings.marginBottom+i*20+fontSz, 0, fontSz, clrs[i], "%s", info_msg_toDisplay[i].c_str());
    }
    SwapBuffers();
}

bool OpenGLGraph::SaveConfig(char *file)
{
    return false;
}
bool OpenGLGraph::LoadConfig(char *file)
{
    return false;
}

/**
	@brief Signals that settings have changed and needs to be reevaluated
*/
void OpenGLGraph::SettingsChanged()
{
	viewChanged = true;
}

/**
	@brief Changes Opengl viewport area to whole window
*/
void OpenGLGraph::switchToWindowView()
{
	glViewport(0, 0, settings.windowWidth, settings.windowHeight);
	glMatrixMode(GL_PROJECTION);    // add perspective to scene
	glLoadIdentity();               // restore matrix to original state
	glOrtho (0, settings.windowWidth, 0, settings.windowHeight, -100, 100);
	glMatrixMode(GL_MODELVIEW);
}

/**
	@brief changes Opengl viewport area to data view region and changes scale
*/
void OpenGLGraph::switchToDataView()
{

	glViewport(settings.marginLeft, settings.marginBottom,
				settings.windowWidth-settings.marginRight-settings.marginLeft,
				settings.windowHeight-settings.marginTop-settings.marginBottom);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();               // restore matrix to original state
	glOrtho (settings.visibleArea.x1, settings.visibleArea.x2, settings.visibleArea.y1, settings.visibleArea.y2, -100, 100);
	glMatrixMode(GL_MODELVIEW);
}

/**
	@brief Prints text in defined position
	@param posx x position of text lower left corner
	@param posy y position of text lower left corner
	@param angle angle in degrees counterclockwise to rotate text
	@param scale font scaling
	@param fmt text format and parameters
*/
GLvoid OpenGLGraph::glRenderText(float posx, float posy, float angle, float scale, unsigned int rgba, const char *fmt, ...)
{
	char text[256];	                // Holds our string
	va_list ap;	        	// Pointer to list of arguments

	if (fmt == NULL)		// If there's no text
		return;			// Do nothing

	va_start(ap, fmt);		// Parses the string for variables
		vsprintf(text, fmt, ap);	// And converts symbols to actual numbers
	va_end(ap);			// Results are stored in text

	glPushMatrix();
	glTranslatef(posx, posy, 0);
	glRotatef(angle, 0, 0, 1);

	//if font has been loaded
	glEnable(GL_TEXTURE_2D);
	if(m_font != NULL)
            m_font->render_textWorldSpace(text, 0, 0, scale, rgba);

	glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
	@brief Resets displayed area to initial area.
*/
void OpenGLGraph::ResetView()
{
	SetDisplayArea(initialDisplayArea.x1, initialDisplayArea.x2, initialDisplayArea.y1, initialDisplayArea.y2);
#ifdef OGL_REDRAW_ENABLED
    Refresh();
#endif
}

void OpenGLGraph::Fit()
{
    float minx=-1, maxx=1, miny=-1, maxy=1;
    float valx, valy;
    for(unsigned i=0; i<series.size() && series[i]->visible; ++i) //set boundary limits to first value
    {
        if(series[i]->size > 0)
        {
            minx = maxx = series[i]->values[0];
            miny = maxy = series[i]->values[1];
            break;
        }
    }
    for(unsigned i=0; i<series.size(); ++i)
    {
        for(unsigned j=0; j<series[i]->size && series[i]->visible; ++j)
        {
            valx = series[i]->values[2*j];
            valy = series[i]->values[2*j+1];
            if(valx < minx)
                minx = valx;
            else if(valx > maxx)
                maxx = valx;
            if(valy < miny)
                miny = valy;
            else if(valy > maxy)
                maxy = valy;
        }
    }
    SetDisplayArea(minx, maxx, miny, maxy);
}

/**
	@brief Handles mouse button pressing events
	@param mouseButton mouse button code
	@param X x coordinate of mouse pointer inside graph window
	@param Y y coordinate of mouse pointer inside graph window

	Given mouse coordinates (0,0) are at top left corner of window
*/
void OpenGLGraph::OnMouseDown(int mouseButton, int X, int Y)
{
#ifdef OGL_INVERT_MOUSE_Y
    Y = settings.windowHeight - Y;
#endif // OGL_INVERT_MOUSE_Y
    m_MouseCoord.x1 = X;
    m_MouseCoord.y1 = Y;

    switch(mouseButton)
    {
	case OGLG_LEFT:
		//move markers
		if( (m_selectedMarker = clickedOnMarker(X, Y)) >= 0)
		{
			m_actionState = OGLG_MOVE_MARKER;
		}
		else //start zooming in
		{
            m_actionState = OGLG_ZOOMIN;
            m_MouseCoord.x2 = X;
			m_MouseCoord.y2 = Y;
        }
		break;
	case OGLG_MIDDLE: // change axis span
        m_actionState = OGLG_SCALE;
        m_lastSpanX = settings.visibleArea.x2 - settings.visibleArea.x1;
		m_lastSpanY = settings.visibleArea.y2 - settings.visibleArea.y1;
		m_MouseCoord.x2 = X;
		m_MouseCoord.y2 = Y;
        break;

	case OGLG_RIGHT: // move display area
		m_actionState = OGLG_PAN;
		m_MouseCoord.x2 = X;
        m_MouseCoord.y2 = Y;
        m_MouseCoord.x1 = X;
        m_MouseCoord.y1 = Y;
        break;
    }
}

/**
	@brief Handles mouse button releasing events
	@param mouseButton mouse button code
	@param X x coordinate of mouse pointer inside graph window
	@param Y y coordinate of mouse pointer inside graph window

	Given mouse coordinates (0,0) are at top left corner of window
*/
void OpenGLGraph::OnMouseUp(int mouseButton, int X, int Y)
{
	switch(mouseButton)
	{
	case OGLG_LEFT:
		if( m_actionState == OGLG_ZOOMIN)
		{
		    m_actionState = OGLG_IDLE;
			if( isInsideDataView(X, Y)) 		//finish zooming in
			{
				ZoomRect(m_MouseCoord.x1, m_MouseCoord.x2, m_MouseCoord.y1, m_MouseCoord.y2);
				#ifdef OGL_REDRAW_ENABLED
                Refresh();
				#endif
			}
            else // if zoomed outside data view border, then reset to initial view
            {
                ResetView();
                #ifdef OGL_REDRAW_ENABLED
                Refresh();
				#endif
            }
        }
        break;
    case OGLG_MIDDLE:

        break;

    case OGLG_RIGHT:
        if((m_MouseCoord.x2 == m_MouseCoord.x1) &&
        (m_MouseCoord.y2 == m_MouseCoord.y1))
            ShowMenu(X, Y);
        break;
    }
	m_actionState = OGLG_IDLE;
}

/**
	@brief Handles mouse moving events
	@param X x coordinate of mouse pointer inside graph window
	@param Y y coordinate of mouse pointer inside graph window

	Given mouse coordinates (0,0) are at top left corner of window
*/
void OpenGLGraph::OnMouseMove(int X, int Y)
{
    float spanx, spany, sx, sy;
#ifdef OGL_INVERT_MOUSE_Y
	Y = settings.windowHeight - Y;
#endif // OGL_INVERT_MOUSE_Y
	switch(m_actionState)
	{
	case OGLG_IDLE:

		break;
	case OGLG_MOVE_MARKER:
		MoveMarker(m_selectedMarker, X);
		break;
	case OGLG_PAN:
		//if mouse leaves data view, stop panning
		if( !isInsideDataView(X, Y) )
		{
			m_actionState = OGLG_IDLE;
			break;
		}
		m_MouseCoord.x2 = m_MouseCoord.x1 - X;
		m_MouseCoord.y2 = m_MouseCoord.y1 - Y;
		Pan(m_MouseCoord.x2, m_MouseCoord.y2);
		m_MouseCoord.x1 = X;
		m_MouseCoord.y1 = Y;
		break;
	case OGLG_ZOOMIN:
		//update coordinates for zoom in rectangle drawing
		m_MouseCoord.x2 = X;
		m_MouseCoord.y2 = Y;
#ifdef OGL_REDRAW_ENABLED
        Refresh();
#endif
		break;
	case OGLG_SCALE:
		//change axis span while mouse is inside data view
		if(!isInsideDataView(X,Y))
		{
			m_actionState = OGLG_IDLE;
			break;
		}
		m_MouseCoord.x2 = X;
		m_MouseCoord.y2 = Y;
        spanx = settings.visibleArea.x2 - settings.visibleArea.x1;
		spany = settings.visibleArea.y2 - settings.visibleArea.y1;
		sx = 1+((float)(m_MouseCoord.x1 - m_MouseCoord.x2))/settings.dataViewWidth;
		sy = 1+((float)(m_MouseCoord.y1 - m_MouseCoord.y2))/settings.dataViewHeight;
        Zoom( settings.visibleArea.x1 + spanx/2, settings.visibleArea.y1 + spany/2, m_lastSpanX*sx, m_lastSpanY*sy);
        break;
    default:
        m_MouseCoord.x2 = X;
		m_MouseCoord.y2 = Y;
		break;
    }
}

/**
	@brief Calculates value pointed by pixel in data view
	@param x x coordinate of mouse pointer
	@param y y coordinate of mouse pointer
	@param &valX x axis data value pointed by mouse
	@param &valY y axis data value pointed by mouse

	x and y coordinate (0, 0) are at the data view lower left corner
*/
void OpenGLGraph::dataViewPixelToValue(int x, int y, float &valX, float &valY)
{
	float deltaX = (settings.visibleArea.x2 - settings.visibleArea.x1) / settings.dataViewWidth;
	float deltaY = (settings.visibleArea.y2 - settings.visibleArea.y1) / settings.dataViewHeight;

	valX = settings.visibleArea.x1 + deltaX * x;
	valY = settings.visibleArea.y1 + deltaY * y;
}

/**
	@brief Checks if pixel is inside data view
	@param X mouse x coordinate
	@param Y mouse y coordinate
	@returns
		- false : pixel is outside data view
		- true : pixel is inside data view
*/
bool OpenGLGraph::isInsideDataView(int X, int Y)
{
	if( X > settings.marginLeft && X < (settings.windowWidth-settings.marginRight) )
	{
		if( Y > settings.marginTop && Y < (settings.windowHeight-settings.marginBottom) )
			return true;
	}
	return false;
}

/**
	@brief Returns text string width in pixels
	@param *std pointer to char array
	@param fontSize font heigth in pixels
*/
int OpenGLGraph::TextWidthInPixels(const char *str)
{
	return m_font->string_width(str);
}

/**
	@brief Returns number text width in pixels
	@param num number
	@param fontSize text height in pixels
	@param prec number of decimal digits
*/
int OpenGLGraph::NumberWidthInPixels(float num, unsigned int prec)
{
	char format[20];
	sprintf(format, "%%.%if", prec);
	char ctemp[20];
	sprintf(ctemp, format, num);
	return TextWidthInPixels(ctemp);
}

/**
    @brief Returns line height in pixels
*/
int OpenGLGraph::LineHeight()
{
    return m_font->lineHeight();
}

/**
	@brief Add marker at selected pixel
	@param posX mouse x coordinate in graph window
	@return marker id
*/
int OpenGLGraph::AddMarker(int posX)
{
	bool found = false;
	//check if allowed to add more markers
    //check if series have any data to mark
    if(series[0]->size > 0)
    {
        OGLMarker *mark = NULL;
        for(size_t i=0; i<markers.size(); ++i)
            if(markers[i].used == false)
            {
                mark = &markers[i];
                break;
            }
        if(mark == NULL)
            return -1;
        //calculate marker position in data view
        float pixelXvalue =(settings.visibleArea.x2 - settings.visibleArea.x1) / settings.dataViewWidth;
        mark->posX = settings.visibleArea.x1+(posX-settings.marginLeft)*pixelXvalue;

        //find closest data point index
        for(unsigned i=0; i<series[0]->size; ++i)
        {
            if ( series[0]->values[2*i] > mark->posX )
            {
                float toLeft = mark->posX-series[0]->values[2*(i-1)];
                float toRight = series[0]->values[2*i]-mark->posX;
                if( toRight < toLeft)
                {
                    mark->dataValueIndex = 2*i;
                }
                else
                {
                    mark->dataValueIndex = 2*(i-1);
                }
                found = true;
                break;
            }
        }
        if(!found) //if no closest point found, add marker to middle point
        {
            mark->dataValueIndex = series[0]->size;
        }
        mark->used = true;
        mark->show = true;
        mark->posX = series[0]->values[mark->dataValueIndex];
        mark->posY = series[0]->values[mark->dataValueIndex+1];
        mark->color = mMarkerColors[markers.size()];
        return mark->id;
    }
	return -1;
}

/**
	@brief Add marker at selected data x value
	@return marker id
*/
int OpenGLGraph::AddMarkerAtValue(float xValue)
{
	bool found = false;
    //check if series have any data to mark
    if(series[0]->size > 0)
    {
        OGLMarker *mark = NULL;
        for(size_t i=0; i<markers.size(); ++i)
            if(markers[i].used == false)
            {
                mark = &markers[i];
                break;
            }
        if(mark == NULL)
            return -1;
        mark->posX = xValue;

        //find closest data point index
        for(unsigned i=0; i<series[0]->size; ++i)
        {
            if ( series[0]->values[2*i] >= mark->posX )
            {
                float toLeft = mark->posX-series[0]->values[2*(i-1)];
                float toRight = series[0]->values[2*i]-mark->posX;
                if( toRight < toLeft)
                {
                    mark->dataValueIndex = 2*i;
                }
                else
                {
                    mark->dataValueIndex = 2*(i-1);
                }
                found = true;
                break;
            }
        }
        if(!found) //if no closest point found, add marker to middle point
        {
            mark->dataValueIndex = series[0]->size;
        }
        mark->used = true;
        mark->posX = series[0]->values[mark->dataValueIndex];
        mark->posY = series[0]->values[mark->dataValueIndex+1];
        mark->color = mMarkerColors[markers.size()];
        mMarkersDlg->refreshMarkFreq = true;
        return mark->id;
    }
	return -1;
}

/**
	@brief Removes last added marker
*/
void OpenGLGraph::RemoveMarker()
{
    for(int i=markers.size()-1; i>=0; --i)
        if(markers[i].used == true)
        {
            markers[i].used = false;
            break;
        }
    Refresh();
}

void OpenGLGraph::RemoveMarker(int id)
{
    for(size_t i=0; i<markers.size(); ++i)
    {
        if(markers[i].id == id)
        {
            markers[i].used = false;
            break;
        }
    }
    Refresh();
}

/**
	@brief Draw data markers
*/
void OpenGLGraph::DrawMarkers()
{
        if(series.size() <= 0)
            return;

	if(settings.markersEnabled && series[0]->size > 0 && series[0] != NULL)
	{
		switchToWindowView();

		float pixelXvalue =(settings.visibleArea.x2 - settings.visibleArea.x1) / settings.dataViewWidth;
		float pixelYvalue = (settings.visibleArea.y2 - settings.visibleArea.y1)/  settings.dataViewHeight;

		glBegin(GL_TRIANGLES);
		int posX, posY;
		for(unsigned i=0; i<markers.size(); ++i)
		{
		    if(markers[i].used == false)
                        continue;
                    for(unsigned int j=0; j<series.size(); j++)
                    {
                        if(series[j]->size > 0 && series[j]->visible)
                        {
                            markers[i].posY = series[j]->values[markers[i].dataValueIndex+1];
                            // X axis grid lines
                            posY = settings.marginBottom + ((series[j]->values[markers[i].dataValueIndex+1]-settings.visibleArea.y1)/pixelYvalue);
                            posX = settings.marginLeft + ((markers[i].posX-settings.visibleArea.x1)/pixelXvalue);
                            markers[i].iposX = posX;
                            markers[i].iposY = posY;
                            markers[i].size = 10;

                            if(posX >= settings.marginLeft && posX <= settings.windowWidth-settings.marginRight)
                            {
                                markers[i].color = mMarkerColors[i];
                                    glColor4f(markers[i].color.red, markers[i].color.green, markers[i].color.blue, markers[i].color.alpha);
                                    if( posY >= settings.marginBottom && posY <= settings.windowHeight-settings.marginTop)
                                    {
                                            glVertex3f( posX, posY, 10);
                                            glVertex3f( posX+markers[i].size, posY+markers[i].size, 10);
                                            glVertex3f( posX-markers[i].size, posY+markers[i].size, 10);
                                    }
                                    glVertex3f( posX, settings.marginBottom+markers[i].size, 10);
                                    glVertex3f( posX-markers[i].size, settings.marginBottom, 10);
                                    glVertex3f( posX+markers[i].size, settings.marginBottom, 10);
                            }
                        }
                    }
		}
		glEnd();
		glFlush();

		//draw marker data at the right upper corner of data view
		char text[256];
		float textScale = 1;
		int hpos = textScale*m_font->lineHeight()*2.5;
		for(unsigned i=0; i<markers.size(); ++i)
		{
		    if(markers[i].used == false)
                        continue;
                    glColor4f(markers[i].color.red, markers[i].color.green, markers[i].color.blue, markers[i].color.alpha);
                    int cnt = sprintf(text, "M%i: % .3f MHz ", i, series[0]->values[markers[i].dataValueIndex]/1000000);

                    for(unsigned int j=0; j<series.size(); j++)
                        if(series[j]->size > 0 && series[j]->visible)
                            cnt += sprintf(text+cnt, "/ Ch %c: %#+3.1f dBFS ", 65+j, series[j]->values[markers[i].dataValueIndex+1]);

                    markers[i].posY = series[0]->values[markers[i].dataValueIndex+1];
                    posX = settings.marginLeft;
                    posY = settings.windowHeight-settings.marginTop - hpos;
                    //glPrint(posX, posY, 0, textScale, "%s", text);
                    if(markers[i].show == false)
                        continue;
                    hpos += textScale*m_font->lineHeight();
		    glRenderText(posX, posY, 0, textScale*m_font->lineHeight(), markers[i].color.getColor4b(), "%s", text);
		}
	}
}

/**
	@brief Checks if mouse clicked on marker
	@param X mouse x coorindate
	@param Y mouse y coorindate
	@returns clicked marker index
*/
int OpenGLGraph::clickedOnMarker(int X, int Y)
{
	for(unsigned i=0; i<markers.size(); ++i)
	{
	    if(markers[i].used == false)
            continue;
		if( X > markers[i].iposX-markers[i].size &&  X < markers[i].iposX+markers[i].size )
		{
			if(Y > settings.marginBottom && Y < settings.marginBottom+markers[i].size)
			{
				printf("selected %i marker\n", i);
                                return i;
                        }

                        for(unsigned int j=0; j<series.size(); j++)
                        {
                            if(series[j]->size > 0 && series[j]->visible)
                            {
                              float pixelYvalue = (settings.visibleArea.y2 - settings.visibleArea.y1)/  settings.dataViewHeight;
                              int posY = settings.marginBottom + ((series[j]->values[markers[i].dataValueIndex+1]-settings.visibleArea.y1)/pixelYvalue);
                              if(Y > posY && Y < posY+markers[i].size)
                              {
                                printf("selected %i marker\n", i);
                                return i;
                              }
                            }
                        }

		}
	}
	return -1;
}

/**
	@brief change marker position
	@param markerID index of marker to be moved
	@param posX mouse x coordinate in graph window
*/
void OpenGLGraph::MoveMarker(int markerID, int posX)
{
	if(series[0]->size > 0 && markerID >= 0)
	{
	    if(markers[markerID].used == false)
            return;
	    float tempY = 0;
		dataViewPixelToValue(posX-settings.marginLeft, 0, markers[markerID].posX, tempY);

		//determine which way marker was moved, search for closest data point
		if( posX < markers[markerID].iposX )
		{
			for(int i=markers[markerID].dataValueIndex; i>0; i-=2)
			{
				if ( series[0]->values[i] <= markers[markerID].posX )
				{
					float toLeft = markers[markerID].posX-series[0]->values[i-2];
					float toRight = series[0]->values[i]-markers[markerID].posX;
					if( toRight < toLeft)
					{
						markers[markerID].dataValueIndex = i;
					}
					else
					{
						markers[markerID].dataValueIndex = i-2;
					}
					break;
				}
			}
		}
		else
		{
			for(unsigned i=markers[markerID].dataValueIndex; i<series[0]->size*2 ; i+=2)
			{
				if ( series[0]->values[i] >= markers[markerID].posX )
				{
					float toLeft = markers[markerID].posX-series[0]->values[i-2];
					float toRight = series[0]->values[i]-markers[markerID].posX;
					if( toRight < toLeft)
					{
						markers[markerID].dataValueIndex = i;
					}
					else
					{
						markers[markerID].dataValueIndex = i-2;
					}
					break;
				}
			}
		}
		float pixelXvalue = (settings.visibleArea.x2 - settings.visibleArea.x1)/  settings.dataViewWidth;
		float pixelYvalue = (settings.visibleArea.y2 - settings.visibleArea.y1)/  settings.dataViewHeight;
		markers[markerID].posX = series[0]->values[markers[markerID].dataValueIndex];
		markers[markerID].posY = series[0]->values[markers[markerID].dataValueIndex+1];
		int posY = settings.marginBottom + ((series[0]->values[markers[markerID].dataValueIndex+1]-settings.visibleArea.y1)/pixelYvalue);
		int posX = settings.marginLeft + ((markers[markerID].posX-settings.visibleArea.x1)/pixelXvalue);
		markers[markerID].iposX = posX;
		markers[markerID].iposY = posY;
	}
    mMarkersDlg->refreshMarkFreq = true;
	#ifdef OGL_REDRAW_ENABLED
        Refresh();
	#endif
}

/**
	@brief change marker position
	@param markerID index of marker to be moved
	@param posX mouse x coordinate in graph window
*/
void OpenGLGraph::ChangeMarker(int markerID, float xValue)
{
	if(series[0]->size > 0 && markerID >= 0)
	{
	    OGLMarker* mark = NULL;
        for(size_t i=0; i<markers.size(); ++i)
            if(markers[i].id == markerID)
                mark = &markers[i];
        if(mark == NULL)
            return;

        mark->posX = xValue;
        bool found = false;
        //find closest data point index
        for(unsigned i=0; i<series[0]->size; ++i)
        {
            if ( series[0]->values[2*i] > mark->posX )
            {
                float toLeft = mark->posX-series[0]->values[2*(i-1)];
                float toRight = series[0]->values[2*i]-mark->posX;
                if( toRight < toLeft)
                {
                    mark->dataValueIndex = 2*i;
                }
                else
                {
                    mark->dataValueIndex = 2*(i-1);
                }
                found = true;
                break;
            }
        }
        if(!found) //if no closest point found, add marker to middle point
        {
            mark->dataValueIndex = series[0]->size;
        }
        mark->posX = series[0]->values[mark->dataValueIndex];
        mark->posY = series[0]->values[mark->dataValueIndex+1];
	}
	Refresh();
	#ifdef OGL_REDRAW_ENABLED
        Refresh();
	#endif
}

void OpenGLGraph::setupViewport(int w, int h)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void OpenGLGraph::render(wxPaintEvent& evt)
{
    Draw();
}

void OpenGLGraph::resized(wxSizeEvent& evt)
{
    wxSize wh;
    wh = evt.GetSize();
    Resize(wh.GetWidth() , wh.GetHeight());
    Refresh();
}

void OpenGLGraph::mouseMoved(wxMouseEvent& event)
{
    OnMouseMove(event.GetX(), event.GetY());
}

void OpenGLGraph::mouseWheelMoved(wxMouseEvent& event)
{

}

void OpenGLGraph::mouseReleased(wxMouseEvent& event)
{
    if(event.LeftUp())
        OnMouseUp(OGLG_LEFT, event.GetX(), event.GetY());
    else if(event.RightUp())
        OnMouseUp(OGLG_RIGHT, event.GetX(), event.GetY());
    else if(event.MiddleUp())
        OnMouseUp(OGLG_MIDDLE, event.GetX(), event.GetY());
}
void OpenGLGraph::rightClick(wxMouseEvent& event)
{
    OnMouseDown(OGLG_RIGHT, event.GetX(), event.GetY());
}
void OpenGLGraph::leftClick(wxMouseEvent& event)
{
    OnMouseDown(OGLG_LEFT, event.GetX(), event.GetY());
}
void OpenGLGraph::middleClick(wxMouseEvent& event)
{
    OnMouseDown(OGLG_MIDDLE, event.GetX(), event.GetY());
}
void OpenGLGraph::mouseLeftWindow(wxMouseEvent& event)
{

}
void OpenGLGraph::keyPressed(wxKeyEvent& event)
{

}
void OpenGLGraph::keyReleased(wxKeyEvent& event)
{

}

void OpenGLGraph::onFit(wxCommandEvent& event)
{
    Fit();
}

void OpenGLGraph::onMouseHelp(wxCommandEvent& event)
{
    wxMessageBox(_("Supported Mouse commands:\n \
        - Left button down + Mark area: Rectangular zoom\n \
        - Right button down + Move: Pan (Move)\n \
        - Wheel button down + Move: vertical & horizontal zooming\n"
        ),_("plot controls"),wxOK,this);
}

void OpenGLGraph::onAddMarker(wxCommandEvent& event)
{
    AddMarker(m_MouseCoord.x2);
    Refresh();
}
void OpenGLGraph::onRemoveMarker(wxCommandEvent& event)
{
    RemoveMarker();
    Refresh();
}

void OpenGLGraph::onShowMarkersMenu(wxCommandEvent& event)
{
    if(m_timer->IsRunning() == false)
        m_timer->Start(500);
    mMarkersDlg->Show();
    mMarkersDlg->refreshMarkFreq = true;;
}

void OpenGLGraph::onLockAspect(wxCommandEvent& event)
{
    settings.lock_aspect = event.GetInt();
}

void OpenGLGraph::ShowMenu(int x, int y)
{
    //modify menu
    wxMenuItem *item = NULL;
    if(settings.markersEnabled == false)
    {
        item = m_popmenu.FindItem(OGLG_ADD_MARK);
        item->Enable(false);
        item = m_popmenu.FindItem(OGLG_REMOVE_MARKER);
        item->Enable(false);
        item = m_popmenu.FindItem(OGLG_SEARCH_PEAK);
        item->Enable(false);
        item = m_popmenu.FindItem(OGLG_SHOW_MARKERS_MENU);
        item->Enable(false);
    }
    PopupMenu( &m_popmenu, x, y);
}

void OpenGLGraph::onSearchPeak(wxCommandEvent& event)
{
    SearchPeak();
    Refresh();
    mMarkersDlg->refreshMarkFreq = true;;
}

void OpenGLGraph::onReset(wxCommandEvent& event)
{
    ResetView();
    Refresh();
}

bool OpenGLGraph::SearchPeak()
{
    bool found = false;
    double maxValue(0.0);
    unsigned maxPos(0);
    //Init max value
    for(unsigned int i=0; i<series.size(); i++)
    {
        if(series[i]->size > 0 && series[i]->visible)
        {
            maxValue = series[i]->values[1];
            found = true;
            break;
        }
    }
    //Find max position
    for(unsigned int i=0; i<series.size(); i++)
    {
        if(series[i]->size > 0 && series[i]->visible)
        {
            for(unsigned j=0; j<series[i]->size; ++j)
            {
                if(maxValue < series[i]->values[2*j+1])
                {
                    maxValue = series[i]->values[2*j+1];
                    maxPos = 2*j+1;
                }
            }
        }
    }

    //Mark max position
    for(unsigned int i=0; i<series.size(); i++)
    {
        if(series[i]->size > 0 && series[i]->visible)
        {
            AddMarkerAtValue(series[i]->values[maxPos-1]);
            break;
        }
    }
    return found;
}

void OpenGLGraph::SetMarker(int id, float xValue, bool enabled, bool show)
{
    if(id >=0 && id < (int)markers.size())
    {
        ChangeMarker(id, xValue);
        markers[id].used = enabled;
        markers[id].show = show;
    }
}

void OpenGLGraph::GetMarker(int id, float &xValue, float &yValue, bool &enabled, bool &show)
{
    if(id >=0 && id < (int)markers.size())
    {
        xValue = markers[id].posX;
        yValue = markers[id].posY;
        enabled = markers[id].used;
        show = markers[id].show;
    }
}

void OpenGLGraph::OnTimer(wxTimerEvent& event)
{
    mMarkersDlg->UpdateValues();
    UpdateInfoDisplay();
}

void OpenGLGraph::UpdateInfoDisplay()
{
    for(size_t i=0; i<info_msg.size(); ++i)
    {
        info_msg_toDisplay[i] = info_msg[i];
    }
}

void OpenGLGraph::SetInfoMessage(const char* msg, unsigned int index)
{
    if(index < info_msg.size())
        info_msg[index] = msg;
}

BEGIN_EVENT_TABLE(OpenGLGraph, wxGLCanvas)
EVT_MOTION(OpenGLGraph::mouseMoved)
EVT_RIGHT_DOWN(OpenGLGraph::rightClick)
EVT_RIGHT_UP(OpenGLGraph::mouseReleased)
EVT_LEFT_DOWN(OpenGLGraph::leftClick)
EVT_LEFT_UP(OpenGLGraph::mouseReleased)
EVT_MIDDLE_UP(OpenGLGraph::mouseReleased)
EVT_MIDDLE_DOWN(OpenGLGraph::middleClick)
EVT_LEAVE_WINDOW(OpenGLGraph::mouseLeftWindow)
EVT_SIZE(OpenGLGraph::resized)
EVT_MOUSEWHEEL(OpenGLGraph::mouseWheelMoved)
EVT_PAINT(OpenGLGraph::render)
EVT_TIMER(wxID_ANY, OpenGLGraph::OnTimer)

EVT_MENU( OGLG_FIT,       OpenGLGraph::onFit)
EVT_MENU( OGLG_HELP_MOUSE, OpenGLGraph::onMouseHelp)
EVT_MENU( OGLG_ADD_MARK, OpenGLGraph::onAddMarker)
EVT_MENU( OGLG_REMOVE_MARKER, OpenGLGraph::onRemoveMarker)
EVT_MENU( OGLG_SHOW_MARKERS_MENU, OpenGLGraph::onShowMarkersMenu)
EVT_MENU( OGLG_LOCKASPECT, OpenGLGraph::onLockAspect)
EVT_MENU( OGLG_SEARCH_PEAK, OpenGLGraph::onSearchPeak)
EVT_MENU( OGLG_RESET, OpenGLGraph::onReset)
END_EVENT_TABLE()
