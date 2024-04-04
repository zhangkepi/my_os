    .text
    .global _start
    .extern kernel_init
_start:
    # 将boot_info参数传给kernel_init函数
    # push %ebp
    # mov %esp, %ebp
    # mov 0x8(%ebp), %eax
    # push %eax

    # 将boot_info参数传给kernel_init函数
    mov 0x4(%esp), %eax
    push %eax

    call kernel_init
    jmp .