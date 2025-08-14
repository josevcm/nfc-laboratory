#pragma once

#ifdef WITH_LZ4

#include <lz4.h>
#include <lz4hc.h>

#include "z5/compression/compressor_base.hxx"
#include "z5/metadata.hxx"


namespace z5 {
namespace compression {

    // TODO I don't know how to set the compression level properly
    // TODO Does n5-java use lz4 or lz4hc
    template<typename T>
    class Lz4Compressor : public CompressorBase<T> {

    public:
        Lz4Compressor(const DatasetMetadata & metadata) {
            init(metadata);
        }

        void compress(const T * dataIn, std::vector<char> & dataOut, std::size_t sizeIn) const {
            const int outSize = LZ4_compressBound(sizeIn * sizeof(T));
            dataOut.resize(outSize);
            // the default compression (fast mode)
            // const int compressed = LZ4_compress_fast((const char *) dataIn,
            //                                          &dataOut[0],
            //                                          sizeIn * sizeof(T),
            //                                          outSize,
            //                                          level_);
            // default compression (default mode without parameters)
            const int compressed = LZ4_compress_default((const char *) dataIn,
                                                        &dataOut[0],
                                                        sizeIn * sizeof(T),
                                                        outSize);
            // high compression mode
            // const int compressed = LZ4_compress_HC((const char *) dataIn,
            //                                        &dataOut[0],
            //                                        sizeIn * sizeof(T),
            //                                        outSize,
            //                                        level_);

            if(compressed <= 0) {
                std::string err = "Exception during lz4 compression: (" + std::to_string(compressed)  + ")";
    		    throw std::runtime_error(err);
            }

            dataOut.resize(compressed);
        }

        void decompress(const std::vector<char> & dataIn, T * dataOut, std::size_t sizeOut) const {
            const int compressed = LZ4_decompress_safe(&dataIn[0], (char *) dataOut,
                                                       dataIn.size(), sizeOut * sizeof(T));
            if(compressed <= 0) {
                std::string err = "Exception during lz4 decompression: (" + std::to_string(compressed)  + ")";
    		    throw std::runtime_error(err);
            }
		}

        inline types::Compressor type() const {
            return types::lz4;
        }

        inline void getOptions(types::CompressionOptions & opts) const {
            opts["level"] = level_;
        }

    private:
        void init(const DatasetMetadata & metadata) {
            // appropriate for hc compression
            // level_ = std::get<int>(metadata.compressionOptions.at("level"));

            // appropriate for fast compression
            level_ = 10 - std::get<int>(metadata.compressionOptions.at("level"));
        }

        // compression level
        int level_;
    };

} // namespace compression
} // namespace z5

#endif
