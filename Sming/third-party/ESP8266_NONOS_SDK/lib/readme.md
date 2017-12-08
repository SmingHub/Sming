# About libc.a and libgcc.a

In order to reduce the size of **.text**, we remove the functions which already in rom code.

The removed functions in libgcc.a:

```
__addsubdf3
__addsubsf3
__divdf3
__divdi3
__divsi3
__extendsfdf2
__fixdfsi
__fixunsdfsi
__fixunssfsi
__floatsidf
__floatsisf
__floatunsidf
__floatunsisf
__muldf3
__muldi3
__mulsf3
__truncdfsf2
__udivdi3
__udivsi3
__umoddi3
__umodsi3
__umulsidi3
```

The removed functions in libc.a:

```
bzero
memcmp
memcpy
memmove
memset
strcmp
strcpy
strlen
strncmp
strncpy
strstr
```

## How to remove the functions in those two lib.

The libc.a in SDK is compiled from newlib v2.0.0, libgcc.a is compiled from gcc v4.8.5.
If you use other version gcc and newlib, you can follow those commands to strip the functions.

```
cp $(TOOLCHAIN)/lib/gcc/xtensa-lx106-elf/<version>/libgcc.a .
xtensa-lx106-elf-ar -M < strip_libgcc_funcs.txt

cp $(TOOLCHAIN)/xtensa-lx106-elf/lib/libc.a .
xtensa-lx106-elf-ar -M < strip_libc_funcs.txt
```