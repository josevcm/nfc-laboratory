#pragma once

#include "z5/handle.hxx"
#include "z5/filesystem/attributes.hxx"

#ifdef WITH_S3
#include "z5/s3/attributes.hxx"
#endif

#ifdef WITH_GCS
#include "z5/gcs/attributes.hxx"
#endif


namespace z5 {

namespace attrs_detail {
    inline void protectN5FileAttributes(const nlohmann::json & j) {
        // check if "n5" is in the attributes to write and throw if it is
        if(j.find("n5") != j.end()) {
            throw std::runtime_error("Can't overwrite n5 version attribute");
        }
    }

    inline void protectN5FileAttributes(const std::string & key) {
        // check if "n5" is in the attributes to write and throw if it is
        if(key == "n5") {
            throw std::runtime_error("Can't overwrite n5 version attribute");
        }
    }


    inline void protectN5DatasetAttributes(const nlohmann::json & j) {
        const std::set<std::string> protectedAttributes = {"dimensions", "blockSize", "dataType",
                                                           "compressionType", "compression"};
        for(const auto & attr : protectedAttributes) {
            if(j.find(attr) != j.end()) {
                throw std::runtime_error("Can't overwrite protected dataset attribute");
            }
        }
    }


    inline void protectN5DatasetAttributes(const std::string & key) {
        const std::set<std::string> protectedAttributes = {"dimensions", "blockSize", "dataType",
                                                           "compressionType", "compression"};
        if(protectedAttributes.find(key) != protectedAttributes.end()) {
            throw std::runtime_error("Can't overwrite protected dataset attribute");
        }
    }


    inline void hideN5DatasetAttributes(nlohmann::json & j) {
        // remove the n5 dataset attributes
        const std::vector<std::string> protectedAttributes = {"dimensions", "blockSize", "dataType",
                                                              "compressionType", "compression"};
        for(const auto & attr : protectedAttributes) {
            j.erase(attr);
        }
    }
}

    template<class GROUP>
    inline void readAttributes(const handle::Group<GROUP> & group, nlohmann::json & j) {

        #ifdef WITH_S3
        if(group.isS3()) {
            s3::readAttributes(group, j);
            return;
        }
        #endif
        #ifdef WITH_GCS
        if(group.isGcs()) {
            gcs::readAttributes(group, j);
            return;
        }
        #endif

        filesystem::readAttributes(group, j);
    }

    template<class GROUP>
    inline void writeAttributes(const handle::Group<GROUP> & group, const nlohmann::json & j) {

        // check if we have permissions to write for this group
        if(!group.mode().canWrite()) {
            const std::string err = "Cannot write attributes in mode " + group.mode().printMode();
            throw std::invalid_argument(err.c_str());
        }

        #ifdef WITH_S3
        if(group.isS3()) {
            s3::writeAttributes(group, j);
            return;
        }
        #endif
        #ifdef WITH_GCS
        if(group.isGcs()) {
            gcs::writeAttributes(group, j);
            return;
        }
        #endif

        filesystem::writeAttributes(group, j);
    }

    template<class GROUP>
    inline void removeAttribute(const handle::Group<GROUP> & group, const std::string & key) {

        // check if we have permissions to write for this group
        if(!group.mode().canWrite()) {
            const std::string err = "Cannot remove attributes in mode " + group.mode().printMode();
            throw std::invalid_argument(err.c_str());
        }

        #ifdef WITH_S3
        if(group.isS3()) {
            s3::removeAttribute(group, key);
            return;
        }
        #endif
        #ifdef WITH_GCS
        if(group.isGcs()) {
            gcs::removeAttribute(group, key);
            return;
        }
        #endif

        filesystem::removeAttribute(group, key);
    }


