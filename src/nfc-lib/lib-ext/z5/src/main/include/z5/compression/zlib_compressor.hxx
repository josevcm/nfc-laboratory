#pragma once

#ifdef WITH_ZLIB

#include <zlib.h>

#include "z5/compression/compressor_base.hxx"
#include "z5/metadata.hxx"

// zlib manual:
// https://zlib.net/manual.html

// calls to ZLIB interface following
// https://blog.cppse.nl/deflate-and-gzip-compress-and-decompress-functions

namespace z5 {
namespace compression {

    template<typename T>
    class ZlibCompressor : public CompressorBase<T> {

    public:
        ZlibCompressor(const DatasetMetadata & metadata) {
            init(metadata);
        }

        void compress(const T * dataIn, std::vector<char> & dataOut, std::size_t sizeIn) const {

            // open the zlib stream
            z_stream zs;
            memset(&zs, 0, sizeof(zs));

            // resize the out data to input size
            dataOut.clear();
            dataOut.reserve(sizeIn * sizeof(T));

            // intermediate output buffer
            // size set to 256 kb, which is recommended in the zlib usage example:
            // http://www.gzip.org/zlib/zlib_how.html/
            const std::size_t bufferSize = 262144;
            std::vector<Bytef> outbuffer(bufferSize);

            // init the zlib or gzip stream
            if(useZlibEncoding_) {
                if(deflateInit(&zs, clevel_) != Z_OK){
                    throw(std::runtime_error("Initializing zLib deflate failed"));
                }
            } else {
                if(deflateInit2(&zs, clevel_,
                                Z_DEFLATED, MAX_WBITS + 16,
                                MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
                    throw(std::runtime_error("Initializing zLib deflate failed"));
                }
                // gzip compression:
                // prepend magic bits to the filename
                // prepend date string to the data
            }

            // set the stream in-pointer to the input data and the input size
            // to the size of the input in bytes
            zs.next_in = (Bytef*) dataIn;
            zs.avail_in = sizeIn * sizeof(T);

            // let zlib compress the bytes blockwise
            int ret;
            std::size_t prevOutBytes = 0;
            std::size_t bytesCompressed;
            do {
                // set the stream out-pointer to the current position of the out-data
                // and set the available out size to the remaining size in the vector not written at

                zs.next_out = &outbuffer[0];
                zs.avail_out = outbuffer.size();

                ret = deflate(&zs, Z_FINISH);
                bytesCompressed = zs.total_out - prevOutBytes;
                prevOutBytes = zs.total_out;

                dataOut.insert(dataOut.end(),
                               outbuffer.begin(),
                               outbuffer.begin() + bytesCompressed);

            } while(ret == Z_OK);

            deflateEnd(&zs);

    		if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
                std::string err = "Exception during zlib compression: (" + std::to_string(ret)  + ")";
    		    throw std::runtime_error(err);
    		}

            // gzip: append checksum to the data
            if(!useZlibEncoding_) {

            }

        }


        void decompress(const std::vector<char> & dataIn, T * dataOut, std::size_t sizeOut) const {

            // open the zlib stream
            z_stream zs;
            memset(&zs, 0, sizeof(zs));

            // init the zlib stream with automatic header detection
            // for zlib and zip format (MAX_WBITS + 32)
            if(inflateInit2(&zs, MAX_WBITS + 32) != Z_OK){
                throw(std::runtime_error("Initializing zLib inflate failed"));
            }

            // set the stream input to the beginning of the input data
            zs.next_in = (Bytef*) &dataIn[0];
            zs.avail_in = dataIn.size();

            // let zlib decompress the bytes blockwise
            int ret;
            std::size_t currentPosition = 0;
            do {
                // set the stream output to the output dat at the current position
                // and set the available size to the remaining bytes in the output data
                // reinterpret_cast because (Bytef*) throws warning
                zs.next_out = reinterpret_cast<Bytef*>(dataOut + currentPosition);
                zs.avail_out = (sizeOut - currentPosition) * sizeof(T);

                ret = inflate(&zs, Z_FINISH);

                // get the current position in the out data
                currentPosition = zs.total_out / sizeof(T);

            } while(ret == Z_OK);

            inflateEnd(&zs);

			if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
                std::string err = "Exception during zlib decompression: (" + std::to_string(ret)  + ")";
    		    throw std::runtime_error(err);
    		}
		}

        virtual types::Compressor type() const {
            return types::zlib;
        }

        inline void getOptions(types::CompressionOptions & opts) const {
            opts["level"] = clevel_;
        }

    private:
        void init(const DatasetMetadata & metadata) {
            clevel_ = std::get<int>(metadata.compressionOptions.at("level"));
            useZlibEncoding_ = std::get<bool>(metadata.compressionOptions.at("useZlib"));
        }

        // compression level
        int clevel_;
        // use zlib or gzip encoding
        bool useZlibEncoding_;
    };

} // namespace compression
} // namespace z5

#endif
