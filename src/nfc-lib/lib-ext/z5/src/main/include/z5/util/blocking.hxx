#pragma once

#include "z5/types/types.hxx"
#include "z5/util/util.hxx"


namespace z5 {
namespace util {

    class Blocking {
    public:
        // TODO support offset
        Blocking(const types::ShapeType & shape,
                 const types::ShapeType & blockShape) : shape_(shape), blockShape_(blockShape) {
            init();
        }

        // empty constructor, leaves things uninitialized !
        // only implemented to allow having as member that is 
        // not assigned in initializer list, but needs
        Blocking(){}

        //
        // get member variables
        //
        inline size_t numberOfBlocks() const {return numberOfBlocks_;}
        inline const types::ShapeType & blocksPerDimension() const {return blocksPerDimension_;}
        inline const types::ShapeType & blockShape() const {return blockShape_;}
        inline const types::ShapeType & shape() const {return shape_;}

        //
        // block id to block coordinate and vice versa
        //
        inline void blockIdToBlockCoordinate(const size_t blockId,
                                             types::ShapeType & blockCoordinate) const {
            blockCoordinate.resize(shape_.size());
            std::size_t index = blockId;
            for(unsigned d = 0; d < shape_.size(); ++d) {
                blockCoordinate[d] = index / blockStrides_[d];
                index -= blockCoordinate[d] * blockStrides_[d];
            }
        }

        inline std::size_t blockCoordinatesToBlockId(const types::ShapeType & blockCoordinate) const {
            std::vector<std::size_t> offsets(shape_.size());
            for(unsigned d = 0; d < shape_.size() ; ++d) {
                offsets[d] = blockStrides_[d] * blockCoordinate[d];
            }
            return std::accumulate(offsets.begin(), offsets.end(), 0);
        }

        //
        // return block coordinate that maps to the global coordinate
        //
        inline void coordinateToBlockCoordinate(const types::ShapeType & coordinate,
                                                types::ShapeType & blockCoordinate) const {
            blockCoordinate.resize(shape_.size());
            for(unsigned d = 0; d < shape_.size(); ++d) {
               blockCoordinate[d] = coordinate[d] / blockShape_[d];
            }
        }

        //
        // coordinates of given block
        //
        inline void getBlockBeginAndShape(const std::size_t blockId,
                                          types::ShapeType & blockBegin,
                                          types::ShapeType & blockShape) const {
            types::ShapeType blockCoordinate;
            blockIdToBlockCoordinate(blockId, blockCoordinate);
            getBlockBeginAndShape(blockCoordinate, blockBegin, blockShape);
        }

        inline void getBlockBeginAndShape(const types::ShapeType & blockCoordinate,
                                          types::ShapeType & blockBegin,
                                          types::ShapeType & blockShape) const {
            blockBegin.resize(shape_.size());
            blockShape.resize(shape_.size());
            for(unsigned d = 0; d < shape_.size(); ++d) {
                blockBegin[d] = blockCoordinate[d] * blockShape_[d];
                blockShape[d] = (std::min)((blockCoordinate[d] + 1) * blockShape_[d], shape_[d]) - blockBegin[d];
            }
        }

        inline void getBlockBeginAndEnd(const std::size_t blockId,
                                        types::ShapeType & blockBegin,
                                        types::ShapeType & blockEnd) const {
            types::ShapeType blockCoordinate;
            blockIdToBlockCoordinate(blockId, blockCoordinate);
            getBlockBeginAndEnd(blockCoordinate, blockBegin, blockEnd);
        }

        inline void getBlockBeginAndEnd(const types::ShapeType & blockCoordinate,
                                        types::ShapeType & blockBegin,
                                        types::ShapeType & blockEnd) const {
            blockBegin.resize(shape_.size());
            blockEnd.resize(shape_.size());
            for(unsigned d = 0; d < shape_.size(); ++d) {
                blockBegin[d] = blockCoordinate[d] * blockShape_[d];
                blockEnd[d] = (std::min)((blockCoordinate[d] + 1) * blockShape_[d], shape_[d]);
            }
        }

        //
        // overlap of blocks and rois
        //

        // return all blocks overlapping with roi
        inline void getBlocksOverlappingRoi(const types::ShapeType & roiBegin,
                                            const types::ShapeType & roiShape,
                                            std::vector<types::ShapeType> & blockList) const {

            std::size_t nDim = roiBegin.size();
            // iterate over the dimension and find the min and max chunk ids
            types::ShapeType minBlockIds(nDim);
            types::ShapeType maxBlockIds(nDim);
            std::size_t endCoordinate, endId;
            for(unsigned d = 0; d < nDim; ++d) {
                // integer division is ok for both min and max-id, because
                // the chunk is labeled by it's lowest coordinate
                minBlockIds[d] = roiBegin[d] / blockShape_[d];
                endCoordinate = (roiBegin[d] + roiShape[d]);
                endId = endCoordinate / blockShape_[d];
                maxBlockIds[d] = (endCoordinate % blockShape_[d] == 0) ? endId - 1 : endId;
            }

            util::makeRegularGrid(minBlockIds, maxBlockIds, blockList);
        }

