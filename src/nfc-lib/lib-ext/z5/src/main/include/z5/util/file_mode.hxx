#pragma once

#include <array>
#include "z5/common.hxx"

namespace z5 {

    class FileMode {
    public:
        // the individual options
        static const char can_write = 1;
        static const char can_create = 2;
        static const char must_not_exist = 4;
        static const char should_truncate = 8;
        // the (python / h5py) I/O modes:
        // r: can only read, file must exist
        // r+: can read and write, file must exist
        // w: can read and write, if file exists, will be overwritten
        // w-: can read and write, file must not exist
        // x: can read and write, file must not exist (same as w- ?!, so omitted here)
        // a: can read and write
        enum modes {r   = 0,
                    r_p = can_write,
                    w   = can_write | should_truncate | can_create,
                    w_m = can_write | can_create | must_not_exist,
                    a   = can_write | can_create};

        FileMode(const FileMode::modes mode = FileMode::a) : mode_(mode)
        {}

        inline bool canWrite() const {return mode_ & can_write;}
        inline bool canCreate() const {return mode_ & can_create;}
        inline bool mustNotExist() const {return mode_ & must_not_exist;}
        inline bool shouldTruncate() const {return mode_ & should_truncate;}

        inline std::string printMode() const {
            switch(mode_) {
                case FileMode::r: return "r";
                case FileMode::r_p: return "r+";
                case FileMode::w: return "w";
                case FileMode::w_m: return "w-";
                case FileMode::a: return "a";
                default: throw std::runtime_error("Invalid file mode");
            }
        }

        inline modes mode() const {return mode_;}

    private:
        FileMode::modes mode_;
    };


}
