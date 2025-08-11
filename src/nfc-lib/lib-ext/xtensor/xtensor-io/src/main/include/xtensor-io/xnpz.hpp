/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

// Inspired by cnpy: https://github.com/rogersce/cnpy/

#ifndef XTENSOR_IO_XNPZ_HPP
#define XTENSOR_IO_XNPZ_HPP

#include <cassert>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

#include "thirdparty/zstr/zstr.hpp"

#include <xtensor/xnpy.hpp>

#include "xtensor_io_config.hpp"

#ifdef __CLING__
    #pragma cling load("z")
#endif

namespace xt
{
    using namespace std::string_literals;

    namespace detail
    {
#pragma pack(push, 2)
        struct zip_local_header
        {
            char sig[4];
            uint16_t version;
            uint16_t gp_flags;
            uint16_t compression_method;
            uint16_t last_modification_time;
            uint16_t last_modification_date;
            uint32_t crc32;
            uint32_t compressed_size;
            uint32_t uncompressed_size;
            uint16_t filename_len;
            uint16_t extra_field_len;

            auto signature() const { return std::string(sig, sizeof(sig)); }
        };
#pragma pack(pop)

        struct extensible_data_field
        {
            uint16_t header_id;
            uint16_t data_size;
        };

#pragma pack(push, 4)
        struct zip64_extended_information
        {
            uint64_t original_size;
            uint64_t compressed_size;
            uint64_t relative_header_offset;  // optional
            uint32_t disk_start_number;       // optional
        };
#pragma pack(pop)

        inline uint64_t extract_zip64_compressed_size_within(std::istream& stream,
                                                      std::streamsize nbytes)
        {
            for (extensible_data_field desc; nbytes > sizeof(desc);)
            {
                if (!stream.read(reinterpret_cast<char*>(&desc), sizeof(desc)))
                    throw std::runtime_error(
                        "load_npz: unexpected end-of-file in extensible data "
                        "fields.");
                nbytes -= static_cast<uint16_t>(sizeof(desc)) + desc.data_size;
                if (desc.header_id == 0x0001)
                {
                    alignas(uint64_t) zip64_extended_information zip64;
                    if (desc.data_size > sizeof(zip64) ||
                        desc.data_size < sizeof(uint64_t) * 2)
                        throw std::runtime_error("load_npz: zip64 extended "
                                                 "information is malformed.");
                    if (!stream.read(reinterpret_cast<char*>(&zip64),
                                     desc.data_size))
                        throw std::runtime_error(
                            "load_npz: unexpected end-of-file in zip64 "
                            "extended information.");
                    if (!stream.ignore(nbytes))
                        throw std::runtime_error(
                            "load_npz: failed reading extra field.");
                    return zip64.compressed_size;
                }
                else
                {
                    if (!stream.ignore(desc.data_size))
                        throw std::runtime_error(
                            "load_npz: failed reading extensible data field.");
                }
            }

            throw std::runtime_error(
                "load_npz: missing zip64 extended information.");
        }

        inline uint64_t extract_zip64_compressed_size(std::istream& stream,
                                               zip_local_header const& entry)
        {
            if (entry.compressed_size == 0xffffffff)
                return extract_zip64_compressed_size_within(
                    stream, entry.extra_field_len);
            else
            {
                if (!stream.ignore(entry.extra_field_len))
                    throw std::runtime_error(
                        "load_npz: failed reading extra field.");
                return entry.compressed_size;
            }
        }
    }

