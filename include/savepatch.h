#pragma once

#include "evt_cmd.h"
#include <common.h>

namespace mod::savepatch {
    #define SEQ_LOAD_BTN_PAGE_LEFT 7
    #define SEQ_LOAD_BTN_PAGE_RIGHT 8

    #define MOVE_NONE 0
    #define MOVE_LEFT_1 1
    #define MOVE_LEFT_2 2
    #define MOVE_RIGHT_1 3
    #define MOVE_RIGHT_2 4

    #define MOVE_DEST_LENGTH 2100.0f
    #define MOVE_TIME_FRMS 70
    #define MOVE_TIME_PHASE1_FRMS 30
    
    typedef struct {
        u8 moveMode;
        bool startMoveMode;
        const char * saveName;
        s32 page;
        s32 moveTimer;
        f32 xOffset;
    } SavepatchWork;

    extern SavepatchWork savepatch_work;
    extern SavepatchWork * savepatch_wp;

    void main(const char * name);
}
