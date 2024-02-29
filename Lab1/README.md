# Lab1
[class webpage](https://nycu-caslab.github.io/OSC2024/labs/lab1.html)
---
## Basic Exercises  
### Basic Exercise 1 - Basic Initialization
+ Linker Script
    - Detail explaination are lied in my ```linker.ld``` comments
    ---
    - Add ```ENTRY(_start)``` to set entry point, where the program start to execute first instruction. 
        - But not adding this is probably OK in raspberry as it still work correctly in QEMU.
    - Use ```. = 0x80000``` to set current location counter ('.' is location counter) to 0x80000 ,current address is automatically incremented when the linker adds data.
        - (Not sure)C stack should start at address 0x80000 and grow downwards, since hardware loads our kernel to address 0x80000 and up, stack can safely run from 0x80000 and down
        - 0x80000 is for aarch64, 0x8000 for 32 bits, and 0x80000 is our where our .text start
    - Add ```.text``` ```.rodata``` ```.data``` ```.bss``` section [ref1](https://blog.louie.lu/2016/11/06/10%E5%88%86%E9%90%98%E8%AE%80%E6%87%82-linker-scripts/) [ref2](https://yodalee.me/2015/04/2015_linkerscript/#provide)
        - Regarding ```.data``` and ```.rodata```: [ref](https://blog.csdn.net/qq_26626709/article/details/51887085)
        > The .text, .rodata, and .data sections contain kernel-compiled instructions, read-only data, and normal data
        - Regarding ```.bss (NOLOAD)```: [ref1](https://zhuanlan.zhihu.com/p/27585869) [ref2](https://stackoverflow.com/questions/57181652/understanding-linker-script-noload-sections-in-embedded-software)
        > Align the section so that it starts at an address that is a multiple of 8. If the section is not aligned, it would be more difficult to use the str instruction to store 0
        - Regarding ```COMMON```: [ref](http://swaywang.blogspot.com/2012/06/elfbss-sectioncommon-section.html)
    - Save bss size in byte for future clearing zero at .S file
    - Regarding the ```linkonce``` in linker script, some reference:[ref1](https://stackoverflow.com/questions/5518083/what-is-a-linkonce-section) [ref2](https://blog.csdn.net/kuankuan02/article/details/91804456?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-91804456-blog-125865933.235%5Ev43%5Epc_blog_bottom_relevance_base8&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-91804456-blog-125865933.235%5Ev43%5Epc_blog_bottom_relevance_base8&utm_relevant_index=2), and below is how GPT explain it:
    > In a linker script, the 'linkonce' attribute is used to specify that a particular section or symbol should be included in the final linked output if it appears only once across all input files. If the section or symbol appears multiple times, the linker will include only one instance of it in the output, discarding any duplicates.
    > This attribute is commonly used for functions or data that are defined in multiple source files but should be treated as a single entity in the final binary. For example, if a function is declared as 'linkonce', the linker will include it in the output if it is defined in only one source file. If the function is defined in multiple source files, the linker will choose one instance to include in the final binary and discard the duplicates.
    > The 'linkonce' attribute helps prevent duplicate symbols or sections from causing linker errors or bloating the size of the final binary. It allows developers to organize their code more flexibly across multiple source files while ensuring that only necessary instances of symbols or sections are included in the linked output.
    - Regarding COMDAT [ref](https://stackoverflow.com/questions/1834597/what-is-the-comdat-section-used-for)
+ .S file
    - Detail explaination are lied in my ```boot.S``` comments.
    ---
    - Procedure:
        - Send all CPU except CPU0 to busy waiting, CPU0 will do the work.
        - Set stack pointer.
        - clear BSS to all 0.
        - jump to ```main``` function in ```kernel_main.c```.
    - Regarding the register, remember w register(32bits) and x register(64bits) shared same address.
### Basic Exercise 2 - Mini UART
[BCM2837 ARM Peripherals manual](https://github.com/raspberrypi/documentation/files/1888662/BCM2837-ARM-Peripherals.-.Revised.-.V2-1.pdf)
> Physical addresses range from 0x3F000000 to 0x3FFFFFFF for peripherals. The bus addresses for peripherals are set up to map onto the peripheral bus address range starting at 0x7E000000. Thus a peripheral advertised here at bus address 0x7Ennnnnn is available at physical address 0x3Fnnnnnn.
+ Background
    > UART stands for Universal asynchronous receiver-transmitter. This device is capable of converting values stored in one of its memory mapped registers to a sequence of high and low voltages. This sequence is passed to your computer via the TTL-to-serial cable and is interpreted by your terminal emulator.

+ Detail explaination lied in my code comments. [ref](https://nycu-caslab.github.io/OSC2024/labs/hardware/uart.html)
    - Initialization:
    > 1. Set AUXENB register to enable mini UART. Then mini UART register can be accessed.
    > 2. Set AUX_MU_CNTL_REG to 0. Disable transmitter and receiver during configuration.
    > 3. Set AUX_MU_IER_REG to 0. Disable interrupt because currently you don’t need interrupt.
    > 4. Set AUX_MU_LCR_REG to 3. Set the data size to 8 bit.
    > 5. Set AUX_MU_MCR_REG to 0. Don’t need auto flow control.
    > 6. Set AUX_MU_BAUD to 270. Set baud rate to 115200
    > 7. After booting, the system clock is 250 MHz.
    > 8. Set AUX_MU_IIR_REG to 6. No FIFO.
    > 9. Set AUX_MU_CNTL_REG to 3. Enable the transmitter and receiver.
    - Read data
    > 1. Check AUX_MU_LSR_REG’s data ready field.
    > 2. If set, read from AUX_MU_IO_REG
    - Write data
    > 1. Check AUX_MU_LSR_REG’s Transmitter empty field.
    > 2. If set, write to AUX_MU_IO_REG
    - ```uart_putc```: print a character
    - ```uart_puts```: print a string.

+ Testing
    - Use qemu for testing: ```qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio```
        - Since we're using UART1 for this exercise, we must provide ```-serial null -serial stdio```
        > NOTE: qemu does not redirect UART1 to terminal by default, only UART0, so you have to use ```-serial null -serial stdio```.
        - Remember to use a linux terminal for this instruction as the VScode teminal might give you "qemu-system-aarch64: symbol lookup error:"
        - if you insist on using VScode terminal, add ```sudo```.
+ Note
    - I notice that in the example it transfer an unsigned int into ```uart_putc```, it should be the same?
    - BCM2837 p.11 AUX_MU_IO_REG Register:
    > 31:8 Reserved, write zero, read as don’t care.

### Basic Exercise 3 - Simple Shell
+ I found that when using real raspberry pi, there will be a replacement character(```\ufffd``` in unicode) in front of my first shell input. Therefore I made a if statement to filter it out. Not sure why this happenes.
+ uart_putc:If no CR, first line of output will be moved right for n chars(n=shell command just input), not sure why(also in raspberry, CR seemed to worked like windows).

### Basic Exercise 4 - Mailbox
+ Background
> Mailboxes facilitate communication between the ARM and the VideoCore.
+ Detail explaination lied in my code comments.
+ Procedure: [ref](https://nycu-caslab.github.io/OSC2024/labs/hardware/mailbox.html)
> 1. Combine the message address (upper 28 bits) with channel number (lower 4 bits)
> 2. Check if Mailbox 0 status register’s full flag is set.
> 3. If not, then you can write to Mailbox 1 Read/Write register.
> 4. Check if Mailbox 0 status register’s empty flag is set.
> 5. If not, then you can read from Mailbox 0 Read/Write register.
> 6. Check if the value is the same as you wrote in step 1.
+ [Regarding the message format](https://github.com/bztsrc/raspi3-tutorial/tree/master/04_mailboxes)
+ The mailbox interface has 28 bits (MSB) available for the value(message address) and 4 bits (LSB) for the channel
+ First take the 64bits address of mailbox(is probably just to ensure it can fits), then take the lower 32bits. Then clear LSB 4 bits(~0xF=1111 1111 1111 1111 1111 1111 1111 0000) and fill with channel number.

+ Revision number
    - As using QEMU, it's emulating RPI 3b, so the revision number will be ```0x00A02082```?
    - Interesting part is that the class github says that revision should be ```0xa020d3``` for rpi3 b+, but it actually stands for Rev1.3. And if you get Rev1.4, it will be ```a020d4```. [ref1](https://forums.raspberrypi.com/viewtopic.php?t=351185) [ref2](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html)
        - How to check revision: take a SD card with official Raspberry Pi OS in it. And type ```cat /sys/firmware/devicetree/base/model``` to get revision number [ref](https://forums.raspberrypi.com/viewtopic.php?t=144434) 
+ Interesting(?) problem encountered
    - I got no output(stuck as soon I start QEMU) when using mailbox. If I comment the global ```mailbox``` array variable declared in ```mailbox.c``` or make it not an array, shell can still work. In the end, I found that I used w1 in my ```boot.S``` instead of w2. This is because that the w1 will destroy x1(upper 32 bits->0, lower: load the content) [ref](https://medium.com/vswe/aarch64-instruction-set-architecture-19d2d68392b)
---
## Advanced Exercises
### Advanced Exercise 1 - Reboot
+ Just follow the TA's provided code, nothing special.
+ There's little data regarding this reset and watchdog register.