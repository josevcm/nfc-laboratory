#pragma once

#include <fstream>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Object.h>
#include <aws/s3/model/GetObjectRequest.h>

#include "z5/dataset.hxx"
#include "z5/s3/handle.hxx"


namespace z5 {
namespace s3 {

    // TODO implement for s3

    template<typename T>
    class Dataset : public z5::Dataset, private z5::MixinTyped<T> {

    public:
        typedef T value_type;
        typedef types::ShapeType shape_type;
        typedef z5::MixinTyped<T> Mixin;

        // create a new array with metadata
        Dataset(const handle::Dataset & handle,
                const DatasetMetadata & metadata) : z5::Dataset(metadata),
                                                    Mixin(metadata),
                                                    handle_(handle){
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
        }


        // read a chunk
        // IMPORTANT we assume that the data pointer is already initialized up to chunkSize_
        inline bool readChunk(const types::ShapeType & chunkIndices, void * dataOut) const {
            // get the chunk handle
            handle::Chunk chunk(handle_, chunkIndices, defaultChunkShape(), shape());

            // TODO implement
            // make sure that we have a valid chunk
            // checkChunk(chunk);

            // throw runtime error if trying to read non-existing chunk
            if(!chunk.exists()) {
                throw std::runtime_error("Trying to read a chunk that does not exist");
            }

            // load the data from disc
            std::vector<char> buffer;
            read(chunk, buffer);

            // format the data
            const bool is_varlen = util::buffer_to_data<T>(chunk, buffer, dataOut, Mixin::compressor_);

            return is_varlen;
        }


        inline void readRawChunk(const types::ShapeType & chunkIndices,
                                 std::vector<char> & buffer) const {
            // TODO implement
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
            const auto & cshape = chunk.shape();
            chunkShape.resize(cshape.size());
            std::copy(cshape.begin(), cshape.end(), chunkShape.begin());
        }

        inline std::size_t getChunkShape(const types::ShapeType & chunkId,
                                         const unsigned dim,
                                         const bool fromHeader=false) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            return chunk.shape()[dim];
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

        inline void getFillValue(void * fillValue) const {
            *((T*) fillValue) = Mixin::fillValue_;
        }

        inline void decompress(const std::vector<char> & buffer,
                               void * dataOut,
                               const std::size_t data_size) const {
            util::decompress<T>(buffer, dataOut, data_size, Mixin::compressor_);
        }

        inline bool checkVarlenChunk(const types::ShapeType & chunkId, std::size_t & chunkSize) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            if(isZarr_ || !chunk.exists()) {
                chunkSize = chunk.size();
                return false;
            }

            const bool is_varlen = read_n5_header(chunk, chunkSize);
            if(!is_varlen) {
                chunkSize = chunk.size();
            }
            return is_varlen;
        }

        inline const FileMode & mode() const {
            return handle_.mode();
        }

        inline void removeChunk(const types::ShapeType & chunkId) const {
            handle::Chunk chunk(handle_, chunkId, defaultChunkShape(), shape());
            chunk.remove();
        }

        inline void remove() const {
            handle_.remove();
        }

        // dummy impls
        inline const fs::path & path() const {}
        inline void chunkPath(const types::ShapeType & chunkId, fs::path & path) const {}

        // delete copy constructor and assignment operator
        // because the compressor cannot be copied by default
        // and we don't really need this to be copyable afaik
        // if this changes at some point, we need to provide a proper
        // implementation here
        Dataset(const Dataset & that) = delete;
        Dataset & operator=(const Dataset & that) = delete;

    private:

        inline void read(const handle::Chunk & chunk, std::vector<char> & buffer) const {
            // get options from somewhere?
            Aws::SDKOptions options;
            Aws::InitAPI(options);

            Aws::S3::S3Client client;
            Aws::S3::Model::GetObjectRequest request;

            const auto & bucketName = chunk.bucketName();
            const auto & objectName = chunk.nameInBucket();
            request.SetBucket(Aws::String(bucketName.c_str(), bucketName.size()));
            request.SetKey(Aws::String(objectName.c_str(), objectName.size()));

            auto outcome = client.GetObject(request);
            if(outcome.IsSuccess()) {
                auto & retrieved = outcome.GetResultWithOwnership().GetBody();

                // NOTE this is really abusing stream / stringstream and copies the data
                // But I haven't found a simple way to do this otherwise with the AWS API yet.

                // std::cout << "Doing horrible things!" << std::endl;

                /*
                std::stringstream stream;
                stream << retrieved.rdbuf();
                const std::string content = stream.str();
                buffer.resize(content.size());
                std::copy(content.begin(), content.end(), buffer.begin());
                */

                std::stringstream stream;
                stream << retrieved.rdbuf();
                auto bos = std::istreambuf_iterator<char>(stream);
                auto eos = std::istreambuf_iterator<char>();
                buffer.resize(std::distance(bos, eos));
                std::copy(bos, eos, buffer.begin());
            } else {
                throw std::runtime_error("Could not read chunk from S3.");
            }

            Aws::ShutdownAPI(options);
        }

        inline bool read_n5_header(const handle::Chunk & chunk, std::size_t & chunkSize) const {

            // TODO think about support for varlen in s3
            chunkSize = chunk.size();
            return false;

            /*
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
            */
        }

        handle::Dataset handle_;
    };


}
}
