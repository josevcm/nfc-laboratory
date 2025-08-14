#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <variant>
#include <complex>

#include "nlohmann/json.hpp"



namespace z5 {
namespace types {

    //
    // Coordinates
    //

    // type for array shapes
    typedef std::vector<std::size_t> ShapeType;

    //
    // Datatypes
    //

    // dtype enum and map
    enum Datatype {
        int8, int16, int32, int64,
        uint8, uint16, uint32, uint64,
        float32, float64,
        complex64, complex128, complex256
    };

    struct Datatypes {
        typedef std::map<std::string, Datatype> DtypeMap;
        typedef std::map<Datatype, std::string> InverseDtypeMap;

        static DtypeMap & zarrToDtype() {
            static DtypeMap dtypeMap({{{"|i1", int8}, {"<i2", int16}, {"<i4", int32}, {"<i8", int64},
                                       {"|u1", uint8}, {"<u2", uint16}, {"<u4", uint32}, {"<u8", uint64},
                                       {"<f4", float32}, {"<f8", float64},
                                       {"<c8", complex64}, {"<c16", complex128}, {"<c32", complex256}}});
            return dtypeMap;
        }

        static InverseDtypeMap & dtypeToZarr() {

            static InverseDtypeMap dtypeMap({{{int8   , "|i1"}, {int16,  "<i2"}, {int32, "<i4"}, {int64, "<i8"},
                                              {uint8  , "|u1"}, {uint16, "<u2"}, {uint32, "<u4"},{uint64,"<u8"},
                                              {float32, "<f4"}, {float64,"<f8"},
                                              {complex64, "<c8"}, {complex128, "<c16"}, {complex256, "<c32"}}});
            return dtypeMap;
        }

        static DtypeMap & n5ToDtype() {
            static DtypeMap dtypeMap({{{"int8", int8},  {"int16", int16},  {"int32", int32},  {"int64", int64},
                                       {"uint8", uint8}, {"uint16", uint16}, {"uint32", uint32}, {"uint64", uint64},
                                       {"float32", float32}, {"float64", float64}}});
            return dtypeMap;
        }

        static InverseDtypeMap & dtypeToN5() {
            static InverseDtypeMap dtypeMap({{{int8   ,"int8"},    {int16,   "int16"},  {int32, "int32"},  {int64, "int64"},
                                              {uint8  ,"uint8"},   {uint16,  "uint16"}, {uint32,"uint32"}, {uint64, "uint64"},
                                              {float32,"float32"}, {float64, "float64"}}});
            return dtypeMap;
        }
    };


    //
    // Compressors
    //

    // the different compressors
    // that are supported
    enum Compressor {
        raw,
        #ifdef WITH_BLOSC
        blosc,
        #endif
        #ifdef WITH_ZLIB
        zlib,
        #endif
        #ifdef WITH_BZIP2
        bzip2,
        #endif
        #ifdef WITH_LZ4
        lz4,
        #endif
        #ifdef WITH_XZ
        xz
        #endif
    };


    struct Compressors {

        typedef std::map<std::string, Compressor> CompressorMap;
        typedef std::map<Compressor, std::string> InverseCompressorMap;

        static CompressorMap & stringToCompressor() {
            static CompressorMap cMap({{
                {"raw", raw},
                #ifdef WITH_BLOSC
                {"blosc", blosc},
                #endif
                #ifdef WITH_ZLIB
                {"zlib", zlib},
                {"gzip", zlib},
                #endif
                #ifdef WITH_BZIP2
                {"bzip2", bzip2},
                #endif
                #ifdef WITH_LZ4
                {"lz4", lz4},
                #endif
                #ifdef WITH_XZ
                {"xz", xz}
                #endif
            }});
            return cMap;
        }

        static CompressorMap & zarrToCompressor() {
            static CompressorMap cMap({{
                {"raw", raw},
                #ifdef WITH_BLOSC
                {"blosc", blosc},
                #endif
                #ifdef WITH_ZLIB
                {"zlib", zlib},
                {"gzip", zlib},
                #endif
                #ifdef WITH_BZIP2
                {"bz2", bzip2},
                #endif
                #ifdef WITH_LZ4
                {"lz4", lz4},
                #endif
            }});
            return cMap;
        }

        static InverseCompressorMap & compressorToZarr() {
            static InverseCompressorMap cMap({{
                {raw, "raw"},
                #ifdef WITH_BLOSC
                {blosc, "blosc"},
                #endif
                #ifdef WITH_ZLIB
                {zlib, "zlib"},
                #endif
                #ifdef WITH_BZIP2
                {bzip2, "bz2"},
                #endif
                #ifdef WITH_LZ4
                {lz4, "lz4"},
                #endif
            }});
            return cMap;
        }

