#include "savepatch.h"
#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"

#include <msl/math.h>
#include <msl/stdio.h>
#include <spm/animdrv.h>
#include <spm/camdrv.h>
#include <spm/dispdrv.h>
#include <spm/evt_nand.h>
#include <spm/evtmgr.h>
#include <spm/evtmgr_cmd.h>
#include <spm/fontmgr.h>
#include <spm/gxsub.h>
#include <spm/icondrv.h>
#include <spm/map_data.h>
#include <spm/nandmgr.h>
#include <spm/seq_load.h>
#include <spm/seq_load_sub.h>
#include <spm/seqdef.h>
#include <spm/seqdrv.h>
#include <spm/spmario.h>
#include <spm/spmario_snd.h>
#include <spm/swdrv.h>
#include <spm/system.h>
#include <spm/winmgr.h>
#include <spm/wpadmgr.h>
#include <wii/gx.h>
#include <wii/mtx.h>
#include <wii/os/OSError.h>
#include <wii/wpad.h>

using namespace spm::nandmgr;
using namespace spm::wpadmgr;
using namespace spm::camdrv;
using namespace spm::animdrv;
using namespace spm::dispdrv;
using namespace spm::evt_nand;
using namespace spm::icondrv;
using namespace spm::memory;
using namespace spm::fontmgr;
using namespace spm::gxsub;
using namespace spm::seq_load_sub;
using namespace spm::system;
using namespace spm::spmario;
using namespace spm::spmario_snd;
using namespace spm::winmgr;
using namespace wii::gx;
using namespace wii::mtx;

namespace mod::savepatch {

    SavepatchWork savepatch_work;
    SavepatchWork * savepatch_wp = &savepatch_work;

    SeqLoadButtonDef buttonDefs[9] = {
        {SEQ_LOAD_BTN_PAGE_LEFT, SEQ_LOAD_BTN_SAVEFILE_2, SEQ_LOAD_BTN_ERASE, SEQ_LOAD_BTN_SAVEFILE_3, 0, {-275.0f, 110.0f}},     // Save 1
        {SEQ_LOAD_BTN_SAVEFILE_1, SEQ_LOAD_BTN_PAGE_RIGHT, SEQ_LOAD_BTN_BACK, SEQ_LOAD_BTN_SAVEFILE_4, 1, {15.0f, 110.0f}},       // Save 2
        {SEQ_LOAD_BTN_PAGE_LEFT, SEQ_LOAD_BTN_SAVEFILE_4, SEQ_LOAD_BTN_SAVEFILE_1, SEQ_LOAD_BTN_ERASE, 2, {-275.0f, -40.0f}},     // Save 3
        {SEQ_LOAD_BTN_SAVEFILE_3, SEQ_LOAD_BTN_PAGE_RIGHT, SEQ_LOAD_BTN_SAVEFILE_2, SEQ_LOAD_BTN_BACK, 3, {15.0f, -40.0f}},       // Save 4
        {SEQ_LOAD_BTN_BACK, SEQ_LOAD_BTN_COPY, SEQ_LOAD_BTN_SAVEFILE_3, SEQ_LOAD_BTN_SAVEFILE_1, 4, {-280.0f, -190.0f}},          // Erase
        {SEQ_LOAD_BTN_ERASE, SEQ_LOAD_BTN_BACK, SEQ_LOAD_BTN_SAVEFILE_3, SEQ_LOAD_BTN_SAVEFILE_1, 4, {-80.0f, -190.0f}},          // Copy
        {SEQ_LOAD_BTN_COPY, SEQ_LOAD_BTN_ERASE, SEQ_LOAD_BTN_SAVEFILE_4, SEQ_LOAD_BTN_SAVEFILE_2, 4, {120.0f, -190.0f}},          // Back
        {SEQ_LOAD_BTN_PAGE_RIGHT, SEQ_LOAD_BTN_SAVEFILE_1, SEQ_LOAD_BTN_SAVEFILE_1, SEQ_LOAD_BTN_SAVEFILE_3, 5, {-380.0f, 0.0f}}, // Left
        {SEQ_LOAD_BTN_SAVEFILE_2, SEQ_LOAD_BTN_PAGE_LEFT, SEQ_LOAD_BTN_SAVEFILE_2, SEQ_LOAD_BTN_SAVEFILE_4, 5, {340.0f, 0.0f}}    // Right
    };

