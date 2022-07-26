/***************************************************************
 * Name:      lms7projectApp.h
 * Purpose:   Defines Application Class
 * Author:    Lime Microsystems ()
 * Created:   2015-03-05
 * Copyright: Lime Microsystems (limemicro.com)
 * License:
 **************************************************************/

#ifndef LMS7PROJECTAPP_H
#define LMS7PROJECTAPP_H

#include <wx/app.h>

class ConnectionManager;

class lms7suiteApp : public wxApp
{
    public:
        virtual bool OnInit();		
};

#endif // LMS7PROJECTAPP_H
