#include <common.h>
#include <cutscene_helpers.h>
#include <evt_cmd.h>
#include <evtpatch.h>
#include <gen.h>
#include <globalop.h>
#include <lp_common.h>
#include <lunadrv.h>
#include <lunatic/localize.h>
#include <mod.h>
#include <rewrite.h>
#include <rfcdrv.h>
#include <tplpatch.h>
#include <util.h>

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
#include <spm/mario_sbr.h>
#include <spm/mario_status.h>
#include <spm/memory.h>
#include <spm/mobjdrv.h>
#include <spm/mot_damage.h>
#include <spm/mot_fairy_mario.h>
#include <spm/mot_jump.h>
#include <spm/mot_walk.h>
#include <spm/msgdrv.h>
#include <spm/npc_dimeen_l.h>
#include <spm/npc_ninja.h>
#include <spm/npc_tile.h>
#include <spm/npc_zunbaba.h>
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
#include <wii/kpad.h>
#include <wii/os/OSError.h>
#include <wii/tpl.h>
#include <wii/wpad.h>

namespace mod {
    /*
        This file contains important EVT rewrites and proprietary user funcs for said EVTs
        I have yet to put a lot of stuff from mod.cc into here. I should do that one day
    */

    using namespace spm;

    const char * restFloorNpcNames[] = {"NONE", "FLMM", "BDIN", "MLNA", "CHST", "MVER", "GBBI", "DMAN"};

    npcdrv::NPCTribeAnimDef _moverAnims[] = {
        {0, "stg2_syuuzin_b_S_1"}, // Idle
        {1, "stg2_syuuzin_b_W_1"}, // Walking
        {2, "stg2_syuuzin_b_R_1"}, // Running
        {3, "stg2_syuuzin_b_T_1"}, // Talking (Idle)
        {-1, "stg2_syuuzin_b_Z_1"}};

    using namespace spm::npcdrv;

    s32 evt_dan_try_disorder(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        // Clear disorder tremor effects on mapchange
        Lunatic->Luna.DW.tremorIntplFrmMax = 0;
        Lunatic->Luna.DW.tremorIntplFrmTimer = 0;
        Lunatic->Luna.DW.tremorState = 0;
        Lunatic->Luna.DW.intplProgress = 0;
        Lunatic->Luna.DW.intplProgressMax = 0;
        // Roll for Disorders, else decrement floorsRem
        s32 currentFloor = swdrv::swByteGet(1);
        s32 currentFloorLastDigit = currentFloor % 10;
        if (Lunatic->Luna.disorder == DISORDER_NULL && Lunatic->Luna.DW.floorsRem == 0 && currentFloorLastDigit < 4) {
            s32 difficulty = lpGetDifficulty();
            DecideDisorder(Lunatic->Mover.moverRNG, difficulty);
        } else if (Lunatic->Luna.DW.floorsRem > 0)
            Lunatic->Luna.DW.floorsRem -= 1;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_dan_try_disorder, 0)

    s32 evt_dan_disorder_set_or_clear(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        s32 evtId = -1;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        if (Lunatic->Luna.DW.floorsRem == 0) {
            if (Lunatic->Luna.disorder > DISORDER_NULL) {
                evtId = ClearDisorder((s32)Lunatic->Luna.disorder, evtEntry);
            } else if (Lunatic->Luna.DW.preId > 0)
                evtId = SetDisorder(Lunatic->Luna.DW.preId, evtEntry);
        }
        evtmgr_cmd::evtSetValue(evtEntry, args[0], evtId);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_dan_disorder_set_or_clear, 1)

