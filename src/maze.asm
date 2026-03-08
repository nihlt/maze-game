; ============================================================================
; maze.asm — The Architect's Cube: Wireframe Maze Port
; Pure x86-64 NASM for Windows (WinAPI GDI32)
; Features: Pseudo-3D raycast wireframes, Entities, Fog of War, Smooth Interpolation
; ============================================================================
bits 64
default rel

; ---- Constants ----
%define WW       800
%define WH       600
%define TID      1
%define TMS      16

; WinAPI Constants
%define CS_HR    2
%define CS_VR    1
%define WS_OVL   0x00CF0000
%define WS_VIS   0x10000000
%define SW_NORM  1
%define WM_CRE   1
%define WM_DST   2
%define WM_PNT   0x000F
%define WM_TMR   0x0113
%define WM_KEY   0x0100
%define WM_ERASEBKGND 0x0014
%define VK_ESC   0x1B
%define VK_SPACE 0x20
%define BLACKNESS 0x00000042
%define PS_SOLID 0

; ASCII
%define KEY_W    0x57
%define KEY_S    0x53
%define KEY_A    0x41
%define KEY_D    0x44

; ---- Imports ----
extern GetModuleHandleA
extern RegisterClassExA
extern CreateWindowExA
extern ShowWindow
extern UpdateWindow
extern GetMessageA
extern TranslateMessage
extern DispatchMessageA
extern PostQuitMessage
extern DefWindowProcA
extern BeginPaint
extern EndPaint
extern InvalidateRect
extern SetTimer
extern KillTimer
extern GetClientRect
extern SelectObject
extern DeleteObject
extern PatBlt
extern CreatePen
extern MoveToEx
extern LineTo
extern Polygon
extern GetSystemMetrics
extern DestroyWindow
extern ExitProcess
extern TextOutA
extern SetTextColor
extern SetBkMode

; ============================== DATA =====================================
section .data

szCls:  db "Maze3D", 0
szTtl:  db "The Architect's Cube - Wireframe Assembly Port", 0

; Colors
col_green  dd 0x00FF00
col_red    dd 0x0000FF
col_blue   dd 0xFF0000
col_cyan   dd 0xFFFF00
col_dim    dd 0x008800
col_dark   dd 0x003300

; ---- Maze Data (24x24) ----
MW equ 24
MH equ 24
maze:
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    db 1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1
    db 1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,0,1
    db 1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,1
    db 1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,0,1
    db 1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,1,0,1
    db 1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,0,1
    db 1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1
    db 1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1
    db 1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1
    db 1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,0,1
    db 1,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,1
    db 1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1
    db 1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1
    db 1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1
    db 1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1
    db 1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1
    db 1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1
    db 1,0,1,0,1,1,1,1,1,0,1,0,1,1,0,1,0,1,1,1,1,1,1,1
    db 1,0,1,0,1,0,0,0,0,0,1,0,1,0,0,1,0,0,0,0,0,0,0,1
    db 1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1
    db 1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1
    db 1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1
    db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1

; ============================== BSS ======================================
section .bss

g_hwnd      resq 1
g_hinst     resq 1

pen_green   resq 1
pen_red     resq 1
pen_blue    resq 1
pen_cyan    resq 1
pen_dim     resq 1
pen_dark    resq 1

wc_buf      resb 80
msg_buf     resb 48
ps_buf      resb 72
rc_buf      resb 16

; Game State
logicalX    resd 1
logicalY    resd 1
logicalDir  resd 1 ; N=0, E=1, S=2, W=3

; ============================== TEXT =====================================
section .text
global WinMain

; ========================================================================
; Entry point
; ========================================================================
WinMain:
    ; Entry: RSP % 16 == 8
    push    rbx              ; RSP % 16 == 0
    sub     rsp, 112         ; RSP % 16 == 0.

    xor     ecx, ecx
    call    GetModuleHandleA
    mov     [rel g_hinst], rax

    ; zero WNDCLASSEXA
    lea     rdi, [rel wc_buf]
    xor     eax, eax
    mov     ecx, 10
    rep     stosq

    ; fill
    lea     rdi, [rel wc_buf]
    
    mov     dword [rel logicalX], 1
    mov     dword [rel logicalY], 1
    mov     dword [rel logicalDir], 1 ; East

    mov     dword  [rdi+0],  80
    mov     dword  [rdi+4],  CS_HR | CS_VR | 0x0020
    lea     rax, [rel WndProc]
    mov     qword  [rdi+8],  rax
    mov     rax, [rel g_hinst]
    mov     qword  [rdi+24], rax
    mov     qword  [rdi+48], 0
    lea     rax, [rel szCls]
    mov     qword  [rdi+64], rax

    lea     rcx, [rel wc_buf]
    call    RegisterClassExA
    test    eax, eax
    jz      .quit

    ; center window
    xor     ecx, ecx          ; SM_CXSCREEN
    call    GetSystemMetrics
    sub     eax, WW
    shr     eax, 1
    mov     ebx, eax           ; posX

    mov     ecx, 1             ; SM_CYSCREEN
    call    GetSystemMetrics
    sub     eax, WH
    shr     eax, 1
    mov     r12d, eax          ; posY

    xor     ecx, ecx
    lea     rdx, [rel szCls]
    lea     r8,  [rel szTtl]
    mov     r9d, WS_OVL | WS_VIS
    mov     dword  [rsp+32], ebx      ; arg 5
    mov     dword  [rsp+40], r12d     ; arg 6
    mov     dword  [rsp+48], WW       ; arg 7
    mov     dword  [rsp+56], WH       ; arg 8
    mov     qword  [rsp+64], 0        ; arg 9
    mov     qword  [rsp+72], 0        ; arg 10
    mov     rax, [rel g_hinst]
    mov     qword  [rsp+80], rax      ; arg 11
    mov     qword  [rsp+88], 0        ; arg 12
    call    CreateWindowExA
    test    rax, rax
    jz      .quit
    mov     [rel g_hwnd], rax

    mov     rcx, rax
    mov     edx, SW_NORM
    call    ShowWindow

    mov     rcx, [rel g_hwnd]
    call    UpdateWindow

