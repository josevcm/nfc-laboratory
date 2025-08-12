#ifndef XTENSOR_IO_VSILFILE_WRAPPER_HPP
#define XTENSOR_IO_VSILFILE_WRAPPER_HPP

#include <cpl_vsi.h>

namespace xt
{
    class xvsilfile_wrapper
    {
    public:
        xvsilfile_wrapper(VSILFILE* pfile);
        std::streampos tellg();
        xvsilfile_wrapper& read_all(std::string& s);
        xvsilfile_wrapper& read(char* s, std::streamsize n);
        void write(const char* s, std::size_t size);
        std::streamsize gcount();
        void flush();
        bool eof();
    private:
        VSILFILE* m_pfile;
        std::size_t m_gcount;
    };

    inline xvsilfile_wrapper::xvsilfile_wrapper(VSILFILE* pfile)
        : m_pfile(pfile)
    {
    }

    inline std::streampos xvsilfile_wrapper::tellg()
    {
        return VSIFTellL(m_pfile);
    }

    inline xvsilfile_wrapper& xvsilfile_wrapper::read_all(std::string& s)
    {
        VSIFSeekL(m_pfile, 0, SEEK_END);
        std::size_t size = VSIFTellL(m_pfile);
        s.resize(size);
        VSIRewindL(m_pfile);
        m_gcount = VSIFReadL(&s[0], 1, size, m_pfile);
        return *this;
    }

    inline xvsilfile_wrapper& xvsilfile_wrapper::read(char* s, std::streamsize n)
    {
        m_gcount = VSIFReadL(&s[0], 1, n, m_pfile);
        return *this;
    }

    inline std::streamsize xvsilfile_wrapper::gcount()
    {
        return m_gcount;
    }

    inline void xvsilfile_wrapper::write(const char* s, std::size_t size)
    {
        VSIFWriteL(s, 1, size, m_pfile);
    }

    inline void xvsilfile_wrapper::flush()
    {
        VSIFFlushL(m_pfile);
    }

    inline bool xvsilfile_wrapper::eof()
    {
        return VSIFEofL(m_pfile);
    }
}

#endif