    s32 LunaticForceBGMChange(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 variant = evtmgr_cmd::evtGetValue(evtEntry, args[0]); // 1-4
        spmario_snd::BgmEntry copy = *spmario_snd::spsndBgmPlayers[0].bgmEntry;
        copy.brsarIdx = variant + 1379;
        msl::string::memcpy(spmario_snd::spsndBgmPlayers[0].bgmEntry, &copy, sizeof(copy));
        spmario_snd::spsndBgmPlayers[0].flags |= 1;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(LunaticForceBGMChange, 1)

    // Like so many other functions used in this mod, this was adapted heavily from decomp dan.c
    // Thank you Seeky! This mod and many others would not exist without your work.
    // You are greatly appreciated by all of us in the SPM modding and reverse engineering community.
    s32 evt_dan_handle_key_failsafe_new(evtmgr::EvtEntry * entry, bool isFirstCall) {
        (void)isFirstCall;
        // Check whether the key exists anywhere
        if (!dan::danCheckEnemyInMapBbox() && !dan::danCheckKeyInMapBbox() &&
            !mario_pouch::pouchCheckHaveItem(48) &&
            !itemdrv::itemCheckForId(48)) {
            // Spawn the key at Mario's position if not
            mario::MarioWork * mario = mario::marioGetPtr();
            itemdrv::itemEntry(NULL, 48, 1, mario->position.x, mario->position.y, mario->position.z, NULL, 0);
            return 2;
        } else
            return 0;
    }
    EVT_DECLARE_USER_FUNC(evt_dan_handle_key_failsafe_new, 0)

    void danAssignSpecialEnemyItem(npcdrv::NPCEntry * npc, s32 useItemDropChance, s32 chestKeyDropChance) {
        npcdrv::NPCDropItem * dropItems = npcdrv::npcGetTribe(npc->tribeId)->dropItemList;
        s32 sup = system::rand() % 100;
        if (sup < useItemDropChance) {
            if (npc->dropItemId == 0 && dropItems[0].itemId != 0) // If it doesn't already have an item, continue
            {
                s32 itemId = -1;
                s32 i;
                // determines dropItems length
                for (i = 0; itemId != 0; i += 1)
                    itemId = dropItems[i].itemId;
                do {
                    itemId = system::rand() % i;
                    npc->dropItemId = dropItems[itemId].itemId;
                    if (npc->dropItemId == ITEM_ID_USE_BLANK_KUN) {
                        s32 sup2 = system::rand() % 100;
                        if (sup2 < 60)
                            npc->dropItemId = ITEM_ID_USE_SHINABITA_KINOKO; // 60% chance to replace Catch Card drops with Dried Shrooms
                    }
                } while (npc->dropItemId <= 0);
            }
        } else if (sup < (useItemDropChance + chestKeyDropChance) && npc->dropItemId == 0)
            npc->dropItemId = ITEM_ID_KEY_MAC_KEY_00;
        return;
    }

    s32 evt_dan_modify_enemy(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[1], 0);
        npcdrv::NPCEntry * npc = npcdrv::npcNameToPtr_NoAssert((const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]));
        s32 sup = system::rand() % 100;
        s32 stellarDiff = 0, remOdds = 0;
        s32 currentFloor = swdrv::swByteGet(1);
        // Nerf all random item drops
        s32 difficulty = lpGetDifficulty();
        if (npc->dropItemId != ITEM_ID_KEY_DAN_KEY || npc->dropItemId != ITEM_ID_KEY_MAC_KEY_00) {
            if (VoucherGetStateById(VOUCHER_STELLAR) == V_ACTIVE)
                stellarDiff = 40;
            switch (difficulty) {
            case 0:
                remOdds = 30;
                break;
            case 1:
                remOdds = 65;
                break;
            case 2:
                remOdds = 85;
                break;
            }
            if (sup < (remOdds - stellarDiff)) {
                npc->dropItemId = 0;
            } else if (VoucherGetStateById(VOUCHER_GREEN) == V_ACTIVE) {
                sup = system::rand() % 100;
                if (sup < 7) // 7% chance to drop a Fried Egg if no other drop is active
                    npc->dropItemId = ITEM_ID_COOK_SUNNY_SIDE_UP;
            }
        }
        if (Lunatic->Luna.disorder == DISORDER_RED && Lunatic->Luna.DW.floorsRem != 0) // APATHY
        {
            npc->maxHp = (u32)msl::math::floor((f32)npc->maxHp * Lunatic->Luna.DW.UW.Apathy.enemyMaxHPMult);
            npc->hp = npc->maxHp;
        }
        /*
            Create a holographic enemy
            Base 10% chance for any enemy spawned past Floor 50 on Normal-Hard modes to be holographic
            Enemy must have a base attack strength of 3+ OR max hp of 10+
            Enemy must not have a tribe that makes the holo effect look buggy or not appear
            Shlurp-type and Koopa-type enemies are excluded because being holo does not significantly affect them
            Todo: maybe try and remove kick behavior from holo koopa-type enemies?
        */
        sup = system::rand() % 100;
        if (sup < (difficulty * 4)) {
            if ((s32)npc != 0 && npc->templateKouraKickScript == 0 && currentFloor > 149 && npc->tribeId != NPC_SHLORP && npc->tribeId != NPC_SHLURP) {
                npcMakeHolo(npc);
                danAssignSpecialEnemyItem(npc, (s32)msl::math::sqrt((f32)npc->maxHp), 0);
                evtmgr_cmd::evtSetValue(evtEntry, args[1], 1);
            }
        } else {
            sup = system::rand() % 100;
            if (sup < 4 && currentFloor > 174 && difficulty >= 2 && npc->tribeId != NPC_BOO && npc->tribeId != NPC_DARK_BOO && npc->tribeId != NPC_DARK_DARK_BOO) {
                npcMakeNegative(npc);
                danAssignSpecialEnemyItem(npc, npc->maxHp * 3, 3);
                evtmgr_cmd::evtSetValue(evtEntry, args[1], 2);
            }
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_dan_modify_enemy, 2)

