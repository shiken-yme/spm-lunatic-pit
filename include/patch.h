#pragma once

#include <common.h>

namespace mod::patch
{

    void clear_DC_IC_Cache(void *ptr, u32 size);
    void _writeBranch(void *ptr, void *destination, bool link = false);
    void _writeWord(void *ptr, u32 value);

#define writeBranch(ptr, offset, destination) \
    mod::patch::_writeBranch((void *)((u32)(ptr) + offset), (void *)(destination))
#define writeBranchLink(ptr, offset, destination) \
    mod::patch::_writeBranch((void *)((u32)(ptr) + offset), (void *)(destination), true)
#define writeWord(ptr, offset, value) \
    mod::patch::_writeWord((void *)((u32)(ptr) + (offset)), (value))

// ASM instructions
#define NOP 0x60000000
#define BLR 0x4E800020
#define LI(reg, val) \
    ((0x38000000 | (val & 0xFFFF)) + (0x1000000 * (reg / 8)) + (0x200000 * (reg % 8)))
#define ADDI(destReg, srcReg, val) \
    ((0x38000000 | (val & 0xFFFF)) + (0x1000000 * (destReg / 8)) + (0x200000 * (destReg % 8)) + (0x10000 * srcReg))
#define MR(destReg, srcReg) \
    (0x7C000378 + (0x1000000 * (srcReg / 8)) + ((0x10000 * destReg) + (0x200000 * (srcReg % 8))) + (0x100 * (srcReg * 8)))
#define B(offset) \
    (0x48000000 | (offset & 0x3FFFFFC))
#define BNE(offset) \
    (0x40820000 | (offset & 0xFFFC))
#define BL(offset) \
    (0x48000001 | (offset & 0x3FFFFFC))
#define RLWINM(destReg, srcReg, shift, maskBegin, maskEnd) \
    (0x54000000 + (destReg * 0x10000) + (srcReg * 0x200000) + (shift * 0x800) + (maskBegin * 0x40) + (maskEnd * 0x2))
#define FCMPO(conditionReg, lFloatReg, rFloatReg) \
    (0xFC000040 + (conditionReg * 0x800000) + (lFloatReg * 0x10000) + (rFloatReg * 0x800))
#define LWZ(destReg, offset, srcReg) \
    ((0x80000000 | (offset & 0xFFFF)) + (destReg * 0x200000) + (srcReg * 0x10000))
#define STW(destReg, offset, srcReg) \
    ((0x90000000 | (offset & 0xFFFF)) + (destReg * 0x200000) + (srcReg * 0x10000))
#define STH(destReg, offset, srcReg) \
    ((0xB0000000 | (offset & 0xFFFF)) + (destReg * 0x200000) + (srcReg * 0x10000))
#define STB(destReg, offset, srcReg) \
    ((0x98000000 | (offset & 0xFFFF)) + (destReg * 0x200000) + (srcReg * 0x10000))
#define CMPWI(reg, val) \
    ((0x2C000000 | (val & 0xFFFF)) + (reg * 0x10000))

    template <typename Func, typename Dest>
    Func hookFunction(Func function, Dest destination)
    {
        u32 *instructions = reinterpret_cast<u32 *>(function);

        u32 *trampoline = new u32[2];

        // Original instruction
        trampoline[0] = instructions[0];
        clear_DC_IC_Cache(&trampoline[0], sizeof(u32));

        // Branch to original function past hook
        _writeBranch(&trampoline[1], &instructions[1]);

        // Write actual hook
        _writeBranch(&instructions[0], reinterpret_cast<void *>(static_cast<Func>(destination)));

        return reinterpret_cast<Func>(trampoline);
    }
}
