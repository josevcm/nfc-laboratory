#ifndef XTENSOR_IO_FILE_WRAPPER_HPP
#define XTENSOR_IO_FILE_WRAPPER_HPP

#include <stdio.h>

namespace xt
{
    class xfile_wrapper
    {
    public:
        xfile_wrapper(FILE* pfile);
        std::streampos tellg();
        xfile_wrapper& read_all(std::string& s);
        xfile_wrapper& read(char* s, std::streamsize n);
        void write(const char* s, std::streamsize size);
        std::streamsize gcount();
        void flush();
        bool eof();
    private:
        FILE* m_pfile;
        std::size_t m_gcount;
    };

    inline xfile_wrapper::xfile_wrapper(FILE* pfile)
        : m_pfile(pfile)
    {
    }

    inline std::streampos xfile_wrapper::tellg()
    {
        return ftell(m_pfile);
    }

    inline xfile_wrapper& xfile_wrapper::read_all(std::string& s)
    {
        fseek(m_pfile, 0, SEEK_END);
        std::size_t size = ftell(m_pfile);
        s.resize(size);
        rewind(m_pfile);
        m_gcount = fread(&s[0], 1, size, m_pfile);
        return *this;
    }

    inline xfile_wrapper& xfile_wrapper::read(char* s, std::streamsize n)
    {
        m_gcount = fread(&s[0], 1, n, m_pfile);
        return *this;
    }

    inline std::streamsize xfile_wrapper::gcount()
    {
        return m_gcount;
    }

    inline void xfile_wrapper::write(const char* s, std::streamsize size)
    {
        fwrite(s, 1, size, m_pfile);
    }

    inline void xfile_wrapper::flush()
    {
        fflush(m_pfile);
    }

    inline bool xfile_wrapper::eof()
    {
        return feof(m_pfile);
    }
}

#endif
