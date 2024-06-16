
sub hello+1, #0x20

mov cx, #message
mov ax, cx
mov bx, ax
int 0x20
mov bx, #exit
int 0x20

.sect .data
message: .data2 1, 4, 1, 6, 0, hello, 0, 0
exit: .data2 1, 1, 0, 0, 0, 0, 0, 0
hello: .ascii "hello\n"
