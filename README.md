# Schönemann
- [About](#about)
- [How it works](#how-it-works)
- [Build](#build)
- [Thanks](#thanks)

# About
This is a simple chess engine written in C++. The name was inspired by this chess line in the Ruy Lopez:
```
1. e4 e5 2. Nf3 Nc6 3. Bb5 f5 4. d4 fxe4 5. Nxe5 Nxe5 6. dxe5 c6 7. Nc3 *
```
in this it seems that white is hanging a bishop but is not worst.
It has currently a strenght about ~2660 Elo (Tested againtst Laser 1.7 STC)

# How it works
-  NNUE with 512 Hidden Layers
-  Trained with Leela Data

-  PVS-Search
-  SEE Prunning + MoveOrdering
-  Check Extension
-  MVA - LVV in MoveOrdering

# Build
Type 
  ```
  make all
  ```

#  Thanks
Special thanks to Disservin's [chess libary](https://github.com/Disservin/chess-library/) witch made the wohle development process simpler.
