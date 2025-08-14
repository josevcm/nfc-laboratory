#pragma once

#include <sstream>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Object.h>
#include <aws/s3/model/GetObjectRequest.h>

#include "z5/s3/handle.hxx"


namespace z5 {
namespace s3 {

namespace attrs_detail {
    inline void readAttributes(const std::string & bucketName,
                               const std::string & objectName,
                               nlohmann::json & j) {
        // get options from somewhere?
        Aws::SDKOptions options;
        Aws::InitAPI(options);

        Aws::S3::S3Client client;
        Aws::S3::Model::GetObjectRequest request;
        request.SetBucket(Aws::String(bucketName.c_str(), bucketName.size()));
        request.SetKey(Aws::String(objectName.c_str(), objectName.size()));

        auto outcome = client.GetObject(request);
        if(outcome.IsSuccess()) {
            auto & retrieved = outcome.GetResultWithOwnership().GetBody();
            std::stringstream stream;
            stream << retrieved.rdbuf();
            const std::string content = stream.str();
            j = nlohmann::json::parse(content);
        }

        Aws::ShutdownAPI(options);
    }

}

    template<class GROUP>
    inline void readAttributes(const z5::handle::Group<GROUP> & group, nlohmann::json & j
    ) {
        std::string objectName = group.nameInBucket();
        if(group.isZarr()) {
            objectName += "/.zattrs";
        } else {
            objectName += "/attributes.json";
        }
        attrs_detail::readAttributes(group.bucketName(), objectName, j);
    }

    template<class GROUP>
    inline void writeAttributes(const z5::handle::Group<GROUP> & group, const nlohmann::json & j) {
    }

    template<class GROUP>
    inline void removeAttribute(const z5::handle::Group<GROUP> & group, const std::string & key) {
    }


    template<class DATASET>
    inline void readAttributes(const z5::handle::Dataset<DATASET> & ds, nlohmann::json & j
    ) {
        std::string objectName = ds.nameInBucket();
        if(ds.isZarr()) {
            objectName += "/.zattrs";
        } else {
            objectName += "/attributes.json";
        }
        attrs_detail::readAttributes(ds.bucketName(), objectName, j);
    }

    template<class DATASET>
    inline void writeAttributes(const z5::handle::Dataset<DATASET> & ds, const nlohmann::json & j) {
    }

    template<class DATASET>
    inline void removeAttribute(const z5::handle::Dataset<DATASET> & ds, const std::string & key) {
    }


    template<class GROUP>
    inline bool isSubGroup(const z5::handle::Group<GROUP> & group, const std::string & key){
        if(group.isZarr()) {
            return group.in(key + "/.zgroup");
        } else {
            nlohmann::json j;
            attrs_detail::readAttributes(group.bucketName(),
                                         group.nameInBucket() + "/attributes.json", j);
            return !z5::handle::hasAllN5DatasetAttributes(j);
        }
    }

}
}