    s32 (*loadMainReal)();

    void loadwinRenderArrow(Mtx34 mtxAnim, Mtx34 mtxPos, Mtx34 mtxScale, Mtx34 mtxRot, GXColor * col, f32 deeznutshagoteem) {
        PSMTXTrans(mtxPos, 360.0f * deeznutshagoteem * -1.0f, 0.0f, 0.0f);
        PSMTXRotRad((PI / 2) * deeznutshagoteem, mtxRot, 'z');
        PSMTXScale(mtxScale, 0.8f, 0.8f, 0.8f);
        PSMTXConcat(mtxPos, mtxRot, mtxPos);
        PSMTXConcat(mtxPos, mtxAnim, mtxPos);
        PSMTXConcat(mtxPos, mtxScale, mtxPos);
        iconDispGxCol(mtxPos, 0x4010, ICON_POINTING_ARROW, col);
        return;
    }

    // copied pretty much this entire thing from customwin lmao
    void loadwinArrowMain() {
        GXColor col = {255, 255, 255, 255};
        Mtx34 mtxAnim, mtxPos, mtxScale, mtxRot;
        PSMTXTrans(mtxAnim, 0.0f, select_disp_arrow_move[gp->frameCounter % 20], 0.0f);
        loadwinRenderArrow(mtxAnim, mtxPos, mtxScale, mtxRot, &col, -1.0f);
        loadwinRenderArrow(mtxAnim, mtxPos, mtxScale, mtxRot, &col, 1.0f);
        return;
    }

    // pretty much just copying the final block of loadMain
    void loadwin_ctrloff_disp() {
        seq_load_sub_wp->cursorPos.x = (seq_load_sub_wp->destCursorPos.x - seq_load_sub_wp->cursorPos.x) * 0.25f + seq_load_sub_wp->cursorPos.x;
        seq_load_sub_wp->cursorPos.y = (seq_load_sub_wp->destCursorPos.y - seq_load_sub_wp->cursorPos.y) * 0.25f + seq_load_sub_wp->cursorPos.y;
        dispEntry(CAM_ID_3D, 4, 1000.0f, (DispCallback *)windowRenderMain, 0);
        return;
    }

    s32 seq_load_sub_chg_saves() {
        if (savepatch_wp->startMoveMode) {
            savepatch_wp->startMoveMode = false;
            // swaps 4 saves files with next/last 4
            if (seq_load_sub_wp->curSelection == SEQ_LOAD_BTN_PAGE_LEFT) {
                savepatch_wp->page -= 1;
            } else {
                savepatch_wp->page += 1;
            }
            if (savepatch_wp->page > 3) {
                savepatch_wp->page = 0;
            } else if (savepatch_wp->page < 0) {
                savepatch_wp->page = 3;
            }
            // reloads nandmgr_wp->saves
            nandmgr_wp->task = 5;
            nandmgr_wp->stage = 0;
            nandmgr_wp->flag = 1;
            nandmgr_wp->code = 0;
            nandmgr_wp->saveId = 0;
            loadwin_ctrloff_disp();
            return 0;
        } else if (savepatch_wp->moveMode != MOVE_NONE) {
            // this whole move mode mechanic is complete nonsense and I hate the way I did all of this. but it works! hope this smurfs
            f32 xOffset = intplGetValue(INTPL_MODE_QUADRATIC_OUT, 0.0f, MOVE_DEST_LENGTH, savepatch_wp->moveTimer, MOVE_TIME_FRMS);
            if (nandmgr_wp->task != 5 || savepatch_wp->moveTimer < MOVE_TIME_PHASE1_FRMS) {
                savepatch_wp->moveTimer++;
            }
            if (savepatch_wp->moveTimer > MOVE_TIME_PHASE1_FRMS) {
                if (savepatch_wp->moveMode == MOVE_RIGHT_1) {
                    savepatch_wp->moveMode = MOVE_RIGHT_2;
                } else if (savepatch_wp->moveMode == MOVE_LEFT_1) {
                    savepatch_wp->moveMode = MOVE_LEFT_2;
                }
            }
            if (savepatch_wp->moveMode == MOVE_RIGHT_1 || savepatch_wp->moveMode == MOVE_RIGHT_2) {
                xOffset *= -1.0f;
            }
            if (savepatch_wp->moveMode == MOVE_LEFT_2) {
                xOffset -= MOVE_DEST_LENGTH;
            } else if (savepatch_wp->moveMode == MOVE_RIGHT_2) {
                xOffset += MOVE_DEST_LENGTH;
            }
            savepatch_wp->xOffset = xOffset;
            // wii::os::OSReport("moveTimer == %d, xOffset == %f\n", savepatch_wp->moveTimer, savepatch_wp->xOffset);
            if (savepatch_wp->moveTimer == MOVE_TIME_FRMS) {
                savepatch_wp->moveTimer = 0;
                savepatch_wp->moveMode = MOVE_NONE;
                savepatch_wp->xOffset = 0.0f;
            }
            loadwin_ctrloff_disp();
            return 0;
        }
        return loadMainReal();
    }

