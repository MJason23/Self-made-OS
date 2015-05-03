# Self-made-OS
A totally self made OS running on qemu simulator, which implement all the basic and fundamental functions of a small OS.

It has boot function implemented in assembly language, and then after the boot part, all the other code were written in C. Graphic interface, shell, process control, timer, interrupt control are all integrated into it, all the code are seprated in different files which each of them is in charge of their own part. So it will be easy to read.

There is a makefile in this project which is to compile all the files in one step.

The only pity thing in this OS is that it has no file management system, which is a little tricky.

This project is my own interest and is with the help of the book "Make your own OS in 30 days" and Minix source.