        // TODO
        // return all blocks strictly in roi
        inline void getBlocksInRoi(const types::ShapeType & roiBegin,
                                   const types::ShapeType & roiShape,
                                   std::vector<types::ShapeType> & blockList) const {
        }

        //
        // get the offset, shape of the block inside of the ROI as well as the offset of the ROI in chunk
        //
        inline bool getCoordinatesInRoi(const types::ShapeType & blockCoordinate,
                                        const types::ShapeType & roiBegin,
                                        const types::ShapeType & roiShape,
                                        types::ShapeType & beginInRoi,
                                        types::ShapeType & shapeInRoi,
                                        types::ShapeType & beginInBlock) const {

            const unsigned nDim = roiBegin.size();
            beginInRoi.resize(nDim);
            shapeInRoi.resize(nDim);
            beginInBlock.resize(nDim);

            types::ShapeType blockBegin, blockShape;
            getBlockBeginAndShape(blockCoordinate, blockBegin, blockShape);

            bool completeOvlp = true;
            std::size_t blockEnd, roiEnd;
            int offDiff, endDiff;
            for(unsigned d = 0; d < nDim; ++d) {

                // first we calculate the block end coordinate
                blockEnd = blockBegin[d] + blockShape[d];
                // next the roi end
                roiEnd = roiBegin[d] + roiShape[d];
                // then we calculate the difference between the block begin / end
                // and the request begin / end
                offDiff = blockBegin[d] - roiBegin[d];
                endDiff = roiEnd - blockEnd;

                // if the offset difference is negative, we are at a starting block
                // that is not completely overlapping
                // -> set all values accordingly
                if(offDiff < 0) {
                    beginInRoi[d] = 0; // start block -> no local offset
                    beginInBlock[d] = -offDiff;
                    completeOvlp = false;
                    // if this block is the beginning block as well as the end block,
                    // we need to adjust the local shape accordingly
                    shapeInRoi[d] = (blockEnd <= roiEnd) ? blockEnd - roiBegin[d] : roiEnd - roiBegin[d];
                }

                // if the end difference is negative, we are at a last block
                // that is not completely overlapping
                // -> set all values accordingly
                else if(endDiff < 0) {
                    beginInRoi[d] = blockBegin[d] - roiBegin[d];
                    beginInBlock[d] = 0;
                    completeOvlp = false;
                    shapeInRoi[d] = roiEnd - blockBegin[d];
                }

                // otherwise we are at a completely overlapping block
                else {
                    beginInRoi[d] = blockBegin[d] - roiBegin[d];
                    beginInBlock[d] = 0;
                    shapeInRoi[d] = blockShape[d];
                }

            }
            return completeOvlp;
        }

        // check that the block coordinate is valid
        inline bool checkBlockCoordinate(const types::ShapeType & blockCoordinate) const {
            // check dimension
            if(blockCoordinate.size() != shape_.size()) {
                return false;
            }

            // check block dimensions
            for(int d = 0; d < shape_.size(); ++d) {
                if(blockCoordinate[d] >= blocksPerDimension_[d]) {
                    return false;
                }
            }
            return true;
        }

    private:
        void init() {
            const unsigned ndim = shape_.size();
            blocksPerDimension_.resize(ndim);

            for(int d = 0; d < ndim; ++d) {
                blocksPerDimension_[d] = shape_[d] / blockShape_[d] + (shape_[d] % blockShape_[d] == 0 ? 0 : 1);
            }
            numberOfBlocks_ = std::accumulate(blocksPerDimension_.begin(),
                                              blocksPerDimension_.end(),
                                              1, std::multiplies<std::size_t>());

            // get the block strides
            blockStrides_.resize(ndim);
            blockStrides_[ndim - 1] = 1;
            for(int d = ndim - 2; d >= 0; --d) {
                blockStrides_[d] = blockStrides_[d + 1] * blocksPerDimension_[d + 1];
            }
        }

        types::ShapeType shape_;
        types::ShapeType blockShape_;
        types::ShapeType blocksPerDimension_;
        types::ShapeType blockStrides_;
        size_t numberOfBlocks_;
    };

}
}
