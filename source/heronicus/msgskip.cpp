#include "msgskip.h"
#include "patch.h"

#include <common.h>
#include <spm/memory.h>
#include <spm/msgdrv.h>
#include <spm/system.h>
#include <spm/windowdrv.h>
#include <spm/wpadmgr.h>
#include <wii/os.h>
#include <wii/wpad.h>

/*
    Made with love by Yme
*/

namespace mod::msgskip {
    using namespace spm;

    s32 msgSkipAdv() {
        s32 ret = 0;
        if ((wpadmgr::wpadGetButtonsPressed(0) & (WPAD_BTN_1 | WPAD_BTN_2)) != 0 ||
            (wpadmgr::wpadGetButtonsHeld(0) & (WPAD_BTN_1 | WPAD_BTN_2)) == (WPAD_BTN_1 | WPAD_BTN_2))
            ret = 1;
        return ret;
    }

    void msgSkipInit() {
        writeBranchLink(msgdrv::msgMain, 0x38C, msgSkipAdv);
        writeWord(msgdrv::msgMain, 0x390, MR(0, 3));
        writeWord(msgdrv::msgMain, 0x394, B(0xC));
        writeBranchLink(msgdrv::msgWindow_Main, 0x31C, msgSkipAdv);
        writeWord(msgdrv::msgWindow_Main, 0x320, MR(0, 3));
        writeWord(msgdrv::msgWindow_Main, 0x324, B(0xC));
        writeBranchLink(msgdrv::msgWindow_Clear_Main, 0x164, msgSkipAdv);
        writeWord(msgdrv::msgWindow_Clear_Main, 0x168, MR(0, 3));
        writeWord(msgdrv::msgWindow_Clear_Main, 0x16C, B(0xC));
    }
}
