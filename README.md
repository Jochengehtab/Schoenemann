# Schönemann
- [About](#about)
- [How it works](#How_it_works)
- [Install](#install)

# About
This is a simple (currently not functional and not UCI-Compatible) chess engine written in C++. The name was inspired by this chess line in the ruy lopez:
```
1. e4 e5 2. Nf3 Nc6 3. Bb5 f5 4. d4 fxe4 5. Nxe5 Nxe5 6. dxe5 c6 7. Nc3 *
```
in this it seems that white is hanging a bishop but is not worst.

# How it works
Currently it is counting the piece values up for each side and uses a simple Alpha-Beta Search.

# Install
There are currently two ways:
1. CMake <br>
  Download CMake from the offical website and execute the following commands:
  ```
  cmake .
  cmake --build . --config Release
  ```  
2. Makefile <br>
  You need Python 3.12 (maybe older versions work as well but i haven't tested it yet) and MingGW. Then simply type the following command:
  ```
  make EXE=Schoenemann.exe
  ```
  It is currently this way because of OpenBench. I am open for a better soulution.

