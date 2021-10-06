EXTERN _text_x
EXTERN _text_y
EXTERN asm_zx_cxy2saddr
PUBLIC _text_ui_write
EXTERN font_even_index
EXTERN font_odd_index

; stack: string to write
; stack: amount to write
; registers used:
;     iyl - number of characters left to write
;     de - current screen address
;     bc - current characted A data address
;     hl - current character B data address
;     ix - current string address
_text_ui_write:
    pop hl                          ; ret
    pop iy                          ; pop the amount into iyl
    pop ix                          ; pop string address into ix
    push hl                         ; ret

    ld a, (_text_x)                 ; get initial screen address
    ld l, a
    ld a, (_text_y)
    ld h, a
    call asm_zx_cxy2saddr

    ex de, hl                       ; de now holds a screen address

_text_ui_write_loop:
    ld a, (ix)
    add a, a
    ld (_text_ui_write_odd_get + 2), a
_text_ui_write_odd_get:
    ld bc, (font_odd_index+0)

    inc ix
    dec iyl
    jp nz, _text_ui_write_odd

    include "text_ui_routine_odd.inc"
    inc d
    include "text_ui_routine_odd.inc"
    inc d
    include "text_ui_routine_odd.inc"
    inc d
    include "text_ui_routine_odd.inc"
    inc d
    include "text_ui_routine_odd.inc"
    inc d
    include "text_ui_routine_odd.inc"
    inc d
    include "text_ui_routine_odd.inc"
    inc d
    include "text_ui_routine_odd.inc"

    ret

_text_ui_write_odd:
    ld a, (ix)
    add a, a
    ld (_text_ui_write_even_get + 1), a
_text_ui_write_even_get:
    ld hl, (font_even_index)

    inc ix

    ; now we hold the following
    ; de - current screen address
    ; bc - current characted A data address
    ; hl - current character B data address

    ; do ([d++]e) << (bc++) | (hl++) 8 times

    include "text_ui_routine.inc"
    inc d
    include "text_ui_routine.inc"
    inc d
    include "text_ui_routine.inc"
    inc d
    include "text_ui_routine.inc"
    inc d
    include "text_ui_routine.inc"
    inc d
    include "text_ui_routine.inc"
    inc d
    include "text_ui_routine.inc"
    inc d
    include "text_ui_routine.inc"

    inc e                           ; onto next screen address position (horisontally)
    ld a, d                         ; restore (h)l from 7 increments
    sub 7
    ld d, a

    dec iyl                         ; do we have more to print?
    ret z                           ; we're done

    jp _text_ui_write_loop
