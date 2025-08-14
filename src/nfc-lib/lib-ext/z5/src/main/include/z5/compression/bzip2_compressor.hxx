#pragma once

#ifdef WITH_BZIP2

#include <bzlib.h>

#include "z5/compression/compressor_base.hxx"
#include "z5/metadata.hxx"

namespace z5 {
namespace compression {

    // TODO gzip encoding
    template<typename T>
    class Bzip2Compressor : public CompressorBase<T> {

    public:
        Bzip2Compressor(const DatasetMetadata & metadata) {
            init(metadata);
        }

        void compress(const T * dataIn, std::vector<char> & dataOut, std::size_t sizeIn) const {

            // resize the out data
            dataOut.clear();
            // the output buffer must be 1% + 600 bytes larger than the input
            std::size_t outSize = sizeof(T) * (sizeIn + static_cast<std::size_t>(0.01 * static_cast<float>(sizeIn)));
            outSize += 600;
            dataOut.resize(outSize);

            // low - level API
            // open the bzip2 stream and set pointer values
            bz_stream bzs;
            bzs.opaque = NULL;
            bzs.bzalloc = NULL;
            bzs.bzfree = NULL;

            if(BZ2_bzCompressInit(&bzs, clevel_, 0, 30) != BZ_OK) {
                throw(std::runtime_error("Initializing bzip compression failed"));
            }

            // set the stream in-pointer to the input data and the input size
            // to the size of the input in bytes
            bzs.next_in = (char*) dataIn;
            bzs.next_out = &dataOut[0];
            bzs.avail_in = sizeIn * sizeof(T);
            bzs.avail_out = outSize;

            int ret = BZ2_bzCompress(&bzs, BZ_FINISH);
            BZ2_bzCompressEnd(&bzs);

    		if (ret != BZ_STREAM_END) {          // an error occurred that was not EOF
                std::string err = "Exception during bzip compression: (" + std::to_string(ret)  + ")";
    		    throw std::runtime_error(err);
    		}

            // resize the output data
            outSize -= bzs.avail_out;;
            dataOut.resize(outSize);
        }


        void decompress(const std::vector<char> & dataIn, T * dataOut, std::size_t sizeOut) const {

            // create the bzip2 stream
            bz_stream bzs;
            bzs.opaque = NULL;
            bzs.bzalloc = NULL;
            bzs.bzfree = NULL;

            // init the bzip2 stream
            // last two arguments:
            // small: if larger than 0, uses alternative algorithm with less performance
            // but smaller memory footprint (we use 0 here)
            // verbosity: 0 -> not verbose
            if(BZ2_bzDecompressInit(&bzs, 0, 0) != BZ_OK) {
                throw(std::runtime_error("Initializing bzip decompression failed"));
            }

            // set the stream input to the beginning of the input data
            bzs.next_in = (char *) &dataIn[0];
            bzs.next_out = reinterpret_cast<char *>(dataOut);

            bzs.avail_in = dataIn.size();
            bzs.avail_out = sizeOut * sizeof(T);

            int ret = BZ2_bzDecompress(&bzs);
            BZ2_bzDecompressEnd(&bzs);

            if(ret != BZ_STREAM_END) {
                std::string err = "Exception during bzip decompression: (" + std::to_string(ret)  + ")";
    		    throw std::runtime_error(err);
            }

		}

        inline types::Compressor type() const {
            return types::bzip2;
        }

        inline void getOptions(types::CompressionOptions & opts) const {
            opts["level"] = clevel_;
        }


    private:
        void init(const DatasetMetadata & metadata) {
            clevel_ = std::get<int>(metadata.compressionOptions.at("level"));
        }

        // compression level
        int clevel_;
    };

} // namespace compression
} // namespace z5

#endif
