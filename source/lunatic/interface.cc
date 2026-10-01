#include "mod.h"
#include "patch.h"
#include <common.h>
#include <customwin.h>
#include <evt_cmd.h>
#include <evtpatch.h>
#include <globalop.h>
#include <interface.h>
#include <lp_common.h>
#include <lunadrv.h>
#include <msgpatch.h>
#include <rfcdrv.h>
#include <tplpatch.h>
#include <util.h>
#include <ymetools.h>

#include "lunatic/localize.h"
#include <gen.h>
#include <rewrite.h>

#include <cutscene_helpers.h>
#include <msl/math.h>
#include <msl/stdio.h>
#include <msl/string.h>
#include <spm/animdrv.h>
#include <spm/bgdrv.h>
#include <spm/camdrv.h>
#include <spm/dispdrv.h>
#include <spm/eff/eff_fire.h>
#include <spm/eff/eff_small_star.h>
#include <spm/eff/eff_spm_confetti.h>
#include <spm/eff/eff_spm_hit.h>
#include <spm/eff/eff_spm_recovery.h>
#include <spm/eff/eff_spm_spindash.h>
#include <spm/eff/eff_zunbaba.h>
#include <spm/evt_cam.h>
#include <spm/evt_case.h>
#include <spm/evt_dimen.h>
#include <spm/evt_door.h>
#include <spm/evt_eff.h>
#include <spm/evt_fade.h>
#include <spm/evt_fairy.h>
#include <spm/evt_frame.h>
#include <spm/evt_guide.h>
#include <spm/evt_hit.h>
#include <spm/evt_img.h>
#include <spm/evt_item.h>
#include <spm/evt_map.h>
#include <spm/evt_mario.h>
#include <spm/evt_mobj.h>
#include <spm/evt_msg.h>
#include <spm/evt_npc.h>
#include <spm/evt_offscreen.h>
#include <spm/evt_paper.h>
#include <spm/evt_pouch.h>
#include <spm/evt_seq.h>
#include <spm/evt_shop.h>
#include <spm/evt_snd.h>
#include <spm/evt_sub.h>
#include <spm/evtmgr.h>
#include <spm/evtmgr_cmd.h>
#include <spm/fadedrv.h>
#include <spm/fontmgr.h>
#include <spm/framedrv.h>
#include <spm/hitdrv.h>
#include <spm/hud.h>
#include <spm/item_data.h>
#include <spm/itemdrv.h>
#include <spm/lz_embedded.h>
#include <spm/map_data.h>
#include <spm/mapdrv.h>
#include <spm/mario.h>
#include <spm/mario_fairy.h>
#include <spm/mario_hit.h>
#include <spm/mario_motion.h>
#include <spm/mario_pouch.h>
#include <spm/mario_status.h>
#include <spm/memory.h>
#include <spm/mobjdrv.h>
#include <spm/mot_damage.h>
#include <spm/mot_fairy_mario.h>
#include <spm/msgdrv.h>
#include <spm/npc_dimeen_l.h>
#include <spm/npc_ninja.h>
#include <spm/npc_tile.h>
#include <spm/npcdrv.h>
#include <spm/parse.h>
#include <spm/pausewin.h>
#include <spm/rel/aa1_01.h>
#include <spm/rel/dan.h>
#include <spm/rel/machi.h>
#include <spm/rel/mi4.h>
#include <spm/rel/relocatable_module.h>
#include <spm/seq_mapchange.h>
#include <spm/seq_title.h>
#include <spm/seqdef.h>
#include <spm/seqdrv.h>
#include <spm/spmario.h>
#include <spm/spmario_snd.h>
#include <spm/swdrv.h>
#include <spm/system.h>
#include <spm/temp_unk.h>
#include <spm/winmgr.h>
#include <spm/wpadmgr.h>
#include <wii/cx.h>
#include <wii/gx.h>
#include <wii/os/OSError.h>
#include <wii/tpl.h>
#include <wii/wpad.h>

namespace mod {
    using namespace spm;
    using namespace customwin;

