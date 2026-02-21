[BITS 32]
[ORG 0x1000]

global _start
_start:
    mov esi, dword [var_hello]
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
    ; auto-free: hello

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
    str_0: db 208, 159, 209, 128, 208, 178, 208, 184, 208, 181, 209, 130, 44, 32, 208, 156, 208, 184, 209, 128, 33, 0
    var_hello: dd str_0

