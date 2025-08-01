

# Library Documentation

To link library.dll your need to generate a definition file and an import library. This can be done using the `gendef` and `dlltool` commands.

## Generating the Definition File

First extracts the exported symbols from the DLL and creates a `.def` file. Suppose MingGW is installed and available in your PATH, you can run the following command in your terminal:

```
$ gendef hydrasdr.dll
 * [library.dll] Found PE+ image
```

You should have a file named `hydrasdr.def` in the current directory. This file contains the exported symbols from the DLL.

## Generating the Import Library

Next, you need to create an import library from the `.def` file. You can do this using the `dlltool` command:

```
$ dlltool.exe -d hydrasdr.def -l hydrasdr.dll.a -v
dlltool.exe: Using file: C:\Develop\Qt\Tools\mingw1310_64\bin\as
dlltool.exe: Processing def file: hydrasdr.def
dlltool.exe: LIBRARY: hydrasdr.dll base: ffffffff
dlltool.exe: Processed def file
dlltool.exe: Processing definitions
dlltool.exe: Processed definitions
dlltool.exe: Creating library file: hydrasdr.dll.a
...
dlltool.exe: Created lib file
``` 

Now you should have a file named `hydrasdr.dll.a` in the current directory. This is the import library that you can link against when compiling your application.