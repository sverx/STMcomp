; ***** Sverx's TileMap decompressor                            *****
; ***** by sverx - https://github.com/sverx/STM_tilemap_decompr *****

;  to use it, you have to do:
;     ld hl,compressed_tilemap     ; - set source address
;     ld de,VRAMAddress|$4000      ; - set destination VRAM address, ORed with $4000
;     call STM_tilemap_decompr

.section "Sverx's TileMap decompressor" free
; HL = compressed tilemap binary (source)
; DE = VRAM address (destination) - ORed with $4000
; destroys AF,BC,DE,HL,IX
STM_tilemap_decompr:
  ld c,$bf                             ; set VRAM address
  out (c),e
  out (c),d
  ld c,$be                             ; set C to VDP data port
  ld d,$00                             ; sets HH to $00
  ld ix,$0000                          ; sets oldHH/flags to $0000
_MainLoop:
  ld a,ixl                             ; do we need to restore HH?
  or a
  jr z,+                               ; if flag is 0 then no restore
  ld d,ixh                             ; restore HH
  ld ixl,$0                            ; reset restore flag
_MainLoop_nocheck:
+:ld a,(hl)
  inc hl
  rra                                  ; is it a RLE run?
  jr c,_RLE
  rra                                  ; is a RAW run?
  jr nc,_RAW

  ; set new HH
  rra                                  ; is it a temporary set?
  jr nc,_noTemp
  ld ixh,d                             ; save old HH
  ld ixl,$1                            ; raise restore flag
_noTemp:
  and $1F                              ; keep only last 5 bits
  ld d,a                               ; set new HH
  jp _MainLoop_nocheck

_RLE:
  rra                                  ; is it a RLE run of same value bytes?
  jr c,_incrementalRLE
  and $3F                              ; keep only last 6 bits (counter)
  add a,2                              ; counter +=2
  ld b,a
  ld e,(hl)                            ; load value
  inc hl
-:out (c),e                            ; push (same) values to VRAM
  out (c),d
  djnz -
  jp _MainLoop

_RAW:
  and $3F                              ; keep only last 6 bits (counter)
  ret z                                ; if counter==0 then data is over, leave!
  ld b,a
-:ld e,(hl)                            ; load value
  inc hl
  out (c),e                            ; push value to VRAM
  out (c),d
  djnz -
  jp _MainLoop

_incrementalRLE:
  and $3F                              ; keep only last 6 bits (counter)
  add a,2                              ; counter +=2
  ld b,a
  ld e,(hl)                            ; load start value
  inc hl
-:out (c),e                            ; push incremental values to VRAM
  out (c),d
  inc de
  djnz -
  dec de                               ; this is to keep HH consistent with last value
  jp _MainLoop
.ends
