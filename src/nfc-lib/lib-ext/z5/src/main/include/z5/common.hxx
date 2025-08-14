#pragma once

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

inline fs::path relativeImpl(const fs::path & from, const fs::path & to){
    return fs::relative(to, from);
}


namespace z5 {

    // NOTE we can't just define variables, because that would not work as header-only include
    //
    // get booleans for preprocessor flags
    //

    inline std::map<std::string, bool> getAvailableCodecs() {
        std::map<std::string, bool> flags;

        flags["raw"] = true;

        flags["blosc"] = false;
        #ifdef WITH_BLOSC
        flags["blosc"] = true;
        #endif

        flags["bzip2"] = false;
        #ifdef WITH_BZIP2
        flags["bzip2"] = true;
        #endif

        flags["lz4"] = false;
        #ifdef WITH_LZ4
        flags["lz4"] = true;
        #endif

        flags["xz"] = false;
        #ifdef WITH_XZ
        flags["xz"] = true;
        #endif

        flags["zlib"] = false;
        flags["gzip"] = false;
        #ifdef WITH_ZLIB
        flags["zlib"] = true;
        flags["gzip"] = true;
        #endif

        return flags;
    }


    inline std::map<std::string, bool> getAvailableBackends() {
        std::map<std::string, bool> flags;

        flags["filesystem"] = true;

        flags["gcs"] = false;
        #ifdef WITH_GCS
        flags["gcs"] = true;
        #endif
        flags["s3"] = false;
        #ifdef WITH_S3
        flags["s3"] = true;
        #endif

        return flags;
    }
}
