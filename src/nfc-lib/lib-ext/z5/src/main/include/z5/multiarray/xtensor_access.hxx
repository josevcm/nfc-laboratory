#pragma once

#include <math.h>

#include "z5/dataset.hxx"
#include "z5/types/types.hxx"
#include "z5/multiarray/xtensor_util.hxx"
#include "z5/util/threadpool.hxx"

#include "xtensor.hpp"
#include "xtensor/views/xstrided_view.hpp"
#include "xtensor/containers/xadapt.hpp"


namespace z5 {
namespace multiarray {


    template<typename T, typename ARRAY>
    inline void readSubarraySingleThreaded(const Dataset & ds,
                                           xt::xexpression<ARRAY> & outExpression,
                                           const types::ShapeType & offset,
                                           const types::ShapeType & shape,
                                           const std::vector<types::ShapeType> & chunkRequests) {
        // need to cast to the actual xtensor implementation
        auto & out = outExpression.derived_cast();

        types::ShapeType offsetInRequest, requestShape, chunkShape;
        types::ShapeType offsetInChunk;

        const std::size_t maxChunkSize = ds.defaultChunkSize();
        const auto & maxChunkShape = ds.defaultChunkShape();

        std::size_t chunkSize, chunkStoreSize;
        std::vector<T> buffer(maxChunkSize);

        const auto & chunking = ds.chunking();
        const bool isZarr = ds.isZarr();

        // get the fillvalue
        T fillValue;
        ds.getFillValue(&fillValue);

        // iterate over the chunks
        for(const auto & chunkId : chunkRequests) {

            // std::cout << "Reading chunk " << chunkId << std::endl;
            bool completeOvlp = chunking.getCoordinatesInRoi(chunkId,
                                                             offset,
                                                             shape,
                                                             offsetInRequest,
                                                             requestShape,
                                                             offsetInChunk);

            // get the view in our array
            xt::xstrided_slice_vector offsetSlice;
            sliceFromRoi(offsetSlice, offsetInRequest, requestShape);
            auto view = xt::strided_view(out, offsetSlice);

            // check if this chunk exists, if not fill output with fill value
            if(!ds.chunkExists(chunkId)) {
                view = fillValue;;
                continue;
            }

            // get the shape and size of the chunk (in the actual grid)
            ds.getChunkShape(chunkId, chunkShape);
            chunkSize = std::accumulate(chunkShape.begin(), chunkShape.end(),
                                        1, std::multiplies<std::size_t>());

            // read the data from storage
            std::vector<char> dataBuffer;
            ds.readRawChunk(chunkId, dataBuffer);

            // get the shape of the chunk (as stored it is stored)
            chunkStoreSize = maxChunkSize;
            if(!isZarr) {
                if(util::read_n5_header(dataBuffer, chunkStoreSize)) {
                    throw std::runtime_error("Can't read from varlen chunks to multiarray");
                }
            }

            // if this is an edge chunk and the size of the chunk stored is different from
            // the size of the chunk in the grid (this is the case when written by zarr)
            // we need to set complete ovlp to false and use the chunkStoreSize
            if(chunkStoreSize != chunkSize) {
                completeOvlp = false;
                // reset chunk shape and chunk size to the full chunk size/shape
                chunkSize = maxChunkSize;
                chunkShape = maxChunkShape;
            }

            // resize the buffer if necessary
            if(chunkSize != buffer.size()) {
                buffer.resize(chunkSize);
            }

            // decompress the data
            ds.decompress(dataBuffer, &buffer[0], chunkSize);

            // reverse the endianness for N5 data (unless datatype is byte)
            if(!isZarr && sizeof(T) > 1) {
                util::reverseEndiannessInplace<T>(&buffer[0], &buffer[0] + chunkSize);
            }

            // request and chunk overlap completely
            // -> we can read all the data from the chunk
            if(completeOvlp) {
                copyBufferToView(buffer, view, out.strides());
            }
            // request and chunk overlap only partially
            // -> we can read the chunk data only partially
            else {
                // get a view to the part of the buffer we are interested in
                auto fullBuffView = xt::adapt(buffer, chunkShape);
                xt::xstrided_slice_vector bufSlice;

                sliceFromRoi(bufSlice, offsetInChunk, requestShape);
                auto bufView = xt::strided_view(fullBuffView, bufSlice);

                // could also implement fast copy for this
                // but this would be harder and might be premature optimization
                view = bufView;
            }
        }
    }


