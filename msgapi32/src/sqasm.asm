IFNDEF __FLAT__

        .model large, pascal
        .code

        public FARWRITE, FARREAD, SHARELOADED

; int far pascal farread(int handle,char far *buf,unsigned int len)
; int far pascal farwrite(int handle,char far *buf,unsigned int len)
; int far pascal shareloaded(void);

FARREAD proc ;uses ds ; , handle:word, buf:dword, len:word
        push    bp
        mov     bp,sp
        push    ds
        
        mov     bx, word ptr [bp+12]    ; Load file handle
        
        mov     cx, word ptr [bp+6]     ; Load length

        mov     ax,word ptr [bp+10]     ; Load segment into AX (and then DS)
        mov     dx,word ptr [bp+8]      ; Load offset into DX

        mov     ds,ax                   ; Move AX (the segment) into DS

        mov     ah,3fh                  ; Do it
        int     21h
        jnc     okay

        mov     ax, -1
okay:
        pop     ds
        pop     bp
        ret     8
FARREAD endp

FARWRITE proc ; uses ds, handle:word, buf:dword, len:word
        push    bp
        mov     bp,sp
        push    ds
        
        mov     bx, [bp+12]             ; Load file handle
        mov     cx, [bp+6]              ; Length of write

        mov     ax,word ptr [bp+10]     ; Load segment into DS

        mov     dx,word ptr [bp+8]      ; Load offset into DX

        mov     ds,ax                   ; Move AX (the segment) into DS

        mov     ah,40h                  ; Call it
        int     21h
        jnc     doneit                  ; Return # of bytes if no error

        mov     ax,-1                   ; Otherwise, return -1

doneit:
        pop     ds
        pop     bp
        ret     8
FARWRITE endp

SHARELOADED proc

        mov     ax,1000h                ; Check for SHARE.EXE installation
        int     2fh                     ; DOS multiplexer interrupt
        cmp     al,0ffh                 ; ffh = SHARE loaded
        je      GetOut

        xor     ax,ax
GetOut: ret

SHARELOADED endp

ELSE            ; __FLAT__

        .386p
        .model small, pascal
        .code

        public FARWRITE, FARREAD, SHARELOADED

; int far pascal farread(int handle,char far *buf,unsigned int len)
; int far pascal farwrite(int handle,char far *buf,unsigned int len)
; int far pascal shareloaded(void);

FARREAD proc
        mov     ecx,[esp+4]             ; Load length

        mov     edx, [esp+8]            ; Load offset
        mov     ebx, [esp+12]           ; Load file handle

        mov     ah,3fh                  ; Do it
        int     21h
        jnc     okay

        mov     eax, -1
okay:   ret 0ch
FARREAD endp

FARWRITE proc
        mov     ebx,[esp+12]            ; Load file handle
        mov     ecx,[esp+4]             ; Length of write

        mov     edx,[esp+8]             ; Load offset into DX

        mov     ah,40h                  ; Call it
        int     21h
        jnc     doneit                  ; Return # of bytes if no error

        mov     eax,-1                  ; Otherwise, return -1

doneit: ret 0ch
FARWRITE endp

SHARELOADED proc
        mov     eax,1000h               ; Check for SHARE.EXE installation
        int     2fh                     ; DOS multiplexer interrupt
        cmp     al,0ffh                 ; ffh = SHARE loaded
        je      GetOut

        xor     ax,ax
GetOut: ret

SHARELOADED endp


ENDIF

end

