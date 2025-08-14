#pragma once
// aws includes
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Object.h>
#include <aws/s3/model/ListObjectsV2Request.h>

#include "z5/handle.hxx"


namespace z5 {
namespace s3 {
namespace handle {

    // TODO need to support more options
    // - different regions than us-east-1 (the default)
    // common functionality for S3 File and Group handles
    class S3HandleImpl {
    public:
        S3HandleImpl(const std::string & bucketName, const std::string & nameInBucket)
            : bucketName_(bucketName.c_str(), bucketName.size()),
              nameInBucket_(nameInBucket),
              options_(){}
        ~S3HandleImpl() {}

        // check if this handle exists
        inline bool existsImpl() const {
            Aws::InitAPI(options_);
            Aws::S3::S3Client client;
            Aws::S3::Model::ListObjectsV2Request request;
            request.WithBucket(Aws::String(bucketName_.c_str(), bucketName_.size()));
            request.WithPrefix(Aws::String(nameInBucket_.c_str(), nameInBucket_.size()));
            request.WithMaxKeys(1);
            const auto object_list = client.ListObjectsV2(request);
            const bool res = object_list.IsSuccess() && object_list.GetResult().GetKeyCount() > 0;
            Aws::ShutdownAPI(options_);
            return res;
        }

        inline void keysImpl(std::vector<std::string> & out) const {
            Aws::InitAPI(options_);
            Aws::S3::S3Client client;
            Aws::S3::Model::ListObjectsV2Request request;
            request.WithBucket(Aws::String(bucketName_.c_str(), bucketName_.size()));
            // add delimiter to the prefix
            const std::string bucketPrefix = nameInBucket_ == "" ? "" : nameInBucket_ + "/";
            request.WithPrefix(Aws::String(bucketPrefix.c_str(), bucketPrefix.size()));
            request.WithDelimiter("/");

            Aws::S3::Model::ListObjectsV2Result object_list;

            do {
                object_list = client.ListObjectsV2(request).GetResult();
                for(const auto & common_prefix : object_list.GetCommonPrefixes()) {
                    const std::string prefix(common_prefix.GetPrefix().c_str(),
                                             common_prefix.GetPrefix().size());
                    if(!prefix.empty() && prefix.back() == '/') {
                        std::vector<std::string> prefixSplit;
                        util::split(prefix, prefixSplit, "/");
                        out.emplace_back(prefixSplit.rbegin()[1]);
                    }
                }
            } while(object_list.GetIsTruncated());
            Aws::ShutdownAPI(options_);
        }

        inline bool inImpl(const std::string & name) const {
            Aws::InitAPI(options_);
            Aws::S3::S3Client client;
            Aws::S3::Model::ListObjectsV2Request request;
            request.WithBucket(Aws::String(bucketName_.c_str(), bucketName_.size()));
            const std::string prefix = nameInBucket_ == "" ? name : (nameInBucket_ + "/" + name);
            request.WithPrefix(Aws::String(prefix.c_str(), prefix.size()));
            request.WithMaxKeys(1);
            const auto object_list = client.ListObjectsV2(request);
            const bool res = object_list.IsSuccess() && object_list.GetResult().GetKeyCount() > 0;
            Aws::ShutdownAPI(options_);
            return res;
        }

        inline bool isZarrGroup() const {
            return inImpl(".zgroup");
        }
        inline bool isZarrDataset() const {
            return inImpl(".zarray");
        }

        inline const std::string & bucketNameImpl() const {
            return bucketName_;
        }

        inline const std::string & nameInBucketImpl() const {
            return nameInBucket_;
        }

    private:
        std::string bucketName_;
        std::string nameInBucket_;
        Aws::SDKOptions options_;
    };


    class File : public z5::handle::File<File>, private S3HandleImpl {
    public:
        typedef z5::handle::File<File> BaseType;

        // for now we only support vanilla SDKOptions
        File(const std::string & bucketName,
             const std::string & nameInBucket="",
             const FileMode mode=FileMode())
            : BaseType(mode),
              S3HandleImpl(bucketName, nameInBucket){}

        // Implement the handle API
        inline bool isS3() const {return true;}
        inline bool isGcs() const {return false;}
        // dummy impl
        const fs::path & path() const {}

        inline bool isZarr() const {
            return isZarrGroup();
        }

        inline bool exists() const {
            return existsImpl();
        }

        inline void create() const {
            if(!mode().canCreate()) {
                const std::string err = "Cannot create new file in file mode " + mode().printMode();
                throw std::invalid_argument(err.c_str());
            }
            // make sure that the file does not exist already
            if(exists()) {
                throw std::invalid_argument("Creating new file failed because it already exists.");
            }
        }

        inline void remove() const {
            if(!mode().canWrite()) {
                const std::string err = "Cannot remove file in file mode " + mode().printMode();
                throw std::invalid_argument(err.c_str());
            }
            if(!exists()) {
                throw std::invalid_argument("Cannot remove non-existing file.");
            }
        }

        // Implement the group handle API
        inline void keys(std::vector<std::string> & out) const {
            keysImpl(out);
        }
        inline bool in(const std::string & key) const {
            return inImpl(key);
        }

