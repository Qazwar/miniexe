# miniexe
A way to creating small executables with Microsoft Visual Studio,without import directory,remove the c run time library initialization code,but can dynamic call  C standard library from msvcrt.dll.

All windows api and c api dynamic load from dll,include `Loadlibrary`,so we can building a exe without import directory.
## 1，how to work?
```
1,get kernel32.dll base from PEB
        |
        |
2,get LoadLibraryA,... api address from kernel32.dll base
        |
        |
3,load user32.dll and msvcrt.dll
        |
        |
4,get others api by loaded mod's export table
        |
        |
5,spec our entry point,do not link default c run time(/NODEFAULTLIB)
```

## 2,how to spec entry point?
realize mainCRTstartup function,by [miniCRT](https://github.com/flydom/MiniCRT)

## 3,how to build?
```
First install cmake, git clone ...
cd .../src
mkdir build
cmake ../
vs2017 build release
```
On my pc,size of exe only 2560Bytes(2.5KB),it could be smaller, no run time dependence,all dlls are windows original.

![PE info](https://raw.githubusercontent.com/p00s/miniexe/master/snipaste/snipaste_20171231_182707.png)