    template<class GROUP>
    inline void writeAttributes(const handle::File<GROUP> & file, const nlohmann::json & j) {

        // check if we have permissions to write for this file
        if(!file.mode().canWrite()) {
            const std::string err = "Cannot write attributes in mode " + file.mode().printMode();
            throw std::invalid_argument(err.c_str());
        }

        if(!file.isZarr()) {
            attrs_detail::protectN5FileAttributes(j);
        }

        #ifdef WITH_S3
        if(file.isS3()) {
            s3::writeAttributes(file, j);
            return;
        }
        #endif
        #ifdef WITH_GCS
        if(file.isGcs()) {
            gcs::writeAttributes(file, j);
            return;
        }
        #endif

        filesystem::writeAttributes(file, j);
    }

    template<class GROUP>
    inline void removeAttribute(const handle::File<GROUP> & file, const std::string & key) {

        // check if we have permissions to write for this file
        if(!file.mode().canWrite()) {
            const std::string err = "Cannot remove attributes in mode " + file.mode().printMode();
            throw std::invalid_argument(err.c_str());
        }

        if(!file.isZarr()) {
            attrs_detail::protectN5FileAttributes(key);
        }

        #ifdef WITH_S3
        if(file.isS3()) {
            s3::removeAttribute(file, key);
            return;
        }
        #endif
        #ifdef WITH_GCS
        if(file.isGcs()) {
            gcs::removeAttribute(file, key);
            return;
        }
        #endif

        filesystem::removeAttribute(file, key);
    }


    template<class DATASET>
    inline void readAttributes(const handle::Dataset<DATASET> & ds, nlohmann::json & j) {

        #ifdef WITH_S3
        if(ds.isS3()) {
            s3::readAttributes(ds, j);
            return;
        }
        #endif
        #ifdef WITH_GCS
        if(ds.isGcs()) {
            gcs::readAttributes(ds, j);
            return;
        }
        #endif

        filesystem::readAttributes(ds, j);
        if(!ds.isZarr()) {
            attrs_detail::hideN5DatasetAttributes(j);
        }
    }

    template<class DATASET>
    inline void writeAttributes(const handle::Dataset<DATASET> & ds, const nlohmann::json & j) {

        // check if we have permissions to write for this ds
        if(!ds.mode().canWrite()) {
            const std::string err = "Cannot write attributes in mode " + ds.mode().printMode();
            throw std::invalid_argument(err.c_str());
        }

        if(!ds.isZarr()) {
            attrs_detail::protectN5DatasetAttributes(j);
        }

        #ifdef WITH_S3
        if(ds.isS3()) {
            s3::writeAttributes(ds, j);
            return;
        }
        #endif
        #ifdef WITH_GCS
        if(ds.isGcs()) {
            gcs::writeAttributes(ds, j);
            return;
        }
        #endif

        filesystem::writeAttributes(ds, j);
    }

    template<class DATASET>
    inline void removeAttribute(const handle::Dataset<DATASET> & ds, const std::string & key) {

        // check if we have permissions to write for this ds
        if(!ds.mode().canWrite()) {
            const std::string err = "Cannot remove attributes in mode " + ds.mode().printMode();
            throw std::invalid_argument(err.c_str());
        }

        if(!ds.isZarr()) {
            attrs_detail::protectN5DatasetAttributes(key);
        }

        #ifdef WITH_S3
        if(ds.isS3()) {
            s3::removeAttribute(ds, key);
            return;
        }
        #endif
        #ifdef WITH_GCS
        if(ds.isGcs()) {
            gcs::removeAttribute(ds, key);
            return;
        }
        #endif

        filesystem::removeAttribute(ds, key);
    }


    template<class GROUP>
    inline bool isSubGroup(const handle::Group<GROUP> & group, const std::string & key){
        #ifdef WITH_S3
        if(group.isS3()) {
            return s3::isSubGroup(group, key);
        }
        #endif
        #ifdef WITH_GCS
        if(group.isGcs()) {
            return gcs::isSubGroup(group, key);
        }
        #endif
        return filesystem::isSubGroup(group, key);
    }
}