    /**
     * Loads a npz file.
     * This function returns a map. The individual arrays are not casted to a
     * specific file format. This has to be done before they can be used as
     * xarrays (e.g. ``auto arr_0 = npz_map["arr_0"].cast<double>();``)
     *
     * @param filename The filename of the npz file
     *
     * @return returns a map with the stored arrays.
     *         Array names are the keys.
     */
    inline auto load_npz(std::string filename)
    {
        std::ifstream stream(filename, std::ifstream::binary);

        if (!stream.is_open())
        {
            throw std::runtime_error("load_npz: failed to open file " + filename);
        }

        using result_type = std::map<std::string, detail::npy_file>;
        result_type arrays;

        alignas(uint32_t) detail::zip_local_header entry;
        for (;;)
        {
            if (!stream.read(reinterpret_cast<char*>(&entry), sizeof(entry)))
            {
                throw std::runtime_error("load_npz: unexpected end-of-file.");
            }

            //if we did not encounter a local header, stop reading
            if (entry.signature() != "PK\x03\x04"s)
            {
                break;
            }

            //read in the variable name
            std::string varname;
            varname.resize(entry.filename_len);
            if (!stream.read(&varname[0], entry.filename_len))
            {
                throw std::runtime_error("load_npz: failed to read variable name.");
            }

            //erase the lagging .npy
            varname.erase(varname.end() - 4, varname.end());

            switch (entry.compression_method)
            {
            case 0:
                //move straight to the file content since npy knows its length
                if (!stream.ignore(entry.extra_field_len))
                {
                    throw std::runtime_error("load_npz: failed reading extra field.");
                }

                arrays.insert(result_type::value_type(varname,
                                                      detail::load_npy_file(stream)));
                break;
            case 8:
            {
                auto compressed_size = extract_zip64_compressed_size(stream, entry);
                auto pos = stream.tellg();
                zstr::istream zstream(stream);

                if (!zstream)
                {
                    throw std::runtime_error("load_npz: failed to open zstream.");
                }

                arrays.insert(result_type::value_type(varname,
                                                      detail::load_npy_file(zstream)));
                if (!stream.seekg(pos + std::streamoff(compressed_size)))
                {
                    throw std::runtime_error(
                        "load_npz: unable to read the next variable.");
                }

                break;
            }
            default:
                throw std::runtime_error("load_npz: unsupported compression method.");
            }
        }
        return arrays;
    }

    /**
     * Loads a specific array indicated by search_varname from npz file.
     * All other data in the npz file is ignored.
     *
     * @param filename The npz filename
     * @param search_varname The array name to be loaded
     *
     * @return xarray with the contents of the loaded array
     */
    template <class T>
    xarray<T> load_npz(std::string filename, std::string search_varname)
    {
        std::ifstream stream(filename, std::ifstream::binary);

        if (!stream.is_open())
        {
            throw std::runtime_error("load_npz: failed to open file " + filename);
        }

        alignas(uint32_t) detail::zip_local_header entry;
        for (;;)
        {
            if (!stream.read(reinterpret_cast<char*>(&entry), sizeof(entry)))
            {
                throw std::runtime_error("load_npz: unexpected end-of-file.");
            }

            //if we did not encounter a local header, stop reading
            if (entry.signature() != "PK\x03\x04"s)
            {
                break;
            }

            //read in the variable name
            std::string varname;
            varname.resize(entry.filename_len);
            if (!stream.read(&varname[0], entry.filename_len))
            {
                throw std::runtime_error("load_npz: failed to read variable name.");
            }

            //erase the lagging .npy
            varname.erase(varname.end() - 4, varname.end());

            if (varname == search_varname)
            {
                switch (entry.compression_method)
                {
                case 0:
                {
                    //move straight to the file content since npy knows its length
                    if (!stream.ignore(entry.extra_field_len))
                    {
                        throw std::runtime_error("load_npz: failed reading extra field.");
                    }

                    detail::npy_file f = detail::load_npy_file(stream);
                    return f.cast<T>();
                }
                case 8:
                {
                    auto compressed_size = extract_zip64_compressed_size(stream, entry);
                    auto pos = stream.tellg();
                    zstr::istream zstream(stream);

                    if (!zstream)
                    {
                        throw std::runtime_error("npz_load: failed to open zstream.");
                    }

                    detail::npy_file f = detail::load_npy_file(zstream);
                    if (!stream.seekg(pos + std::streamoff(compressed_size)))
                    {
                        throw std::runtime_error(
                            "load_npz: unable to read the next variable.");
                    }
                    return f.cast<T>();
                }
                default:
                    throw std::runtime_error("load_npz: unsupported compression method.");
                }
            }
            else
            {
                stream.seekg(static_cast<std::streamsize>(entry.compressed_size),
                             std::ios_base::cur);
            }
        }
        throw std::runtime_error("Array "s + search_varname + " not found in file: "s + filename);
    }

    namespace detail
    {
        /**
         * Helper class that implements a vector of chars.
         * Exposes something similar to a minimalistic stream
         * interface by overloading the << operator.
         */
        class binary_vector : public std::vector<char>
        {
        public:

