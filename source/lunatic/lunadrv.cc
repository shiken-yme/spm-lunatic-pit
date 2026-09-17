#include "lunatic/localize.h"
#include "mod.h"
#include "patch.h"
#include <common.h>
#include <customwin.h>
#include <effpatch.h>
#include <evt_cmd.h>
#include <evtpatch.h>
#include <gen.h>
#include <lp_common.h>
#include <msgpatch.h>
#include <tplpatch.h>
#include <util.h>

#include <msl/math.h>
#include <msl/stdio.h>
#include <msl/string.h>
#include <spm/animdrv.h>
#include <spm/camdrv.h>
#include <spm/dispdrv.h>
#include <spm/evt_cam.h>
#include <spm/evt_door.h>
#include <spm/evt_eff.h>
#include <spm/evt_fade.h>
#include <spm/evt_fairy.h>
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
#include <spm/fontmgr.h>
#include <spm/framedrv.h>
#include <spm/gxsub.h>
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
#include <spm/npc_tile.h>
#include <spm/npcdrv.h>
#include <spm/parse.h>
#include <spm/pausewin.h>
#include <spm/rel/dan.h>
#include <spm/rel/machi.h>
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
#include <wii/cx.h>
#include <wii/gx.h>
#include <wii/os/OSError.h>
#include <wii/tpl.h>
#include <wii/wpad.h>

namespace mod {
    /*
        lunadrv contains most code for the Merluna-focused features, Disorders and Divine Judgement
        Now just how are Disorders related to Merluna?... That's a secret, hehehe~ :3
        Divine Judgement doesn't have code here yet because I yet to reimplement it. One day!!
    */

    using namespace spm;

