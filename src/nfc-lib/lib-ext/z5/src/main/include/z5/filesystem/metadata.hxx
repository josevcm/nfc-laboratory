#pragma once

#include "z5/metadata.hxx"
#include "z5/filesystem/attributes.hxx"

namespace z5 {
namespace filesystem {

namespace metadata_detail {

    inline void writeMetadata(const fs::path & path, const nlohmann::json & j) {
        std::ofstream file(path);
        file << std::setw(4) << j << std::endl;
        file.close();
    }

    inline void readMetadata(const fs::path & path, nlohmann::json & j) {
        std::ifstream file(path);
        file >> j;
        file.close();
    }

    inline bool getMetadataPath(const handle::Dataset & handle, fs::path & path) {
        fs::path zarrPath = handle.path();
        fs::path n5Path = handle.path();
        zarrPath /= ".zarray";
        n5Path /= "attributes.json";
        if(fs::exists(zarrPath) && fs::exists(n5Path)) {
            throw std::runtime_error("Zarr and N5 specification are not both supported");
        }
        if(!fs::exists(zarrPath) && !fs::exists(n5Path)){
            throw std::runtime_error("Invalid path: no metadata existing");
        }
        const bool isZarr = fs::exists(zarrPath);
        path = isZarr ? zarrPath : n5Path;
        return isZarr;
    }
}

    template<class GROUP>
    inline void writeMetadata(const z5::handle::File<GROUP> & handleBase, const Metadata & metadata) {
        const auto & handle = handleBase;
        const bool isZarr = metadata.isZarr;
        const auto path = handle.path() / (isZarr ? ".zgroup" : "attributes.json");
        nlohmann::json j;
        if(isZarr) {
            j["zarr_format"] = metadata.zarrFormat;
        } else {
            // n5 stores attributes and metadata in the same file,
            // so we need to make sure that we don't overwrite attributes
            try {
                readAttributes(handle, j);
            } catch(std::runtime_error) {}  // read attributes throws RE if there are no attributes, we can just ignore this
            j["n5"] = metadata.n5Format();
        }
        metadata_detail::writeMetadata(path, j);
    }


    template<class GROUP>
    inline void writeMetadata(const z5::handle::Group<GROUP> & handle, const Metadata & metadata) {
        const bool isZarr = metadata.isZarr;
        const auto path = handle.path() / (isZarr ? ".zgroup" : "attributes.json");
        nlohmann::json j;
        if(isZarr) {
            j["zarr_format"] = metadata.zarrFormat;
        } else {
            // we don't need to write metadata for n5 groups
            return;
        }
        metadata_detail::writeMetadata(path, j);
    }


    inline void writeMetadata(const handle::Dataset & handle, const DatasetMetadata & metadata) {
        const auto path = handle.path() / (metadata.isZarr ? ".zarray" : "attributes.json");
        nlohmann::json j;
        metadata.toJson(j);
        metadata_detail::writeMetadata(path, j);
    }


    template<class GROUP>
    inline void readMetadata(const z5::handle::Group<GROUP> & handle, nlohmann::json & j) {
        const bool isZarr = handle.isZarr();
        const auto path = handle.path() / (isZarr ? ".zgroup" : "attributes.json");
        nlohmann::json jTmp;
        metadata_detail::readMetadata(path, jTmp);
        if(isZarr) {
            j["zarr_format"] = jTmp["zarr_format"];
        } else {
            auto jIt = jTmp.find("n5");
            if(jIt != jTmp.end()) {
                j["n5"] = jIt.value();
            }
        }
    }


    inline void readMetadata(const handle::Dataset & handle, DatasetMetadata & metadata) {
        nlohmann::json j;
        fs::path path;
        auto isZarr = metadata_detail::getMetadataPath(handle, path);
        metadata_detail::readMetadata(path, j);
        metadata.fromJson(j, isZarr);
    }

}
}