        inline const std::string & bucketName() const {
            return bucketNameImpl();
        }
        inline const std::string & nameInBucket() const {
            return nameInBucketImpl();
        }
    };


    class Group : public z5::handle::Group<Group>, private S3HandleImpl {
    public:
        typedef z5::handle::Group<Group> BaseType;

        template<class GROUP>
        Group(const z5::handle::Group<GROUP> & group, const std::string & key)
            : BaseType(group.mode()),
              S3HandleImpl(group.bucketName(),
                           (group.nameInBucket() == "") ? key : group.nameInBucket() + "/" + key){}

        // Implement th handle API
        inline bool isS3() const {return true;}
        inline bool isGcs() const {return false;}
        inline bool exists() const {return existsImpl();}
        inline bool isZarr() const {return isZarrGroup();}
        const fs::path & path() const {}

        inline void create() const {
            if(mode().mode() == FileMode::modes::r) {
                const std::string err = "Cannot create new group in file mode " + mode().printMode();
                throw std::invalid_argument(err.c_str());
            }
            // make sure that the file does not exist already
            if(exists()) {
                throw std::invalid_argument("Creating new group failed because it already exists.");
            }
        }

        inline void remove() const {
            if(!mode().canWrite()) {
                const std::string err = "Cannot remove group in group mode " + mode().printMode();
                throw std::invalid_argument(err.c_str());
            }
            if(!exists()) {
                throw std::invalid_argument("Cannot remove non-existing group.");
            }
        }

        // Implement the group handle API
        inline void keys(std::vector<std::string> & out) const {keysImpl(out);}
        inline bool in(const std::string & key) const {return inImpl(key);}
        inline const std::string & bucketName() const {
            return bucketNameImpl();
        }
        inline const std::string & nameInBucket() const {
            return nameInBucketImpl();
        }
    };


    class Dataset : public z5::handle::Dataset<Dataset>, private S3HandleImpl {
    public:
        typedef z5::handle::Dataset<Dataset> BaseType;

        template<class GROUP>
        Dataset(const z5::handle::Group<GROUP> & group, const std::string & key)
            : BaseType(group.mode()),
              S3HandleImpl(group.bucketName(),
                           (group.nameInBucket() == "") ? key : group.nameInBucket() + "/" + key){}

        // Implement th handle API
        inline bool isS3() const {return true;}
        inline bool isGcs() const {return false;}
        inline bool exists() const {return existsImpl();}
        inline bool isZarr() const {return isZarrDataset();}
        // dummy implementation
        const fs::path & path() const {}

        inline void create() const {
            // check if we have permissions to create a new dataset
            if(mode().mode() == FileMode::modes::r) {
                const std::string err = "Cannot create new dataset in mode " + mode().printMode();
                throw std::invalid_argument(err.c_str());
            }
            // make sure that the file does not exist already
            if(exists()) {
                throw std::invalid_argument("Creating new dataset failed because it already exists.");
            }
        }

        inline void remove() const {
            if(!mode().canWrite()) {
                const std::string err = "Cannot remove dataset in dataset mode " + mode().printMode();
                throw std::invalid_argument(err.c_str());
            }
            if(!exists()) {
                throw std::invalid_argument("Cannot remove non-existing dataset.");
            }
        }

        inline const std::string & bucketName() const {
            return bucketNameImpl();
        }
        inline const std::string & nameInBucket() const {
            return nameInBucketImpl();
        }
    };


    class Chunk : public z5::handle::Chunk<Chunk>, private S3HandleImpl {
    public:
        typedef z5::handle::Chunk<Chunk> BaseType;

        Chunk(const Dataset & ds,
              const types::ShapeType & chunkIndices,
              const types::ShapeType & chunkShape,
              const types::ShapeType & shape) : BaseType(chunkIndices, chunkShape, shape, ds.mode()),
                                                dsHandle_(ds),
                                                S3HandleImpl(ds.bucketName(), ds.nameInBucket() + "/" + getChunkKey(ds.isZarr())){}

        inline void remove() const {
        }

        inline const Dataset & datasetHandle() const {
            return dsHandle_;
        }

        inline bool isZarr() const {
            return dsHandle_.isZarr();
        }

        inline bool exists() const {return existsImpl();}

        // dummy impl
        const fs::path & path() const {}

        inline bool isS3() const {return true;}
        inline bool isGcs() const {return false;}
        inline const std::string & bucketName() const {return bucketNameImpl();}
        inline const std::string & nameInBucket() const {return nameInBucketImpl();}

    private:
        const Dataset & dsHandle_;
    };


    //
    // additional handle factory functions for compatibility with C
    //

    // TODO
    // implement handle factories for File and Group
    /*
    // get z5::filesystem::handle::File from char pointer corresponding
    // to the file on filesystem
    inline File getFileHandle(const char *) {
        File ret();
        return ret;
    }

    // get z5::filesystem::handle::File from char pointer corresponding
    // to the file on filesystem and char pointer corresponding to key of the group
    inline Group getGroupHandle(const char *, const char *) {
        Group ret();
        return ret;
    }
    */

}
}
}
