#ifndef XTENSOR_IO_STREAM_WRAPPER_HPP
#define XTENSOR_IO_STREAM_WRAPPER_HPP

#include <stdio.h>

namespace xt
{
    class xistream_wrapper
    {
    public:
        xistream_wrapper(std::istream& stream);
        xistream_wrapper& read_all(std::string& s);
        xistream_wrapper& read(char* s, std::streamsize n);
        bool eof();
        std::streamsize gcount();
    private:
        std::istream& m_stream;
    };

    inline xistream_wrapper::xistream_wrapper(std::istream& stream)
        : m_stream(stream)
    {
    }

    inline xistream_wrapper&  xistream_wrapper::read_all(std::string& s)
    {
        s = {std::istreambuf_iterator<char>{m_stream}, {}};
        return *this;
    }

    inline xistream_wrapper& xistream_wrapper::read(char* s, std::streamsize n)
    {
        m_stream.read(s, n);
        return *this;
    }

    inline bool xistream_wrapper::eof()
    {
        return m_stream.eof();
    }

    inline std::streamsize xistream_wrapper::gcount()
    {
        return m_stream.gcount();
    }



    class xostream_wrapper
    {
    public:
        xostream_wrapper(std::ostream& stream);
        xostream_wrapper& write(const char* s, std::streamsize n);
        void flush();
    private:
        std::ostream& m_stream;
    };

    inline xostream_wrapper::xostream_wrapper(std::ostream& stream)
        : m_stream(stream)
    {
    }

    inline xostream_wrapper& xostream_wrapper::write(const char* s, std::streamsize n)
    {
        m_stream.write(s, std::streamsize(n));
        return *this;
    }

    inline void xostream_wrapper::flush()
    {
        m_stream.flush();
    }
}

#endif
