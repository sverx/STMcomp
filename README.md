STMcomp
=====================

Sverx's TileMap compressed format Bmp2Tile plugin and Z80 decompression routine



Format details: (everything is stored as run of bytes)

first byte -- other bytes that may be following
%nnnnnn01  -- dd : run of n+2 (2 to 65) same word value HHdd (a.k.a. "normal" RLE)
%nnnnnn11  -- dd : run of n+2 (2 to 65) successive word values (HHdd, HHdd+1, HHdd+2 ...)
                 : will eventually change HH too!
%nnnnnn00  -- dd kk pp [...] : run of n (1 to 63) words using each of the following bytes (HHdd, HHkk, HHpp...) 
                             : n=0 means end of compressed data
%hhhhht10   : set new HH to %000hhhhh 
      t = 1 : set the value temporarily (revert to previous value after next run) 
      t = 0 : set it permanently
(HH is initialized to $00 at launch, both on compressor and on decompressor)
