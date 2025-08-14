#pragma once

#include "xtensor.hpp"
#include "xtensor/containers/xadapt.hpp"
#include "xtensor/views/xstrided_view.hpp"

namespace z5 {
namespace multiarray {


    // small helper function to convert a ROI given by (offset, shape) into
    // a proper xtensor sliceing
    inline void sliceFromRoi(xt::xstrided_slice_vector & roiSlice,
                             const types::ShapeType & offset,
                             const types::ShapeType & shape) {
        for(int d = 0; d < offset.size(); ++d) {
            roiSlice.push_back(xt::range(offset[d], offset[d] + shape[d]));
        }
    }


    inline std::size_t offsetAndStridesFromRoi(const types::ShapeType & outStrides,
                                          const types::ShapeType & requestShape,
                                          const types::ShapeType & offsetInRequest,
                                          types::ShapeType & requestStrides) {
        // first, calculate the flat offset
        // -> sum( coordinate_offset * out_strides )
        std::size_t flatOffset = 1;
        for(int d = 0; d < outStrides.size(); ++d) {
            flatOffset += (outStrides[d] * offsetInRequest[d]);
        }

        // TODO this depends on the layout type.....
        // next, calculate the new strides
        requestStrides.resize(requestShape.size());
        for(int d = 0; d < requestShape.size(); ++d) {
            requestStrides[d] = std::accumulate(requestShape.rbegin(), requestShape.rbegin() + d, 1, std::multiplies<std::size_t>());
        }
        std::reverse(requestStrides.begin(), requestStrides.end());

        return flatOffset;
    }

    template<typename T, typename VIEW, typename SHAPE_TYPE>
    inline void copyBufferToViewND(const std::vector<T> & buffer,
                                   xt::xexpression<VIEW> & viewExpression,
                                   const SHAPE_TYPE & arrayStrides) {
        // get the view into the out array and the number of dimension
        auto & view = viewExpression.derived_cast();
        const std::size_t dim = view.dimension();
        // buffer size and view shape
        const std::size_t bufSize = buffer.size();
        const auto & viewShape = view.shape();
        // initialize the (1d) offset into the buffer and view
        std::size_t bufferOffset = 0;
        std::size_t viewOffset = 0;
        // vector to keep track along the position along each dimension
        types::ShapeType dimPositions(dim);
        // THIS ASSUMES C-ORDER
        // -> memory is consecutive along the last axis
        const std::size_t memLen = viewShape[dim - 1];

        // we copy data to consecutive pieces of memory in the view
        // until we have exhausted the buffer

        // we start the outer loop at the second from last dimension
        // (last dimension is the fastest moving and consecutive in memory)
        for(int d = dim - 2; d >= 0;) {
            // copy the piece of buffer that is consecutive to our view
            std::copy(buffer.begin() + bufferOffset,
                      buffer.begin() + bufferOffset + memLen,
                      &view(0) + viewOffset);

            // increase the buffer offset by what we have just written to the view
            bufferOffset += memLen;
            // increase the view offsets by the strides along the second from last dimension
            viewOffset += arrayStrides[dim - 2];

            // check if we need to decrease the dimension
            for(d = dim - 2; d >= 0; --d) {
                // increase the position in the current dimension
                dimPositions[d] += 1;

                // if we are smaller than the shape in this dimension, stay in this dimension
                // (i.e. break and go back to the copy loop)
                if(dimPositions[d] < viewShape[d]) {
                    break;
                // otherwise, decrease the dimension
                } else {

                    // reset the position in this dimension
                    dimPositions[d] = 0;

                    // we don't need to increase the view offset if we are at
                    // the end of the next lower dim !
                    if(d > 0) {
                        if(dimPositions[d - 1] + 1 == viewShape[d - 1]) {
                            continue;
                        }
                    }

                    // increase the viewOffset to jump to the next point in memory
                    // for this, we increase by the stride of the next lower dimension
                    // but need to correct to jump back to the front of the view
                    // in that dimension
                    if(d > 0) {

                        // the correction to jump back to the front of the view
                        // in the next dim
                        std::size_t correction = 0;
                        for(int dd = dim - 2; dd >= d; --dd) {
                            correction += arrayStrides[dd] * (viewShape[dd] - 1);
                        }
                        // further correction because we incremented
                        // one time to much
                        correction += arrayStrides[dim - 2];

                        // increase the view offset
                        viewOffset += (arrayStrides[d - 1] - correction);
                    }
                }
            }
        }
    }


