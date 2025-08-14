#pragma once

#include <ios>

#include "z5/dataset.hxx"
#include "z5/filesystem/handle.hxx"


namespace z5 {
namespace filesystem {


    template<typename T>
    class Dataset : public z5::Dataset, private z5::MixinTyped<T> {

    public:

        typedef T value_type;
        typedef types::ShapeType shape_type;
        typedef z5::MixinTyped<T> Mixin;
        typedef z5::Dataset BaseType;

        // create a new array with metadata
        Dataset(const handle::Dataset & handle,
                const DatasetMetadata & metadata) : BaseType(metadata),
                                                    Mixin(metadata),
                                                    handle_(handle){
            // disable sync of c++ and c streams for potentially faster I/O
            std::ios_base::sync_with_stdio(false);
        }

        //
        // Implement Dataset API
        //

        inline void writeChunk(const types::ShapeType & chunkIndices, const void * dataIn,
                               const bool isVarlen=false, const std::size_t varSize=0) const {

            // check if we are allowed to write
            if(!handle_.mode().canWrite()) {
                const std::string err = "Cannot write data in file mode " + handle_.mode().printMode();
                throw std::invalid_argument(err.c_str());
            }

            // create chunk handle and check if this chunk is valid
            handle::Chunk chunk(handle_, chunkIndices, defaultChunkShape(), shape());
            checkChunk(chunk, isVarlen);
            const auto & path = chunk.path();

            // create the output buffer and format the data
            std::vector<char> buffer;
            // data_to_buffer will return false if there's nothing to write
            if(!util::data_to_buffer(chunk, dataIn, buffer, Mixin::compressor_, Mixin::fillValue_, isVarlen, varSize)) {
                // if we have data on disc for the chunk, delete it
                if(fs::exists(path)) {
                    fs::remove(path);
                }
                return;
            }

            // write the chunk to disc
            if(!isZarr_ || zarrDelimiter_ == "/") {
                // need to make sure we have the root directory if this is an nested chunk
                chunk.create();
            }
            write(path, buffer);
        }


        // read a chunk
        // IMPORTANT we assume that the data pointer is already initialized up to chunkSize_
        inline bool readChunk(const types::ShapeType & chunkIndices, void * dataOut) const {
            // get the chunk handle
            handle::Chunk chunk(handle_, chunkIndices, defaultChunkShape(), shape());

            // make sure that we have a valid chunk
            checkChunk(chunk);

            // throw runtime error if trying to read non-existing chunk
            if(!chunk.exists()) {
                throw std::runtime_error("Trying to read a chunk that does not exist");
            }

            // load the data from disc
            std::vector<char> buffer;
            read(chunk.path(), buffer);

            // format the data
            const bool is_varlen = util::buffer_to_data<T>(chunk, buffer, dataOut, Mixin::compressor_);

            return is_varlen;
        }

        inline void readRawChunk(const types::ShapeType & chunkIndices,
                                 std::vector<char> & buffer) const {
            handle::Chunk chunk(handle_, chunkIndices, defaultChunkShape(), shape());
            read(chunk.path(), buffer);
        }

        inline void checkRequestType(const std::type_info & type) const {
            if(type != typeid(T)) {
                // TODO all in error message
                std::cout << "Mytype: " << typeid(T).name() << " your type: " << type.name() << std::endl;
                throw std::runtime_error("Request has wrong type");
            }
        }

        inline bool chunkExists(const types::ShapeType & chunkId) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            return chunk.exists();
        }


        inline std::size_t getChunkSize(const types::ShapeType & chunkId) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            return chunk.size();
        }


