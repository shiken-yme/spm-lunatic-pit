#pragma once

#include <common.h>
#include <spm/seqdrv.h>
#include <spm/evtmgr.h>
#include <spm/filemgr.h>
#include <wii/mtx.h>
#include <wii/gx.h>

CPP_WRAPPER(spm::seq_load_sub)

USING(spm::seqdrv::SeqWork)
USING(spm::evtmgr::EvtEntry)
USING(spm::filemgr::FileEntry)
USING(wii::gx::GXColor)
USING(wii::mtx::Vec2)

#define SEQ_LOAD_BTN_SAVEFILE_1 0
#define SEQ_LOAD_BTN_SAVEFILE_2 1
#define SEQ_LOAD_BTN_SAVEFILE_3 2
#define SEQ_LOAD_BTN_SAVEFILE_4 3
#define SEQ_LOAD_BTN_ERASE 4
#define SEQ_LOAD_BTN_COPY 5
#define SEQ_LOAD_BTN_BACK 6


typedef struct {
/* 0x00 */ u8 nextSelLeft;
/* 0x01 */ u8 nextSelRight;
/* 0x02 */ u8 nextSelUp;
/* 0x03 */ u8 nextSelDown;
/* 0x04 */ s32 bgColIdx;
/* 0x08 */ Vec2 cursorPos;
} SeqLoadButtonDef;
SIZE_ASSERT(SeqLoadButtonDef, 0x10)

typedef struct {
/* 0x00 */ u32 flags;
/* 0x04 */ u32 state;
/* 0x08 */ FileEntry * fileselTpl;
/* 0x0C */ s32 curSelection; // 0-3 savefiles, 4-6 erase/copy/back
/* 0x10 */ s32 colorIntplProgress;
/* 0x14 */ s32 prevSelection; // Never used ingame, but definitely written to
/* 0x18 */ Vec2 destCursorPos;
/* 0x20 */ Vec2 cursorPos;
/* 0x28 */ u8 unknown_0x28[0x2C - 0x28];
/* 0x2C */ EvtEntry * saveUpdateEvt;
/* 0x30 */ s32 saveUpdateEvtId;
/* 0x34 */ s32 saveCopyFrom;
/* 0x38 */ s32 saveCopyTo;
/* 0x3C */ u8 unknown_0x3C[0x44 - 0x3C];
/* 0x44 */ char * headerMsgName; // not constant
/* 0x48 */ GXColor lastBgGradientTop;
/* 0x4C */ GXColor lastBgGradientBottom;
/* 0x50 */ GXColor bgGradientTop;
/* 0x54 */ GXColor bgGradientBottom;
/* 0x58 */ u8 unknown_0x58[0x5C - 0x58];
} SeqLoadSubWork;
SIZE_ASSERT(SeqLoadSubWork, 0x5C)

DECOMP_STATIC(SeqLoadSubWork * seq_load_sub_wp)

DECOMP_STATIC(SeqLoadButtonDef seq_loadWindowButtonDefs[7]);
DECOMP_STATIC(SeqLoadButtonDef seq_loadWindowButtonDefs_EraseCopy[7]);

s32 seq_load_sub_loadMain();

void windowRenderMain();

CPP_WRAPPER_END()
