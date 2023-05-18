#ifndef CUSTOM_FILE_LIBRARY_DEFS
#define CUSTOM_FILE_LIBRARY_DEFS

#define FILE_NAMESPACE_BEGIN namespace MmapFiles {
#define FILE_NAMESPACE_END   }

enum class Errors : int
{
    no_error = 0, // okay, no errors
    inavlid_args, // arguements to function were invalid
    system,       // system call failed with error
    cant_lock,    // cannot obtain lock
    not_found,    // requested hash/key value not found
    cant_get,     // cannot get the resource
    full          // resource cannot have more data added
};

#endif
