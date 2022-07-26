#ifdef QUICKTEST_GUI
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Box.H>
#include "FL/fl_draw.H"
#endif
#include <vector>
#include <string>
#include <fstream>

#include "LimeSDRTest.h"
#include <iostream>
#include <getopt.h>

#ifdef QUICKTEST_GUI
#ifdef NDEBUG
#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif

static Fl_Button** buttons;
static Fl_Window* popup = nullptr;
static Fl_Multiline_Output* out;
static Fl_Output* detect;
static std::string logfile;
static int started = false;
Fl_Button* start;

const Fl_Color CL_DARK = fl_rgb_color(0x18, 0x8A, 0x2E);

class Lms_Double_window : public Fl_Double_Window
{
private:
    int error_msg;
public:
    Lms_Double_window(int W, int H, const char* L = 0) : Fl_Double_Window(W, H, L)
    {
        error_msg = 0;
    };
    void SetErrorMsg(int msg) {
        error_msg = msg;
    }
    void draw()
    {
        Fl_Double_Window::draw();
        if (error_msg != 0)
        {
            fl_rectf(w() / 4, h() / 4, w() / 2, h() / 4,FL_WHITE);
            fl_rect(w() / 4, h() / 4, w() / 2, h() / 4, FL_BLACK);
            fl_color(FL_RED);
            fl_font(FL_HELVETICA, 20);
            if (error_msg == -1)
            {
                fl_draw("No Board Detected", 320, h()/4+50);
                fl_font(FL_HELVETICA, 20);
            }
            else if (error_msg == -2)
            {
                fl_font(FL_HELVETICA, 20);
                fl_draw("Multiple Boards Detected", 295, h() / 4 + 50);
            }
        }
    }
};

Lms_Double_window* win;

class LmsPopupResult : public Fl_Window {
public:
    LmsPopupResult(int X, int Y, int W, int H, bool passed, const char* L=0) :Fl_Window(W, H, L)
    {
        position(X, Y);
        color(FL_WHITE);
        Fl_Button* b = new Fl_Button(W / 2 - 40, H - 50, 80, 30, "OK");
        b->callback(Button_CB, this);
        b->shortcut(FL_Enter);
        Fl_Multiline_Output* out = new Fl_Multiline_Output(64, 15, W - 128, 50);
        out->box(FL_NO_BOX);
        out->textsize(24);
        out->textcolor(passed ? CL_DARK : FL_RED);
        out->value(passed ? "Board Tests PASSED" : "Board Tests FAILED");
        end();
        set_modal();
        show();
    }
    ~LmsPopupResult() {};
private:
    static void Button_CB(Fl_Widget* obj, void* v)
    {
        ((Fl_Window*)v)->hide();
    }
};

class LmsPopupError : public Fl_Window {
    Fl_Output* out;
public:
    LmsPopupError(int W, int H, const char* L = 0) :Fl_Window(W, H, L)
    {
        color(FL_WHITE);
        out = new Fl_Output(64, 15, W - 128, 40);
        out->box(FL_NO_BOX);
        out->textsize(32);
        out->textcolor(FL_RED);
        end();
        set_modal();
    };
    void Update(int X, int Y, const char* msg)
    {
        position(X, Y);
        out->value(msg);
    }
    ~LmsPopupError() {};
};

static LmsPopupError* error_popup = nullptr;

void ShowMessage(void* v)
{
    if (popup != nullptr)
    {
        popup->hide();
        delete popup;
        popup = nullptr;
    }
    bool passed = *((bool*)v);
    popup = new LmsPopupResult(win->x() + win->w() / 4, win->y() + win->h() / 4, win->w() / 2, 120, passed, "");
    while (popup && popup->shown())
        Fl::wait();
    delete (bool*)v;
    delete popup;
    popup = nullptr;
}

void Timer_CB(void *data)
{
    static int counter = 30;
    Fl::repeat_timeout(1.0f / 25.0f, Timer_CB, nullptr);

    if (counter++ == 30)
    {
        counter = 0;
        Fl::lock();
        std::string str;

        win->SetErrorMsg(0);
        int ret;
        str = "";
        if ((ret = LimeSDRTest::CheckDevice(str)) == 0)
        {
            detect->textcolor(CL_DARK);
            if (!started)
                start->activate();
            win->SetErrorMsg(0);
        }
        else
        {
            detect->textcolor(FL_RED);
            if (!started)
                start->deactivate();
            if (popup != nullptr)
                popup->hide();
            win->SetErrorMsg(ret);
        }
        if (str != "")
            detect->value(str.c_str());

        win->redraw();
        win->damage(FL_DAMAGE_ALL, 150, 460, 320, 30);
        Fl::unlock();
    }
}

static void Window_CB(Fl_Widget*, void*)
{
    if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
        return; // ignore Escape
    exit(0);
}

