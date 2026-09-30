#pragma once

#include <common.h>
#include <spm/mario.h>
#include <spm/hitdrv.h>

CPP_WRAPPER(spm::mot_fairy_mario)

USING(spm::mario::MarioWork)
USING(spm::hitdrv::HitObj)

s32 fairySpindashUseFunc();

void mot_fleep(MarioWork * mario);
void mot_fleep_post(MarioWork * mario);
void mot_spindash(MarioWork * mario);
void mot_spindash_post(MarioWork * mario);
void mot_hammer(MarioWork * mario);
void mot_hammer_post(MarioWork * mario);
HitObj * func_8016ece4(int * param_1);
u32 func_80167320();
u32 func_80166904();
void func_80170384();

CPP_WRAPPER_END()
