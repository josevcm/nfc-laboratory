#pragma once

#include <vector>
#include "z5/types/types.hxx"

namespace z5 {
namespace compression {

    // abstract basis class for compression
    template<typename T>
    class CompressorBase {

    public:
        //
        // API -> must be implemented by child classes
        //

        virtual ~CompressorBase() {}
        virtual void compress(const T *, std::vector<char> &, std::size_t) const = 0;
        virtual void decompress(const std::vector<char> &, T *, std::size_t) const = 0;
        virtual types::Compressor type() const = 0;
        virtual void getOptions(types::CompressionOptions &) const = 0;
    };


}
} // namespace::z5
