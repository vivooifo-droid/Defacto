[BITS 32]
[ORG 0x1000]

global _start
_start:

__defacto_drv_keyboard:
    ; Built-in keyboard driver
    pushad
    call _init_keyboard
    popad
    ret
    mov esi, dword [var_msg]
    mov edi, dword [__defacto_cursor]
disp0_loop:
    movzx eax, byte [esi]
    test al, al
    jz disp0_done
    cmp al, 10
    je disp0_nl
    mov byte [0xB8000 + edi], al
    mov bl, byte [__defacto_attr]
    mov byte [0xB8000 + edi + 1], bl
    add edi, 2
    add esi, 1
    jmp disp0_loop
disp0_nl:
    xor edx, edx
    mov eax, edi
    mov ecx, 160
    div ecx
    inc eax
    imul eax, eax, 160
    mov edi, eax
    add esi, 1
    jmp disp0_loop
disp0_done:
    mov dword [__defacto_cursor], edi
    call __defacto_drv_keyboard
    mov esi, dword [var_key]
    mov edi, dword [__defacto_cursor]
disp1_loop:
    movzx eax, byte [esi]
    test al, al
    jz disp1_done
    cmp al, 10
    je disp1_nl
    mov byte [0xB8000 + edi], al
    mov bl, byte [__defacto_attr]
    mov byte [0xB8000 + edi + 1], bl
    add edi, 2
    add esi, 1
    jmp disp1_loop
disp1_nl:
    xor edx, edx
    mov eax, edi
    mov ecx, 160
    div ecx
    inc eax
    imul eax, eax, 160
    mov edi, eax
    add esi, 1
    jmp disp1_loop
disp1_done:
    mov dword [__defacto_cursor], edi
    ; auto-free: key
    ; auto-free: msg

.hang:
    cli
    hlt
    jmp .hang

; Built-in driver stubs
_init_keyboard:
    ret
_init_mouse:
    ret
_init_speaker:
    ret

__defacto_cursor: dd 0
__defacto_attr: db 15
    var_key: dd 0
    str_0: db 80, 114, 101, 115, 115, 32, 97, 32, 107, 101, 121, 0
    var_msg: dd str_0

