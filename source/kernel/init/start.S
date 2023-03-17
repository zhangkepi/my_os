

    .text
    .extern kernel_init
    .global _start
_start:
    # 保存调用者ebp
    push %ebp
    # 更新ebp指向当前被调用者栈定
    mov %esp, %ebp

    # 从栈中取boot_info到eax
    mov 0x8(%esp), %eax

    # boot_info压栈，向kernel_init传递参数
    push %eax
    call kernel_init
    jmp .