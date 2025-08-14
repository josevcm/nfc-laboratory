#pragma once

#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <iomanip>

#include "z5/types/types.hxx"


namespace z5 {

    // general format
    struct Metadata {
        bool isZarr; // flag to specify whether we have a zarr or n5 array
        int zarrFormat;
        int n5Major;
        int n5Minor;
        int n5Patch;

        Metadata(const bool isZarr) : isZarr(isZarr),
                                      zarrFormat(2),
                                      n5Major(2),
                                      n5Minor(0),
                                      n5Patch(0)
        {}
        inline std::string n5Format() const {
            return std::to_string(n5Major) + "." + std::to_string(n5Minor) + "." + std::to_string(n5Patch);
        }
    };


    struct DatasetMetadata : public Metadata {

        //template<typename T>
        DatasetMetadata(
            const types::Datatype dtype,
            const types::ShapeType & shape,
            const types::ShapeType & chunkShape,
            const bool isZarr,
            const types::Compressor compressor=types::raw,
            const types::CompressionOptions & compressionOptions=types::CompressionOptions(),
            const double fillValue=0,
            const std::string & zarrDelimiter="."
            ) : Metadata(isZarr),
                dtype(dtype),
                shape(shape),
                chunkShape(chunkShape),
                compressor(compressor),
                compressionOptions(compressionOptions),
                fillValue(fillValue),
                zarrDelimiter(zarrDelimiter)
        {
            checkShapes();
        }


        // empty constructor
        DatasetMetadata() : Metadata(true)
        {}


        //
        void toJson(nlohmann::json & j) const {
            if(isZarr) {
                toJsonZarr(j);
            } else {
                toJsonN5(j);
            }
        }


        //
        void fromJson(nlohmann::json & j, const bool isZarrDs) {
            isZarr = isZarrDs;
            isZarr ? fromJsonZarr(j) : fromJsonN5(j);
            checkShapes();
        }

    private:
        void toJsonZarr(nlohmann:: json & j) const {

            nlohmann::json compressionOpts;
            types::writeZarrCompressionOptionsToJson(compressor, compressionOptions, compressionOpts);
            j["compressor"] = compressionOpts;

            j["dtype"] = types::Datatypes::dtypeToZarr().at(dtype);
            j["shape"] = shape;
            j["chunks"] = chunkShape;

            if (std::isnan(fillValue)) {
              j["fill_value"] = "NaN";
            } else if (std::isinf(fillValue)) {
              j["fill_value"] = fillValue > 0 ? "Infinity" : "-Infinity";
            } else {
              j["fill_value"] = fillValue;
            }

            j["filters"] = nullptr;
            j["order"] = "C";
            j["zarr_format"] = zarrFormat;
            j["dimension_separator"] = zarrDelimiter;
        }

        void toJsonN5(nlohmann::json & j) const {

            // N5-Axis order: we need to reverse the shape when writing to metadata
            types::ShapeType rshape(shape.rbegin(), shape.rend());
            j["dimensions"] = rshape;

            // N5-Axis order: we need to reverse the block-size when writing to metadata
            types::ShapeType rchunks(chunkShape.rbegin(), chunkShape.rend());
            j["blockSize"] = rchunks;

            j["dataType"] = types::Datatypes::dtypeToN5().at(dtype);

            // write the new format
            nlohmann::json jOpts;
            types::writeN5CompressionOptionsToJson(compressor, compressionOptions, jOpts);
            j["compression"] = jOpts;
        }


        void fromJsonZarr(const nlohmann::json & j) {
            checkJson(j);
            try {
                dtype = types::Datatypes::zarrToDtype().at(j["dtype"]);
            } catch(std::out_of_range) {
                throw std::runtime_error("Unsupported zarr dtype: " + static_cast<std::string>(j["dtype"]));
            }
            shape = types::ShapeType(j["shape"].begin(), j["shape"].end());
            chunkShape = types::ShapeType(j["chunks"].begin(), j["chunks"].end());

            const auto & fillValJson = j["fill_value"];
            if(fillValJson.type() == nlohmann::json::value_t::string) {
                if (fillValJson == "NaN") {
                    fillValue = std::numeric_limits<double>::quiet_NaN();
                } else if (fillValJson == "Infinity") {
                    fillValue = std::numeric_limits<double>::infinity();
                } else if (fillValJson == "-Infinity") {
                    fillValue = -std::numeric_limits<double>::infinity();
                } else {
                    throw std::runtime_error("Invalid string value for fillValue");
                }
            } else if(fillValJson.type() == nlohmann::json::value_t::null) {
                fillValue = std::numeric_limits<double>::quiet_NaN();
            } else {
                fillValue = static_cast<double>(fillValJson);
            }

            auto jIt = j.find("dimension_separator");
            if(jIt != j.end()) {
                zarrDelimiter = *jIt;
            } else {
                zarrDelimiter = ".";
            }

            const auto & compressionOpts = j["compressor"];

            std::string zarrCompressorId = compressionOpts.is_null() ? "raw" : compressionOpts["id"];
            try {
                compressor = types::Compressors::zarrToCompressor().at(zarrCompressorId);
            } catch(std::out_of_range) {
                throw std::runtime_error("z5.DatasetMetadata.fromJsonZarr: wrong compressor for zarr format: " + zarrCompressorId);
            }

            types::readZarrCompressionOptionsFromJson(compressor, compressionOpts, compressionOptions);
        }


