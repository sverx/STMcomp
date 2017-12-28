; ***** ShrunkTileMap (previously known as "Sverx's TileMap") decompressor       *****
; ***** by sverx - https://github.com/sverx/STMcomp/tree/master/decompressor/src *****
; *****                                        (now VRAM / Interrupt *SAFE* !!!) *****
; ***** (note: code backported from devkitSMS)                                   *****

; ************************************************************************************
;  to use this function, you have to do this:
;     ld hl,STMcompr_tilemap       ; load address of STM data into HL
;     ld de,VRAMAddress            ; set destination VRAM address into DE
;     call STM_tilemap_decompr
; the function needs 9 bytes of RAM to work, see ramsection at the end of this file
; ************************************************************************************

.section "ShortTileMap decompressor" free
; HL = compressed tilemap (source)
; DE = VRAM address (destination)
; destroys AF,BC,DE,HL
STM_tilemap_decompr:
        ld a,$40
        or d
        ld d,a
        ld (STM_dest),de
        xor a
        ld (STM_HH),a
        ld (STM_oldHH),a
        ld (STM_needRestore),a
        ld a,(hl)
        ld (STM_width),a
        ld (STM_ttw),a
        inc hl
        ld (STM_src),hl
        ld hl,(STM_dest)
      	ld c, $BF                     ; set VDP Control Port
	di                            ; make it interrupt SAFE
	out (c),l
        out (c),h
        ei
_STM_loop:
        ld        hl,(STM_src)
        ld        a,(hl)
        inc        hl
        ld        (STM_src),hl
        rra
        jr        nc,_STM_noRLE
        rra
        jr        nc,_STM_RLE
; ***************** RLE of successive words
        and        $3F      ; keep only last 6 bits (counter)
        add        a,$02
        ld        b,a ; b = cnt
        ld        a,(STM_HH)
        ld        d,a ; d = high part of tile
        ld        e,(hl) ; e = low part of tile
        inc        hl
        ld        (STM_src),hl ; src++
        ld        hl,STM_ttw
        ld        c,(hl) ; c = ttw
_STM_inc_RLE_loop:
        ld        a,e ; write tile to VRAM respecting access time costraints
        out        ($BE),a ; 11
        ld        a,d ; 4
        sub        $00 ; *delay* 7
        dec        c ; ttw-- 4 = 26 (VRAM SAFE)
        out        ($BE),a
        call        z,_STM_setAddr ; ttw==0?
        dec        b
        jr        z,_STM_inc_RLE_done ; cnt==0?
        inc        de ; tile++
        jr        _STM_inc_RLE_loop
_STM_inc_RLE_done:
        ld        hl,STM_HH ; save high part of tile
        ld        (hl),d
        ld        hl,STM_ttw ; save ttw
        ld        (hl),c
_STM_check_restore:
        ld        a,(STM_needRestore)
        or        a
        jr        z,_STM_loop
        xor        a
        ld        (STM_needRestore),a
        ld        a,(STM_oldHH)
        ld        (STM_HH),a
        jr        _STM_loop
_STM_noRLE:
        rra
        jr        nc,_STM_RAW
; ***************** special
        rra
        jr        nc,_STM_no_temp_HH
        ld        b,a
        ld        a,(STM_HH)
        ld        (STM_oldHH),a
        ld        a,$01
        ld        (STM_needRestore),a
        ld        a,b
_STM_no_temp_HH:
        and        $1F ; keep only last 5 bits
        ld        (STM_HH),a
        jr        _STM_loop
_STM_RLE:
; ***************** RLE
        and        $3F ; keep only last 6 bits (counter)
        add        a,$02
        ld        b,a ; b = cnt
        ld        a,(STM_HH)
        ld        d,a ; d = high part of tile
        ld        e,(hl) ; e = low part of tile
        inc        hl
        ld        (STM_src),hl ; src++
        ld        hl,STM_ttw
        ld        c,(hl) ; c = ttw
_STM_RLE_loop:
        ld        a,e ; write tile to VRAM respecting access time costraints
        out        ($BE),a ; 11
        ld        a,d ; 4
        sub        $00 ; *delay* 7
        dec        c ; ttw-- 4 = 26 (VRAM SAFE)
        out        ($BE),a
        call        z,_STM_setAddr ; ttw==0?
        dec        b
        jr        nz,_STM_RLE_loop ; cnt==0?
        ld        hl,STM_ttw ; save ttw
        ld        (hl),c
        jr        _STM_check_restore
_STM_RAW:
; ***************** RAW
        and        $3F ; keep only last 6 bits (counter)
        or        a
        ret       z  ; if cnt is 0 - end of STM
        ld        b,a ; b = cnt
        ld        hl,STM_ttw
        ld        c,(hl) ; c = ttw
_STM_RAW_loop:
        ld        hl,(STM_src)
        ld        a,(hl)
        out        ($BE),a ; 11 low part of tile
        inc        hl ; 6
        ld        (STM_src),hl ; 16 src++
        ld        a,(STM_HH) ; 13 high part of tile
        out        ($BE),a ; xx = VRAM safe
        dec        c
        call        z,_STM_setAddr
        dec        b
        jr        nz,_STM_RAW_loop
        ld        hl,STM_ttw
        ld        (hl),c
        jr        _STM_check_restore
_STM_setAddr:
        ld        hl,(STM_dest)
        ld        a,b ; preserve b
        ld        bc,64
        add        hl,bc
        ld        b,a ; restore b
        ld        (STM_dest),hl
        ld        c,$BF ; set VDP Control Port
        di        ; make it interrupt SAFE
        out        (c),l
        out        (c),h
        ei
        ld        a,(STM_width)
        ld        (STM_ttw),a
        ld        c,a
        ret
.ends

.ramsection "STM variables" slot 3
  STM_src           dw
  STM_dest          dw
  STM_HH            db
  STM_oldHH         db
  STM_needRestore   db
  STM_ttw           db
  STM_width         db
.ends