    template<typename T, typename ARRAY>
    inline void readSubarrayMultiThreaded(const Dataset & ds,
                                          xt::xexpression<ARRAY> & outExpression,
                                          const types::ShapeType & offset,
                                          const types::ShapeType & shape,
                                          const std::vector<types::ShapeType> & chunkRequests,
                                          const int numberOfThreads) {
        // need to cast to the actual xtensor implementation
        auto & out = outExpression.derived_cast();

        // construct threadpool and make a buffer for each thread
        util::ThreadPool tp(numberOfThreads);
        const int nThreads = tp.nThreads();

        const std::size_t maxChunkSize = ds.defaultChunkSize();
        const auto & maxChunkShape = ds.defaultChunkShape();

        typedef std::vector<T> Buffer;
        // TODO the thread buffers should be allocated by the thread that uses them
        // for optimal performance
        std::vector<Buffer> threadBuffers(nThreads, Buffer(maxChunkSize));

        const auto & chunking = ds.chunking();
        const bool isZarr = ds.isZarr();

        // get the fillvalue
        T fillValue;
        ds.getFillValue(&fillValue);

        // read the chunks in parallel
        const std::size_t nChunks = chunkRequests.size();
        util::parallel_foreach(tp, nChunks, [&](const int tId, const std::size_t chunkIndex){

            const auto & chunkId = chunkRequests[chunkIndex];
            auto & buffer = threadBuffers[tId];

            types::ShapeType offsetInRequest, requestShape, chunkShape;
            types::ShapeType offsetInChunk;

            //std::cout << "Reading chunk " << chunkId << std::endl;
            bool completeOvlp = chunking.getCoordinatesInRoi(chunkId,
                                                             offset,
                                                             shape,
                                                             offsetInRequest,
                                                             requestShape,
                                                             offsetInChunk);

            // get the view in our array
            xt::xstrided_slice_vector offsetSlice;
            sliceFromRoi(offsetSlice, offsetInRequest, requestShape);
            auto view = xt::strided_view(out, offsetSlice);

            // check if this chunk exists, if not fill output with fill value
            if(!ds.chunkExists(chunkId)) {
                view = fillValue;;
                return;
            }

            // get the current chunk-shape
            ds.getChunkShape(chunkId, chunkShape);
            std::size_t chunkSize = std::accumulate(chunkShape.begin(), chunkShape.end(),
                                                    1, std::multiplies<std::size_t>());

            // read the data from storage
            std::vector<char> dataBuffer;
            ds.readRawChunk(chunkId, dataBuffer);

            // get the shape of the chunk (as stored it is stored)
            std::size_t chunkStoreSize = maxChunkSize;
            if(!isZarr) {
                if(util::read_n5_header(dataBuffer, chunkStoreSize)) {
                    throw std::runtime_error("Can't read from varlen chunks to multiarray");
                }
            }

            // if this is an edge chunk and the size of the chunk stored is different from
            // the size of the chunk in the grid (this is the case when written by zarr)
            // we need to set complete ovlp to false and use the chunkStoreSize
            if(chunkStoreSize != chunkSize) {
                completeOvlp = false;
                // reset chunk shape and chunk size to the full chunk size/shape
                chunkSize = maxChunkSize;
                chunkShape = maxChunkShape;
            }

            // resize the buffer if necessary
            if(chunkSize != buffer.size()) {
                buffer.resize(chunkSize);
            }

            // decompress the data
            ds.decompress(dataBuffer, &buffer[0], chunkSize);

            // reverse the endianness for N5 data (unless datatype is byte)
            if(!isZarr && sizeof(T) > 1) {
                util::reverseEndiannessInplace<T>(&buffer[0], &buffer[0] + chunkSize);
            }

            // request and chunk overlap completely
            // -> we can read all the data from the chunk
            if(completeOvlp) {
                // fast copy implementation
                copyBufferToView(buffer, view, out.strides());
            }
            // request and chunk overlap only partially
            // -> we can read the chunk data only partially
            else {
                // get a view to the part of the buffer we are interested in
                auto fullBuffView = xt::adapt(buffer, chunkShape);
                xt::xstrided_slice_vector bufSlice;
                sliceFromRoi(bufSlice, offsetInChunk, requestShape);
                auto bufView = xt::strided_view(fullBuffView, bufSlice);

                // could also implement smart view for this,
                // but this would be kind of hard and premature optimization
                view = bufView;
            }
        });
    }


    template<typename T, typename ARRAY, typename ITER>
    inline void readSubarray(const Dataset & ds,
                             xt::xexpression<ARRAY> & outExpression,
                             ITER roiBeginIter,
                             const int numberOfThreads=1) {

        // need to cast to the actual xtensor implementation
        auto & out = outExpression.derived_cast();

        // get the offset and shape of the request and check if it is valid
        types::ShapeType offset(roiBeginIter, roiBeginIter+out.dimension());
        types::ShapeType shape(out.shape().begin(), out.shape().end());
        ds.checkRequestShape(offset, shape);
        ds.checkRequestType(typeid(T));

        // get the chunks that are involved in this request
        std::vector<types::ShapeType> chunkRequests;
        const auto & chunking = ds.chunking();
        chunking.getBlocksOverlappingRoi(offset, shape, chunkRequests);

        // read single or multi-threaded
        if(numberOfThreads == 1) {
            readSubarraySingleThreaded<T>(ds, out, offset, shape, chunkRequests);
        } else {
            readSubarrayMultiThreaded<T>(ds, out, offset, shape, chunkRequests, numberOfThreads);
        }
    }