        static CompressorMap & n5ToCompressor() {
            static CompressorMap cMap({{
                {"raw", raw},
                #ifdef WITH_ZLIB
                {"gzip", zlib},
                #endif
                #ifdef WITH_BZIP2
                {"bzip2", bzip2},
                #endif
                #ifdef WITH_XZ
                {"xz", xz},
                #endif
                #ifdef WITH_LZ4
                {"lz4", lz4},
                #endif
                #ifdef WITH_BLOSC
                {"blosc", blosc}
                #endif
            }});
            return cMap;
        }

        static InverseCompressorMap & compressorToN5() {
            static InverseCompressorMap cMap({{
                {raw, "raw"},
                #ifdef WITH_ZLIB
                {zlib, "gzip"},
                #endif
                #ifdef WITH_BZIP2
                {bzip2, "bzip2"},
                #endif
                #ifdef WITH_XZ
                {xz, "xz"},
                #endif
                #ifdef WITH_LZ4
                {lz4, "lz4"},
                #endif
                #ifdef WITH_BLOSC
                {blosc, "blosc"}
                #endif
            }});
            return cMap;
        }
    };


    //
    // Compression Options and Fill Value
    //
    typedef std::map<std::string, std::variant<int, bool, std::string>> CompressionOptions;

    inline void readZarrCompressionOptionsFromJson(Compressor compressor,
                                                   const nlohmann::json & jOpts,
                                                   CompressionOptions & options) {
        std::string codec;
        switch(compressor) {
            #ifdef WITH_BLOSC
            case blosc: options["codec"] = jOpts["cname"].get<std::string>();
                        options["level"] = jOpts["clevel"].get<int>();
                        options["shuffle"] = jOpts["shuffle"].get<int>();
                        // load blocksize with default value 0
                        options["blocksize"] = (jOpts.find("blocksize") == jOpts.end()) ? 0 : jOpts["blocksize"].get<int>();
                        break;
            #endif
            #ifdef WITH_ZLIB
            case zlib: options["level"] = jOpts["level"].get<int>();
                       options["useZlib"] = jOpts["id"].get<std::string>() == "zlib";
                       break;
            #endif
            #ifdef WITH_BZIP2
            case bzip2: options["level"] = jOpts["level"].get<int>(); break;
            #endif
            #ifdef WITH_LZ4
            case lz4: options["level"] = jOpts["acceleration"].get<int>(); break;
            #endif
            // raw compression has no parameters
            default: break;
        }
    }


    inline void writeZarrCompressionOptionsToJson(Compressor compressor,
                                                  const CompressionOptions & options,
                                                  nlohmann::json & jOpts) {
        try {
            if(compressor == types::raw) {
                jOpts = nullptr;
            } else {
                jOpts["id"] = types::Compressors::compressorToZarr().at(compressor);
            }
        } catch(std::out_of_range) {
            throw std::runtime_error("z5.DatasetMetadata.toJsonZarr: wrong compressor for zarr format");
        }

        switch(compressor) {
            #ifdef WITH_BLOSC
            case blosc: jOpts["cname"]   = std::get<std::string>(options.at("codec"));
                        jOpts["clevel"]  = std::get<int>(options.at("level"));
                        jOpts["shuffle"] = std::get<int>(options.at("shuffle"));
                        jOpts["blocksize"] = std::get<int>(options.at("blocksize"));
                        break;
            #endif
            #ifdef WITH_ZLIB
            case zlib: jOpts["id"] = std::get<bool>(options.at("useZlib")) ? "zlib" : "gzip";
                       jOpts["level"] = std::get<int>(options.at("level"));
                       break;
            #endif
            #ifdef WITH_BZIP2
            case bzip2: jOpts["level"] = std::get<int>(options.at("level")); break;
            #endif
            #ifdef WITH_LZ4
            case lz4: jOpts["acceleration"] = std::get<int>(options.at("level")); break;
            #endif
            // raw compression has no parameters
            default: break;
        }
    }


    inline void readN5CompressionOptionsFromJson(Compressor compressor,
                                                 const nlohmann::json & jOpts,
                                                 CompressionOptions & options) {
        std::string codec;
        switch(compressor) {
            #ifdef WITH_ZLIB
            case zlib: options["level"] = jOpts["level"].get<int>();
                       options["useZlib"] = false;
                       break;
            #endif
            #ifdef WITH_BZIP2
            case bzip2: options["level"] = jOpts["blockSize"].get<int>(); break;
            #endif
            #ifdef WITH_XZ
            case xz: options["level"] = jOpts["preset"].get<int>(); break;
            #endif
            #ifdef WITH_LZ4
            case lz4: options["level"] = jOpts["blockSize"].get<int>(); break;
            #endif
            #ifdef WITH_BLOSC
            case blosc: options["codec"] = jOpts["cname"].get<std::string>();
                        options["level"] = jOpts["clevel"].get<int>();
                        options["shuffle"] = jOpts["shuffle"].get<int>();
                        // load blocksize with default value 0
                        options["blocksize"] = (jOpts.find("blocksize") == jOpts.end()) ? 0 : jOpts["blocksize"].get<int>();
                        // load nthreads with default value 1
                        options["nthreads"] = (jOpts.find("nthreads") == jOpts.end()) ? 1 : jOpts["nthreads"].get<int>();
                        break;
            #endif
            // raw compression has no parameters
            default: break;
        }
    }


