#pragma once

#include "z5/s3/handle.hxx"
#include "z5/s3/attributes.hxx"


namespace z5 {
namespace s3 {

    template<class GROUP>
    inline void writeMetadata(const z5::handle::File<GROUP> & handle, const Metadata & metadata) {
    }


    template<class GROUP>
    inline void writeMetadata(const z5::handle::Group<GROUP> & handle, const Metadata & metadata) {
    }


    inline void writeMetadata(const handle::Dataset & handle, const DatasetMetadata & metadata) {
    }


    inline void readMetadata(const handle::Dataset & handle, DatasetMetadata & metadata) {
        const bool isZarr = handle.isZarr();
        std::string objectName = handle.nameInBucket();
        if(isZarr) {
            objectName += "/.zarray";
        } else {
            objectName += "/attributes.json";
        }
        nlohmann::json j;
        attrs_detail::readAttributes(handle.bucketName(), objectName, j);
        metadata.fromJson(j, isZarr);
    }


}
}
