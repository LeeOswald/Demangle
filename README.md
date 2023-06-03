# Demangle
C++ (GCC) demangler command line utility
========================================

Can demangle either a single given symbol or all symbols in a file. At the moment, only C++ GCC mangling scheme is supported. Libc-based system is required.


Building
--------

Build it as you would biuld any CMake-based project:

```bash
mkdir demangle-build
cd demangle-build
cmake ../demangle-master
cmake . --build
```

A single binary is produced.


Usage
-----

You can demangle a single symbol:

```bash
$ demangle _ZN12SignalReader10onActivityEPN4maux6WaiterEi
```

```bash
$ SignalReader::onActivity(maux::Waiter*, int)
```

demangle process will set the appropriate exit code you can analyze in a shell script:  
0 - success  
1 - help message printed  
2 - out of memory  
3 - invalid symbol name  
4 - invalid argument  
5 - unknown error  
6 - failed to open input file  
7 - failed to create output file  
  
Or, you can demangle all symbols in a given file:

```bash
$ demangle [--brackets] --file source.txt dest.txt
```

Use '--brackets' option to surround the demangled symbols with [].
Lines that contain no symbols or un-demangable symbols are left intact.