    void ApathySet() {
        ApathyWork * wp = &Lunatic->Luna.DW.UW.Apathy;
        msl::string::memset(wp, 0, sizeof(ApathyWork));
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            wp->marioHpMult = 0.1;
            wp->enemyDamageIncrease = 1;
            wp->marioDamageDecrease = 0;
            wp->enemyMaxHPMult = 1.25;
            wp->dispMaxHPDecrease = 10;
            wp->dispEnemyHPIncrease = 0;
            break;
        case 1:
            wp->marioHpMult = 0.2;
            wp->enemyDamageIncrease = 2;
            wp->marioDamageDecrease = 1;
            wp->enemyMaxHPMult = 1.5;
            wp->dispMaxHPDecrease = 20;
            wp->dispEnemyHPIncrease = 20;
            break;
        case 2:
            wp->marioHpMult = 0.33;
            wp->enemyDamageIncrease = 2;
            wp->marioDamageDecrease = 1;
            wp->enemyMaxHPMult = 1.75;
            wp->dispMaxHPDecrease = 33;
            wp->dispEnemyHPIncrease = 50;
            break;
        default:
            wp->marioHpMult = 0.5;
            wp->enemyDamageIncrease = 3;
            wp->marioDamageDecrease = 2;
            wp->enemyMaxHPMult = 2.0;
            wp->dispMaxHPDecrease = 50;
            wp->dispEnemyHPIncrease = 100;
            break;
        }
        wp->storedHP = msl::math::floor((f32)mario_pouch::pouchGetPtr()->maxHp * wp->marioHpMult);
        wp->storedCritRate = (Lunatic->Stats.CritRate / 2);
        wp->storedCritMult = msl::math::floor(Lunatic->Stats.CritMult / 2.0);
        lpAddHp(-wp->storedHP, 0);
        lpAddCrit(-wp->storedCritRate, -wp->storedCritMult);
        return;
    }

    void ApathyClear() {
        ApathyWork * wp = &Lunatic->Luna.DW.UW.Apathy;
        lpAddHp(wp->storedHP, wp->storedHP);
        lpAddCrit(wp->storedCritRate, wp->storedCritMult);
        return;
    }

    void DreadSet() {
        DreadWork * wp = &Lunatic->Luna.DW.UW.Dread;
        msl::string::memset(wp, 0, sizeof(DreadWork));
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            wp->dispBlockChance = 10;
            break;
        case 1:
            wp->dispBlockChance = 20;
            break;
        case 2:
            wp->dispBlockChance = 30;
            break;
        default:
            wp->dispBlockChance = 50;
            break;
        }
        return;
    }

    void PrejudiceSet() {
        PrejudiceWork * wp = &Lunatic->Luna.DW.UW.Prejudice;
        msl::string::memset(wp, 0, sizeof(PrejudiceWork));
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            wp->dispInstantCoinLoss = 10;
            wp->coinLossChance = 30;
            break;
        case 1:
            wp->dispInstantCoinLoss = 15;
            wp->coinLossChance = 50;
            break;
        case 2:
            wp->dispInstantCoinLoss = 20;
            wp->coinLossChance = 70;
            break;
        default:
            wp->dispInstantCoinLoss = 25;
            wp->coinLossChance = 100;
            break;
        }
        wp->coinThreshold = (s32)(msl::math::floor((f32)pouch->coins * 0.15) + 1);
        return;
    }

    void PrejudiceAction() {
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        s32 coinsLost;
        f32 mod = (f32)(system::rand() % 11 + (Lunatic->Luna.DW.UW.Prejudice.dispInstantCoinLoss - 5));
        mod /= 100;
        coinsLost = (s32)msl::math::floor(pouch->coins * mod);
        if (hud::hud_wp->countdownTimer == 299) {
            if (pouch->coins > 0 && coinsLost != 0)
                pouch->coins -= coinsLost;
        } else if (hud::hud_wp->countdownTimer < 295) {
            s32 odds = system::rand() % 100;
            if (pouch->coins > 0 && odds < Lunatic->Luna.DW.UW.Prejudice.coinLossChance)
                pouch->coins -= 1;
        }
        return;
    }

    void IndifferenceSet() {
        IndifferenceWork * wp = &Lunatic->Luna.DW.UW.Indifference;
        msl::string::memset(wp, 0, sizeof(IndifferenceWork));
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            wp->repeat = 1;
            break;
        case 1:
            wp->repeat = 1;
            break;
        case 2:
            wp->repeat = 2;
            break;
        default:
            wp->repeat = 3;
            break;
        }
        return;
    }

    s32 indiffItems[] = {ITEM_ID_USE_OBAKE_KINOKO, ITEM_ID_USE_DOKU_KINOKO, ITEM_ID_USE_DOKU_KINOKO, ITEM_ID_COOK_NURU_ESSENCE, ITEM_ID_COOK_BOMB_EGG,
                         ITEM_ID_COOK_BOMB_EGG, ITEM_ID_COOK_TRIAL_PAN, ITEM_ID_COOK_DANGEROUS_COOKING, ITEM_ID_COOK_NORMAL_CHOKO, ITEM_ID_COOK_NORMAL_CHOKO,
                         ITEM_ID_COOK_GERORIN_FOOD, ITEM_ID_COOK_GERORIN_FOOD};

    s32 IndifferenceAction(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 i, idx;
        s32 itemsSpawned = 0;
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        for (i = 0; i < 8; i += 1)
            evtmgr_cmd::evtSetValue(evtEntry, args[i], 0);
        for (i = 0; i < Lunatic->Luna.DW.UW.Indifference.repeat; i += 1) {
            s32 odds = system::rand() % 100;
            if (odds < 67 || i == 0) {
                idx = system::rand() % (sizeof(indiffItems) / 4);
                if ((mario_pouch::pouchCountUseItems() + itemsSpawned) < 10) {
                    itemsSpawned += 1;
                    evtmgr_cmd::evtSetValue(evtEntry, args[(i * 2)], indiffItems[idx]);
                } else {
                    s32 invIdx = system::rand() % 10;
                    evtmgr_cmd::evtSetValue(evtEntry, args[(i * 2)], (s32)msgdrv::msgSearch(item_data::itemDataTable[pouch->useItem[invIdx]].nameMsg));
                    pouch->useItem[invIdx] = indiffItems[idx];
                    evtmgr_cmd::evtSetValue(evtEntry, args[((i * 2) + 1)], (s32)msgdrv::msgSearch(item_data::itemDataTable[pouch->useItem[invIdx]].nameMsg));
                }
            }
        }
        return 2;
    }

    EVT_BEGIN(dan_disorder_indifference_dialogue)
    // LW(9) is the original item, LW(10) is the replacement
    IF_NOT_EQUAL(LW(10), 0) // If the replacement is set, then continue/assert LW(9) is the original item
    ADD(LW(11), 1)
    IF_EQUAL(LW(11), 1)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(disorderIndifferenceItemNotif), 0, 0, LW(9), LW(10))
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print_add_insert, 1, PTR(disorderIndifferenceItemNotif2), LW(9), LW(10))
    END_IF()
    WAIT_MSEC(1000)
    END_IF()
    RETURN()
    EVT_END()

    EVT_BEGIN(dan_disorder_indifference)
    // Floors rem can be 0 only if preId is non-zero.
    USER_FUNC(DisorderGetFloorsRem, LW(5))
    IF_EQUAL(LW(5), 0)
    RETURN()
    END_IF()
    USER_FUNC(IndifferenceAction, LW(1), LW(2), LW(3), LW(4), LW(5), LW(6), LW(7), LW(8))
    // Spawn items that don't have replacements
    IF_EQUAL(LW(2), 0)
    IF_NOT_EQUAL(LW(1), 0)
    USER_FUNC(evt_item::evt_item_entry, PTR("IN1"), LW(1), 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("IN1"), 0x8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("IN1"))
    END_IF()
    END_IF()
    IF_EQUAL(LW(4), 0)
    IF_NOT_EQUAL(LW(3), 0)
    USER_FUNC(evt_item::evt_item_entry, PTR("IN2"), LW(3), 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("IN2"), 0x8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("IN2"))
    END_IF()
    END_IF()
    IF_EQUAL(LW(6), 0)
    IF_NOT_EQUAL(LW(5), 0)
    USER_FUNC(evt_item::evt_item_entry, PTR("IN3"), LW(5), 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("IN3"), 0x8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("IN3"))
    END_IF()
    END_IF()
    IF_EQUAL(LW(8), 0)
    IF_NOT_EQUAL(LW(7), 0)
    USER_FUNC(evt_item::evt_item_entry, PTR("IN4"), LW(7), 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("IN4"), 0x8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("IN4"))
    END_IF()
    END_IF()
    // Iterate through items to see if they were forced into the inventory
    SET(LW(11), 0)
    SET(LW(9), LW(1))
    SET(LW(10), LW(2))
    RUN_CHILD_EVT(dan_disorder_indifference_dialogue)
    SET(LW(9), LW(3))
    SET(LW(10), LW(4))
    RUN_CHILD_EVT(dan_disorder_indifference_dialogue)
    SET(LW(9), LW(5))
    SET(LW(10), LW(6))
    RUN_CHILD_EVT(dan_disorder_indifference_dialogue)
    SET(LW(9), LW(7))
    SET(LW(10), LW(8))
    RUN_CHILD_EVT(dan_disorder_indifference_dialogue)
    IF_LARGE(LW(11), 0)
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR("\n<k>"))
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_IF()
    RETURN()
    EVT_END()

    void RecalcitranceSet() {
        RecalcitranceWork * wp = &Lunatic->Luna.DW.UW.Recalcitrance;
        msl::string::memset(wp, 0, sizeof(RecalcitranceWork));
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            wp->dispXPMult = -50;
            wp->dispHealingNerf = 10;
            break;
        case 1:
            wp->dispXPMult = -100;
            wp->dispHealingNerf = 25;
            break;
        case 2:
            wp->dispXPMult = -200;
            wp->dispHealingNerf = 50;
            break;
        default:
            wp->dispXPMult = -300;
            wp->dispHealingNerf = 80;
            break;
        }
        return;
    }

    s32 RecalcitranceCalcHealing(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        f32 healing = (f32)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        healing -= (healing * ((f32)Lunatic->Luna.DW.UW.Recalcitrance.dispHealingNerf / 100.0f));
        evtmgr_cmd::evtSetValue(evtEntry, args[1], round(abs_value(healing)));
        return 2;
    }

    void DepravitySet() {
        DepravityWork * wp = &Lunatic->Luna.DW.UW.Depravity;
        msl::string::memset(wp, 0, sizeof(DepravityWork));
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            wp->allLv4FloorThreshold = 80;
            break;
        case 1:
            wp->allLv4FloorThreshold = 60;
            break;
        case 2:
            wp->allLv4FloorThreshold = 40;
            break;
        default:
            wp->allLv4FloorThreshold = 1;
            break;
        }
        return;
    }

    s32 DepravityGetAllLv4Threshold(s32 difficulty) {
        // This can't be part of the DepravityWork struct because work is only initialized after the color takes effect
        // This variable is needed before the first room displays at all
        // Consider reworking disorder init to use preId when setting, then alloc work before display?
        s32 ret;
        switch (difficulty) {
        case 0:
            ret = 180;
            break;
        case 1:
            ret = 160;
            break;
        case 2:
            ret = 40;
            break;
        default:
            ret = 20;
            break;
        }
        return ret;
    }

    bool DepravityCheckActive() {
        if ((Lunatic->Luna.disorder == DISORDER_BLUE && Lunatic->Luna.DW.floorsRem != 0) || Lunatic->Luna.DW.preId == 6)
            return true;
        return false;
    }

    s32 DepravityAction(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        s32 currentFloor = swdrv::swByteGet(1);
        if (DepravityCheckActive() == true) {
            DanGen_Enemies(currentFloor, true);
            DanGen_Enemies_Apply();
        }
        return 2;
    }

    void IndolenceSet() {
        IndolenceWork * wp = &Lunatic->Luna.DW.UW.Indolence;
        msl::string::memset(wp, 0, sizeof(IndolenceWork));
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            wp->attackEffectChance = 15;
            wp->slowDuration = 5;
            break;
        case 1:
            wp->attackEffectChance = 25;
            wp->slowDuration = 5;
            break;
        case 2:
            wp->attackEffectChance = 35;
            wp->slowDuration = 10;
            break;
        default:
            wp->attackEffectChance = 50;
            wp->slowDuration = 10;
            break;
        }
        spmario::gp->gameSpeed = 0.82f;
        return;
    }

    void IndolenceClear() {
        spmario::gp->gameSpeed = 1.0f;
        return;
    }

    void MelancholySet() {
        MelancholyWork * wp = &Lunatic->Luna.DW.UW.Melancholy;
        msl::string::memset(wp, 0, sizeof(MelancholyWork));
        wp->subtimer = 10;
        return;
    }

    void MelancholyAction() {
        MelancholyWork * wp = &Lunatic->Luna.DW.UW.Melancholy;
        if (hud::hud_wp->countdownTimer >= 300) {
            return;
        } else if (wp->subtimer > 1) {
            wp->subtimer -= 1;
        } else if (wp->stacks < 5) {
            wp->stacks += 1;
            wp->subtimer = 10;
            effdrv::EffEntry * eff = temp_unk::effSpmVoltEntry(0, 0.33f, lpMakeEffTarget(effdrv::TARGET_MARIO), -1);
            wii::gx::GXColor black = {0, 0, 0, 255};
            effpatch::effpatchColorMaskEntry(eff, black, black, nullptr);
            wii::mtx::Vec3 * pos = &mario::marioGetPtr()->position;
            spmario_snd::spsndSFXOn_3D("SFX_I_THUNDER1", pos);
            spmario_snd::spsndSFXOn_3D("SFX_I_BIRIBIRI2", pos);
            spmario_snd::spsndSFXOn_3D("SFX_EVT_HELWANWAN_POWERUP1", pos);
        } else {
            wp->subtimer = 10;
        }
        return;
    }

    // Establish global data tables for Blessings, Curses, and Disorders

    DivineJudgement Blessings[MERLUNA_PARAMITA] =
        {
            {spectreName, {120, 0}, 0, nullptr, nullptr},
            {houraiName, {0, 0}, 0, nullptr, nullptr},
            {paramitaName, {0, 0}, 0, nullptr, nullptr}};

    DivineJudgement Curses[MERLUNA_MIGRAINE] =
        {
            {shionName, {0, 0}, 0, nullptr, nullptr},
            {hexName, {0, 0}, 0, nullptr, nullptr},
            {migraineName, {0, 0}, 0, nullptr, nullptr}};

    LeyLineDisorder Disorders[DISORDER_BLACK] =
        {
            {apathyName, apathyDesc, apathyIntro, apathyIntro2, {255, 0, 0, 30}, {255, 0, 0, 50}, {60, 0, 0, 255}, 20, 0.7, 3000, ApathySet, ApathyClear},                                     // APATHY
            {dreadName, dreadDesc, dreadIntro, dreadIntro2, {255, 128, 0, 30}, {255, 128, 0, 65}, {60, 30, 0, 255}, 40, 1.2, 5000, DreadSet, nullptr},                                         // DREAD
            {prejudiceName, prejudiceDesc, prejudiceIntro, prejudiceIntro2, {255, 255, 0, 30}, {255, 255, 0, 50}, {60, 60, 0, 255}, 15, 0.7, 4000, PrejudiceSet, nullptr},                     // PREJUDICE
            {indifferenceName, indifferenceDesc, indifferenceIntro, indifferenceIntro2, {0, 255, 0, 20}, {0, 255, 0, 40}, {0, 60, 0, 255}, 20, 1.0, 4500, IndifferenceSet, nullptr},           // INDIFFERENCE
            {recalcitranceName, recalcitranceDesc, recalcitranceIntro, recalcitranceIntro2, {0, 255, 255, 20}, {0, 255, 225, 40}, {0, 60, 60, 255}, 30, 1.2, 4200, RecalcitranceSet, nullptr}, // RECALCITRANCE
            {depravityName, depravityDesc, depravityIntro, depravityIntro2, {0, 0, 255, 30}, {0, 0, 255, 50}, {0, 0, 60, 255}, 50, 1.3, 6900, DepravitySet, nullptr},                          // DEPRAVITY
            {indolenceName, indolenceDesc, indolenceIntro, indolenceIntro2, {128, 0, 255, 30}, {128, 0, 225, 50}, {30, 0, 60, 255}, 10, 0.5, 2000, IndolenceSet, IndolenceClear},              // INDOLENCE
            {melancholyName, melancholyDesc, melancholyIntro, nullptr, {255, 255, 255, 30}, {225, 225, 225, 225}, {60, 60, 60, 255}, 20, 0.8, 4200, MelancholySet, nullptr},                   // MELANCHOLY
            {ruinName, ruinDesc, nullptr, nullptr, {0, 0, 0, 60}, {0, 0, 0, 255}, {10, 10, 10, 255}, 75, 1.5, 6666, nullptr, nullptr}                                                          // RUIN
    };

    void * DisorderDataGetPtr() {
        return &Disorders[0];
    }

    void SetDisorderSub(DisorderId id) {
        Lunatic->Luna.disorder = id;
        Lunatic->Luna.DW.preId = 0;
        Lunatic->Luna.DW.floorsRem = 5;
        Lunatic->Luna.Disorder = &Disorders[(s32)id - 1];
        if (Lunatic->Luna.Disorder->SetFunc != nullptr)
            (Lunatic->Luna.Disorder->SetFunc)();
        return;
    }

    void ClearDisorderSub(s32 id) {
        (void)id; // may need this again one day, functionality stripped at present
        if (Lunatic->Luna.Disorder->ClearFunc != nullptr)
            (Lunatic->Luna.Disorder->ClearFunc)();
        Lunatic->Luna.disorder = DISORDER_NULL;
        Lunatic->Luna.Disorder = nullptr;
        return;
    }

    s32 DisorderGetId(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)Lunatic->Luna.disorder);
        //    evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)Lunatic->Luna.DW.preId);
        return 2;
    }

    s32 DisorderGetPreId(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], Lunatic->Luna.DW.preId);
        return 2;
    }

    s32 DisorderGetFloorsRem(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)Lunatic->Luna.DW.floorsRem);
        return 2;
    }

    s32 DisorderDraw(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        if (Lunatic->Luna.disorder == DISORDER_NULL || Lunatic->Luna.DW.tremorState == 4)
            return 0;
        wii::gx::GXColor color;
        u8 red = Disorders[Lunatic->Luna.disorder - 1].mainCol.r;
        u8 green = Disorders[Lunatic->Luna.disorder - 1].mainCol.g;
        u8 blue = Disorders[Lunatic->Luna.disorder - 1].mainCol.b;
        u8 alpha = Disorders[Lunatic->Luna.disorder - 1].mainCol.a;
        switch (Lunatic->Luna.DW.tremorState) {
        case 1:                                          // Tremor fadein
            if (Lunatic->Luna.DW.tremorIntplFrmMax == 0) // Init variables
            {
                Lunatic->Luna.DW.tremorIntplFrmMax = (s32)msl::math::floor((f32)(Lunatic->Luna.DW.finalShakeTime / 1000 * 60));
                Lunatic->Luna.DW.tremorIntplFrmTimer = 0;
                Lunatic->Luna.DW.intplProgressMax = 60;
                Lunatic->Luna.DW.intplProgress = 0;
            }
            red = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.r, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.r, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            green = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.g, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.g, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            blue = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.b, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.b, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            alpha = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.a, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.a, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            Lunatic->Luna.DW.tremorIntplFrmTimer += 1;
            Lunatic->Luna.DW.intplProgress += 1;
            if (Lunatic->Luna.DW.tremorIntplFrmTimer >= 60) {
                Lunatic->Luna.DW.intplProgress = 0;
                Lunatic->Luna.DW.tremorState = 2;
            }
            break;
        case 2: // Stay at severecol
            red = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.r, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.r, Lunatic->Luna.DW.intplProgressMax, Lunatic->Luna.DW.intplProgressMax);
            green = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.g, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.g, Lunatic->Luna.DW.intplProgressMax, Lunatic->Luna.DW.intplProgressMax);
            blue = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.b, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.b, Lunatic->Luna.DW.intplProgressMax, Lunatic->Luna.DW.intplProgressMax);
            alpha = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.a, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.a, Lunatic->Luna.DW.intplProgressMax, Lunatic->Luna.DW.intplProgressMax);
            Lunatic->Luna.DW.tremorIntplFrmTimer += 1;
            if ((Lunatic->Luna.DW.tremorIntplFrmMax - Lunatic->Luna.DW.tremorIntplFrmTimer) <= 60) // Check if should fade back to normal
                Lunatic->Luna.DW.tremorState = 3;
            break;
        case 3: // Tremor fadeout
            red = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.r, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.r, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            green = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.g, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.g, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            blue = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.b, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.b, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            alpha = system::intplGetValue(system::INTPL_MODE_LINEAR, (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.a, (f32)Disorders[Lunatic->Luna.disorder - 1].severeCol.a, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            red = Disorders[Lunatic->Luna.disorder - 1].severeCol.r - red + (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.r;
            green = Disorders[Lunatic->Luna.disorder - 1].severeCol.g - green + (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.g;
            blue = Disorders[Lunatic->Luna.disorder - 1].severeCol.b - blue + (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.b;
            alpha = Disorders[Lunatic->Luna.disorder - 1].severeCol.a - alpha + (f32)Disorders[Lunatic->Luna.disorder - 1].mainCol.a;
            Lunatic->Luna.DW.intplProgress += 1;
            Lunatic->Luna.DW.tremorIntplFrmTimer += 1;
            if (Lunatic->Luna.DW.tremorIntplFrmTimer == Lunatic->Luna.DW.tremorIntplFrmMax) {
                Lunatic->Luna.DW.tremorIntplFrmMax = 0;
                Lunatic->Luna.DW.tremorIntplFrmTimer = 0;
                Lunatic->Luna.DW.tremorState = 0;
                Lunatic->Luna.DW.intplProgress = 0;
                Lunatic->Luna.DW.intplProgressMax = 0;
            }
        }
        color = {red, green, blue, alpha};
        mapdrv::mapSetColor(&color);
        return 0;
    }

    s32 DisorderSetColor(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 id = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 clear = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        f32 red, green, blue, alpha;
        if (firstRun) {
            Lunatic->Luna.DW.tremorState = 4;
            Lunatic->Luna.DW.intplProgress = 0;
            Lunatic->Luna.DW.intplProgressMax = 100;
        }
        if (Lunatic->Luna.DW.intplProgress < Lunatic->Luna.DW.intplProgressMax) {
            red = system::intplGetValue(system::INTPL_MODE_LINEAR, 0.0f, (f32)Disorders[id - 1].mainCol.r, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            green = system::intplGetValue(system::INTPL_MODE_LINEAR, 0.0f, (f32)Disorders[id - 1].mainCol.g, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            blue = system::intplGetValue(system::INTPL_MODE_LINEAR, 0.0f, (f32)Disorders[id - 1].mainCol.b, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            alpha = system::intplGetValue(system::INTPL_MODE_LINEAR, 0.0f, (f32)Disorders[id - 1].mainCol.a, Lunatic->Luna.DW.intplProgress, Lunatic->Luna.DW.intplProgressMax);
            if (clear == 1) {
                red = -(red - Disorders[id - 1].mainCol.r);
                green = -(green - Disorders[id - 1].mainCol.g);
                blue = -(blue - Disorders[id - 1].mainCol.b);
                alpha = -(alpha - Disorders[id - 1].mainCol.a);
            }
            wii::gx::GXColor color = {(u8)red, (u8)green, (u8)blue, (u8)alpha};
            mapdrv::mapSetColor(&color);
            Lunatic->Luna.DW.intplProgress += 1;
            // DisorderDraw will attempt to draw after this interpolation ends & before ClearDisorderSub, so we set disorder to null directly beforehand
            if (Lunatic->Luna.DW.intplProgress == 99)
                Lunatic->Luna.disorder = DISORDER_NULL;
            return 0;
        } else {
            Lunatic->Luna.DW.intplProgressMax = 0;
            Lunatic->Luna.DW.intplProgress = 0;
            if (clear == 0) {
                SetDisorderSub((DisorderId)id);
            } else
                ClearDisorderSub(id);
            Lunatic->Luna.DW.tremorState = 0;
            return 2;
        }
    }
    EVT_DECLARE_USER_FUNC(DisorderSetColor, 2)

    s32 DisorderGetIntroText(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 id = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        if (id <= 0) {
            evtmgr_cmd::evtSetValue(evtEntry, args[1], 0);
            return 2;
        }
        s32 gswf = 1680 + id;
        const char * msg = (const char *)0;
        if (swdrv::swGet(gswf) == false) {
            msg = Disorders[id - 1].introMsg;
            swdrv::swSet(gswf);
        } else if (Lunatic->Luna.DW.floorsRem > 0) {
            msg = Disorders[id - 1].introMsg2;
        }
        evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)msg);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(DisorderGetIntroText, 2)

    EVT_BEGIN(EvtDisorderInitOrClear)
    USER_FUNC(evt_door::evt_door_wait_flag, 0x100)
    /*DO(0)
    USER_FUNC(evt_mario::evt_mario_get_flags, 0, LW(0))
    IF_NOT_FLAG(LW(0), 0x8)
    DO_BREAK()
    END_IF()
    WAIT_FRM(1)
    WHILE()*/
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    WAIT_MSEC(300)
    INLINE_EVT()
    WAIT_MSEC(1000)
    USER_FUNC(DisorderSetColor, LW(10), LW(11))
    END_INLINE()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("E_3"), 0)
    USER_FUNC(evt_snd::evt_snd_sfxon, PTR("SFX_EVT_QUAKE1L"))
    USER_FUNC(evt_snd::evt_snd_get_last_sfx_id, LW(0))
    IF_EQUAL(GSWF(1630), 0) // Lighter camera tremors ACTIVE
    DIVF(LW(12), 4)
    END_IF()
    USER_FUNC(evt_cam::evt_cam_shake, camdrv::CAM_ID_3D, LW(12), LW(12), FLOAT(0.0), 3000, 0)
    USER_FUNC(evt_snd::evt_snd_sfxoff, LW(0))
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_7"), 0)
    WAIT_MSEC(700)
    USER_FUNC(DisorderGetId, LW(5))
    IF_LARGE(LW(5), 0)
    USER_FUNC(DisorderGetIntroText, LW(5), LW(6))
    IF_NOT_EQUAL(LW(6), 0)
    USER_FUNC(evt_msg::evt_msg_print, 1, LW(6), 0, 0)
    END_IF()
    END_IF()
    IF_EQUAL(GSWF(1661), 0)
    SET(GSWF(1661), 1)
    USER_FUNC(MsgIconReplaceIdx, 3, (s32)(TPLPATCH_ICON(ICON_B)))
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(disorderIntro), 0, 0)
    USER_FUNC(MsgIconReplaceIdx, 3, (s32)icondrv::ICON_BTN_1)
    WAIT_MSEC(300)
    END_IF()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    WAIT_MSEC(300)
    /*IF_EQUAL(LW(5), 4)
    RUN_CHILD_EVT(dan_disorder_indifference)
    END_IF()*/
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    EVT_END()

    s32 EvtDisorderReleaseTremor(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        f32 intensity = evtmgr_cmd::evtGetFloat(evtEntry, args[0]);
        camdrv::CamEntry * cam = camdrv::camPtrTbl[camdrv::CAM_ID_3D];
        if (cam->shakeIntensity.x == intensity && cam->shakeIntensity.y == intensity && cam->shakeDuration >= 1900.0f) // Rough check to ensure shake params are correct
        {
            Lunatic->Luna.DW.tremorState = 1;
            return 2;
        }
        return 0;
    }
    EVT_DECLARE_USER_FUNC(EvtDisorderReleaseTremor, 1)

    EVT_BEGIN(EvtDisorderTremor)
    USER_FUNC(evt_snd::evt_snd_sfxon, PTR("SFX_EVT_QUAKE1L"))
    USER_FUNC(evt_snd::evt_snd_get_last_sfx_id, LW(0))
    IF_EQUAL(GSWF(1630), 0) // Lighter camera tremors ACTIVE
    DIVF(LW(12), 4)
    END_IF()
    INLINE_EVT()
    USER_FUNC(EvtDisorderReleaseTremor, LW(12))
    END_INLINE()
    USER_FUNC(evt_cam::evt_cam_shake, camdrv::CAM_ID_3D, LW(12), LW(12), FLOAT(0.0), LW(10), 0)
    USER_FUNC(evt_snd::evt_snd_sfxoff, LW(0))
    RETURN()
    EVT_END()

    s32 SetDisorder(s32 id, evtmgr::EvtEntry * evtEntry) {
        evtmgr::EvtEntry * evt = evtmgr::evtChildEntry(evtEntry, EvtDisorderInitOrClear, 0);
        evt->lw[10] = (s32)id;
        evt->lw[11] = 0;
        evt->lw[12] = FLOAT(1.2);
        return evt->id;
    }

    s32 ClearDisorder(s32 id, evtmgr::EvtEntry * evtEntry) {
        evtmgr::EvtEntry * evt = evtmgr::evtChildEntry(evtEntry, EvtDisorderInitOrClear, 0);
        evt->lw[10] = (s32)id;
        evt->lw[11] = 1;
        evt->lw[12] = FLOAT(0.6);
        return evt->id;
    }

    void DecideDisorder(s32 num, s32 difficulty) {
        // Check for debug mode first
        s32 debugDisorderId = swdrv::swByteGet(1660);
        if (DebugMode && debugDisorderId != 255 && debugDisorderId > 0) {
            Lunatic->Luna.DW.preId = debugDisorderId;
            swdrv::swByteSet(1660, 0);
            return;
        }
        // Roll through each difficulty to decide whether or not to set a disorder
        s32 compare;
        s32 currentFloor = swdrv::swByteGet(1);
        s32 forcedDisorder = -1;
        switch (difficulty) {
        case 0:
            compare = 60;
            break;
        case 1:
            compare = 90;
            break;
        case 2:
            compare = 150;
            break;
        default:
            compare = 350;
            break;
        }
        if (difficulty == 2 && currentFloor >= 189) {
            forcedDisorder = DISORDER_WHITE;
            compare = 350;
            if ((currentFloor % 10) == 3)
                compare = 1000;
        }
        if (num == 999)
            num = system::irand(999); // Please don't ask
        if (num < compare) {
            s32 disorderRNG = -1;
            for (s32 i = 0; i < 50; i += 1) { // Block Prejudice before Floor 20 & prevent disorders from reoccurring in a run
                if (forcedDisorder == -1)
                    disorderRNG = system::rand() % DISORDER_PURPLE + 1;
                else
                    disorderRNG = forcedDisorder;
                if ((disorderRNG == DISORDER_YELLOW && currentFloor < 19) || Lunatic->Luna.DW.disorderTrig[disorderRNG - 1] == true)
                    continue;
                Lunatic->Luna.DW.preId = disorderRNG;
                Lunatic->Luna.DW.disorderTrig[disorderRNG - 1] = true;
                break;
            }
        }
        return;
    }

    void DisorderHandleTremors() {
        s32 odds = system::rand() % 1000;
        if (odds < Lunatic->Luna.Disorder->shakeOdds && Lunatic->Luna.DW.tremorState == 0) {
            s32 time = (Lunatic->Luna.Disorder->shakeTime - ((s32)msl::math::floor(system::rand() % (Lunatic->Luna.Disorder->shakeTime / 2))));
            if (time < 2000)
                time = 2000;
            Lunatic->Luna.DW.finalShakeTime = time;
            evtmgr::EvtEntry * evt = evtmgr::evtEntryType(EvtDisorderTremor, 0, 0, 0);
            evt->lw[10] = time;
            evt->lw[12] = FLOAT(Lunatic->Luna.Disorder->shakeStrength);
        }
        return;
    }
}
