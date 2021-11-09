# rBPF: Riot-style Berkeley Packet Filters

The rBPF subsystem provides a minimal virtual machine for microcontrollers.

## Introduction

rBPF is a virtual machine based on the popular Linux BPF virtual machine. It is
small and fast enough to host multiple instances on platforms targeted by RIOT.

By using the already standardized ISA, the toolchain for BPF can be reused to
compile VM application for RIOT.

## Architecture

The rBPF virtual machine is a small VM optimized for short running tasks. The VM
itself is event triggered and executes a small application to respond to the
event.

The virtual machine architecture is standardized in the Linux kernel, with the
RIOT implementation following the ISA[ISA] defined for Linux.

### Security

As a VM, the implementation must ensure that the influence from the application
code to the host is strictly limited. This includes memory access and execution
of the application.

The memory protection mechanism works with simple ACLs (a `bpf_mem_region_t`),
which are queried for every store and load instruction. The VM automatically
includes a region for the allocated stack space. Other regions can be added by
the caller. By default the VM denies access to any location outside the allowed
regions.  Each region can add separate permissions for both read and write
access. Regions can be added to the list with the `bpf_add_region` function

The other protection mechanism is the in the branch and jump code. Here the
checks ensure that the code remains within the execution address space of the
loaded application.

## Options

Two core VM implementations are currently available, each with their own
advantages and disadvantages. The first implementation is based on a switch-case
statement to parse instructions. The second implementation is a computed
jump table approach.

### Switch-Case

The switch-case implementation parses each opcode using standard C switch and
case statements. The implementation trades some speed for a smaller
implementations.

### Computed Jump Table

The computed jump table approach uses a GNU C extension to resolve each opcode
number to a label and jump there with a `goto` instruction. It uses a
significant amount of C macros to generate the different instruction
implementations.

This implementation has some significant speed advantages, but the
implementation is also larger than the switch-case statement as similar
instructions are not deduplicated.

## Interaction

Interaction to outside of the virtual machine from within happens via two
mechanisms. First is the syscall mechanism. Second is the key-value store.

The syscall mechanism uses the bpf native call interface to hook existing OS
facilities into the virtual machine. When a call instruction hits, the parser
checks the immediate of the instruction. If this resolves to a valid call
number, the function associated with that number is executed. It is passed the
first five register values as arguments. These match with the first five
arguments of the function definition. More than five call arguments is out of
scope here. The called function can return a value by placing it in the zeroth
register. The called function does not have to respect the memory ACLs.
See `sys/include/bpf/bpfapi/helpers.h` for a mapping between functions and call
values. The implementations are in `sys/bpf/call.c`.

The second mechanism, the key-value store, allows for storing simple
integer-based values between invocations. The key-value is accessible via calls.
Every VM has its own key-value store and one global key-value store exists for
sharing values between instances.


## See also

[rBPF paper](https://hal.inria.fr/hal-03019639)


[ISA]: https://github.com/iovisor/bpf-docs/blob/master/eBPF.md
