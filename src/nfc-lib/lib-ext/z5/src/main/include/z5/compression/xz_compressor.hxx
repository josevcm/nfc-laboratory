#pragma once

#ifdef WITH_XZ

#include <lzma.h>

#include "z5/compression/compressor_base.hxx"
#include "z5/metadata.hxx"

// zlib manual:
// https://zlib.net/manual.html

// calls to ZLIB interface following
// https://blog.cppse.nl/deflate-and-gzip-compress-and-decompress-functions


namespace z5 {
namespace compression {

    template<typename T>
    class XzCompressor : public CompressorBase<T> {

    public:
        XzCompressor(const DatasetMetadata & metadata) {
            init(metadata);
        }

        void compress(const T * dataIn, std::vector<char> & dataOut, std::size_t sizeIn) const {

            // create lzma stream
            lzma_stream lzs;
            memset(&lzs, 0, sizeof(lzs));

            // initialize lzma stream
            // TODO is second argument (= preset) the level ?!
            if(lzma_easy_encoder(&lzs, level_, LZMA_CHECK_CRC64) != LZMA_OK) {
                throw(std::runtime_error("Initializing xz stream failed"));
            }

            // temporary outbuffer
            // TODO buffersize ?!
            const std::size_t bufferSize = 262144;
            std::vector<uint8_t> outbuffer(bufferSize);

            lzs.next_in = (uint8_t *) dataIn;
            lzs.avail_in = sizeIn * sizeof(T);

            // settings for compression
            auto action = LZMA_RUN;
            lzma_ret ret = LZMA_OK;
            std::size_t prevOutBytes = 0;
            std::size_t bytesCompressed;

            do {

                lzs.next_out = &outbuffer[0];
                lzs.avail_out = outbuffer.size();

                ret = lzma_code(&lzs, action);

                bytesCompressed = lzs.total_out - prevOutBytes;
                prevOutBytes = lzs.total_out;

                dataOut.insert(dataOut.end(),
                               outbuffer.begin(),
                               outbuffer.begin() + bytesCompressed);

                if(lzs.avail_in == 0) {
                    action = LZMA_FINISH;
                }

            } while(ret == LZMA_OK);

            lzma_end(&lzs);

            if(ret != LZMA_STREAM_END) {
                std::string err = "Exception during xz compression: (" + std::to_string(ret)  + ")";
    		    throw std::runtime_error(err);
            }
        }


        void decompress(const std::vector<char> & dataIn, T * dataOut, std::size_t sizeOut) const {

            // create lzma stream
            lzma_stream lzs;
            memset(&lzs, 0, sizeof(lzs));

            if(lzma_stream_decoder(&lzs, UINT64_MAX, LZMA_CONCATENATED) != LZMA_OK) {
                throw(std::runtime_error("Initializing xz stream failed"));
            }

            lzs.next_in = (uint8_t *) &dataIn[0];
            lzs.avail_in = dataIn.size();

            // let xz decompress the bytes blockwise
            lzma_ret ret = LZMA_OK;
            auto action = LZMA_RUN;
            std::size_t currentPosition = 0;
            do {
                // set the stream output to the output dat at the current position
                // and set the available size to the remaining bytes in the output data
                lzs.next_out = reinterpret_cast<uint8_t*>(dataOut + currentPosition);
                lzs.avail_out = (sizeOut - currentPosition) * sizeof(T);

                ret = lzma_code(&lzs, action);

                // get the current position in the out data
                currentPosition = lzs.total_out / sizeof(T);

                if(lzs.avail_in == 0) {
                    action = LZMA_FINISH;
                }

            } while(ret == LZMA_OK);

            lzma_end(&lzs);

            if(ret != LZMA_STREAM_END) {
                std::string err = "Exception during xz decompression: (" + std::to_string(ret)  + ")";
    		    throw std::runtime_error(err);
            }
		}

        inline types::Compressor type() const {
            return types::xz;
        }

        inline void getOptions(types::CompressionOptions & opts) const {
            opts["level"] = level_;
        }

    private:
        void init(const DatasetMetadata & metadata) {
            level_ = std::get<int>(metadata.compressionOptions.at("level"));
        }

        // compression level
        int level_;

    };

}
}
#endif