    inline void writeN5CompressionOptionsToJson(Compressor compressor,
                                                const CompressionOptions & options,
                                                nlohmann::json & jOpts) {
        try {
            jOpts["type"] = types::Compressors::compressorToN5().at(compressor);
        } catch(std::out_of_range) {
            throw std::runtime_error("z5.DatasetMetadata.toJsonN5: wrong compressor for N5 format");
        }

        switch(compressor) {
            #ifdef WITH_ZLIB
            case zlib: jOpts["level"] = std::get<int>(options.at("level"));
                       break;
            #endif
            #ifdef WITH_BZIP2
            case bzip2: jOpts["blockSize"] = std::get<int>(options.at("level"));
                        break;
            #endif
            #ifdef WITH_XZ
            case xz: jOpts["preset"] = std::get<int>(options.at("level")); break;
            #endif
            #ifdef WITH_LZ4
            case lz4: jOpts["blockSize"] = std::get<int>(options.at("level")); break;
            #endif
            #ifdef WITH_BLOSC
            case blosc: jOpts["cname"] = std::get<std::string>(options.at("codec"));
                        jOpts["clevel"] = std::get<int>(options.at("level"));
                        jOpts["shuffle"] = std::get<int>(options.at("shuffle"));
                        jOpts["blocksize"] = std::get<int>(options.at("blocksize"));
                        jOpts["nthreads"] = std::get<int>(options.at("nthreads"));
                        break;
            #endif
            // raw compression has no parameters
            default: break;
        }
    }


    inline void defaultCompressionOptions(Compressor compressor,
                                          CompressionOptions & options,
                                          const bool isZarr) {

        switch(compressor) {
            #ifdef WITH_BLOSC
            case blosc: if(options.find("codec") == options.end()){options["codec"] = std::string("lz4");}
                        if(options.find("level") == options.end()){options["level"] = 5;}
                        if(options.find("shuffle") == options.end()){options["shuffle"] = 1;}
                        if(options.find("blocksize") == options.end()){options["blocksize"] = 0;}
                        break;
            #endif
            #ifdef WITH_ZLIB
            case zlib: if(options.find("level") == options.end()){options["level"] = 5;}
                       if(options.find("useZlib") == options.end()){options["useZlib"] = isZarr;}
                       break;
            #endif
            #ifdef WITH_BZIP2
            case bzip2: if(options.find("level") == options.end()){options["level"] = 5;}
                        break;
            #endif
            #ifdef WITH_LZ4
            case lz4: if(options.find("level") == options.end()){options["level"] = 6;}
                      break;
            #endif
            #ifdef WITH_XZ
            case xz: if(options.find("level") == options.end()){options["level"] = 6;}
                     break;
            #endif
            // raw compression has no parameters
            default: break;
        }
    }

    // generic translation from compression type to json
    // for pybindings
    inline void jsonToCompressionType(const nlohmann::json & j, CompressionOptions & opts) {
        for(const auto & elem : j.items()) {
            const std::string & key = elem.key();
            const auto & val = elem.value();
            if(val.type() == nlohmann::json::value_t::boolean) {
                opts[key] = static_cast<bool>(val);
            } else if (val.type() == nlohmann::json::value_t::number_integer || val.type() == nlohmann::json::value_t::number_unsigned) {
                opts[key] = static_cast<int>(val);
            } else if (val.type() == nlohmann::json::value_t::string) {
                // msvc does not like the static cast here ....
                const std::string tmp = val;
                opts[key] = tmp;
            } else {
                std::cout << val.type_name() << std::endl;
                throw std::runtime_error("Invalid type conversion for compression type");
            }
        }
    }

    inline void compressionTypeToJson(const CompressionOptions & opts, nlohmann::json & j) {
        for(auto & elem : opts) {
            const auto & val = elem.second;
            if(std::holds_alternative<int>(val)){
                j[elem.first] = std::get<int>(val);
            } else if(std::holds_alternative<bool>(val)){
                j[elem.first] = std::get<bool>(val);
            } else if(std::holds_alternative<std::string>(val)){
                j[elem.first] = std::get<std::string>(val);
            } else {
                throw std::runtime_error("Invalid type conversion for compression type");
            }

        }
    }

} // namespace::types
    // overload ostream operator for ShapeType (a.k.a) vector for convenience
    inline std::ostream & operator << (std::ostream & os, const types::ShapeType & coord) {
        os << "Coordinates(";
        for(const auto & cc: coord) {
            os << " " << cc;
        }
        os << " )";
        return os;
    }
} // namespace::z5