    template<typename T, typename ARRAY>
    inline void writeSubarraySingleThreaded(const Dataset & ds,
                                            const xt::xexpression<ARRAY> & inExpression,
                                            const types::ShapeType & offset,
                                            const types::ShapeType & shape,
                                            const std::vector<types::ShapeType> & chunkRequests) {

        const auto & in = inExpression.derived_cast();
        types::ShapeType offsetInRequest, requestShape, chunkShape;
        types::ShapeType offsetInChunk;

        // get the fillvalue
        T fillValue;
        ds.getFillValue(&fillValue);

        const std::size_t maxChunkSize = ds.defaultChunkSize();
        std::size_t chunkSize = maxChunkSize;
        std::vector<T> buffer(chunkSize, fillValue);

        const auto & chunking = ds.chunking();

        // if we have a zarr dataset, we always write the full chunk
        const bool isZarr = ds.isZarr();

        // iterate over the chunks
        for(const auto & chunkId : chunkRequests) {

            bool completeOvlp = chunking.getCoordinatesInRoi(chunkId, offset,
                                                             shape, offsetInRequest,
                                                             requestShape, offsetInChunk);
            // get shape and size of this chunk
            ds.getChunkShape(chunkId, chunkShape);
            chunkSize = std::accumulate(chunkShape.begin(), chunkShape.end(),
                                        1, std::multiplies<std::size_t>());

            // get the view into the in-array
            xt::xstrided_slice_vector offsetSlice;
            sliceFromRoi(offsetSlice, offsetInRequest, requestShape);
            const auto view = xt::strided_view(in, offsetSlice);

            // if this is an edge chunk and we are writing zarr format,
            // we need to set complete ovlp to false and clear the buffer
            if(chunkSize != maxChunkSize && isZarr) {
                completeOvlp = false;
                // reset chunk shape and chunk size
                chunkShape = ds.defaultChunkShape();
                chunkSize = maxChunkSize;
                // clear the buffer
                std::fill(buffer.begin(), buffer.end(), fillValue);
            }

            // resize the buffer if necessary
            if(chunkSize != buffer.size()) {
                buffer.resize(chunkSize);
            }

            // request and chunk overlap completely
            // -> we can write the whole chunk
            if(completeOvlp) {
                copyViewToBuffer(view, buffer, in.strides());
                ds.writeChunk(chunkId, &buffer[0]);
            }

            // request and chunk overlap only partially
            // -> we can only write partial data and need
            // to preserve the data that will not be written
            else {

                // check if this chunk exists, and if it does, read the chunk's data
                // to preserve the part that is not written to
                if(ds.chunkExists(chunkId)) {
                    // load the current data into the buffer
                    if(ds.readChunk(chunkId, &buffer[0])) {
                        throw std::runtime_error("Can't write to varlen chunks from multiarray");
                    }
                } else {
                    std::fill(buffer.begin(), buffer.end(), fillValue);
                }

                // overwrite the data that is covered by the request
                auto fullBuffView = xt::adapt(buffer, chunkShape);
                xt::xstrided_slice_vector bufSlice;
                sliceFromRoi(bufSlice, offsetInChunk, requestShape);
                auto bufView = xt::strided_view(fullBuffView, bufSlice);

                // could also implement smart view for this,
                // but this would be kind of hard and premature optimization
                bufView = view;

                // write the chunk
                ds.writeChunk(chunkId, &buffer[0]);
            }
        }
    }


