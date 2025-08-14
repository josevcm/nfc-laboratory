#pragma once

#include "z5/dataset.hxx"
#include "z5/multiarray/xtensor_access.hxx"
#include "z5/util/threadpool.hxx"

#include "xtensor/core/xeval.hpp"


namespace z5 {
namespace multiarray {


    template<typename T>
    void writeScalarSingleThreaded(const Dataset & ds,
                                   const types::ShapeType & offset,
                                   const types::ShapeType & shape,
                                   const T val,
                                   const std::vector<types::ShapeType> & chunkRequests) {

        // get the fillvalue
        T fillValue;
        ds.getFillValue(&fillValue);

        types::ShapeType offsetInRequest, requestShape, chunkShape, offsetInChunk;
        // out buffer holding data for a single chunk
        std::size_t chunkSize = ds.defaultChunkSize();
        std::vector<T> buffer(chunkSize, val);

        const auto & chunking = ds.chunking();

        // iterate over the chunks and write the buffer
        for(const auto & chunkId : chunkRequests) {

            const bool completeOvlp = chunking.getCoordinatesInRoi(chunkId,
                                                                   offset,
                                                                   shape,
                                                                   offsetInRequest,
                                                                   requestShape,
                                                                   offsetInChunk);


            ds.getChunkShape(chunkId, chunkShape);
            chunkSize = std::accumulate(chunkShape.begin(), chunkShape.end(),
                                        1, std::multiplies<std::size_t>());

            // reshape buffer if necessary
            if(buffer.size() != chunkSize) {
                buffer.resize(chunkSize, val);
            }

            // request and chunk overlap completely
            // -> we can write the whole chunk
            if(completeOvlp) {
                ds.writeChunk(chunkId, &buffer[0]);
            }

            // request and chunk overlap only partially
            // -> we can only write partial data and need
            // to preserve the data that will not be written
            else {
                // load the current data into the buffer
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
                bufView = val;
                ds.writeChunk(chunkId, &buffer[0]);

                // need to reset the buffer to our fill value
                std::fill(buffer.begin(), buffer.end(), val);
            }
        }
    }


    template<typename T>
    void writeScalarMultiThreaded(const Dataset & ds,
                                  const types::ShapeType & offset,
                                  const types::ShapeType & shape,
                                  const T val,
                                  const std::vector<types::ShapeType> & chunkRequests,
                                  const int numberOfThreads) {

        // construct threadpool and make a buffer for each thread
        util::ThreadPool tp(numberOfThreads);
        const int nThreads = tp.nThreads();

        // get the fillvalue
        T fillValue;
        ds.getFillValue(&fillValue);

        // out buffer holding data for a single chunk
        std::size_t chunkSize = ds.defaultChunkSize();
        typedef std::vector<T> Buffer;
        std::vector<Buffer> threadBuffers(nThreads, Buffer(chunkSize, val));

        const auto & chunking = ds.chunking();

        // write scalar to the chunks in parallel
        const std::size_t nChunks = chunkRequests.size();
        util::parallel_foreach(tp, nChunks, [&](const int tId, const std::size_t chunkIndex){

            const auto & chunkId = chunkRequests[chunkIndex];
            auto & buffer = threadBuffers[tId];

            types::ShapeType offsetInRequest, requestShape, chunkShape, offsetInChunk;
            const bool completeOvlp = chunking.getCoordinatesInRoi(chunkId,
                                                                   offset,
                                                                   shape,
                                                                   offsetInRequest,
                                                                   requestShape,
                                                                   offsetInChunk);


            ds.getChunkShape(chunkId, chunkShape);
            chunkSize = std::accumulate(chunkShape.begin(), chunkShape.end(),
                                        1, std::multiplies<std::size_t>());

            // reshape buffer if necessary
            if(buffer.size() != chunkSize) {
                buffer.resize(chunkSize, val);
            }

            // request and chunk overlap completely
            // -> we can write the whole chunk
            if(completeOvlp) {
                ds.writeChunk(chunkId, &buffer[0]);
            }

            // request and chunk overlap only partially
            // -> we can only write partial data and need
            // to preserve the data that will not be written
            else {
                // load the current data into the buffer
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
                bufView = val;
                ds.writeChunk(chunkId, &buffer[0]);

                // need to reset the buffer to our fill value
                std::fill(buffer.begin(), buffer.end(), val);
            }
        });
    }

    template<typename T, typename ITER>
    void writeScalar(const Dataset & ds,
                     ITER roiBeginIter,
                     ITER roiShapeIter,
                     const T val,
                     const int numberOfThreads=1) {

        // get the offset and shape of the request and check if it is valid
        types::ShapeType offset(roiBeginIter, roiBeginIter+ds.dimension());
        types::ShapeType shape(roiShapeIter, roiShapeIter+ds.dimension());
        ds.checkRequestShape(offset, shape);
        ds.checkRequestType(typeid(T));

        // get the chunks that are involved in this request
        std::vector<types::ShapeType> chunkRequests;
        const auto & chunking = ds.chunking();
        chunking.getBlocksOverlappingRoi(offset, shape, chunkRequests);

        // write scalar single or multi-threaded
        if(numberOfThreads == 1) {
            writeScalarSingleThreaded<T>(ds, offset, shape, val, chunkRequests);
        } else {
            writeScalarMultiThreaded<T>(ds, offset, shape, val, chunkRequests, numberOfThreads);
        }
    }


    // unique ptr API
    template<typename T, typename ITER>
    inline void writeScalar(std::unique_ptr<Dataset> & ds,
                            ITER roiBeginIter,
                            ITER roiShapeIter,
                            const T val,
                            const int numberOfThreads=1) {
       writeScalar(*ds, roiBeginIter, roiShapeIter, val, numberOfThreads);
    }
}
}
