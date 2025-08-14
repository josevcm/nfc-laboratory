#pragma once

#include "z5/handle.hxx"

namespace z5 {
namespace util {


    template<class T, class COMPRESSOR>
    inline void compress(const T * dataIn, const std::size_t dataSize, std::vector<char> & buffer, const COMPRESSOR & compressor) {
        // if we have raw compression (== no compression = enum 0), we bypass the compression step
        // but need to copy the data
        if(compressor->type() == 0) {
            buffer.resize(dataSize * sizeof(T));
            memcpy(&buffer[0], dataIn, dataSize * sizeof(T));
        } else {
            compressor->compress(dataIn, buffer, dataSize);
        }
    }


    inline void write_n5_header(std::vector<char> & buffer, const types::ShapeType & shape,
                                const bool isVarlen, const uint32_t varlen) {

        const unsigned ndim = shape.size();
        buffer.resize((ndim + 1) * 4);

        // write the mode
        // 2 bytes | sum 2
        uint16_t mode = isVarlen;
        util::reverseEndiannessInplace(mode);
        memcpy(&buffer[0], &mode, 2);

        // write the number of dimensions
        // 2 bytes | sum 4
        uint16_t nDimsOut = shape.size();
        util::reverseEndiannessInplace(nDimsOut);
        memcpy(&buffer[2], &nDimsOut, 2);

        // write the shape
        // ndim * 4 bytes | (ndim + 1) * 4

        // inverse endianness and dimension
        std::vector<uint32_t> shapeOut(shape.begin(), shape.end());
        util::reverseEndiannessInplace<uint32_t>(shapeOut.begin(), shapeOut.end());

        // N5-Axis order: we need to reverse the chunk shape written to the header
        std::reverse(shapeOut.begin(), shapeOut.end());
        // write chunk shape to header
        for(int d = 0; d < ndim; ++d) {
            memcpy(&buffer[(1 + d) * 4], &shapeOut[d], 4);
        }

        // need to write the actual size if we allow for varlength mode
        if(isVarlen) {
            const std::size_t buf_size = buffer.size();
            buffer.resize(buf_size + 4);
            uint32_t varlenOut = varlen;
            util::reverseEndiannessInplace<uint32_t>(varlenOut);
            memcpy(&buffer[buf_size], &varlenOut, 4);
        }
    }


    template<class T, class COMPRESSOR>
    inline void data_to_n5_format(const void * dataIn, const std::size_t dataSize,
                                  const types::ShapeType & shape, std::vector<char> & buffer,
                                  const COMPRESSOR & compressor, const bool isVarlen) {

        // write the n5 header
        write_n5_header(buffer, shape, isVarlen, dataSize);
        std::vector<char> compressed;

        // reverse the endianness if necessary
        if(sizeof(T) > 1) {
            // copy the data and reverse endianness
            std::vector<T> dataTmp(static_cast<const T*>(dataIn),
                                   static_cast<const T*>(dataIn) + dataSize);
            util::reverseEndiannessInplace<T>(dataTmp.begin(), dataTmp.end());

            // compress the data
            compress(&dataTmp[0], dataSize, compressed, compressor);

        } else {
            compress((const T *) dataIn, dataSize, compressed, compressor);
        }

        // append compressed to the buffer
        buffer.reserve(buffer.size() + compressed.size());
        buffer.insert(buffer.end(), compressed.begin(), compressed.end());
    }


