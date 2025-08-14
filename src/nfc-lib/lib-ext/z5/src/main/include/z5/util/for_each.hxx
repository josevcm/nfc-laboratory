#pragma once


#include "z5/dataset.hxx"
#include "z5/util/threadpool.hxx"
#include "z5/util/blocking.hxx"


namespace z5 {
namespace util {


    template<class F>
    void parallel_for_each_chunk(const Dataset & dataset, const int nThreads, F && f) {
        const auto & chunking = dataset.chunking();
        util::parallel_foreach(nThreads, dataset.numberOfChunks(), [&](const int tid, const size_t chunkId){
            types::ShapeType chunkCoord;
            chunking.blockIdToBlockCoordinate(chunkId, chunkCoord);
            f(tid, dataset, chunkCoord);
        });
    }


    template<class F>
    void parallel_for_each_chunk_in_roi(const Dataset & dataset,
                                        const types::ShapeType & roiBegin,
                                        const types::ShapeType & roiEnd,
                                        const int nThreads, F && f) {
        // get roi shape
        types::ShapeType roiShape(roiEnd);
        for(unsigned d = 0; d < roiShape.size(); ++d) {
            roiShape[d] -= roiBegin[d];
        }

        // get the (coordinate) chunk ids that overlap with the roi
        std::vector<types::ShapeType> chunks;
        const auto & chunking = dataset.chunking();
        chunking.getBlocksOverlappingRoi(roiBegin, roiShape, chunks);

        // loop over chunks in parallel and call lambda
        const size_t nChunks = chunks.size();
        util::parallel_foreach(nThreads, nChunks, [&](const int tid, const size_t chunkId){
            f(tid, dataset, chunks[chunkId]);
        });
    }


    template<class F>
    void parallel_for_each_block(const Dataset & dataset, const types::ShapeType & blockShape,
                                 const int nThreads, F && f) {
        const Blocking blocking(dataset.shape(), blockShape);
        const size_t nBlocks = blocking.numberOfBlocks();

        // loop over blocks in parallel
        util::parallel_foreach(nThreads, nBlocks, [&](const int tid, const size_t blockId){
            types::ShapeType blockBegin, blockShape;
            blocking.getBlockBeginAndShape(blockId, blockBegin, blockShape);
            f(tid, dataset, blockBegin, blockShape);
        });
    }


    template<class F>
    void parallel_for_each_block_in_roi(const Dataset & dataset, const types::ShapeType & blockShape,
                                        const types::ShapeType & roiBegin,
                                        const types::ShapeType & roiEnd,
                                        const int nThreads, F && f) {
        // get the roi shape
        types::ShapeType roiShape(roiEnd);
        for(unsigned d = 0; d < roiShape.size(); ++d) {
            roiShape[d] -= roiBegin[d];
        }

        // get blocks overlapping the roi
        const Blocking blocking(dataset.shape(), blockShape);
        std::vector<types::ShapeType> blockList;
        blocking.getBlocksOverlappingRoi(roiBegin, roiShape, blockList);
        const size_t nBlocks = blockList.size();

        // loop over blocks in parallel
        util::parallel_foreach(nThreads, nBlocks, [&](const int tid, const size_t blockId){
            types::ShapeType blockBegin, blockShape;
            blocking.getBlockBeginAndShape(blockList[blockId], blockBegin, blockShape);
            f(tid, dataset, blockBegin, blockShape);
        });

    }

}
}