    // TODO this only works for row-major (C) memory layout
    template<typename T, typename VIEW, typename SHAPE_TYPE>
    inline void copyBufferToView(const std::vector<T> & buffer,
                                 xt::xexpression<VIEW> & viewExpression,
                                 const SHAPE_TYPE & arrayStrides) {
        auto & view = viewExpression.derived_cast();
        // ND impl doesn't work for 1D
        if(view.dimension() == 1) {
            // std::copy(buffer.begin(), buffer.end(), view.begin());
            const auto bufferView = xt::adapt(buffer, view.shape());
            view = bufferView;
        } else {
            copyBufferToViewND(buffer, viewExpression, arrayStrides);
        }
    }


    template<typename T, typename VIEW, typename SHAPE_TYPE>
    inline void copyViewToBufferND(const xt::xexpression<VIEW> & viewExpression,
                                  std::vector<T> & buffer,
                                  const SHAPE_TYPE & arrayStrides) {
        // get the view into the out array and the number of dimension
        const auto & view = viewExpression.derived_cast();
        const std::size_t dim = view.dimension();
        // buffer size and view shape
        const std::size_t bufSize = buffer.size();
        const auto & viewShape = view.shape();
        // initialize the (1d) offset into the buffer and view
        std::size_t bufferOffset = 0;
        std::size_t viewOffset = 0;
        // vector to keep track along the position along each dimension
        types::ShapeType dimPositions(dim);
        // THIS ASSUMES C-ORDER
        // -> memory is consecutive along the last axis
        const std::size_t memLen = viewShape[dim - 1];

        // we copy data that is consecutive in the view to the buffer
        // until we have exhausted the iew

        // we start the outer loop at the second from last dimension
        // (last dimension is the fastest moving and consecutive in memory)
        for(int d = dim - 2; d >= 0;) {
            // copy the piece of buffer that is consecutive to our view
            std::copy(&view(0) + viewOffset,
                      &view(0) + viewOffset + memLen,
                      buffer.begin() + bufferOffset);

            // increase the buffer offset by what we have just written to the view
            bufferOffset += memLen;
            // increase the view offsets by the strides along the second from last dimension
            viewOffset += arrayStrides[dim - 2];

            // check if we need to decrease the dimension
            for(d = dim - 2; d >= 0; --d) {
                // increase the position in the current dimension
                dimPositions[d] += 1;

                // if we are smaller than the shape in this dimension, stay in this dimension
                // (i.e. break and go back to the copy loop)
                if(dimPositions[d] < viewShape[d]) {
                    break;
                // otherwise, decrease the dimension
                } else {

                    // reset the position in this dimension
                    dimPositions[d] = 0;

                    // we don't need to increase the view offset if we are at
                    // the end of the next lower dim !
                    if(d > 0) {
                        if(dimPositions[d - 1] + 1 == viewShape[d - 1]) {
                            continue;
                        }
                    }

                    // increase the viewOffset to jump to the next point in memory
                    // for this, we increase by the stride of the next lower dimension
                    // but need to correct to jump back to the front of the view
                    // in that dimension
                    if(d > 0) {

                        // the correction to jump back to the front of the view
                        // in the next dim
                        std::size_t correction = 0;
                        for(int dd = dim - 2; dd >= d; --dd) {
                            correction += arrayStrides[dd] * (viewShape[dd] - 1);
                        }
                        // further correction because we incremented
                        // one time to much
                        correction += arrayStrides[dim - 2];

                        // increase the view offset
                        viewOffset += (arrayStrides[d - 1] - correction);
                    }
                }
            }
        }
    }


    // TODO this only works for row-major (C) memory layout
    template<typename T, typename VIEW, typename SHAPE_TYPE>
    inline void copyViewToBuffer(const xt::xexpression<VIEW> & viewExpression,
                                 std::vector<T> & buffer,
                                 const SHAPE_TYPE & arrayStrides) {
        const auto & view = viewExpression.derived_cast();
        // can't use the ND implementation in 1d, hence we resort to xtensor
        // which should be fine in 1D
        if(view.dimension() == 1) {
            // std::copy(view.begin(), view.end(), buffer.begin());
            auto bufferView = xt::adapt(buffer, view.shape());
            bufferView = view;
        } else {
            copyViewToBufferND(viewExpression, buffer, arrayStrides);
        }
    }


}
}
