## Byte code execution tool

This simple tool converts binary code written in a text file to a proper int (*foo)() and calls it printing the returned result to a console. It ignores white spaces and handles assembler like comments.
Works only for windows.

### Example
10111000 00000101 00000000 00000000 00000000 ; mov eax, 5  
11000011 ; ret

### Tested configuration
Windows 10  
Debug/Release|x86
