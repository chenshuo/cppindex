#!/usr/bin/python

# $ find . -name '*.o.cmd' | xargs ./ckernel.py | parallel -j 8
import re, os, sys

# cmd_net/socket.o := /home/schen/downloads/clang+llvm-3.8.0-x86_64-linux-gnu-ubuntu-14.04/bin/clang -Wp,-MD,net/.socket.o.d  -nostdinc -isystem /home/schen/downloads/clang+llvm-3.8.0-x86_64-linux-gnu-ubuntu-14.04/bin/../lib/clang/3.8.0/include -I./arch/x86/include -Iarch/x86/include/generated/uapi -Iarch/x86/include/generated  -Iinclude -I./arch/x86/include/uapi -Iarch/x86/include/generated/uapi -I./include/uapi -Iinclude/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -Qunused-arguments -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -no-integrated-as -std=gnu89 -m64 -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -O2 -Wframe-larger-than=2048 -fno-stack-protector -Wno-unused-variable -Wno-format-invalid-specifier -Wno-gnu -Wno-asm-operand-widths -Wno-initializer-overrides -fno-builtin -Wno-tautological-compare -mno-global-merge -fno-omit-frame-pointer -fno-optimize-sibling-calls -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Wno-initializer-overrides -Wno-unused-value -Wno-format -Wno-unknown-warning-option -Wno-sign-compare -Wno-format-zero-length -Wno-uninitialized    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(socket)"  -D"KBUILD_MODNAME=KBUILD_STR(socket)" -c -o net/socket.o net/socket.c

P = re.compile('cmd_(.*) := ([^ ]+/bin/clang) (.*)')
I = re.compile('-isystem ([^ ]+/include) ')

def process(line):
    m = P.match(line)
    if not m:
        return
    obj = m.group(1)
    if obj.split('/')[0] not in ['init', 'net', 'mm', 'fs', 'kernel']:
        return
    command = m.group(3)
    command = I.sub('-isystem /usr/lib/clang/3.5.2/include ', command)  # match getBuiltinHeaders in index.cc
    command = command.replace('-no-integrated-as', '').replace('=\#s', '=#s')
    print os.path.expanduser('~/git/cppindex/a.out ') + command + ' || exit'


for cmd in sys.argv[1:]:
    with open(cmd) as f:
        line = f.readline()
        process(line)


