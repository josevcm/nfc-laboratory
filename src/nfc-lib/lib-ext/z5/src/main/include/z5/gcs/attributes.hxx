#pragma once

#include "z5/gcs/handle.hxx"


namespace z5 {
namespace gcs {

    // TODO implement for gcs

    /*
     * Attribute functionality for group
     */

    template<class GROUP>
    inline void readAttributes(const z5::handle::Group<GROUP> & group, nlohmann::json & j
    ) {
    }

    template<class GROUP>
    inline void writeAttributes(const z5::handle::Group<GROUP> & group, const nlohmann::json & j) {
    }

    template<class GROUP>
    inline void removeAttribute(const z5::handle::Group<GROUP> & group, const std::string & key) {
    }

    template<class GROUP>
    inline bool isSubGroup(const z5::handle::Group<GROUP> & group, const std::string & key){
    }

    /*
     * Attribute functionality for dataset
     */

    template<class DATASET>
    inline void readAttributes(const z5::handle::Dataset<DATASET> & ds, nlohmann::json & j
    ) {
    }

    template<class DATASET>
    inline void writeAttributes(const z5::handle::Dataset<DATASET> & ds, const nlohmann::json & j) {
    }

    template<class DATASET>
    inline void removeAttribute(const z5::handle::Dataset<DATASET> & group, const std::string & key) {
    }

}
}