    template<class CHUNK, class T, class COMPRESSOR>
    inline bool data_to_buffer(const z5::handle::Chunk<CHUNK> & chunk,
                               const void * dataIn,
                               std::vector<char> & buffer,
                               const COMPRESSOR & compressor,
                               const T fillValue,
                               const bool isVarlen=false,
                               const std::size_t varSize=0) {
        const bool isZarr = chunk.isZarr();
        if(isVarlen && isZarr) {
            throw std::runtime_error("Varlen chunks are not supported in zarr.");
        }

        // get the chunk size and the data size (can be different iif varlen)
        const std::size_t chunkSize = isZarr ? chunk.defaultSize() : chunk.size();
        const std::size_t dataSize = isVarlen ? varSize : chunkSize;
        const auto & chunk_shape = isZarr ? chunk.defaultShape() : chunk.shape();

        // check if the chunk is empty (i.e. all fillvalue) and if so don't write it
        // this does not apply if we have a varlen dataset for which the fillvalue is not defined
        if(!isVarlen) {
            // we need the temporary reference to capture in the lambda
            const bool isEmpty = std::all_of(static_cast<const T*>(dataIn),
                                             static_cast<const T*>(dataIn) + chunkSize,
                                             [fillValue](const T val){return val == fillValue;});
            // return false if the chunk is empty
            if(isEmpty) {
                return false;
            }
        }
        else if(isVarlen && varSize == 0) {
            return false;
        }

        // for zarr format, we only need to compress the data.
        // for n5, we need to also reverse the endianness and add the header
        if(isZarr) {
            compress((const T *) dataIn, dataSize, buffer, compressor);
        } else {
            data_to_n5_format<T>(dataIn, dataSize,
                                 chunk_shape, buffer,
                                 compressor, isVarlen);
        }
        return true;
    }


    template<class T, class COMPRESSOR>
    inline void decompress(const std::vector<char> & buffer, void * dataOut,
                           const std::size_t data_size, const COMPRESSOR & compressor) {
        // we don't need to decompress for raw compression
        if(compressor->type() == 0) {
            // mem-copy the binary data that was read to typed out data
            memcpy((T*) dataOut, &buffer[0], buffer.size());
        } else {
            compressor->decompress(buffer, static_cast<T*>(dataOut), data_size);
        }
    }

    inline bool read_n5_header(std::vector<char> & buffer,
                               std::size_t & data_size) {
        // read the mode
        uint16_t mode;
        memcpy(&mode, &buffer[0], 2);
        util::reverseEndiannessInplace(mode);

        // read the number of dimensions
        uint16_t ndim;
        memcpy(&ndim, &buffer[2], 2);
        util::reverseEndiannessInplace(ndim);

        // read temporary shape with uint32 entries
        std::vector<uint32_t> shape(ndim);
        for(int d = 0; d < ndim; ++d) {
            memcpy(&shape[d], &buffer[(d + 1) * 4], 4);
        }
        util::reverseEndiannessInplace<uint32_t>(shape.begin(), shape.end());

        // // N5-Axis order: we need to reverse the chunk shape read from the header
        std::reverse(shape.begin(), shape.end());

        std::size_t headerlen = (ndim + 1) * 4;

        const bool is_varlen = mode == 1;
        // read the varlength if the chunk is in varlength mode
        if(is_varlen) {
            uint32_t varlength;
            memcpy(&varlength, &buffer[headerlen], 4);
            util::reverseEndiannessInplace(varlength);
            data_size = varlength;
            headerlen += 4;
        } else {
            data_size = std::accumulate(shape.begin(), shape.end(),
                                        1, std::multiplies<uint32_t>());
        }

        // cut header from the buffer
        buffer.erase(buffer.begin(), buffer.begin() + headerlen);

        return is_varlen;
    }


    template<class T, class CHUNK, class COMPRESSOR>
    inline bool buffer_to_data(const z5::handle::Chunk<CHUNK> & chunk,
                               std::vector<char> & buffer,
                               void * dataOut,
                               const COMPRESSOR & compressor) {

        const bool is_zarr = chunk.isZarr();
        std::size_t chunk_size = chunk.defaultSize();
        bool is_varlen = false;

        if(!is_zarr) {
            is_varlen = read_n5_header(buffer, chunk_size);
        }

        decompress<T>(buffer, dataOut, chunk_size, compressor);

        // reverse the endianness for N5 data (unless datatype is byte)
        if(!is_zarr && sizeof(T) > 1) {
            util::reverseEndiannessInplace<T>(static_cast<T*>(dataOut),
                                              static_cast<T*>(dataOut) + chunk_size);
        }
        return is_varlen;
    }


}
}
