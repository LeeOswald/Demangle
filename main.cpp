#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string_view>

#include <cxxabi.h>

enum class ExitCode
{
    Success = 0,
    Help = 1,
    OutOfMemory = 2,
    InvalidName = 3,
    InvalidArg = 4,
    OtherFailure = 5,
    NoInputFile = 6,
    NoOutputFile = 7
};


static int print_help()
{
    std::cout << "Demangle a single symbol:\n";
    std::cout << "$demangle _ZN12SignalReader10onActivityEPN4maux6WaiterEi\n";
    std::cout << "or, demangle all symbols ion a file:\n";
    std::cout << "$demangle [--brackets] --file source.log destination.log\n";
    std::cout << "--brackets: surround demangled names with square brackets\n";
    std::cout << "--file:     process all mangled names in a file\n";
    std::cout << "demangle will set the appropriate exit code:\n";
    std::cout << "0 - success\n";
    std::cout << "1 - this message printed\n";
    std::cout << "2 - out of memory\n";
    std::cout << "3 - invalid symbol name\n";
    std::cout << "4 - invalid argument\n";
    std::cout << "5 - unknown error\n";
    std::cout << "6 - failed to open input file\n";
    std::cout << "7 - failed to create output file\n";

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


std::string demangle_internal(const std::string& s)
{
    size_t length = 0;
    int error = 0;
    Buffer out(abi::__cxa_demangle(s.c_str(), nullptr, &length, &error));
    switch (error)
    {
    case 0:
        return std::string(out.get(), std::min(length, std::strlen(out.get())));

    default:
        return s;
    }
}

static std::string demangle_string(const std::string& s, bool brackets)
{
    std::ostringstream result;

    char prev = 0;
    bool in_name = false;
    std::ostringstream current;

    for (auto c = s.begin(); c != s.end(); ++c)
    {
        if (in_name)
        {
            // maybe we're inside a mangled name
            if ((*c == '_') || std::isalnum(*c))
            {
                current << *c;
            }
            else
            {
                // maybe end of name: try to demangle it
                auto demangled = demangle_internal(current.str());

                if (brackets)
                    result << "[";
                result << demangled;
                if (brackets)
                    result << "]";

                in_name = false;
                current = std::ostringstream();
            }
        }
        else
        {
            // not in a mangled name
            if (!prev)
            {
                // mahnged name not yet started
                if (*c == '_')
                {
                    // maybe a mangled name starts
                    prev = *c;
                }
                else
                {
                    // not in mangled name
                    result << *c;
                }
            }
            else
            {
                // we already have '_'
                current << prev << *c;
                prev = 0;

                if (*c == 'Z')
                {
                    // maybe a mangled name
                    in_name = true;
                }
                else
                {
                    // no that was not a mangled name
                    result << current.str();
                    current.clear();
                }
            }
        }
    }

    if (in_name)
    {
        // maybe end of name: try to demangle it
        auto demangled = demangle_internal(current.str());

        if (brackets)
            result << "[";
        result << demangled;
        if (brackets)
            result << "]";
    }

    return result.str();
}

static int demangle_file(std::ifstream& in, std::ofstream& out, bool brackets)
{
    std::string s;
    while (std::getline(in, s))
    {
        auto demangled = demangle_string(s, brackets);
        out << demangled << std::endl;
    }

    return int(ExitCode::Success);
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

    bool brackets = false;
    const char* in_file = nullptr;
    bool in_file_expected = false;
    const char* out_file = nullptr;
    bool out_file_expected = false;

    for (int i = 1; i < argc; ++i)
    {
        if (!std::strcmp(argv[i], "--help"))
        {
            return print_help();
        }
        else if (!std::strcmp(argv[i], "--brackets"))
        {
            brackets = true;
        }
        else if (!std::strcmp(argv[i], "--file"))
        {
            in_file_expected = true;
        }
        else if (std::strstr(argv[i], "--") == argv[i])
        {
            std::cout << "Unknwn option " << argv[i] << ". Use --help for help.\n";
            return int(ExitCode::Help);
        }
        else if (in_file_expected)
        {
            in_file = argv[i];
            in_file_expected = false;
            out_file_expected = true;
        }
        else if (out_file_expected)
        {
            out_file = argv[i];
            out_file_expected = false;
        }
    }

    if (!in_file)
    {
        std::cout << "Source file name expected. Use --help for help.\n";
        return int(ExitCode::Help);
    }

    if (!out_file)
    {
        std::cout << "Destination file name expected. Use --help for help.\n";
        return int(ExitCode::Help);
    }

    std::ifstream in(in_file);
    if (!in.good())
    {
        std::cout << "Failed to open " << in_file << "\n";
        return int(ExitCode::NoInputFile);
    }

    std::ofstream out(out_file);
    if (!out.good())
    {
        std::cout << "Failed to create " << out_file << "\n";
        return int(ExitCode::NoOutputFile);
    }

    return demangle_file(in, out, brackets);
}