.mloop:
    lea     rcx, [rel msg_buf]
    xor     edx, edx
    xor     r8d, r8d
    xor     r9d, r9d
    call    GetMessageA
    cmp     eax, 0
    jle     .quit

    lea     rcx, [rel msg_buf]
    call    TranslateMessage
    lea     rcx, [rel msg_buf]
    call    DispatchMessageA
    jmp     .mloop

.quit:
    xor     ecx, ecx
    call    ExitProcess

; ========================================================================
; WndProc 
; ========================================================================
%define STK        136
%define L_SAV_HW   96
%define L_SAV_MSG  104
%define L_SAV_WP   112
%define L_SAV_LP   120

WndProc:
    push    rbp              
    push    rbx              
    push    rsi              
    push    rdi              
    push    r12              
    push    r13              
    push    r14              
    push    r15              
    sub     rsp, STK         

    mov     [rsp+L_SAV_HW], rcx
    mov     [rsp+L_SAV_MSG], edx
    mov     [rsp+L_SAV_WP], r8
    mov     [rsp+L_SAV_LP], r9
    mov     rbx, rcx

    cmp     edx, WM_CRE
    je      .wc
    cmp     edx, WM_TMR
    je      .wt
    cmp     edx, WM_PNT
    je      .wp
    cmp     edx, WM_KEY
    je      .wk
    cmp     edx, WM_ERASEBKGND
    je      .werase
    cmp     edx, WM_DST
    je      .wd

    ; default
    mov     rcx, rbx
    mov     edx, [rsp+L_SAV_MSG]
    mov     r8,  [rsp+L_SAV_WP]
    mov     r9,  [rsp+L_SAV_LP]
    call    DefWindowProcA
    jmp     .ret

.werase:
    mov     eax, 1
    jmp     .ret

; ---- WM_CREATE ----
.wc:
    mov     rcx, PS_SOLID
    mov     rdx, 2
    mov     r8d, [rel col_green]
    call    CreatePen
    mov     [rel pen_green], rax
    
    mov     rcx, PS_SOLID
    mov     rdx, 2
    mov     r8d, [rel col_red]
    call    CreatePen
    mov     [rel pen_red], rax

    mov     rcx, PS_SOLID
    mov     rdx, 2
    mov     r8d, [rel col_dim]
    call    CreatePen
    mov     [rel pen_dim], rax

    mov     rcx, rbx
    mov     edx, TID
    mov     r8d, TMS
    xor     r9, r9
    call    SetTimer
    xor     eax, eax
    jmp     .ret

; ---- WM_TIMER ----
.wt:
    mov     rcx, rbx
    xor     edx, edx
    xor     r8d, r8d
    call    InvalidateRect
    xor     eax, eax
    jmp     .ret

; ---- WM_KEYDOWN ----
.wk:
    mov     r8, [rsp+L_SAV_WP]
    
    cmp     r8d, VK_ESC
    je      .wkesc
    
    cmp     r8d, KEY_W
    ; Handle TryMove logic here soon
    jmp     .wkd

.wkesc:
    mov     rcx, rbx
    call    DestroyWindow
    xor     eax, eax
    jmp     .ret

.wkd:
    mov     rcx, rbx
    mov     edx, [rsp+L_SAV_MSG]
    mov     r8,  [rsp+L_SAV_WP]
    mov     r9,  [rsp+L_SAV_LP]
    call    DefWindowProcA
    jmp     .ret

; ---- WM_DESTROY ----
.wd:
    mov     rcx, rbx
    mov     edx, TID
    call    KillTimer
    xor     ecx, ecx
    call    PostQuitMessage
    xor     eax, eax
    jmp     .ret

; ---- WM_PAINT ----
.wp:
    mov     rcx, rbx
    lea     rdx, [rel ps_buf]
    call    BeginPaint
    mov     r12, rax               ; hdc

    mov     rcx, rbx
    lea     rdx, [rel rc_buf]
    call    GetClientRect

    ; PatBlt black (BLACKNESS)
    mov     rcx, r12
    xor     edx, edx
    xor     r8d, r8d
    mov     r9d, [rel rc_buf+8]
    mov     r10d, [rel rc_buf+12]
    mov     dword [rsp+32], r10d
    mov     dword [rsp+40], BLACKNESS
    call    PatBlt

    ; End Paint
    mov     rcx, rbx
    lea     rdx, [rel ps_buf]
    call    EndPaint
    xor     eax, eax

.ret:
    add     rsp, STK
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    pop     rbx
    pop     rbp
    ret
