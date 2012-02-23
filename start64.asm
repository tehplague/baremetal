; This is the kernel's entry point. We could either call main here,
; or we can use this to setup the stack or other nice stuff, like
; perhaps setting up the GDT and segments. Please note that interrupts
; are disabled at this point: More on interrupts later!
[BITS 32]
global start
extern Realm32

MODULEALIGN equ  1<<0
MEMINFO     equ  1<<1
FLAGS       equ  MODULEALIGN | MEMINFO
MAGIC       equ  0x1BADB002
CHECKSUM    equ  -(MAGIC + FLAGS)

STACKSPACE  equ 0x4000                      ; initial Stack (16k)

SECTION .text
ALIGN 4
MultiBootHeader:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

extern hw_info          ; first element of structure is uint32_t for multiboot info address

start:
    mov esp, _sys_stack     ; This points the stack to our new stack area (reserved in .bss)

    mov [hw_info], ebx

    
    ; debug output
    mov ax, 0x0F00+'0'
    mov [0xB8000], ax
    mov ax, 0x0F00+'0'
    mov [0xB8002], ax


%include "start.asm"

setup:                  ; now 32-bit specific set up

    mov ax, 0x0F00+'2'
    mov [0xB8000], ax
    mov ax, 0x0F00+'0'
    mov [0xB8002], ax


    ; check if ext'd CPUID function 8000_0001 is available
    mov eax, 0x8000_0000
    cpuid
    cmp eax, 0x8000_0001
    jb NoLongMode

    ; check if long mode is available: CPUID[8000_0001]:EDX[29]
    mov eax, 0x8000_0001
    cpuid
    test edx, 1 << 29
    jz NoLongMode

    mov ax, 0x0F00+'2'
    mov [0xB8000], ax
    mov ax, 0x0F00+'2'
    mov [0xB8002], ax
    

    ; disable 32-bit paging (just in case it was activated...)
    mov eax, cr0
    and eax, 0x7FFFFFFF  ;~(1<<31)
    ;and eax, ~(1<<31)
    mov cr0, eax

    mov ax, 0x0F00+'2'
    mov [0xB8000], ax
    mov ax, 0x0F00+'3'
    mov [0xB8002], ax

    ; prepare page tables
    ; first, set entries to 0
    mov edi, 0x1000             ; page tables are put here. (TODO: Why?!)
    mov cr3, edi                ; set cr3 to the PML4T
    xor eax, eax
    mov ecx, 7*1024             ; x * 1k  ( x: number of pages for page tables needed)
    rep stosd                   ; 4 * x * 1k   (4: storing dw's (4B) from eax := 0  )

    ; initialize identity paging (Flags: R/W (read and write), P (present))
    mov edi, cr3                ; edi = PML4T (0x1000)
    mov DWORD [edi], 0x2003     ; PML4T[0]=[0x1000] := 0x2003 (Frame 2; R/W, P)
    add edi, 0x1000             ; edi = PDPT (0x2000, Frame 2)
    mov DWORD [edi], 0x3003     ; PDPT[0]=[0x2000] := 0x3003 (Frame 3; R/W, P)
    mov DWORD [edi+3*8],0x4003  ; PDPT[3]=[0x2018] := 0x4003 (Frame 4; R/W, P)
    add edi, 0x1000             ; edi = PDT.0 (0x3000, Frame 3)
    mov DWORD [edi], 0x5003     ; PDT.0[0]=[0x3000] := 0x4003 (Frame 5, R/W, P)
    add edi, 0x1000             ; edi = PDT.3 (0x4000, Frame 4)
    mov DWORD [edi+0x1f6*8], 0x6003  ; PDT.3[0x1f6] := 0x6003 (Frame 6, R/W, P)
    mov DWORD [edi+0x1f7*8], 0x7003  ; PDT.3[0x1f7] := 0x7003 (Frame 7, R/W, P)

    add edi, 0x1000             ; edi = PTE.0.0 (0x5000, Frame 5)
    mov ebx, 0x0000_0003        ; first frame (0) (..03 : Flags for R/W and Present)
    mov ecx, 512                ; loop over 512 entries
.SetEntry:
    mov DWORD [edi], ebx
    add ebx, 0x1000             ; next page: +4k
    add edi, 8                  ; next entry: +8
    loop .SetEntry              ; counting ECX down (from 512)
    ; so far, the first 512 * 4k = 2 MB are set up.

    ; for I/O APIC
    mov edi, 0x6000             ; edi = PTE.3.1f6 (0x6000, Frame 6)
    mov ebx, 0xfec0_0013        ; physical frame for I/O APIC access (deactivate cache!)
    mov DWORD [edi], ebx        ; set first entry

    ; for local APIC
    add edi, 0x1000             ; edi = PDE.3.1f7 (0x7000, Frame 7)
    mov ebx, 0xfee0_0013        ; physical frame for local APIC access (deactivate cache!)
    mov DWORD [edi], ebx        ; set first entry
    ; Page Table Flags (e.g. AMD Arch. Programmer's Manual, Vol. 2, Chapter 5.4 Page-Translation-Table Entry Fields)
    ; Bit 0 : P(resent) 
    ; Bit 1 : R(ead)/W(rite) : 0 - Read only (also for lower levels of page-directories)
    ; Bit 3 : Page-Level Writethrough : 0 - Writeback / 1 - Writethrough (TODO: same with Intel?)
    ; Bit 4 : Page-Level Cache Disable (PCD): 0 - Cache on / 1 - Cache off for this Page

    ; enable PAE (cr4[5])
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax


    mov ax, 0x0F00+'2'
    mov [0xB8000], ax
    mov ax, 0x0F00+'4'
    mov [0xB8002], ax

    ; the switch from real-mode

    ; set LM-bit
    mov ecx, 0xC000_0080        ; EFER MSR
    rdmsr
    or eax, 1<<8                ; LM-bit (EFER MSR[8])
    wrmsr
    ; enable paging and protected mode
    mov eax, cr0
    or eax, 1<<31 | 1 <<0       ; PG ( cr0[31] ) and PM ( cr0[0] )
    mov cr0, eax

    ; now in compatibility mode

    mov ax, 0x0F00+'2'
    mov [0xB8000], ax
    mov ax, 0x0F00+'5'
    mov [0xB8002], ax


    jmp Realm32


endless:
    hlt
    jmp endless



NoLongMode:             ; CPU does not support 64 bit long mode
    mov eax, 'E'
    mov [0xB8000], eax
    mov eax, '4'
    mov [0xB8002], eax
    jmp endless



; Here is the definition of our BSS section. Right now, we'll use
; it just to store the stack. Remember that a stack actually grows
; downwards, so we declare the size of the data before declaring
; the identifier '_sys_stack'
SECTION .bss
_sys_stackend:
    resb STACKSPACE               ; This reserves 8KBytes of memory here
_sys_stack:
