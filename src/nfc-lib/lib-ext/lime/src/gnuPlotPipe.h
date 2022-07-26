#ifndef GNUPLOT_PIPE_H
#define GNUPLOT_PIPE_H

#include <stdio.h>
#include <cstdio>
#include <stdarg.h>

class GNUPlotPipe
{
public:
    GNUPlotPipe(bool persistent = true)
    {
#ifdef __unix__
        if(persistent)
            pipeHandle = popen("gnuplot -persistent", "w");
        else
            pipeHandle = popen("gnuplot", "w");
#else
        if(persistent)
            pipeHandle = _popen("gnuplot -persistent", "w");
        else
            pipeHandle = _popen("gnuplot", "w");
#endif       
    }
    ~GNUPlotPipe()
    {
        write("exit\n");
#ifdef __unix__
        pclose(pipeHandle);
#else
        _pclose(pipeHandle);
#endif
    }

    void write(const char* str)
    {
        fprintf(pipeHandle, "%s", str);
        fflush(pipeHandle);
    }

    void writef(const char* format, ...)
    {
        static char temp[512];
        va_list argList;
        va_start(argList, format);
        vsnprintf(temp, 512, format, argList);
        write(temp);
        va_end(argList);
    }

    void flush()
    {
        fflush(pipeHandle);
    }
protected:
    FILE *pipeHandle;
};

#endif
