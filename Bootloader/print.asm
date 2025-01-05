welcomeMsg      db "Welcome to CmOS", 0


; called with the string in si
; or ch = 0xFF to show the welcome message
printString:
    pusha

    cmp     ch, 0xFF
    jne     printLoop

    mov     si, welcomeMsg 

printLoop:

    lodsb

    ;checks for null terminator
    or      al, al
    jz      exitPrint

    mov     ah, 0x0E
    int     0x10

    jmp     printLoop

exitPrint:
    popa
    ret