    static void LPGUIDrawText(f32 x, f32 y, f32 scale, u8 alpha, wii::gx::GXColor color, bool rainbow, const char * msg) {
        if (alpha > 0)
            fontmgr::FontDrawStart_alpha(alpha);
        else
            fontmgr::FontDrawStart();
        fontmgr::FontDrawEdge();
        fontmgr::FontDrawColor(&color);
        fontmgr::FontDrawScale(scale);
        fontmgr::FontDrawNoiseOff();
        if (rainbow)
            fontmgr::FontDrawRainbowColor();
        else
            fontmgr::FontDrawRainbowColorOff();
        fontmgr::FontDrawString(x, y, msg);
        return;
    }

    void youSuckDisplay(f32 offset) {
        if (!Lunatic->Interface.youSuck)
            return;
        const char * youSuckText = "YOU SUCK";
        f32 scale = 3.69f;
        LPGUIDrawText(-((fontmgr::FontGetMessageWidth(youSuckText) * scale) / 2), (15.0f + (offset * 2.5f)), scale, 0, {255, 255, 255, 255}, true, youSuckText);
    }

    void new_dan_gameover() {
        Lunatic->Interface.youSuck = true;
    }

    void disorderDisplay(f32 offset) {
        f32 x = -350.0f;
        f32 y = 77.0f + offset + hud::hud_wp->flipTimeCells[0].pos;
        s32 disorderNum = (s32)Lunatic->Luna.disorder;
        if (disorderNum > 0) {
            wii::mtx::Vec3 position = {x, y, 0.0f};
            s32 mainIconId = (disorderNum - 1 + TPLPATCH_ICON(ICON_DISORDER_APATHY));
            icondrv::iconDispGxAlpha(1.0f, &position, 0x10, mainIconId, 255);
            icondrv::iconDispGxAlpha(1.0f, &position, 0x10, TPLPATCH_ICON(ICON_BORDER_DISORDER), 255);
        }
        s32 disorderRooms = Lunatic->Luna.DW.floorsRem;
        if (disorderRooms > 0) {
            wii::gx::GXColor funnyColor = {Lunatic->Luna.Disorder->mainCol.r, Lunatic->Luna.Disorder->mainCol.g, Lunatic->Luna.Disorder->mainCol.b, 255};
            char buffer[4];
            msl::stdio::sprintf(buffer, "%d", disorderRooms);
            const char * msg = buffer;
            LPGUIDrawText(x + 27.0f, y + 30.0f, 0.9f, 200, funnyColor, true, msg);
        }
    }

    void voucherDisplay(f32 offset) {
        f32 y = 47.0f;
        if ((swdrv::swByteGet(1) % 10) == 0)
            y += 25.0f;
        f32 x = 345.0f + offset;
        wii::mtx::Mtx34 mtxPos, mtxRot, mtxScale;
        for (s32 i = 0; i < VOUCHER_MAX; i += 1) {
            if (Lunatic->Voucher.Work[i] != nullptr) {
                if (Lunatic->Voucher.Work[i]->iconId != 0) {
                    // TODO: For Z axis rotation, look into PSVECNormalize
                    wii::mtx::PSMTXTrans(mtxPos, x, y, 0.0f);
                    wii::mtx::PSMTXScale(mtxScale, 1.0f, 1.0f, 1.0f);
                    wii::mtx::PSMTXRotRad((Lunatic->Voucher.Work[i]->iconRotation * PI / 180.0f), mtxRot, 0x79); // 'y'
                    wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
                    wii::mtx::PSMTXConcat(mtxPos, mtxRot, mtxPos);
                    icondrv::iconDispGxCol(mtxPos, 0x10, TPLPATCH_ICON(Lunatic->Voucher.Work[i]->iconId), {255, 255, 255, Lunatic->Voucher.Work[i]->iconAlpha});
                    y -= 44.0f;
                }
            }
        }
    }

