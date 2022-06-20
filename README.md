# CRC32
Command line utility to calculate standard CRC32 for your files

You can build it from MS Visual Studio command prompt. You can use any version of VS for 32 or 64 bits Windows.
It can start from Windows 95 and end with Windows 11 64 bits or above with Visual Studio 2022 or above.
File clexe.bat provided and you can build csum.exe from csum.cpp.

Below presented build and test run of CRC32 csum in Visual Studio command prompt:
```
**********************************************************************
** Visual Studio 2022 Developer Command Prompt v17.2.4
** HostCPU=AMD64 Tools=x64 Target=x64
** Copyright (c) 2022 Microsoft Corporation
**********************************************************************

D:\My>cd proj\tr

D:\My\Proj\tr>clexe csum.cpp
Microsoft (R) C/C++ Optimizing Compiler Version 19.32.31329 for x64
Copyright (C) Microsoft Corporation.  All rights reserved.

csum.cpp
Microsoft (R) Incremental Linker Version 14.32.31329.0
Copyright (C) Microsoft Corporation.  All rights reserved.

/out:csum.exe
/MAP
csum.obj
D:\My\Proj\tr>csum csum.*
CRC32        Length  Filename:
------------------------------
D3074AA2      5845 csum.cpp
FE0ED7C7     13312 csum.exe
237EB5A1     33787 csum.map
C93BC684      7985 csum.obj
------------------------------
C74CEE40     60929 bytes in 4 file[s]

D:\My\Proj\tr>
```