            using self_type = binary_vector;

            template <class T>
            self_type& operator<<(const T& rhs)
            {
                for (size_t byte = 0; byte < sizeof(T); byte++)
                {
                    char val = *((char*)&rhs + byte);
                    this->push_back(val);
                }
                return *this;
            }

            self_type& operator<<(const std::string& rhs)
            {
                this->insert(this->end(), rhs.begin(), rhs.end());
                return *this;
            }

            self_type& operator<<(const char* rhs)
            {
                //write in little endian
                std::size_t len = strlen(rhs);
                this->reserve(len);
                for (std::size_t byte = 0; byte < len; byte++)
                {
                    this->push_back(rhs[byte]);
                }
                return *this;
            }

            bool write(const char* char_arr, std::size_t size)
            {
                this->reserve(this->size() + size);
                for (std::size_t byte = 0; byte < size; byte++)
                {
                    this->push_back(char_arr[byte]);
                }
                return true;
            }

            bool put(char c)
            {
                this->push_back(c);
                return true;
            }
        };

        inline void parse_zip_footer(std::istream& stream,
                              uint16_t& nrecs,
                              std::streamsize& global_header_size,
                              std::streamoff& global_header_offset)
        {
            std::vector<char> footer(22);
            stream.seekg(-22, std::ios_base::end);

            stream.read(&footer[0], 22);

            if (!stream)
            {
                throw std::runtime_error("parse_zip_footer: failed to read");
            }

            uint16_t disk_no, disk_start, nrecs_on_disk, comment_len;
            disk_no = *reinterpret_cast<uint16_t*>(&footer[4]);
            disk_start = *reinterpret_cast<uint16_t*>(&footer[6]);
            nrecs_on_disk = *reinterpret_cast<uint16_t*>(&footer[8]);
            nrecs = *reinterpret_cast<uint16_t*>(&footer[10]);
            global_header_size = *reinterpret_cast<uint32_t*>(&footer[12]);
            global_header_offset = *reinterpret_cast<uint32_t*>(&footer[16]);
            comment_len = *reinterpret_cast<uint16_t*>(&footer[20]);

            assert(disk_no == 0);
            assert(disk_start == 0);
            assert(nrecs_on_disk == nrecs);
            assert(comment_len == 0);
        }
    }

    namespace detail
    {
        inline uint16_t msdos_time(uint16_t hour, uint16_t min, uint16_t sec)
        {
            return static_cast<uint16_t>(hour << 11 | min << 5 | (sec / 2));
        }

        inline uint16_t msdos_date(uint16_t year, uint16_t month, uint16_t day)
        {
            return static_cast<uint16_t>((year - 1980) << 9 | month << 5 | day);
        }

        inline std::pair<uint16_t, uint16_t> time_pair()
        {
            std::time_t t = std::time(nullptr);
            auto tm = std::localtime(&t);
            return {msdos_time(static_cast<uint16_t>(tm->tm_hour), static_cast<uint16_t>(tm->tm_min), static_cast<uint16_t>(tm->tm_sec)),
                    msdos_date(static_cast<uint16_t>(tm->tm_year + 1900), static_cast<uint16_t>(tm->tm_mon + 1), static_cast<uint16_t>(tm->tm_mday))};
        }
    }