    s32 new_sprintf(char * buffer, const char * saveName, s32 saveId) {
        saveId += (savepatch_wp->page * 4);
        return msl::stdio::sprintf(buffer, "%s%02d", savepatch_wp->saveName, saveId);
    }

    void nandInitNew() {
        SaveFile * saves = (SaveFile *)__memAlloc(HEAP_MEM1_UNUSED, sizeof(SaveFile) * 16);
        msl::string::memcpy(saves, nandmgr_wp->saves, sizeof(SaveFile) * 4);
        __memFree(HEAP_MAIN, nandmgr_wp->saves);
        nandmgr_wp->saves = saves;
        for (s32 i = 4; i < 16; i++)
            nandClearSave(i);
    }

    void loadMainCtrlNew() {
        s32 selection = seq_load_sub_wp->curSelection;
        u32 pressed = wpadGetButtonsPressed(0);
        u32 held_rpt = wpadGetButtonsHeldRepeat(0);
        if (flag(pressed, WPAD_BTN_2)) {
            if (selection < 4) { // Selection is savefile
                SaveFile * saves = nandGetSaveFiles();
                if (flag(saves[seq_load_sub_wp->curSelection].flags, 0x2)) {
                    spsndSFXOn("SFX_SYS_SELECT_NG1");
                } else if (flag(saves[seq_load_sub_wp->curSelection].flags, 0x1)) {
                    spsndSFXOn("SFX_SYS_MENU_DESIDE1");
                    seq_load_sub_wp->state = 200;
                } else {
                    spsndSFXOn("SFX_SYS_MENU_DESIDE1");
                    seq_load_sub_wp->state = 300;
                }
            }
            switch (selection) {
            case SEQ_LOAD_BTN_ERASE:
                if (flag(seq_load_sub_wp->flags, 0x1)) {
                    spsndSFXOn("SFX_SYS_SELECT_NG1");
                } else {
                    spsndSFXOn("SFX_SYS_MENU_DESIDE1");
                    seq_load_sub_wp->state = 1000;
                    seq_load_sub_wp->curSelection = SEQ_LOAD_BTN_SAVEFILE_1;
                }
                break;
            case SEQ_LOAD_BTN_COPY:
                if (flag(seq_load_sub_wp->flags, 0x2)) {
                    spsndSFXOn("SFX_SYS_SELECT_NG1");
                } else {
                    spsndSFXOn("SFX_SYS_MENU_DESIDE1");
                    seq_load_sub_wp->state = 2000;
                    seq_load_sub_wp->curSelection = SEQ_LOAD_BTN_SAVEFILE_1;
                }
                break;
            case SEQ_LOAD_BTN_BACK:
                spsndSFXOn("SFX_SYS_MENU_DESIDE1");
                seq_load_sub_wp->state = 100;
                break;
            case SEQ_LOAD_BTN_PAGE_LEFT:
                spsndSFXOn("SFX_SYS_MENU_NEXT1");
                savepatch_wp->moveMode = MOVE_LEFT_1;
                savepatch_wp->startMoveMode = true;
                break;
            case SEQ_LOAD_BTN_PAGE_RIGHT:
                spsndSFXOn("SFX_SYS_MENU_NEXT1");
                savepatch_wp->moveMode = MOVE_RIGHT_1;
                savepatch_wp->startMoveMode = true;
                break;
            }
        } else if (flag(pressed, WPAD_BTN_1)) {
            spsndSFXOn("SFX_SYS_MENU_CANCEL1");
            seq_load_sub_wp->state = 100;
        } else {
            if (flag(held_rpt, WPAD_BTN_UP)) { // H-LEFT
                seq_load_sub_wp->curSelection = buttonDefs[seq_load_sub_wp->curSelection].nextSelLeft;
            } else if (flag(held_rpt, WPAD_BTN_DOWN)) { // H-RIGHT
                seq_load_sub_wp->curSelection = buttonDefs[seq_load_sub_wp->curSelection].nextSelRight;
            } else if (flag(held_rpt, WPAD_BTN_RIGHT)) { // H-UP
                seq_load_sub_wp->curSelection = buttonDefs[seq_load_sub_wp->curSelection].nextSelUp;
            } else if (flag(held_rpt, WPAD_BTN_LEFT)) { // H-DOWN
                seq_load_sub_wp->curSelection = buttonDefs[seq_load_sub_wp->curSelection].nextSelDown;
            }
            seq_load_sub_wp->destCursorPos = buttonDefs[seq_load_sub_wp->curSelection].cursorPos;
            if (selection != seq_load_sub_wp->curSelection) { // If selection changed this frame
                spsndSFXOn("SFX_SYS_MENU_CURSOR_MOVE1");
                seq_load_sub_wp->prevSelection = selection;
                seq_load_sub_wp->colorIntplProgress = 0;
                seq_load_sub_wp->lastBgGradientTop = seq_load_sub_wp->bgGradientTop;
                seq_load_sub_wp->lastBgGradientBottom = seq_load_sub_wp->bgGradientBottom;
            }
        }
        return;
    }

