/* Based on https://github.com/hnes/libaco */

.text
.globl schmitDetailsCoroutineContextSwitch
.type  schmitDetailsCoroutineContextSwitch, @function
.intel_syntax noprefix
schmitDetailsCoroutineContextSwitch:
    /* rdi - srce
       rsi - dest */
    mov     rdx, QWORD PTR [rsp + 0x00]       /* retaddr */
    lea     rcx, QWORD PTR [rsp + 0x08]       /* rsp */
    mov          QWORD PTR [rdi + 0x08] , r12
    mov          QWORD PTR [rdi + 0x10] , r13
    mov          QWORD PTR [rdi + 0x18] , r14
    mov          QWORD PTR [rdi + 0x20] , r15
    mov          QWORD PTR [rdi + 0x28] , rdx /* retaddr */
    mov          QWORD PTR [rdi + 0x30] , rcx /* rsp */
    mov          QWORD PTR [rdi + 0x38] , rbx
    mov          QWORD PTR [rdi + 0x40] , rbp
    stmxcsr      DWORD PTR [rdi + 0x48]
    fnstcw        WORD PTR [rdi + 0x4c]
    mov     r12, QWORD PTR [rsi + 0x08]
    mov     r13, QWORD PTR [rsi + 0x10]
    mov     r14, QWORD PTR [rsi + 0x18]
    mov     r15, QWORD PTR [rsi + 0x20]
    mov     rax, QWORD PTR [rsi + 0x28] /* retaddr */
    mov     rcx, QWORD PTR [rsi + 0x30] /* rsp */
    mov     rbx, QWORD PTR [rsi + 0x38]
    mov     rbp, QWORD PTR [rsi + 0x40]
    ldmxcsr      DWORD PTR [rsi + 0x48]
    fldcw         WORD PTR [rsi + 0x4c]
    mov     rsp, rcx
    jmp     rax

.globl schmitDetailsCoroutineMmxFpuSave
.type  schmitDetailsCoroutineMmxFpuSave, @function
.intel_syntax noprefix
schmitDetailsCoroutineMmxFpuSave:
    stmxcsr DWORD PTR [rdi + 0x0]
    fnstcw   WORD PTR [rdi + 0x4]
    ret
