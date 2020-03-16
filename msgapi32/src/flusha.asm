; Truly flush a file handle to disk

IFNDEF __FLAT__

        .model large, pascal
        .code

        PUBLIC FLUSH_HANDLE2

FLUSH_HANDLE2 proc 
        ; duplicate file handle, then close the duplicate.
        push    bp
        mov     bp,sp
        mov     ah,45h
        mov     bx, [bp+6]
        int     21h
        jc      Er
        mov     bx,ax
        mov     ah,3eh
        int     21h
Er:     pop     bp
        ret     2
FLUSH_HANDLE2 endp

ELSE            ; __FLAT__

        .386p
        .model small, pascal
        .code

        PUBLIC FLUSH_HANDLE2

FLUSH_HANDLE2 proc
        ; duplicate file handle, then close the duplicate.
        mov     ah,45h
        mov     ebx, [esp+4]
        int     21h
        jc      Er

        mov     ebx,eax
        mov     ah,3eh
        int     21h
Er:     ret 4
FLUSH_HANDLE2 endp

ENDIF

END