        inline void getChunkShape(const types::ShapeType & chunkId,
                                  types::ShapeType & chunkShape,
                                  const bool fromHeader=false) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            if(!isZarr_ && fromHeader) {
                read_shape_from_n5_header(chunk.path(), chunkShape);
            } else {
                const auto & cshape = chunk.shape();
                chunkShape.resize(cshape.size());
                std::copy(cshape.begin(), cshape.end(), chunkShape.begin());
            }
        }


        inline std::size_t getChunkShape(const types::ShapeType & chunkId,
                                         const unsigned dim,
                                         const bool fromHeader=false) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            if(!isZarr_ && fromHeader) {
                types::ShapeType chunkShape;
                read_shape_from_n5_header(chunk.path(), chunkShape);
                return chunkShape[dim];
            } else {
                return chunk.shape()[dim];
            }
        }


        // compression options
        inline types::Compressor getCompressor() const {return Mixin::compressor_->type();}
        inline void getCompressor(std::string & compressor) const {
            auto compressorType = getCompressor();
            compressor = isZarr_ ? types::Compressors::compressorToZarr()[compressorType] : types::Compressors::compressorToN5()[compressorType];
        }
        inline void getCompressionOptions(types::CompressionOptions & opts) const {
            Mixin::compressor_->getOptions(opts);
        }

        inline void decompress(const std::vector<char> & buffer,
                               void * dataOut,
                               const std::size_t data_size) const {
            util::decompress<T>(buffer, dataOut, data_size, Mixin::compressor_);
        }

        inline void getFillValue(void * fillValue) const {
            *((T*) fillValue) = Mixin::fillValue_;
        }


        inline bool checkVarlenChunk(const types::ShapeType & chunkId, std::size_t & chunkSize) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            if(isZarr_ || !chunk.exists()) {
                chunkSize = chunk.size();
                return false;
            }

            const bool is_varlen = read_varlen_from_n5_header(chunk.path(), chunkSize);
            if(!is_varlen) {
                chunkSize = chunk.size();
            }
            return is_varlen;
        }

        inline const FileMode & mode() const {
            return handle_.mode();
        }
        inline const fs::path & path() const {
            return handle_.path();
        }
        inline void chunkPath(const types::ShapeType & chunkId, fs::path & path) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            path = chunk.path();
        }
        inline void removeChunk(const types::ShapeType & chunkId) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            chunk.remove();
        }
        inline void remove() const {
            handle_.remove();
        }

        // delete copy constructor and assignment operator
        // because the compressor cannot be copied by default
        // and we don't really need this to be copyable afaik
        // if this changes at some point, we need to provide a proper
        // implementation here
        Dataset(const Dataset & that) = delete;
        Dataset & operator=(const Dataset & that) = delete;

    private:

        inline void write(const fs::path & path, const std::vector<char> & buffer) const {
            std::ofstream file(path, std::ios::binary);
            file.write(&buffer[0], buffer.size());
            file.close();
        }

        inline void read(const fs::path & path, std::vector<char> & buffer) const {
            // open input stream and read the filesize
            std::ifstream file(path, std::ios::binary);

            file.seekg(0, std::ios::end);
            const std::size_t file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            // resize the data vector
            buffer.resize(file_size);

            // read the file
            file.read(&buffer[0], file_size);
            file.close();
        }


        // check that the chunk handle is valid
        inline void checkChunk(const handle::Chunk & chunk,
                               const bool isVarlen=false) const {
            // check dimension
            const auto & chunkIndices = chunk.chunkIndices();
            if(!chunking_.checkBlockCoordinate(chunkIndices)) {
                throw std::runtime_error("Invalid chunk");
            }
            // varlen chunks are only supported in n5
            if(isVarlen && isZarr_) {
                throw std::runtime_error("Varlength chunks are not supported in zarr");
            }
        }


        inline bool read_varlen_from_n5_header(const fs::path & path,
                                               std::size_t & chunkSize) const {
            std::ifstream file(path, std::ios::binary);

            // read the mode
            uint16_t mode;
            file.read((char *) &mode, 2);
            util::reverseEndiannessInplace(mode);

            if(mode == 0) {
                return false;
            }

            // read the number of dimensions
            uint16_t ndim;
            file.read((char *) &ndim, 2);
            util::reverseEndiannessInplace(ndim);

            // advance the file by ndim * 4 to skip the shape
            file.seekg((ndim + 1) * 4);

            uint32_t varlength;
            file.read((char*) &varlength, 4);
            util::reverseEndiannessInplace(varlength);
            chunkSize = varlength;

            file.close();
            return true;
        }


        inline void read_shape_from_n5_header(const fs::path & path,
                                              types::ShapeType & chunkShape) const {
            std::ifstream file(path, std::ios::binary);

            // advance the file by 2 to skip the mode
            file.seekg(2);

            // read the number of dimensions
            uint16_t ndim;
            file.read((char *) &ndim, 2);
            util::reverseEndiannessInplace(ndim);

            // read temporary shape with uint32 entries
            std::vector<uint32_t> shapeTmp(ndim);
            for(int d = 0; d < ndim; ++d) {
                file.read((char *) &shapeTmp[d], 4);
            }
            util::reverseEndiannessInplace<uint32_t>(shapeTmp.begin(), shapeTmp.end());

            // // N5-Axis order: we need to reverse the chunk shape read from the header
            std::reverse(shapeTmp.begin(), shapeTmp.end());

            chunkShape.resize(ndim);
            std::copy(shapeTmp.begin(), shapeTmp.end(), chunkShape.begin());

            file.close();
        }

    private:
        handle::Dataset handle_;
    };


}
}