        void fromJsonN5(const nlohmann::json & j) {

            dtype = types::Datatypes::n5ToDtype().at(j["dataType"]);

            // N5-Axis order: we need to reverse the shape when reading from metadata
            shape = types::ShapeType(j["dimensions"].rbegin(), j["dimensions"].rend());

            // N5-Axis order: we need to reverse the chunk shape when reading from metadata
            chunkShape = types::ShapeType(j["blockSize"].rbegin(), j["blockSize"].rend());

            // we need to deal with two different encodings for compression in n5:
            // in the old format, we only have the field 'compressionType', indicating which compressor should be used
            // in the new format, we have the field 'type', which indicates the compressor
            // and can have additional attributes for options

            std::string n5Compressor;
            auto jIt = j.find("compression");

            if(jIt != j.end()) {
                const auto & jOpts = *jIt;
                auto j2It = jOpts.find("type");
                if(j2It != jOpts.end()) {
                    n5Compressor = *j2It;
                } else {
                    throw std::runtime_error("z5.DatasetMetadata.fromJsonN5: wrong compression format");
                }

                // get the actual compressor
                try {
                    compressor = types::Compressors::n5ToCompressor().at(n5Compressor);
                } catch(std::out_of_range) {
                    throw std::runtime_error("z5.DatasetMetadata.fromJsonN5: wrong compressor for n5 format");
                }

                readN5CompressionOptionsFromJson(compressor, jOpts, compressionOptions);
            }

            else {
                auto j2It = j.find("compressionType");
                if(j2It != j.end()) {
                    n5Compressor = *j2It;
                } else {
                    throw std::runtime_error("z5.DatasetMetadata.fromJsonN5: wrong compression format");
                }

                // get the actual compressor
                try {
                    compressor = types::Compressors::n5ToCompressor().at(n5Compressor);
                } catch(std::out_of_range) {
                    throw std::runtime_error("z5.DatasetMetadata.fromJsonN5: wrong compressor for n5 format");
                }

                // for the old compression, we just write the default gzip options
                compressionOptions["level"] = 5;
                compressionOptions["useZlib"] = false;
            }

            fillValue = 0;
        }

    public:
        // metadata values that can be set
        types::Datatype dtype;
        types::ShapeType shape;
        types::ShapeType chunkShape;

        // compressor name and options
        types::Compressor compressor;
        types::CompressionOptions compressionOptions;

        double fillValue;
        std::string zarrDelimiter;

        // metadata values that are fixed for now
        // zarr format is fixed to 2
        // const std::string order = "C";
        // const std::nullptr_t filters = nullptr;

    private:

        // make sure that shapes agree
        void checkShapes() {
            if(shape.size() != chunkShape.size()) {
                throw std::runtime_error("Dimension of shape and chunks does not agree");
            }
        }


        // make sure that fixed metadata values agree
        void checkJson(const nlohmann::json & j) {

            // check if order exists and check for the correct value
            auto jIt = j.find("order");
            if(jIt != j.end()) {
                if(*jIt != "C") {
                    throw std::runtime_error(
                        "Invalid Order: Z5 only supports C order"
                    );
                }
            }

            jIt = j.find("zarr_format");
            if(jIt != j.end()) {
                if(*jIt != zarrFormat) {
                    throw std::runtime_error(
                        "Invalid Zarr format: Z5 only supports zarr format 2"
                    );
                }
            }

            jIt = j.find("filters");
            if(jIt != j.end()) {
                if(!j["filters"].is_null() && j["filters"].size() > 0) {
                    throw std::runtime_error(
                        "Invalid Filters: Z5 does not support filters"
                    );
                }
            }
        }
    };


    inline void createDatasetMetadata(
        const std::string & dtype,
        const types::ShapeType & shape,
        const types::ShapeType & chunkShape,
        const bool createAsZarr,
        const std::string & compressor,
        const types::CompressionOptions & compressionOptions,
        const double fillValue,
        const std::string & zarrDelimiter,
        DatasetMetadata & metadata)
    {
        // get the internal data type
        types::Datatype internalDtype;
        try {
            internalDtype = types::Datatypes::n5ToDtype().at(dtype);
        } catch(const std::out_of_range & e) {
            throw std::runtime_error("z5::createDatasetMetadata: Invalid dtype for dataset");
        }

        // get the compressor
        types::Compressor internalCompressor;
        try {
            internalCompressor = types::Compressors::stringToCompressor().at(compressor);
        } catch(const std::out_of_range & e) {
            throw std::runtime_error("z5::createDatasetMetadata: Invalid compressor for dataset");
        }

        // add the default compression options if necessary
        // we need to make a copy of the compression options, because they are const
        auto internalCompressionOptions = compressionOptions;
        types::defaultCompressionOptions(internalCompressor, internalCompressionOptions, createAsZarr);

        metadata = DatasetMetadata(internalDtype, shape,
                                   chunkShape, createAsZarr,
                                   internalCompressor, internalCompressionOptions,
                                   fillValue, zarrDelimiter);
    }


} // namespace::z5