    void chestKeyDisplay(f32 offset) {
        f32 y = 95.0f + offset;
        if ((swdrv::swByteGet(1) % 10) == 0)
            y += 25.0f;
        f32 x = 335.0f;
        wii::mtx::Mtx34 mtxPos, mtxScale;
        wii::mtx::PSMTXTrans(mtxPos, x, y, 0.0f);
        wii::mtx::PSMTXScale(mtxScale, 0.8f, 0.8f, 0.8f);
        wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
        icondrv::iconDispGxCol(mtxPos, 0x10, TPLPATCH_ICON(ICON_CHEST_KEY), {255, 255, 255, 255});
        char buffer[4];
        msl::stdio::sprintf(buffer, "%d", Lunatic->RFC.chestKeysOwned);
        const char * msg = buffer;
        LPGUIDrawText((x + 16.0f), (y + 24.0f), 0.8f, 0, {255, 135, 75, 255}, false, msg);
    }

    void floorDisplay(f32 x) {
        f32 y = -65.0f;
        Vec3 pos = {x, y, 0.0f};
        icondrv::iconDispGxAlpha(1.0f, &pos, 0x10, TPLPATCH_ICON(ICON_FLOOR), 255);
        pos.x += 60.0f;
        pos.y -= 0.5f;
        s32 numDisp[3] = {0, 0, 0};
        s32 numDigits = split(lpGetFloorNum(), numDisp);
        for (s32 i = 0; i < numDigits; i += 1) {
            if (numDisp[i] == 1 && i > 0)
                pos.x -= 3.0f;
            icondrv::iconDispGxAlpha(1.0f, &pos, 0x10, (s32)(icondrv::ICON_0_BIG + (numDisp[i] * 2)), 255);
            pos.x += 20.0f;
            if (numDisp[i] == 1)
                pos.x -= 3.0f;
        }
    }

    void critDisplay() {
        f32 slide = -100.0f;
        if (Lunatic->Interface.critDispStartDisp) {
            if (!Lunatic->Interface.critDisp) {
                Lunatic->Interface.critDisp = true;
                Lunatic->Interface.critDispProgress = 0;
            }
            slide += system::intplGetValue(system::INTPL_MODE_QUADRATIC_OUT, 0.0f, 200.0f, Lunatic->Interface.critDispProgress, 30);
            if (Lunatic->Interface.critDispProgress < 30)
                Lunatic->Interface.critDispProgress += 1;
        } else {
            if (Lunatic->Interface.critDisp) {
                Lunatic->Interface.critDisp = false;
                Lunatic->Interface.critDispProgress = 0;
                Lunatic->Interface.critDispSlideOutAdj = 200.0f;
            }
            slide -= system::intplGetValue(system::INTPL_MODE_QUADRATIC_IN, 0.0f, 200.0f, Lunatic->Interface.critDispProgress, 30);
            slide += Lunatic->Interface.critDispSlideOutAdj;
            if (Lunatic->Interface.critDispProgress < 30)
                Lunatic->Interface.critDispProgress += 1;
            else
                Lunatic->Interface.critDispSlideOutAdj = 0.0f;
        }
        f32 x = -440.0f + slide;
        f32 y = -20.0f;
        Vec3 pos = {x, y, 0.0f};
        if (Lunatic->Interface.floorDispOn)
            floorDisplay(x - 10.0f);
        icondrv::iconDispGxAlpha(0.8f, &pos, 0x10, TPLPATCH_ICON(ICON_LP_STATS), 255);
        // Draw text icons
        s32 cmDisp[4] = {0, 0, 0, 0};
        s32 crDisp[4] = {0, 0, 0, 0};
        s32 defDisp[4] = {0, 0, 0, 0};
        s32 drDisp[4] = {0, 0, 0, 0};
        s32 cmNumDigits = split((s32)Lunatic->Stats.CritMult, cmDisp);
        s32 crNumDigits = split(Lunatic->Stats.CritRate, crDisp);
        s32 defNumDigits = split(Lunatic->Stats.AegisDef, defDisp);
        s32 drNumDigits = split((s32)Lunatic->Stats.AuspiceDR, drDisp);
        pos.x += 13.0f;
        pos.y += 6.5f;
        f32 cmnX = pos.x;
        for (s32 i = 0; i < drNumDigits; i += 1) {
            icondrv::iconDispGxAlpha(0.7f, &pos, 0x10, (s32)(icondrv::ICON_0_BIG + (drDisp[i] * 2)), 255);
            pos.x += 14.5f;
            if (drDisp[i] == 1)
                pos.x -= 1.0f;
        }
        icondrv::iconDispGxAlpha(0.7f, &pos, 0x10, (s32)(TPLPATCH_ICON(ICON_PERCENT)), 255);
        pos.y += 40.0f;
        pos.x = cmnX;
        for (s32 i = 0; i < defNumDigits; i += 1) {
            icondrv::iconDispGxAlpha(0.7f, &pos, 0x10, (s32)(icondrv::ICON_0_BIG + (defDisp[i] * 2)), 255);
            pos.x += 14.5f;
            if (defDisp[i] == 1)
                pos.x -= 1.0f;
        }
        pos.y += 42.0f;
        pos.x = cmnX;
        for (s32 i = 0; i < crNumDigits; i += 1) {
            icondrv::iconDispGxAlpha(0.7f, &pos, 0x10, (s32)(icondrv::ICON_0_BIG + (crDisp[i] * 2)), 255);
            pos.x += 14.5f;
            if (crDisp[i] == 1)
                pos.x -= 1.0f;
        }
        icondrv::iconDispGxAlpha(0.7f, &pos, 0x10, (s32)(TPLPATCH_ICON(ICON_PERCENT)), 255);
        pos.y += 42.0f;
        pos.x = cmnX;
        for (s32 i = 0; i < cmNumDigits; i += 1) {
            icondrv::iconDispGxAlpha(0.7f, &pos, 0x10, (s32)(icondrv::ICON_0_BIG + (cmDisp[i] * 2)), 255);
            pos.x += 14.5f;
            if (cmDisp[i] == 1)
                pos.x -= 1.0f;
        }
        icondrv::iconDispGxAlpha(0.7f, &pos, 0x10, (s32)(TPLPATCH_ICON(ICON_PERCENT)), 255);
        return;
    }