    template<typename T, typename ARRAY>
    inline void writeSubarrayMultiThreaded(const Dataset & ds,
                                           const xt::xexpression<ARRAY> & inExpression,
                                           const types::ShapeType & offset,
                                           const types::ShapeType & shape,
                                           const std::vector<types::ShapeType> & chunkRequests,
                                           const int numberOfThreads) {

        const auto & in = inExpression.derived_cast();

        // get the fillvalue
        T fillValue;
        ds.getFillValue(&fillValue);

        // construct threadpool and make a buffer for each thread
        util::ThreadPool tp(numberOfThreads);
        const int nThreads = tp.nThreads();

        const std::size_t maxChunkSize = ds.defaultChunkSize();
        typedef std::vector<T> Buffer;
        std::vector<Buffer> threadBuffers(nThreads, Buffer(maxChunkSize, fillValue));

        const auto & chunking = ds.chunking();
        const bool isZarr = ds.isZarr();

        // write the chunks in parallel
        const std::size_t nChunks = chunkRequests.size();
        util::parallel_foreach(tp, nChunks, [&](const int tId, const std::size_t chunkIndex){

            const auto & chunkId = chunkRequests[chunkIndex];
            auto & buffer = threadBuffers[tId];

            types::ShapeType offsetInRequest, requestShape, chunkShape;
            types::ShapeType offsetInChunk;

            bool completeOvlp = chunking.getCoordinatesInRoi(chunkId, offset,
                                                             shape, offsetInRequest,
                                                             requestShape, offsetInChunk);
            ds.getChunkShape(chunkId, chunkShape);
            std::size_t chunkSize = std::accumulate(chunkShape.begin(), chunkShape.end(),
                                                    1, std::multiplies<std::size_t>());

            // get the view into the in-array
            xt::xstrided_slice_vector offsetSlice;
            sliceFromRoi(offsetSlice, offsetInRequest, requestShape);
            const auto view = xt::strided_view(in, offsetSlice);

            // if this is an edge chunk and we are writing zarr format,
            // we need to set complete ovlp to false and clear the buffer
            if(chunkSize != maxChunkSize && isZarr) {
                completeOvlp = false;
                // reset chunk shape and chunk size
                chunkSize = maxChunkSize;
                chunkShape = ds.defaultChunkShape();
                // clear the buffer
                std::fill(buffer.begin(), buffer.end(), fillValue);
            }

            // resize buffer if necessary
            if(chunkSize != buffer.size()) {
                buffer.resize(chunkSize);
            }

            // request and chunk overlap completely
            // -> we can write the whole chunk
            if(completeOvlp) {
                copyViewToBuffer(view, buffer, in.strides());
                ds.writeChunk(chunkId, &buffer[0]);
            }

            // request and chunk overlap only partially
            // -> we can only write partial data and need
            // to preserve the data that will not be written
            else {
                if(ds.chunkExists(chunkId)) {

                    // load the current data into the buffer
                    if(ds.readChunk(chunkId, &buffer[0])) {
                        throw std::runtime_error("Can't write to varlen chunks from multiarray");
                    }
                } else {
                    std::fill(buffer.begin(), buffer.end(), fillValue);
                }

                // overwrite the data that is covered by the request
                auto fullBuffView = xt::adapt(buffer, chunkShape);
                xt::xstrided_slice_vector bufSlice;
                sliceFromRoi(bufSlice, offsetInChunk, requestShape);
                auto bufView = xt::strided_view(fullBuffView, bufSlice);

                // could also implement smart view for this,
                // but this would be kind of hard and premature optimization
                bufView = view;

                // write the chunk
                ds.writeChunk(chunkId, &buffer[0]);
            }
        });
    }


    template<typename T, typename ARRAY, typename ITER>
    inline void writeSubarray(const Dataset & ds,
                              const xt::xexpression<ARRAY> & inExpression,
                              ITER roiBeginIter,
                              const int numberOfThreads=1) {

        const auto & in = inExpression.derived_cast();

        // get the offset and shape of the request and check if it is valid
        types::ShapeType offset(roiBeginIter, roiBeginIter+in.dimension());
        types::ShapeType shape(in.shape().begin(), in.shape().end());

        ds.checkRequestShape(offset, shape);
        ds.checkRequestType(typeid(T));

        // get the chunks that are involved in this request
        std::vector<types::ShapeType> chunkRequests;
        const auto & chunking = ds.chunking();
        chunking.getBlocksOverlappingRoi(offset, shape, chunkRequests);

        // write data multi or single threaded
        if(numberOfThreads == 1) {
            writeSubarraySingleThreaded<T>(ds, in, offset, shape, chunkRequests);
        } else {
            writeSubarrayMultiThreaded<T>(ds, in, offset, shape, chunkRequests, numberOfThreads);
        }

    }


    // unique ptr API
    template<typename T, typename ARRAY, typename ITER>
    inline void readSubarray(std::unique_ptr<Dataset> & ds,
                             xt::xexpression<ARRAY> & out,
                             ITER roiBeginIter,
                             const int numberOfThreads=1) {
       readSubarray<T>(*ds, out, roiBeginIter, numberOfThreads);
    }


    template<typename T, typename ARRAY, typename ITER>
    inline void writeSubarray(std::unique_ptr<Dataset> & ds,
                              const xt::xexpression<ARRAY> & in,
                              ITER roiBeginIter,
                              const int numberOfThreads=1) {
        writeSubarray<T>(*ds, in, roiBeginIter, numberOfThreads);
    }

}
}
