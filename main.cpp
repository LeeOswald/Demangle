#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string_view>

#include <cxxabi.h>

enum class ExitCode
{
    Success = 0,
    Help = 1,
    OutOfMemory = 2,
    InvalidName = 3,
    InvalidArg = 4,
    OtherFailure = 5
};


static int print_help()
{
    std::cout << "Demangle a single symbol:\n";
    std::cout << "demangle _ZN12SignalReader10onActivityEPN4maux6WaiterEi\n";

    return int(ExitCode::Help);
}

struct Deleter
{
    void operator()(void* p) noexcept
    {
        ::free(p);
    }
};

using Buffer = std::unique_ptr<char, Deleter>;

static int demangle_one(const char* what)
{
    size_t length = 0;
    int error = 0;
    Buffer out(abi::__cxa_demangle(what, nullptr, &length, &error));

    switch (error)
    {
    case 0:
    {
        std::string_view s(out.get(), std::min(length, std::strlen(out.get())));
        std::cout << s << std::endl;
        return int(ExitCode::Success);
    }
    case -1:
        std::cout << "<out of memory>\n";
        return int(ExitCode::OutOfMemory);
    case -2:
        std::cout << "<invalid name>\n";
        return int(ExitCode::InvalidName);
    case -3:
        std::cout << "<invalid arg>\n";
        return int(ExitCode::InvalidArg);
    default:
        std::cout << "<\?\?\?>\n";
        return int(ExitCode::OtherFailure);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
         std::cout << "Nothing to do. Use --help for help.\n";
         return int(ExitCode::Help);
    }

    if ((argc == 2) && (std::strstr(argv[1], "--") != argv[1]))
    {
        return demangle_one(argv[1]);
    }

    for (int i = 1; i < argc; ++i)
    {
        if (!std::strcmp(argv[i], "--help"))
        {
            return print_help();
        }
        else if (std::strstr(argv[i], "--") == argv[i])
        {
            std::cout << "Unknwn option " << argv[i] << ". Use --help for help.\n";
            return int(ExitCode::Help);
        }
    }

    return int(ExitCode::Success);
}
