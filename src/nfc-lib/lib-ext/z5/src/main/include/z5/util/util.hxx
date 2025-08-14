#pragma once

#include <string>

#include "z5/types/types.hxx"

namespace z5 {
namespace util {

    template<typename ITER>
    inline void join(const ITER & begin, const ITER & end, std::string & out, const std::string & delimiter) {
        for(ITER it = begin; it != end; ++it) {
            if(!out.empty()) {
                out.append(delimiter);
            }
            out.append(std::to_string(*it));
        }
    }


    inline void split(const std::string & in, std::vector<std::string> & out, const std::string & delimiter) {
        std::size_t curr, prev = 0;
        curr = in.find(delimiter);
        while(curr != std::string::npos) {
            out.push_back(in.substr(prev, curr - prev));
            prev = curr + 1;
            curr = in.find(delimiter, prev);
        }
        out.push_back(in.substr(prev, curr - prev));
    }

    // make regular grid between `minCoords` and `maxCoords` with step size 1.
    // uses imglib trick for ND code.
    inline void makeRegularGrid(const types::ShapeType & minCoords,
                                const types::ShapeType & maxCoords,
                                std::vector<types::ShapeType> & grid) {
        // get the number of dims and initialize the positions
        // at the min coordinates
        const int nDim = minCoords.size();
        types::ShapeType positions = minCoords;

        // start iteration in highest dimension
        for(int d = nDim - 1; d >= 0;) {
            // write out the coordinates
            grid.emplace_back(positions);
            // decrease the dimension
            for(d = nDim - 1; d >= 0; --d) {
                // increase position in the given dimension
                ++positions[d];
                // continue in this dimension if we have not reached
                // the max coord yet, otherwise reset the position to
                // the minimum coord and go to next lower dimension
                if(positions[d] <= maxCoords[d]) {
                    break;
                } else {
                    positions[d] = minCoords[d];
                }
            }
        }
    }


    // TODO in the long run this should be implemented as a filter for iostreams
    // reverse endianness for all values in the iterator range
    // boost endian would be nice, but it doesn't support floats...
    template<typename T, typename ITER>
    inline void reverseEndiannessInplace(ITER begin, ITER end) {
        int typeLen = sizeof(T);
        int typeMax = typeLen - 1;
        T ret;
        char * valP;
        char * retP = (char *) & ret;
        for(ITER it = begin; it != end; ++it) {
            valP = (char*) &(*it);
            for(int ii = 0; ii < typeLen; ++ii) {
                retP[ii] = valP[typeMax - ii];
            }
            *it = ret;
        }
    }

    // reverse endianness for as single value
    template<typename T>
    inline void reverseEndiannessInplace(T & val) {
        int typeLen = sizeof(T);
        int typeMax = typeLen - 1;
        T ret;
        char * valP = (char *) &val;
        char * retP = (char *) &ret;
        for(int ii = 0; ii < typeLen; ++ii) {
            retP[ii] = valP[typeMax - ii];
        }
        val = ret;
    }
}
}