    /**
     * Save a xarray or xtensor to a NPZ file.
     * If a npz file with ``filename`` exists already, the new data
     * is appended to the existing file by default. Note: currently no checking
     * of name-collision is performed!
     *
     * @param filename filename to save to
     * @param varname desired name of the variable
     * @param e xexpression to save
     * @param compression true enables compression, otherwise store uncompressed (default false)
     * @param append_to_existing_file If true, appends new data to existing file
     */
    template <class E>
    void dump_npz(std::string filename,
                  std::string varname, const xt::xexpression<E>& e,
                  bool compression = false,
                  bool append_to_existing_file = true)
    {
        //first, append a .npy to the fname
        varname += ".npy";

        //now, on with the show
        std::ifstream in_stream(filename, std::ios_base::in | std::ios_base::binary);

        uint16_t nrecs = 0;
        std::streamoff global_header_offset = 0;
        detail::binary_vector global_header;

        if (in_stream && append_to_existing_file)
        {
            // zip file exists. we need to add a new npy file to it.
            // first read the footer. this gives us the offset and size
            // of the global header then read and store the global header.
            // below, we will write the the new data at the start of the
            // global header then append the global header and footer below it
            std::streamsize global_header_size;
            detail::parse_zip_footer(in_stream, nrecs, global_header_size, global_header_offset);
            in_stream.seekg(global_header_offset);
            global_header.resize(static_cast<std::size_t>(global_header_size));
            in_stream.read(&global_header[0], global_header_size);
            if (!in_stream)
            {
                throw std::runtime_error("dump_npz: header read error while adding to existing zip");
            }
        }

        std::ofstream stream;
        if (append_to_existing_file && global_header_offset)
        {
            stream = std::ofstream(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::in);
            stream.seekp(global_header_offset);
        }
        else
        {
            stream = std::ofstream(filename, std::ios_base::out | std::ios_base::binary);
        }

        detail::binary_vector array_contents;
        detail::dump_npy_stream(array_contents, e);

        //get the CRC of the data to be added
        uint32_t crc = static_cast<uint32_t>(crc32(0L, reinterpret_cast<uint8_t*>(&array_contents[0]), static_cast<unsigned int>(array_contents.size())));
        std::size_t uncompressed_size = array_contents.size();
        std::size_t compressed_size = uncompressed_size;

        uint16_t compression_method = 0;

        if (compression)
        {
            std::ostringstream ss;
            compression_method = 8;
            zstr::ostream compressed_array(ss, compression_method);
            compressed_array.write(&array_contents[0], static_cast<std::streamsize>(array_contents.size()));
            compressed_array.flush();
            array_contents.clear();
            array_contents << ss.str();
            compressed_size = array_contents.size();
        }
        auto date_time_pair = detail::time_pair();

        //build the local header
        detail::binary_vector local_header;
        local_header << "PK"                // first part of signature
                     << (uint16_t) 0x0403   // second part of signature
                     << (uint16_t) 20       // minimum version to extract
                     << (uint16_t) 0        // general purpose bit flag
                     << (uint16_t) compression_method    // compression method
                     << (uint16_t) date_time_pair.first  // file last mod time
                     << (uint16_t) date_time_pair.second // file last mod date
                     << (uint32_t) crc      // checksum
                     << (uint32_t) compressed_size     // compressed size
                     << (uint32_t) uncompressed_size   // uncompressed size
                     << (uint16_t) varname.size() // file name length
                     << (uint16_t) 0        // extra field size
                     << varname;            // file name

        //build global header
        global_header << "PK"               // first part of sig
                      << (uint16_t) 0x0201  // second part of sig
                      << (uint16_t) 0x0314; // version made by .. hardcoded to what NumPy uses here
                                            // (not sure if this depends on zlib version)

        // Copy from local header
        global_header.insert(global_header.end(),
                             local_header.begin() + 4,
                             local_header.begin() + 30);

        global_header << (uint16_t) 0 // file comment length
                      << (uint16_t) 0 // disk number where file starts
                      << (uint16_t) 0 // internal file attributes
                      << (uint32_t) 0x81800000 // external file attributes (taken from numpy)
                      << (uint32_t) global_header_offset // relative offset of local file header,
                                                         // since it begins where the global header used to begin
                      << varname;

        // build footer
        detail::binary_vector footer;
        footer << "PK"              // first part of sig
               << (uint16_t) 0x0605 // second part of sig
               << (uint16_t) 0      // number of this disk
               << (uint16_t) 0      // disk where footer starts
               << (uint16_t) (nrecs + 1) // number of records on this disk
               << (uint16_t) (nrecs + 1) // total number of records
               << (uint32_t) global_header.size() // nbytes of global headers
               // offset of start of global headers, since global header now starts after newly written array
               << (uint32_t) ((std::size_t) global_header_offset + compressed_size + local_header.size())
               << (uint16_t) 0;     // zip file comment length

        // write everything
        stream.write(&local_header[0], (std::streamsize) local_header.size());
        stream.write(&array_contents[0], (std::streamsize) array_contents.size());
        stream.write(&global_header[0], (std::streamsize) global_header.size());
        stream.write(&footer[0], (std::streamsize) footer.size());
    }

}  // namespace xt

#endif
