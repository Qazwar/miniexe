# miniexe
visual studio building minimum size exe,without import directory

All windows api and c api dynamic load from dll.
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
On my pc,size of exe only 3kb,it could be smaller, no run time depend,all dlls are windows original.