    mario::MarioWork * danDisplay() {
        if (msl::string::strstr(spmario::gp->mapName, "dan") != nullptr) {
            f32 offset = hud::hud_wp->basePos.y;
            disorderDisplay(offset);
            voucherDisplay(offset);
            chestKeyDisplay(offset);
            youSuckDisplay(offset);
            critDisplay();
        }
        return mario::marioGetPtr();
    }

    static seqdef::SeqFunc * seq_titleMainReal;
    void seq_titleMainOverride(seqdrv::SeqWork * wp) {
        const char * msg = MOD_VERSION;
        const char * debugMsg = "LP Debug Mode activated! ^w^";
        wii::gx::GXColor notgreen = {230, 116, 216, 255};
        f32 scale = 0.8f;
        f32 y = 200.0f;
        f32 x;
        if (!DebugMode) {
            x = -((fontmgr::FontGetMessageWidth(msg) * scale) / 2);
            LPGUIDrawText(x, y, scale, 0, notgreen, false, msg);
        } else {
            x = -((fontmgr::FontGetMessageWidth(debugMsg) * scale) / 2);
            LPGUIDrawText(x, y, scale, 0, notgreen, true, debugMsg);
        }
        // Hotkey to enable debug mode
        if (!DebugMode && (wpadmgr::wpadGetButtonsHeld(0) & WPAD_BTN_Z) != 0 && (wpadmgr::wpadGetButtonsPressed(0) & WPAD_BTN_B) != 0) {
            spmario_snd::spsndSFXOn("SFX_I_BRUNK_APPEAR1");
            lpEnableDebugMode();
        }
        seq_titleMainReal(wp);
    }

    void danYouSuck() {
        patch::hookFunction(dan::danCountdownDone, new_dan_gameover);
    }

    s32 LPGUIShowHideStats(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        // s32 showHide = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        // showHide == 1 ? Lunatic->Interface.critDispStartDisp = true : Lunatic->Interface.critDispStartDisp = false;
        Lunatic->Interface.critDispStartDisp = (bool)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        Lunatic->Interface.floorDispOn = (bool)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        return 2;
    }

    bool DoNothing(winmgr::WinmgrSelect * sel) {
        (void)sel;
        return false;
    }

