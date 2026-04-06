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
    mov          QWORD PTR [rdi + 0x00] , r12
    mov          QWORD PTR [rdi + 0x08] , r13
    mov          QWORD PTR [rdi + 0x10] , r14
    mov          QWORD PTR [rdi + 0x18] , r15
    mov          QWORD PTR [rdi + 0x20] , rdx /* retaddr */
    mov          QWORD PTR [rdi + 0x28] , rcx /* rsp */
    mov          QWORD PTR [rdi + 0x30] , rbx
    mov          QWORD PTR [rdi + 0x38] , rbp
    stmxcsr      DWORD PTR [rdi + 0x40]
    fnstcw        WORD PTR [rdi + 0x44]
    mov     r12, QWORD PTR [rsi + 0x00]
    mov     r13, QWORD PTR [rsi + 0x08]
    mov     r14, QWORD PTR [rsi + 0x10]
    mov     r15, QWORD PTR [rsi + 0x18]
    mov     rax, QWORD PTR [rsi + 0x20] /* retaddr */
    mov     rcx, QWORD PTR [rsi + 0x28] /* rsp */
    mov     rbx, QWORD PTR [rsi + 0x30]
    mov     rbp, QWORD PTR [rsi + 0x38]
    ldmxcsr      DWORD PTR [rsi + 0x40]
    fldcw         WORD PTR [rsi + 0x44]
    mov     rsp, rcx
    jmp     rax

.globl schmitDetailsCoroutineMmxFpuSave
.type  schmitDetailsCoroutineMmxFpuSave, @function
.intel_syntax noprefix
schmitDetailsCoroutineMmxFpuSave:
    stmxcsr DWORD PTR [rdi + 0x0]
    fnstcw   WORD PTR [rdi + 0x4]
    ret
