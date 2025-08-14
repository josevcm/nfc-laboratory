#pragma once

#include "z5/dataset.hxx"
#include "z5/types/types.hxx"
#include "andres/marray.hxx"

// free functions to read and write from multiarrays
// (it's hard to have these as members due to dynamic type inference)

namespace z5 {
namespace multiarray {


    // FIXME this segfaults if the size of the view is larger than the buffer
    template<typename T>
    void copy_to_view(const std::vector<T> & buffer, andres::View<T> & view) {
        types::ShapeType shape(view.shapeBegin(), view.shapeEnd());
        //types::ShapeType strides(view.stridesBegin(), view.stridesEnd());
        std::size_t offset;
        for(std::size_t index = 0; index < view.size(); ++index) {
            // TODO if this does not work, use coordinate instead
            view.indexToOffset(index, offset);
            *(&view(0) + offset) = buffer[index];
        }
    }


    //
    template<typename T, typename ITER>
    void readSubarray(const Dataset & ds, andres::View<T> & out, ITER roiBeginIter) {

        // get the offset and shape of the request and check if it is valid
        types::ShapeType offset(roiBeginIter, roiBeginIter+out.dimension());
        types::ShapeType shape(out.shapeBegin(), out.shapeEnd());
        ds.checkRequestShape(offset, shape);
        ds.checkRequestType(typeid(T));

        // get the chunks that are involved in this request
        std::vector<types::ShapeType> chunkRequests;

        const auto & chunking = ds.chunking();
        chunking.getBlocksOverlappingRoi(offset, shape, chunkRequests);

        types::ShapeType offsetInRequest, shapeInRequest, chunkShape, offsetInChunk;

        // TODO writing directly to a view does not work, probably because it is not continuous in memory (?!)
        // that's why we use the buffer for now.
        // In the end, it would be nice to do this without the buffer (which introduces an additional copy)
        // Benchmark and figure this out !
        types::ShapeType bufferShape;
        // N5-Axis order: we need to reverse the max chunk shape
        if(ds.isZarr()) {
           bufferShape = types::ShapeType(ds.maxChunkShape().begin(), ds.maxChunkShape().end());
        } else {
           bufferShape = types::ShapeType(ds.maxChunkShape().rbegin(), ds.maxChunkShape().rend());
        }
        andres::Marray<T> buffer(andres::SkipInitialization, bufferShape.begin(), bufferShape.end());

        // buffer size
        auto bufferSize = std::accumulate(bufferShape.begin(), bufferShape.end(), 1, std::multiplies<std::size_t>());

        // iterate over the chunks
        for(const auto & chunkId : chunkRequests) {

            const bool completeOvlp = chunking.getCoordinatesInRoi(chunkId, offset,
                                                                   shape, offsetInRequest,
                                                                   shapeInRequest, offsetInChunk);
            auto view = out.view(offsetInRequest.begin(), shapeInRequest.begin());

            // get the current chunk-shape and resize the buffer if necessary
            ds.getChunkShape(chunkId, chunkShape);

            // N5-Axis order: we need to transpose the view and reverse the
            // chunk shape internally
            if(!ds.isZarr()) {
                view.transpose();
                std::reverse(chunkShape.begin(), chunkShape.end());
            }

            // reshape buffer if necessary
            if(bufferShape != chunkShape) {
                buffer.resize(andres::SkipInitialization, chunkShape.begin(), chunkShape.end());
                bufferShape = chunkShape;
                //buffer.resize
            }

            // read the current chunk into the buffer
            ds.readChunk(chunkId, &buffer(0));

            // FIXME tmp exps
            //ds.readChunk(chunkId, &buffer[0]);
            //copy_to_view(buffer, view);

            // request and chunk completely overlap
            // -> we can read all the data from the chunk
            if(completeOvlp) {
                // without data copy: not working
                //ds.readChunk(chunkId, &view(0));

                // THIS IS SUPER-SLOW!
                // copy the data from the buffer into the view
                view = buffer;

                // copy buffer into our view
                //copy_to_view(buffer, view);
            }
            // request and chunk overlap only partially
            // -> we can read the chunk data only partially
            else {

                // FIXME tmp exps
                //ds.readChunk(chunkId, &buffer(0));
                // copy the data from the correct buffer-view to the out view
                view = buffer.view(offsetInChunk.begin(), shapeInRequest.begin());
            }
        }
    }


