#pragma once

#include "z5/gcs/handle.hxx"


namespace z5 {
namespace gcs {

    // TODO implement for gcs

    template<class GROUP>
    inline void writeMetadata(const z5::handle::File<GROUP> & handle, const Metadata & metadata) {
    }


    template<class GROUP>
    inline void writeMetadata(const z5::handle::Group<GROUP> & handle, const Metadata & metadata) {
    }


    inline void writeMetadata(const handle::Dataset & handle, const DatasetMetadata & metadata) {
    }


    inline void readMetadata(const handle::Dataset & handle, DatasetMetadata & metadata) {
    }


}
}
