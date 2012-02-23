%include "config.inc"

; This is the kernel's entry point. We could either call main here,
; or we can use this to setup the stack or other nice stuff, like
; perhaps setting up the GDT and segments. Please note that interrupts
; are disabled at this point: More on interrupts later!
[BITS 32]
global start

start:
    ;mov esp, _sys_stack     ; This points the stack to our new stack area

    extern stack            ; stack_t stack[MAX_CPUS] (in smp.c / smp.h)
    mov edi, STACK_FRAMES*4096  ; stack size
    add edi, stack          ; add offset of first stack
    mov esp, edi            ; set Stack Pointer

    jmp stublet

; This part MUST be 4byte aligned, so we solve that issue using 'ALIGN 4'
ALIGN 4
mboot:
    ; Multiboot macros to make a few lines later more readable
    MULTIBOOT_PAGE_ALIGN	equ 1<<0
    MULTIBOOT_MEMORY_INFO	equ 1<<1
    MULTIBOOT_AOUT_KLUDGE	equ 1<<16
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
    MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
    EXTERN code, bss, end

    ; This is the GRUB Multiboot header. A boot signature
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    
    ; AOUT kludge - must be physical addresses. Make a note of these:
    ; The linker script fills in the data for these ones!
    dd mboot
    dd code
    dd bss
    dd end
    dd start
mbi dw 0

extern hw_info          ; first element of structure is uint32_t for multiboot info address

stublet:
    mov [hw_info], ebx

    
    ; debug output  ('00' : first instruction in start32.asm after multiboot)
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


    ; load GDT
    lgdt [GDT32.Pointer]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2     ; the far-jump loads the code selector which activates the protected mode
flush2:                 ; now in protected mode

    mov ax, 0x0F00+'3'
    mov [0xB8000], ax
    mov ax, 0x0F00+'0'
    mov [0xB8002], ax


    ; jump into C main()
extern main_bsp
    call main_bsp         ; main()   (further on in the main kernel)

    ; when returning from main: go into endless halt loop
endless:
    hlt
    jmp endless


; Shortly we will add code for loading the GDT right here!

GDT32:
    .Null: equ $ - GDT32        ; the null descriptor
    dw 0                        ; Limit (low)
    dw 0                        ; Base (low)
    db 0                        ; Base (middle)
    db 0                        ; Access
    db 0                        ; Granularity
    db 0                        ; Base (high)
    .Code : equ $ - GDT32
    dw 0xFFFF                    ; Limit (low)
    dw 0                         ; Base (low)
    db 0                         ; Base (middle)
    db 0x9A                      ; Access.       p=1  dpl=00  11  c=0  r=0  a=0  (code segment)
    db 0xCF                      ; Granularity. 
    db 0                         ; Base (high).
    .Data: equ $ - GDT32         ; The data descriptor.
    dw 0xFFFF                    ; Limit (low).
    dw 0                         ; Base (low).
    db 0                         ; Base (middle)
    db 0x92                      ; Access.       p=1  dpl=00  10  e=0  w=0  a=0  (data segment)
    db 0xCF                      ; Granularity.
    db 0                         ; Base (high).
    .Pointer:                    ; The GDT-pointer.
    dw $ - GDT32 - 1             ; Limit.
    dd GDT32                     ; Base.

global GDT_Code
GDT_Code : equ GDT32.Code



; In just a few pages in this tutorial, we will add our Interrupt
; Service Routines (ISRs) right here!

global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

%macro ISR_EXCEPTION_WITHOUT_ERRCODE 1
    global isr %+ %1
    isr %+ %1 :
        push dword 0
        push dword %1
        jmp isr_common_stub
%endmacro

%macro ISR_EXCEPTION_WITH_ERRCODE 1
    global isr %+ %1
    isr %+ %1 :
        ; Errcode is already on the stack
        push dword %1
        jmp isr_common_stub
%endmacro

%macro ISR_INTERRUPT 1
    global isr %+ %1
    isr %+ %1 :
        push dword 0
        push dword %1
        jmp isr_common_stub
%endmacro

ISR_EXCEPTION_WITHOUT_ERRCODE 0
ISR_EXCEPTION_WITHOUT_ERRCODE 1
ISR_EXCEPTION_WITHOUT_ERRCODE 2
ISR_EXCEPTION_WITHOUT_ERRCODE 3
ISR_EXCEPTION_WITHOUT_ERRCODE 4
ISR_EXCEPTION_WITHOUT_ERRCODE 5
ISR_EXCEPTION_WITHOUT_ERRCODE 6
ISR_EXCEPTION_WITHOUT_ERRCODE 7
ISR_EXCEPTION_WITH_ERRCODE 8
ISR_EXCEPTION_WITHOUT_ERRCODE 9
ISR_EXCEPTION_WITH_ERRCODE 10
ISR_EXCEPTION_WITH_ERRCODE 11
ISR_EXCEPTION_WITH_ERRCODE 12
ISR_EXCEPTION_WITH_ERRCODE 13
ISR_EXCEPTION_WITH_ERRCODE 14
ISR_EXCEPTION_WITHOUT_ERRCODE 15
ISR_EXCEPTION_WITHOUT_ERRCODE 16
ISR_EXCEPTION_WITHOUT_ERRCODE 17
ISR_EXCEPTION_WITHOUT_ERRCODE 18
ISR_EXCEPTION_WITHOUT_ERRCODE 19
ISR_EXCEPTION_WITHOUT_ERRCODE 20
ISR_EXCEPTION_WITHOUT_ERRCODE 21
ISR_EXCEPTION_WITHOUT_ERRCODE 22
ISR_EXCEPTION_WITHOUT_ERRCODE 23
ISR_EXCEPTION_WITHOUT_ERRCODE 24
ISR_EXCEPTION_WITHOUT_ERRCODE 25
ISR_EXCEPTION_WITHOUT_ERRCODE 26
ISR_EXCEPTION_WITHOUT_ERRCODE 27
ISR_EXCEPTION_WITHOUT_ERRCODE 28
ISR_EXCEPTION_WITHOUT_ERRCODE 29
ISR_EXCEPTION_WITHOUT_ERRCODE 30
ISR_EXCEPTION_WITHOUT_ERRCODE 31

ISR_INTERRUPT 128




extern int_handler
isr_common_stub:
    ; save complete context (multi-purpose&integer, no fpu/sse)
    ; the structure is also defined in system.h:struct regs
    pusha
    push ds
    push es
    push fs
    push gs
    mov eax, cr3
    push eax
    mov eax, cr2
    push eax
    mov ax, GDT32.Code
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp            ; Stack-Pointer to EAX
    push eax                ; push EAX; 1st Parameter to int_handler()
    mov eax, int_handler
    call eax
    pop eax ; Param (esp)
    pop eax ; cr2
    pop eax ; cr3
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8      ; clear up error code and ISR number (two elements)
    iret


; Wake-up code for SMP Application Processors
%include 'start_smp.inc'
    ; this include file is left by the APs in 32 bit protected mode.

    ; just call main_smp()
    extern main_ap
    call main_ap

    ; in case, main_smp() ever should return
.endless:
    hlt
    jmp .endless
   


; Here is the definition of our BSS section. Right now, we'll use
; it just to store the stack. Remember that a stack actually grows
; downwards, so we declare the size of the data before declaring
; the identifier '_sys_stack'
;SECTION .bss
;    resb 8192               ; This reserves 8KBytes of memory here
;_sys_stack:



