/**
 * 自己动手写操作系统
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * 该部分程序存储于磁盘的第1个扇区，在计算机启动时将会由BIOS加载到0x7c00处
 * 之后，将由BIOS跳转至0x7c00处开始运行
 */
	#include "boot.h"

  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start
	.extern boot_entry
_start:

	# 平坦模型，所有段寄存器设为0 (段基址为0)
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# 0x7c00作为boot程序栈起始地址
	mov $_start, %esp

	# 调用BIOS软中断在屏幕输出字符 https://www.yuque.com/lishutong-docs/diyx86os/gl7gku
	mov $0xe, %ah
	mov $'L', %al
	int $0x10

read_loader:
	# 借助BIOS从硬盘第二个扇区读取loader程序  https://www.yuque.com/lishutong-docs/diyx86os/qnibf6
	mov $0x8000, %bx
	mov $0x2, %ah
	mov $0x2, %cx
	mov $64, %al
	mov $0x0080, %dx
	int $0x13
	jc read_loader

	jmp boot_entry
	
	// 引导结束段
	.section boot_end, "ax"
boot_sig: .byte 0x55, 0xaa
