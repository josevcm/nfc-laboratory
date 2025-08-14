#pragma once

#include <fstream>
#include "z5/filesystem/handle.hxx"


namespace z5 {
namespace filesystem {


namespace attrs_detail {

    inline void readAttributes(const fs::path & path, nlohmann::json & j) {
        if(!fs::exists(path)) {
            return;
        }
        std::ifstream file(path);
        file >> j;
        file.close();
    }

    inline void writeAttributes(const fs::path & path, const nlohmann::json & j) {
        nlohmann::json jOut;
        // if we already have attributes, read them
        if(fs::exists(path)) {
            std::ifstream file(path);
            file >> jOut;
            file.close();
        }
        for(auto jIt = j.begin(); jIt != j.end(); ++jIt) {
            jOut[jIt.key()] = jIt.value();
        }
        std::ofstream file(path);
        file << jOut;
        file.close();
    }

    inline void removeAttribute(const fs::path & path, const std::string & key) {
        nlohmann::json jOut;
        // if we already have attributes, read them
        if(fs::exists(path)) {
            std::ifstream file(path);
            file >> jOut;
            file.close();
        }
        else {
            return;
        }
        jOut.erase(key);
        std::ofstream file(path);
        file << jOut;
        file.close();
    }
}

    template<class GROUP>
    inline void readAttributes(const z5::handle::Group<GROUP> & group, nlohmann::json & j
    ) {
        const auto path = group.path() / (group.isZarr() ? ".zattrs" : "attributes.json");
        attrs_detail::readAttributes(path, j);
    }

    template<class GROUP>
    inline void writeAttributes(const z5::handle::Group<GROUP> & group, const nlohmann::json & j) {
        const auto path = group.path() / (group.isZarr() ? ".zattrs" : "attributes.json");
        attrs_detail::writeAttributes(path, j);
    }

    template<class GROUP>
    inline void removeAttribute(const z5::handle::Group<GROUP> & group, const std::string & key) {
        const auto path = group.path() / (group.isZarr() ? ".zattrs" : "attributes.json");
        attrs_detail::removeAttribute(path, key);
    }


    template<class DATASET>
    inline void readAttributes(const z5::handle::Dataset<DATASET> & ds, nlohmann::json & j
    ) {
        const auto path = ds.path() / (ds.isZarr() ? ".zattrs" : "attributes.json");
        attrs_detail::readAttributes(path, j);
    }

    template<class DATASET>
    inline void writeAttributes(const z5::handle::Dataset<DATASET> & ds, const nlohmann::json & j) {
        const auto path = ds.path() / (ds.isZarr() ? ".zattrs" : "attributes.json");
        attrs_detail::writeAttributes(path, j);
    }

    template<class DATASET>
    inline void removeAttribute(const z5::handle::Dataset<DATASET> & ds, const std::string & key) {
        const auto path = ds.path() / (ds.isZarr() ? ".zattrs" : "attributes.json");
        attrs_detail::removeAttribute(path, key);
    }


    template<class GROUP>
    inline bool isSubGroup(const z5::handle::Group<GROUP> & group, const std::string & key){
        fs::path path = group.path() / key;
        if(!fs::exists(path)) {
            return false;
        }
        if(group.isZarr()) {
            path /= ".zgroup";
            return fs::exists(path);
        } else {
            path /= "attributes.json";
            if(!fs::exists(path)) {
                return true;
            }
            nlohmann::json j;
            attrs_detail::readAttributes(path, j);
            return !z5::handle::hasAllN5DatasetAttributes(j);
        }
    }

}
}
