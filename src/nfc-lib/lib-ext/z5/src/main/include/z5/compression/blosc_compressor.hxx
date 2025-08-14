#pragma once

#ifdef WITH_BLOSC

#include <blosc.h>
#include "z5/compression/compressor_base.hxx"
#include "z5/metadata.hxx"

namespace z5 {
namespace compression {

    template<typename T>
    class BloscCompressor : public CompressorBase<T> {

    public:
        BloscCompressor(const DatasetMetadata & metadata) {
            init(metadata);
        }

        void compress(const T * dataIn, std::vector<char> & dataOut, std::size_t sizeIn) const {

            const std::size_t sizeOut = sizeIn * sizeof(T) + BLOSC_MAX_OVERHEAD;
            dataOut.clear();
            dataOut.resize(sizeOut);

            // compress the data
            int sizeCompressed = blosc_compress_ctx(
                clevel_, shuffle_,
                sizeof(T),
                sizeIn * sizeof(T), dataIn,
                &dataOut[0], sizeOut,
                compressor_.c_str(),
                blocksize_,
                nthreads_
            );

            // check for errors
            if(sizeCompressed <= 0) {
                throw std::runtime_error("Blosc compression failed");
            }

            // resize the out data
            dataOut.resize(sizeCompressed);
        }

        void decompress(const std::vector<char> & dataIn, T * dataOut, std::size_t sizeOut) const {

            // decompress the data
            int sizeDecompressed = blosc_decompress_ctx(
                &dataIn[0], dataOut,
                sizeOut * sizeof(T),
                nthreads_
            );

            // check for errors
            if(sizeDecompressed <= 0) {
                throw std::runtime_error("Blosc decompression failed");
            }
        }

        inline types::Compressor type() const {
            return types::blosc;
        }

        inline void getOptions(types::CompressionOptions & opts) const {
            opts["codec"] = compressor_;
            opts["shuffle"] = shuffle_;
            opts["level"] = clevel_;
            opts["blocksize"] = blocksize_;
            opts["nthreads"] = nthreads_;
        }

    private:
        // set the compression parameters from metadata
        void init(const DatasetMetadata & metadata) {
            const auto & cOpts = metadata.compressionOptions;

            clevel_     = std::get<int>(cOpts.at("level"));
            shuffle_    = std::get<int>(cOpts.at("shuffle"));
            compressor_ = std::get<std::string>(cOpts.at("codec"));
            blocksize_ = std::get<int>(cOpts.at("blocksize"));

            // set nthreads with a default value of 1
            nthreads_ = 1;
            auto threadsIt = cOpts.find("nthreads");
            if(threadsIt != cOpts.end()) {
                nthreads_ = std::get<int>(threadsIt->second);
            }
        }

        // the blosc compressor
        std::string compressor_;
        // compression level
        int clevel_;
        // blosc shuffle
        int shuffle_;
        int blocksize_;
        int nthreads_;
    };

} // namespace compression
} // namespace z5

#endif