    void write_file_num_to_menu(Mtx34 mtx, s32 n) {
        char buf[16];
        s32 page = savepatch_wp->page;
        if (nandmgr_wp->task == 5) { // If loading new files, this display is ahead of prepared files, so resync
            if (seq_load_sub_wp->curSelection == SEQ_LOAD_BTN_PAGE_LEFT) {
                page += 1;
            } else {
                page -= 1;
            }
            if (page == -1) {
                page = 3;
            } else if (page == 4) {
                page = 0;
            }
        }
        msl::stdio::sprintf(buf, "File %d", (n + 1 + (page * 4)));
        FontDrawMessageMtx(mtx, buf);
        return;
    }

    void draw_arrows(f32 scale, Vec3 * pos, u32 flags, s32 type) {
        loadwinArrowMain();
        iconDispGx(scale, pos, flags, type);
        return;
    }

    void write_file_num_to_load_msg(evtmgr::EvtEntry * evt) {
        evt->lw[0] = (seq_load_sub_wp->curSelection + 1 + (savepatch_wp->page * 4));
        return;
    }

    void write_file_num_to_erase_msg(evtmgr::EvtEntry * evt) {
        evt->lw[0] = seq_load_sub_wp->curSelection;
        evt->lw[13] = (seq_load_sub_wp->curSelection + 1 + (savepatch_wp->page * 4));
        return;
    }

    SaveFile * write_file_num_to_copy_msg(evtmgr::EvtEntry * evt) {
        evt->lw[13] = (seq_load_sub_wp->saveCopyFrom + 1 + (savepatch_wp->page * 4));
        evt->lw[14] = (seq_load_sub_wp->saveCopyTo + 1 + (savepatch_wp->page * 4));
        return nandGetSaveFiles();
    }

    SeqLoadButtonDef * getBtn() {
        return &buttonDefs[seq_load_sub_wp->curSelection];
    }