    s32 LPGUIUnpause(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        pausewin::pausewinUnpauseGame();
        hud::hudUnhide();
        return 2;
    }
    EVT_DECLARE_USER_FUNC(LPGUIUnpause, 0)

    s32 GetEffectInfo(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 type = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        switch (type) {
        case 0: // Blessing
            break;
        case 1: // Curse
            break;
        case 2: // Disorder
            if (Lunatic->Luna.disorder > 0) {
                evtmgr_cmd::evtSetValue(evtEntry, args[2], (s32)Lunatic->Luna.disorder - 1 + TPLPATCH_ICON(ICON_DISORDER_APATHY));
                evtmgr_cmd::evtSetValue(evtEntry, args[3], (s32)Lunatic->Luna.Disorder->name);
                msl::string::memset(Lunatic->Interface.aeDescBuf, 0, sizeof(Lunatic->Interface.aeDescBuf));
                LeyLineDisorder * Disorders = (LeyLineDisorder *)DisorderDataGetPtr();
                switch (Lunatic->Luna.disorder) {
                case DISORDER_RED:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[0].desc, Lunatic->Luna.DW.UW.Apathy.dispMaxHPDecrease, Lunatic->Luna.DW.UW.Apathy.dispEnemyHPIncrease, Lunatic->Luna.DW.UW.Apathy.enemyDamageIncrease, Lunatic->Luna.DW.UW.Apathy.marioDamageDecrease);
                    break;
                case DISORDER_ORANGE:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[1].desc, Lunatic->Luna.DW.UW.Dread.dispBlockChance);
                    break;
                case DISORDER_YELLOW:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[2].desc, Lunatic->Luna.DW.UW.Prejudice.dispInstantCoinLoss, Lunatic->Luna.DW.UW.Prejudice.coinLossChance, Lunatic->Luna.DW.UW.Prejudice.coinThreshold);
                    break;
                case DISORDER_GREEN:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[3].desc, Lunatic->Luna.DW.UW.Indifference.repeat);
                    break;
                case DISORDER_CYAN:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[4].desc, Lunatic->Luna.DW.UW.Recalcitrance.dispXPMult, Lunatic->Luna.DW.UW.Recalcitrance.dispHealingNerf);
                    break;
                case DISORDER_BLUE:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[5].desc, Lunatic->Luna.DW.UW.Depravity.allLv4FloorThreshold);
                    break;
                case DISORDER_PURPLE:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[6].desc, Lunatic->Luna.DW.UW.Indolence.attackEffectChance, Lunatic->Luna.DW.UW.Indolence.slowDuration);
                    break;
                case DISORDER_WHITE:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[7].desc);
                    break;
                case DISORDER_BLACK:
                    msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, Disorders[8].desc);
                    break;
                default:
                    break;
                }
                evtmgr_cmd::evtSetValue(evtEntry, args[4], (s32)Lunatic->Interface.aeDescBuf);
                evtmgr_cmd::evtSetValue(evtEntry, args[5], (s32)&Lunatic->Luna.Disorder->textDrawCol);
            }
            evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)Lunatic->Luna.disorder);
            break;
        case 3: // Voucher
            RFCItemData * RFC_SpecialItems = (RFCItemData *)RFCSpecialGetPtr();
            s32 i = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
            VoucherWork * Voucher = Lunatic->Voucher.Work[i];
            if (Voucher != nullptr) {
                s32 itemId = Voucher->itemId;
                evtmgr_cmd::evtSetValue(evtEntry, args[2], TPLPATCH_ICON((s32)Voucher->iconId));
                evtmgr_cmd::evtSetValue(evtEntry, args[3], (s32)RFC_SpecialItems[itemId].name);
                msl::string::memset(Lunatic->Interface.aeDescBuf, 0, sizeof(Lunatic->Interface.aeDescBuf));
                msl::stdio::sprintf(Lunatic->Interface.aeDescBuf, RFC_SpecialItems[itemId].description, VoucherGetTearChance(VoucherTearChances[itemId]), VoucherGuaranteeTrigs[itemId]);
                evtmgr_cmd::evtSetValue(evtEntry, args[4], (s32)Lunatic->Interface.aeDescBuf);
                evtmgr_cmd::evtSetValue(evtEntry, args[5], (s32)&RFC_SpecialItems[itemId].textDrawCol);
                break;
            } else
                evtmgr_cmd::evtSetValue(evtEntry, args[2], 0);
            break;
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(GetEffectInfo, 6)

    EVT_BEGIN(LPGUIActiveEffects)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 255, 128, 1000)
    INLINE_EVT()
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(evt_map::evt_map_set_blend, 1, LW(0), LW(0), LW(0), 255)
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    END_INLINE()
    WAIT_MSEC(500)
    USER_FUNC(LPGUIShowHideStats, 1, 1)
    USER_FUNC(EvtCWSelectEntry, PTR("Active"), CWSELECT_DEFAULT, PTR("Active Effects"), PTR(""), 0, 0)
    USER_FUNC(EvtCWSelectHideDescWindow, PTR("Active"))
    USER_FUNC(EvtCWSelectOverrideBtnBehavior, PTR("Active"), BTN_2, PTR(DoNothing))
    // Disorder: effect type, disorder id, icon id, name, description, text col
    USER_FUNC(GetEffectInfo, 2, LW(2), LW(5), LW(6), LW(7), LW(9))
    IF_LARGE(LW(2), 0)
    SET(LW(8), 1)
    USER_FUNC(EvtCWSelectAddListing, PTR("Active"), LW(6), LW(7), LW(5), 0, 0, LW(9))
    END_IF()
    // Voucher: effect type, voucher idx, icon id, name, description, text col
    SET(LW(2), 0)
    DO(VOUCHER_MAX)
    USER_FUNC(GetEffectInfo, 3, LW(2), LW(5), LW(6), LW(7), LW(9))
    IF_LARGE(LW(5), 0)
    SET(LW(8), 1)
    USER_FUNC(EvtCWSelectAddListing, PTR("Active"), LW(6), LW(7), LW(5), 0, 0, LW(9))
    END_IF()
    ADD(LW(2), 1)
    WHILE()
    IF_EQUAL(LW(8), 0) // If 1, indicates that a listing was created
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(activeConditionsNone), 0, 0)
    ELSE()
    USER_FUNC(EvtCWSelectMenuStart, PTR("Active"), 0, 0)
    USER_FUNC(EvtCWSelectReset)
    END_IF()
    USER_FUNC(EvtCWSelectDelete, PTR("Active"))
    USER_FUNC(LPGUIShowHideStats, 0, 1)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 128, 255, 500)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(evt_map::evt_map_set_blend, 1, LW(0), LW(0), LW(0), 255)
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    USER_FUNC(LPGUIUnpause)
    RETURN()
    EVT_END()

    void danInterfaceMain() {
        mario::MarioWork * mario = mario::marioGetPtr();
        // Handle opening the config menu
        if (((mario->buttonsPressed & WPAD_BTN_B) == WPAD_BTN_B) && ((mario->buttonsHeld & WPAD_BTN_Z) != WPAD_BTN_Z)) // B pressed, Z not held
        {
            bool canPause = mario_motion::func_80146f0c();
            bool keyOff = mario::marioKeyOffChk();
            bool ctrlOff = mario::marioCtrlOffChk();
            bool noFade = fadedrv::fadeIsFinish();
            if (noFade && !keyOff && !ctrlOff && canPause && msl::string::strstr(spmario::gp->mapName, "dan") != nullptr && !(mario->motionId >= 31 && mario->motionId <= 33)) {
                pausewin::pausewinPauseGame();
                hud::hudHide();
                evtmgr::evtEntryType(LPGUIActiveEffects, 0, 0, 0);
            }
        }
    }

    void guiOverrides() {
        seq_titleMainReal = seqdef::seq_data[seqdrv::SEQ_TITLE].main;
        seqdef::seq_data[seqdrv::SEQ_TITLE].main = &seq_titleMainOverride;
        writeBranchLink(hud::hudDisp, 0x54, danDisplay);
        globalop::globalopAddEntry((void *)danInterfaceMain, nullptr);
    }
}