    // heavily adapted from dan.c decomp, thanks again Seeky!
    s32 evt_dan_distribute_keys(evtmgr::EvtEntry * entry, bool isFirstCall) {
        (void)isFirstCall;
        (void)entry;
        s32 i = 0, n = 0, j = 0, k = 0, currentFloor = swdrv::swByteGet(1), phase = 0, enemiesInCycle = 0, difficulty = lpGetDifficulty();
        bool assign = false, randomKeyAssigned = false;
        npcdrv::NPCWork * npcWp = npcdrv::npcGetWorkPtr();
        NPCEntry * curNpc = npcWp->entries;
        s32 enemyCount = 0;
        NPCEntry * enemies[80];
        if ((currentFloor % 10) == 0) {
            msl::string::memset(&Lunatic->RFC.chestKeysToSpawn[0], 0xff, 4);
            for (i = 0; i < 2; i += 1) {
            rerollFloor:
                u8 rand = (u8)(system::rand() % 9);
                assign = true;
                for (j = 0; j < 4; j += 1) {
                    if (rand == Lunatic->RFC.chestKeysToSpawn[j])
                        assign = false;
                }
                if (assign)
                    Lunatic->RFC.chestKeysToSpawn[i] = rand;
                else
                    goto rerollFloor;
            }
            OSREPORTF("Guaranteed chest keys for this cycle @ rooms ending in %d, %d\n", Lunatic->RFC.chestKeysToSpawn[0] + 1, Lunatic->RFC.chestKeysToSpawn[1] + 1);
        }
        // Create list of enemies to give keys in the current room
        for (i = 0; i < npcWp->num; curNpc++, i++) {
            if (CHECK_ANY_MASK(curNpc->flag8, 0x1) && !CHECK_ANY_MASK(curNpc->flag8, 0x40000))
                enemies[enemyCount++] = curNpc;
        }
        // Give floor key on 1st run, maybe chest key on 2nd
        for (i = 0; i < 2; ++i) {
            n = 0;
        buh:
            n += 1;
            if (n > 50) // Failsafe
                return 2;
            s32 random = system::rand() % enemyCount;
            if (npcCheckDanFlag(enemies[random], (DAN_NPC_HOLOGRAPHIC | DAN_NPC_NEGATIVE)) == true) // Block holographic and negative enemies
                goto buh;
            if (i == 0) { // Distribute main floor key
                enemies[random]->dropItemId = item_data::ITEM_ID_KEY_DAN_KEY;
            } else // Distribute chest key
            {
                if (enemies[random]->dropItemId == item_data::ITEM_ID_KEY_DAN_KEY || enemies[random]->dropItemId == item_data::ITEM_ID_KEY_MAC_KEY_00)
                    goto buh;
                assign = false;
                for (j = 0; j < 4; j += 1) {
                    if ((u8)(currentFloor % 10) == Lunatic->RFC.chestKeysToSpawn[j])
                        assign = true;
                }
                if (assign) // Assign guaranteed chest key if this floor is queued to have one
                    enemies[random]->dropItemId = item_data::ITEM_ID_KEY_MAC_KEY_00;
                if (assign || Lunatic->Luna.disorder == DISORDER_BLUE)
                    return 2;
                // Calculate enemiesInCycle
                for (j = 0; j < 10; j += 1) {
                    phase = (currentFloor / 10) % 10;
                    currentFloor = (phase * 10) + j;
                    for (k = 0; k < Lunatic->Floor[currentFloor].enemyTypes; k += 1)
                        enemiesInCycle += Lunatic->Floor[currentFloor].Enemies[k].num;
                }
                // OSREPORTF("Enemies in current cycle: %d\n", enemiesInCycle);
                // If a floor does not have a guaranteed chest key, small chance for any enemy to be assigned one
                for (j = 0; j < enemyCount; j += 1) {
                    /*
                        If 100 enemies in a 10-floor phase,
                        16/12/8/4 in 1000 chance for an enemy to drop a random key
                    */
                    s32 odds = system::rand() % (enemiesInCycle * 10);
                    if (odds < ((4 - difficulty) * 4) && !randomKeyAssigned) // Spawn chance = (1 / # enemies in this cycle) repeated enemyCount times
                    {
                        enemies[random]->dropItemId = item_data::ITEM_ID_KEY_MAC_KEY_00;
                        randomKeyAssigned = true;
                    }
                }
            }
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_dan_distribute_keys, 0)

    EVT_BEGIN(homogenize_lock_interact)
    USER_FUNC(evt_mario::evt_mario_key_off, 0)
    IF_EQUAL(GSWF(1620), 1)
    USER_FUNC(evt_pouch::evt_pouch_check_have_item, 48, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
    USER_FUNC(evt_sub::evt_sub_item_select_menu, 0, PTR(&dan::dan_flipsideLockItems), LW(0), 0)
    IF_NOT_EQUAL(LW(0), 48)
    USER_FUNC(evt_mobj::evt_mobj_exec_cancel, PTR("me"))
    END_IF()
    END_IF()
    END_IF()
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    EVT_END()

    EVT_BEGIN(dan_enemy_room_init_evt_new)
    SET(GSW(1623), 4) // Set rest floor NPC to "undetermined" for crash report purposes
    SET(LW(0), GSW(1))
    IF_EQUAL(LW(0), 0)
    USER_FUNC(evt_dan_init_lunatic)
    END_IF()
    USER_FUNC(set_mover_rng)
    USER_FUNC(get_mover_rng, LW(1)) // Movers
    IF_SMALL_EQUAL(LW(1), 14)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("mover"), PTR("n_stg2_syuuzin_b"), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("mover"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(_moverAnims))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("mover"), 0, true)
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("mover"))
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("mover"), -40, 0, 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("mover"), 9, PTR(fwd_mover_speech))
    ELSE()
    USER_FUNC(evt_dan_try_disorder)
    INLINE_EVT()
    USER_FUNC(DisorderDraw)
    END_INLINE()
    END_IF()
    USER_FUNC(dan::evt_dan_read_data)
    USER_FUNC(dan::evt_dan_handle_map_parts, LW(0))
    USER_FUNC(dan::evt_dan_handle_dokans, LW(0))
    USER_FUNC(evt_door::evt_door_set_dokan_descs, PTR(&dan::dan_dokanDescs), 8)
    USER_FUNC(DisorderGetId, LW(1))
    USER_FUNC(DisorderGetPreId, LW(2))
    IF_EQUAL(LW(1), 6)
    USER_FUNC(DepravityAction)
    ELSE()
    IF_EQUAL(LW(2), 6)
    USER_FUNC(DepravityAction)
    END_IF()
    END_IF()
    SET(LW(1), 0)
    USER_FUNC(dan::evt_dan_handle_doors, LW(0), LW(1), LW(10), LW(11), LW(2), LW(3), LW(4))
    USER_FUNC(evt_door::evt_door_set_map_door_descs, PTR(&dan::dan_mapDoorDescs), 2)
    USER_FUNC(evt_door::evt_door_enable_disable_map_door_desc, 0, LW(10))
    USER_FUNC(evt_door::evt_door_enable_disable_map_door_desc, 0, LW(11))
    USER_FUNC(evt_mobj::evt_mobj_zyo, PTR("lock_00"), 48, LW(2), LW(3), LW(4), 0, PTR(homogenize_lock_interact), PTR(dan::dan_lock_open_evt), 0) // Only one lock type
    USER_FUNC(dan::evt_dan_make_spawn_table, LW(0))
    SET(LW(10), 0)
    SET(LW(9), 0)
    DO(16)
    USER_FUNC(dan::evt_dan_get_enemy_info, LW(0), LW(10), LW(11), LW(12))
    IF_LARGE(LW(12), 0)
    DO(LW(12))
    USER_FUNC(dan::evt_dan_get_enemy_spawn_pos, LW(9), LW(0), LW(10), LW(13), LW(14), LW(15))
    ADD(LW(9), 1)
    USER_FUNC(evt_npc::evt_npc_entry_from_template, 0, LW(11), LW(13), LW(14), LW(15), LW(5), EVT_NULLPTR)
    USER_FUNC(evt_dan_modify_enemy, LW(5), LW(6))
    SWITCH(LW(6))
    CASE_EQUAL(1)
    USER_FUNC(evt_npc::evt_npc_set_animpose_disp_callback, LW(5), PTR(mi4::mi4MimiHolographicEffect), 0)
    IF_EQUAL(GSWF(1605), 0)
    SET(GSWF(1605), 1)
    SET(LSWF(6), 1)
    END_IF()
    CASE_EQUAL(2)
    USER_FUNC(evt_npc::evt_npc_set_animpose_disp_callback, LW(5), PTR(DanEnemyNegativeDispCb), 0)
    IF_EQUAL(GSWF(1606), 0)
    SET(GSWF(1606), 1)
    SET(LSWF(7), 1)
    END_IF()
    END_SWITCH()
    WHILE()
    END_IF()
    ADD(LW(10), 1)
    WHILE()
    USER_FUNC(evt_dan_distribute_keys)
    /*IF_EQUAL(GSW(1601), 1) // Tatarian Aster
    USER_FUNC(rand100, LW(8))
    IF_SMALL(LW(8), 70)
    USER_FUNC(dan::evt_dan_decide_key_enemy, 48)
    END_IF()
    ELSE()
    USER_FUNC(dan::evt_dan_decide_key_enemy, 48)
    END_IF()*/
    USER_FUNC(evt_npc::evt_npc_freeze_all)
    USER_FUNC(evt_hit::evt_hitobj_attr_onoff, 1, 1, PTR("A2"), 1073741824)
    USER_FUNC(evt_hit::evt_hitobj_attr_onoff, 1, 1, PTR("A3"), 536870912)
    USER_FUNC(evt_map::evt_mapobj_flag_onoff, 1, 0, PTR("S"), 2)
    USER_FUNC(evt_map::evt_mapobj_flag4_onoff, 1, 1, PTR("S"), 16)
    RUN_CHILD_EVT(evt_door::door_init_evt)
    ADD(GSW(1), 1)
    RUN_CHILD_EVT(custom_pit_music)
    USER_FUNC(evt_snd::evt_snd_set_sfx_reverb_mode, 0)
    IF_SMALL(GSW(1602), 1)
    USER_FUNC(handleBlessingWearOff)
    END_IF()
    INLINE_EVT()
    USER_FUNC(evt_door::evt_door_wait_flag, 256)
    USER_FUNC(evt_sub::evt_sub_display_room_name, 1, 6)
    END_INLINE()
    USER_FUNC(dan::evt_dan_start_countdown)
    INLINE_EVT()
    USER_FUNC(evt_door::evt_door_wait_flag, 256)
    USER_FUNC(evt_lp_get_difficulty, LW(5))
    IF_EQUAL(LW(5), 2)
    IF_EQUAL(GSWF(1603), 0)
    SET(GSWF(1603), 1)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    // Customwin msg test
    // USER_FUNC(customwin::EvtCWMsgPrint, PTR("Test"), 1, PTR(hardDifficultyFirst), 0, 0)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(hardDifficultyFirst), 0, 0)
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_IF()
    END_IF()
    IF_EQUAL(LSWF(6), 1)
    SET(LSWF(6), 0)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(holoEnemyIntro), 0, 0)
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_IF()
    IF_EQUAL(LSWF(7), 1)
    SET(LSWF(7), 0)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(negativeEnemyIntro), 0, 0)
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_IF()
    USER_FUNC(evt_dan_disorder_set_or_clear, LW(5))
    IF_NOT_EQUAL(LW(5), -1)
    DO(0)
    CHK_EVT(LW(5), LW(2))
    IF_EQUAL(LW(2), 0)
    DO_BREAK()
    END_IF()
    WAIT_FRM(1)
    WHILE()
    END_IF()
    USER_FUNC(DisorderGetId, LW(5))
    IF_EQUAL(LW(5), 4)
    RUN_CHILD_EVT(dan_disorder_indifference)
    END_IF()
    USER_FUNC(evt_npc::evt_npc_unfreeze_all)
    USER_FUNC(evt_mario_set_invincibility, FLOAT(20), 1)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 255, 0, 1000)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(dan::evt_dan_get_door_names, LW(2), LW(3))
    USER_FUNC(evt_map::evt_mapobj_color, 1, LW(2), 255, 255, 255, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    USER_FUNC(evt_map::evt_mapobj_flag_onoff, 1, 1, LW(2), 1)
    END_INLINE()
    INLINE_EVT()
    USER_FUNC(evt_dan_handle_key_failsafe_new) // Completely overhauls key despawn behavior
    END_INLINE()
    USER_FUNC(EvtVoucherProc, VOUCHER_CAKE)
    USER_FUNC(evt_sub::func_800d4de4, 1, 0)
    RETURN()
    EVT_END()

    // Patches Dimentio to have a dynamic movement zone rather than being hardcoded for one room.
    s32 dimen_determine_move_pos_new(evtmgr::EvtEntry * entry, bool isFirstCall) {
        mario::MarioWork * marioWork = mario::marioGetPtr();
        npcdrv::NPCEntry * npc = (npcdrv::NPCEntry *)entry->ownerNPC;
        double destYPos = 0;
        f32 marioZ = ((marioWork->position).z);
        f32 destXPos = 0;
        u32 dimenMoveRand = 0;
        wii::mtx::Vec3 min;
        wii::mtx::Vec3 max;
        hitdrv::hitGetMapEntryBbox(0, &min, &max);
        s32 i = 0;
        do {
            while (true) {
                do {
                    i = i + 1;
                    dimenMoveRand = system::irand(400);
                    destXPos = ((marioWork->position).x + (f32)dimenMoveRand - 200);
                    if (i > 50) {
                        destXPos = npc->position.x;
                        goto outOfBounds;
                    }
                } while ((destXPos <= (min.x + 25)) || ((max.x - 25) <= destXPos));
            outOfBounds:
                u32 yMoveBehavior = system::irand(100);
                if (yMoveBehavior < 67) {
                    dimenMoveRand = system::irand(4);
                    destYPos = (10.0 * (f32)dimenMoveRand + 20.0);
                } else {
                    dimenMoveRand = system::irand(3);
                    destYPos = (32.0 * (f32)dimenMoveRand + 40.0);
                }
                if (npc->flippedTo3d != 0)
                    break;
                if ((100.0 < abs_value((destXPos - (marioWork->position).x))) || (80.0 < destYPos))
                    goto setFloats;
            }
            destYPos = system::distABf(destXPos, marioZ, ((marioWork->position).x), marioZ);
        } while ((destYPos <= 120.0) && (destYPos <= 80.0));
    setFloats:
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)entry->pCurData;
        evtmgr_cmd::evtSetFloat(entry, args[0], destXPos);
        evtmgr_cmd::evtSetFloat(entry, args[1], destYPos);
        evtmgr_cmd::evtSetFloat(entry, args[2], marioZ);
        return 2;
    }