    void move_mtx(Mtx34 mtx, f32 x, f32 y, f32 z) {
        Mtx34 mtxTrans;
        PSMTXTrans(mtx, x, y, z);
        PSMTXTrans(mtxTrans, savepatch_wp->xOffset, 0.0f, 0.0f);
        PSMTXConcat(mtx, mtxTrans, mtx);
        return;
    }

    void main(const char * name) {
        msl::string::memset(savepatch_wp, 0, sizeof(SavepatchWork));
        savepatch_wp->saveName = name;
        loadMainReal = patch::hookFunction(seq_load_sub_loadMain, seq_load_sub_chg_saves);
        // Read refs to custom saves instead of wiimarioXX saves
        writeBranchLink(nandWriteBannerLoadAllSavesMain, 0x42C, new_sprintf);
        writeBranchLink(nandWriteAllSavesMain, 0x184, new_sprintf);
        writeBranchLink(nandWriteAllSavesMain, 0x210, new_sprintf);
        writeBranchLink(nandWriteSaveMain, 0x16C, new_sprintf);
        writeBranchLink(nandDeleteSaveMain, 0x15C, new_sprintf);
        // Extend nandmgr savefile allocation space
        nandInitNew();
        // Create 16 saves instead of 4
        writeWord(nandWriteAllSavesMain, 0x458, CMPWI(0, 16));
        // "save file X" messages recognize files 5-16
        writeBranchLink(seq_load_sub_loadMain, 0x89C, write_file_num_to_load_msg);
        writeBranchLink(seq_load_sub_loadMain, 0xBA8, write_file_num_to_erase_msg);
        writeBranchLink(seq_load_sub_loadMain, 0x109C, write_file_num_to_copy_msg);
        // I really don't want to make evtpatch a dependency so we're gonna do evt patches like thiiiiis
        writeWord(delete_save_confirm_evt, 0x3C, LW(13));
        writeWord(delete_save_confirm_evt, 0xCC, LW(13));
        writeWord(copy_save_confirm_evt, 0x54, LW(14));
        writeWord(copy_save_confirm_evt, 0x120, LW(13));
        writeWord(copy_save_confirm_evt, 0x124, LW(14));
        // Implement new control scheme
        writeBranchLink(seq_load_sub_loadMain, 0x3E4, loadMainCtrlNew); // Replace first instruction of block with bl to new ctrl
        writeWord(seq_load_sub_loadMain, 0x3E8, B(0xD20));              // Jump to end of function (as if fade hasn't finished)
        // Replace button def references with new array
        writeBranchLink(windowRenderMain, 0xC4, getBtn);
        writeWord(windowRenderMain, 0xCC, MR(0, 3));
        // Draw arrow graphics
        writeBranchLink(windowRenderMain, 0x1DAC, draw_arrows);
        // Move graphics
        writeBranchLink(windowRenderMain, 0x1360, move_mtx);
        writeBranchLink(windowRenderMain, 0x138C, move_mtx);
        writeBranchLink(windowRenderMain, 0x13FC, move_mtx);
        writeBranchLink(windowRenderMain, 0x1488, move_mtx);
        writeBranchLink(windowRenderMain, 0x15AC, move_mtx);
        writeBranchLink(windowRenderMain, 0x1658, move_mtx);
        writeBranchLink(windowRenderMain, 0x16F4, move_mtx);
        writeBranchLink(windowRenderMain, 0x16B0, move_mtx);
        writeBranchLink(windowRenderMain, 0x17F8, move_mtx);
        writeBranchLink(windowRenderMain, 0x186C, move_mtx);
        writeBranchLink(windowRenderMain, 0x18D4, move_mtx);
        writeBranchLink(windowRenderMain, 0x1570, move_mtx);
        writeBranchLink(windowRenderMain, 0x1510, move_mtx);
        // Display accurate save file numbers (Forces English)
        writeWord(windowRenderMain, 0x14C4, MR(4, 17)); // Moves i to param 2, replacing the original string
        writeBranchLink(windowRenderMain, 0x14CC, write_file_num_to_menu);
    }

}
