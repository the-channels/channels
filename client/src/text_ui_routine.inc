    ld a, (bc)
    or (hl)
    ld (de), a                      ; put it on screen

    inc bc
    inc hl