static void AddLine(const char * str)
{
    std::string val;
    val = out->value();
    val += str;
    val += "\n";
    out->value(val.c_str());
    out->position(val.length());
    if (logfile != "")
    {
        std::ofstream file(logfile, std::ios_base::out | std::ios_base::app);
        if (file.is_open())
        {
            file << str << std::endl;
            file.close();
        }
    }
}

static void DrawButtons(int X, int Y)
{
    buttons = new Fl_Button*[testNames.size()];
    Fl_Group* g = new Fl_Group(X-10,Y-8,240,testNames.size()*55+10,"Board Tests");
    //g->align(FL_ALIGN_TOP_LEFT);
    g->box(FL_ENGRAVED_FRAME);
    for (unsigned i = 0; i < testNames.size(); i++)
    {
        auto button = new Fl_Button(X, Y+i*55, 220, 40,testNames[i].c_str());
        button->box(FL_ROUND_UP_BOX);
        button->deactivate();
        buttons[i] = button;
    }
    g->end();
}

static int CB_Function(int id, int event, const char* msg)
{
    Fl::lock();
    if (event == LMS_TEST_INFO)
    {
        if (id != -1)
        {
            buttons[id]->activate();
            buttons[id]->color(FL_YELLOW);
        }
    }
    else if (event == LMS_TEST_FAIL)
    {
        if (id != -1)
        {
            buttons[id]->activate();
            buttons[id]->color(FL_RED);
        }
        else
        {
            bool* passed = new bool;
            *passed = false;
            started = false;
            Fl::awake(ShowMessage, passed);
        }
    }
    else if (event == LMS_TEST_SUCCESS)
    {
        if (id != -1)
        {
            buttons[id]->activate();
            buttons[id]->color(FL_GREEN);
        }
        else
        {
            bool* passed = new bool;
            *passed = true;
            started = false;
            Fl::awake(ShowMessage, passed);
        }
    }
    else if (event == LMS_TEST_LOGFILE)
    {
        logfile = "LimeSDR-Mini_";
        logfile += msg;
        logfile +=".log";
        std::ofstream file(logfile, std::ios_base::out | std::ios_base::app);
        if (file.is_open())
        {
            file << out->value();
            file.close();
        }
        std::string str = "  Serial Number: ";
        str += msg;
        AddLine(str.c_str());
    }

    if (msg && event != LMS_TEST_LOGFILE)
        AddLine(msg);

    if (id == -1)
    start->activate();

    win->redraw();
    Fl::unlock();
    return 0;
}

static void Start_CB(Fl_Widget*, void*)
{
    out->value("");
    logfile = "";
    for (unsigned i = 0; i < testNames.size(); i++)
    {
        buttons[i]->color(FL_BACKGROUND_COLOR);
        buttons[i]->deactivate();
    }
    started = true;
    if (LimeSDRTest::RunTests(CB_Function) == 0)
        start->deactivate();
}
#endif

static int CB_FunctionCLI(int id, int event, const char* msg)
{
    if (event == LMS_TEST_LOGFILE)
    {
        std::string str = "  Serial Number: ";
        str += msg;
        std::cout << str << std::endl;
    }
    else
        std::cout << msg << std::endl;
    return 0;
}


int main(int argc, char** argv)
{
#ifdef QUICKTEST_GUI
#ifdef __unix
    int gui = 0;
#else
    int gui = 1;
#endif
    struct option long_options[] = {
        {"gui", no_argument, &gui, 1},
        {"no-gui", no_argument, &gui, 0},
        {0, 0, 0, 0}
    };

    int option;
    int option_index = 0;
    while ((option = getopt_long(argc, argv, "", long_options, &option_index)) != -1)
    {
        if (option != 0)
        {
            std::cout << " --gui\t\tenables GUI\n --no-gui\tdisables GUI\n" << std::endl;
            return -1;
        }
    }
    if (gui)
    {
        Fl::scheme("gleam");
        win = new Lms_Double_window(860, 135+testNames.size()*55, "LimeSDR TestApp");
        out = new Fl_Multiline_Output(260, 22, win->w()-270, win->h()-50, "Message Log");
        out->align(FL_ALIGN_TOP_LEFT);
        out->textsize(11);
        start = new Fl_Button(20, win->h()-82, 220, 40, "Start Test");
        start->callback(Start_CB);
        start->shortcut(FL_Enter);
        DrawButtons(20, 30);
        detect = new Fl_Output(10, win->h()-28, 480, 30);
        detect->box(FL_NO_BOX);
        detect->textsize(16);
        win->show();
        win->end();
        error_popup = new LmsPopupError(win->w()/2, win->h()/4,"");
        win->callback(Window_CB);
        Fl::add_timeout(0.1f, Timer_CB, nullptr);
        Fl::lock();
        Fl::run();
        delete win;
        return 0;
    }
#endif
    return LimeSDRTest::RunTests(CB_FunctionCLI, false); //returns bit field of failed tests

}



