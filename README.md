﻿### Overview

Hakkapeliitta is my UCI-compabtile chess engine written in C++11/14. Its rating
is around 2775 elo and has a highly tactical and entertaining playing style. It 
also supports Syzygy tablebases.

### UCI-parameters:

 - Hash: set the amount of memory the main transposition table can use (in MB).
 - PawnHash: set the amount of memory the pawn hash table can use (in MB).
 - Contempt: positive values make Hakkapeliitta avoid draws, negative values make it prefer them.
 - SyzygyProbeLimit: values 0, 1 and 2 mean that tablebases aren't probed, value 3 means that 3-man tablebases are probed etc. On a 32-bit machine SyzygyProbeLimit can only be set at maximum to 5.
 - SyzygyPath: set the path to the Syzygy tablebases. Multiple directories are possible if they are separated by ";" on Windows and ":" on Linux. Do not use spaces around the ";" or ":".

### Compiling it yourself

Hakkapeliitta can be compiled from the source code. No makefiles are provided 
as I am unfamiliar with them. Supported platforms are Windows and Linux.
Support for Macintosh isn't planned due to me not having a computer with 
Macintosh. Supported compilers are MSVC 12.0 (or higher), GCC 4.9.0 (or higher) 
and Clang 3.4 (or higher). 

### Acknowledgements	

Thanks to the following people (or organizations) my engine is what it is today.
If you think I have forgotten somebody/something please let me know.

 - Chess Programming Wiki
 - Talkchess
 - Glaurung, Tord Romstad
 - Ronald de Man 
 - Stockfish, Tord Romstad, Marco Costalba, and Joona Kiiski
 - Crafty, Robert Hyatt
 - Texel, Peter Österlund
 - Ivanhoe, ??? 
 - Inside Prodeo, Ed Schroeder
 - Winglet, Stef Luijten
 - Maverick, Steve Maughan
 - TSCP, Tom Kerrigan 

 