#define SPINDASH_SPD_MULT 1.69f
#define SPINDASH_SHAKE_THRESHOLD 1.8f

    void mot_spinjump(mario::MarioWork * mario) {
        /*
            Mot temp 4 is used for speed preservation from an NPC jump AND for max speed mid-jump
            Mot temps 5 and 6 are used for left/right timers respectively
        */
        bool switchedFlags = false;
        if ((mario->trigFlags & 0x1) != 0) // MARIO_TRIG_FLAG_MOTION_ID_CHANGED
        {
            mario->trigFlags &= ~0x1; // MARIO_TRIG_FLAG_MOTION_ID_CHANGED
            mario->miscFlags &= ~0x1;
            mario->flags |= 0x10000;  // JUMP MODE
            mario->flags &= ~0x20000; // END ANY FALL MODE
            mario->motTime = 2.0;     // IDK but mot_jump does this lolololol
            // spmario_snd::spsndSFXOff(mario->motTempS[2]); // Disable spindash sound
            mario->motTempS[2] = -1;
            mario->hitObjJumpFrom = mario->hitObjRide;
            mario_hit::clear_hitobj_ride();
            mario->airTimer = 0.0;
            mario->jumpPeakAirTime = 0.0;
            // Set jump para defs
            u8 chara = mario->character;
            f32 p1, p2, p3, p4;
            if (mario->prevMotionId == MOT_SPINJUMP) // Re-entering spinjump state, i.e. from npc jump
            {
                mot_jump::motJumpNpcPlaySfx();
                p1 = (mario_motion::marioJumpFallParaDefs[chara][0]) * 1.2;
                p2 = (mario_motion::marioJumpFallParaDefs[chara][1]) * 1.0;
                mario->xzSpeed = mario->motTempF[4];
                if (mario->stickLateralMagnitude <= 0.0) // Not holding any direction
                {
                    mario->xzSpeedFactor = 0.0; // Test this at 720 and 0, else 1440
                } else                          // Holding a direction
                {
                    if ((mario->buttonsHeld & WPAD_BTN_UP) == WPAD_BTN_UP) // H_LEFT
                    {
                        mario->directionView = 270.0;
                        mario->directionWorld = 270.0;
                        mario->unknown_0x3c4 = 270.0;
                        mario->unknown_0x3c8 = 270.0;
                    } else if ((mario->buttonsHeld & WPAD_BTN_DOWN) == WPAD_BTN_DOWN) // H_RIGHT
                    {
                        mario->directionView = 90.0;
                        mario->directionWorld = 90.0;
                        mario->unknown_0x3c4 = 90.0;
                        mario->unknown_0x3c8 = 90.0;
                    }
                    mario->xzSpeedFactor = 1440.0;
                }
            } else {
                mot_jump::motJumpPlaySfx();
                mario->motTempF[5] = mario->xzSpeed;
                p1 = (mario_motion::marioJumpFallParaDefs[chara][0]) * 1.1;
                p2 = (mario_motion::marioJumpFallParaDefs[chara][1]) * 1.2;
                mario->directionView = mario->directionWorld;
                mario->unknown_0x3c4 = mario->directionWorld;
                mario->unknown_0x3c8 = mario->directionWorld;
            }
            p3 = (mario_motion::marioJumpFallParaDefs[chara][2]);
            p4 = (mario_motion::marioJumpFallParaDefs[chara][3]);
            mario_motion::marioSetJumpParaDirect(p1, p2, p3, p4);
            // End jump para
            mario->lastJumpStartEndPos.x = mario->position.x;
            mario->lastJumpStartEndPos.y = mario->position.y;
            mario->lastJumpStartEndPos.z = mario->position.z;
            mario->lastGroundSpeed = mario->xzSpeed;
            f32 dashSpd = mot_walk::marioGetDashSpd();
            if (mario->xzSpeed < (dashSpd * SPINDASH_SPD_MULT)) {
                mario->motTempF[5] = (dashSpd * SPINDASH_SPD_MULT);
            }
            spmario_snd::spsndSFXOn_3D("SFX_P_PEACH_WATER_STEP1L", &mario->position);
            spmario_snd::spsndSFXOn_3D("SFX_E_KOTEKUTAI_JUMP1", &mario->position);
        }
        // Executes every frame
        f32 f = mario::marioGetGameSpeedScale();
        mario->motTime = mario->motTime + f;
        mario->airTimer = mario->airTimer + f;
        if (mario->jumpFallPara.nextSpeedY <= 0.0 && !switchedFlags) {
            mario->flags |= 0x20000;
            mario->flags &= ~0x10000;
            mario->motTime = 0.0;
            mario->lastFallPeakPos.x = mario->position.x;
            mario->lastFallPeakPos.y = mario->position.y;
            mario->lastFallPeakPos.z = mario->position.z;
            switchedFlags = true;
        }
        if (mario->hitObjRide != nullptr && mario->motTime >= 5.0) // End immediately if on ground
        {
            mario_sbr::marioAdjustMoveDir();
            mario->flags &= ~0x10000;
            mario_motion::marioChgMot(MOT_STAY);
        }
        f = f * 18.0 + mario->dispDirectionTarget; // Calculates the angle Mario faces during the spindash (rotation effect)
        mario->dispDirectionTarget = f;
        mario->dispDirectionCurrent = f;
        return;
    }

    static void mot_spinjump_allow_fall() {
        mario::MarioWork * mario = mario::marioGetPtr();
        if (mario->motionId != MOT_SPINJUMP) {
            mario_motion::marioChgMotSub(MOT_FALL, 1);
        }
        return;
    }

    static void mot_spinjump_enemy_bounce() {
        mario::MarioWork * mario = mario::marioGetPtr();
        if (mario->motionId != MOT_SPINJUMP) {
            mario_motion::marioChgMotSub(MOT_BOUNCE, 1);
        } else {
            mario->motTempF[4] = mario->xzSpeed;
            mario_motion::marioChgMotSub(MOT_SPINJUMP, 1);
        }
        return;
    }

    void mot_spindash_new(mario::MarioWork * mario) {
        bool endMot;
        f32 f;
        f32 dashSpd;
        f32 angle;

        if ((mario->trigFlags & 0x1) != 0) { // MARIO_TRIG_FLAG_MOTION_ID_CHANGED
            // Init spindash parameters and effects
            mario->trigFlags &= ~0x1; // MARIO_TRIG_FLAG_MOTION_ID_CHANGED
            mario->subMotionId = 10;
            mario->motTime = 0.0;
            if (mario->caught.npc == nullptr) {
                if (mario->character == mario::PLAYER_PEACH) {
                    mario::marioChgPose("KJ_1A");
                } else {
                    mario::marioChgPose("SD_1");
                }
            } else {
                mario::marioChgPose("N_8");
            }
            mario->dispFlags |= 0x104;            // MARIO_DISP_FLAG_0x4 | MARIO_DISP_FLAG_OVERRIDE_FACING
            if (mario->prevMotionId < MOT_JUMP) { // if previous mot was STAY, WALK, or DASH
                mario->flags &= ~0x10000;
                mario->miscFlags &= ~0x1;
            }
            if ((mario->miscFlags & MARIO_MISC_FLAG_SPACE_SWIM) == 0) { // NOT in space
                if ((mario->miscFlags & MARIO_MISC_FLAG_WATER) == 0) {  // NOT in water
                    if (mario->stickLateralMagnitude <= 0.0) {          // Not holding any direction
                        mario->motTempF[0] = 0.0;
                        mario->xzSpeedFactor = 0.0;
                    } else { // Holding a direction
                        dashSpd = mot_walk::marioGetDashSpd();
                        if ((mario->statusFlags & 0x2) != 0) // Slow
                            dashSpd = mot_walk::marioGetWalkSpd() * 1.2f;
                        mario->xzSpeedFactor = 1440.0; // 1440 is the max; lower this to limit max speed
                        mario->motTempF[0] = (dashSpd * SPINDASH_SPD_MULT);
                    }
                } else if (mario->hitObjRide == nullptr) { // No floor, in water
                    mario->motTempF[0] = -1.0;
                    mario->miscFlags |= 0x100020;
                } else if (mario->stickLateralMagnitude <= 0.0) { // Not holding any direction, in water
                    mario->motTempF[0] = 0.0;
                    mario->xzSpeedFactor = 0.0;
                } else { // Holding a direction, in water
                    dashSpd = mot_walk::marioGetDashSpd();
                    if ((mario->statusFlags & 0x2) != 0) // Slow
                        dashSpd = mot_walk::marioGetWalkSpd() * 1.2f;
                    mario->motTempF[0] = (dashSpd * SPINDASH_SPD_MULT);
                    mario->xzSpeedFactor = 1440.0;
                }
            } else { // In space
                mario->miscFlags |= 0x20;
                mario->motTempF[0] = -1.0;
            }
            mario->motTempF[1] = (s32)mario->dispDirectionTarget;
            mario->motTempS[2] = spmario_snd::spsndSFXOn_3D("SFX_F_SPIN_DASH1", &mario->position);
            mario->motTempS[5] = 0;
            if ((!mario::marioCheck3d() || abs_value(mario->directionWorld - mario->directionView) >= 135.0f) && mario->xzSpeed > 0.0f) {
                mario->motTempS[5] = 1;
                mario->motTempF[4] = mario->directionWorld;
                mario->directionView = mario->directionWorld;
            }
            mario::marioEffectFlagOn(1);
            effdrv::EffEntry * eff = eff_spm_spindash::effSpmSpindashEntry((mario->position).x, (mario->position).y, (mario->position).z, 1, 0);
            eff_spm_spindash::effSpmSpindashSetTargetMario(eff);
            effdrv::effSetName(eff, "marioSpin");
        }
        spmario_snd::spsndSetSfxPlayerPos(mario->motTempS[2], &mario->position);
        if (((mario->buttonsPressed & WPAD_BTN_1) == WPAD_BTN_1) && fairy::fairyItemIdToPtr(ITEM_ID_FAIRY_HAMMER) != nullptr) {
            mario_motion::marioChgMot(MOT_HAMMER);
        }
        switch (mario->subMotionId) {
        case 10: // Immediately after init
            mario->motTime += mario::marioGetGameSpeedScale();
            if (mario->motTime < 60.0) {                                                 // First 60 frames of spindash; speed phase
                f = mario::marioGetGameSpeedScale() * 18.0 + mario->dispDirectionTarget; // Calculates the angle Mario faces during the spindash (rotation effect)
                if (f > 360.0f)
                    f -= 360.0f;
                mario->dispDirectionTarget = f;
                mario->dispDirectionCurrent = f;
                if ((mario->miscFlags & MARIO_MISC_FLAG_SPACE_SWIM) == 0) {                    // NOT in space
                    if (mario->motTempF[0] < 0.0) {                                            // In water, no solid ground
                        mario_motion::func_80148c28(mario, 0, 0);                              // Controls some sort of moving downward action, I think
                    } else if (mario->motTime < 30.0) {                                        // First 30 frames of spindash
                        mario->xzSpeed = mario->motTempF[0] * mario::marioGetGameSpeedScale(); // xzSpeed = (dashSpd * SPINDASH_SPD_MULT) * 1
                    } else {                                                                   // Next 30 frames; deceleration phase
                        s32 progress = (s32)(mario->motTime - 30.0);
                        mario->xzSpeed = system::intplGetValue(4, mario->motTempF[0], 0.0, progress, 30) * mario::marioGetGameSpeedScale();
                        mario->xzSpeedFactor = system::intplGetValue(4, mario->xzSpeedFactor, 480.0f, progress, 30) * mario::marioGetGameSpeedScale();
                    }
                }
            } else { // After 60 frames of spindashing
                mario->subMotionId = 20;
            }
            if ((mario->miscFlags & MARIO_MISC_FLAG_SPACE_SWIM) == 0) { // Again, NOT in space
                if (mario->hitObjRide == nullptr) {                    // No longer on a floor; end spindash immediately
                    spmario_snd::spsndSFXOff(mario->motTempS[2]);
                    mario->motTempS[2] = -1;
                    mario_sbr::marioAdjustMoveDir();
                    mario_motion::marioChgMot(MOT_FALL);
                } else {                                                                                        // Still on a floor
                    if (DebugMode && (mario->miscFlags & MARIO_MISC_FLAG_WATER) == 0 && mario->motTime < 6.1) { // Actuate spinjump if spindash is cancelled very early (~6 frames)
                        if ((mario->buttonsPressed & WPAD_BTN_2) == WPAD_BTN_2) {
                            mario_motion::marioChgMot(MOT_SPINJUMP);
                        } else {
                            mario_motion::marioEndMotIfJump(); // Spindash can be cancelled by jumping
                        }
                    } else {
                        mario_motion::marioEndMotIfJump(); // Spindash can be cancelled by jumping
                    }
                }
            }
            break;
        case 20: // After deceleration is finished; turn off afterimage effect
            mario::marioEffectFlagOff(1);
            mario->motTime = 0.0;
            mario->subMotionId = mario->subMotionId + 1;
            break;
        case 21: // Spin in place
            f = mario::marioGetGameSpeedScale();
            mario->motTime = f * 18.0 + mario->motTime;
            f = f * 18.0 + mario->dispDirectionTarget;
            mario->dispDirectionTarget = f;
            mario->dispDirectionCurrent = f;
            if ((mario->miscFlags & MARIO_MISC_FLAG_SPACE_SWIM) == 0) { // NOT in space
                if (mario->motTempF[0] < 0.0) {                         // No floor, in water
                    mario_motion::func_80148c28(mario, 0, 0);
                } else {
                    mario->xzSpeed = 0.0;
                    mario->xzSpeedFactor = 0.0;
                }
            }
            if (180.0 <= mario->motTime) { // After 10 frames have passed in this state
                mario->subMotionId = mario->subMotionId + 1;
            }
            break;
        case 22: // Determine which direction to face coming out of the spindash, then set that direction
            angle = system::reviseAngle(mario->dispDirectionTarget);
            f = mario::marioGetGameSpeedScale();
            endMot = false;
            mario->dispDirectionTarget = angle;
            if ((mario->motTempF[1] <= 90.0) || (270.0 <= mario->motTempF[1])) {
                if ((-(f * 18.0 - 360.0) <= mario->dispDirectionTarget) && (mario->dispDirectionTarget < 360.0)) {
                    endMot = true;
                    mario->dispDirectionTarget = 0.0;
                }
            } else {
                if ((-(f * 18.0 - 180.0) <= mario->dispDirectionTarget) && (mario->dispDirectionTarget < 180.0)) {
                    mario->dispDirectionTarget = 180.0;
                    endMot = true;
                }
            }
            if (endMot) {
                mario->subMotionId = 30;
                mario->dispDirectionCurrent = mario->dispDirectionTarget;
            } else {
                f = f * 18.0 + mario->dispDirectionTarget;
                mario->dispDirectionTarget = f;
                mario->dispDirectionCurrent = f;
            }
            break;
        case 30: // Finalize and transition back to MOT_STAY
            spmario_snd::spsndSFXOff(mario->motTempS[2]);
            mario->motTempS[2] = -1;
            mario_motion::marioChgMot(MOT_STAY);
            break;
        }
        return;
    }

    void mot_spindash_post_new(mario::MarioWork * mario) {
        mario->dispDirectionTarget = mario->motTempF[1];
        mario->miscFlags &= ~0x20100020; // FORCE_SLOWDOWN, REDUCE_AIR_SPEED_GAIN (vestigial?), FORCE_FULL_JUMP (spindash cancelling immediately after jumping out of water won't force a full jump)
        mario->dispDirectionCurrent = mario->motTempF[1];
        if (mario->motTempS[2] != -1) {
            spmario_snd::spsndSFXOff(mario->motTempS[2]);
            mario->motTempS[2] = -1;
        }
        if (mario->motionId != MOT_SPINJUMP) { // NOT transitioning to a spinjump
            if (mario->motTempS[5] == 1) {
                if (!mario::marioCheck3d()) {
                    mario->directionView = mario->directionWorld;
                    mario->unknown_0x3c4 = mario->motTempF[4];
                    mario->unknown_0x3c8 = mario->motTempF[4];
                } else {
                    mario->directionView = mario->directionWorld;
                    mario->unknown_0x3c4 = mario->directionWorld;
                    mario->unknown_0x3c8 = mario->directionWorld;
                }
            }
            mario->dispFlags &= ~0x104; // MARIO_DISP_FLAG_0x4 | MARIO_DISP_FLAG_OVERRIDE_FACING
            mario::marioEffectFlagOff(1);
            effdrv::EffEntry * eff = effdrv::effNameToPtr("marioSpin");
            if (eff != nullptr) {
                effdrv::effSoftDelete(eff);
            }
        } else {
            mario->flags &= ~0x10000;
        }
        return;
    }

    static void detectSpindash() {
        mario::MarioWork * mario = mario::marioGetPtr();
        s32 motion = mario->motionId;
        if ((motion == MOT_WALK || motion == MOT_DASH || motion == MOT_STAY || motion == MOT_SPACE_SWIM) && (mario->hitObjHead == nullptr) && (msl::string::strcmp(mario->curPoseName, "D_2") != 0) && (mario->dispFlags & 0x11) == 0 && (mario->flags & 8) == 0) {
            wii::mtx::Vec3 shake = {0.0f, 0.0f, 0.0f};
            wpadmgr::WpadWork * wpad = wpadmgr::wpadGetWork();
            wii::kpad::KPADStatus * kpad = wpad->statuses[0];
            for (u8 i = 0; i < wpad->kpadReadRet[0]; i = i + 1) {
                shake.x = (shake.x + (kpad->acceleration).x) * 0.5;
                shake.y = (shake.y + (kpad->acceleration).y) * 0.5;
                shake.z = (shake.z + (kpad->acceleration).z) * 0.5;
            }
            if (((SPINDASH_SHAKE_THRESHOLD < abs_value(shake.x)) || (SPINDASH_SHAKE_THRESHOLD < abs_value(shake.y))) || (SPINDASH_SHAKE_THRESHOLD < abs_value(shake.z))) {
                mario_motion::marioChgMot(MOT_SPINDASH);
            }
        }
        return;
    }

    /*
        Test to patch the Fracktail tree back into the game
    */
    /*s32 fracktailTreeVisCb(npcdrv::NPCEntry * npc, s32 grpIdx) {
        s32 idx = animdrv::animPoseGetGroupIdx(npc->m_Anim.m_nPoseId, "TREE");
        animdrv::animdrv_wp->animPose[npc->m_Anim.m_nPoseId].visibilityGrps2[idx] = 1;
        return 0;
    }

    s32 fracktailTreeVisCb2(npc_zunbaba::ZunbabaSegmentDef * segment, s32 grpIdx, wii::mtx::Mtx34 mtx) {
        // wii::mtx::Mtx34 mtx2;
        if (grpIdx == segment->antennaLightGroupIdx) {
            // wii::mtx::PSMTXTrans(mtx2, -11.7, 20.9, 0.0);
            // wii::mtx::PSMTXConcat(mtx, mtx2, mtx);
            npc_zunbaba::zunbaba_wp->npcEntry->position = {mtx[0][3], mtx[1][3], mtx[2][3]};
        }
        s32 idx = animdrv::animPoseGetGroupIdx(segment->animPoseId, "TREE");
        animdrv::animdrv_wp->animPose[segment->animPoseId].visibilityGrps2[idx] = 1;
        npc_zunbaba::npcZunbabaSegmentDispCb(segment, grpIdx, mtx);
        return 0;
    }

    EVT_BEGIN(fracktailTreeProcCb)
    WAIT_FRM(5)
    USER_FUNC(evt_npc::evt_npc_set_disp_callback, PTR("zun"), PTR(fracktailTreeVisCb))
    SET(GW(7), 0)
    RETURN()
    EVT_END()

    EVT_BEGIN(fracktailTreeVisEvt_1)
    RUN_EVT(fracktailTreeProcCb)
    RETURN_FROM_CALL()*/

    void rewrite_main() {
        // Enemy room init evt complete rewrite
        evtpatch::hookEvtReplace(dan::dan_enemy_room_init_evt, 1, dan_enemy_room_init_evt_new);
        patch::hookFunction(npc_dimeen_l::npc_dimen_determine_move_pos, dimen_determine_move_pos_new);
        /*// Fracktail test
        evtpatch::hookEvt(0x80d44cc0, 4, fracktailTreeVisEvt_1);
        patch::hookFunction(npc_zunbaba::npcZunbabaHeadDispCb, fracktailTreeVisCb2);*/
        // Spindash
        patch::hookFunction(mot_fairy_mario::mot_spindash, mod::mot_spindash_new);
        patch::hookFunction(mot_fairy_mario::mot_spindash_post, mod::mot_spindash_post_new);
        mario_motion::marioMotTbl[MOT_SPINJUMP].mainFunc = mot_spinjump;
        mario_motion::marioMotTbl[MOT_SPINJUMP].deleteFunc = mot_spindash_post_new;
        writeBranchLink(mario_motion::marioJump, 0x610, mot_spinjump_allow_fall);
        writeBranchLink(mario_hit::marioJumpNpc, 0xA8, mot_spinjump_enemy_bounce);
        globalop::globalopAddEntry((void *)detectSpindash, nullptr);
    }
}
