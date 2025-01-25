To link library.dll to your project, you need to add the following line to your project file:

gendef library.dll
dlltool -d library.def -l library.dll.a