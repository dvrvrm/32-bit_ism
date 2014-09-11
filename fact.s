mov $1,%ebx
mov $10,%ecx
jmp L2
add $3,%edx
add $3,%edx
L3 :
mov %ebx,%eax
mull %ecx
mov %eax,%ebx
sub $1,%ecx
L2 :
cmp $0,%ecx
jg L3
add $3,%edx
add $5,%edx
leave