    template<typename T, typename ITER>
    void writeSubarray(const Dataset & ds, const andres::View<T> & in, ITER roiBeginIter) {

        // get the offset and shape of the request and check if it is valid
        types::ShapeType offset(roiBeginIter, roiBeginIter+in.dimension());
        types::ShapeType shape(in.shapeBegin(), in.shapeEnd());

        ds.checkRequestShape(offset, shape);
        ds.checkRequestType(typeid(T));

        // get the chunks that are involved in this request
        std::vector<types::ShapeType> chunkRequests;
        const auto & chunking = ds.chunking();
        chunking.getBlocksOverlappingRoi(offset, shape, chunkRequests);

        types::ShapeType localOffset, localShape, chunkShape;
        types::ShapeType inChunkOffset;

        // TODO try to figure out writing without memcopys for fully overlapping chunks
        // create marray buffer
        types::ShapeType bufferShape;
        // N5-Axis order: we need to reverse the max chunk shape
        if(ds.isZarr()) {
            bufferShape = types::ShapeType(ds.maxChunkShape().begin(), ds.maxChunkShape().end());
        } else {
            bufferShape = types::ShapeType(ds.maxChunkShape().rbegin(), ds.maxChunkShape().rend());
        }
        andres::Marray<T> buffer(andres::SkipInitialization, bufferShape.begin(), bufferShape.end());

        // iterate over the chunks
        for(const auto & chunkId : chunkRequests) {

            const bool completeOvlp = chunking.getCoordinatesInRoi(chunkId, offset,
                                                                   shape, localOffset,
                                                                   localShape, inChunkOffset);
            ds.getChunkShape(chunkId, chunkShape);

            auto view = in.constView(localOffset.begin(), localShape.begin());
            // N5-Axis order: we need to reverse the chunk shape internally
            if(!ds.isZarr()) {
                std::reverse(chunkShape.begin(), chunkShape.end());
            }

            // resize buffer if necessary
            if(bufferShape != chunkShape) {
                buffer.resize(andres::SkipInitialization, chunkShape.begin(), chunkShape.end());
                bufferShape = chunkShape;
            }

            // request and chunk overlap completely
            // -> we can write the whole chunk
            if(completeOvlp) {

                // for now this does not work without copying,
                // because views are not contiguous in memory (I think ?!)
                //ds.writeChunk(chunkId, &view(0));

                // THIS IS SUPER-SLOW!
                buffer = view;
                ds.writeChunk(chunkId, &buffer(0));
            }

            // request and chunk overlap only partially
            // -> we can only write partial data and need
            // to preserve the data that will not be written
            else {
                // load the current data into the buffer
                ds.readChunk(chunkId, &buffer(0));
                // overwrite the data that is covered by the view
                auto bufView = buffer.view(inChunkOffset.begin(), localShape.begin());
                bufView = view;
                ds.writeChunk(chunkId, &buffer(0));
            }
        }
    }


    // unique ptr API
    template<typename T, typename ITER>
    inline void readSubarray(std::unique_ptr<Dataset> & ds, andres::View<T> & out, ITER roiBeginIter) {
       readSubarray(*ds, out, roiBeginIter);
    }

    template<typename T, typename ITER>
    inline void writeSubarray(std::unique_ptr<Dataset> & ds, const andres::View<T> & in, ITER roiBeginIter) {
        writeSubarray(*ds, in, roiBeginIter);
    }

}
}
