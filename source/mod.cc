#include "mod.h"
#include "exception.h"
#include "patch.h"
#include "romfontexpand.h"
#include <acpatch.h>
// #include <berobero.h>
#include <common.h>
#include <customwin.h>
#include <effpatch.h>
#include <evt_cmd.h>
#include <evtpatch.h>
#include <globalop.h>
#include <lp_common.h>
#include <mempatch.h>
#include <msgpatch.h>
#include <sndpatch.h>
#include <tplpatch.h>
#include <util.h>
#include <ymetools.h>

#include <gen.h>
#include <interface.h>
#include <lunadrv.h>
#include <lunatic/localize.h>
#include <rewrite.h>
#include <rfcdrv.h>

#include <cutscene_helpers.h>
#include <msl/math.h>
#include <msl/stdio.h>
#include <msl/string.h>
#include <spm/animdrv.h>
#include <spm/bgdrv.h>
#include <spm/camdrv.h>
#include <spm/dispdrv.h>
#include <spm/eff/eff_damage_star.h>
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
#include <spm/evt_env.h>
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
#include <spm/levelupwindow.h>
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
#include <spm/npcmisc.h>
#include <spm/parse.h>
#include <spm/pausewin.h>
#include <spm/rel/aa1_01.h>
#include <spm/rel/dan.h>
#include <spm/rel/machi.h>
#include <spm/rel/mi4.h>
#include <spm/rel/relocatable_module.h>
#include <spm/seq_gameover.h>
#include <spm/seq_load_sub.h>
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
    /*
        Main file for Lunatic Pit code
        This is by far the messiest file, as it has a lot of the oldest code & a lot of artifacts from earlier development
        Not everything I'd like to move out of this file has been moved out, reorganized, or refactored
        I promise I tried at one point!
    */

    using namespace spm;
    using namespace customwin;

    LunaticPitWork * Lunatic = nullptr;

    bool DebugMode = false;

    wii::gx::GXColor MusicHeaderCol = {185, 135, 240, 255};

    wii::gx::GXColor RFCHeaderCol = {210, 100, 95, 255};

    wii::gx::GXColor MitchHeaderCol = {118, 114, 232, 255};

    wii::gx::GXColor GabbiHeaderCol = {209, 144, 203, 255};

    wii::gx::GXColor RFCArtiHeaderCol = {255, 99, 236, 255};

    s32 whackaItems[] = {106, -1};

    s32 marioMaxHp = 0;
    s32 houraiStoredAtk = 0;
    bool hpMaxed = false;
    bool houraiActivation = 0;
    s32 blessStorage = 0;
    s32 curseStorage = 0;
    s32 motId = 0;

    s16 frameR = 255;
    s16 frameG = 0;
    s16 frameB = 0;
    s16 frameColPhase = 0;

    npcdrv::NPCTribeAnimDef luigiAnims[] = {{0, "luigi_S_1"}, {1, "luigi_W_1"}, {2, "luigi_R_2"}, {3, "luigi_T_1"}, {4, "luigi_D_7"}, {6, "luigi_D_7"}, {7, "luigi_D_7"}, {8, "luigi_D_7"}, {9, "luigi_K_1"}, {10, "luigi_Y_1"}, {11, "luigi_Y_1"}, {12, "luigi_K_1"}, {13, "luigi_K_1"}, {14, "luigi_E_2"}, {15, "luigi_Y_1"}, {25, "luigi_J_1B"}, {26, "luigi_T_3A"}, {27, "luigi_J_1C"}, {28, "luigi_I_1"}, {29, "luigi_S_3"}, {30, "luigi_D_4"}, {-1, "luigi_Z_1"}};

    npcdrv::NPCPartDef sbParts[2] = {npcdrv::npcTribes[38].partsList[0], npcdrv::npcTribes[38].partsList[1]};

    npcdrv::NPCPartDef gcParts[7] = {npcdrv::npcTribes[124].partsList[0], npcdrv::npcTribes[124].partsList[1], npcdrv::npcTribes[124].partsList[2],
                                     npcdrv::npcTribes[124].partsList[3], npcdrv::npcTribes[124].partsList[4], npcdrv::npcTribes[124].partsList[5],
                                     npcdrv::npcTribes[124].partsList[6]};

    npcdrv::NPCPartDef bbParts[10] = {npcdrv::npcTribes[171].partsList[0], npcdrv::npcTribes[171].partsList[1], npcdrv::npcTribes[171].partsList[2],
                                      npcdrv::npcTribes[171].partsList[3], npcdrv::npcTribes[171].partsList[4], npcdrv::npcTribes[171].partsList[5],
                                      npcdrv::npcTribes[171].partsList[6], npcdrv::npcTribes[171].partsList[7], npcdrv::npcTribes[171].partsList[8],
                                      npcdrv::npcTribes[171].partsList[9]};

    // Shy Guy anim defs
    npcdrv::NPCTribeAnimDef heihoAnims[] = {                   // Copied from Goombas, then R_2 thrown in lol
                                            {0, "heiho_S_1"},  // Idle
                                            {1, "heiho_W_1"},  // Walking
                                            {2, "heiho_R_2"},  // Running (Replace with R_1 for normal running)
                                            {3, "heiho_T_1"},  // Talking
                                            {4, "heiho_D_1"},  // Damaged
                                            {6, "heiho_D_1"},  // Damaged
                                            {7, "heiho_D_1"},  // Damaged
                                            {8, "heiho_D_1"},  // Damaged
                                            {9, "heiho_K_1"},  // Dizzy
                                            {10, "heiho_Z_1"}, // Idle
                                            {11, "heiho_Z_1"}, // Idle
                                            {12, "heiho_K_1"}, // Dizzy
                                            {13, "heiho_K_1"}, // Dizzy
                                            {14, "heiho_N_1"}, // Eepy
                                            {15, "heiho_Z_1"}, // Idle
                                            {26, "heiho_R_2"}, // Ludicrous speed!!! (Use only for non-enemy stuff)
                                            {-1, "heiho_Z_1"}};

    EVT_BEGIN(heiho_g_panic_real)
    DO(0)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(4), LW(5), LW(6))
    USER_FUNC(evt_npc::evt_npc_get_axis_movement_unit, PTR("me"), LW(7))
    USER_FUNC(evt_npc::evt_npc_get_unitwork, PTR("me"), 13, LW(13))
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(14), 0, LW(15))
    USER_FUNC(evt_mario::evt_mario_check_3d, LW(10))
    IF_EQUAL(LW(10), 0) // Check if in 3D
                        // Check if Shy Guy is facing Mario and is within 200 units on X (Z only if in 3D)
    IF_EQUAL(LW(13), 0) // Not already in Panicked state
    SUB(LW(14), LW(4))
    MUL(LW(14), LW(7))
    IF_SMALL(LW(14), 0) // If NPC is facing Mario & under 200 units away. Trust me, this works
    IF_LARGE(LW(14), -100)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(1), LW(2), LW(3))
    SUB(LW(5), LW(2))
    IF_EQUAL(LW(5), 0) // Same Y val. Execute code now
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 8, 1)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(14), LW(2), LW(15))
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(14), LW(15), 0, 0, 532, 0, 0)
    MUL(LW(1), -1)
    MUL(LW(7), -1)
    WAIT_MSEC(450)
    //        USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), LW(7))
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 13, 1)
    WAIT_MSEC(350)
    MUL(LW(7), 8192)
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(7), LW(3), 0, 127, 532, 0, 0)
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 8, 0)
    DO_BREAK()
    END_IF()
    END_IF()
    END_IF()
    END_IF()
    END_IF()
    WAIT_FRM(5)
    WHILE()
    RETURN()
    EVT_END()

    EVT_BEGIN(heiho_g_panic)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(12))
    USER_FUNC(evt_npc::evt_npc_get_unitwork, PTR("me"), 13, LW(13))
    IF_EQUAL(LW(13), 0)
    IF_EQUAL(LW(12), 530)
    RUN_EVT(heiho_g_panic_real)
    END_IF()
    END_IF()
    IF_EQUAL(LW(11), 0)
    IF_EQUAL(LW(12), 530) // Is Green Shy Guy
    USER_FUNC(evt_npc::evt_npc_get_unitwork, PTR("me"), 13, LW(13))
    IF_EQUAL(LW(13), 1) // In Panicked state
    USER_FUNC(evt_npc::evt_npc_get_unitwork, PTR("me"), 8, LW(8))
    IF_EQUAL(LW(8), 0)
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(1), LW(3), 0, 255, 532, 0, 0)
    END_IF()
    ELSE() // Not in Panicked state
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(1), LW(3), 0, 40, 532, 0, 0)
    END_IF()
    ELSE() // Not Green Shy Guy
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(1), LW(3), 0, 30, 532, 0, 0)
    END_IF()
    ELSE() // Not Green Shy Guy OR Goomba
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(1), LW(3), 0, 30, 540, 0, 0)
    END_IF()
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 8, 0)
    RETURN_FROM_CALL()

    // Define enemy defenses
    npcdrv::NPCDefense createDef(int type, s32 defense, s32 flags) {
        npcdrv::NPCDefense def;
        def.type = type;
        def.defense = defense;
        def.flags = flags;
        return def;
    }
    npcdrv::NPCDefense def0_1 = createDef(0x0, 0x1, 0x0);
    npcdrv::NPCDefense def1_1 = createDef(0x1, 0x1, 0x0);
    npcdrv::NPCDefense def2_1 = createDef(0x2, 0x1, 0x0);
    npcdrv::NPCDefense def3_1 = createDef(0x3, 0x1, 0x0);
    npcdrv::NPCDefense def4_1 = createDef(0x4, 0x1, 0x0);
    npcdrv::NPCDefense def7_1 = createDef(0x7, 0x1, 0x0);
    npcdrv::NPCDefense def10_1 = createDef(0xa, 0x1, 0x0);
    npcdrv::NPCDefense def11_1 = createDef(0xb, 0x1, 0x0);
    npcdrv::NPCDefense def12_1 = createDef(0xc, 0x1, 0x0);
    npcdrv::NPCDefense def13_1 = createDef(0xd, 0x1, 0x0);
    npcdrv::NPCDefense def15_1 = createDef(0xf, 0x1, 0x0);
    npcdrv::NPCDefense def17_1 = createDef(0x11, 0x1, 0x0);
    npcdrv::NPCDefense def29_1 = createDef(0x1d, 0x1, 0x0);
    npcdrv::NPCDefense def31_1 = createDef(0x1f, 0x1, 0x0);

    // Initiators / Terminators (41 for sure, 48 unsure?)
    // Use def0 for enemies with no default defense
    npcdrv::NPCDefense def0 = createDef(0x0, 0x0, 0x0);
    npcdrv::NPCDefense def48 = createDef(0x30, 0x32, 0x0);
    npcdrv::NPCDefense defterm = createDef(41, 0x64, 0x0);
    static npcdrv::NPCDefense defaultDefenses[] = {def0, def48, defterm};

    bool critActuate = false;
    bool guaranteeCritActuation = false;

    // Add/reduce damage to certain enemies; later, maybe set DEFs for enemies that could actually use it and don't
    // override all defenses with damage reduction. There are certainly a few in this array that can use DEFs, direct DR
    // is just easier for me right now.
    s32 (*marioCalcDamageToEnemy)(s32 damageType, s32 tribeId);
    void (*marioTakeDamage)(wii::mtx::Vec3 * position, u32 flags, s32 damage);
    s32 (*npcDamageMario)(npcdrv::NPCEntry * npcEntry, npcdrv::NPCPart * part, wii::mtx::Vec3 * position, u32 status, s32 damage, u32 flags);
    s32 (*npcHandleHitXp)(mario::MarioWork * marioWork, npcdrv::NPCEntry * npcEntry, s32 killXp, s32 unk_variant);
    s32 (*npcTakeDamage)(npcdrv::NPCEntry * npc, npcdrv::NPCPart * npcPart, s32 damageType, s32 power, u32 flags, s32 param_6);
    effdrv::EffEntry * (*effDamageStarEntry)(f32 x, f32 y, f32 z, s32 variant, s32 damage);
    void (*camShakeInit)(f32 x, f32 y, f32 z, camdrv::CameraId camId, u32 duration);
    void (*marioChgMotSub)(s32 motionId, s32 p2);
    void patchMarioDamage() {
        marioCalcDamageToEnemy = patch::hookFunction(mario::marioCalcDamageToEnemy, [](s32 damageType, s32 tribeId) {
            // handle enemy damage reduction
            s32 dmg = marioCalcDamageToEnemy(damageType, tribeId);
            s32 paramitaTimer = swdrv::swByteGet(1610);
            s32 floor = swdrv::swByteGet(1);
            if (paramitaTimer > 20) {
                if (floor < 150) {
                    dmg = (dmg * 2);
                } else if (floor < 175) {
                    dmg = (dmg * 3);
                } else {
                    dmg = (dmg * 4);
                }
            }
            s32 disorderId = Lunatic->Luna.disorder;
            if (disorderId == DISORDER_RED)
                dmg -= Lunatic->Luna.DW.UW.Apathy.marioDamageDecrease;
            // Kilo Muth, DEF 3 -> 1
            // Ice Bro, DEF 3 --> 1
            // Red Spike Top, DEF 6 --> 4
            // Sky-Blue Spiny, DEF 6 --> 4
            if (tribeId == 506 || tribeId == 61 || tribeId == 29 || tribeId == 38) {
                dmg += 2;
            }
            // Spunia, DEF 0 --> 1
            // Green Shy Guy, DEF 0 --> 1
            else if (tribeId == 496 || tribeId == 530) {
                dmg -= 1;
            }
            // Shady Bros, DEF 3 --> 2
            // Red Chomp & Skellobytes, DEF 4? --> 1 less
            if (tribeId == 61 || tribeId == 55 || tribeId == 49 || tribeId == 225 || tribeId == 228 || tribeId == 123) {
                dmg += 1;
            }
            // Green (Broom) Magikoopa, DEF 0 --> 3
            // (Flip) Shady Koopa, DEF 0 --> 3
            else if (tribeId == 473 || tribeId == 474 || tribeId == 465 || tribeId == 466) {
                dmg -= 3;
            }
            // Blue Shy Guy, DEF 0 --> 3
            else if (tribeId == 531) {
                dmg -= 3;
            }
            // Dark Koopatrol, DEF 8 --> 5
            else if (tribeId == 19) {
                dmg += 3;
            }
            // Gold Chomp, DEF 10 --> 6
            else if (tribeId == 124) {
                dmg += 4;
            }
            if (disorderId == DISORDER_YELLOW) {
                if (mario_pouch::pouchGetPtr()->coins < Lunatic->Luna.DW.UW.Prejudice.coinThreshold)
                    dmg = (s32)(dmg / 2);
            }
            if (dmg < 0) {
                dmg = 0;
            }
            return dmg;
        });

        marioTakeDamage = patch::hookFunction(mario::marioTakeDamage, [](wii::mtx::Vec3 * position, u32 flags, s32 damage) {
            s32 odds;
            // Vulnerability Hex
            s32 curse = swdrv::swByteGet(1601);
            if (curse == CurseId::MERLUNA_HEX) {
                s32 floor = swdrv::swByteGet(1);
                if (floor < 49) {
                    damage = damage + 1;
                } else if (floor < 174) {
                    damage = damage + 2;
                } else {
                    damage = damage + 3;
                }
                if (damage < 0) {
                    damage = 0;
                }
            }
            // Migraine
            if (curse == CurseId::MERLUNA_MIGRAINE) {
                odds = system::rand() % 100;
                if (odds < 40) // 40%
                {
                    mario_status::marioStatusApplyStatuses(STATUS_FLIPPED_CONTROLS, 2);
                    swdrv::swSet(1670);
                } else if (odds < 60) // 20%
                {
                    mario_status::marioStatusApplyStatuses(STATUS_NO_SKILLS, 2);
                    swdrv::swSet(1670);
                } else if (odds < 70) // 10%
                {
                    mario_status::marioStatusApplyStatuses(STATUS_SLOW, 2);
                    swdrv::swSet(1670);
                } else if (odds < 75) // 5%, otherwise no status.
                {
                    mario_status::marioStatusApplyStatuses(STATUS_NO_JUMP, 2);
                    swdrv::swSet(1670);
                }
            }
            // Melancholy
            if (Lunatic->Luna.disorder == DISORDER_WHITE) {
                s32 power = (Lunatic->Luna.DW.UW.Melancholy.stacks - 1);
                if (power >= 0) {
                    damage += (s32)pow(2, power);
                    effdrv::EffEntry * eff = temp_unk::effItemThunderEntry(0.0f, 0.0f, 0.0f, 1.0f, 0, 1, 0, 0);
                    temp_unk::effItemThunderSetTargetMario(eff);
                    wii::gx::GXColor black = {0, 0, 0, 255};
                    wii::gx::GXColor white = {255, 255, 255, 255};
                    effpatch::effpatchColorMaskEntry(eff, black, white, nullptr);
                    wii::mtx::Vec3 * pos = &mario::marioGetPtr()->position;
                    spmario_snd::spsndSFXOn_3D("SFX_I_THUNDER1", pos);
                    spmario_snd::spsndSFXOn_3D("SFX_I_BIRIBIRI2", pos);
                    spmario_snd::spsndSFXOn_3D("SFX_EVT_HELWANWAN_MISS1", pos);
                    eff = eff_spm_hit::effSpmHitEntry(pos->x, pos->y + (mario_hit::marioGetHeight() / 2.0f), pos->z, 2);
                    effpatch::effpatchColorMaskEntry(eff, black, black, nullptr);
                    Lunatic->Luna.DW.UW.Melancholy.stacks = 0;
                }
                Lunatic->Luna.DW.UW.Melancholy.subtimer = 10;
            }
            marioTakeDamage(position, flags, damage);
        });

        npcDamageMario = patch::hookFunction(npcdrv::npcDamageMario,
                                             [](npcdrv::NPCEntry * npcEntry, npcdrv::NPCPart * part, wii::mtx::Vec3 * position, u32 status, s32 damage, u32 flags) {
                                                 f32 fDmg;
                                                 if (npcEntry == nullptr) // Patches a crash that occurs when npcDamageMario is called with no npcEntry pointer
                                                 {
                                                     if (part == nullptr && status == 0 && damage == 20 && flags == 4) // Dark Bowser's fire meets these conditions
                                                     {
                                                         if (lpGetDifficulty() == 0)
                                                             damage /= 2;
                                                     }
                                                     return npcDamageMario(npcEntry, part, position, status, damage, flags);
                                                 }
                                                 if (npcCheckDanFlag(part->owner, DAN_NPC_HOLOGRAPHIC) == true) // Holographic enemies have 2x direct ATK
                                                     damage *= 2;
                                                 if (npcCheckDanFlag(part->owner, DAN_NPC_NEGATIVE) == true) // Negative enemies have 3x direct ATK
                                                 {
                                                     if (damage > 5)
                                                         damage = 15;
                                                     else
                                                         damage *= 3;
                                                 }
                                                 // Apathy
                                                 s32 disorderId = Lunatic->Luna.disorder;
                                                 if (disorderId == DISORDER_RED)
                                                     damage += Lunatic->Luna.DW.UW.Apathy.enemyDamageIncrease;
                                                 // Indolence
                                                 if (disorderId == DISORDER_PURPLE) {
                                                     s32 odds = system::rand() % 100;
                                                     if (odds < Lunatic->Luna.DW.UW.Indolence.attackEffectChance) {
                                                         odds = system::rand() % 100;
                                                         if (odds < 33) // Freeze
                                                         {
                                                             status |= 0x2000;
                                                         } else {
                                                             mario_status::marioStatusApplyStatuses(STATUS_SLOW, 2);
                                                             swdrv::swSet(1670);
                                                         }
                                                     }
                                                 }
                                                 // Patience Voucher proc
                                                 if (VoucherGetStateById(VOUCHER_CYAN) == V_ACTIVE && system::irand(9) < 2) { // 20% proc chance
                                                     npcdrv::npcTakeDamage(nullptr, npcdrv::npcGetMainPart(npcEntry, mario::marioCheck3d()), 29, damage, 0, 0);
                                                 }
                                                 if (npcEntry->tribeId == 62) // Ice Bro projectiles will freeze you
                                                 {
                                                     status |= 0x2000;
                                                 }
                                                 if (npcEntry->tribeId <= 333 && npcEntry->tribeId >= 330) // Patch Shadoo damage
                                                 {
                                                     if (lpGetDifficulty() == 0)
                                                         damage /= 2;
                                                 }
                                                 // Auspice and Aegis
                                                 if (msl::string::strstr(spmario::gp->mapName, "dan") != nullptr) {
                                                     // Aegis flat damage reduction
                                                     damage -= Lunatic->Stats.AegisDef;
                                                     // Auspice damage% reduction
                                                     f32 dr = Lunatic->Stats.AuspiceDR;
                                                     // Increase DR by 20 when Bravery is torn & LV is 4 OR Holo/Neg
                                                     if (VoucherChkTorn(VOUCHER_ORANGE) && (npcGetDanLv(npcEntry->tribeId) == 4 || npcCheckDanFlag(npcEntry, (DAN_NPC_HOLOGRAPHIC | DAN_NPC_NEGATIVE)) == true)) {
                                                         dr += 20.0f;
                                                     }
                                                     fDmg = (f32)damage * (1.0f - (dr / 100.0f));
                                                     damage = round(fDmg);
                                                     if (damage == 0 && !Lunatic->RFC.rfcSpecialObtained[AEGIS_2]) // Only reach 0 damage if you have the Aegis Invocation
                                                         damage = 1;
                                                 }
                                                 if (damage < 0)
                                                     damage = 0;
                                                 return npcDamageMario(npcEntry, part, position, status, damage, flags);
                                             });

        npcHandleHitXp = patch::hookFunction(temp_unk::npcHandleHitXp, [](mario::MarioWork * marioWork, npcdrv::NPCEntry * npcEntry, s32 killXp, s32 unk_variant) {
            s32 disorderId = Lunatic->Luna.disorder;
            if (disorderId == DisorderId::DISORDER_ORANGE) // If Dread is active, disable XP
                killXp = 0;
            if (npcCheckDanFlag(npcEntry, DAN_NPC_CHILD) == true) // If child npc, give 10% XP
                killXp /= 10;
            if (disorderId == DisorderId::DISORDER_CYAN && killXp > 0 && npcEntry->attackedDamageType == 2) // If Recalcitrance is active & enemy was defeated with a jump, multiply XP negatively
                killXp *= ((f32)Lunatic->Luna.DW.UW.Recalcitrance.dispXPMult / 100.0f);
            if (npcCheckDanFlag(npcEntry, DAN_NPC_HOLOGRAPHIC) == true)
                killXp *= 2; //  Holographic enemies in the Pit will give 2x score
            if (npcCheckDanFlag(npcEntry, DAN_NPC_NEGATIVE) == true)
                killXp *= -1; //  Negative enemies give negative score
            killXp = round_to((s32)killXp, 10);
            return npcHandleHitXp(marioWork, npcEntry, killXp, unk_variant);
        });

        npcTakeDamage =
            patch::hookFunction(npcdrv::npcTakeDamage, [](npcdrv::NPCEntry * npc, npcdrv::NPCPart * npcPart, s32 damageType, s32 power, u32 flags, s32 param_6) {
                s32 difficulty = lpGetDifficulty();
                // OSREPORTF("npcTakeDamage: %s dealt damage of type %d to %s.\n", npcTribeToName(npcPart->owner->tribeId), damageType, npcTribeToName(npc->tribeId));
                if (damageType == 5 && difficulty >= 2) {
                    // No damage is dealt from shells unless both enemies are spinning
                    if ((npc->moveMode == NPC_MOVE_SPIN && npcPart->owner->moveMode == NPC_MOVE_SPIN) || (npc->moveMode == NPC_MOVE_SPIN && npcPart->owner->moveMode == NPC_MOVE_THROWN) || (npc->moveMode == NPC_MOVE_THROWN && npcPart->owner->moveMode == NPC_MOVE_SPIN)) {
                        spmario_snd::spsndSFXOn_3D("SFX_E_CRUSH1", &npcPart->position);
                        eff_spm_hit::effSpmHitEntry(npcPart->position.x, npcPart->position.y, npcPart->position.z, 0);
                        spmario_snd::spsndSFXOn_3D("SFX_E_CRUSH1", &npc->position);
                        eff_spm_hit::effSpmHitEntry(npc->position.x, npc->position.y, npc->position.z, 0);
                        npcTakeDamage(npcPart->owner, &npc->parts[0], damageType, power, flags, param_6);
                        return npcTakeDamage(npc, npcPart, damageType, power, flags, param_6);
                    } else if ((u32)npcPart->owner == (u32)mario::marioGetPtr()->caught.npc) {
                        spmario_snd::spsndSFXOn_3D("SFX_E_CRUSH1", &npcPart->position);
                        eff_spm_hit::effSpmHitEntry(npcPart->position.x, npcPart->position.y, npcPart->position.z, 0);
                        return npcTakeDamage(npc, npcPart, 6, power, flags, param_6);
                    } else {
                        return npcTakeDamage(0, 0, damageType, 0, 0, param_6);
                    }
                }
                if (damageType == 5 || (damageType == 29 && npc != (npcdrv::NPCEntry *)mario::marioGetPtr())) // Hit by Koopa shell or Barried projectile
                {
                    if (npcPart != 0) {
                        spmario_snd::spsndSFXOn_3D("SFX_E_CRUSH1", &npcPart->position);
                        eff_spm_hit::effSpmHitEntry(npcPart->position.x, npcPart->position.y, npcPart->position.z, 0);
                    } else if (npc != 0) {
                        spmario_snd::spsndSFXOn_3D("SFX_E_CRUSH1", &npc->position);
                        eff_spm_hit::effSpmHitEntry(npc->position.x, npc->position.y, npc->position.z, 0);
                    }
                } else if (damageType == 6) // Thoreau collision
                {
                    if (npcPart != 0) {
                        spmario_snd::spsndSFXOn_3D("SFX_E_CRUSH1", &npcPart->position);
                        eff_spm_hit::effSpmHitEntry(npcPart->position.x, npcPart->position.y, npcPart->position.z, 0);
                    }
                    if (npc != 0) {
                        spmario_snd::spsndSFXOn_3D("SFX_E_CRUSH1", &npc->position);
                        eff_spm_hit::effSpmHitEntry(npc->position.x, npc->position.y, npc->position.z, 0);
                    }
                }
                if (npcCheckDanFlag(npcPart->owner, DAN_NPC_HOLOGRAPHIC) == true)
                    power -= 3;
                if (npcCheckDanFlag(npcPart->owner, DAN_NPC_NEGATIVE) == true &&
                    !((damageType == 6 || damageType == 4) && npcCheckDanFlag(npc, DAN_NPC_NEGATIVE) == true) && power >= 3) // Negative enemies should be vulnerable to Thoreau damage ONLY when colliding with another negative enemy
                    power = round((f32)power / 3.0f);
                s32 odds;
                s32 disorderId = Lunatic->Luna.disorder;
                if (disorderId == DISORDER_ORANGE && power > 0) {
                    odds = system::irand(100);
                    if (odds < Lunatic->Luna.DW.UW.Dread.dispBlockChance) {
                        npcPart->owner->hp += (s32)(msl::math::sqrt(power));
                        if (npcPart->owner->hp > npcPart->owner->maxHp)
                            npcPart->owner->hp = npcPart->owner->maxHp;
                        // effdrv::EffEntry *recovery = eff_spm_recovery::effSpmRecoveryEntry(npcPart->owner->position.x,
                        // npcPart->owner->position.y, npcPart->owner->position.z, (power / 2));
                        // eff_spm_recovery::func_80091794(recovery, -1, npcPart->owner->name);
                        power = 0;
                    }
                } else // Crits should be mutually exclusive from forced dodges
                {
                    // Calculate critical strikes
                    f32 critOdds = system::rand() % 100;
                    if (Lunatic->Stats.CritRate > critOdds || guaranteeCritActuation) {
                        f32 fDmg = (f32)power;
                        f32 cm = Lunatic->Stats.CritMult;
                        // Bravery torn, powerful enemy
                        if (VoucherChkTorn(VOUCHER_ORANGE) && (npcGetDanLv(npcPart->owner->tribeId) == 4 || npcCheckDanFlag(npcPart->owner, (DAN_NPC_HOLOGRAPHIC | DAN_NPC_NEGATIVE)) == true)) {
                            cm += 100.0f;
                        }
                        fDmg *= (cm / 100.0f + 1.0f);
                        power = (s32)msl::math::floor(fDmg) + 1;
                        critActuate = true;
                    }
                }
                // Check if crit should actually occur
                u32 hp = npcPart->owner->hp;
                s32 ret = npcTakeDamage(npc, npcPart, damageType, power, flags, param_6);
                if (guaranteeCritActuation) {
                    guaranteeCritActuation = false;
                    critActuate = true;
                } else if (hp == npcPart->owner->hp || (npcPart->owner->flagC & 0x4000000) != 0 || power < 0 || damageType == 33 || damageType == 37) {
                    critActuate = false;
                }
                // OSREPORTF("%d damage dealt of type %d.\n", power, damageType);
                return ret;
            });

        effDamageStarEntry = patch::hookFunction(eff_damage_star::effDamageStarEntry, [](f32 x, f32 y, f32 z, s32 variant, s32 damage) {
            effdrv::EffEntry * eff = effDamageStarEntry(x, y, z, variant, damage);
            if (critActuate) {
                wii::gx::GXColor CritPurple = {150, 70, 200, 255};
                wii::gx::GXColor CritPurple2 = {70, 35, 150, 255};
                effpatch::effpatchColorMaskEntry(eff, CritPurple, CritPurple2, nullptr);
                spmario_snd::spsndSFXOn_3D("SFX_P_ACROBAT_RENZOKU3", &mario::marioGetPtr()->position);
                critActuate = false;
            }
            return eff;
        });

        camShakeInit = patch::hookFunction(camdrv::camShakeInit, [](f32 x, f32 y, f32 z, camdrv::CameraId camId, u32 duration) {
            // OSREPORTF("camShakeInit Call: tremorState == %d.\n", Lunatic->Luna.DW.tremorState);
            if (Lunatic->Luna.DW.tremorState > 0)
                return;
            camShakeInit(x, y, z, camId, duration);
        });
        marioChgMotSub = patch::hookFunction(mario_motion::marioChgMotSub, [](s32 mot, s32 p2) {
            // Patch double damage glitch and Recalcitrance not allowing mot_hit
            if (mot == MOT_BOUNCE && mario::marioGetPtr()->motionId == MOT_HIT)
                return;
            marioChgMotSub(mot, p2);
        });
    }

    void npcInheritDanFlag(npcdrv::NPCEntry * parent, npcdrv::NPCEntry * child) {
        if (child == nullptr)
            return;
        npcSetDanFlag(child, DAN_NPC_CHILD);
        if (npcCheckDanFlag(parent, DAN_NPC_HOLOGRAPHIC) == true)
            npcMakeHolo(child);
        if (npcCheckDanFlag(parent, DAN_NPC_NEGATIVE) == true)
            npcMakeNegative(child);
        return;
    }

    s32 (*func_801f9294)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_801f9cfc)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_801fa3d0)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_80202ea8)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_80200f5c)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_802052fc)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_8022e008)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_80206a84)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_802232c0)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_80221cf4)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_801d9a88)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_802355f8)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_801d8d94)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_802259f0)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_801f6514)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_8021eaac)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_8021e8ac)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_80234368)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_80234d3c)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_80234fd4)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_8021259c)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_8024b198)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*func_801f8220)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    s32 (*npc_jugemu_toss_spiny)(evtmgr::EvtEntry * evtEntry, bool firstRun);
    static void hookNpcChildrenFromParentSpawnFuncs() {
        // Hammer Bros
        func_801f9294 = patch::hookFunction(temp_unk::func_801f9294, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_801f9294(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Boomerang Bros
        func_801f9cfc = patch::hookFunction(temp_unk::func_801f9cfc, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_801f9cfc(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Fire Bros
        func_801fa3d0 = patch::hookFunction(temp_unk::func_801fa3d0, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_801fa3d0(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Dull Bones
        func_80202ea8 = patch::hookFunction(temp_unk::func_80202ea8, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_80202ea8(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Magikoopas
        func_80200f5c = patch::hookFunction(temp_unk::func_80200f5c, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_80200f5c(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Koopa Strikers
        func_802052fc = patch::hookFunction(temp_unk::func_802052fc, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_802052fc(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Pokeys
        func_8022e008 = patch::hookFunction(temp_unk::func_8022e008, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_8022e008(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Ruff Puffs
        func_80206a84 = patch::hookFunction(temp_unk::func_80206a84, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_80206a84(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Dayzees
        func_802232c0 = patch::hookFunction(temp_unk::func_802232c0, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_802232c0(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Squigs
        func_80221cf4 = patch::hookFunction(temp_unk::func_80221cf4, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_80221cf4(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Boomboxers
        func_801d9a88 = patch::hookFunction(temp_unk::func_801d9a88, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_801d9a88(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Sproing-Oings
        func_802355f8 = patch::hookFunction(temp_unk::func_802355f8, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_802355f8(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Mr. Is
        func_801d8d94 = patch::hookFunction(temp_unk::func_801d8d94, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_801d8d94(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Foton
        func_802259f0 = patch::hookFunction(temp_unk::func_802259f0, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_802259f0(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Eel/Hooligons
        func_801f6514 = patch::hookFunction(temp_unk::func_801f6514, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_801f6514(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Ninjoes (Bombs)
        func_8021eaac = patch::hookFunction(temp_unk::func_8021eaac, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_8021eaac(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Ninjoes (Shurikens)
        func_8021e8ac = patch::hookFunction(temp_unk::func_8021e8ac, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_8021e8ac(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Skellobomber (Head-Type)
        func_80234368 = patch::hookFunction(temp_unk::func_80234368, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_80234368(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Skellobomber (Bait-Type)
        func_80234d3c = patch::hookFunction(temp_unk::func_80234d3c, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_80234d3c(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Skellobait (Breath)
        func_80234fd4 = patch::hookFunction(temp_unk::func_80234fd4, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_80234fd4(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Magiblots
        func_8021259c = patch::hookFunction(temp_unk::func_8021259c, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_8021259c(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Cherbils
        func_8024b198 = patch::hookFunction(temp_unk::func_8024b198, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_8024b198(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Growmebas
        func_801f8220 = patch::hookFunction(temp_unk::func_801f8220, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = func_801f8220(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
        // Lakitus
        npc_jugemu_toss_spiny = patch::hookFunction(temp_unk::npc_jugemu_toss_spiny, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            s32 ret = npc_jugemu_toss_spiny(evtEntry, firstRun);
            if (ret == 2)
                npcInheritDanFlag((npcdrv::NPCEntry *)evtEntry->ownerNPC, (npcdrv::NPCEntry *)evtEntry->tempU[0]);
            return ret;
        });
    }

    // Called to remove default segments and their shadows from the map (Top 7 are the hitobjs, followed by mapobjs for
    // visible segment and dropshadow)
    s32 danClearMapParts() {
        // Hitobjs for default segs
        hitdrv::hitGrpFlagOn(0, "pCube11_q00009", 1);
        hitdrv::hitGrpFlagOn(0, "pCube12_q00010", 1);
        hitdrv::hitGrpFlagOn(0, "pCube15_q00011", 1);
        hitdrv::hitGrpFlagOn(0, "pCube16_q00012", 1);
        hitdrv::hitGrpFlagOn(0, "pCube17_q00013", 1);
        hitdrv::hitGrpFlagOn(0, "pCube18_q00014", 1);
        hitdrv::hitGrpFlagOn(0, "pCube19_q00015", 1);

        // dan_01 mapobjs for default segs and shadows
        mapdrv::mapGrpFlagOn(0, "pCube11_q00028", 1);
        mapdrv::mapGrpFlagOn(0, "pCube11_q00029", 1);
        mapdrv::mapGrpFlagOn(0, "pCube12_q00030", 1);
        mapdrv::mapGrpFlagOn(0, "pCube12_q00031", 1);
        mapdrv::mapGrpFlagOn(0, "pCube15_q00032", 1);
        mapdrv::mapGrpFlagOn(0, "pCube15_q00033", 1);
        mapdrv::mapGrpFlagOn(0, "pCube16_q00034", 1);
        mapdrv::mapGrpFlagOn(0, "pCube16_q00035", 1);
        mapdrv::mapGrpFlagOn(0, "pCube17_q00036", 1);
        mapdrv::mapGrpFlagOn(0, "pCube17_q00037", 1);
        mapdrv::mapGrpFlagOn(0, "pCube18_q00038", 1);
        mapdrv::mapGrpFlagOn(0, "pCube18_q00039", 1);
        mapdrv::mapGrpFlagOn(0, "pCube19_q00040", 1);
        mapdrv::mapGrpFlagOn(0, "pCube19_q00041", 1);

        // dan_02 mapobjs for default segs and shadows (1 shared with dan_01)
        mapdrv::mapGrpFlagOn(0, "pCube11_q00036", 1);
        mapdrv::mapGrpFlagOn(0, "pCube11_q00037", 1);
        mapdrv::mapGrpFlagOn(0, "pCube12_q00036", 1);
        mapdrv::mapGrpFlagOn(0, "pCube12_q00037", 1);
        mapdrv::mapGrpFlagOn(0, "pCube15_q00036", 1);
        mapdrv::mapGrpFlagOn(0, "pCube15_q00037", 1);
        mapdrv::mapGrpFlagOn(0, "pCube16_q00038", 1);
        mapdrv::mapGrpFlagOn(0, "pCube16_q00039", 1);
        mapdrv::mapGrpFlagOn(0, "pCube17_q00039", 1);
        mapdrv::mapGrpFlagOn(0, "pCube17_q00040", 1);
        mapdrv::mapGrpFlagOn(0, "pCube18_q00040", 1);
        mapdrv::mapGrpFlagOn(0, "pCube18_q00041", 1);

        // dan_43 mapobjs (tails are offset by 1 from dan_01)
        mapdrv::mapGrpFlagOn(0, "pCube11_q00027", 1);
        mapdrv::mapGrpFlagOn(0, "pCube12_q00029", 1);
        mapdrv::mapGrpFlagOn(0, "pCube15_q00031", 1);
        mapdrv::mapGrpFlagOn(0, "pCube16_q00033", 1);
        mapdrv::mapGrpFlagOn(0, "pCube17_q00035", 1);
        mapdrv::mapGrpFlagOn(0, "pCube18_q00037", 1);
        mapdrv::mapGrpFlagOn(0, "pCube19_q00039", 1);
        return 2;
    }

    const char * danTexMapNames[] = {"dan_44_zaku.tpl", "dan_44_blank.tpl", "dan_64_zaku_insp.tpl"};

    // Gigantic shoutouts to L and Seeky for helping me get this function to work!!!!!!
    static void loadNewDanTex() {
        const char * loadMap = seq_mapchange::seq_mapchange_wp->mapName;
        mapdrv::MapWorkGroup * mapWork = mapdrv::mapGetWork(); // Gets MapWorkGroup, which should contain the MapEntry for the current map
        mapdrv::MapEntry * mapEntry = &mapWork->entries[0];    // The MapEntry for current map
        s32 seq = seqdrv::seqGetSeq();
        if (msl::string::strstr(loadMap, "dan") != nullptr && msl::string::strstr(loadMap, "dan_70") == nullptr && seq != 4 && seq != 1) {
            s32 currentFloor = swdrv::swByteGet(1);
            bool overwriteMap = false;
            s32 danTexNum = -1;
            if (currentFloor >= 189 && msl::string::strstr(loadMap, "dan_44") != nullptr) {
                danTexNum = 1;
                overwriteMap = true;
            } else if (currentFloor >= 179 && msl::string::strstr(loadMap, "dan_44") != nullptr) {
                danTexNum = 0;
                overwriteMap = true;
            } else if (currentFloor >= 188 && currentFloor <= 190 && msl::string::strstr(loadMap, "dan_64") != nullptr) {
                danTexNum = 2;
                overwriteMap = true;
            }
            if (overwriteMap) {
                wii::tpl::TPLHeader * bossRoomTpl = allocTPL(danTexMapNames[danTexNum], "map", memory::HEAP_MAP, true);
                /*const char *danTexMapName = danTexMapNames[danTexNum];
                filemgr::FileEntry *bossRoomFile = filemgr::fileAllocf(0, "%s/map/%s.tpl", dvdRoot, danTexMapName);
                s32 tplSize = bossRoomFile->length;
                wii::tpl::TPLHeader *bossRoomTpl = (wii::tpl::TPLHeader *)memory::__memAlloc(memory::Heap::HEAP_MAP,
                tplSize); msl::string::memcpy(bossRoomTpl, bossRoomFile->sp->data, tplSize);
                filemgr::fileFree(bossRoomFile);
                wii::tpl::TPLBind(bossRoomTpl); // Initializes custom TPL in memory*/
                mapEntry->tpl = bossRoomTpl; // Replaces normal tpl for map with the boss room TPL
            } else {
                wii::tpl::TPLBind(mapEntry->tpl);
            }
        } else {
            wii::tpl::TPLBind(mapEntry->tpl);
        }
    }

    static const char * getNextDanMapnameNew(s32 dungeonNo) {
        switch (dungeonNo) {
        // Rest Floors
        case 9:
            return "dan_21"; // Ch1
            break;
        case 19:
            return "dan_22"; // Ch2
            break;
        case 29:
            return "dan_23"; // Ch3
            break;
        case 39:
            return "dan_24"; // Ch4
            break;
        case 49:
        case 59:
        case 69:
        case 79:
        case 89:
        case 99:
        case 109:
        case 119:
        case 129:
        case 139:
        case 149:
            return "dan_61"; // Ch5, almost all of these are failsafes
        case 159:
            return "dan_62"; // Ch6
            break;
        case 169:
            return "dan_63"; // Ch7
            break;
        case 179:
            return "dan_64"; // Ch8
            break;
        case 189:
            return "dan_64"; // Also Ch8, texture replaced dynamically
            break;
        case 199:
            return "dan_70"; // Shadoo
            break;
        // Enemy rooms
        default:
            // Flipside
            if ((dungeonNo >= 0) && (dungeonNo <= 9))
                return "dan_01";
            else if ((dungeonNo >= 10) && (dungeonNo <= 19))
                return "dan_02";
            else if ((dungeonNo >= 20) && (dungeonNo <= 29))
                return "dan_03";
            else if ((dungeonNo >= 30) && (dungeonNo <= 39))
                return "dan_04";
            else if ((dungeonNo >= 40) && (dungeonNo <= 149))
                return "dan_41";
            else if ((dungeonNo >= 150) && (dungeonNo <= 159))
                return "dan_42";
            else if ((dungeonNo >= 160) && (dungeonNo <= 169))
                return "dan_43";
            else if ((dungeonNo >= 170) && (dungeonNo <= 199)) // 71-79 are White Pure Heart, 81-89 are Bleck/Tippi, 91-99 are Shadoo Blank
                return "dan_44";
        }
        return nullptr;
    }

    static void debugModeGayFrame() {
        if (DebugMode) {
            framedrv::FrameEntry * frame = framedrv::framedrv_wp->entries;
            s32 cur = 0;
            for (cur = 0; cur < framedrv::framedrv_wp->num; cur = cur + 1) {
                frame->color = {(u8)frameR, (u8)frameG, (u8)frameB, 255};
                frame = frame + 1;
            }
        }
        msl::string::strncmp(spmario::gp->mapName, "ls", 2);
    }

    static s32 patchBackCursya() {
        if (msl::string::strstr(spmario::gp->mapName, "dan") == 0)
            seqdrv::seqSetSeq(seqdrv::SEQ_MAPCHANGE, "mac_02", "dokan_3");
        else
            seqdrv::seqSetSeq(seqdrv::SEQ_MAPCHANGE, "mac_05", "dokan_1");
        return 0;
    }

    void patchTitle(wii::tpl::TPLHeader * tpl) {
        TPLBind(tpl);
        if (msl::string::strcmp(seq_mapchange::seq_mapchange_wp->mapName, "title") != 0 && seqdrv::seqGetSeq() != 1)
            return;
        if (LPTitleTPLHeader == nullptr) {
            LPTitleTPLHeader = allocTPL("LPTITLE.tpl", nullptr, memory::HEAP_MAP, true);
            LPTitleTPLCurIndex = 0;
            LPTitleTPLCurN = 0;
        }
        s32 n = LPTitleTPLCurN++;
        if (n <= 2 || n == 5 || n == 6 || n == 10 || n == 12 || n == 15 || n == 17) {
            tplpatch::patchTpl2(0, LPTitleTPLCurIndex++, tpl, LPTitleTPLHeader, nullptr, 0, 1);
        } else if (n == 27) {
            LPTitleTPLCurIndex = 0;
            LPTitleTPLCurN = 0;
        }
        return;
    }

    // I hate how I had to do this; the ASM context for deciding big or small coins in vanilla is very complex. (It's also region-dependent. Fun.)
    #if defined(SPM_EU0) || defined(SPM_EU1)
    void makeCoinsBig(s32 null, s32 itemId) {
        asm("addi 27, 3, 14");
        asm("mr 4, 30");
        if (itemId == item_data::ITEM_ID_WORLD_COIN && system::irand(9) == 0 && VoucherGetStateById(VOUCHER_YELLOW) == V_ACTIVE) {
            VoucherProc(VOUCHER_YELLOW);
            asm("li 30, 2");
            return;
        }
        return;
    }
    #else
    void makeCoinsBig(s32 null, s32 itemId) {
        asm("addi 28, 3, 14");
        asm("mr 4, 30");
        if (itemId == item_data::ITEM_ID_WORLD_COIN && system::irand(9) == 0 && VoucherGetStateById(VOUCHER_YELLOW) == V_ACTIVE) {
            VoucherProc(VOUCHER_YELLOW);
            asm("li 30, 2");
            return;
        }
        return;
    }
    #endif

    void danResetLunatic() {
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        pouch->attack -= Lunatic->Stats.DemiseATK;
        pouch->maxHp -= Lunatic->Stats.DelightHP;
        if (pouch->hp > pouch->maxHp)
            pouch->hp = pouch->maxHp;
        for (s32 i = 0; i < VOUCHER_MAX; i += 1) {
            if (Lunatic->Voucher.Work[i] != nullptr) {
                memory::__memFree(0, Lunatic->Voucher.Work[i]->UW.Any);
                memory::__memFree(0, Lunatic->Voucher.Work[i]);
            }
        }
        if (Lunatic->Misc.savedCoins != 0) {
            mario_pouch::pouchSetCoin(Lunatic->Misc.savedCoins);
        }
        msl::string::memset(Lunatic, 0, sizeof(LunaticPitWork));
        item_data::itemDataTable[ITEM_ID_KEY_MAC_KEY_00].iconId = icondrv::ICON_MACHI_KEY;
        item_data::itemDataTable[ITEM_ID_KEY_MAC_KEY_00].nameMsg = "in_town_key_00";
        item_data::itemDataTable[ITEM_ID_KEY_MAC_KEY_00].descMsg = "msg_town_key_00";
        swdrv::swClear(1644); // Clear Blue% speedrun mode flag
        return;
    }

    static void neutralize() { return; }

    static void lpOnDeath() {
        /*s32 blessing = swdrv::swByteGet(1600);
        houraiActivation = swdrv::swGet(1671);
        if (blessing == BlessId::MERLUNA_HOURAI && !houraiActivation) {
            mario_motion::marioChgMot(MOT_DAMAGE);
            mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
            mario::marioKeyOff();
            swdrv::swSet(1671);
            houraiStoredAtk = pouch->attack;
            s32 halvedAtk = (f32)(msl::math::floor((f32)(pouch->attack / 2)));
            if ((halvedAtk * 2) < pouch->attack) // Workaround since ceil just doesn't seem to exist in SPM
            {
                pouch->attack = (halvedAtk + 1); // i.e. if original ATK is 5, desired result is 3. Floors to 2. 4 < 5, so 2 + 1 = 3.
            } else {
                pouch->attack = halvedAtk; // i.e. if original ATK is 4, desired result is 2. Floors to 2. 4 !< 4, so remains 2.
            }
            if (pouch->maxHp != 999) {
                pouch->maxHp = (pouch->maxHp + 10);
            } else {
                hpMaxed = true;
            }
            pouch->hp = pouch->maxHp;
            swdrv::swSet(1672);
            return;
        } else {*/
        swdrv::swByteSet(1600, 0);
        swdrv::swByteSet(1601, 0);
        swdrv::swByteSet(1602, 0);
        // Revert globals that affect Baldio state
        if (!swdrv::swGet(1631)) {
            mario::marioCharPoseNames[mario::PLAYER_MARIO][0] = "p_wii_mario";
            mario::marioCharBackPoseNames[mario::PLAYER_MARIO][0] = "p_wii_mario_r";
            msgpatch::msgpatchDelEntry("in_pc_mario");
            msgpatch::msgpatchDelEntry("msg_pc_mario");
        }
        mario_motion::marioChgMot(24); // MOT_DEATH
        return;
        //}
    }

    bool lpPostDeath() {
        bool finish = fadedrv::fadeIsFinish();
        if (finish)
            danResetLunatic();
        return finish;
    }

    void lpOnFileLoad() {
        if (msl::string::strcmp(spmario::gp->saveName, "Yme") == 0)
            lpEnableDebugMode();
        bool bald = false;
        if (!swdrv::swGet(1631) && spmario::gp->gsw0 != 0)
            bald = true;
        lpBaldioOnOff(bald);
        if (mario::marioGetPtr()->character == mario::PLAYER_MARIO)
            mario_motion::_marioChangeCharacter(mario::PLAYER_MARIO); // Refresh model
    }

    void levelUpHalveHPBonus() {
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        mario_pouch::pouchSetHp(pouch->hp + round((f32)pouch->maxHp / 2.0f));
        return;
    }

    s32 marioMakeBald(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        lpBaldioOnOff((bool)evtmgr_cmd::evtGetValue(evtEntry, args[0]));
        if (mario::marioGetPtr()->character == mario::PLAYER_MARIO) {
            mario_motion::_marioChangeCharacter(mario::PLAYER_MARIO); // Refresh model
            mario::marioGetPtr()->curPoseName = "P_1";
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(marioMakeBald, 1)

    void marioChgBaseSpeedStats() {
        mario::MarioWork * mario = mario::marioGetPtr();
        if (mario->character == mario::PLAYER_MARIO && !swdrv::swGet(1631)) {
            mario->walkSpeed *= 1.01969;
            mario->dashSpeed *= 1.01969;
            mario->swimSpeed *= 1.01969;
        }
        mario_hit::func_80131ee8();
        return;
    }

    // thanks lily :D
    f32 cudgeFloat = 0.5f;
    u32 npcTakeDamageRetLocation = (u32)(&spm::npcdrv::npcTakeDamage) + 0x1E0;
    void setCudgeFloat() {
        u32 addr_float = (u32)&cudgeFloat;
        u32 addr_ret = (u32)&npcTakeDamageRetLocation;

        asm("lfs 0, 0(%1)\n" // Load the float into f0

            "lwz 12, 0(%0)\n" // Load Cudge
            "mtctr 12\n"      // Move to ctr
            "bctr\n"          // Branch to Cudge
            :
            : "r"(addr_ret), "r"(addr_float)
            : "r12", "memory");
    }

    static void OnDanCountdown() {
        hud::hud_wp->countdownTimer -= 1;
        s32 disorderId = (s32)Lunatic->Luna.disorder;
        // Handle random Disorder tremors
        if (disorderId > 0)
            DisorderHandleTremors();
        // Prejudice
        if (disorderId == DisorderId::DISORDER_YELLOW)
            PrejudiceAction();
        // Melancholy
        if (disorderId == DisorderId::DISORDER_WHITE)
            MelancholyAction();
        return;
    }

    s32 handleBlessingWearOff(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        houraiActivation = swdrv::swGet(1671);
        if (houraiActivation) {
            swdrv::swClear(1671);
            mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
            pouch->attack = houraiStoredAtk;
            s32 currentHp = pouch->hp;
            if (!hpMaxed) {
                pouch->maxHp = (pouch->maxHp - 10);
            }
            if (currentHp > pouch->maxHp) {
                pouch->hp = pouch->maxHp;
            }
        }
        return 2;
    }

    void marioAfterimgInheritMaterialEvtColor(s32 animPoseId, wii::gx::GXColor color) {
        mario::MarioWork * mario = mario::marioGetPtr();
        if ((mario->effectFlags & 0xC) == 0 && Lunatic->Misc.marioFullColor.a == 0) {
            wii::gx::GXColor col = animdrv::animPoseGetMaterialEvtColor(animPoseId);
            if (!swdrv::swGet(1612)) // Mario NOT removed
                col.a = color.a;
            animdrv::animPoseSetMaterialEvtColor(animPoseId, col);
        } else {
            animdrv::animPoseSetMaterialEvtColor(animPoseId, color);
        }
        return;
    }

    // Returns true to cancel sfx, returns false to play sfx
    bool sndMuteLowHpSfx(const char * sfxName) {
        if (swdrv::swGet(1621) == true)
            return false;
        if (msl::string::strcmp(sfxName, "SFX_SYS_PINCH1") == 0 || msl::string::strcmp(sfxName, "SFX_SYS_DANGER1") == 0)
            return true;
        return false;
    }

    /*
        Rewrite animGroupBaseAsync to utilize a failsafe instead of an assert
        If an entry is not in ag2tg, it will assume the texture name and model name are identical
    */
    u32 animGroupBaseAsyncNew(const char * animPoseName, s32 param_2, void * readDoneCb) {
        char curAnim[32], targetAnim[32];
        u32 size = wii::cx::CXGetUncompressedSize(lz_embedded::ag2tg), i = 0;
        const char * ag2tg = (const char *)animdrv::animdrv_wp->ag2tg->data;
        filemgr::FileEntry * model = filemgr::fileAsyncf(5, readDoneCb, "a/%s", animPoseName);
        filemgr::FileEntry * texture = nullptr;
        msl::string::strcpy(targetAnim, animPoseName);
        msl::string::strlwr(targetAnim);
        for (; (msl::string::strcmp(curAnim, targetAnim) == 0) || ((i * 64) >= size); i += 1) {
            msl::string::strcpy(curAnim, (ag2tg + (i * 64)));
            msl::string::strlwr(curAnim);
        }
        if ((i * 32) >= size) {
            texture = filemgr::fileAsyncf(4, nullptr, "a/%s-", animPoseName);
        } else {
            texture = filemgr::fileAsyncf(4, nullptr, "a/%s-", (ag2tg + (i * 64) + 32));
        }
        if ((s32)model == -1) {
            size = 1;
        } else if (model == nullptr) {
            size = 0;
        } else if ((s32)texture == -1) {
            size = 1;
        } else
            size = (-(s32)texture | (u32)texture) >> 0x1f;
        return size;
    }

    void stylish_10() {
        mario::MarioWork * mario = mario::marioGetPtr();
        mario->curStylishXp = round_to(mario->curStylishXp, 10);
        mario->jumpCombo += 1;
        return;
    }

    static const char * (*searchGetNpcMsgReal)(npcdrv::NPCEntry * npc);
    void (*spsndSFXOnReal)(const char * name);
    void (*spsndSFXOnVolReal)(const char * name, u8 volume);
    void (*seq_titleExitReal)(seqdrv::SeqWork * work);
    void (*nandLoadSaveReal)(s32 saveId);
    s32 (*evt_map_set_blend)(evtmgr::EvtEntry * evtEntry, bool firstCall);
    s32 (*evt_map_blend_off)(evtmgr::EvtEntry * evtEntry, bool firstCall);
    itemdrv::ItemEntry * (*itemEntryReal)(const char * name, s32 type, s32 behaviour, f32 x, f32 y, f32 z, evtmgr::EvtScriptCode * pickupScript,
                                          evtmgr::EvtVar switchNumber);
    s32 (*itemCollectPouchItemReal)(itemdrv::ItemEntry * item);
    static void miscLambdas() {
        searchGetNpcMsgReal = patch::hookFunction(search::searchGetNpcMsg, [](npcdrv::NPCEntry * npc) {
            // Patch all custom/modified NPCs to return a custom tattle instead of a message name that doesn't exist
            const char * tattle = msgSearchCustomNpc(npc);
            if (tattle == nullptr) {
                return searchGetNpcMsgReal(npc);
            } else if (msl::string::strcmp(tattle, "") == 0) {
                return searchGetNpcMsgReal(npc);
            } else {
                return tattle;
            }
        });

        spsndSFXOnReal = patch::hookFunction(spmario_snd::spsndSFXOn, [](const char * name) {
            if (sndMuteLowHpSfx(name) == false)
                spmario_snd::__spsndSFXOn(name, -1, 255, 0, 0, 0);
            return;
        });

        spsndSFXOnVolReal = patch::hookFunction(spmario_snd::spsndSFXOnVol, [](const char * name, u8 volume) {
            if (sndMuteLowHpSfx(name) == false)
                spmario_snd::__spsndSFXOn(name, volume, 255, 0, 0, 0);
            return;
        });

        itemEntryReal = patch::hookFunction(
            itemdrv::itemEntry, [](const char * name, s32 type, s32 behaviour, f32 x, f32 y, f32 z, evtmgr::EvtScriptCode * pickupScript, evtmgr::EvtVar switchNumber) {
                if (type == ITEM_ID_KEY_MAC_KEY_00) // Chest keys no longer fly away
                    behaviour = 1;
                itemdrv::ItemEntry * item = itemEntryReal(name, type, behaviour, x, y, z, pickupScript, switchNumber);
                if (type == 1 && switchNumber == 0 && behaviour != 0) // Coins dropped by enemies are no longer searchable
                    item->flags |= 0x4000000;
                if (type == ITEM_ID_KEY_MAC_KEY_00) // Chest keys no longer disappear when dropped & drop confetti
                {
                    spmario_snd::spsndSFXOnVol("SFX_I_STAR_MEDAL1", 100);
                    item->flags &= ~0x200;
                    eff_small_star::effSmallStarEntry(x, y, z, 0, -1.0, 0, 4, 8);
                    //    effpatch::effpatchColorMaskEntry(star, {0, 0, 0, 255}, {255, 255, 255, 255}, nullptr);
                }
                if (type == ITEM_ID_KEY_DAN_KEY)
                    spmario_snd::spsndSFXOnVol("SFX_I_BRUNK_APPEAR1", 73);
                return item;
            });

        itemCollectPouchItemReal = patch::hookFunction(itemdrv::itemCollectPouchItem, [](itemdrv::ItemEntry * item) {
            s32 ret = itemCollectPouchItemReal(item);
            if (msl::string::strstr(spmario::gp->mapName, "dan") != nullptr) {
                if (item->type == ITEM_ID_KEY_MAC_KEY_00) {
                    if (mario_pouch::pouchCheckHaveItem(ITEM_ID_KEY_MAC_KEY_00) == true) {
                        Lunatic->RFC.chestKeysOwned += 1;
                        mario_pouch::pouchRemoveItem(ITEM_ID_KEY_MAC_KEY_00);
                        OSREPORTF("Chest keys: %d (%p)\n", Lunatic->RFC.chestKeysOwned, &Lunatic->RFC.chestKeysOwned);
                    }
                } else if (item->type == ITEM_ID_KEY_DAN_KEY) {
                    if (mario_pouch::pouchCheckHaveItem(ITEM_ID_KEY_DAN_KEY) == true) {
                        // Determination
                        VoucherProc(VOUCHER_RED);
                    }
                }
            }
            if (mario::marioGetPtr()->motionId == MOT_SPINDASH)
                mario_motion::marioChgMot(MOT_STAY);
            return ret;
        });

        seq_titleExitReal = patch::hookFunction(seq_title::seq_titleExit, [](seqdrv::SeqWork * work) {
            seq_titleExitReal(work);
            LPTitleTPLHeader = nullptr;
            return;
        });

        nandLoadSaveReal = patch::hookFunction(nandmgr::nandLoadSave, [](s32 saveId) {
            nandLoadSaveReal(saveId);
            lpOnFileLoad();
            return;
        });

        evt_map_set_blend = patch::hookFunction(evt_map::evt_map_set_blend, [](evtmgr::EvtEntry * evtEntry, bool firstCall) {
            if (Lunatic->Misc.hardShadooBlockColChg == true)
                return 2;
            return evt_map_set_blend(evtEntry, firstCall);
        });

        evt_map_blend_off = patch::hookFunction(evt_map::evt_map_blend_off, [](evtmgr::EvtEntry * evtEntry, bool firstCall) {
            if (Lunatic->Misc.hardShadooBlockColChg == true)
                return 2;
            return evt_map_blend_off(evtEntry, firstCall);
        });
    }

    s32 SaveWriteModVersion(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 saveId = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 len = msl::string::strlen(MOD_VERSION);
        assertf(len <= 48, "MOD_VERSION is too long (%s) @ %d", MOD_VERSION, len);
        if (saveId == -1)
            msl::string::strcpy((char *)&spmario::gp->gsw[2000], MOD_VERSION);
        else
            msl::string::strcpy((char *)&nandmgr::nandmgr_wp->saves[saveId - 1].spmarioGlobals.gsw[2000], MOD_VERSION);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(SaveWriteModVersion, 1)

    s32 SaveChkModVersion(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 saveId = evtmgr_cmd::evtGetValue(evtEntry, args[0]) - 1;
        const char * mod_ver = (char *)&nandmgr::nandmgr_wp->saves[saveId].spmarioGlobals.gsw[2000];
        if (msl::string::strcmp(mod_ver, MOD_VERSION) == 0 || nandmgr::nandmgr_wp->saves[saveId].spmarioGlobals.gsw0 == 0) {
            evtmgr_cmd::evtSetValue(evtEntry, args[1], 0);
        } else if ((s32)*mod_ver != 0) {
            evtmgr_cmd::evtSetValue(evtEntry, args[1], 1);
        } else {
            evtmgr_cmd::evtSetValue(evtEntry, args[1], 2);
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(SaveChkModVersion, 2)

    EVT_BEGIN(new_savefile_start_evt)
    SET(GF(0), 0)
    SET(LW(3), LW(0))
    USER_FUNC(SaveChkModVersion, LW(3), LW(1))
    SWITCH(LW(1))
    CASE_EQUAL(0)
    USER_FUNC(evt_msg::evt_msg_print_insert, 0, PTR("msg_savefile_start"), 0, 0, LW(3))
    CASE_EQUAL(1)
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(savefile_start_oldversion), 0, 0, LW(3))
    CASE_EQUAL(2)
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(savefile_start_noversion), 0, 0, LW(3))
    END_SWITCH()
    USER_FUNC(evt_msg::evt_msg_select, 0, PTR("msg_mcard_yesno"))
    SET(LW(2), LW(0))
    IF_LARGE_EQUAL(LW(1), 1)
    IF_EQUAL(LW(2), 0)
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(savefile_start_update))
    USER_FUNC(evt_msg::evt_msg_select, 0, PTR("msg_mcard_yesno"))
    IF_EQUAL(LW(0), 0)
    USER_FUNC(SaveWriteModVersion, LW(3))
    END_IF()
    END_IF()
    END_IF()
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_NOT_EQUAL(LW(2), 0)
    SET(GF(0), 1)
    END_IF()
    RETURN()
    EVT_END()

    evtmgr::EvtEntry * savefileStartEvtOverwrite() {
        return evtmgr::evtEntryType(new_savefile_start_evt, 0, 0, 0);
    }

    static void danOverwrite() {
        patch::hookFunction(dan::evt_dan_read_data, evt_dan_read_data_new);
        patch::hookFunction(seq_title::getNextDanMapname, getNextDanMapnameNew);
        writeBranchLink(mapdrv::mapLoad, 0x3B8, loadNewDanTex);
        writeBranchLink(mot_damage::onHpEquals0, 0x68, lpOnDeath);
        writeBranchLink(seq_gameover::seq_gameOverMain, 0x10C, lpPostDeath);
        writeBranchLink(hud::hudMain, 0x47C, OnDanCountdown);
        writeBranchLink(levelupwindow::levelUpWindowMain, 0x2D0, levelUpHalveHPBonus);
        // just throwing this here
        writeBranchLink(temp_unk::backCursyaHit, 0x104, patchBackCursya);
        // also throwing these here
        writeBranchLink(npcdrv::func_801b19e4, 0x6B0, neutralize);
        writeBranchLink(npcdrv::func_801b19e4, 0x694, neutralize);
        writeBranchLink(npcdrv::func_801b19e4, 0x588, neutralize);
        writeBranchLink(npcdrv::func_801b19e4, 0x494, neutralize);
        // lol why not
        writeBranchLink(framedrv::frameDisp, 0x3EC, debugModeGayFrame);
        // patch AGB Async
        patch::hookFunction(animdrv::animGroupBaseAsync, animGroupBaseAsyncNew);
        // round all stylish xp to nearest 10
        writeBranchLink(npcdrv::npcTakeDamage, 0xDA4, stylish_10);
        writeBranchLink(npcdrv::npcTakeDamage, 0xC9C, stylish_10);
        // cudge patch - thanks lily!
        writeBranch(npcdrv::npcTakeDamage, 0x1DC, setCudgeFloat);
        // make coins big justice i guess
        #if defined(SPM_EU0) || defined(SPM_EU1)
        writeBranchLink(npcmisc::npcDropItem, 0x320, makeCoinsBig);
        #else
        writeBranchLink(npcmisc::npcDropItem, 0x2B4, makeCoinsBig);
        #endif
        // patch title screen
        writeBranchLink(temp_unk::func_802f2860, 0x38, patchTitle);
        // add mod version check to save load
        writeBranchLink(seq_load_sub::loadMain, 0x880, savefileStartEvtOverwrite);
        // aerodynamic
        writeBranchLink(mario::marioUpdateCharProperties, 0xD4, marioChgBaseSpeedStats);
        writeBranchLink(mario::marioUpdateCharProperties, 0x148, marioChgBaseSpeedStats);
        writeBranchLink(mario::marioUpdateCharProperties, 0x228, marioChgBaseSpeedStats);
        // Inherit color during spindash effect
        writeBranchLink(mario::marioDrawAfterimg, 0x198, marioAfterimgInheritMaterialEvtColor);
        // Remove anything that sets or reads npcentry->unkShellSfx
        writeWord(npcdrv::func_801cdb84, 0xB6C, NOP); // remove the call to play unkShellSfx
        writeWord(evt_npc::evt_npc_set_unk_shell_sfx, 0x58, NOP);
        // Patch shell enemies to not despawn within the Pit (distance check goes from 500 to 1000)
        writeWord(npcdrv::func_801c8d70, 0x954, FCMPO(0, 1, 28));
// Reverse-patch late revisions to allow more lock interactions (Surprisingly, US1 didn't introduce this!)
#if defined(SPM_EU0) || defined(SPM_EU1) || defined(SPM_US2)
        writeWord(evt_mobj::mobj_zyo, 0x54, BNE(0x44));
#endif
    }

    static void danDontFuckingCrash() { patch::hookFunction(dan::evt_dan_get_enemy_info, evt_dan_get_enemy_info_new); }

    s32 updateDebugFrameColor(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        // Every frame, progress color(s) by 4 units.
        // PHASE 0    PHASE 1      PHASE 2    PHASE 3      PHASE 4    PHASE 5      PHASE 0
        // 255,0,0 -> 255,255,0 -> 0,255,0 -> 0,255,255 -> 0,0,255 -> 255,0,255 -> 255,0,0
        switch (frameColPhase) {
        case 0:
            frameG = frameG + 16;
            if (frameG > 255) {
                frameG = 255;
                frameColPhase = 1;
            }
            break;
        case 1:
            frameR = frameR - 16;
            if (frameR < 0) {
                frameR = 0;
                frameColPhase = 2;
            }
            break;
        case 2:
            frameB = frameB + 16;
            if (frameB > 255) {
                frameB = 255;
                frameColPhase = 3;
            }
            break;
        case 3:
            frameG = frameG - 16;
            if (frameG < 0) {
                frameG = 0;
                frameColPhase = 4;
            }
            break;
        case 4:
            frameR = frameR + 16;
            if (frameR > 255) {
                frameR = 255;
                frameColPhase = 5;
            }
            break;
        case 5:
            frameB = frameB - 16;
            if (frameB < 0) {
                frameB = 0;
                frameColPhase = 0;
            }
            break;
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(updateDebugFrameColor, 0)

    s32 bump_use_selected(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        pausewin::pausewin_pluswinWp->selectedItemId = 45;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(bump_use_selected, 0)

    s32 DebugModeGetStatus(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)DebugMode);
        return 2;
    }

    wii::gx::GXColor marioIfHeWerentBlue = {255, 255, 255, 255};
    wii::gx::GXColor marioIfHeWereBlue = {60, 100, 255, 255};
    wii::gx::GXColor marioIfHeWerent = {255, 255, 255, 0};

    void hijackFullMarioColor(wii::gx::GXTexObj * texObj) {
        wii::gx::GXSetTevColor(1, &Lunatic->Misc.marioFullColor);
        return;
    }

    s32 animPoseSetMaterialEvtColorWrapper(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        mario::MarioWork * mario = mario::marioGetPtr();
        bool marioRemoved = swdrv::swGet(1613);
        bool blueToggle = swdrv::swGet(1643);
        s32 i;
        for (i = 0; i < 8; ++i) {
            if (mario->animPoseIds[i] != -1) {
                if (mario->character == 0 && marioRemoved) {
                    animdrv::animPoseSetMaterialEvtColor(mario->animPoseIds[i], marioIfHeWerent);
                    animdrv::animPoseSetMaterialFlagOn(mario->animPoseIds[i], 0x2000);
                    mario->effectFlags &= ~0x3; // Remove trail effects
                } else if (blueToggle) {
                    animdrv::animPoseSetMaterialEvtColor(mario->animPoseIds[i], marioIfHeWereBlue);
                    animdrv::animPoseSetMaterialFlagOn(mario->animPoseIds[i], 0x2000);
                } else {
                    if (mario->animPoseIds[i] != -1) {
                        animdrv::animPoseSetMaterialEvtColor(mario->animPoseIds[i], marioIfHeWerentBlue);
                        animdrv::animPoseSetMaterialFlagOff(mario->animPoseIds[i], 0x2000);
                    }
                }
                if (Lunatic->Misc.marioFullColor.a != 0 && !(mario->character == 0 && marioRemoved)) {
                    animdrv::animPoseSetDispCallback2(mario->animPoseIds[i], (void *)hijackFullMarioColor, nullptr);
                } else {
                    animdrv::animPoseSetDispCallback2(mario->animPoseIds[i], nullptr, nullptr);
                }
            }
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(animPoseSetMaterialEvtColorWrapper, 0)

    s32 update_bump_tex(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        // Whacka Bump
        if (swdrv::swGet(1643)) {
            item_data::itemDataTable[45].iconId = TPLPATCH_ICON(ICON_BUMP_GRAY);
            if (Lunatic->Misc.marioFullColor.a != 0) {
                Lunatic->Misc.marioFullColor = marioIfHeWereBlue;
            }
        } else {
            item_data::itemDataTable[45].iconId = TPLPATCH_ICON(ICON_BUMP_BLUE);
            if (Lunatic->Misc.marioFullColor.a != 0) {
                Lunatic->Misc.marioFullColor = marioIfHeWerentBlue;
            }
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(update_bump_tex, 0)

    EVT_BEGIN(bump_use_evt)
    IF_EQUAL(GSWF(1643), 0)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(bumpUseMsg), 0, 0)
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(bumpUnuseMsg), 0, 0)
    END_IF()
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(yesNoSelect_YesByDefault))
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_EQUAL(LW(0), 0)
    USER_FUNC(bump_use_selected)
    END_IF()
    RETURN()
    EVT_END()

    EVT_BEGIN(bump_item_use)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    // GENERATE EFFS
    // WHITE
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(10), LW(11), LW(12))
    USER_FUNC(evt_mario::evt_mario_get_height, LW(13))
    DIVF(LW(13), FLOAT(2.0))
    ADDF(LW(11), LW(13))
    ADDF(LW(10), FLOAT(10))
    ADDF(LW(12), FLOAT(10))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(10), LW(11), LW(12))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 8, LW(10), LW(11), LW(12), FLOAT(1.5), 0, 0, 0, 0, 0, 0, 0)
    SUBF(LW(12), FLOAT(20))
    SUBF(LW(10), FLOAT(20))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 8, LW(10), LW(11), LW(12), FLOAT(1.5), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(30)
    // PURPLE
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(10), LW(11), LW(12))
    USER_FUNC(evt_mario::evt_mario_get_height, LW(13))
    DIVF(LW(13), FLOAT(2.0))
    ADDF(LW(11), LW(13))
    ADDF(LW(10), FLOAT(10))
    ADDF(LW(12), FLOAT(10))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(10), LW(11), LW(12))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 13, LW(10), LW(11), LW(12), FLOAT(2.2), 0, 0, 0, 0, 0, 0, 0)
    SUBF(LW(12), FLOAT(20))
    SUBF(LW(10), FLOAT(20))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 13, LW(10), LW(11), LW(12), FLOAT(2.2), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_11"), 0)
    // END EFF GENERATION
    IF_EQUAL(GSWF(1643), 0)
    SET(GSWF(1643), 1)
    ELSE()
    SET(GSWF(1643), 0)
    END_IF()
    USER_FUNC(update_bump_tex)
    WAIT_MSEC(1200)
    IF_EQUAL(GSWF(1643), 0)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(bumpYoureNotBlueNow), 0, 0)
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(bumpYoureBlueNow), 0, 0)
    END_IF()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    EVT_END()

    EVT_BEGIN(itemEventEvtHook)
    // Check if item being used is the Mysterious Whacka Bump
    IF_EQUAL(LW(5), 45)
    RUN_EVT(bump_item_use)
    RETURN()
    END_IF()
    // Check if Kindness is torn
    USER_FUNC(EvtVoucherChkTorn, (s32)VOUCHER_GREEN, LW(8))
    IF_EQUAL(LW(8), 1)
    MULF(LW(6), FLOAT(1.2))
    END_IF()
    // Check if Recalcitrance is active
    USER_FUNC(DisorderGetId, LW(8))
    IF_EQUAL(LW(8), (s32)DISORDER_CYAN)
    USER_FUNC(RecalcitranceCalcHealing, LW(6), LW(6))
    END_IF()
    RETURN_FROM_CALL()

    static mario_pouch::MarioPouchWork * pluswinWhackaBumpFunc() {
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        if (pouch->keyItem[pausewin::pausewin_pluswinWp->submenuSelectedButton] == 45) {
            spmario_snd::spsndSFXOn("SFX_SYS_MENU_DESIDE1");
            pausewin::pausewin_pluswinWp->keyItemEvt = evtmgr::evtEntryType((evtmgr::EvtScriptCode *)bump_use_evt, 0, 0, 0);
            pausewin::pausewin_pluswinWp->keyItemEvtId = pausewin::pausewin_pluswinWp->keyItemEvt->id;
        }
        return pouch;
    }

    static void pluswinWhackaBump() { writeBranchLink(pausewin::pluswinKeyItemMain, 0x5E4, pluswinWhackaBumpFunc); }

    void DanEnemyNegativeDispCb(wii::gx::GXTexObj * texObj) {
        wii::gx::GXSetTevOrder(texObj->stage, 255, 255, 255);
        wii::gx::GXSetBlendMode(2, 2, 2, 12);
        wii::gx::GXSetAlphaUpdate(1);
        wii::gx::GXSetColorUpdate(1);
        wii::gx::GXSetTevColorIn(texObj->stage, 0, 0, 0, 15);
        wii::gx::GXSetTevAlphaIn(texObj->stage, 7, 7, 7, 0);
        texObj->stage += 1;
        return;
    }

    static void patchNpcRgbaFuncs() {
        patch::hookFunction(evt_npc::evt_npc_set_color, [](evtmgr::EvtEntry * evtEntry, bool firstRun) {
            // Patch all NPCs with Tribe IDs to ignore this function, except for Shadoo.
            evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
            const char * name = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
            npcdrv::NPCEntry * npc = evt_npc::evtNpcNameToPtr(evtEntry, name);
            if (npc->tribeId > 0) {
                if (npc->tribeId <= 333 && npc->tribeId >= 330) {
                    (npc->m_Anim).red = 127;
                    (npc->m_Anim).green = 127;
                    (npc->m_Anim).blue = 127;
                    animdrv::animPoseSetDispCallback2((npc->m_Anim).m_nPoseId, (void *)mi4::mi4MimiHolographicEffect, evtEntry);
                }
            } else {
                s32 red = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
                s32 green = evtmgr_cmd::evtGetValue(evtEntry, args[2]);
                s32 blue = evtmgr_cmd::evtGetValue(evtEntry, args[3]);
                s32 alpha = evtmgr_cmd::evtGetValue(evtEntry, args[4]);
                (npc->m_Anim).red = (u8)clamp(red, 0, 255);
                (npc->m_Anim).blue = (u8)clamp(blue, 0, 255);
                (npc->m_Anim).green = (u8)clamp(green, 0, 255);
                (npc->m_Anim).alpha = (u8)clamp(alpha, 0, 255);
            }
            return 2;
        });

        patch::hookFunction(evt_npc::evt_npc_set_part_color, [](evtmgr::EvtEntry * entry, bool isFirstCall) {
            (void)entry;
            (void)isFirstCall;
            return 2;
        });
        patch::hookFunction(evt_npc::evt_npc_set_all_part_color, [](evtmgr::EvtEntry * entry, bool isFirstCall) {
            (void)entry;
            (void)isFirstCall;
            return 2;
        });
    }

    static void danPatchPitEnemies() {
        // Define default animposes for new enemies
        npcdrv::npcTribes[5].animPoseName = "e_kuribo_y";     // Unused Spiked Goomba --> Spiked Gloomba
        npcdrv::npcTribes[2].animPoseName = "e_kuribo_h";     // Dark Goomba --> Hyper Goomba
        npcdrv::npcTribes[6].animPoseName = "e_kuribo_h";     // Dark Spiked Goomba --> Hyper Spiked Goomba
        npcdrv::npcTribes[8].animPoseName = "e_kuribo_h";     // Dark Paragoomba --> Hyper Paragoomba
        npcdrv::npcTribes[16].animPoseName = "e_nokoteki_d";  // Dark Koopa --> Dark Koopa
        npcdrv::npcTribes[24].animPoseName = "e_nokoteki_d";  // Dark Paratroopa --> Dark Paratroopa
        npcdrv::npcTribes[446].animPoseName = "e_cheririn_a"; // Dark Cherbil --> Ash Cherbil
        npcdrv::npcTribes[38].animPoseName = "e_togezb";      // Dark Spiny --> Sky-Blue Spiny
        npcdrv::npcTribes[29].animPoseName = "e_mer";         // Dark Spike Top --> Red Spike Top
        npcdrv::npcTribes[98].animPoseName = "e_chorobon_g";  // Dark Fuzzy --> Green Fuzzy
        npcdrv::npcTribes[43].animPoseName = "e_karon_d";     // Dark Dull Bones --> Dark Bones
        npcdrv::npcTribes[44].animPoseName = "e_karon_d";     // Dark Bones projectile
        npcdrv::npcTribes[111].animPoseName = "e_kmond";      // Dark Ruff Puff --> Dark Puff
        npcdrv::npcTribes[95].animPoseName = "e_gabow";       // Dark Clubba --> White Clubba
        npcdrv::npcTribes[66].animPoseName = "e_kames";       // Dark Magikoopa --> Shady Magikoopa
        npcdrv::npcTribes[67].animPoseName = "e_kames";       // Dark Broom Magikoopa --> Shady Broom Magikoopa
        npcdrv::npcTribes[124].animPoseName = "e_wanwan_g";   // Dark Chomp --> Gold Chomp
        npcdrv::npcTribes[167].animPoseName = "e_ntl_p";      // Dark Tileoid --> Tileoid PU
        npcdrv::npcTribes[220].animPoseName = "e_nin_d";      // Dark Ninjoe --> Ninjeremiah
        npcdrv::npcTribes[221].animPoseName = "e_nin_d";      // Ninjeremiah projectile
        npcdrv::npcTribes[19].animPoseName = "e_togenokd";    // Dark Koopatrol --> Dark Koopatrol
        npcdrv::npcTribes[171].animPoseName = "e_jyama_b";    // Dark Jawbus --> Bawbus
        npcdrv::npcTribes[10].animPoseName = "e_k_kuribo";    // Dark Headbonk Goomba --> Kamikaze Goomba
        npcdrv::npcTribes[225].animPoseName = "e_tesita_bt";  // Dark Skellobit --> Skellobyte
        npcdrv::npcTribes[228].animPoseName = "e_tesita_sb";  // Dark Spiky Skellobit --> Spiky Skellobyte
        npcdrv::npcTribes[49].animPoseName = "e_buross_h";    // Dark Hammer Bro --> Shady Hammer Bro
        npcdrv::npcTribes[50].animPoseName = "e_buross_h";    // Shady Hammer Bro projectile
        npcdrv::npcTribes[55].animPoseName = "e_buross_b";    // Dark Boomerang Bro --> Shady Boomerang Bro
        npcdrv::npcTribes[56].animPoseName = "e_buross_b";    // Shady Boomerang Bro projectile
        npcdrv::npcTribes[61].animPoseName = "e_burosu_i";    // Dark Fire Bro --> Ice Bro
        npcdrv::npcTribes[62].animPoseName = "e_burosu_i";    // Ice Bro projectile
        npcdrv::npcTribes[101].animPoseName = "e_sinemoh";    // Dark Cleft --> Hyper Cleft
        npcdrv::npcTribes[75].animPoseName = "e_k_shoote4";   // Dark Striker --> Shady Striker
        npcdrv::npcTribes[86].animPoseName = "e_teresa_b";    // Dark Dark Boo --> Bomb Boo

        // Replace the Sky-Blue Spiny's parts list so its partsList[1] animPoseName displays properly as e_togezb
        msl::string::memcpy(&sbParts[0], &npcdrv::npcTribes[38].partsList[0], sizeof(npcdrv::npcTribes[38].partsList[0]));
        msl::string::memcpy(&sbParts[1], &npcdrv::npcTribes[38].partsList[1], sizeof(npcdrv::npcTribes[38].partsList[1]));
        npcdrv::npcTribes[38].partsList = sbParts;
        npcdrv::npcTribes[38].partsList[1].animPoseName = "e_togezb";

        // Patch Gold Chomps so their parts appear correctly!
        msl::string::memcpy(&gcParts[0], &npcdrv::npcTribes[124].partsList[0], sizeof(npcdrv::npcTribes[124].partsList[0]));
        msl::string::memcpy(&gcParts[1], &npcdrv::npcTribes[124].partsList[1], sizeof(npcdrv::npcTribes[124].partsList[1]));
        msl::string::memcpy(&gcParts[2], &npcdrv::npcTribes[124].partsList[2], sizeof(npcdrv::npcTribes[124].partsList[2]));
        msl::string::memcpy(&gcParts[3], &npcdrv::npcTribes[124].partsList[3], sizeof(npcdrv::npcTribes[124].partsList[3]));
        msl::string::memcpy(&gcParts[4], &npcdrv::npcTribes[124].partsList[4], sizeof(npcdrv::npcTribes[124].partsList[4]));
        msl::string::memcpy(&gcParts[5], &npcdrv::npcTribes[124].partsList[5], sizeof(npcdrv::npcTribes[124].partsList[5]));
        msl::string::memcpy(&gcParts[6], &npcdrv::npcTribes[124].partsList[6], sizeof(npcdrv::npcTribes[124].partsList[6]));
        npcdrv::npcTribes[124].partsList = gcParts;
        npcdrv::npcTribes[124].partsList[1].animPoseName = "e_wanwan_g";
        npcdrv::npcTribes[124].partsList[2].animPoseName = "e_wanwan_g";
        npcdrv::npcTribes[124].partsList[3].animPoseName = "e_wanwan_g";
        npcdrv::npcTribes[124].partsList[4].animPoseName = "e_wanwan_g";
        npcdrv::npcTribes[124].partsList[5].animPoseName = "e_wanwan_g";
        npcdrv::npcTribes[124].partsList[6].animPoseName = "e_wanwan_g";
        item_data::itemDataTable[359].animPoseName = "e_wanwan_g";
        npcdrv::npcTribes[124].coinDropChance = 100;
        npcdrv::npcTribes[124].coinDropBaseCount = 25;
        npcdrv::npcTribes[124].coinDropExtraChance = 80;
        npcdrv::npcTribes[124].coinDropExtraMax = 50;

        // Bawbus
        msl::string::memcpy(&bbParts[0], &npcdrv::npcTribes[171].partsList[0], sizeof(npcdrv::npcTribes[171].partsList[0]));
        msl::string::memcpy(&bbParts[1], &npcdrv::npcTribes[171].partsList[1], sizeof(npcdrv::npcTribes[171].partsList[1]));
        msl::string::memcpy(&bbParts[2], &npcdrv::npcTribes[171].partsList[2], sizeof(npcdrv::npcTribes[171].partsList[2]));
        msl::string::memcpy(&bbParts[3], &npcdrv::npcTribes[171].partsList[3], sizeof(npcdrv::npcTribes[171].partsList[3]));
        msl::string::memcpy(&bbParts[4], &npcdrv::npcTribes[171].partsList[4], sizeof(npcdrv::npcTribes[171].partsList[4]));
        msl::string::memcpy(&bbParts[5], &npcdrv::npcTribes[171].partsList[5], sizeof(npcdrv::npcTribes[171].partsList[5]));
        msl::string::memcpy(&bbParts[6], &npcdrv::npcTribes[171].partsList[6], sizeof(npcdrv::npcTribes[171].partsList[6]));
        msl::string::memcpy(&bbParts[7], &npcdrv::npcTribes[171].partsList[7], sizeof(npcdrv::npcTribes[171].partsList[7]));
        msl::string::memcpy(&bbParts[8], &npcdrv::npcTribes[171].partsList[8], sizeof(npcdrv::npcTribes[171].partsList[8]));
        msl::string::memcpy(&bbParts[9], &npcdrv::npcTribes[171].partsList[9], sizeof(npcdrv::npcTribes[171].partsList[9]));
        npcdrv::npcTribes[171].partsList = bbParts;
        npcdrv::npcTribes[171].partsList[1].animPoseName = "e_jyama_b";
        npcdrv::npcTribes[171].partsList[4].animPoseName = "e_jyama_b";
        npcdrv::npcTribes[171].partsList[5].animPoseName = "e_jyama_b";
        npcdrv::npcTribes[171].partsList[6].animPoseName = "e_jyama_b";
        npcdrv::npcTribes[171].partsList[7].animPoseName = "e_jyama_b";
        npcdrv::npcTribes[171].partsList[8].animPoseName = "e_jyama_b";
        npcdrv::npcTribes[171].partsList[9].animPoseName = "e_jyama_b";
        item_data::itemDataTable[388].animPoseName = "e_card_jyama_b";

        // Patch Dark Enemies from appearing as all-black on their Cards.
        s32 id = 0;
        for (id = 282; id < 537; ++id) {
            item_data::itemDataTable[id].dispFlags = 0;
        }

        /* Didn't pan out, maybe one day though.
                // Just for fun, let's make Dark Luigi actually be Luigi
                npcdrv::npcTribes[331].animPoseName = "p_luigi";
                npcdrv::npcTribes[331].animDefs = luigiAnims;
                */

        // Adjust new NPC stats
        // Spinia
        npcdrv::npcTribes[495].catchCardItemId = 344;
        item_data::itemDataTable[344].tribe = 495;
        item_data::itemDataTable[344].animPoseName = "e_hannya_g";
        item_data::itemDataTable[344].animName = "Z_1";
        npcdrv::npcTribes[495].catchCardDefense = 5;
        npcdrv::npcTribes[495].maxHp = 4;
        npcdrv::npcTribes[495].killXp = 200;
        npcdrv::npcTribes[495].coinDropChance = 100;
        npcdrv::npcTribes[495].coinDropBaseCount = 0;
        npcdrv::npcTribes[495].coinDropExtraChance = 45;
        npcdrv::npcTribes[495].coinDropExtraMax = 1;
        npcdrv::npcTribes[495].attackStrength = 1;
        npcdrv::npcTribes[495].bounceEjection = 2;
        npcdrv::npcTribes[495].jumpEjection = 0;

        // Spunia
        npcdrv::npcTribes[496].catchCardItemId = 350;
        item_data::itemDataTable[350].tribe = 496;
        item_data::itemDataTable[350].animPoseName = "e_hannya_p";
        item_data::itemDataTable[350].animName = "Z_1";
        npcdrv::npcTribes[496].catchCardDefense = 20;
        npcdrv::npcTribes[496].maxHp = 9;
        npcdrv::npcTribes[496].killXp = 600;
        npcdrv::npcTribes[496].coinDropChance = 100;
        npcdrv::npcTribes[496].coinDropBaseCount = 0;
        npcdrv::npcTribes[496].coinDropExtraChance = 45;
        npcdrv::npcTribes[496].coinDropExtraMax = 3;
        npcdrv::npcTribes[496].attackStrength = 3;
        npcdrv::npcTribes[496].bounceEjection = 2;
        npcdrv::npcTribes[496].jumpEjection = 0;

        // Shady Koopa, ATK 5 DEF 3
        npcdrv::npcTribes[465].catchCardItemId = 304;
        item_data::itemDataTable[304].tribe = 465;
        item_data::itemDataTable[304].animPoseName = "e_uranoko";
        item_data::itemDataTable[304].animName = "NKT_Z_1";
        npcdrv::npcTribes[465].catchCardDefense = 30;
        npcdrv::npcTribes[465].maxHp = 15;
        npcdrv::npcTribes[465].killXp = 300;
        npcdrv::npcTribes[465].coinDropChance = 100;
        npcdrv::npcTribes[465].coinDropBaseCount = 0;
        npcdrv::npcTribes[465].coinDropExtraChance = 75;
        npcdrv::npcTribes[465].coinDropExtraMax = 5;
        npcdrv::npcTribes[465].attackStrength = 4;
        npcdrv::npcTribes[465].bounceEjection = 1;
        npcdrv::npcTribes[465].jumpEjection = 0;

        // Flip Shady Koopa, ATK 5 DEF 3
        npcdrv::npcTribes[466].catchCardItemId = 304;
        npcdrv::npcTribes[466].catchCardDefense = 30;
        npcdrv::npcTribes[466].maxHp = 15;
        npcdrv::npcTribes[466].killXp = 300;
        npcdrv::npcTribes[466].coinDropChance = 100;
        npcdrv::npcTribes[466].coinDropBaseCount = 0;
        npcdrv::npcTribes[466].coinDropExtraChance = 75;
        npcdrv::npcTribes[466].coinDropExtraMax = 5;
        npcdrv::npcTribes[466].attackStrength = 4;
        npcdrv::npcTribes[466].bounceEjection = 1;
        npcdrv::npcTribes[466].jumpEjection = 0;

        // Spiked Gloomba, ATK 3
        npcdrv::npcTribes[5].catchCardItemId = 336;
        item_data::itemDataTable[336].tribe = 5;
        item_data::itemDataTable[336].animPoseName = "e_kuribo_y";
        item_data::itemDataTable[336].animName = "toge_kuribo_Z_2";
        npcdrv::npcTribes[5].catchCardDefense = 6;
        npcdrv::npcTribes[5].maxHp = 2;
        npcdrv::npcTribes[5].killXp = 200;
        npcdrv::npcTribes[5].coinDropChance = 100;
        npcdrv::npcTribes[5].coinDropBaseCount = 0;
        npcdrv::npcTribes[5].coinDropExtraChance = 45;
        npcdrv::npcTribes[5].coinDropExtraMax = 1;
        npcdrv::npcTribes[5].attackStrength = 3;

        // Green Magikoopa, DEF 2
        npcdrv::npcTribes[473].catchCardItemId = 353;
        item_data::itemDataTable[353].tribe = 473;
        item_data::itemDataTable[353].animPoseName = "e_kamek_g";
        item_data::itemDataTable[353].animName = "KMG_Z_1";
        npcdrv::npcTribes[473].catchCardDefense = 20;
        npcdrv::npcTribes[473].maxHp = 6;
        npcdrv::npcTribes[473].killXp = 600;
        npcdrv::npcTribes[473].coinDropChance = 100;
        npcdrv::npcTribes[473].coinDropBaseCount = 0;
        npcdrv::npcTribes[473].coinDropExtraChance = 60;
        npcdrv::npcTribes[473].coinDropExtraMax = 4;
        npcdrv::npcTribes[473].attackStrength = 3;
        npcdrv::npcTribes[473].bounceEjection = 2;
        npcdrv::npcTribes[473].jumpEjection = 0;

        // Green Broom Magikoopa, DEF 2
        npcdrv::npcTribes[474].catchCardItemId = 353;
        npcdrv::npcTribes[474].catchCardDefense = 20;
        npcdrv::npcTribes[474].maxHp = 6;
        npcdrv::npcTribes[474].killXp = 600;
        npcdrv::npcTribes[474].coinDropChance = 100;
        npcdrv::npcTribes[474].coinDropBaseCount = 0;
        npcdrv::npcTribes[474].coinDropExtraChance = 60;
        npcdrv::npcTribes[474].coinDropExtraMax = 4;
        npcdrv::npcTribes[474].attackStrength = 3;
        npcdrv::npcTribes[474].bounceEjection = 2;
        npcdrv::npcTribes[474].jumpEjection = 0;

        // Green Magi Projectile
        npcdrv::npcTribes[475].catchCardItemId = 353;
        npcdrv::npcTribes[NPC_GREEN_MAGIKOOPA_PROJ].partsList[0].defenses = npcdrv::npcTribes[NPC_MAGIKOOPA_PROJ].partsList[0].defenses;

        // White Magikoopa
        npcdrv::npcTribes[476].catchCardItemId = 352;
        item_data::itemDataTable[352].tribe = 476;
        item_data::itemDataTable[352].animPoseName = "e_kamek_w";
        item_data::itemDataTable[352].animName = "KMW_Z_1";
        npcdrv::npcTribes[476].catchCardDefense = 20;
        npcdrv::npcTribes[476].maxHp = 18;
        npcdrv::npcTribes[476].killXp = 600;
        npcdrv::npcTribes[476].coinDropChance = 100;
        npcdrv::npcTribes[476].coinDropBaseCount = 0;
        npcdrv::npcTribes[476].coinDropExtraChance = 60;
        npcdrv::npcTribes[476].coinDropExtraMax = 4;
        npcdrv::npcTribes[476].attackStrength = 3;
        npcdrv::npcTribes[476].bounceEjection = 2;
        npcdrv::npcTribes[476].jumpEjection = 0;

        // White Broom Magikoopa
        npcdrv::npcTribes[477].catchCardItemId = 352;
        npcdrv::npcTribes[477].catchCardDefense = 20;
        npcdrv::npcTribes[477].maxHp = 18;
        npcdrv::npcTribes[477].killXp = 600;
        npcdrv::npcTribes[477].coinDropChance = 100;
        npcdrv::npcTribes[477].coinDropBaseCount = 0;
        npcdrv::npcTribes[477].coinDropExtraChance = 60;
        npcdrv::npcTribes[477].coinDropExtraMax = 4;
        npcdrv::npcTribes[477].attackStrength = 3;
        npcdrv::npcTribes[477].bounceEjection = 2;
        npcdrv::npcTribes[477].jumpEjection = 0;

        // White Magi Projectile
        npcdrv::npcTribes[478].catchCardItemId = 352;
        npcdrv::npcTribes[NPC_WHITE_MAGIKOOPA_PROJ].partsList[0].defenses = npcdrv::npcTribes[NPC_MAGIKOOPA_PROJ].partsList[0].defenses;

        // Red Magikoopa
        npcdrv::npcTribes[479].catchCardItemId = 351;
        item_data::itemDataTable[351].tribe = 479;
        item_data::itemDataTable[351].animPoseName = "e_kamek_r";
        item_data::itemDataTable[351].animName = "KMR_Z_1";
        npcdrv::npcTribes[479].catchCardDefense = 20;
        npcdrv::npcTribes[479].maxHp = 6;
        npcdrv::npcTribes[479].killXp = 600;
        npcdrv::npcTribes[479].coinDropChance = 100;
        npcdrv::npcTribes[479].coinDropBaseCount = 0;
        npcdrv::npcTribes[479].coinDropExtraChance = 60;
        npcdrv::npcTribes[479].coinDropExtraMax = 4;
        npcdrv::npcTribes[479].attackStrength = 5;
        npcdrv::npcTribes[479].bounceEjection = 2;
        npcdrv::npcTribes[479].jumpEjection = 0;

        // Red Broom Magikoopa
        npcdrv::npcTribes[480].catchCardItemId = 351;
        npcdrv::npcTribes[480].catchCardDefense = 20;
        npcdrv::npcTribes[480].maxHp = 5;
        npcdrv::npcTribes[480].killXp = 600;
        npcdrv::npcTribes[480].coinDropChance = 100;
        npcdrv::npcTribes[480].coinDropBaseCount = 0;
        npcdrv::npcTribes[480].coinDropExtraChance = 60;
        npcdrv::npcTribes[480].coinDropExtraMax = 4;
        npcdrv::npcTribes[480].attackStrength = 5;
        npcdrv::npcTribes[480].bounceEjection = 2;
        npcdrv::npcTribes[480].jumpEjection = 0;

        // Red Magi Projectile, ATK 6
        npcdrv::npcTribes[481].catchCardItemId = 351;
        npcdrv::npcTribes[NPC_RED_MAGIKOOPA_PROJ].partsList[0].defenses = npcdrv::npcTribes[NPC_MAGIKOOPA_PROJ].partsList[0].defenses;

        // Bleepboxer, ATK 3
        npcdrv::npcTribes[504].catchCardItemId = 384;
        npcdrv::npcTribes[504].catchCardDefense = 15;
        npcdrv::npcTribes[504].maxHp = 12;
        npcdrv::npcTribes[504].killXp = 500;
        npcdrv::npcTribes[504].coinDropChance = 100;
        npcdrv::npcTribes[504].coinDropBaseCount = 0;
        npcdrv::npcTribes[504].coinDropExtraChance = 45;
        npcdrv::npcTribes[504].coinDropExtraMax = 3;
        npcdrv::npcTribes[504].attackStrength = 4;
        npcdrv::npcTribes[504].bounceEjection = 2;
        npcdrv::npcTribes[504].jumpEjection = 0;
        npcdrv::npcTribes[505].catchCardItemId = 384; // Projectile
        item_data::itemDataTable[384].tribe = 504;    // Vanilla card data works fine
        npcdrv::npcTribes[NPC_GREEN_BOOMBOXER_PROJ].partsList[0].defenses = npcdrv::npcTribes[NPC_BOOMBOXER_PROJ].partsList[0].defenses;

        // Kilo Muth, ATK 2
        npcdrv::npcTribes[506].catchCardItemId = 427;
        item_data::itemDataTable[427].tribe = 506; // Vanilla card data works fine
        npcdrv::npcTribes[506].catchCardDefense = 15;
        npcdrv::npcTribes[506].maxHp = 25;
        npcdrv::npcTribes[506].killXp = 700;
        npcdrv::npcTribes[506].attackStrength = 2;
        npcdrv::npcTribes[506].bounceEjection = 2;
        npcdrv::npcTribes[506].jumpEjection = 0;

        // Bawbus, ATK 3
        npcdrv::npcTribes[171].maxHp = 15;
        npcdrv::npcTribes[171].killXp = 700;
        npcdrv::npcTribes[171].attackStrength = 3;
        npcdrv::npcTribes[171].catchCardDefense = 12;

        // Mega Muth and Muth HP drop
        npcdrv::npcTribes[257].maxHp = 50;
        npcdrv::npcTribes[258].maxHp = 75;

        // Ice Bro nerf, ATK 3 DEF 2
        npcdrv::npcTribes[61].catchCardDefense = 16;
        item_data::itemDataTable[316].animPoseName = "e_burosu_i";
        item_data::itemDataTable[316].animName = "BRO_Z_1";
        npcdrv::npcTribes[61].maxHp = 10;
        npcdrv::npcTribes[61].killXp = 800;
        npcdrv::npcTribes[61].coinDropChance = 100;
        npcdrv::npcTribes[61].coinDropBaseCount = 0;
        npcdrv::npcTribes[61].coinDropExtraChance = 50;
        npcdrv::npcTribes[61].coinDropExtraMax = 5;
        npcdrv::npcTribes[61].attackStrength = 3;

        // Dark Puff nerf
        npcdrv::npcTribes[111].catchCardDefense = 8;
        npcdrv::npcTribes[111].maxHp = 6;
        npcdrv::npcTribes[111].killXp = 300;
        npcdrv::npcTribes[111].coinDropChance = 100;
        npcdrv::npcTribes[111].coinDropBaseCount = 0;
        npcdrv::npcTribes[111].coinDropExtraChance = 40;
        npcdrv::npcTribes[111].coinDropExtraMax = 2;
        npcdrv::npcTribes[111].attackStrength = 2;

        // Flip Buzzy Beetle/Flip Spike Top patch
        npcdrv::npcTribes[470].catchCardItemId = 298;
        npcdrv::npcTribes[470].catchCardDefense = 2;
        npcdrv::npcTribes[470].maxHp = 1;
        npcdrv::npcTribes[470].killXp = 200;
        npcdrv::npcTribes[470].coinDropChance = 100;
        npcdrv::npcTribes[470].coinDropBaseCount = 0;
        npcdrv::npcTribes[470].coinDropExtraChance = 45;
        npcdrv::npcTribes[470].coinDropExtraMax = 2;
        npcdrv::npcTribes[470].attackStrength = 1;
        npcdrv::npcTribes[470].bounceEjection = 2;
        npcdrv::npcTribes[470].jumpEjection = 0;

        npcdrv::npcTribes[471].catchCardItemId = 299;
        npcdrv::npcTribes[471].catchCardDefense = 10;
        npcdrv::npcTribes[471].maxHp = 1;
        npcdrv::npcTribes[471].killXp = 300;
        npcdrv::npcTribes[471].coinDropChance = 100;
        npcdrv::npcTribes[471].coinDropBaseCount = 0;
        npcdrv::npcTribes[471].coinDropExtraChance = 45;
        npcdrv::npcTribes[471].coinDropExtraMax = 2;
        npcdrv::npcTribes[471].attackStrength = 2;
        npcdrv::npcTribes[471].bounceEjection = 2;
        npcdrv::npcTribes[471].jumpEjection = 0;

        // Green Fuzzy nerf
        npcdrv::npcTribes[98].catchCardDefense = 16;
        item_data::itemDataTable[333].animPoseName = "e_chorobon_g";
        item_data::itemDataTable[333].animName = "CBN_Z_1";
        npcdrv::npcTribes[98].maxHp = 10;
        npcdrv::npcTribes[98].killXp = 600;
        npcdrv::npcTribes[98].coinDropChance = 100;
        npcdrv::npcTribes[98].coinDropBaseCount = 0;
        npcdrv::npcTribes[98].coinDropExtraChance = 40;
        npcdrv::npcTribes[98].coinDropExtraMax = 2;
        npcdrv::npcTribes[98].attackStrength = 2;

        // Tileoid PU stats
        npcdrv::npcTribes[167].catchCardDefense = 12;
        item_data::itemDataTable[402].animPoseName = "e_ntl_p";
        item_data::itemDataTable[402].animName = "Z_1";
        npcdrv::npcTribes[167].maxHp = 1;
        npcdrv::npcTribes[167].killXp = 600;
        npcdrv::npcTribes[167].coinDropChance = 100;
        npcdrv::npcTribes[167].coinDropBaseCount = 0;
        npcdrv::npcTribes[167].coinDropExtraChance = 40;
        npcdrv::npcTribes[167].coinDropExtraMax = 6;
        npcdrv::npcTribes[167].attackStrength = 5;
        npcdrv::npcTribes[167].bounceEjection = 10;
        npcdrv::npcTribes[167].jumpEjection = 10;
        // We're actually gonna nerf the other Tileoids' ejection strengths, too!
        npcdrv::npcTribes[163].bounceEjection = 1.0;
        npcdrv::npcTribes[164].bounceEjection = 1.0;
        npcdrv::npcTribes[165].bounceEjection = 1.0;
        npcdrv::npcTribes[166].bounceEjection = 1.0;
        npcdrv::npcTribes[163].jumpEjection = 1.0;
        npcdrv::npcTribes[164].jumpEjection = 1.0;
        npcdrv::npcTribes[165].jumpEjection = 1.0;
        npcdrv::npcTribes[166].jumpEjection = 1.0;

        // White Clubba nerf
        npcdrv::npcTribes[95].maxHp = 30;
        npcdrv::npcTribes[95].attackStrength = 4;
        item_data::itemDataTable[324].animPoseName = "e_gabow";
        item_data::itemDataTable[324].animName = "gabon_Z_1";

        // Bomb Boo
        item_data::itemDataTable[356].animPoseName = "e_teresa_b";

        // Frackle point buff
        npcdrv::npcTribes[450].killXp = 10;

        // (Spiky) Skellobyte nerfs
        npcdrv::npcTribes[225].maxHp = 20;
        npcdrv::npcTribes[228].maxHp = 20;
        item_data::itemDataTable[438].animPoseName = "e_tesita_bt";
        item_data::itemDataTable[438].animName = "Z_1";
        item_data::itemDataTable[439].animPoseName = "e_tesita_sb";
        item_data::itemDataTable[439].animName = "Z_1";

        // Patch remaining enemies' card textures
        // Red Spike Top
        npcdrv::npcTribes[29].catchCardItemId = 300;
        npcdrv::npcTribes[29].killXp = 400;
        item_data::itemDataTable[300].tribe = 29;
        item_data::itemDataTable[300].animPoseName = "e_mer";
        item_data::itemDataTable[300].animName = "TMT_Z_1";
        // Hyper Goomba
        item_data::itemDataTable[290].tribe = 2;
        item_data::itemDataTable[290].animPoseName = "e_kuribo_h";
        item_data::itemDataTable[290].animName = "kuribo_Z_1";
        // Hyper Paragoomba
        item_data::itemDataTable[286].tribe = 8;
        item_data::itemDataTable[286].animPoseName = "e_kuribo_h";
        item_data::itemDataTable[286].animName = "pata_kuribo_Z_2";
        // Hyper Spiked Goomba
        item_data::itemDataTable[284].tribe = 6;
        item_data::itemDataTable[284].animPoseName = "e_kuribo_h";
        item_data::itemDataTable[284].animName = "toge_kuribo_Z_2";
        // Sky-Blue Spiny
        item_data::itemDataTable[306].tribe = 38;
        item_data::itemDataTable[306].animPoseName = "e_togezb";
        item_data::itemDataTable[306].animName = "TGZ_Z_1";
        // Dark Koopa
        item_data::itemDataTable[294].tribe = 16;
        item_data::itemDataTable[294].animPoseName = "e_nokoteki_d";
        item_data::itemDataTable[294].animName = "NKT_Z_1";
        // Dark Paratroopa
        item_data::itemDataTable[297].tribe = 24;
        item_data::itemDataTable[297].animPoseName = "e_nokoteki_d";
        item_data::itemDataTable[297].animName = "PTP_Z_1";
        // Dark Paratroopa
        item_data::itemDataTable[297].tribe = 24;
        item_data::itemDataTable[297].animPoseName = "e_nokoteki_d";
        item_data::itemDataTable[297].animName = "PTP_Z_1";
        // Ash Cherbil
        item_data::itemDataTable[535].tribe = 446;
        item_data::itemDataTable[535].animPoseName = "e_cheririn_a";
        item_data::itemDataTable[535].animName = "Z_1";
        // Dark Bones
        item_data::itemDataTable[309].tribe = 43;
        item_data::itemDataTable[309].animPoseName = "e_karon_d";
        item_data::itemDataTable[309].animName = "KRN_Z_1";
        // Shady Magikoopa
        item_data::itemDataTable[318].tribe = 66;
        item_data::itemDataTable[318].animPoseName = "e_kames";
        item_data::itemDataTable[318].animName = "KMK_Z_1";
        // Shady Striker
        item_data::itemDataTable[322].tribe = 75;
        item_data::itemDataTable[322].animPoseName = "e_k_shoote4";
        item_data::itemDataTable[322].animName = "Z_1";
        // Ninjeremiah
        item_data::itemDataTable[434].tribe = 220;
        item_data::itemDataTable[434].animPoseName = "e_nin_d";
        item_data::itemDataTable[434].animName = "Z_1";
        // Dark Koopatrol
        item_data::itemDataTable[293].tribe = 19;
        item_data::itemDataTable[293].animPoseName = "e_togenokd";
        item_data::itemDataTable[293].animName = "Z_1";
        npcdrv::npcTribes[NPC_DARK_KOOPATROL].killXp = 900;
        // Kamikaze Goomba
        item_data::itemDataTable[289].tribe = 10;
        item_data::itemDataTable[289].animPoseName = "e_k_kuribo";
        item_data::itemDataTable[289].animName = "z_kuribo_Z_1";
        npcdrv::npcTribes[NPC_DARK_HEADBONK_GOOMBA].killXp = 800;
        // Shady Hammer Bro
        item_data::itemDataTable[314].tribe = 49;
        item_data::itemDataTable[314].animPoseName = "e_buross_h";
        item_data::itemDataTable[314].animName = "Z_1";
        // Shady Boomerang Bro
        item_data::itemDataTable[315].tribe = 55;
        item_data::itemDataTable[315].animPoseName = "e_buross_b";
        item_data::itemDataTable[315].animName = "Z_1";
        // Hyper Cleft
        item_data::itemDataTable[364].tribe = 101;
        item_data::itemDataTable[364].animPoseName = "e_sinemoh";
        item_data::itemDataTable[364].animName = "SIN_Z_1";
        // Bullet William Blaster
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL_BLASTER].partsList = npcdrv::npcTribes[NPC_BILL_BLASTER].partsList;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL_BLASTER].maxHp = 30;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL_BLASTER].height = 0x1e;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL_BLASTER].width = 0x32;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL_BLASTER].length = 0x14;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL_BLASTER].attackStrength = 4;
        npcdrv::npcEnemyTemplates[109].atkScript = npcdrv::npcEnemyTemplates[107].atkScript;
        item_data::itemDataTable[416].tribe = NPC_BOMBSHELL_BILL_BLASTER;
        item_data::itemDataTable[416].animPoseName = "e_killtai_g";
        item_data::itemDataTable[416].animName = "KTA_Z_1";
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL_BLASTER].catchCardItemId = 416;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL_BLASTER].killXp = 1500;

        // Bullet William
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL].partsList = npcdrv::npcTribes[NPC_BULLET_BILL].partsList;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL].coinDropExtraMax = 0;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL].height = 0x14;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL].width = 0x14;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL].length = 0x14;
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL].attackStrength = 6;
        npcdrv::npcEnemyTemplates[108].onSpawnScript = npcdrv::npcEnemyTemplates[106].onSpawnScript;
        npcdrv::npcEnemyTemplates[108].deathScript = npcdrv::npcEnemyTemplates[106].deathScript;
        item_data::itemDataTable[419].tribe = NPC_BOMBSHELL_BILL;
        item_data::itemDataTable[419].animPoseName = "e_kilr_g";
        item_data::itemDataTable[419].animName = "KIL_Z_1";
        npcdrv::npcTribes[NPC_BOMBSHELL_BILL].catchCardItemId = 419;

        // Create Dark Lakitu (Template 30, Tribe 534)
        npcdrv::npcEnemyTemplates[30].tribeId = 534;
        npcdrv::npcEnemyTemplates[30].unknown_0x0 = 1;
        npcdrv::npcEnemyTemplates[30].unknown_0x1 = 1;
        npcdrv::npcEnemyTemplates[30].unknown_0x2 = 0;
        npcdrv::npcEnemyTemplates[30].unknown_0x3 = 0;
        npcdrv::npcEnemyTemplates[30].unknown_0x8 = 0;
        npcdrv::npcEnemyTemplates[30].flags = 0x210;
        npcdrv::npcEnemyTemplates[30].onSpawnScript = npcdrv::npcEnemyTemplates[61].onSpawnScript;
        npcdrv::npcEnemyTemplates[30].initScript = 0;
        npcdrv::npcEnemyTemplates[30].moveScript = npcdrv::npcEnemyTemplates[61].moveScript;
        npcdrv::npcEnemyTemplates[30].onHitScript = npcdrv::npcEnemyTemplates[61].onHitScript;
        npcdrv::npcEnemyTemplates[30].pickupScript = npcdrv::npcEnemyTemplates[61].pickupScript;
        npcdrv::npcEnemyTemplates[30].throwScript = npcdrv::npcEnemyTemplates[61].throwScript;
        npcdrv::npcEnemyTemplates[30].deathScript = npcdrv::npcEnemyTemplates[61].deathScript;
        npcdrv::npcEnemyTemplates[30].atkScript = npcdrv::npcEnemyTemplates[61].atkScript;
        npcdrv::npcEnemyTemplates[30].miscScript = npcdrv::npcEnemyTemplates[61].miscScript;
        npcdrv::npcEnemyTemplates[30].kouraKickScript = npcdrv::npcEnemyTemplates[61].kouraKickScript;
        npcdrv::npcEnemyTemplates[30].unkDefinitionTable = npcdrv::npcEnemyTemplates[61].unkDefinitionTable;
        npcdrv::npcTribes[534].catchCardItemId = 381;
        item_data::itemDataTable[381].tribe = 534;
        item_data::itemDataTable[381].animPoseName = "e_jugemu_d";
        item_data::itemDataTable[381].animName = "JGM_Z_1";
        npcdrv::npcTribes[534].catchCardDefense = 20;
        npcdrv::npcTribes[534].maxHp = 13;
        npcdrv::npcTribes[534].killXp = 900;
        npcdrv::npcTribes[534].coinDropChance = 100;
        npcdrv::npcTribes[534].coinDropBaseCount = 0;
        npcdrv::npcTribes[534].coinDropExtraChance = 68;
        npcdrv::npcTribes[534].coinDropExtraMax = 6;
        npcdrv::npcTribes[534].attackStrength = 3;
        npcdrv::npcTribes[534].animPoseName = "e_jugemu_d";
        npcdrv::npcTribes[534].animDefs = npcdrv::npcTribes[34].animDefs;
        npcdrv::npcTribes[534].hitXp = 0;
        /*
                // Create KP Koopa (Template 7, Tribe 533)
                npcdrv::npcEnemyTemplates[7].tribeId = 533;
                npcdrv::npcEnemyTemplates[7].unknown_0x0 = 1;
                npcdrv::npcEnemyTemplates[7].unknown_0x1 = 1;
                npcdrv::npcEnemyTemplates[7].unknown_0x2 = 0;
                npcdrv::npcEnemyTemplates[7].unknown_0x3 = 0;
                npcdrv::npcEnemyTemplates[7].unknown_0x8 = 0;
                npcdrv::npcEnemyTemplates[7].flags = 0x18;
                npcdrv::npcEnemyTemplates[7].onSpawnScript = npcdrv::npcEnemyTemplates[9].onSpawnScript;
                npcdrv::npcEnemyTemplates[7].initScript = npcdrv::npcEnemyTemplates[9].initScript;
                npcdrv::npcEnemyTemplates[7].moveScript = npcdrv::npcEnemyTemplates[9].moveScript;
                npcdrv::npcEnemyTemplates[7].onHitScript = npcdrv::npcEnemyTemplates[9].onHitScript;
                npcdrv::npcEnemyTemplates[7].pickupScript = npcdrv::npcEnemyTemplates[9].pickupScript;
                npcdrv::npcEnemyTemplates[7].throwScript = npcdrv::npcEnemyTemplates[9].throwScript;
                npcdrv::npcEnemyTemplates[7].deathScript = npcdrv::npcEnemyTemplates[9].deathScript;
                npcdrv::npcEnemyTemplates[7].atkScript = 0;
                npcdrv::npcEnemyTemplates[7].miscScript = 0;
                npcdrv::npcEnemyTemplates[7].kouraKickScript = npcdrv::npcEnemyTemplates[9].kouraKickScript;
                npcdrv::npcEnemyTemplates[7].unkDefinitionTable = npcdrv::npcEnemyTemplates[9].unkDefinitionTable;
                npcdrv::npcTribes[533].catchCardItemId = 291;
                npcdrv::npcTribes[533].catchCardDefense = 8;
                npcdrv::npcTribes[533].maxHp = 5;
                npcdrv::npcTribes[533].killXp = 200;
                npcdrv::npcTribes[533].coinDropChance = 100;
                npcdrv::npcTribes[533].coinDropBaseCount = 0;
                npcdrv::npcTribes[533].coinDropExtraChance = 45;
                npcdrv::npcTribes[533].coinDropExtraMax = 3;
                npcdrv::npcTribes[533].attackStrength = 2;
                npcdrv::npcTribes[533].partsCount = 2;
                npcdrv::npcTribes[533].partsList[0] = npcdrv::npcTribes[14].partsList[0];
                npcdrv::npcTribes[533].partsList[1] = npcdrv::npcTribes[14].partsList[1];
                npcdrv::npcTribes[533].animPoseName = "e_touginoko";
                npcdrv::npcTribes[533].animDefs = npcdrv::npcTribes[14].animDefs;
                npcdrv::npcTribes[533].hitXp = 0;
        */
        // Create Red Shy Guy (Template 32, Tribe 532)
        npcdrv::npcEnemyTemplates[32].tribeId = 532;
        npcdrv::npcEnemyTemplates[32].unknown_0x0 = 1;
        npcdrv::npcEnemyTemplates[32].unknown_0x1 = 1;
        npcdrv::npcEnemyTemplates[32].unknown_0x2 = 0;
        npcdrv::npcEnemyTemplates[32].unknown_0x3 = 0;
        npcdrv::npcEnemyTemplates[32].unknown_0x8 = 0;
        npcdrv::npcEnemyTemplates[32].flags = 0x18;
        npcdrv::npcEnemyTemplates[32].onSpawnScript = npcdrv::npcEnemyTemplates[2].onSpawnScript;
        npcdrv::npcEnemyTemplates[32].initScript = npcdrv::npcEnemyTemplates[2].initScript;
        npcdrv::npcEnemyTemplates[32].moveScript = npcdrv::npcEnemyTemplates[2].moveScript;
        npcdrv::npcEnemyTemplates[32].onHitScript = npcdrv::npcEnemyTemplates[2].onHitScript;
        npcdrv::npcEnemyTemplates[32].pickupScript = npcdrv::npcEnemyTemplates[2].pickupScript;
        npcdrv::npcEnemyTemplates[32].throwScript = npcdrv::npcEnemyTemplates[2].throwScript;
        npcdrv::npcEnemyTemplates[32].deathScript = npcdrv::npcEnemyTemplates[2].deathScript;
        npcdrv::npcEnemyTemplates[32].atkScript = 0;
        npcdrv::npcEnemyTemplates[32].miscScript = 0;
        npcdrv::npcEnemyTemplates[32].kouraKickScript = 0;
        npcdrv::npcEnemyTemplates[32].unkDefinitionTable = npcdrv::npcEnemyTemplates[2].unkDefinitionTable;
        npcdrv::npcTribes[532].catchCardItemId = 330;
        item_data::itemDataTable[330].tribe = 532;
        item_data::itemDataTable[330].animPoseName = "e_heiho";
        item_data::itemDataTable[330].animName = "heiho_Z_1";
        npcdrv::npcTribes[532].catchCardDefense = 3;
        npcdrv::npcTribes[532].maxHp = 5;
        npcdrv::npcTribes[532].killXp = 200;
        npcdrv::npcTribes[532].coinDropChance = 100;
        npcdrv::npcTribes[532].coinDropBaseCount = 0;
        npcdrv::npcTribes[532].coinDropExtraChance = 45;
        npcdrv::npcTribes[532].coinDropExtraMax = 3;
        npcdrv::npcTribes[532].attackStrength = 2;
        npcdrv::npcTribes[532].animPoseName = "e_heiho";
        npcdrv::npcTribes[532].animDefs = heihoAnims;
        npcdrv::npcTribes[532].hitXp = 0;

        // Create Blue Shy Guy (Template 33, Tribe 531)
        npcdrv::npcEnemyTemplates[33].tribeId = 531;
        npcdrv::npcEnemyTemplates[33].unknown_0x0 = 1;
        npcdrv::npcEnemyTemplates[33].unknown_0x1 = 1;
        npcdrv::npcEnemyTemplates[33].unknown_0x2 = 0;
        npcdrv::npcEnemyTemplates[33].unknown_0x3 = 0;
        npcdrv::npcEnemyTemplates[33].unknown_0x8 = 0;
        npcdrv::npcEnemyTemplates[33].flags = 0x18;
        npcdrv::npcEnemyTemplates[33].onSpawnScript = npcdrv::npcEnemyTemplates[2].onSpawnScript;
        npcdrv::npcEnemyTemplates[33].initScript = npcdrv::npcEnemyTemplates[2].initScript;
        npcdrv::npcEnemyTemplates[33].moveScript = npcdrv::npcEnemyTemplates[2].moveScript;
        npcdrv::npcEnemyTemplates[33].onHitScript = npcdrv::npcEnemyTemplates[2].onHitScript;
        npcdrv::npcEnemyTemplates[33].pickupScript = npcdrv::npcEnemyTemplates[2].pickupScript;
        npcdrv::npcEnemyTemplates[33].throwScript = npcdrv::npcEnemyTemplates[2].throwScript;
        npcdrv::npcEnemyTemplates[33].deathScript = npcdrv::npcEnemyTemplates[2].deathScript;
        npcdrv::npcEnemyTemplates[33].atkScript = 0;
        npcdrv::npcEnemyTemplates[33].miscScript = 0;
        npcdrv::npcEnemyTemplates[33].kouraKickScript = 0;
        npcdrv::npcEnemyTemplates[33].unkDefinitionTable = npcdrv::npcEnemyTemplates[2].unkDefinitionTable;
        npcdrv::npcTribes[531].catchCardItemId = 330;
        npcdrv::npcTribes[531].catchCardDefense = 5;
        npcdrv::npcTribes[531].maxHp = 5;
        npcdrv::npcTribes[531].killXp = 300;
        npcdrv::npcTribes[531].coinDropChance = 100;
        npcdrv::npcTribes[531].coinDropBaseCount = 0;
        npcdrv::npcTribes[531].coinDropExtraChance = 45;
        npcdrv::npcTribes[531].coinDropExtraMax = 3;
        npcdrv::npcTribes[531].attackStrength = 2;
        npcdrv::npcTribes[531].animPoseName = "e_beiho";
        npcdrv::npcTribes[531].animDefs = heihoAnims;
        npcdrv::npcTribes[531].hitXp = 0;
        npcdrv::npcTribes[531].bounceEjection = 1;
        npcdrv::npcTribes[531].jumpEjection = 0;
        npcdrv::npcTribes[531].powBlockDeathSfx = "SFX_BS_DDN_CRASH_LANDING1";

        // Create Green Shy Guy (Template 34, Tribe 530)
        npcdrv::npcEnemyTemplates[34].tribeId = 530;
        npcdrv::npcEnemyTemplates[34].unknown_0x0 = 1;
        npcdrv::npcEnemyTemplates[34].unknown_0x1 = 1;
        npcdrv::npcEnemyTemplates[34].unknown_0x2 = 0;
        npcdrv::npcEnemyTemplates[34].unknown_0x3 = 0;
        npcdrv::npcEnemyTemplates[34].unknown_0x8 = 0;
        npcdrv::npcEnemyTemplates[34].flags = 0x18;
        npcdrv::npcEnemyTemplates[34].onSpawnScript = npcdrv::npcEnemyTemplates[2].onSpawnScript;
        npcdrv::npcEnemyTemplates[34].initScript = npcdrv::npcEnemyTemplates[2].initScript;
        npcdrv::npcEnemyTemplates[34].moveScript = npcdrv::npcEnemyTemplates[2].moveScript;
        npcdrv::npcEnemyTemplates[34].onHitScript = npcdrv::npcEnemyTemplates[2].onHitScript;
        npcdrv::npcEnemyTemplates[34].pickupScript = npcdrv::npcEnemyTemplates[2].pickupScript;
        npcdrv::npcEnemyTemplates[34].throwScript = npcdrv::npcEnemyTemplates[2].throwScript;
        npcdrv::npcEnemyTemplates[34].deathScript = npcdrv::npcEnemyTemplates[2].deathScript;
        npcdrv::npcEnemyTemplates[34].atkScript = 0;
        npcdrv::npcEnemyTemplates[34].miscScript = 0;
        npcdrv::npcEnemyTemplates[34].kouraKickScript = 0;
        npcdrv::npcEnemyTemplates[34].unkDefinitionTable = npcdrv::npcEnemyTemplates[2].unkDefinitionTable;
        npcdrv::npcTribes[530].catchCardItemId = 330;
        npcdrv::npcTribes[530].catchCardDefense = 5;
        npcdrv::npcTribes[530].maxHp = 5;
        npcdrv::npcTribes[530].killXp = 300;
        npcdrv::npcTribes[530].coinDropChance = 100;
        npcdrv::npcTribes[530].coinDropBaseCount = 0;
        npcdrv::npcTribes[530].coinDropExtraChance = 45;
        npcdrv::npcTribes[530].coinDropExtraMax = 3;
        npcdrv::npcTribes[530].attackStrength = 2;
        npcdrv::npcTribes[530].animPoseName = "e_geiho";
        npcdrv::npcTribes[530].animDefs = heihoAnims;
        npcdrv::npcTribes[530].hitXp = 0;
        npcdrv::npcTribes[530].bounceEjection = 1;
        npcdrv::npcTribes[530].jumpEjection = 0;

        // Create Yellow Shy Guy (Template 35, Tribe 529)
        npcdrv::npcEnemyTemplates[35].tribeId = 529;
        npcdrv::npcEnemyTemplates[35].unknown_0x0 = 1;
        npcdrv::npcEnemyTemplates[35].unknown_0x1 = 1;
        npcdrv::npcEnemyTemplates[35].unknown_0x2 = 0;
        npcdrv::npcEnemyTemplates[35].unknown_0x3 = 0;
        npcdrv::npcEnemyTemplates[35].unknown_0x8 = 0;
        npcdrv::npcEnemyTemplates[35].flags = 0x18;
        npcdrv::npcEnemyTemplates[35].onSpawnScript = npcdrv::npcEnemyTemplates[2].onSpawnScript;
        npcdrv::npcEnemyTemplates[35].initScript = npcdrv::npcEnemyTemplates[2].initScript;
        npcdrv::npcEnemyTemplates[35].moveScript = npcdrv::npcEnemyTemplates[2].moveScript;
        npcdrv::npcEnemyTemplates[35].onHitScript = npcdrv::npcEnemyTemplates[2].onHitScript;
        npcdrv::npcEnemyTemplates[35].pickupScript = npcdrv::npcEnemyTemplates[2].pickupScript;
        npcdrv::npcEnemyTemplates[35].throwScript = npcdrv::npcEnemyTemplates[2].throwScript;
        npcdrv::npcEnemyTemplates[35].deathScript = npcdrv::npcEnemyTemplates[2].deathScript;
        npcdrv::npcEnemyTemplates[35].atkScript = 0;
        npcdrv::npcEnemyTemplates[35].miscScript = 0;
        npcdrv::npcEnemyTemplates[35].kouraKickScript = 0;
        npcdrv::npcEnemyTemplates[35].unkDefinitionTable = 0;
        npcdrv::npcTribes[529].catchCardItemId = 330;
        npcdrv::npcTribes[529].catchCardDefense = 8;
        npcdrv::npcTribes[529].maxHp = 12;
        npcdrv::npcTribes[529].killXp = 300;
        npcdrv::npcTribes[529].coinDropChance = 100;
        npcdrv::npcTribes[529].coinDropBaseCount = 0;
        npcdrv::npcTribes[529].coinDropExtraChance = 45;
        npcdrv::npcTribes[529].coinDropExtraMax = 3;
        npcdrv::npcTribes[529].attackStrength = 2;
        npcdrv::npcTribes[529].animPoseName = "e_yeiho";
        npcdrv::npcTribes[529].animDefs = heihoAnims;
        npcdrv::npcTribes[529].hitXp = 0;
        npcdrv::npcTribes[529].bounceEjection = 1;
        npcdrv::npcTribes[529].jumpEjection = 0;
        npcdrv::NPCDefense testDefs[5] = {def1_1, def7_1, def0_1, def48, defterm};
        npcdrv::npcTribes[529].partsList[0].defenses = testDefs;
        //    OSREPORTF("def0_1: %d.\n", def0_1.defense);
        //    OSREPORTF("defterm: %d.\n", defterm.defense);

        // Force all enemies' stylish XP to be lower
        for (s32 i = 0; i < NPCTRIBE_MAX; i += 1)
            npcdrv::npcTribes[i].stylishXp = (npcdrv::npcTribes[i].killXp / 15);

        // Let's replace the pit key texture while we're at it lol
        // Ty kiki!!! <3
        item_data::itemDataTable[48].iconId = TPLPATCH_ICON(ICON_SKULL_KEY);

        // Change some names for the blue whacka bump
        item_data::itemDataTable[45].nameMsg = mystBumpNamePtr;
        item_data::itemDataTable[45].descMsg = mystBumpDescPtr;
    }

    s32 npc_killtai_spawn_child(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        npcdrv::MiscSetupDataV6 setupData;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 uw2 = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        npcdrv::NPCEntry * owner = (npcdrv::NPCEntry *)evtEntry->ownerNPC;
        npcdrv::NPCEntry * child = nullptr;
        if (firstCall) {
            wii::mtx::Vec3 pos = {owner->position.x, owner->position.y + 10.0f, owner->position.z};
            f32 dir_adj = (f32)(((s32)owner->axisMovementUnit << 5) * 1.5f); // -48, 48
            if (owner->flippedTo3d == 0) {
                pos.x += dir_adj;
                pos.z += 1.0f;
            } else {
                pos.x -= 1.0f;
                pos.z += dir_adj;
            }
            msl::string::memset(&setupData, 0, sizeof(MiscSetupDataV6));
            setupData.unknown_0x04 = 4000;
            setupData.unknown_0x08 = 4000;
            setupData.unknown_0x50 = owner->axisMovementUnit;
            setupData.unknown_0x54 = owner->flippedTo3d;
            setupData.unknown_0x58 = 4;
            setupData.gravityRotation = owner->gravRotation;
            s32 templateId = 106;
            if (owner->tribeId == NPC_BOMBSHELL_BILL_BLASTER)
                templateId = 108;
            child = npcdrv::npcEntryFromSetupEnemy(0, &pos, templateId, &setupData);
            evtEntry->tempU[0] = (u32)child;
            owner->unitWork[4] = child->id;
            child->unitWork[0] = 180; // speed
            if (child->tribeId == NPC_BOMBSHELL_BILL)
                child->unitWork[0] = 300;
            child->unitWork[1] = owner->axisMovementUnit * 1000;
            child->unitWork[2] = uw2;
            evtEntry->tempU[1] = child->onSpawnEvtId;
        }
        child = (npcdrv::NPCEntry *)evtEntry->tempU[0];
        if (evtEntry->tempU[1] != 0) {
            if (evtmgr::evtCheckID(evtEntry->tempU[1]) == false)
                return 0;
            evtEntry->tempU[1] = 0;
        }
        if (evtEntry->tempU[1] == 0) {
            const evtmgr::EvtScriptCode * moveScript = npcdrv::npcGetScript(child, 1);
            evtmgr::EvtEntry * moveEvt = evtmgr::evtEntry(moveScript, 0, 0x20);
            moveEvt->ownerNPC = (void *)child;
            child->unkEvtId = moveEvt->id;
            child->unknown_0x428 = 0;
            child->unknown_0x42c = 0;
            // Hardcode the call for inheriting neg/pos here since we can't hookFunction twice
            npcInheritDanFlag(owner, child);
            return 2;
        }
        return 0;
    }

    s32 patch_tileoid_pu(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        npcdrv::NPCEntry * npc = (npcdrv::NPCEntry *)evtEntry->ownerNPC;
        if (npc->tribeId == 167) {
            npc_tile::TileoidWork * tileWrk = (npc_tile::TileoidWork *)npc->unitWork[0];
            tileWrk->rotationSpeedScale = 3.5;
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(patch_tileoid_pu, 0)

    s32 setHitFlags(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 flagVal = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        npcdrv::NPCEntry * ownerNpc = (npcdrv::NPCEntry *)evtEntry->ownerNPC;
        ownerNpc->parts[0].hitFlags = flagVal;
        ownerNpc->parts[1].hitFlags = flagVal;
        ownerNpc->parts[2].hitFlags = flagVal;
        ownerNpc->parts[3].hitFlags = flagVal;
        ownerNpc->parts[4].hitFlags = flagVal;
        ownerNpc->parts[5].hitFlags = flagVal;
        ownerNpc->parts[6].hitFlags = flagVal;
        ownerNpc->parts[7].hitFlags = flagVal;
        ownerNpc->parts[8].hitFlags = flagVal;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(setHitFlags, 1)

    s32 setFlag2c(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 partId = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 flagVal = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        npcdrv::NPCEntry * ownerNpc = (npcdrv::NPCEntry *)evtEntry->ownerNPC;
        ownerNpc->parts[partId].flag2c = flagVal;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(setFlag2c, 2)

    s32 setHitboxScale(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 partId = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        f32 xVal = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        f32 yVal = evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        f32 zVal = evtmgr_cmd::evtGetValue(evtEntry, args[3]);
        npcdrv::NPCEntry * ownerNpc = (npcdrv::NPCEntry *)evtEntry->ownerNPC;
        ownerNpc->parts[partId].hitboxScale.x = xVal;
        ownerNpc->parts[partId].hitboxScale.y = yVal;
        ownerNpc->parts[partId].hitboxScale.z = zVal;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(setHitboxScale, 4)

    s32 declare_shadoo_stats(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        npcdrv::NPCEntry * npc = npcdrv::npcNameToPtr_NoAssert((const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]));
        // Shadoo HP = Mario max HP at minimum.
        if (lpGetDifficulty() == 0) {
            npcdrv::npcTribes[npc->tribeId].attackStrength = 5;
        } else {
            npcdrv::npcTribes[npc->tribeId].attackStrength = 10;
        }
        npc->maxHp = pouch->maxHp * (lpGetDifficulty() + 1);
        npc->hp = npc->maxHp;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(declare_shadoo_stats, 1)

    s32 get_mover_rng(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], Lunatic->Mover.moverRNG);
        return 2;
    }

    s32 set_mover_rng(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        Lunatic->Mover.moverRNG = system::rand() % 1000;
        s32 floor = swdrv::swByteGet(1);
        bool blockMovers = swdrv::swGet(1610);
        if (blockMovers || floor < 19)
            Lunatic->Mover.moverRNG = clamp(Lunatic->Mover.moverRNG + 15, 15, 1000);
        if ((floor >= 43 && floor <= 148) || floor > 194 || Lunatic->Luna.disorder > DISORDER_NULL)
            Lunatic->Mover.moverRNG = 999;
        if (DebugMode && (wpadmgr::wpadGetButtonsHeld(0) & (WPAD_BTN_1 | WPAD_BTN_2)) == (WPAD_BTN_1 | WPAD_BTN_2))
            Lunatic->Mover.moverRNG = 2;
        // THRESHOLD IS 14!!!!
        OSREPORTF("moverRNG: %d.\n", Lunatic->Mover.moverRNG);
        return 2;
    }

    s32 evt_dan_init_lunatic(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        LPDifficulty difficulty = Lunatic->Misc.difficulty;
        msl::string::memset(Lunatic, 0, sizeof(LunaticPitWork));
        Lunatic->Misc.difficulty = difficulty;
        Lunatic->Stats.CritRate = 4;
        Lunatic->Stats.CritMult = 50.0f;
        if (swdrv::swGet(1654) == true)
            lpAddCrit(12, 0);
        if (swdrv::swGet(1655) == true)
            lpAddCrit(0, 100.0f);
        if (swdrv::swGet(1656) == true)
            Lunatic->Stats.AegisDef += 1;
        if (swdrv::swGet(1657) == true)
            Lunatic->Stats.AuspiceDR += 25.0f;
        if (swdrv::swGet(1658) == true)
            lpAddHp(30, 0);
        if (swdrv::swGet(1659) == true)
            lpAddAtk(2);
        Lunatic->Misc.savedCoins = mario_pouch::pouchGetCoin();
        if (!DebugMode) {
            mario_pouch::pouchSetCoin(0);
            // If Movers are blocked & B, +, and - are all held upon entering Pit, enter Blue% Speedrun Mode
            if (swdrv::swGet(1610) && (wpadmgr::wpadGetButtonsHeld(0) & (WPAD_BTN_B | WPAD_BTN_PLUS | WPAD_BTN_MINUS)) == (WPAD_BTN_B | WPAD_BTN_PLUS | WPAD_BTN_MINUS))
                swdrv::swSet(1644);
        } else {
            mario_pouch::pouchSetCoin(999);
            Lunatic->RFC.chestKeysOwned = 69;
        }
        item_data::itemDataTable[ITEM_ID_KEY_MAC_KEY_00].iconId = TPLPATCH_ICON(ICON_CHEST_KEY);
        item_data::itemDataTable[ITEM_ID_KEY_MAC_KEY_00].nameMsg = chestKeyNamePtr;
        item_data::itemDataTable[ITEM_ID_KEY_MAC_KEY_00].descMsg = chestKeyDescPtr;
        return 2;
    }

    s32 evt_dan_reset_lunatic(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        danResetLunatic();
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_dan_reset_lunatic, 0)

    s32 rand100(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        s32 rand100Num = system::rand() % 100;
        OSREPORTF("rand100Num: %d.\n", rand100Num);
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], rand100Num);
        return 2;
    }

    s32 osReportFloat(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        // arg1 is like PTR("Variable name:"), arg2 is actual value to pass through
        // You can also just use one variable if you only want to pass a ptr string or float on its own
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * introStr = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        f32 printFloat = evtmgr_cmd::evtGetFloat(evtEntry, args[1]);
        if (introStr == nullptr || introStr == 0) {
            OSREPORTF("%f\n", printFloat);
        } else {
            OSREPORTF("%s: %f\n", introStr, printFloat);
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(osReportFloat, 2)

    s32 osReportInt(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * introStr = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 printInt = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        if (introStr == nullptr) {
            OSREPORTF("%d\n", printInt);
        } else {
            OSREPORTF("%s: %d\n", introStr, printInt);
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(osReportInt, 2)

    s32 evt_mario_motion_chg_mot(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        motId = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        mario_motion::marioChgMot(motId);
        mario::MarioWork * mario = mario::marioGetPtr();
        mario->flags &= ~(0x10000000);
        mario->invincibilityTimer = 60;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_mario_motion_chg_mot, 1)

    s32 evt_mario_chk_key(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        s32 marioKey = mario::marioChkKey(); // "is mario allowed to move?" returns true if so
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], marioKey);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_mario_chk_key, 1)

    s32 evt_strstr(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * mapName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        const char * comparison = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 compstrReturn = 0;
        // OSREPORTF("p1: %s; p2: %s\n", mapName, comparison);
        if (msl::string::strstr(mapName, comparison) != nullptr)
            compstrReturn = 1;
        evtmgr_cmd::evtSetValue(evtEntry, args[2], compstrReturn);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_strstr, 3)

    s32 migraineShortenCurseTimers(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        mario::MarioWork * mario = mario::marioGetPtr();
        s32 yesRand;
        for (mario::MarioStatus * yes = mario->firstStatus; yes != 0; yes = yes->next) {
            if (Lunatic->Luna.disorder == DISORDER_PURPLE && yes->type == STATUS_SLOW) {
                yes->timer = (Lunatic->Luna.DW.UW.Indolence.slowDuration * 60) - 30;
            } else if (yes->type == STATUS_FLIPPED_CONTROLS || yes->type == STATUS_NO_SKILLS || yes->type == STATUS_NO_JUMP || yes->type == STATUS_SLOW) {
                // WAIT_MSEC(500) precedes this user func, so it starts with .5s
                // Thus, status conditions may last anywhere from 1-10s
                yesRand = system::rand() % 540 + 30;
                yes->timer = (f32)yesRand;
            }
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(migraineShortenCurseTimers, 0)

    s32 evt_eff_spm_spindash(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        f32 x = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        f32 y = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        f32 z = evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        f32 scale = evtmgr_cmd::evtGetValue(evtEntry, args[3]);
        f32 uwVal = evtmgr_cmd::evtGetValue(evtEntry, args[4]);
        eff_spm_spindash::effSpmSpindashEntry(x, y, z, scale, uwVal);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_eff_spm_spindash, 5)

    s32 spectreFloor(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        double hp = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 floor = swdrv::swByteGet(1);
        s32 hpReturn = 0;
        if (hp <= 1) {
            hpReturn = 0;
            evtmgr_cmd::evtSetValue(evtEntry, args[1], hpReturn);
            return 2;
        }
        if (floor <= 24) // Lv1
        {
            hp = (hp / 10);
        } else if (floor <= 48) // Lv2
        {
            hp = (hp / 6.667);
        } else if (floor <= 174) // Lv3
        {
            hp = (hp / 5);
        } else // Lv4
        {
            hp = (hp / 4);
        }
        hpReturn = msl::math::floor(hp);
        if (hpReturn == 0) {
            hpReturn = 1;
        }
        evtmgr_cmd::evtSetValue(evtEntry, args[1], hpReturn);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(spectreFloor, 2)

    s32 paramitaFloor(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        if (pouch->hp >= 1) {
            f32 hp = (f32)(pouch->hp * 0.2);
            s32 hpReturn = msl::math::floor(hp);
            evtmgr_cmd::evtSetValue(evtEntry, args[0], hpReturn);
        } else {
            evtmgr_cmd::evtSetValue(evtEntry, args[0], 0);
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(paramitaFloor, 1)

    char moverSelectBuf[256];

    s32 generate_mover_prices(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        // Change Mover prices based on Pit Level
        s32 moverDown2Price = 10 + (lpGetDanLv() * 10) + (lpGetDifficulty() * 10);
        if (VoucherChkTorn(VOUCHER_YELLOW) == true)
            moverDown2Price *= 0.7f;
        s32 moverDown5Price = moverDown2Price * 2;
        msl::string::memset(moverSelectBuf, 0, sizeof(moverSelectBuf));
        msl::stdio::sprintf(moverSelectBuf, moverChoices, moverDown2Price, moverDown5Price);
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], moverDown2Price);
        evtmgr_cmd::evtSetValue(evtEntry, args[1], moverDown5Price);
        evtmgr_cmd::evtSetValue(evtEntry, args[2], (s32)moverSelectBuf);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(generate_mover_prices, 3)

    s32 mover_down_2(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        s32 floor = swdrv::swByteGet(1);
        floor = floor + 1;
        swdrv::swByteSet(1, floor);
        const char * destMap = getNextDanMapnameNew(floor);
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)destMap);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(mover_down_2, 1)

    s32 mover_down_5(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        s32 floor = swdrv::swByteGet(1);
        if (DebugMode) {
            if ((wpadmgr::wpadGetButtonsHeld(0) & (WPAD_BTN_1 | WPAD_BTN_2)) == (WPAD_BTN_1 | WPAD_BTN_2)) {
                floor += 8;
            } else if ((wpadmgr::wpadGetButtonsHeld(0) & WPAD_BTN_A) != 0) {
                floor += 198;
            } else
                floor += 4;
        } else
            floor += 4;
        swdrv::swByteSet(1, floor);
        const char * destMap = getNextDanMapnameNew(floor);
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)destMap);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(mover_down_5, 1)

    s32 start_from_21(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)evtEntry;
        (void)firstRun;
        mario_pouch::pouchSetMaxHp(20);
        mario_pouch::pouchSetHp(20);
        mario_pouch::pouchAddXp(60000);
        mario_pouch::pouchSetLevel(5);
        mario_pouch::pouchSetAttack(3);
        mario_pouch::pouchAddItem(50);
        mario_pouch::pouchAddItem(217);
        mario_pouch::pouchAddItem(221);
        mario_pouch::pouchAddItem(222);
        mario_pouch::pouchAddItem(80);
        mario_pouch::pouchAddItem(80);
        mario_pouch::pouchAddItem(167);
        mario_pouch::pouchAddItem(70);
        mario_pouch::pouchAddItem(118);
        mario_pouch::pouchSetPixlSelected(ITEM_ID_FAIRY_THROW);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(start_from_21, 0)

    s32 start_from_61(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)evtEntry;
        (void)firstRun;
        // Stats
        mario_pouch::pouchSetMaxHp(30);
        mario_pouch::pouchSetHp(30);
        mario_pouch::pouchAddXp(250000);
        mario_pouch::pouchSetLevel(10);
        mario_pouch::pouchSetAttack(4);
        // Important things
        mario_pouch::pouchAddItem(50);
        mario_pouch::pouchAddItem(36);
        // Char/pixl
        mario_pouch::pouchAddItem(217);
        mario_pouch::pouchAddItem(218);
        mario_pouch::pouchAddItem(221);
        mario_pouch::pouchAddItem(222);
        mario_pouch::pouchAddItem(223);
        mario_pouch::pouchAddItem(224);
        mario_pouch::pouchAddItem(225);
        mario_pouch::pouchAddItem(226);
        mario_pouch::pouchAddItem(227);
        mario_pouch::pouchAddItem(228);
        // Use items
        mario_pouch::pouchAddItem(80);
        mario_pouch::pouchAddItem(167);
        mario_pouch::pouchAddItem(118);
        mario_pouch::pouchAddItem(102);
        mario_pouch::pouchAddItem(113);
        // GSWFs
        swdrv::swSet(515);
        swdrv::swSet(511);
        swdrv::swSet(514);
        swdrv::swSet(513);
        swdrv::swSet(510);
        swdrv::swSet(508);
        swdrv::swSet(534);
        swdrv::swSet(1037);
        mario_pouch::pouchSetPixlSelected(ITEM_ID_FAIRY_BAKUDAN);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(start_from_61, 0)

    s32 start_from_eg(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)evtEntry;
        (void)firstRun;
        mario_pouch::pouchSetMaxHp(40);
        mario_pouch::pouchSetHp(40);
        mario_pouch::pouchAddXp(360000);
        mario_pouch::pouchSetLevel(12);
        mario_pouch::pouchSetAttack(6);
        mario_pouch::pouchAddItem(50);
        mario_pouch::pouchAddItem(36);
        mario_pouch::pouchAddItem(217);
        mario_pouch::pouchAddItem(218);
        mario_pouch::pouchAddItem(219);
        mario_pouch::pouchAddItem(221);
        mario_pouch::pouchAddItem(222);
        mario_pouch::pouchAddItem(223);
        mario_pouch::pouchAddItem(224);
        mario_pouch::pouchAddItem(225);
        mario_pouch::pouchAddItem(226);
        mario_pouch::pouchAddItem(227);
        mario_pouch::pouchAddItem(228);
        mario_pouch::pouchAddItem(229);
        mario_pouch::pouchAddItem(230);
        mario_pouch::pouchAddItem(81);
        mario_pouch::pouchAddItem(167);
        mario_pouch::pouchAddItem(118);
        mario_pouch::pouchAddItem(123);
        mario_pouch::pouchAddItem(68);
        swdrv::swSet(515);
        swdrv::swSet(511);
        swdrv::swSet(514);
        swdrv::swSet(513);
        swdrv::swSet(517);
        swdrv::swSet(510);
        swdrv::swSet(508);
        swdrv::swSet(418);
        swdrv::swSet(533);
        swdrv::swSet(534);
        swdrv::swSet(535);
        swdrv::swSet(536);
        swdrv::swSet(1037);
        swdrv::swSet(1234);
        swdrv::swSet(1235);
        swdrv::swSet(1236);
        swdrv::swSet(1237);
        mario_pouch::pouchSetPixlSelected(ITEM_ID_FAIRY_HAMMER);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(start_from_eg, 0)

    s32 evt_npc_drop_item_new(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 itemType = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 coinCount = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        npcdrv::NPCEntry * npc = evt_npc::evtNpcNameToPtr(evtEntry, "me");
        // If npc has a key already, just drop it
        if (npcCheckHasKey(npc, true, itemType) == false) {
            // If Disorder: Dread is active, disable all item and coin drops except for the Pit Key and Chest Key.
            if (Lunatic->Luna.disorder == DisorderId::DISORDER_ORANGE)
                return 2;
            else if (npcCheckDanFlag(npc, DAN_NPC_CHILD) == true) // If child npc, drop nothing
                return 2;
            else if (npcCheckDanFlag(npc, DAN_NPC_HOLOGRAPHIC) == true) // If holo, increase coin output
                coinCount = clamp((coinCount * 3), 8, 24);
            else if (itemType == ITEM_ID_COOK_SUNNY_SIDE_UP)
                VoucherProc(VOUCHER_GREEN);
            else if (itemType != ITEM_ID_NULL) // If NOT holo, check for Stellar and register a use if an item drops
                VoucherProc(VOUCHER_STELLAR);
        }
        npcmisc::npcDropItem(npc, itemType, coinCount);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_npc_drop_item_new, 2)

    s32 boodinShopItemPool[] = {
        // Custom Pit Rando enemies
        283, 284, 286, 289, 290, 293, 294, 297, 300, 304, 306, 309, 314, 315, 316, 318, 322, 324, 330, 333, 336, 342, 344, 350, 351, 352, 353, 356, 359, 364, 381, 384,
        388, 402, 427, 434, 438, 439, 535, 512,
        // Vanilla enemies
        283, 285, 287, 288, 291, 292, 296, 298, 299, 301, 302, 303, 305, 307, 308, 310, 311, 312, 313, 317, 319, 323, 328, 329, 331, 332, 334, 335, 338, 341, 343, 345,
        346, 347, 348, 349, 354, 355, 358, 360, 362, 363, 365, 366, 372, 373, 374, 375, 377, 378, 379, 380, 382, 383, 385, 386, 387, 389, 392, 393, 394, 395, 396, 398,
        399, 400, 401, 403, 408, 409, 412, 414, 415, 420, 421, 423, 424, 426, 428, 429, 431, 432, 433, 436, 437, 440, 441, 442, 444, 446, 447, 448, 528, 529, 530, 531};

    s32 dan_boodin_setup_cards(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        Lunatic->Boodin.cardNum = system::rand() % 9 + 5; // 5-13
        Lunatic->Boodin.Cards = (CWSelectItemDesc *)memory::__memAlloc(memory::HEAP_MAP, sizeof(CWSelectItemDesc) * Lunatic->Boodin.cardNum);
        Lunatic->Boodin.Cards[0].itemId = (s32)item_data::ItemType::ITEM_ID_USE_SUPER_BLANK_KUN;
        Lunatic->Boodin.Cards[0].iconId = -1;
        Lunatic->Boodin.Cards[0].cost = 80;
        if (VoucherChkTorn(VOUCHER_YELLOW) == true)
            Lunatic->Boodin.Cards[0].cost *= 0.7f;
        s32 i;
        s32 poolCard;
        for (i = 1; i < Lunatic->Boodin.cardNum; i += 1) {
            CWSelectItemDesc * Card = &Lunatic->Boodin.Cards[i];
            poolCard = system::rand() % (sizeof(boodinShopItemPool) / 8);
            Card->itemId = boodinShopItemPool[poolCard];
            Card->iconId = -1;
            Card->cost = (s32)msl::math::floor((f32)item_data::itemDataTable[Card->itemId].buyPrice * 0.8f);
            if (VoucherChkTorn(VOUCHER_YELLOW) == true)
                Card->cost *= 0.7f;
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(dan_boodin_setup_cards, 0)

    s32 dan_boodin_get_descs(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], s32(Lunatic->Boodin.Cards));
        evtmgr_cmd::evtSetValue(evtEntry, args[1], Lunatic->Boodin.cardNum);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(dan_boodin_get_descs, 2)

    s32 dan_boodin_backup_descs(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)evtEntry;
        (void)firstRun;
        s32 id = customwin::CWSelectKeyToId("Cards");
        Lunatic->Boodin.cardNum = GlobalCW->Select[id]->num;
        msl::string::memset(Lunatic->Boodin.Cards, 0, (sizeof(CWSelectItemDesc) * Lunatic->Boodin.cardNum));
        msl::string::memcpy(Lunatic->Boodin.Cards, GlobalCW->Select[id]->Descs, (sizeof(CWSelectItemDesc) * Lunatic->Boodin.cardNum));
        return 2;
    }
    EVT_DECLARE_USER_FUNC(dan_boodin_backup_descs, 0)

    /*s32 merlunaCutsceneSetMaterialFlags(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        mario::MarioWork *mario = mario::marioGetPtr();
        animdrv::animPoseSetMaterialFlagOn(mario->animPoseIds[0], 0x40000000);
        animdrv::animPoseSetMaterialFlagOn(npcdrv::npcNameToPtr("rebear")->m_Anim.m_nPoseId, 0x40000000);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(merlunaCutsceneSetMaterialFlags, 0)

    s32 render_judgement(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        // Blessing
        s32 blessing = system::rand() % 3 + 1;
        blessStorage = blessing;
        char *blessName;
        switch (blessing)
        {
        case BlessId::MERLUNA_SPECTRE:
            blessName = "Spectre";
            break;
        case BlessId::MERLUNA_HOURAI:
            blessName = "Hourai Doll";
            break;
        case BlessId::MERLUNA_PARAMITA:
            blessName = "Paramita";
            break;
        }
        // Curse
        s32 curse = system::rand() % 3 + 1;
        curseStorage = curse;
        char *curseName;
        switch (curse)
        {
        case CurseId::MERLUNA_SHION:
            curseName = "Tatarian Aster";
            break;

        case CurseId::MERLUNA_HEX:
            curseName = "Vulnerability Hex";
            break;

        case CurseId::MERLUNA_MIGRAINE:
            curseName = "Migraine";
            break;
        }

        // Determine which one actually happens
        s32 difficulty = lpGetDifficulty();
        s32 blessThreshold = 0;
        switch (difficulty)
        {
        case 0:
            blessThreshold = 80;
            break;
        case 1:
            blessThreshold = 70;
            break;
        case 2:
            blessThreshold = 50;
            break;
        }
        s32 judgement = system::rand() % 100;
        if (judgement < blessThreshold)
        {
            judgement = 0; // Is Blessing
        }
        else
        {
            judgement = 1; // Is Curse
        }
        //    OSREPORTF("merluna bless name: %s.\n", blessName);
        //    OSREPORTF("merluna curse name: %s.\n", curseName);
        evtmgr_cmd::evtSetValue(evtEntry, args[0], blessing);
        evtmgr_cmd::evtSetValue(evtEntry, args[1], blessName);
        evtmgr_cmd::evtSetValue(evtEntry, args[2], curse);
        evtmgr_cmd::evtSetValue(evtEntry, args[3], curseName);
        evtmgr_cmd::evtSetValue(evtEntry, args[4], judgement);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(render_judgement, 5)

    s32 rerender_judgement(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        // Blessing
        char *blessName;
        switch (blessStorage)
        {
        case BlessId::MERLUNA_SPECTRE:
            blessName = "Spectre";
            break;
        case BlessId::MERLUNA_HOURAI:
            blessName = "Hourai Doll";
            break;
        case BlessId::MERLUNA_PARAMITA:
            blessName = "Paramita";
            break;
        }
        // Curse
        char *curseName;
        switch (curseStorage)
        {
        case CurseId::MERLUNA_SHION:
            curseName = "Tatarian Aster";
            break;

        case CurseId::MERLUNA_HEX:
            curseName = "Vulnerability Hex";
            break;

        case CurseId::MERLUNA_MIGRAINE:
            curseName = "Migraine";
            break;
        }

        // Determine which one actually happens
        s32 difficulty = lpGetDifficulty();
        s32 blessThreshold = 0;
        switch (difficulty)
        {
        case 0:
            blessThreshold = 80;
            break;
        case 1:
            blessThreshold = 70;
            break;
        case 2:
            blessThreshold = 50;
            break;
        }
        s32 judgement = system::rand() % 100;
        if (judgement < blessThreshold)
        {
            judgement = 0; // Is Blessing
        }
        else
        {
            judgement = 1; // Is Curse
        }
        //   OSREPORTF("RERENDER merluna bless name: %s.\n", blessName);
        //   OSREPORTF("RERENDER merluna curse name: %s.\n", curseName);
        //   OSREPORTF("RERENDER merluna judgement: %d.\n", judgement);
        evtmgr_cmd::evtSetValue(evtEntry, args[0], blessStorage);
        evtmgr_cmd::evtSetValue(evtEntry, args[1], blessName);
        evtmgr_cmd::evtSetValue(evtEntry, args[2], curseStorage);
        evtmgr_cmd::evtSetValue(evtEntry, args[3], curseName);
        evtmgr_cmd::evtSetValue(evtEntry, args[4], judgement);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(rerender_judgement, 5)

    s32 render_judgement_2(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 bless = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 curse = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 blessingOrCurse = evtmgr_cmd::evtGetValue(evtEntry, args[2]);

        // Set target GSW to the judgement ID
        if (blessingOrCurse == 0)
        {
            swdrv::swByteSet(1600, bless);
            swdrv::swByteSet(1602, 20);
        }
        else
        {
            swdrv::swByteSet(1601, curse);
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(render_judgement_2, 3)

    s32 explain_judgement(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 bless = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 curse = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 judgementType = evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        s32 speaker = evtmgr_cmd::evtGetValue(evtEntry, args[3]);

        // Blessing
        if (judgementType == 0)
        {
            switch (bless)
            {
            case BlessId::MERLUNA_SPECTRE:
                return evt_msg::_evt_msg_print(evtEntry, firstRun, 1, merlunaSpectre, 0, speaker);
                break;
            case BlessId::MERLUNA_HOURAI:
                return evt_msg::_evt_msg_print(evtEntry, firstRun, 1, merlunaHourai, 0, speaker);
                break;
            case BlessId::MERLUNA_PARAMITA:
                return evt_msg::_evt_msg_print(evtEntry, firstRun, 1, merlunaParamita, 0, speaker);
                break;
            }
        }
        else // Curse
        {
            switch (curse)
            {
            case CurseId::MERLUNA_SHION:
                return evt_msg::_evt_msg_print(evtEntry, firstRun, 1, merlunaAster, 0, speaker);
                break;
            case CurseId::MERLUNA_HEX:
                return evt_msg::_evt_msg_print(evtEntry, firstRun, 1, merlunaHex, 0, speaker);
                break;
            case CurseId::MERLUNA_MIGRAINE:
                return evt_msg::_evt_msg_print(evtEntry, firstRun, 1, merlunaMigraine, 0, speaker);
                break;
            }
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(explain_judgement, 4)*/

    static evt_door::DokanDesc new_dan_70_dokan_desc = {evt_door::IN_DOWN, 0, 0, "dokan", "dan_70", "A2D_dokan_1", "A3D_dokan_1", "mac_05", "dokan_1"};

    npcdrv::NPCTribeAnimDef merlunaAnims[] = {{0, "n_stg2_mistS_1"},      // Idle
                                              {1, "n_stg2_mistW_1"},      // Walking
                                              {2, "n_stg2_mistR_1"},      // Running
                                              {3, "n_stg2_mistT_1"},      // Talking (Idle)
                                              {10, "n_stg2_mistM_1a"},    // Raise your glowsticks~
                                              {11, "n_stg2_mistM_1b"},    // Wave them around~
                                              {12, "n_stg2_mistM_1c"},    // Bakudaaaan~
                                              {13, "n_stg2_mistM_1d"},    // Phew!
                                              {25, "n_stg2_mistA_1"},     // ):<
                                              {26, "n_stg2_mistA_2"},     // D:<
                                              {27, "n_stg2_mistO_1"},     // O_O
                                              {28, "n_stg2_mistTW_1_a"},  // KEK
                                              {29, "n_stg2_mistTW_1_b"},  // LMAOOOOOO
                                              {30, "n_stg2_mistTW_1_c"},  // lol
                                              {31, "n_stg2_mistTOB_1_a"}, // >_<
                                              {32, "n_stg2_mistTOB_1_b"}, // @_@
                                              {33, "n_stg2_mistTOB_1_c"}, // Back to normal
                                              {34, "n_stg2_mistHi_1"},    // :3
                                              {35, "n_stg2_mistS_2"},     // Idle (demure)
                                              {36, "n_stg2_mistJ_1_a"},   // Jump
                                              {37, "n_stg2_mistJ_1_b"},   // Land
                                              {-1, "n_stg2_mistZ_1"}};

    npcdrv::NPCTribeAnimDef moverAnims[] = {{0, "stg2_syuuzin_b_S_1"}, // Idle
                                            {1, "stg2_syuuzin_b_W_1"}, // Walking
                                            {2, "stg2_syuuzin_b_R_1"}, // Running
                                            {3, "stg2_syuuzin_b_T_1"}, // Talking (Idle)
                                            {-1, "stg2_syuuzin_b_Z_1"}};

    npcdrv::NPCTribeAnimDef boodinAnims[] = {{0, "S_1"},  // Idle
                                             {1, "W_1"},  // Walking
                                             {2, "R_1"},  // Running
                                             {3, "T_1"},  // Talking
                                             {24, "T_2"}, // One bow goodbye
                                             {25, "T_3"}, // Three small bows goodbye (unused)
                                             {26, "O_1"}, // WHAT
                                             {-1, "Z_1"}};

    npcdrv::NPCTribeAnimDef mitchAnims[] = {{0, "S_1"},  // Idle
                                            {1, "W_1"},  // Walking
                                            {2, "R_1"},  // Running
                                            {3, "T_1"},  // Talking
                                            {10, "H_2"}, // Nodders
                                            {-1, "Z_1"}};

    npcdrv::NPCTribeAnimDef gabbiAnims_Madge[] = {{0, "S_4B"},  // Idle
                                                  {1, "W_1"},   // Walking
                                                  {2, "R_1"},   // Running
                                                  {3, "T_4"},   // Talking
                                                  {21, "Y_2"},  // Asleep
                                                  {22, "Y_1A"}, // Shock
                                                  {23, "Y_1B"}, // Shock -> normal
                                                  {24, "S_4A"}, // Normal -> madge
                                                  {25, "S_4C"}, // Madge -> normal
                                                  {26, "S_3A"}, // Start blush
                                                  {-1, "Z_1"}};

    npcdrv::NPCTribeAnimDef gabbiAnims_Sadge[] = {{0, "S_2B"},  // Idle
                                                  {1, "W_1"},   // Walking
                                                  {2, "R_1"},   // Running
                                                  {3, "T_2"},   // Talking
                                                  {21, "S_2A"}, // Start being sad
                                                  {22, "S_2C"}, // Sadge -> normal
                                                  {23, "S_3A"}, // Start blush
                                                  {-1, "Z_1"}};

    npcdrv::NPCTribeAnimDef gabbiAnims_Neutral[] = {{0, "S_1"},   // Idle
                                                    {1, "W_1"},   // Walking
                                                    {2, "R_1"},   // Running
                                                    {3, "T_1"},   // Talking
                                                    {21, "S_2A"}, // Start sad
                                                    {22, "S_4A"}, // Start mad
                                                    {23, "S_3A"}, // Start blush
                                                    {-1, "Z_1"}};

    npcdrv::NPCTribeAnimDef gabbiAnims_Blush[] = {{0, "S_3B"},  // Idle
                                                  {1, "W_1"},   // Walking
                                                  {2, "R_1"},   // Running
                                                  {3, "T_3"},   // Talking
                                                  {21, "S_3A"}, // Start blush
                                                  {22, "S_3C"}, // Blush -> normal
                                                  {23, "S_2C"}, // Sadge -> normal
                                                  {-1, "Z_1"}};

    /* static evt_door::DokanDesc temp_mac_04_2_dokan_desc = {
         0, 0, 0, "dokan", "mac_04", "A2D_dokan_1", "A3D_dokan_1", "dan_70", "dokan_1"}; */

    EVT_BEGIN(handle_dj_misc_behavior)
    // Search for dialogue that needs to be run
    IF_EQUAL(GSWF(1671), 1)
    WAIT_MSEC(300)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(houraiWearOff), 0, 0)
    END_IF()
    USER_FUNC(handleBlessingWearOff)
    RETURN()
    EVT_END()

    EVT_BEGIN(fix_seq_transition)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(10), LW(11), LW(12))
    USER_FUNC(evt_mario::evt_mario_get_height, LW(13))
    DIVF(LW(13), FLOAT(2.0))
    ADDF(LW(11), LW(13))
    USER_FUNC(evt_fade::evt_fade_set_center_pos, LW(10), LW(11), LW(12))
    RETURN()
    EVT_END()

    EVT_BEGIN(mover_speech)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(generate_mover_prices, LW(3), LW(4), LW(5))
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(moverIntro), 0, PTR("me"))
    USER_FUNC(evt_msg::evt_msg_select, 1, LW(5))
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_EQUAL(LW(0), 2) // "I'm good, thanks"
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(moverNo), 0, PTR("me"))
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_IF()
    USER_FUNC(evt_pouch::evt_pouch_get_coins, LW(1))
    IF_EQUAL(LW(0), 1)     // Down 5 Floors
    IF_SMALL(LW(1), LW(4)) // Checks if Mario is poor...
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(moverScam), 0, PTR("me"))
    USER_FUNC(evt_mario::evt_mario_key_on)
    ELSE() // ...but if he's not, Proceed tm
    USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
    WAIT_MSEC(500)
    MUL(LW(4), -1)
    USER_FUNC(evt_pouch::evt_pouch_add_coins, LW(4))
    USER_FUNC(evt_shop::evt_shop_wait_coin_sfx)
    WAIT_MSEC(500)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(moverYes), 0, PTR("me"))
    USER_FUNC(mover_down_5, LW(2))
    USER_FUNC(evt_pouch::evt_pouch_check_have_item, 48, LW(5)) // Removes Pit Key if Mario has one
    IF_EQUAL(LW(5), 1)
    USER_FUNC(evt_pouch::evt_pouch_remove_item, 48)
    END_IF()
    RUN_CHILD_EVT(fix_seq_transition)
    WAIT_MSEC(500)
    USER_FUNC(evt_seq::evt_seq_set_seq, seqdrv::SEQ_MAPCHANGE, LW(2), PTR(0))
    END_IF()
    END_IF()
    IF_EQUAL(LW(0), 0)     // Down 2 Floors
    IF_SMALL(LW(1), LW(3)) // Checks if Mario is poor...
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(moverScam), 0, PTR("me"))
    USER_FUNC(evt_mario::evt_mario_key_on)
    ELSE() // ...but if he's not, Proceed tm
    USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
    WAIT_MSEC(500)
    MUL(LW(3), -1)
    USER_FUNC(evt_pouch::evt_pouch_add_coins, LW(3))
    USER_FUNC(evt_shop::evt_shop_wait_coin_sfx)
    WAIT_MSEC(500)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(moverYes), 0, PTR("me"))
    USER_FUNC(mover_down_2, LW(2))
    USER_FUNC(evt_pouch::evt_pouch_check_have_item, 48, LW(5)) // Removes Pit Key if Mario has one
    IF_EQUAL(LW(5), 1)
    USER_FUNC(evt_pouch::evt_pouch_remove_item, 48)
    END_IF()
    RUN_CHILD_EVT(fix_seq_transition)
    WAIT_MSEC(500)
    USER_FUNC(evt_seq::evt_seq_set_seq, seqdrv::SEQ_MAPCHANGE, LW(2), PTR(0))
    END_IF()
    END_IF()
    RETURN()
    EVT_END()

    EVT_BEGIN(fwd_mover_speech)
    RUN_EVT(mover_speech)
    RETURN()
    EVT_END()

    EVT_BEGIN(LunaticMusicHandler)
    SWITCH(GSW(1))
    CASE_SMALL(25)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_LUNATIC_A"), 500)
    CASE_SMALL(150)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_LUNATIC_B"), 500)
    CASE_SMALL(175)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_LUNATIC_C"), 500)
    CASE_ETC()
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_LUNATIC_D"), 500)
    END_SWITCH()
    RETURN()
    EVT_END()

    // Overwrite Pit music
    EVT_BEGIN(custom_pit_music)
    SWITCH(GSW(1621))
    CASE_EQUAL(0)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_100F"), 500)
    CASE_EQUAL(1)
    RUN_CHILD_EVT(LunaticMusicHandler)
    CASE_EQUAL(2)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_100F8BIT"), 500)
    CASE_EQUAL(3)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_100FSYNTH"), 500)
    CASE_EQUAL(4)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_100FPIANO"), 500)
    CASE_EQUAL(5)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MAP_100FBEATS"), 500)
    CASE_EQUAL(6)
    USER_FUNC(evt_snd::evt_snd_bgmoff, 0)
    END_SWITCH()
    USER_FUNC(evt_snd::evt_snd_set_sfx_reverb_mode, 0)
    RETURN()
    EVT_END()

    s32 evt_shadoo_set_mario_col(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        if (swdrv::swGet(1643)) {
            Lunatic->Misc.marioFullColor = marioIfHeWereBlue;
            Lunatic->Misc.marioFullColor.a = 0;
        } else {
            Lunatic->Misc.marioFullColor = marioIfHeWerent;
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_shadoo_set_mario_col, 0)

    s32 evt_shadoo_intpl_mario_col(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        Lunatic->Misc.hardShadooBlockColChg = false;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        u8 alpha = 255 - (u8)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        Lunatic->Misc.marioFullColor.a = alpha;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_shadoo_intpl_mario_col, 1)

    s32 evt_shadoo_block_colchg(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        Lunatic->Misc.hardShadooBlockColChg = true;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_shadoo_block_colchg, 0)

    EVT_BEGIN(hard_shadoo_fadeout)
    USER_FUNC(evt_shadoo_set_mario_col)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 255, 0, 1500)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(evt_map::evt_map_set_blend, 1, LW(0), LW(0), LW(0), 255)
    USER_FUNC(evt_shadoo_intpl_mario_col, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    USER_FUNC(evt_shadoo_block_colchg)
    RETURN()
    EVT_END()

    EVT_BEGIN(hard_shadoo_fadein)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 0, 255, 2500)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(evt_map::evt_map_set_blend, 1, LW(0), LW(0), LW(0), 255)
    USER_FUNC(evt_shadoo_intpl_mario_col, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    RETURN()
    EVT_END()

    EVT_BEGIN(new_shadoo_evt)
    SET(GSW(24), 1) // just in case
    DO(0)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    IF_LARGE(LW(0), -50)
    DO_BREAK()
    END_IF()
    WAIT_FRM(1)
    WHILE()
    USER_FUNC(evt_mario::evt_mario_key_off, 0)
    USER_FUNC(evt_mario::evt_mario_fairy_reset)
    USER_FUNC(evt_cam::evt_cam_look_at_door, 1, 0)
    INLINE_EVT()
    WAIT_MSEC(200)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_11"), 0)
    END_INLINE()
    IF_LARGE(GSW(1622), 0) // If you've already fought Shadoo, new dialogue
    USER_FUNC(evt_lp_get_difficulty, LW(13))
    ADD(LW(13), 1)
    IF_LARGE_EQUAL(GSW(1622), LW(13))
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(shadooIntroMock), 0, 0)
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(shadooIntroRamp), 0, 0)
    END_IF()
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(shadooIntro), 0, 0)
    END_IF()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("E_3"), 0)
    USER_FUNC(evt_snd::evt_snd_sfxon, PTR("SFX_EVT_QUAKE1L"))
    USER_FUNC(evt_snd::evt_snd_get_last_sfx_id, LW(0))
    SET(LW(15), FLOAT(1.5))
    IF_EQUAL(GSWF(1630), 0) // Lighter camera tremors ACTIVE
    SET(LW(15), FLOAT(0.5))
    END_IF()
    USER_FUNC(evt_lp_get_difficulty, LW(13))
    IF_LARGE_EQUAL(LW(13), 2)
    RUN_EVT(hard_shadoo_fadeout)
    END_IF()
    USER_FUNC(evt_cam::evt_cam_shake, camdrv::CAM_ID_3D, LW(15), LW(15), FLOAT(0.0), 2000, 0)
    USER_FUNC(evt_snd::evt_snd_sfxoff, LW(0))
    USER_FUNC(evt_mario::evt_mario_get_character, LW(1))
    IF_EQUAL(LW(1), 1)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1B"), 0)
    ELSE()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    END_IF()
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, 50, 150, 334, 50, 150, -16, 500, 11)
    WAIT_MSEC(500)
    WAIT_MSEC(1000)
    IF_NOT_EQUAL(GSW(1621), 6) // No music playing
    IF_LARGE_EQUAL(LW(13), 2)
    USER_FUNC(evt_snd::evt_snd_bgmon, 0, PTR("BGM_MAP_HARDBOSS"))
    ELSE()
    USER_FUNC(evt_snd::evt_snd_bgmon, 0, PTR("BGM_BTL_BOSS_MIDDLE1"))
    END_IF()
    END_IF()
    USER_FUNC(evt_npc::evt_npc_tribe_agb_async, 286)
    USER_FUNC(evt_npc::evt_npc_entry_from_template, 0, 286, 0, -100, 0, LW(10), EVT_NULLPTR)
    USER_FUNC(evt_npc::evt_npc_set_anim, LW(10), 0, 1)
    USER_FUNC(evt_npc::func_80107c38, LW(10), 0)
    USER_FUNC(evt_npc::evt_npc_flag8_onoff, LW(10), 0, 8)
    USER_FUNC(evt_npc::evt_npc_flag8_onoff, LW(10), 1, 65536)
    USER_FUNC(declare_shadoo_stats, LW(10))
    USER_FUNC(evt_npc::evt_npc_flip_to, LW(10), 1)
    USER_FUNC(evt_npc::evt_npc_finish_flip_instant, LW(10))
    USER_FUNC(evt_npc::evt_npc_set_position, LW(10), 50, 110, 20)
    USER_FUNC(evt_npc::func_800ff8f8, LW(10), 50, 110, 20)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_DRAW1"), LW(10))
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_100_PC_LINE_TURN1"), LW(10))
    USER_FUNC(evt_npc::evt_npc_flip, LW(10))
    USER_FUNC(evt_npc::evt_npc_wait_flip_finished, LW(10))
    WAIT_MSEC(1000)
    USER_FUNC(evt_npc::evt_npc_flag8_onoff, LW(10), 1, 8)
    WAIT_MSEC(500)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_P_MARIO_LAND1"), LW(10))
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, 0, 75, 484, 0, 75, -16, 800, 11)
    WAIT_MSEC(800)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d_player_character, PTR("SFX_P_V_MARIO_BIKKURI1"), PTR("SFX_P_V_PEACH_ATTACK4"), PTR("SFX_P_V_KOOPA_BIKKURI1"),
              PTR("SFX_P_V_LUIGI_BIKKURI1"))
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("E_5"), 0)
    USER_FUNC(evt_mario::evt_mario_wait_anim)
    USER_FUNC(evt_mario::evt_mario_get_character, LW(0))
    IF_EQUAL(LW(0), 1)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1B"), 0)
    ELSE()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    END_IF()
    WAIT_MSEC(300)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(shadooDoBattle), 0, LW(10))
    USER_FUNC(evt_npc::evt_npc_restart_evt_id, LW(10))
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 500, 11)
    WAIT_MSEC(500)
    USER_FUNC(evt_mario::evt_mario_set_anim_change_handler, 0)
    USER_FUNC(evt_mario::evt_mario_key_on)
    RUN_CHILD_EVT(dan::dan_shadoo_fight_evt)
    RETURN()
    EVT_END()

    customwin::CWSelectColorDef rainbowSelectBgCols[] = {
        {{255, 190, 190, 200}, {255, 200, 190, 200}, 80, 0}, // Red-Orange
        {{255, 190, 255, 200}, {255, 190, 190, 200}, 80, 0}, // Magenta-Red
        {{200, 190, 255, 200}, {255, 190, 255, 200}, 80, 0}, // Purple-Magenta
        {{190, 190, 255, 200}, {200, 190, 255, 200}, 80, 0}, // Blue-Purple
        {{190, 255, 255, 200}, {190, 190, 255, 200}, 80, 0}, // Cyan-Blue
        {{190, 255, 190, 200}, {190, 255, 255, 200}, 80, 0}, // Green-Cyan
        {{255, 255, 190, 200}, {190, 255, 190, 200}, 80, 0}, // Yellow-Green
        {{255, 200, 190, 200}, {255, 255, 190, 200}, 80, 0}  // Orange-Yellow
    };

    s32 dan_70_generate_artifact_defs(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        customwin::CWSelectItemDesc * ArtifactDefs = (customwin::CWSelectItemDesc *)memory::__memAlloc(memory::HEAP_MAP, sizeof(customwin::CWSelectItemDesc) * 6);
        RFCItemData * RFC_SpecialItems = (RFCItemData *)RFCSpecialGetPtr();
        s32 n = 0;
        for (s32 i = 0; i < 6; i += 1) {
            if (swdrv::swGet(1690 + i) == true)
                continue;
            s32 itemId = i + ARTIFACT_SOUL;
            msl::string::memcpy(ArtifactDefs[n].nameTxt, RFC_SpecialItems[itemId].name, msl::string::strlen(RFC_SpecialItems[itemId].name));
            msl::string::memcpy(ArtifactDefs[n].descTxt, RFC_SpecialItems[itemId].description, msl::string::strlen(RFC_SpecialItems[itemId].description));
            ArtifactDefs[n].nameColor = RFC_SpecialItems[itemId].textDrawCol;
            ArtifactDefs[n].iconId = TPLPATCH_ICON(RFC_SpecialItems[itemId].iconId);
            n += 1;
        }
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)ArtifactDefs);
        evtmgr_cmd::evtSetValue(evtEntry, args[1], n);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(dan_70_generate_artifact_defs, 2)

    s32 dan_70_artifact_get_lpitem_id(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 selectionIdx = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 itemId = customwin::CWSelectGetActiveEntry()->Descs[selectionIdx].iconId + (ARTIFACT_SOUL - ICON_ARTIFACT_SOUL) - TPLPATCH_ICON_REDIRECT;
        s32 trueIdx = selectionIdx;
        for (s32 i = 0; i < 6; i += 1) {
            if (i <= (itemId - ARTIFACT_SOUL) && swdrv::swGet(1690 + i) == true)
                trueIdx += 1;
        }
        evtmgr_cmd::evtSetValue(evtEntry, args[1], itemId);
        evtmgr_cmd::evtSetValue(evtEntry, args[2], trueIdx);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(dan_70_artifact_get_lpitem_id, 3)

    EVT_BEGIN(artifactReward)
    USER_FUNC(evt_mobj::evt_mobj_wait_animation_end, PTR("me"), 0)
    USER_FUNC(dan_70_generate_artifact_defs, LW(0), LW(13))
    USER_FUNC(EvtCWSelectEntry, PTR("Artifact"), CWSELECT_DEFAULT, PTR("Treasure"), PTR("Choose wisely!"), LW(0), LW(13))
    USER_FUNC(EvtCWSelectSetHeaderColor, PTR("Artifact"), PTR(&RFCArtiHeaderCol))
    USER_FUNC(EvtCWSelectSetBGColor, PTR("Artifact"), PTR(rainbowSelectBgCols), 8)
    USER_FUNC(EvtCWSelectMenuStart, PTR("Artifact"), 0, LW(0))
    IF_NOT_EQUAL(LW(0), -1) // Select menu NOT cancelled
    USER_FUNC(dan_70_artifact_get_lpitem_id, LW(0), LW(5), LW(14))
    IF_SMALL(LW(14), 4) // Demise and Delight will not trigger this
    USER_FUNC(LPGUIShowHideStats, 1)
    WAIT_MSEC(500)
    END_IF()
    WAIT_MSEC(500)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(1), LW(2), LW(3))
    USER_FUNC(RFCProcEffect, LW(5))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_I_HEART_HUERU1"), LW(1), LW(2), LW(3))
    INLINE_EVT()
    WAIT_MSEC(200)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("I_2"), 0)
    END_INLINE()
    USER_FUNC(RFCAnalyzeSpecial, LW(0), 0)
    WAIT_MSEC(1400)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    IF_SMALL(LW(14), 4) // Demise and Delight will not trigger this
    USER_FUNC(LPGUIShowHideStats, 0)
    WAIT_MSEC(500)
    END_IF()
    WAIT_MSEC(100)
    ADD(LW(14), 1690)
    USER_FUNC(ToggleGSWF, LW(14))
    END_IF()
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("Artifact"))
    // Set GSW(1622) to highest difficulty beaten + 1
    USER_FUNC(evt_lp_get_difficulty, LW(0))
    IF_SMALL(GSW(1622), LW(0))
    SET(GSW(1622), LW(0))
    ADD(GSW(1622), 1)
    END_IF()
    SET(LSWF(1), 1)
    RETURN()
    EVT_END()

    EVT_BEGIN(dan_70_rewards_cont)
    USER_FUNC(evt_mario::evt_mario_key_off, 0)
    USER_FUNC(evt_mario::evt_mario_fairy_reset)
    USER_FUNC(evt_map::evt_mapobj_get_position, PTR("dokan"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_cam::func_800e01f8)
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, LW(0), EVT_NULLPTR, EVT_NULLPTR, LW(0), EVT_NULLPTR, EVT_NULLPTR, 1000, 11)
    WAIT_MSEC(500)
    WAIT_MSEC(1000)
    USER_FUNC(evt_map::evt_mapobj_flag_onoff, 1, 0, PTR("dokan"), 1)
    USER_FUNC(evt_hit::evt_hitobj_onoff, PTR("A2D_dokan"), 1, 1)
    USER_FUNC(evt_hit::evt_hitobj_onoff, PTR("A3D_dokan"), 1, 1)
    USER_FUNC(evt_hit::evt_hit_bind_mapobj, PTR("A2D_dokan"), PTR("dokan"))
    USER_FUNC(evt_hit::evt_hit_bind_mapobj, PTR("A3D_dokan"), PTR("dokan"))
    USER_FUNC(evt_map::evt_mapobj_get_position, PTR("dokan"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_MOBJ_DOKAN_BORN1"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, -3000, 0, 1000)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    DIVF(LW(0), FLOAT(100.0))
    USER_FUNC(evt_map::evt_mapobj_trans, PTR("dokan"), 0, LW(0), 0)
    USER_FUNC(evt_hit::evt_hit_bind_update, PTR("A2D_dokan"))
    USER_FUNC(evt_hit::evt_hit_bind_update, PTR("A3D_dokan"))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    WAIT_MSEC(1000)
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 500, 11)
    WAIT_MSEC(500)
    USER_FUNC(evt_door::evt_door_enable_disable_dokan_desc, 1, PTR("dokan"))
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    EVT_END()

    EVT_BEGIN(fwd_new_shadoo_evt)
    RUN_EVT(new_shadoo_evt)
    RETURN_FROM_CALL()

    s32 dan_70_determine_artifact_spawn(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 difficulty = lpGetDifficulty();
        bool beatenThisDifficulty = swdrv::swGet(1650 + difficulty);
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)beatenThisDifficulty);
        if (!beatenThisDifficulty)
            swdrv::swSet(1650 + difficulty);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(dan_70_determine_artifact_spawn, 1)

    EVT_BEGIN(dan_70_new_rewards)
    WAIT_MSEC(500)
    USER_FUNC(evt_lp_get_difficulty, LW(13))
    IF_EQUAL(LW(13), 2)
    SET(GW(15), 1)
    WAIT_MSEC(500)
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 666, 11)
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("E_3"), 0)
    USER_FUNC(evt_snd::evt_snd_sfxon, PTR("SFX_EVT_QUAKE1L"))
    USER_FUNC(evt_snd::evt_snd_get_last_sfx_id, LW(0))
    SET(LW(15), FLOAT(1.2))
    IF_EQUAL(GSWF(1630), 0) // Lighter camera tremors ACTIVE
    SET(LW(15), FLOAT(0.4))
    END_IF()
    USER_FUNC(evt_snd::evt_snd_bgmoff_f_d, 0, 2500)
    RUN_EVT(hard_shadoo_fadein)
    USER_FUNC(evt_cam::evt_cam_shake, camdrv::CAM_ID_3D, LW(15), LW(15), FLOAT(0.0), 3500, 0)
    USER_FUNC(evt_snd::evt_snd_sfxoff, LW(0))
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_7"), 0)
    WAIT_MSEC(1500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(hardShadooBadFeeling), 0, 0)
    WAIT_MSEC(500)
    END_IF()
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, LW(0), 60, 384, LW(0), 60, -16, 500, 11)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_11"), 0)
    WAIT_MSEC(1000)
    IF_EQUAL(GSW(1622), 0)
    USER_FUNC(evt_mario::evt_mario_get_character, LW(0))
    SWITCH(LW(0))
    CASE_EQUAL(0)
    USER_FUNC(evt_guide::evt_guide_get_can_search, LW(0))
    IF_EQUAL(LW(0), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(tippiDS), 0, PTR("__guide__"))
    END_IF()
    CASE_EQUAL(1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(peachDS), 0, PTR("__mario__"))
    CASE_EQUAL(2)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(bowserDS), 0, PTR("__mario__"))
    CASE_EQUAL(3)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(luigiDS), 0, PTR("__mario__"))
    END_SWITCH()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    ELSE()
    USER_FUNC(evt_guide::evt_guide_get_can_search, LW(0))
    IF_EQUAL(LW(0), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(tippiDSAgain), 0, PTR("__guide__"))
    END_IF()
    END_IF()
    IF_EQUAL(GSWF(1600), 1)
    IF_EQUAL(GSWF(1602), 0)
    SET(GSWF(1602), 1)
    WAIT_MSEC(1500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(tippiDS_Sus), 0, PTR("__guide__"))
    END_IF()
    END_IF()
    USER_FUNC(evt_snd::evt_snd_bgmoff_f_d, 0, 2000)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    USER_FUNC(evt_mario::evt_mario_face_coords, 200, 0)
    USER_FUNC(dan_70_determine_artifact_spawn, LW(2))
    IF_EQUAL(LW(2), 0)
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, 0, 160, 1034, 0, 160, -16, 500, 11)
    WAIT_MSEC(500)
    INLINE_EVT()
    USER_FUNC(evt_snd::evt_snd_bgmon, 2, PTR("BGM_FF_CORRECT1"))
    USER_FUNC(evt_snd::evt_snd_get_bgm_wait_time, 2, LW(0))
    WAIT_MSEC(LW(0))
    USER_FUNC(evt_snd::evt_snd_bgmoff_f_d, 2, 1000)
    RUN_EVT(custom_pit_music)
    END_INLINE()
    USER_FUNC(evt_mobj::evt_mobj_thako, 1, PTR("Artifact"), 0, -100, -75, 0, PTR(artifactReward), 0, 0)
    USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, 0, PTR("Artifact"), 0x40)
    USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, 1, PTR("Artifact"), 0x10000)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 1000, 0, 2000)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    DIVF(LW(0), FLOAT(1000.0))
    SETF(LW(2), LW(0))
    MULF(LW(2), FLOAT(-100.0))
    SETF(LW(3), LW(0))
    MULF(LW(3), FLOAT(1440.0))
    USER_FUNC(evt_mobj::evt_mobj_get_position, PTR("Artifact"), LW(5), LW(6), LW(7))
    USER_FUNC(evt_mobj::evt_mobj_set_position, PTR("Artifact"), LW(5), LW(2), LW(7))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    ELSE()
    SET(LSWF(1), 1)
    END_IF()
    USER_FUNC(evt_mario::evt_mario_set_anim_change_handler, 0)
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 500, 11)
    WAIT_MSEC(500)
    USER_FUNC(evt_mario::evt_mario_key_on)
    DO(0)
    IF_EQUAL(LSWF(1), 1)
    RUN_EVT(dan_70_rewards_cont)
    RETURN()
    END_IF()
    WAIT_FRM(1)
    WHILE()
    RETURN()
    EVT_END()

    EVT_BEGIN(shadoo_defeat_msg)
    SET(GSWF(1604), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(shadooDefeated), 0, PTR("me"))
    RUN_EVT(dan_70_new_rewards)
    RETURN_FROM_CALL()

    EVT_BEGIN(merluna_walmart)
    WAIT_FRM(1)
    RETURN()
    EVT_END()

    /*EVT_BEGIN(merluna_rehighlight)
    INLINE_EVT()
    USER_FUNC(evt_snd::evt_snd_bgmoff_f_d, 0, 1000)
    WAIT_MSEC(500)
    RUN_CHILD_EVT(custom_pit_music)
    END_INLINE()
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 128, 255, 1000)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(evt_map::evt_map_set_blend, 1, LW(0), LW(0), LW(0), 255)
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    RETURN()
    EVT_END()

    EVT_BEGIN(merluna_speech_2)
    // Screen darkens on first evt run
    IF_EQUAL(LW(14), 0)
    SET(LW(14), 1)
    USER_FUNC(merlunaCutsceneSetMaterialFlags)
    INLINE_EVT()
    USER_FUNC(evt_snd::evt_snd_bgmoff_f_d, 0, 1000)
    WAIT_MSEC(500)
    USER_FUNC(evt_snd::evt_snd_bgmon_f_d, 0, PTR("BGM_MINI_KOURA1"), 500)
    END_INLINE()
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 255, 128, 1200)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(evt_map::evt_map_set_blend, 1, LW(0), LW(0), LW(0), 255)
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    END_IF()
    // Merluna prepares judgement
    IF_EQUAL(GSWF(1669), 0)
    SET(GSWF(1669), 1)
    USER_FUNC(render_judgement, GSW(1603), LW(2), GSW(1604), LW(4), LW(5))
    WAIT_MSEC(500)
    END_IF()
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(merlunaJudgement), 0, PTR("me"), LW(2), LW(4))
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(merlunaOptions))
    USER_FUNC(evt_msg::evt_msg_continue)
    SWITCH(LW(0))
    CASE_EQUAL(0) // Chooses to receive Judgement
    USER_FUNC(evt_pouch::evt_pouch_get_coins, LW(10))
    IF_SMALL(LW(10), 10) // Checks if Mario is poor...
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaScam), 0, PTR("me"))
    INLINE_EVT()
    RUN_CHILD_EVT(merluna_rehighlight)
    END_INLINE()
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 1000, 11)
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario::evt_mario_key_on)
    ELSE() // ...but if he's not, Proceed tm
    USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
    WAIT_MSEC(500)
    USER_FUNC(evt_pouch::evt_pouch_add_coins, -10)
    USER_FUNC(evt_shop::evt_shop_wait_coin_sfx)
    WAIT_MSEC(500)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
    WAIT_MSEC(500)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(13), LW(14), LW(15))
    USER_FUNC(evt_mario::evt_mario_get_height, LW(14))
    DIVF(LW(14), 2)
    ADDF(LW(15), 10)
    SUBF(LW(13), 3)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 12, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(render_judgement_2, GSW(1603), GSW(1604), LW(5))
    IF_EQUAL(LW(5), 0) // Receives Blessing
    USER_FUNC(evt_snd::evt_snd_sfxon_3d_player, PTR("SFX_EVT_MAJINAI_EFFECT1"))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(13), LW(14), LW(15), FLOAT(5.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_11"), 0)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_EVT_QUIZ_7_SEIKAI1"), LW(13), LW(14), LW(15))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 13, 1)
    WAIT_MSEC(1000)
    INLINE_EVT()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    RUN_CHILD_EVT(merluna_rehighlight)
    END_INLINE()
    WAIT_MSEC(1300)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaBless), 0, PTR("me"))
    ELSE() // Receives Curse
    USER_FUNC(evt_snd::evt_snd_sfxon_3d_player, PTR("SFX_EVT_MAJINAI_EFFECT1"))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 9, LW(13), LW(14), LW(15), FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("D_2"), 0)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_EVT_QUIZ_FAULT1"), LW(13), LW(14), LW(15))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 13, 1)
    WAIT_MSEC(1000)
    INLINE_EVT()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    RUN_CHILD_EVT(merluna_rehighlight)
    END_INLINE()
    WAIT_MSEC(1300)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaCurse), 0, PTR("me"))
    END_IF()
    SWITCH_BREAK()
    CASE_EQUAL(3) // No thanks!
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaDecline), 0, PTR("me"))
    INLINE_EVT()
    RUN_CHILD_EVT(merluna_rehighlight)
    END_INLINE()
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 1000, 11)
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    SWITCH_BREAK()
    CASE_EQUAL(1) // About Blessing...
    SET(LW(6), PTR("me"))
    USER_FUNC(explain_judgement, GSW(1603), GSW(1604), 0, LW(6))
    WAIT_MSEC(250)
    RUN_CHILD_EVT(merluna_speech_2)
    RETURN()
    SWITCH_BREAK() // lol
    CASE_EQUAL(2)  // About Curse...
    SET(LW(6), PTR("me"))
    USER_FUNC(explain_judgement, GSW(1603), GSW(1604), 1, LW(6))
    WAIT_MSEC(250)
    RUN_CHILD_EVT(merluna_speech_2)
    RETURN()
    SWITCH_BREAK() // lol
    END_SWITCH()
    // Merluna disappears after judgement is dispensed
    INLINE_EVT()
    SET(GSWF(1669), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("rebear"), 9, PTR(merluna_walmart))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 28, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 29, 1)
    END_INLINE()
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 800, 11)
    WAIT_MSEC(800)
    USER_FUNC(evt_mario::evt_mario_key_on)
    WAIT_MSEC(500)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_delete, PTR("me"))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), 398, 0, 0)
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, 398, 0, 0, FLOAT(3.5), 0, 0, 0, 0, 0, 0, 0)
    RETURN()
    EVT_END()

    EVT_BEGIN(merluna_curse_speech)
    INLINE_EVT()
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 28, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 29, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 30, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    END_INLINE()
    WAIT_MSEC(2000)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaRelieveCurse), 0, PTR("me"))
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(merlunaRelieveOptions))
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_EQUAL(LW(0), 0)
    USER_FUNC(evt_pouch::evt_pouch_get_coins, LW(1))
    IF_SMALL(LW(1), 100) // Checks if Mario is poor...
    WAIT_MSEC(800)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaRelieveScam), 0, PTR("me"))
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 800, 11)
    WAIT_MSEC(800)
    USER_FUNC(evt_mario::evt_mario_key_on)
    ELSE() // ...but if he's not, Proceed tm
    USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
    WAIT_MSEC(500)
    USER_FUNC(evt_pouch::evt_pouch_add_coins, -100)
    USER_FUNC(evt_shop::evt_shop_wait_coin_sfx)
    WAIT_MSEC(500)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
    // CINEMATIC
    USER_FUNC(merlunaCutsceneSetMaterialFlags)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 255, 128, 1200)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(evt_map::evt_map_set_blend, 1, LW(0), LW(0), LW(0), 255)
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    WAIT_MSEC(500)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(13), LW(14), LW(15))
    USER_FUNC(evt_mario::evt_mario_get_height, LW(14))
    DIVF(LW(14), 2)
    ADDF(LW(15), 10)
    SUBF(LW(13), 3)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 12, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d_player, PTR("SFX_EVT_MAJINAI_EFFECT1"))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(13), LW(14), LW(15), FLOAT(5.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("I_2"), 0)
    SET(GSW(1601), 0)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 13, 1)
    WAIT_MSEC(1000)
    INLINE_EVT()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_init, 11, 128, 255, 1000)
    DO(0)
    USER_FUNC(evt_sub::evt_sub_intpl_msec_get_value)
    USER_FUNC(evt_map::evt_map_set_blend, 1, LW(0), LW(0), LW(0), 255)
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 0)
    DO_BREAK()
    END_IF()
    WHILE()
    END_INLINE()
    WAIT_MSEC(1300)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaRelieveReal), 0, PTR("me"))
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(merlunaRetryOptions))
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_EQUAL(LW(0), 0)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaRetryAccept), 0, PTR("me"))
    SET(LW(14), 0)
    RUN_CHILD_EVT(merluna_speech_2)
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaRetryDecline), 0, PTR("me"))
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 800, 11)
    WAIT_MSEC(800)
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_IF()
    END_IF()
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaRelieveDecline), 0, PTR("me"))
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 800, 11)
    WAIT_MSEC(800)
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_IF()
    RETURN()
    EVT_END()*/

    EVT_BEGIN(merluna_speech)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, FLOAT(237), FLOAT(101), FLOAT(644), FLOAT(340), FLOAT(50), FLOAT(200), 1000, 11)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(4), LW(5), LW(6))
    USER_FUNC(evt_mario::evt_mario_walk_to, 350, 0, 800)
    USER_FUNC(evt_mario::evt_mario_face_npc, PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), -1)
    // Placeholder
    WAIT_MSEC(2000)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(IMISSMYBROTHER), 0, PTR("me"))
    INLINE_EVT()
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("rebear"), 9, PTR(merluna_walmart))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 28, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 29, 1)
    END_INLINE()
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 800, 11)
    WAIT_MSEC(800)
    USER_FUNC(evt_item::evt_item_entry, PTR("IMISSYOU"), (s32)item_data::ITEM_ID_USE_SHINABITA_KINOKO, 0, 0, -1000, 0, 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("IMISSYOU"), 8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("IMISSYOU"))
    USER_FUNC(evt_mario::evt_mario_key_on)
    WAIT_MSEC(500)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_delete, PTR("me"))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), 398, 0, 0)
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, 398, 0, 0, FLOAT(3.5), 0, 0, 0, 0, 0, 0, 0)
    // End placeholder
    /*// Merluna first cutscene
    IF_EQUAL(GSWF(1600), 0)
    SET(GSWF(1600), 1)
    INLINE_EVT()
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 28, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 29, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 30, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    END_INLINE()
    WAIT_MSEC(2000)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(merlunaIntro), 0, PTR("me"))
    WAIT_MSEC(250)
    MARIO_SPAWN_QUESTION_MARK()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_10"), 0)
    WAIT_MSEC(1000)
    INLINE_EVT()
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 34, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    END_INLINE()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    WAIT_MSEC(600)
    USER_FUNC(evt_lp_get_difficulty, LW(15))
    SWITCH(LW(15))
    CASE_EQUAL(0)
    SET(LW(6), 90)
    CASE_EQUAL(1)
    SET(LW(6), 70)
    CASE_EQUAL(2)
    SET(LW(6), 50)
    END_SWITCH()
    SET(LW(7), 100)
    SUB(LW(7), LW(6))
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(merlunaIntro2), 0, PTR("me"), LW(6), LW(7))
    ELSE() // Not first cutscene
    WAIT_MSEC(1000)
    IF_LARGE(GSW(1601), 0)
    RUN_CHILD_EVT(merluna_curse_speech)
    RETURN()
    ELSE()
    SWITCH(LW(15))
    CASE_EQUAL(0)
    SET(LW(6), 90)
    CASE_EQUAL(1)
    SET(LW(6), 70)
    CASE_EQUAL(2)
    SET(LW(6), 50)
    END_SWITCH()
    SET(LW(7), 100)
    SUB(LW(7), LW(6))
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(merlunaIntro3), 0, PTR("me"), LW(6), LW(7))
    IF_EQUAL(GSWF(1669), 1)
    USER_FUNC(rerender_judgement, LW(1), LW(2), LW(3), LW(4), LW(5))
    END_IF()
    WAIT_MSEC(600)
    END_IF()
    END_IF()
    RUN_CHILD_EVT(merluna_speech_2)*/
    RETURN()
    EVT_END()

    EVT_BEGIN(fwd_merluna_speech)
    RUN_EVT(merluna_speech)
    RETURN()
    EVT_END()

    customwin::CWSelectColorDef boodinSelectBgCols[] = {
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{185, 200, 240, 255}, {185, 200, 240, 255}, 120, 120}, // Blue
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{185, 240, 240, 255}, {185, 240, 240, 255}, 120, 120}, // Cyan
    };

    EVT_BEGIN(boodin_speech)
    IF_EQUAL(LW(7), 0)
    USER_FUNC(evt_mario::evt_mario_key_off, 0)
    SET(LW(6), 0)
    USER_FUNC(dan_boodin_get_descs, LW(10), LW(11))
    USER_FUNC(EvtCWSelectEntry, PTR("Cards"), CWSELECT_SHOP, PTR(msgdrv::msgSearch("msg_window_title_4")), PTR(msgdrv::msgSearch("msg_window_select_4")), LW(10), LW(11))
    USER_FUNC(EvtCWSelectSetBGColor, PTR("Cards"), PTR(boodinSelectBgCols), 4)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("dan_card"), 0, 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(boodinIntro), 0, PTR("dan_card"))
    END_IF()
    USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
    USER_FUNC(EvtCWSelectMenuStart, PTR("Cards"), 0, LW(2)) // LW(4) item ID, LW(5) item name, LW(1) buy price
    USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
    IF_NOT_EQUAL(LW(2), -1)
    USER_FUNC(EvtCWSelectGetSelectionCost, LW(2), LW(1))
    USER_FUNC(EvtCWSelectGetSelectionName, LW(2), LW(5))
    USER_FUNC(EvtCWSelectGetSelectionItemId, LW(2), LW(4))
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(boodinItemSelected), 0, PTR("dan_card"), LW(5), LW(1))
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(boodinSelect))
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_EQUAL(LW(0), 0)
    USER_FUNC(evt_pouch::evt_pouch_get_coins, LW(3))
    IF_SMALL(LW(3), LW(1))
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(boodinClassism), 0, PTR("dan_card"))
    ELSE()
    IF_NOT_EQUAL(LW(2), (s32)item_data::ITEM_ID_USE_SUPER_BLANK_KUN)
    GOTO(81)
    END_IF()
    USER_FUNC(evt_pouch::evt_pouch_check_free_use_item, LW(3))
    IF_EQUAL(LW(3), 0)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(boodinNoSpace), 0, PTR("dan_card"))
    ELSE()
    LBL(81)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
    WAIT_MSEC(500)
    MUL(LW(1), -1)
    USER_FUNC(evt_pouch::evt_pouch_add_coins, LW(1))
    USER_FUNC(evt_shop::evt_shop_wait_coin_sfx)
    WAIT_MSEC(500)
    USER_FUNC(evt_item::evt_item_entry, PTR("card_item"), LW(4), 0, 0, -1000, 0, 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("card_item"), 8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("card_item"))
    USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    IF_LARGE(LW(2), 0)
    USER_FUNC(EvtCWSelectRemoveListing, PTR("Cards"), LW(2))
    USER_FUNC(dan_boodin_backup_descs)
    END_IF()
    // BUY ANOTHER?
    SET(LW(6), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(boodinWantMore), 0, PTR("dan_card"))
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(boodinSelect))
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_EQUAL(LW(0), 0)
    SET(LW(7), 1)
    RUN_CHILD_EVT(boodin_speech)
    RETURN()
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(boodinSatisfied), 0, PTR("dan_card"))
    END_IF()
    END_IF()
    END_IF()
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(boodinDecline), 0, PTR("dan_card"))
    END_IF()
    ELSE()
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(boodinDecline), 0, PTR("dan_card"))
    END_IF()
    USER_FUNC(EvtCWSelectDelete, PTR("Cards"))
    USER_FUNC(evt_mario::evt_mario_key_on)
    INLINE_EVT()
    IF_EQUAL(LW(6), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("dan_card"), 25, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("dan_card"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("dan_card"), 0, 1)
    ELSE()
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("dan_card"), 24, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("dan_card"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("dan_card"), 0, 1)
    END_IF()
    END_INLINE()
    RETURN()
    EVT_END()

    EVT_BEGIN(fwd_boodin_speech)
    RUN_EVT(boodin_speech)
    RETURN()
    EVT_END()

    EVT_BEGIN(gabbi_speech_deadge)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    SWITCH(GSW(1631))
    CASE_EQUAL(0)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiSleeping), 0, 0)
    CASE_EQUAL(1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiSleeping2), 0, 0)
    CASE_ETC()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiSleeping3), 0, 0)
    END_SWITCH()
    ADD(GSW(1631), 1) // If you really want to talk to Gabbi 256 times to overflow this byte for fun, I don't really care
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    EVT_END()

    EVT_BEGIN(fwd_gabbi_speech_deadge)
    RUN_EVT(gabbi_speech_deadge)
    RETURN()
    EVT_END()

    s32 GabbiGetKeyCost() {
        s32 cost = (75 + (25 * lpGetDifficulty()));
        if (VoucherChkTorn(VOUCHER_YELLOW) == true)
            cost *= 0.7f;
        return cost;
    }

    bool GabbiPressPlus(winmgr::WinmgrSelect * select) {
        (void)select;
        s32 cost = GabbiGetKeyCost();
        s32 keys = CWSelectGetActiveEntry()->Descs[0].cost / cost;
        s32 after_cost = ((keys + 1) * cost);
        if (after_cost > mario_pouch::pouchGetPtr()->coins) {
            spmario_snd::spsndSFXOn("SFX_SYS_SELECT_NG1");
        } else {
            spmario_snd::spsndSFXOn("SFX_SYS_FILE_MOJI_SET1");
            msl::string::memset(CWSelectGetActiveEntry()->Descs[0].nameTxt, 0, CWSELECT_NAME_TXT_LENGTH);
            msl::stdio::sprintf(CWSelectGetActiveEntry()->Descs[0].nameTxt, "Chest Keys (%d)", keys + 1);
            CWSelectGetActiveEntry()->Descs[0].cost = after_cost;
        }
        return true;
    }

    bool GabbiPressMinus(winmgr::WinmgrSelect * select) {
        (void)select;
        s32 cost = GabbiGetKeyCost();
        s32 keys = CWSelectGetActiveEntry()->Descs[0].cost / cost;
        s32 after_cost = ((keys - 1) * cost);
        if (after_cost <= 0) {
            spmario_snd::spsndSFXOn("SFX_SYS_SELECT_NG1");
        } else {
            spmario_snd::spsndSFXOn("SFX_SYS_FILE_MOJI_DELETE1");
            msl::string::memset(CWSelectGetActiveEntry()->Descs[0].nameTxt, 0, CWSELECT_NAME_TXT_LENGTH);
            msl::stdio::sprintf(CWSelectGetActiveEntry()->Descs[0].nameTxt, "Chest Keys (%d)", keys - 1);
            CWSelectGetActiveEntry()->Descs[0].cost = after_cost;
        }
        return true;
    }

    s32 evt_gabbi_get_key_cost(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], GabbiGetKeyCost());
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_gabbi_get_key_cost, 1)

    EVT_BEGIN(gabbi_menu)
    USER_FUNC(evt_gabbi_get_key_cost, LW(15))
    USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
    USER_FUNC(EvtCWSelectEntry, PTR("Gabbi"), CWSELECT_SHOP, PTR("Chest Keys"), PTR(gabbiSelectInstructions), 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Gabbi"), PTR("Chest Keys (1)"), PTR(chestKeyDesc), TPLPATCH_ICON(ICON_CHEST_KEY), LW(15), 0, 0)
    USER_FUNC(EvtCWSelectOverrideBtnBehavior, PTR("Gabbi"), BTN_PLUS, PTR(GabbiPressPlus))
    USER_FUNC(EvtCWSelectOverrideBtnBehavior, PTR("Gabbi"), BTN_MINUS, PTR(GabbiPressMinus))
    USER_FUNC(EvtCWSelectSetHeaderColor, PTR("Gabbi"), PTR(&GabbiHeaderCol))
    // USER_FUNC(EvtCWSelectSetBGColor, PTR("Gabbi"), PTR(GabbiSelectBgCols), 4)
    USER_FUNC(EvtCWSelectMenuStart, PTR("Gabbi"), 0, LW(0))
    USER_FUNC(EvtCWSelectGetSelectionCost, LW(0), GW(7)) // If cancelled, sets to -1
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("Gabbi"))
    RETURN()
    EVT_END()

    // Uses LW(10) as a parameter for next GSW(1630) (Gabbi emotional state)
    EVT_BEGIN(gabbi_anim_handler)
    IF_EQUAL(GSW(1630), LW(10))
    RETURN()
    END_IF()
    SWITCH(GSW(1630)) // Transition from current emotional state to neutral
    CASE_EQUAL((s32)GABBI_SAD)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 22, 1) // S_2C
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    CASE_EQUAL((s32)GABBI_BLUSH)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 22, 1) // S_3C
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    CASE_EQUAL((s32)GABBI_MAD)                             // Currently Mad
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 25, 1) // S_4C
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    END_SWITCH()
    SET(GSW(1630), LW(10))
    SWITCH(GSW(1630)) // Transition to next emotional state defined by LW(10)
    CASE_EQUAL((s32)GABBI_NEUTRAL)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Neutral))
    CASE_EQUAL((s32)GABBI_SAD)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Sadge))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 21, 1) // S_2A
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    CASE_EQUAL((s32)GABBI_BLUSH)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Blush))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 21, 1) // S_3A
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    CASE_EQUAL((s32)GABBI_MAD)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Madge))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 24, 1) // S_4A
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    END_SWITCH()
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, true)
    RETURN()
    EVT_END()

    EVT_BEGIN(gabbi_walkto)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("gabbi"), LW(3), 0, LW(5))
    USER_FUNC(evt_cam::evt_cam_get_pos, 5, LW(0), LW(1), LW(2))
    USER_FUNC(evt_cam::evt_cam_get_target, 5, LW(6), LW(7), LW(8))
    // Pan to right of Gabbi
    SET(LW(6), LW(3))
    ADD(LW(6), 20)
    SET(LW(0), LW(6))
    // Move slightly downward
    SUB(LW(1), 20)
    SUB(LW(7), 20)
    // Zoom
    ADD(LW(8), 50)
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, LW(0), LW(1), LW(2), LW(6), LW(7), LW(8), 1200, 11)
    WAIT_MSEC(200)
    ADD(LW(3), 40)
    USER_FUNC(evt_mario::evt_mario_walk_to, LW(3), LW(5), 800)
    USER_FUNC(evt_mario::evt_mario_face_npc, PTR("gabbi"))
    WAIT_MSEC(1000)
    RETURN()
    EVT_END()

    EVT_BEGIN(gabbi_main)
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(yesNoSelect_YesByDefault))
    SET(LW(2), GW(7))
    WAIT_MSEC(200)
    IF_EQUAL(LW(0), 0) // Yes
    USER_FUNC(evt_gabbi_get_key_cost, LW(15))
    USER_FUNC(evt_pouch::evt_pouch_get_coins, LW(3))
    IF_SMALL(LW(3), LW(15))
    SET(LW(10), (s32)GABBI_SAD)
    RUN_CHILD_EVT(gabbi_anim_handler)
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(gabbiBroke))
    GOTO(99)
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(gabbiSayYes))
    END_IF()
    RUN_CHILD_EVT(gabbi_menu)
    IF_NOT_EQUAL(GW(7), -1)
    SET(LW(7), GW(7))
    DIV(LW(7), LW(15))
    INLINE_EVT()
    WAIT_MSEC(200)
    IF_LARGE(LW(7), 3)
    SET(LW(10), (s32)GABBI_BLUSH)
    ELSE()
    SET(LW(10), (s32)GABBI_NEUTRAL)
    END_IF()
    RUN_CHILD_EVT(gabbi_anim_handler)
    END_INLINE()
    MUL(GW(7), -1)
    USER_FUNC(evt_pouch::evt_pouch_add_coins, GW(7))
    USER_FUNC(evt_shop::evt_shop_wait_coin_sfx)
    WAIT_MSEC(200)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 1)
    USER_FUNC(evt_lp_add_chest_keys, LW(7))
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(gabbiSysGetKeys), 0, 0, LW(7))
    SET(GW(7), LW(2))
    IF_EQUAL(LW(10), (s32)GABBI_BLUSH)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiThanks_B), 0, PTR("me"))
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiThanks_N), 0, PTR("me"))
    END_IF()
    USER_FUNC(evt_sub::evt_sub_hud_configure, 1)
    ELSE()
    USER_FUNC(evt_sub::evt_sub_hud_configure, 1)
    WAIT_MSEC(100)
    SET(LW(10), (s32)GABBI_SAD)
    RUN_CHILD_EVT(gabbi_anim_handler)
    WAIT_MSEC(100)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiBuyNothing), 0, PTR("me"))
    GOTO(99)
    END_IF()
    ELSE() // No
    SET(LW(10), (s32)GABBI_SAD)
    RUN_CHILD_EVT(gabbi_anim_handler)
    WAIT_MSEC(100)
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(gabbiSayNo))
    END_IF()
    LBL(99)
    SET(GW(7), LW(2))
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 700, 11)
    WAIT_MSEC(700)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
    USER_FUNC(evt_mario::evt_mario_key_on)
    SET(GW(7), 0)
    RETURN()
    EVT_END()

    EVT_BEGIN(gabbi_talk)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    RUN_EVT(gabbi_walkto)
    USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), 1)
    WAIT_MSEC(1500)
    SWITCH(GSW(1630))
    CASE_EQUAL((s32)GABBI_NEUTRAL)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiIntro_N), 0, PTR("me"))
    CASE_EQUAL((s32)GABBI_SAD)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiIntro_S), 0, PTR("me"))
    CASE_EQUAL((s32)GABBI_BLUSH)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiIntro_B), 0, PTR("me"))
    END_SWITCH()
    RUN_CHILD_EVT(gabbi_main)
    RETURN()
    EVT_END()

    EVT_BEGIN(fwd_gabbi_talk)
    RUN_EVT(gabbi_talk)
    RETURN()
    EVT_END()

    EVT_BEGIN(gabbi_hit)
    USER_FUNC(evt_npc::evt_npc_get_damage_type, PTR("me"), LW(0))
    IF_NOT_EQUAL(LW(0), 17) // Boomer
    USER_FUNC(evt_npc::evt_npc_restart_evt_id, PTR("gabbi"))
    RETURN()
    END_IF()
    // Set up post-cutscene properties
    SET(GSWF(1645), 1)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), 2, 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), 9, PTR(fwd_gabbi_talk))
    // Gabbi takes damage
    USER_FUNC(evt_npc::func_80108194, PTR("me"), 0)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 22, 1) // Y_1A
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_cam::evt_cam_look_at_door, 1, 0)
    RUN_EVT(gabbi_walkto)
    WAIT_MSEC(2000)
    // Gabbi gets fucking mad
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 23, 1) // Y_1B
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 24, 1) // S_4A
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1) // S_4B
    USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), 1)
    WAIT_MSEC(500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiMadge), 0, PTR("me"))
    WAIT_MSEC(1000)
    // Gabbi apologizes
    SET(GSW(1630), (s32)GABBI_MAD)
    SET(LW(10), (s32)GABBI_SAD)
    RUN_CHILD_EVT(gabbi_anim_handler)
    WAIT_MSEC(100)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiSorry), 0, PTR("me"))
    SET(LW(10), (s32)GABBI_NEUTRAL)
    RUN_CHILD_EVT(gabbi_anim_handler)
    WAIT_MSEC(100)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(gabbiIntro), 0, PTR("me"))
    RUN_CHILD_EVT(gabbi_main)
    RETURN()
    EVT_END()

    // MITCH

    s32 evt_mitch_setup_inv(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        Lunatic->Mitch.itemNum = system::rand() % 3 + 3; // 3-5
        Lunatic->Mitch.Items = (CWSelectItemDesc *)memory::__memAlloc(memory::HEAP_MAP, sizeof(CWSelectItemDesc) * Lunatic->Mitch.itemNum);
        msl::string::memset(Lunatic->Mitch.Items, 0, sizeof(CWSelectItemDesc) * Lunatic->Mitch.itemNum);
        s32 itemId = 0, itemRarity = 0, odds = 0;
        msl::string::memset(Lunatic->Mitch.itemIds, 0, sizeof(Lunatic->Mitch.itemIds));
        RFCItemData * RFC_SpecialItems = (RFCItemData *)RFCSpecialGetPtr();
        RFCColorDef * RFC_Colors = (RFCColorDef *)RFCColorsGetPtr();
        for (s32 i = 0; i < Lunatic->Mitch.itemNum; i += 1) {
            if (itemRarity != 3) {
                odds = system::rand() % 100;
                // 6% chance to select a higher-tier item
                if (odds < 6)
                    itemRarity += 1;
            }
            itemId = DanGen_Items_SelectItem(itemRarity, Lunatic->Mitch.itemIds, 6, true);
            if (itemId == -1) {
                if (itemRarity != 3) { // If all items from this rarity are taken OR item rarity has been upgraded, go up a tier
                    i -= 1;
                    itemRarity += 1;
                    continue;
                } else { // Failsafe: if all Legendary items are taken, simply cut offerings short
                    Lunatic->Mitch.itemNum = i;
                    break;
                }
            } else if (itemId == Lunatic->RFC.rfcItemIds[0] || itemId == Lunatic->RFC.rfcItemIds[1] || itemId == Lunatic->RFC.rfcItemIds[2]) {
                i -= 1;
                continue;
            }
            Lunatic->Mitch.itemIds[i] = itemId;
            CWSelectItemDesc * Item = &Lunatic->Mitch.Items[i];
            Item->itemId = 0;
            Item->iconId = TPLPATCH_ICON((s32)RFC_SpecialItems[itemId - RFC_SPECIAL_START].iconId);
            Item->cost = RFC_SpecialItems[itemId - RFC_SPECIAL_START].buyPrice;
            Item->nameColor = RFC_Colors[itemRarity].textCol;
            msl::string::memcpy(Item->nameTxt, RFC_SpecialItems[itemId - RFC_SPECIAL_START].name, msl::string::strlen(RFC_SpecialItems[itemId - RFC_SPECIAL_START].name));
            if ((itemId - RFC_SPECIAL_START) >= VOUCHER_CAKE && (itemId - RFC_SPECIAL_START) <= VOUCHER_BLACK) // Check if voucher
                msl::stdio::sprintf(Item->descTxt, RFC_SpecialItems[itemId - RFC_SPECIAL_START].description, VoucherGetTearChance(VoucherTearChances[itemId - RFC_SPECIAL_START]), VoucherGuaranteeTrigs[itemId - RFC_SPECIAL_START]);
            else
                msl::string::memcpy(Item->descTxt, RFC_SpecialItems[itemId - RFC_SPECIAL_START].description, msl::string::strlen(RFC_SpecialItems[itemId - RFC_SPECIAL_START].description));
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_mitch_setup_inv, 0)

    s32 evt_mitch_get_descs(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)Lunatic->Mitch.Items);
        evtmgr_cmd::evtSetValue(evtEntry, args[1], Lunatic->Mitch.itemNum);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_mitch_get_descs, 2)

    s32 evt_mitch_backup_descs(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)evtEntry;
        (void)firstRun;
        s32 id = customwin::CWSelectKeyToId("Mitch");
        Lunatic->Mitch.itemNum = GlobalCW->Select[id]->num;
        msl::string::memset(Lunatic->Mitch.Items, 0, (sizeof(CWSelectItemDesc) * Lunatic->Mitch.itemNum));
        msl::string::memcpy(Lunatic->Mitch.Items, GlobalCW->Select[id]->Descs, (sizeof(CWSelectItemDesc) * Lunatic->Mitch.itemNum));
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_mitch_backup_descs, 0)

    customwin::CWSelectColorDef MitchSelectBgCols[] = {
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{228, 217, 255, 255}, {228, 217, 255, 255}, 120, 120}, // Orange
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{248, 196, 255, 255}, {248, 196, 255, 255}, 120, 120}, // Yellow
    };

    EVT_BEGIN(mitch_menu)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
    USER_FUNC(evt_mitch_get_descs, LW(6), LW(7))
    USER_FUNC(EvtCWSelectEntry, PTR("Mitch"), CWSELECT_SHOP, PTR("\"Stuff\""), PTR("I'm stuff... ;)"), LW(6), LW(7))
    USER_FUNC(EvtCWSelectSetShopIcon, PTR("Mitch"), (s32)TPLPATCH_ICON(ICON_CHEST_KEY), FLOAT(0.8))
    USER_FUNC(EvtCWSelectSetHeaderColor, PTR("Mitch"), PTR(&MitchHeaderCol))
    USER_FUNC(EvtCWSelectSetBGColor, PTR("Mitch"), PTR(MitchSelectBgCols), 4)
    USER_FUNC(EvtCWSelectMenuStart, PTR("Mitch"), 0, GW(6))
    IF_NOT_EQUAL(GW(6), -1)
    USER_FUNC(EvtCWSelectGetSelectionCost, GW(6), GW(7))
    SET(GW(9), GW(6))
    USER_FUNC(RFCAnalyzeSpecial, GW(6), GW(8))
    END_IF()
    RETURN()
    EVT_END()

    EVT_BEGIN(mitch_walkto)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("mitch"), LW(3), 0, LW(5))
    USER_FUNC(evt_cam::evt_cam_get_pos, 5, LW(0), LW(1), LW(2))
    USER_FUNC(evt_cam::evt_cam_get_target, 5, LW(6), LW(7), LW(8))
    // Pan to right of Mitch
    SET(LW(6), LW(3))
    ADD(LW(6), 20)
    SET(LW(0), LW(6))
    // Move slightly downward
    SUB(LW(1), 20)
    SUB(LW(7), 20)
    // Zoom
    ADD(LW(8), 50)
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, LW(0), LW(1), LW(2), LW(6), LW(7), LW(8), 1200, 11)
    WAIT_MSEC(200)
    ADD(LW(3), 40)
    USER_FUNC(evt_mario::evt_mario_walk_to, LW(3), LW(5), 800)
    USER_FUNC(evt_mario::evt_mario_face_npc, PTR("mitch"))
    WAIT_MSEC(1000)
    RETURN()
    EVT_END()

    EVT_BEGIN(mitch_main)
    RUN_CHILD_EVT(mitch_menu)
    IF_NOT_EQUAL(GW(6), -1)
    USER_FUNC(evt_lp_get_chest_keys, LW(8))
    IF_SMALL(LW(8), GW(7))
    USER_FUNC(evt_sub::evt_sub_hud_configure, 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(mitchPoor), 0, PTR("me"))
    GOTO(99)
    END_IF()
    MUL(GW(7), -1)
    USER_FUNC(evt_lp_add_chest_keys, GW(7))
    WAIT_MSEC(300)
    USER_FUNC(EvtCWSelectRemoveListing, PTR("Mitch"), GW(9))
    USER_FUNC(evt_mitch_backup_descs)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(mitchThanks), 0, PTR("me"))
    USER_FUNC(LPGUIShowHideStats, 1)
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(1), LW(2), LW(3))
    USER_FUNC(RFCProcEffect, GW(6))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_I_HEART_HUERU1"), LW(1), LW(2), LW(3))
    INLINE_EVT()
    WAIT_MSEC(200)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("I_2"), 0)
    END_INLINE()
    WAIT_MSEC(1000)
    IF_NOT_EQUAL(GSWF(1606), 1) // Voucher intro text not seen
    IF_LARGE_EQUAL(GW(6), (s32)VOUCHER_CAKE)
    IF_SMALL_EQUAL(GW(6), (s32)VOUCHER_BLACK)
    SET(GSWF(1606), 1)
    USER_FUNC(MsgIconReplaceIdx, 3, (s32)(TPLPATCH_ICON(ICON_B)))
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(voucherIntro), 0, 0)
    USER_FUNC(MsgIconReplaceIdx, 3, (s32)icondrv::ICON_BTN_1)
    END_IF()
    END_IF()
    END_IF()
    IF_NOT_EQUAL(GW(8), -1) // Dialogue
    USER_FUNC(evt_msg::evt_msg_print, 1, GW(8), 0, 0)
    END_IF()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    USER_FUNC(LPGUIShowHideStats, 0)
    WAIT_MSEC(1000)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(mitchEnjoy), 0, PTR("me"))
    ELSE()
    USER_FUNC(evt_sub::evt_sub_hud_configure, 1)
    WAIT_MSEC(500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(mitchBuyNothing), 0, PTR("me"))
    END_IF()
    LBL(99)
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("Mitch"))
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 700, 11)
    WAIT_MSEC(700)
    USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
    USER_FUNC(evt_mario::evt_mario_key_on)
    SET(GW(6), 0)
    SET(GW(7), 0)
    SET(GW(8), 0)
    SET(GW(9), 0)
    RETURN()
    EVT_END()

    EVT_BEGIN(mitch_talk)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_mitch_get_descs, 0, LW(7))
    IF_EQUAL(LW(7), 0)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(mitchNoInventory), 0, PTR("me"))
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    END_IF()
    RUN_EVT(mitch_walkto)
    USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), 1)
    WAIT_MSEC(1500)
    IF_EQUAL(GSWF(1646), 0)
    SET(GSWF(1646), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(mitchIntro_1), 0, PTR("me"))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d_player_character, PTR("SFX_P_V_MARIO_GIMON1"), PTR("SFX_P_V_PEACH_GIMON1"), PTR("SFX_P_V_KOOPA_GIMON1"), PTR("SFX_P_V_LUIGI_GIMON1"))
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_2"), 0)
    USER_FUNC(evt_mario::evt_mario_wait_anim)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    WAIT_MSEC(200)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(mitchIntro_2), 0, PTR("me"))
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(mitchIntro_B), 0, PTR("me"))
    END_IF()
    RUN_CHILD_EVT(mitch_main)
    RETURN()
    EVT_END()

    EVT_BEGIN(fwd_mitch_talk)
    RUN_EVT(mitch_talk)
    RETURN()
    EVT_END()

    s32 evt_dan_patch_dokan(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)evtEntry;
        (void)firstRun;
        dan::dan_chestRoomDokanDesc.destMapName = "mac_05";
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_dan_patch_dokan, 0)

    EVT_BEGIN(dan_chest_close_evt)
    USER_FUNC(evt_mobj::evt_mobj_get_position, PTR(rfcChestName), LW(0), LW(1), LW(2))
    /*ADDF(LW(1), 15)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(6.0), 0, 0, 0, 0, 0, 0, 0)*/
    USER_FUNC(evt_mobj::evt_mobj_delete, PTR(rfcChestName))
    USER_FUNC(RFCGenerate, 0, 0x42069) // Magic number necessary since this needs to call new_dan_chest_open_evt, but that evt needs to call this one too
    USER_FUNC(evt_npc::evt_npc_unfreeze_all)
    RETURN()
    EVT_END()

    customwin::CWSelectColorDef rfcSelectBgCols[] = {
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{255, 220, 180, 255}, {255, 220, 180, 255}, 120, 120}, // Orange
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{255, 250, 200, 255}, {255, 250, 200, 255}, 120, 120}, // Yellow
    };

    EVT_BEGIN(new_dan_chest_open_evt)
    USER_FUNC(evt_mobj::evt_mobj_wait_animation_end, PTR(rfcChestName), 0)
    USER_FUNC(RFCGetPtr, LW(1))
    USER_FUNC(EvtCWSelectEntry, PTR("RFC"), CWSELECT_DEFAULT, PTR("Loot"), PTR("Pick an item,\nany item!"), LW(1), 3)
    USER_FUNC(EvtCWSelectSetHeaderColor, PTR("RFC"), PTR(&RFCHeaderCol))
    USER_FUNC(EvtCWSelectSetBGColor, PTR("RFC"), PTR(rfcSelectBgCols), 4)
    USER_FUNC(EvtCWSelectMenuStart, PTR("RFC"), 0, LW(0))
    IF_EQUAL(LW(0), -1) // Select menu cancelled
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("RFC"))
    RUN_EVT(dan_chest_close_evt)
    RETURN()
    END_IF()
    USER_FUNC(EvtCWSelectGetSelectionItemId, LW(0), LW(4))
    IF_LARGE(LW(4), 0)
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("RFC"))
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(1), LW(2), LW(3))
    USER_FUNC(evt_item::evt_item_entry, PTR("item"), LW(4), 0, LW(1), LW(2), LW(3), 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("item"), 0x8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("item"))
    ELSE() // Is special item
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(1), LW(2), LW(3))
    USER_FUNC(RFCAnalyzeSpecial, LW(0), LW(5)) // Takes cwselect item idx, returns RFCCustomItem idx and msg id
    IF_SMALL(LW(0), (s32)ARTIFACT_SOUL)
    IF_LARGE(LW(0), (s32)VOUCHER_BLACK)
    USER_FUNC(LPGUIShowHideStats, 1)
    WAIT_MSEC(500)
    END_IF()
    END_IF()
    WAIT_MSEC(500)
    USER_FUNC(RFCProcEffect, LW(0))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_I_HEART_HUERU1"), LW(1), LW(2), LW(3))
    INLINE_EVT()
    WAIT_MSEC(200)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("I_2"), 0)
    END_INLINE()
    WAIT_MSEC(1500)
    IF_NOT_EQUAL(GSWF(1606), 1) // Voucher intro text not seen
    IF_LARGE_EQUAL(LW(0), (s32)VOUCHER_CAKE)
    IF_SMALL_EQUAL(LW(0), (s32)VOUCHER_BLACK)
    SET(GSWF(1606), 1)
    USER_FUNC(MsgIconReplaceIdx, 3, (s32)(TPLPATCH_ICON(ICON_B)))
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(voucherIntro), 0, 0)
    USER_FUNC(MsgIconReplaceIdx, 3, (s32)icondrv::ICON_BTN_1)
    END_IF()
    END_IF()
    END_IF()
    IF_NOT_EQUAL(LW(5), -1) // Dialogue
    USER_FUNC(evt_msg::evt_msg_print, 1, LW(5), 0, 0)
    END_IF()
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("RFC"))
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    IF_SMALL(LW(0), (s32)ARTIFACT_SOUL)
    IF_LARGE(LW(0), (s32)VOUCHER_BLACK)
    USER_FUNC(LPGUIShowHideStats, 0)
    END_IF()
    END_IF()
    END_IF()
    RETURN()
    EVT_END()

    EVT_BEGIN(dan_chest_poor_on_keys_evt)
    USER_FUNC(evt_sub::evt_sub_random, 3, LW(5))
    ADD(LW(5), 2)
    MUL(LW(5), 1000)
    WAIT_MSEC(LW(5))
    MARIO_SPAWN_QUESTION_MARK()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_10"), 0)
    WAIT_MSEC(1500)
    USER_FUNC(RFCBakudan)
    USER_FUNC(evt_mobj::evt_mobj_get_position, PTR(rfcChestName), LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_NINJA_BOMB_FIRE1"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("spm_explosion"), 0, LW(0), LW(1), LW(2), FLOAT(1.5), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_mobj::evt_mobj_delete, PTR(rfcChestName))
    USER_FUNC(evt_npc::evt_npc_unfreeze_all)
    RETURN()
    EVT_END()

    EVT_BEGIN(new_dan_chest_interact_evt)
    LBL(99)
    USER_FUNC(RFCGetRarity, 0, LW(0))
    USER_FUNC(RFCGetChestKeyParams, LW(1), LW(2), LW(3)) // Cost to open, chest keys owned, cost to reroll
    USER_FUNC(MsgIconReplaceIdx, 3, (s32)(TPLPATCH_ICON(ICON_CHEST_KEY)))
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(chestText), 0, 0, LW(0), LW(2), LW(1), LW(3))
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(chestOptions))
    USER_FUNC(MsgIconReplaceIdx, 3, (s32)icondrv::ICON_BTN_1)
    USER_FUNC(DebugModeGetStatus, LW(4))
    SWITCH(LW(0))
    CASE_EQUAL(0) // Open it!
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_EQUAL(LW(4), 1)
    RETURN()
    END_IF()
    IF_SMALL(LW(2), LW(1))
    RUN_CHILD_EVT(dan_chest_poor_on_keys_evt)
    RETURN()
    END_IF()
    SUB(LW(2), LW(1))
    USER_FUNC(RFCSetChestKeys, LW(2))
    CASE_EQUAL(1) // Close
    USER_FUNC(evt_msg::evt_msg_continue)
    USER_FUNC(evt_mobj::evt_mobj_delete, PTR(rfcChestName))
    USER_FUNC(RFCGenerate, PTR(new_dan_chest_interact_evt), PTR(new_dan_chest_open_evt))
    WAIT_MSEC(100)
    USER_FUNC(evt_mario::evt_mario_key_on)
    CASE_EQUAL(2) // Reroll
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_NOT_EQUAL(LW(4), 1)
    IF_SMALL(LW(2), LW(3))
    RUN_CHILD_EVT(dan_chest_poor_on_keys_evt)
    RETURN()
    END_IF()
    SUB(LW(2), LW(3))
    USER_FUNC(RFCSetChestKeys, LW(2))
    END_IF()
    WAIT_MSEC(1000)
    USER_FUNC(evt_mobj::evt_mobj_get_position, PTR(rfcChestName), LW(0), LW(1), LW(2))
    ADDF(LW(1), 15)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(6.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_mobj::evt_mobj_delete, PTR(rfcChestName))
    USER_FUNC(RFCReroll)
    USER_FUNC(RFCGenerate, PTR(new_dan_chest_interact_evt), PTR(new_dan_chest_open_evt))
    USER_FUNC(evt_npc::evt_npc_unfreeze_all)
    WAIT_MSEC(500)
    USER_FUNC(evt_mario::evt_mario_key_on)
    CASE_EQUAL(3) // Help
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(rfcHelp))
    GOTO(99)
    END_SWITCH()
    RETURN()
    EVT_END()

    EVT_BEGIN(whacka_hit)
    USER_FUNC(evt_npc::func_80108194, PTR("dan_koburon"), 0)
    USER_FUNC(evt_npc::evt_npc_get_damage_type, PTR("me"), LW(0))
    SWITCH(LW(0))
    CASE_EQUAL(1)
    CASE_EQUAL(2)
    CASE_EQUAL(10)
    CASE_EQUAL(4)
    CASE_EQUAL(5)
    CASE_EQUAL(17)
    CASE_ETC()
    USER_FUNC(evt_npc::evt_npc_restart_evt_id, PTR("dan_koburon"))
    RETURN()
    END_SWITCH()
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    IF_EQUAL(GSWF(1640), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), 2, 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), 9, 0)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 4, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
    ADD(LW(1), 30)
    ADD(LW(2), 10)
    USER_FUNC(evt_item::evt_item_entry, PTR("bump"), 106, 0, LW(0), LW(1), LW(2), 1, 0, 0, 0)
    WAIT_MSEC(500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaOww), 0, PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    SET(GSWF(1640), 1)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_KOBURON_IN1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 26, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("me"), 0, -1000, 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_koburon"), 1, 0)
    ELSE()
    WAIT_MSEC(500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaCantHurt), 0, PTR("me")) // You can't hurt me, I'm bluuuue!
    WAIT_MSEC(500)
    USER_FUNC(evt_mario::evt_mario_get_character, LW(0))
    USER_FUNC(evt_guide::evt_guide_get_can_search, LW(1))
    SWITCH(LW(0))
    CASE_EQUAL(0)
    IF_EQUAL(LW(1), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(tippiHitWhacka), 0,
              PTR("__guide__")) // I don't think being extra blue makes you invincible...
    END_IF()
    CASE_EQUAL(1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(peachHitWhacka), 0, PTR("__mario__"))
    CASE_EQUAL(2)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(bowserHitWhacka), 0, PTR("__mario__"))
    CASE_EQUAL(3)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(luigiHitWhacka), 0, PTR("__mario__"))
    END_SWITCH()
    IF_EQUAL(LW(1), 1)
    WAIT_MSEC(500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaCantHurt2), 0, PTR("me")) // Oh, really?
    END_IF()
    WAIT_MSEC(200)
    END_IF()
    USER_FUNC(evt_mario::evt_mario_key_on)
    USER_FUNC(evt_npc::evt_npc_restart_evt_id, PTR("dan_koburon"))
    RETURN()
    EVT_END()

    EVT_BEGIN(whacka_move)
    USER_FUNC(evt_npc::func_80108194, PTR("me"), 0)
    USER_FUNC(evt_npc::evt_npc_set_move_mode, PTR("me"), 1)
    DO(0)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    SET(LW(3), 75)
    SET(LW(4), 25)
    SET(LW(5), FLOAT(-87.5))
    USER_FUNC(evt_sub::evt_sub_get_dist, LW(0), 0, LW(3), 0, LW(1))
    IF_LARGE(LW(1), 100)
    IF_EQUAL(GSWF(384), 1)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_KOBURON_IN1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 26, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("me"), 0, -1000, 0)
    SET(GSWF(384), 0)
    END_IF()
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("me"), LW(3), -1000, LW(5))
    ELSE()
    IF_EQUAL(GSWF(384), 0)
    IF_EQUAL(GSWF(1640), 1)
    USER_FUNC(evt_npc::evt_npc_set_color, PTR("dan_koburon"), 60, 100, 255, 255)
    END_IF()
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("me"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_KOBURON_OUT1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 25, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    SET(GSWF(384), 1)
    END_IF()
    END_IF()
    WAIT_FRM(5)
    WHILE()
    RETURN()
    EVT_END()

    EVT_BEGIN(whacka_speech)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    IF_EQUAL(GSWF(1640), 0) // Checks if you've hit Whacka in the Pit
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaHi), 0, PTR("me"))
    ELSE()
    IF_EQUAL(GSWF(1641), 0) // Whacka hasn't given you the Mysterious Bump
    IF_EQUAL(GSWF(1642), 0) // Checks if you've talked to him after hitting him once already
    SET(GSWF(1642), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaHiBlueLong1), 0, PTR("me")) // Long
    USER_FUNC(evt_mario::evt_mario_get_character, LW(0))
    USER_FUNC(evt_guide::evt_guide_get_can_search, LW(1))
    SWITCH(LW(0))
    CASE_EQUAL(0)
    IF_EQUAL(LW(1), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(tippiBlueWhacka), 0, PTR("__guide__"))
    END_IF()
    CASE_EQUAL(1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(peachBlueWhacka), 0, PTR("__mario__"))
    CASE_EQUAL(2)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(bowserBlueWhacka), 0, PTR("__mario__"))
    CASE_EQUAL(3)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(luigiBlueWhacka), 0, PTR("__mario__"))
    END_SWITCH()
    IF_EQUAL(LW(1), 1)
    WAIT_MSEC(500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaHiBlueLong2), 0, PTR("me"))
    END_IF()
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaHiBlueShort), 0, PTR("me")) // Short
    END_IF()
    USER_FUNC(evt_pouch::evt_pouch_check_have_item, 106, LW(0)) // Checks if Mario has a Whacka Bump
    IF_EQUAL(LW(0), 1)
    USER_FUNC(evt_sub::evt_sub_item_select_menu, 0, PTR(&whackaItems), LW(0), 0)
    IF_EQUAL(LW(0), 106)
    USER_FUNC(evt_pouch::evt_pouch_remove_item, 106)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaThanks), 0, PTR("me"))
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_KOBURON_IN1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 26, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("me"), 75, -100, -90)
    USER_FUNC(evt_snd::evt_snd_bgmoff_f_d, 0, 1000)
    WAIT_MSEC(3000)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_EVT_DOKAN_MAKE1"), PTR("me"))
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_10"), 0)
    WAIT_MSEC(1000)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_P_V_PEACH_PANIC1"), PTR("me"))
    WAIT_MSEC(250)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_11"), 0)
    WAIT_MSEC(750)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_BS_DMN_FLAME_BOMB1"), PTR("me"))
    WAIT_MSEC(400)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("E_5"), 0)
    USER_FUNC(evt_mario::evt_mario_wait_anim)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_7"), 0)
    WAIT_MSEC(1000)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_BS_DMN_FLAME_BOMB1"), PTR("me"))
    WAIT_MSEC(4000)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("me"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_KOBURON_OUT1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 25, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 0, 1)
    WAIT_MSEC(500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaGiveBump1), 0, PTR("me"))
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    USER_FUNC(evt_item::evt_item_entry, PTR("i1"), 45, 0, LW(0), LW(1), LW(2), 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("i1"), 8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("i1"))
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    RUN_EVT(custom_pit_music)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaGiveBump2), 0, PTR("me"))
    SET(GSWF(1641), 1)
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaHmph), 0, PTR("me"))
    END_IF()
    ELSE()
    WAIT_MSEC(1500)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaDontHaveBump), 0, PTR("me"))
    END_IF()
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(whackaHiBluePost), 0, PTR("me"))
    END_IF()
    END_IF()
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    EVT_END()

    EVT_BEGIN(spawn_whacka)
    SET(GSWF(384), 0)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("dan_koburon"), PTR("n_koburon"), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_koburon"), 14, PTR(&temp_unk::whacka_anim_defs))
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_koburon"), 9, PTR(whacka_speech))
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_koburon"), 2, PTR(whacka_hit))
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_koburon"), 1, PTR(whacka_move))
    USER_FUNC(evt_npc::evt_npc_set_move_mode, PTR("dan_koburon"), 1)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("dan_koburon"), 75, -1000, FLOAT(-87.5))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("dan_koburon"), 26, 1)
    USER_FUNC(evt_npc::evt_npc_flag8_onoff, PTR("dan_koburon"), 1, 0x4410004)
    USER_FUNC(evt_npc::evt_npc_animflag_onoff, PTR("dan_koburon"), 1, 0x20)
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("dan_koburon"))
    USER_FUNC(evt_npc::func_80108194, PTR("dan_koburon"), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_koburon"), 11, 20)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_koburon"), 10, 20)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_koburon"), 12, 20)
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("dan_koburon"), -1, 15, PTR(&temp_unk::whacka_defenses))
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("dan_koburon"), 1, 11, 20)
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("dan_koburon"), 1, 10, 20)
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("dan_koburon"), 1, 12, 20)
    USER_FUNC(evt_npc::evt_npc_restart_evt_id, PTR("dan_koburon"))
    RETURN()
    EVT_END()

    EVT_BEGIN(spawn_flimm)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("roten"), PTR("n_machi_roten"), 0)
    USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("roten"), -1)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("roten"), 14, PTR(&dan::dan_rotenTribeAnimDefs))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("roten"), 0, 1)
    USER_FUNC(evt_npc::evt_npc_flag8_onoff, PTR("roten"), 1, 0x4400004)
    USER_FUNC(evt_npc::evt_npc_animflag_onoff, PTR("roten"), 1, 32)
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("roten"))
    USER_FUNC(evt_npc::evt_npc_set_move_mode, PTR("roten"), 1)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("roten"), 398, 0, 0)
    USER_FUNC(evt_npc::func_80108194, PTR("roten"), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("roten"), 11, 40)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("roten"), 10, 60)
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("roten"), -1, 11, 40)
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("roten"), -1, 10, 60)
    USER_FUNC(evt_shop::evt_shop_set_defs, PTR(&dan::dan_rotenShopDef), 1) // redo this entirely soon tm
    RETURN()
    EVT_END()

    EVT_BEGIN(spawn_boodin)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("dan_card"), PTR("n_machi_c_card"), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_card"), 14, PTR(boodinAnims))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("dan_card"), 0, 1)
    USER_FUNC(evt_npc::evt_npc_flag8_onoff, PTR("dan_card"), 1, 0x4400004)
    USER_FUNC(evt_npc::evt_npc_animflag_onoff, PTR("dan_card"), 1, 32)
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("dan_card"))
    USER_FUNC(evt_npc::evt_npc_set_move_mode, PTR("dan_card"), 1)
    USER_FUNC(evt_npc::func_80108194, PTR("dan_card"), 0)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("dan_card"), 398, 0, 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("dan_card"), 9, PTR(fwd_boodin_speech))
    USER_FUNC(dan_boodin_setup_cards)
    RETURN()
    EVT_END()

    EVT_BEGIN(spawn_merluna)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("rebear"), PTR("n_rebear"), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("rebear"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(merlunaAnims))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("rebear"), 0, true)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("rebear"), 398, 0, 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("rebear"), 9, PTR(fwd_merluna_speech))
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("rebear"))
    RETURN()
    EVT_END()

    EVT_BEGIN(spawn_mover)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("mover"), PTR("n_stg2_syuuzin_b"), 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("mover"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(moverAnims))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("mover"), 0, true)
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("mover"))
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("mover"), 200, 0, 0)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("mover"), 9, PTR(fwd_mover_speech))
    RETURN()
    EVT_END()

    EVT_BEGIN(spawn_mitch)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("mitch"), PTR("n_dan_mitch"), 0)
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("mitch"))
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("mitch"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(mitchAnims))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("mitch"), 0, true)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("mitch"), 9, PTR(fwd_mitch_talk))
    USER_FUNC(evt_npc::func_80108194, PTR("mitch"), 0)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("mitch"), 200, 0, -100)
    USER_FUNC(evt_mitch_setup_inv)
    RETURN()
    EVT_END()

    EVT_BEGIN(spawn_gabbi)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("gabbi"), PTR("n_dan_gabbi"), 0)
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("gabbi"))
    IF_EQUAL(GSWF(1645), 0)
    USER_FUNC(evt_mobj::evt_mobj_check, PTR(rfcChestName), LW(0)) // Check if Whacka exists
    IF_EQUAL(LW(0), 0)
    SET(GSW(1624), 0) // Set slot 2 NPC to null
    USER_FUNC(evt_npc::evt_npc_delete, PTR("gabbi"))
    RETURN()
    END_IF()
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Madge))
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), 2, PTR(gabbi_hit))
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), 9, PTR(fwd_gabbi_speech_deadge))
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), 11, 20)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), 10, 20)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), 12, 20)
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("gabbi"), -1, 15, PTR(&temp_unk::whacka_defenses))
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("gabbi"), 1, 11, 20)
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("gabbi"), 1, 10, 20)
    USER_FUNC(evt_npc::evt_npc_modify_part, PTR("gabbi"), 1, 12, 20)
    USER_FUNC(evt_npc::evt_npc_restart_evt_id, PTR("gabbi"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("gabbi"), 21, true)
    ELSE()
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), 9, PTR(fwd_gabbi_talk))
    SWITCH(GSW(1630))
    CASE_EQUAL((s32)GABBI_NEUTRAL)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Neutral))
    CASE_EQUAL((s32)GABBI_SAD)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Sadge))
    CASE_EQUAL((s32)GABBI_BLUSH)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Blush))
    CASE_EQUAL((s32)GABBI_MAD)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("gabbi"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(gabbiAnims_Madge))
    END_SWITCH()
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("gabbi"), 0, true)
    END_IF()
    USER_FUNC(evt_npc::func_80108194, PTR("gabbi"), 0)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("gabbi"), 200, 0, -100)
    RETURN()
    EVT_END()

    s32 small_chest_get_params(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
    tryAgain:
        s32 rand = system::irand(99);
        if (rand < 50) { // 50% chance to give 15-30 coins
            if (mario_pouch::pouchGetCoin() > 200)
                goto tryAgain;
            evtmgr_cmd::evtSetValue(evtEntry, args[0], 0);
            rand = system::irand(15);
            evtmgr_cmd::evtSetValue(evtEntry, args[1], 15 + rand);
        } else if (rand < 70) { // 20% chance to give 10, 15, or 20 HP
            if (mario_pouch::pouchGetMaxHp() == mario_pouch::pouchGetHp())
                goto tryAgain;
            evtmgr_cmd::evtSetValue(evtEntry, args[0], 1);
            s32 hp = 10;
            rand = system::irand(99);
            if (rand > 50 && rand < 85) {
                hp += 5;
            } else if (rand >= 85) {
                hp += 10;
            }
            evtmgr_cmd::evtSetValue(evtEntry, args[1], hp);
        } else if (rand < 85) { // 15% chance to drop a random Common/Uncommon use item
            if (mario_pouch::pouchCountUseItems() == 10)
                goto tryAgain;
            evtmgr_cmd::evtSetValue(evtEntry, args[0], 2);
            s32 * array = RFCItems_Common;
            s32 size = RFCItems_Common_Size - 1;
            rand = system::irand(99);
            if (rand < 40) { // 40% for Uncommon
                array = RFCItems_Uncommon;
                size = RFCItems_Uncommon_Size - 1;
            }
            s32 item = 0;
            do {
                rand = system::irand(size);
                item = array[rand];
            } while (item >= RFC_SPECIAL_START || item == 0);
            evtmgr_cmd::evtSetValue(evtEntry, args[1], item);
        } else if (rand < 95) { // 10% chance to raise Crit Mult by 25%
            evtmgr_cmd::evtSetValue(evtEntry, args[0], 3);
            evtmgr_cmd::evtSetValue(evtEntry, args[1], 25);
        } else { // 5% chance to raise Crit Rate by 4%
            evtmgr_cmd::evtSetValue(evtEntry, args[0], 4);
            evtmgr_cmd::evtSetValue(evtEntry, args[1], 4);
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(small_chest_get_params, 2)

    EVT_BEGIN(small_chest_open_evt)
    USER_FUNC(evt_mobj::evt_mobj_wait_animation_end, PTR("small_chest"), 0)
    USER_FUNC(small_chest_get_params, LW(0), LW(1))
    SWITCH(LW(0))
    CASE_EQUAL(0) // coins
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(smallChestCoins), 0, 0, LW(1))
    USER_FUNC(evt_pouch::evt_pouch_add_coins, LW(1))
    CASE_EQUAL(1) // hp
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(smallChestHp), 0, 0, LW(1))
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(3), LW(4), LW(5))
    USER_FUNC(evt_mario::evt_mario_get_height, LW(7))
    ADD(LW(4), LW(7))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("spm_recovery"), LW(3), LW(4), LW(5), LW(1), 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_pouch::evt_pouch_add_hp, LW(1))
    CASE_EQUAL(2) // item
    USER_FUNC(evt_item::evt_item_entry, PTR("smc_item"), LW(1), 0, 0, -1000, 0, 0, 0, 0, 0)
    USER_FUNC(evt_item::evt_item_flag_onoff, 1, PTR("smc_item"), 0x8)
    USER_FUNC(evt_item::evt_item_wait_collected, PTR("smc_item"))
    CASE_EQUAL(3) // crit mult
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(spirit1Get), 0, 0)
    USER_FUNC(evt_lp_add_crit, 0, LW(1))
    CASE_EQUAL(4) // crit rate
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(soul1Get), 0, 0)
    USER_FUNC(evt_lp_add_crit, LW(1), 0)
    END_SWITCH()
    RETURN()
    EVT_END()

    EVT_BEGIN(spawn_small_chest)
    USER_FUNC(evt_mobj::evt_mobj_thako, 0, PTR("small_chest"), 398, 0, -75, 0, PTR(small_chest_open_evt), 0, 0)
    RETURN()
    EVT_END()

    s32 spawn_rfnpc(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], 0);
        evtmgr_cmd::evtSetValue(evtEntry, args[1], 0);
        /*
            Decide slot 1 NPCs - Flimm, Boodin, Merluna, and a small chest
            Merluna will only show up once per run (Will have a normal weight when DJ is implemented), will not show up if disabled
            Small chest is more likely to appear when coins are low
        */
        RFNPCId slot1Npc = NONE;
        s32 flimmWeight = 140;
        s32 boodinWeight = 60;
        s32 merlunaWeight = 2;
        s32 smallChestWeight = 100;
        if (swdrv::swGet(1611) == true) // Merluna disabled
            merlunaWeight = 0;
        if (mario_pouch::pouchGetCoin() <= 50) // Has 50 or fewer coins
            smallChestWeight = 200;
        s32 rand = system::irand(flimmWeight + boodinWeight + merlunaWeight + smallChestWeight - 1);
        if (rand < flimmWeight) {
            slot1Npc = FLMM;
            evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)spawn_flimm);
        } else {
            rand -= flimmWeight;
            if (rand < boodinWeight) {
                slot1Npc = BDIN;
                evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)spawn_boodin);
            } else {
                rand -= boodinWeight;
                if (rand < merlunaWeight) {
                    evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)spawn_merluna);
                    slot1Npc = MLNA;
                } else {
                    slot1Npc = CHST;
                    evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)spawn_small_chest);
                }
            }
        }
        /*
            Decide slot 2 NPCs - Mover, Gabbi, D-Man, but usually nothing
            If Movers are disabled, obviously they aren't gonna show up
        */
        RFNPCId slot2Npc = NONE;
        s32 moverWeight = 25;
        s32 gabbiWeight = 200;
        s32 mitchWeight = 200;
        if (swdrv::swGet(1610) == true || swdrv::swByteGet(1) >= 179) // Movers disabled OR on floor 80/90
            moverWeight = 0;
        s32 nothingWeight = 999 - (moverWeight + gabbiWeight + mitchWeight);
        rand = system::irand(999);
        if (rand > nothingWeight) {
            rand -= nothingWeight;
            if (rand < moverWeight) {
                slot2Npc = MVER;
                evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)spawn_mover);
            } else {
                rand -= moverWeight;
                if (rand < gabbiWeight) {
                    slot2Npc = GBBI;
                    evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)spawn_gabbi);
                } else {
                    slot2Npc = MTCH;
                    evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)spawn_mitch);
                }
            }
        }
        swdrv::swByteSet(1623, (u8)slot1Npc);
        swdrv::swByteSet(1624, (u8)slot2Npc);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(spawn_rfnpc, 2)

    EVT_BEGIN(rest_floor_npc_setup)
    RUN_EVT(custom_pit_music)
    // Overwrite vanilla chests
    USER_FUNC(RFCCreateItemTable)
    USER_FUNC(evt_mobj::evt_mobj_delete, PTR(rfcChestName)) // vanilla chest, "box"
    USER_FUNC(RFCGenerate, PTR(new_dan_chest_interact_evt), PTR(new_dan_chest_open_evt))
    // Handle Whacka replacing the chest
    IF_NOT_EQUAL(GSW(22), 8) // If Whacka has not been brutally murdered in vanilla
    IF_EQUAL(GSWF(1612), 0) // Whacka NOT disabled
    USER_FUNC(evt_sub::evt_sub_random, 100, LW(0))
    IF_SMALL_EQUAL(LW(0), 4) // 5% chance to replace chest with Whacka
    USER_FUNC(evt_mobj::evt_mobj_delete, PTR(rfcChestName))
    RUN_EVT(spawn_whacka)
    ELSE()
    IF_EQUAL(GSWF(1644), 1)
    RUN_EVT(spawn_whacka)
    END_IF()
    END_IF()
    END_IF()
    END_IF()
    USER_FUNC(spawn_rfnpc, LW(0), LW(2))
    IF_NOT_EQUAL(LW(0), 0)
    RUN_CHILD_EVT(LW(0))
    END_IF()
    IF_NOT_EQUAL(LW(2), 0)
    RUN_CHILD_EVT(LW(2))
    END_IF()
    // Handle blessing wear-off effects in rest floors
    /*IF_SMALL(GSW(1602), 1)
    USER_FUNC(handleBlessingWearOff)
    END_IF()*/
    USER_FUNC(evt_dan_patch_dokan)
    RETURN_FROM_CALL()

    customwin::CWSelectColorDef musicSelectBgCols[] = {
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{215, 195, 255, 255}, {215, 195, 255, 255}, 120, 120}, // Purple
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{245, 195, 255, 255}, {245, 195, 255, 255}, 120, 120}, // Magenta
    };

    EVT_BEGIN(cwselect_music)
    USER_FUNC(EvtCWSelectEntry, PTR("Music"), CWSELECT_DEFAULT, PTR(selectMusicBlueText), PTR(selectMusicBox), 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Music"), PTR(vMusicName), PTR(vMusicDesc), icondrv::ICON_COOKING_DISK_R, 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Music"), PTR(nyMusicName), PTR(nyMusicDesc), icondrv::ICON_POWER_PLUS, 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Music"), PTR(ttMusicName), PTR(ttMusicDesc), icondrv::ICON_HP_PLUS, 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Music"), PTR(plMusicName), PTR(plMusicDesc), icondrv::ICON_HP_PLUS, 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Music"), PTR(jdMusicName), PTR(jdMusicDesc), icondrv::ICON_HP_PLUS, 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Music"), PTR(zkMusicName), PTR(zkMusicDesc), icondrv::ICON_HP_PLUS, 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Music"), PTR(noMusicName), PTR(noMusicDesc), icondrv::ICON_CATCH_CARD_SP, 0, 0, 0)
    USER_FUNC(EvtCWSelectSetHeaderColor, PTR("Music"), PTR(&MusicHeaderCol))
    USER_FUNC(EvtCWSelectSetBGColor, PTR("Music"), PTR(musicSelectBgCols), 4)
    // USER_FUNC(EvtCWSelectSetPointerIcon, PTR("Music"), icondrv::ICON_FORGET_ME_NOT_CARD_BLECK, FLOAT(0.3))
    // USER_FUNC(EvtCWSelectSetInstantOpenClose, PTR("Music"), 0, 1)
    // USER_FUNC(EvtCWSelectModifySfx, PTR("Music"), PTR("SFX_E_CRUSH1"), 0, -1)
    USER_FUNC(EvtCWSelectMenuStart, PTR("Music"), 0, LW(0))
    IF_NOT_EQUAL(LW(0), -1)
    SET(GSW(1621), LW(0))
    END_IF()
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("Music"))
    RETURN()
    EVT_END()

    EVT_BEGIN(cwselect_disorder)
    USER_FUNC(EvtCWSelectEntry, PTR("Disorder"), CWSELECT_DEFAULT, PTR("Disorders"), PTR("!!! Debug Menu !!!\nSelect a Disorder\nOwO What;s This\nTesting 4 lines\nActually 5 now"), 0, 0)
    USER_FUNC(EvtCWSelectSetBGColor, PTR("Disorder"), PTR(rainbowSelectBgCols), 8)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(apathyName), PTR(apathyDesc), TPLPATCH_ICON(ICON_DISORDER_APATHY), 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(dreadName), PTR(dreadDesc), TPLPATCH_ICON(ICON_DISORDER_DREAD), 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(prejudiceName), PTR(prejudiceDesc), TPLPATCH_ICON(ICON_DISORDER_PREJUDICE), 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(indifferenceName), PTR(indifferenceDesc), TPLPATCH_ICON(ICON_DISORDER_INDIFFERENCE), 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(recalcitranceName), PTR(recalcitranceDesc), TPLPATCH_ICON(ICON_DISORDER_RECALCITRANCE), 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(depravityName), PTR(depravityDesc), TPLPATCH_ICON(ICON_DISORDER_DEPRAVITY), 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(indolenceName), PTR(indolenceDesc), TPLPATCH_ICON(ICON_DISORDER_INDOLENCE), 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(melancholyName), PTR(melancholyDesc), TPLPATCH_ICON(ICON_DISORDER_MELANCHOLY), 0, 0, 0)
    USER_FUNC(EvtCWSelectAddListing, PTR("Disorder"), PTR(ruinName), PTR(ruinDesc), TPLPATCH_ICON(ICON_DISORDER_RUIN), 0, 0, 0)
    // USER_FUNC(EvtCWSelectSetInstantOpenClose, PTR("Disorder"), 1, 0)
    USER_FUNC(EvtCWSelectMenuStart, PTR("Disorder"), 0, LW(0))
    IF_NOT_EQUAL(LW(0), -1)
    ADD(LW(0), 1)
    SET(GSW(1660), LW(0))
    ELSE()
    SET(GSW(1660), 255)
    END_IF()
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("Disorder"))
    RETURN()
    EVT_END()

    EVT_BEGIN(determine_custom_music)
    // Difficulty
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(difficultyText))
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(difficultyOptions))
    USER_FUNC(evt_msg::evt_msg_continue)
    USER_FUNC(evt_lp_set_difficulty, LW(0))
    // Music
    RUN_CHILD_EVT(cwselect_music)
    SET(GW(5), 0) // Fixes the Dark Prognosticus breaking holographic enemies. What a funny ass bug
    USER_FUNC(DebugModeGetStatus, LW(0))
    IF_EQUAL(LW(0), 1)
    RUN_CHILD_EVT(cwselect_disorder)
    END_IF()
    RETURN_FROM_CALL()

    EVT_BEGIN(msg_continue)
    USER_FUNC(evt_msg::evt_msg_continue)
    RETURN_FROM_CALL()

    EVT_BEGIN(custom_music_sign)
    IF_EQUAL(GSWF(1601), 0)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(musicSignRFText), 0, 0)
    SET(GSWF(1601), 1)
    END_IF()
    RUN_CHILD_EVT(cwselect_music)
    IF_NOT_EQUAL(LW(0), -1)
    USER_FUNC(evt_snd::evt_snd_bgmoff_f_d, 0, 1000)
    WAIT_MSEC(1200)
    SET(LW(15), 1)
    RUN_EVT(custom_pit_music)
    END_IF()
    RETURN_FROM_CALL()

    EVT_BEGIN(spectre_logic)
    /*IF_EQUAL(GSW(1600), 1)
    USER_FUNC(rand100, LW(1))
    IF_SMALL(LW(1), 20)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(1))
    IF_NOT_EQUAL(LW(1), 156)
    IF_NOT_EQUAL(LW(1), 157)
    USER_FUNC(evt_npc::evt_npc_get_max_hp, PTR("me"), LW(1))
    USER_FUNC(spectreFloor, LW(1), LW(2))
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(3), LW(4), LW(5))
    USER_FUNC(evt_mario::evt_mario_get_height, LW(7))
    ADD(LW(4), LW(7))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("spm_recovery"), LW(3), LW(4), LW(5), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_pouch::evt_pouch_add_hp, LW(2))
    END_IF()
    END_IF()
    END_IF()
    END_IF()*/
    USER_FUNC(evt_npc::evt_npc_get_max_hp, PTR("me"), LW(1))
    USER_FUNC(ThunderVoucherIncrementCtr, LW(1))
    RETURN_FROM_CALL()

    EVT_BEGIN(npc_drop_item_patch)
    USER_FUNC(evt_npc_drop_item_new, LW(10), LW(11))
    RETURN_FROM_CALL()

    // Dialogue to determine quickstart or no
    EVT_BEGIN(determine_quickstart)
    USER_FUNC(SaveWriteModVersion, -1)
    // Disable some Jimbo parameters
    SET(GSWF(1611), 1) // Merluna
    SET(GSWF(1621), 1) // Low HP Sounds
    SET(GSWF(1630), 1) // Lighter Tremors
    SET(GSWF(1631), 1) // Aerodynamic
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(quickstartText), 0, 0)
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(quickstartOptions))
    USER_FUNC(evt_msg::evt_msg_continue)
    SWITCH(LW(0))
    CASE_EQUAL(1) // 2-1
    SET(GSW(137), 1)
    SET(GSW(0), 78)
    USER_FUNC(start_from_21)
    CASE_EQUAL(2) // 6-1
    SET(GSW(137), 2)
    SET(GSW(0), 226)
    USER_FUNC(start_from_61)
    CASE_EQUAL(3) // Postgame
    SET(GSW(137), 3)
    SET(GSW(0), 424)
    USER_FUNC(start_from_eg)
    CASE_ETC() // Normal pills
    SET(GSW(137), 0)
    END_SWITCH()
    IF_NOT_EQUAL(GSW(137), 0)
    SET(GSWF(2), 1)
    SET(GSWF(386), 1)
    SET(GSWF(387), 1)
    SET(GSWF(431), 1)
    SET(GSWF(501), 1)
    SET(GSWF(512), 1)
    SET(GSWF(612), 1)
    SET(GSWF(614), 1)
    SET(GSWF(615), 1)
    SET(GSWF(616), 1)
    USER_FUNC(evt_seq::evt_seq_set_seq, seqdrv::SEQ_MAPCHANGE, PTR("mac_05"), PTR("elv1"))
    END_IF()
    RETURN_FROM_CALL()

    EVT_BEGIN(disable_flopside_pit_entrance)
    USER_FUNC(evt_door::evt_door_enable_disable_dokan_desc, 0, PTR("dokan_1"))
    USER_FUNC(evt_map::evt_mapobj_flag_onoff, 1, 1, PTR("dokan_1"), 1)
    USER_FUNC(evt_hit::evt_hitobj_onoff, PTR("A2d_dokan_1"), 1, 0)
    USER_FUNC(evt_hit::evt_hitobj_onoff, PTR("A3d_dokan_1"), 1, 0)
    RETURN_FROM_CALL()

    EVT_BEGIN(patch_pit_exit)
    SET(GSW(1601), 0)
    SET(GSW(1600), 0)
    SET(GSW(1602), 0)
    USER_FUNC(evt_door::evt_door_set_dokan_descs, PTR(&new_dan_70_dokan_desc), 1)
    USER_FUNC(evt_lp_get_chest_keys, LW(7))
    IF_LARGE(LW(7), 0)
        INLINE_EVT()
            USER_FUNC(evt_door::evt_door_wait_flag, 256)
            USER_FUNC(evt_mario::evt_mario_key_off, 1)
            USER_FUNC(evt_sub::evt_sub_hud_configure, 0)
            WAIT_MSEC(100)
            SET(LW(6), 4)
            USER_FUNC(evt_lp_get_difficulty, LW(5))
            SUB(LW(6), LW(5))
            SET(LW(10), LW(7))
            MUL(LW(7), LW(6))
            USER_FUNC(evt_pouch::evt_pouch_get_hp, LW(9))
            USER_FUNC(evt_pouch::evt_pouch_get_max_hp, LW(8))
            SUB(LW(8), LW(9))
            CLAMP_INT(LW(7), 0, LW(8))
            SET(LW(11), LW(10))
            MUL(LW(11), -1)
            USER_FUNC(evt_lp_add_chest_keys, LW(11))
            USER_FUNC(evt_mario::evt_mario_set_pose, PTR("I_2"), 0)
            USER_FUNC(evt_mario::evt_mario_get_pos, LW(3), LW(4), LW(5))
            USER_FUNC(evt_mario::evt_mario_get_height, LW(2))
            ADD(LW(4), LW(2))
            USER_FUNC(evt_eff::evt_eff, 0, PTR("spm_recovery"), LW(3), LW(4), LW(5), LW(7), 0, 0, 0, 0, 0, 0, 0, 0)
            USER_FUNC(evt_pouch::evt_pouch_add_hp, LW(7))
            WAIT_MSEC(1500)
            USER_FUNC(evt_sub::evt_sub_hud_configure, 2)
            USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(chestKeysToHp), 0, 0, LW(10), LW(7))
            WAIT_MSEC(200)
            USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
            USER_FUNC(evt_mario::evt_mario_key_on)
        END_INLINE()
    END_IF()
    RETURN_FROM_CALL()

    EVT_BEGIN(patch_shadoo_health)
    USER_FUNC(declare_shadoo_stats, LW(10))
    RETURN_FROM_CALL()

    s32 clear_disorder(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        (void)evtEntry;
        if (Lunatic->Luna.disorder != DISORDER_NULL)
            ClearDisorderSub(0);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(clear_disorder, 0)

    s32 JimboGSWFBase = 0;

    s32 JimboSetGSWFBase(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        JimboGSWFBase = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(JimboSetGSWFBase, 1)

    bool JimboToggleOption(winmgr::WinmgrSelect * select) {
        s32 gswf = JimboGSWFBase + select->selectedItemIdx;
        if (swdrv::swGet(gswf) == true) {
            swdrv::swClear(gswf);
            spmario_snd::spsndSFXOn("SFX_SYS_FILE_MOJI_SET1");
        } else {
            swdrv::swSet(gswf);
            spmario_snd::spsndSFXOn("SFX_SYS_FILE_MOJI_DELETE1");
        }
        customwin::CWSelectGetActiveEntry()->Descs[select->selectedItemIdx].iconId = TPLPATCH_ICON(ICON_SETTING_ON) + (u8)swdrv::swGet(gswf);
        return false;
    }

    customwin::CWSelectColorDef featuresSelectBgCols[] = {
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{250, 200, 200, 255}, {250, 200, 200, 255}, 120, 120}, // Red
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{235, 235, 235, 255}, {235, 235, 235, 255}, 120, 120}, // Gray
    };

    customwin::CWSelectColorDef patchesSelectBgCols[] = {
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{225, 255, 225, 255}, {225, 255, 225, 255}, 120, 120}, // Green
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{235, 235, 235, 255}, {235, 235, 235, 255}, 120, 120}, // Gray
    };

    customwin::CWSelectColorDef accessSelectBgCols[] = {
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{210, 220, 255, 255}, {210, 220, 255, 255}, 120, 120}, // Blue
        {{255, 255, 255, 255}, {255, 255, 255, 255}, 120, 60},  // White
        {{235, 235, 235, 255}, {235, 235, 235, 255}, 120, 120}, // Gray
    };

    EVT_BEGIN(cwselect_features)
    USER_FUNC(JimboSetGSWFBase, 1610)
    USER_FUNC(EvtCWSelectEntry, PTR("Features"), CWSELECT_DEFAULT, PTR(selectJimboBlueText), PTR(selectJimboBox), 0, 0)
    SET(LW(1), (s32)TPLPATCH_ICON(ICON_SETTING_ON))
    ADD(LW(1), GSWF(1610))
    USER_FUNC(EvtCWSelectAddListing, PTR("Features"), PTR(moverFeaturesName), PTR(moverFeaturesDesc), LW(1), 0, 0, 0)
    SET(LW(1), (s32)TPLPATCH_ICON(ICON_SETTING_ON))
    ADD(LW(1), GSWF(1611))
    USER_FUNC(EvtCWSelectAddListing, PTR("Features"), PTR(merlunaFeaturesName), PTR(merlunaFeaturesDesc), LW(1), 0, 0, 0)
    SET(LW(1), (s32)TPLPATCH_ICON(ICON_SETTING_ON))
    ADD(LW(1), GSWF(1612))
    USER_FUNC(EvtCWSelectAddListing, PTR("Features"), PTR(whackaFeaturesName), PTR(whackaFeaturesDesc), LW(1), 0, 0, 0)
    SET(LW(1), (s32)TPLPATCH_ICON(ICON_SETTING_ON))
    ADD(LW(1), GSWF(1613))
    USER_FUNC(EvtCWSelectAddListing, PTR("Features"), PTR(marioFeaturesName), PTR(marioFeaturesDesc), LW(1), 0, 0, 0)
    USER_FUNC(EvtCWSelectOverrideBtnBehavior, PTR("Features"), BTN_2, PTR(JimboToggleOption))
    USER_FUNC(EvtCWSelectSetBGColor, PTR("Features"), PTR(featuresSelectBgCols), 4)
    USER_FUNC(EvtCWSelectMenuStart, PTR("Features"), 0, 0)
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("Features"))
    RETURN()
    EVT_END()

    EVT_BEGIN(cwselect_patches)
    USER_FUNC(JimboSetGSWFBase, 1620)
    USER_FUNC(EvtCWSelectEntry, PTR("Patches"), CWSELECT_DEFAULT, PTR(selectJimboBlueText), PTR(selectJimboBox), 0, 0)
    SET(LW(1), (s32)TPLPATCH_ICON(ICON_SETTING_ON))
    ADD(LW(1), GSWF(1620))
    USER_FUNC(EvtCWSelectAddListing, PTR("Patches"), PTR(lockPatchesName), PTR(lockPatchesDesc), LW(1), 0, 0, 0)
    SET(LW(1), (s32)TPLPATCH_ICON(ICON_SETTING_ON))
    ADD(LW(1), GSWF(1621))
    USER_FUNC(EvtCWSelectAddListing, PTR("Patches"), PTR(hpPatchesName), PTR(hpPatchesDesc), LW(1), 0, 0, 0)
    USER_FUNC(EvtCWSelectOverrideBtnBehavior, PTR("Patches"), BTN_2, PTR(JimboToggleOption))
    USER_FUNC(EvtCWSelectSetBGColor, PTR("Patches"), PTR(patchesSelectBgCols), 4)
    USER_FUNC(EvtCWSelectMenuStart, PTR("Patches"), 0, 0)
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("Patches"))
    RETURN()
    EVT_END()

    EVT_BEGIN(cwselect_accessibility)
    USER_FUNC(JimboSetGSWFBase, 1630)
    USER_FUNC(EvtCWSelectEntry, PTR("Accessibility"), CWSELECT_DEFAULT, PTR(selectJimboBlueText), PTR(selectJimboBox), 0, 0)
    SET(LW(1), (s32)TPLPATCH_ICON(ICON_SETTING_ON))
    ADD(LW(1), GSWF(1630))
    USER_FUNC(EvtCWSelectAddListing, PTR("Accessibility"), PTR(explosionShakeAccessName), PTR(explosionShakeAccessDesc), LW(1), 0, 0, 0)
    SET(LW(1), (s32)TPLPATCH_ICON(ICON_SETTING_ON))
    ADD(LW(1), GSWF(1631))
    USER_FUNC(EvtCWSelectAddListing, PTR("Accessibility"), PTR(aerodynamicAccessName), PTR(aerodynamicAccessDesc), LW(1), 0, 0, 0)
    USER_FUNC(EvtCWSelectOverrideBtnBehavior, PTR("Accessibility"), BTN_2, PTR(JimboToggleOption))
    USER_FUNC(EvtCWSelectSetBGColor, PTR("Accessibility"), PTR(accessSelectBgCols), 4)
    USER_FUNC(evt_npc::evt_npc_agb_async, PTR("p_wii_bldio"), 0)
    USER_FUNC(evt_npc::evt_npc_agb_async, PTR("p_wii_bldio_r"), 0)
    SET(LW(2), GSWF(1631))
    INLINE_EVT_ID(LW(15))
    DO(0)
    IF_EQUAL(LW(2), GSWF(1631))
    WAIT_FRM(1)
    DO_CONTINUE()
    END_IF()
    SET(LW(2), GSWF(1631))
    IF_EQUAL(GSWF(1631), 1) // Disabled
    USER_FUNC(marioMakeBald, 0)
    ELSE()
    USER_FUNC(marioMakeBald, 1)
    END_IF()
    WAIT_FRM(1)
    WHILE()
    END_INLINE()
    USER_FUNC(EvtCWSelectMenuStart, PTR("Accessibility"), 0, 0)
    USER_FUNC(EvtCWSelectReset)
    USER_FUNC(EvtCWSelectDelete, PTR("Accessibility"))
    DELETE_EVT(LW(15))
    RETURN()
    EVT_END()

    EVT_BEGIN(jimbo_real)
    SWITCH(GW(0))
    // FEATURES
    CASE_EQUAL(0)
    IF_EQUAL(GSWF(1604), 0)
        IF_EQUAL(GSW(1622), 0) // Shadoo not defeated, but Features menu opened
            USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(jimboWarn))
            USER_FUNC(evt_msg::evt_msg_select, 1, PTR(yesNoSelect_NoByDefault))
            IF_EQUAL(LW(0), 1)
                SET(GW(0), -1)
                SET(LW(1), -1)
            END_IF()
        END_IF()
    END_IF()
    IF_NOT_EQUAL(GW(0), -1)
    SET(GSWF(1604), 1)
    USER_FUNC(evt_msg::evt_msg_continue)
    RUN_CHILD_EVT(cwselect_features)
    END_IF()
    // PATCHES
    CASE_EQUAL(1)
    USER_FUNC(evt_msg::evt_msg_continue)
    RUN_CHILD_EVT(cwselect_patches)
    // ACCESSIBILITY
    CASE_EQUAL(2)
    USER_FUNC(evt_msg::evt_msg_continue)
    RUN_CHILD_EVT(cwselect_accessibility)
    // LOOP OR END SEQUENCE
    END_SWITCH()
    IF_EQUAL(GW(0), -1) // Exclusively upon declining to view Features
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(jimboAnythingElse2))
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(jimboAnythingElse), 0, PTR("me"))
    END_IF()
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(jimboSettings))
    IF_NOT_EQUAL(LW(0), 3)
    SET(GW(0), LW(0))
    RUN_CHILD_EVT(jimbo_real)
    RETURN()
    ELSE()
    // END JIMBO CUTSCENE
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(jimboBye))
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 1000, 11)
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_IF()
    RETURN()
    EVT_END()

    EVT_BEGIN(jimbo_speech)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_cam::evt_cam_get_pos, 5, LW(0), LW(1), LW(2))
    USER_FUNC(evt_cam::evt_cam3d_evt_zoom_in, 1, LW(0), 60, LW(2), EVT_NULLPTR, 60, -1100, 1000, 11)
    WAIT_MSEC(500)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(3), LW(4), LW(5))
    SUB(LW(3), 45)
    USER_FUNC(evt_mario::evt_mario_walk_to, LW(3), LW(5), 500)
    USER_FUNC(evt_mario::evt_mario_face_npc, PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), 0)
    WAIT_MSEC(1000)
    IF_EQUAL(GSWF(586), 0) // Jimbo first cutscene
    SET(GSWF(586), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(jimboIntro), 0, PTR("me"))
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(jimboIntro2), 0, PTR("me"))
    END_IF()
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(jimboSettings))
    IF_NOT_EQUAL(LW(0), 3)
    SET(GW(0), LW(0))
    RUN_CHILD_EVT(jimbo_real)
    RETURN()
    END_IF()
    // END JIMBO CUTSCENE
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(jimboBye))
    USER_FUNC(evt_cam::evt_cam_zoom_to_coords, 1000, 11)
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario::evt_mario_key_on)
    RETURN()
    EVT_END()

    EVT_BEGIN(fwd_jimbo_speech)
    RUN_EVT(jimbo_speech)
    RETURN()
    EVT_END()

    EVT_BEGIN(patch_mac_05)
    SET(GSW(1), 0)
    SET(GSW(1600), 0)
    SET(GSW(1601), 0)
    SET(GSW(1602), 0)
    SET(GSW(1610), 0)
    SET(GSW(1611), 0)
    USER_FUNC(evt_dan_reset_lunatic)
    USER_FUNC(evt_pouch::evt_pouch_check_have_item, 48, LW(15)) // Removes Pit Key if Mario has one
    IF_EQUAL(LW(15), 1)
    USER_FUNC(evt_pouch::evt_pouch_remove_item, 48)
    END_IF()
    RUN_CHILD_EVT(handle_dj_misc_behavior)
    USER_FUNC(clear_disorder)
    USER_FUNC(DebugModeGetStatus, LW(15))
    IF_EQUAL(LW(15), 1)
    USER_FUNC(evt_npc::evt_npc_entry, PTR("jimbo"), PTR("e_kazmi"), 0)
    ELSE()
    USER_FUNC(evt_npc::evt_npc_entry, PTR("jimbo"), PTR("e_antho"), 0)
    END_IF()
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("jimbo"), mod::cutscene_helpers::NPCProperty::ANIMS, PTR(heihoAnims))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("jimbo"), 0, true)
    USER_FUNC(evt_npc::evt_npc_set_position, PTR("jimbo"), -50, 0, -1250)
    USER_FUNC(evt_npc::evt_npc_set_property, PTR("jimbo"), 9, PTR(fwd_jimbo_speech))
    USER_FUNC(evt_npc::evt_npc_add_flip_part, PTR("jimbo"))
    USER_FUNC(evt_npc::evt_npc_set_scale, PTR("jimbo"), FLOAT(1.1287), FLOAT(1.1287), FLOAT(1.1287))
    RETURN_FROM_CALL()

    EVT_BEGIN(global_operations_non_dan)
    USER_FUNC(update_bump_tex)
    DO(0)
    USER_FUNC(animPoseSetMaterialEvtColorWrapper)
    // Also handle debug frame color while we're at it
    USER_FUNC(updateDebugFrameColor)
    WAIT_FRM(1)
    WHILE()
    RETURN()
    EVT_END()

    EVT_BEGIN(global_operations)
    RUN_EVT(global_operations_non_dan)
    USER_FUNC(evt_sub::evt_sub_get_mapname, 0, LW(14)) // Pit-Only runtime
    USER_FUNC(evt_strstr, LW(14), PTR("dan"), LW(15))
    IF_NOT_EQUAL(LW(15), 1)
    RETURN()
    ELSE()
    DO(0)                   // Start global operations loop
    IF_EQUAL(GSWF(1670), 1) // If Migraine or Indolence (Slow) has just been activated
    SET(GSWF(1670), 0)
    INLINE_EVT()
    WAIT_MSEC(500)
    USER_FUNC(migraineShortenCurseTimers)
    END_INLINE()
    END_IF()                // End Migraine check
    IF_EQUAL(GSWF(1672), 1) // Hourai Doll cutscene
    SET(GSWF(1672), 0)
    INLINE_EVT()
    WAIT_MSEC(1000)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(3), LW(4), LW(5))
    USER_FUNC(evt_mario::evt_mario_get_height, LW(7))
    ADD(LW(4), LW(7))
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("T_11"), 0)
    WAIT_MSEC(700)
    SUB(LW(4), 7)
    USER_FUNC(evt_eff::evt_eff, 0, PTR("spm_recovery"), LW(3), LW(4), LW(5), -1, 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(3), LW(4), LW(5), FLOAT(3.5), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(2)
    ADD(LW(3), 10)
    ADD(LW(4), 10)
    ADD(LW(5), 10)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(3), LW(4), LW(5), FLOAT(3.5), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(2)
    ADD(LW(3), -20)
    ADD(LW(4), -20)
    ADD(LW(5), -20)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(3), LW(4), LW(5), FLOAT(3.5), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(2)
    ADD(LW(4), 20)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(3), LW(4), LW(5), FLOAT(3.5), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(2)
    ADD(LW(3), 20)
    ADD(LW(4), -20)
    ADD(LW(5), 20)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_APPEAR1"), LW(3), LW(4), LW(5))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(3), LW(4), LW(5), FLOAT(3.5), 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(200)
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("I_2"), 0)
    SET(GSW(1602), 19)
    WAIT_MSEC(1800)
    IF_EQUAL(GSWF(1673), 0)
    SET(GSWF(1673), 1)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(houraiFirstUse), 0, 0)
    ELSE()
    WAIT_MSEC(700)
    END_IF()
    USER_FUNC(evt_mario::evt_mario_set_pose, PTR("S_1"), 0)
    USER_FUNC(evt_mario_motion_chg_mot, 0)
    USER_FUNC(evt_mario::evt_mario_key_on)
    END_INLINE()
    END_IF()               // End Hourai Doll activation check
    IF_EQUAL(GSW(1600), 3) // Begin Paramita active check
    // Handle cooldown timer first
    IF_LARGE(GSW(1610), 0)  // Seconds timer above 0
    IF_LARGE(GSW(1611), 0)  // Frame timer above 0
    SUB(GSW(1611), 1)       // Reduce frame timer by 1
    ELSE()                  // Frame timer hits 0, underflow
    SET(GSW(1611), 59)      // Reset frames to 60
    SUB(GSW(1610), 1)       // Subtract 1 second from the primary Paramita timer
    IF_EQUAL(GSW(1610), 20) // Play sfx when cooldown begins
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(4), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_EXPLOSION1"), LW(0), LW(1), LW(2))
    END_IF()
    END_IF()
    ELSE()
    IF_LARGE(GSW(1611), 0) // If sec = 0, set frame timer to 0
    SET(GSW(1611), 0)
    END_IF()
    END_IF()
    // Handle Paramita activation detection
    IF_EQUAL(GSW(1610), 0)
    IF_EQUAL(GSW(1611), 0)              // Both timers must be zeroed
    USER_FUNC(evt_mario_chk_key, LW(0)) // Mario must be keyed on
    IF_EQUAL(LW(0), 1)
    USER_FUNC(evt_sub::evt_key_get_button, 0, LW(0))
    IF_FLAG(LW(0), 0x400) // B must be a button held
    SET(GSW(1610), 50)
    SET(GSW(1611), 60) // Sets Paramita timers
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(4), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_EXPLOSION1"), LW(0), LW(1), LW(2))
    USER_FUNC(paramitaFloor, LW(0))
    USER_FUNC(evt_mario::evt_mario_take_damage, 2, 0, 0, 0, 0, LW(0))
    END_IF()
    END_IF()
    END_IF()
    END_IF()
    END_IF()
    WAIT_FRM(1)
    WHILE()
    END_IF()
    RETURN()
    EVT_END()

    EVT_BEGIN(run_global_operations)
    RUN_EVT(global_operations)
    RETURN_FROM_CALL()

    EVT_BEGIN(do_nothing)
    WAIT_MSEC(0)
    RETURN_FROM_CALL()

    EVT_BEGIN(do_almost_nothing)
    USER_FUNC(evt_mario_set_invincibility, FLOAT(125), 1)
    RETURN_FROM_CALL()

    // TEMP
    /*  EVT_BEGIN(temp_pipe)
      USER_FUNC(evt_door::evt_door_set_dokan_descs, PTR(&temp_mac_04_2_dokan_desc), 1)
      RETURN_FROM_CALL() */

    // Dark Puff contact ATK patch
    EVT_BEGIN(d_puff_dir_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 1)
    RETURN_FROM_CALL()

    // Dark Puff projectile ATK patch
    EVT_BEGIN(d_puff_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 2)
    RETURN_FROM_CALL()

    // Ice Bro projectile ATK patch
    EVT_BEGIN(i_bro_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 3)
    RETURN_FROM_CALL()

    // Ice Bro contact ATK patch
    EVT_BEGIN(i_bro_dir_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 2)
    RETURN_FROM_CALL()

    // Hyper Goomba contact ATK patch
    EVT_BEGIN(h_goomba_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 3)
    RETURN_FROM_CALL()

    // Hyper Paragoomba contact ATK patch
    EVT_BEGIN(h_pgoomba_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 3)
    RETURN_FROM_CALL()

    // Red Magikoopa projectile ATK patch
    EVT_BEGIN(r_magi_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 5)
    RETURN_FROM_CALL()

    // Tileoid PU ATK patch
    EVT_BEGIN(p_tile_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 5)
    RETURN_FROM_CALL()

    // Tileoid PU speed patch
    EVT_BEGIN(p_tile_speed)
    USER_FUNC(patch_tileoid_pu)
    RETURN_FROM_CALL()

    // Kilo Muth ATK/DEF patch
    EVT_BEGIN(k_muth_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 2)
    RETURN_FROM_CALL()

    // Spiked Gloomba ATK patch
    EVT_BEGIN(s_gloomba_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 3)
    RETURN_FROM_CALL()

    // Shady Koopa/Flip Shady Koopa ATK patch
    EVT_BEGIN(uranoko_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 4)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 2, 4)
    RETURN_FROM_CALL()

    // Bleepboxer projectile ATK patch
    EVT_BEGIN(bleep_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 3)
    RETURN_FROM_CALL()

    // White Clubba ATK patch
    EVT_BEGIN(w_clubba_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 4)
    RETURN_FROM_CALL()

    // Bawbus ATK patch
    EVT_BEGIN(bawb_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 3)
    RETURN_FROM_CALL()

    // Shady Boomerang Bro contact ATK patch
    EVT_BEGIN(sbb_dir_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 2)
    RETURN_FROM_CALL()

    // Shady Boomerang Bro proj ATK patch
    EVT_BEGIN(sbb_proj_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 5)
    RETURN_FROM_CALL()

    // Shady Magikoopa projectile speedup
    EVT_BEGIN(s_magi_proj_speed)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(9))
    IF_EQUAL(LW(9), 68)
    USER_FUNC(evt_npc::evt_npc_glide_to, PTR("me"), LW(0), LW(1), LW(2), 5000, FLOAT(420.0), 0, 0, 0, 0)
    ELSE()
    USER_FUNC(evt_npc::evt_npc_glide_to, PTR("me"), LW(0), LW(1), LW(2), 5000, FLOAT(120.0), 0, 0, 0, 0)
    END_IF()
    RETURN_FROM_CALL()

    // Shady Magikoopa projectile ATK
    EVT_BEGIN(s_magi_proj_atk)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 5)
    RETURN_FROM_CALL()

    // Shady Magikoopa animation delay
    EVT_BEGIN(s_magi_delay)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(9))
    SWITCH(LW(9))
    CASE_EQUAL(66)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_KAMEKU_MAGIC_SING1"), PTR("me"))
    WAIT_MSEC(1200)
    SWITCH_BREAK()
    CASE_EQUAL(67)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_KAMEKU_MAGIC_SING1"), PTR("me"))
    WAIT_MSEC(1200)
    SWITCH_BREAK()
    CASE_ETC()
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_KAMEKU_MAGIC_SING1"), PTR("me"))
    END_SWITCH()
    RETURN_FROM_CALL()

    // White Clubba movement speed
    EVT_BEGIN(w_clubba_mvmt)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(14))
    IF_EQUAL(LW(14), 95)
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(3), LW(5), 0, 120, 4, 0, 0)
    ELSE()
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(3), LW(5), 0, 60, 4, 0, 0)
    END_IF()
    RETURN_FROM_CALL()

    // White Clubba triple attack
    EVT_BEGIN(w_clubba_triple)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(14))
    IF_EQUAL(LW(14), 95)
    IF_SMALL(LW(6), 128)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_GABON_HURU1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 26, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 27, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 1, 1)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_GABON_HURU1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 26, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 27, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 1, 1)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_GABON_HURU1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 26, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 27, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 1, 1)
    END_IF()
    ELSE()
    IF_SMALL(LW(6), 60)
    USER_FUNC(evt_snd::evt_snd_sfxon_npc, PTR("SFX_E_GABON_HURU1"), PTR("me"))
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 26, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 27, 1)
    USER_FUNC(evt_npc::evt_npc_wait_anim_end, PTR("me"), 1)
    USER_FUNC(evt_npc::evt_npc_set_anim, PTR("me"), 1, 1)
    END_IF()
    END_IF()
    RETURN_FROM_CALL()

    // Shady Koopa AI rework
    EVT_BEGIN(uranoko_brain)
    USER_FUNC(evt_npc::evt_npc_get_unitwork, PTR("me"), 9, LW(8))
    IF_EQUAL(LW(8), 1)
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 9, 0)
    USER_FUNC(evt_npc::evt_npc_get_axis_movement_unit, PTR("me"), LW(9))
    MUL(LW(9), -1)
    USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), LW(9))
    END_IF()
    RETURN_FROM_CALL()

    // Shady Koopa movement speed
    EVT_BEGIN(uranoko_mvmt)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(4), LW(5), LW(6))
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("target"), LW(7), LW(8), LW(9))
    SUB(LW(7), LW(4))
    IF_SMALL(LW(7), 0)
    SUB(LW(1), 250)
    ELSE()
    ADD(LW(1), 250)
    END_IF()
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 9, 1)
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(1), LW(3), 0, 300, 4, 0, 0)
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 9, 0)
    RETURN_FROM_CALL()

    EVT_BEGIN(uranoko_dendrite)
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 9, 0)
    RETURN_FROM_CALL()

    s32 kami_bomb(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        npcdrv::NPCEntry * npc = (npcdrv::NPCEntry *)evtEntry->ownerNPC;
        npcdrv::NPCPart * part = npcdrv::npcGetPartById(npc, 1);
        mario::MarioWork * mario = mario::marioGetPtr();
        f32 dist = PSVECSquareDistance(&npc->position, &mario->position);
        u8 marioIn3D = mario::marioCheck3d();
        u8 npcIn3D = npc->flippedTo3d;
        u8 failsIfOne = marioIn3D + npcIn3D;
        f32 distThreshold = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        if (dist < distThreshold && failsIfOne != 1) {
            if (mario->invincibilityTimer == 0 && mario->motionId != MOT_DAMAGE && mario->motionId != MOT_HIT && mario->motionId != MOT_FLIP &&
                mario->motionId != MOT_FLIP_AIR && mario->motionId != MOT_BOTTOMLESS && mario->motionId != MOT_CHAR_CHANGE && mario->motionId != MOT_FAIRY_CHANGE &&
                mario->motionId != MOT_LIFE_SHROOM) {
                npcdrv::npcDamageMario(npc, part, &part->position, 0, part->attackPower, 4);
            }
        }
        return 2;
    }
    EVT_DECLARE_USER_FUNC(kami_bomb, 1)

    EVT_BEGIN(kami_no_dokkan_sub)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("target"), LW(11), LW(8), LW(9))
    ADD(LW(8), 50)
    USER_FUNC(evt_npc::evt_npc_arc_to, PTR("me"), LW(11), LW(8), LW(9), 800, 0, 45, 0, 16, 0)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(11), LW(8), LW(9))
    USER_FUNC(kami_bomb, 5200)
    USER_FUNC(evt_eff::evt_eff, 0, PTR("spm_explosion"), 2, LW(11), LW(8), LW(9), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_EXPLOSION1"), LW(11), LW(8), LW(9))
    INLINE_EVT()
    IF_EQUAL(GSWF(1630), 1)
    USER_FUNC(evt_cam::evt_cam_shake, 5, FLOAT(0.9), FLOAT(0.9), FLOAT(0.0), 173, 0)
    ELSE()
    USER_FUNC(evt_cam::evt_cam_shake, 5, FLOAT(0.5), FLOAT(0.5), FLOAT(0.0), 173, 0)
    END_IF()
    END_INLINE()
    RETURN()
    EVT_END()

    // Kamikaze Goomba explosion visuals & second attack
    EVT_BEGIN(kami_no_dokkan)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(11))
    IF_EQUAL(LW(11), 10)
    // Explosion #1
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(11), LW(8), LW(9))
    USER_FUNC(kami_bomb, 5200)
    USER_FUNC(evt_eff::evt_eff, 0, PTR("spm_explosion"), 2, LW(11), LW(8), LW(9), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_EXPLOSION1"), LW(11), LW(8), LW(9))
    INLINE_EVT()
    IF_EQUAL(GSWF(1630), 1)
    USER_FUNC(evt_cam::evt_cam_shake, 5, FLOAT(0.9), FLOAT(0.9), FLOAT(0.0), 173, 0)
    ELSE()
    USER_FUNC(evt_cam::evt_cam_shake, 5, FLOAT(0.5), FLOAT(0.5), FLOAT(0.0), 173, 0)
    END_IF()
    END_INLINE()
    USER_FUNC(evt_lp_get_difficulty, LW(11))
    // Explosion #2 if difficulty is Normal
    IF_LARGE_EQUAL(LW(11), 1)
    RUN_CHILD_EVT(kami_no_dokkan_sub)
    END_IF()
    // Explode twice more if difficulty is Hard
    IF_LARGE_EQUAL(LW(11), 2)
    RUN_CHILD_EVT(kami_no_dokkan_sub)
    RUN_CHILD_EVT(kami_no_dokkan_sub)
    END_IF()
    // Bounce
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("target"), LW(11), LW(8), LW(9))
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(13), LW(14), LW(15))
    USER_FUNC(evt_npc::evt_npc_arc_to, PTR("me"), LW(11), LW(14), LW(9), 500, 0, 30, 0, 16, 0)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(11), LW(8), LW(9))
    END_IF()
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_KURIBOO_LANDING1"), LW(11), LW(8), LW(9))
    RETURN_FROM_CALL()

    // Bomb Boo explosion damage
    EVT_BEGIN(bomb_boo_atk)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(13))
    IF_EQUAL(LW(13), 86)
    USER_FUNC(kami_bomb, 6900)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(13), LW(14), LW(15))
    USER_FUNC(evt_eff::evt_eff, 0, PTR("spm_explosion"), 2, LW(13), LW(14), LW(15), FLOAT(1.2), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_E_CAMEREBOM2_EXPLOSION1"), LW(13), LW(14), LW(15))
    INLINE_EVT()
    IF_EQUAL(GSWF(1630), 1)
    USER_FUNC(evt_cam::evt_cam_shake, 5, FLOAT(1.3), FLOAT(1.3), FLOAT(0.0), 173, 0)
    ELSE()
    USER_FUNC(evt_cam::evt_cam_shake, 5, FLOAT(0.6), FLOAT(0.6), FLOAT(0.0), 173, 0)
    END_IF()
    END_INLINE()
    END_IF()
    RETURN_FROM_CALL()

    // Skellobyte delay shorten
    EVT_BEGIN(skello_ai)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(6))
    SWITCH(LW(6))
    CASE_EQUAL(225)
    WAIT_MSEC(0)
    SWITCH_BREAK()
    CASE_EQUAL(228)
    WAIT_MSEC(0)
    SWITCH_BREAK()
    CASE_ETC()
    USER_FUNC(evt_npc::evt_npc_arc_to, PTR("me"), LW(0), LW(1), LW(2), 200, 0, FLOAT(10.0), 0, 0, 0)
    SWITCH_BREAK()
    END_SWITCH()
    RETURN_FROM_CALL()

    // Skellobyte faster attacks
    EVT_BEGIN(skello_ai2)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(6))
    SWITCH(LW(6))
    CASE_EQUAL(225)
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(0), LW(2), 0, FLOAT(255.0), 28, 0, 0)
    SWITCH_BREAK()
    CASE_EQUAL(228)
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(0), LW(2), 0, FLOAT(255.0), 28, 0, 0)
    SWITCH_BREAK()
    CASE_ETC()
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(0), LW(2), 0, FLOAT(100.0), 28, 0, 0)
    SWITCH_BREAK()
    END_SWITCH()
    RETURN_FROM_CALL()

    // Skellobyte faster movement in general
    EVT_BEGIN(skello_ai3)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(6))
    SWITCH(LW(6))
    CASE_EQUAL(225)
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(1), LW(3), 0, FLOAT(60.0), 28, 0, 0)
    SWITCH_BREAK()
    CASE_EQUAL(228)
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(1), LW(3), 0, FLOAT(60.0), 28, 0, 0)
    SWITCH_BREAK()
    CASE_ETC()
    USER_FUNC(evt_npc::evt_npc_walk_to, PTR("me"), LW(1), LW(3), 0, FLOAT(40.0), 28, 0, 0)
    SWITCH_BREAK()
    END_SWITCH()
    RETURN_FROM_CALL()

    /*
        EVT_BEGIN(uranoko_mvmtcheck)
        LBL(73)
        USER_FUNC(evt_npc::evt_npc_get_position, PTR("target"), LW(4), LW(5), LW(6))
        USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(7), LW(8), LW(9))
        SUB(LW(8), LW(5))
        IF_LARGE(LW(8), 50)
        WAIT_FRM(1)
        GOTO(73)
        END_IF()
        IF_SMALL(LW(8), -50)
        WAIT_FRM(1)
        GOTO(73)
        END_IF()
        RETURN_FROM_CALL() */

    // IF_SMALL(LW(7), 0)
    // USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), 1)
    // ELSE()
    // USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), -1)
    // END_IF()

    // Dark Lakitu stats
    EVT_BEGIN(dlak_stats)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(1))
    IF_EQUAL(LW(1), 34)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 1)
    ELSE()
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 3)
    USER_FUNC(setHitFlags, 0)
    USER_FUNC(setHitboxScale, 0, FLOAT(24), FLOAT(32), FLOAT(10))
    END_IF()
    RETURN_FROM_CALL()

    // Dark Lakitu proj limit patch
    EVT_BEGIN(dlak_limit)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(1))
    IF_EQUAL(LW(1), 34)
    USER_FUNC(temp_unk::lakitu_count_spinies, 36, LW(0))
    ELSE()
    USER_FUNC(temp_unk::lakitu_count_spinies, 38, LW(0))
    END_IF()
    RETURN_FROM_CALL()

    /*   // Replace Sky Blue Spiny anim defs to fix Dark Lakitu bullshit
       EVT_BEGIN(skyblue_anim_defs)
       USER_FUNC(evt_npc::evt_npc_set_property, PTR("me"), 14, PTR(&skyblueAnims))
       RETURN_FROM_CALL() */

    // Dark Lakitu proj patch 2
    EVT_BEGIN(dlak_skyblue_2)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(1))
    IF_EQUAL(LW(1), 34)
    USER_FUNC(temp_unk::npc_jugemu_toss_spiny, 59)
    ELSE()
    USER_FUNC(temp_unk::npc_jugemu_toss_spiny, 336)
    END_IF()
    RETURN_FROM_CALL()

    // Dark Lakitu proj patch
    EVT_BEGIN(dlak_skyblue)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(1))
    IF_EQUAL(LW(1), 34)
    USER_FUNC(evt_npc::evt_npc_agb_async, PTR("e_togezo"), LW(0))
    ELSE()
    USER_FUNC(evt_npc::evt_npc_agb_async, PTR("e_togezb"), LW(0))
    END_IF()
    RETURN_FROM_CALL()

    /*   // KP Koopa stats
       EVT_BEGIN(kp_stats)
       USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(1))
       IF_EQUAL(LW(1), 14)
       USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 1)
       ELSE()
       USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 2)
       //   USER_FUNC(setHitFlags, 1)
       //   USER_FUNC(setFlag2c, 0, 0x304000)
       //   USER_FUNC(setFlag2c, 1, 0x224000)
       //   USER_FUNC(setHitboxScale, 0, FLOAT(20), FLOAT(22), FLOAT(20))
       //   USER_FUNC(setHitboxScale, 1, FLOAT(14), FLOAT(10), FLOAT(20))
       END_IF()
       RETURN_FROM_CALL() */

    // Shy Guy stats
    EVT_BEGIN(heiho_stats)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(1))
    IF_EQUAL(LW(1), 0)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 1)
    ELSE()
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), -1, 2)
    IF_LARGE(LW(1), 500)
    //  USER_FUNC(setHitboxScale, 0, FLOAT(27), FLOAT(27), FLOAT(27))
    USER_FUNC(evt_npc::evt_npc_set_scale, PTR("me"), FLOAT(1.1287), FLOAT(1.1287), FLOAT(1.1287))
    END_IF()
    END_IF()
    RETURN_FROM_CALL()

    EVT_BEGIN(william_blaster_new_onspawn)
    USER_FUNC(evt_npc::evt_npc_get_position, PTR("me"), LW(0), 0, 0)
    USER_FUNC(evt_mario::evt_mario_get_pos, LW(1), 0, 0)
    IF_LARGE(LW(1), LW(0))
    USER_FUNC(evt_npc::evt_npc_set_axis_movement_unit, PTR("me"), 1)
    END_IF()
    USER_FUNC(evt_npc::evt_npc_agb_async, PTR("e_kilr_g"), EVT_NULLPTR)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 4)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 2, 4)
    USER_FUNC(evt_npc::evt_npc_get_unitwork, PTR("me"), 2, LW(0))
    IF_SMALL_EQUAL(LW(0), 0)
    USER_FUNC(evt_npc::evt_npc_set_unitwork, PTR("me"), 2, 1000)
    END_IF()
    RETURN()
    EVT_END()

    EVT_BEGIN(william_new_atk)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(0))
    IF_EQUAL(LW(0), (s32)NPC_BULLET_BILL)
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 2)
    ELSE()
    USER_FUNC(evt_npc::evt_npc_set_part_attack_power, PTR("me"), 1, 6)
    END_IF()
    RETURN_FROM_CALL()

    s32 shadooIncreaseFireImmunity(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        npcdrv::NPCEntry * npc = (npcdrv::NPCEntry *)evtEntry->ownerNPC;
        npc->fireImmunityTime = 1.5f;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(shadooIncreaseFireImmunity, 0)

    EVT_BEGIN(shadoo_reduce_wait_time)
    USER_FUNC(evt_lp_get_difficulty, LW(13))
    IF_LARGE_EQUAL(LW(13), 1) // Normal or harder
    USER_FUNC(shadooIncreaseFireImmunity)
    DIVF(LW(1), FLOAT(2.0))
    END_IF()
    USER_FUNC(evt_npc::evt_npc_wait_for, PTR("me"), LW(1))
    RETURN_FROM_CALL()

    EVT_BEGIN(dark_luigi_reduce_wait_time)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(13))
    IF_EQUAL(LW(13), (s32)NPC_DARK_LUIGI)
    USER_FUNC(evt_lp_get_difficulty, LW(13))
    IF_LARGE_EQUAL(LW(13), 1) // Normal or harder
    USER_FUNC(shadooIncreaseFireImmunity)
    DIVF(LW(1), FLOAT(2.0))
    END_IF()
    USER_FUNC(evt_npc::evt_npc_wait_for, PTR("me"), LW(1))
    END_IF()
    RETURN_FROM_CALL()

    EVT_BEGIN(dark_luigi_negate_knockback)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(13))
    IF_NOT_EQUAL(LW(13), (s32)NPC_DARK_LUIGI)
    RUN_CHILD_EVT(temp_unk::evt_8043bc68)
    END_IF()
    RETURN_FROM_CALL()

    EVT_BEGIN(dark_bowser_reduce_wait_time)
    USER_FUNC(evt_npc::evt_npc_get_property, PTR("me"), 13, LW(13))
    IF_EQUAL(LW(13), (s32)NPC_DARK_BOWSER)
    USER_FUNC(evt_lp_get_difficulty, LW(13))
    IF_LARGE_EQUAL(LW(13), 1) // Normal or harder
    USER_FUNC(shadooIncreaseFireImmunity)
    DIVF(LW(1), FLOAT(1.5))
    END_IF()
    USER_FUNC(evt_npc::evt_npc_wait_for, PTR("me"), LW(1))
    END_IF()
    RETURN_FROM_CALL()

    static void evtPatches() {
        // Render Flopside Pit pipe useless and patch Pit exit pipe
        evtmgr_cmd::EvtScriptCode * disableFlopsidePitEntrance = map_data::mapDataPtr("mac_15")->initScript;
        evtpatch::hookEvt(disableFlopsidePitEntrance, 11, disable_flopside_pit_entrance);
        evtpatch::hookEvt(dan::dan_70_init_evt, 39, patch_pit_exit);

        // Get enemy onSpawnScripts from templates
        evtmgr_cmd::EvtScriptCode * dPuffDirAtk = npcdrv::npcEnemyTemplates[357].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * dPuffAtk = npcdrv::npcEnemyTemplates[358].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * iBroDirAtk = npcdrv::npcEnemyTemplates[341].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * iBroAtk = npcdrv::npcEnemyTemplates[344].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * hGoombaAtk = npcdrv::npcEnemyTemplates[329].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * hpGoombaAtk = npcdrv::npcEnemyTemplates[328].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * rMagiAtk = npcdrv::npcEnemyTemplates[324].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * pTileAtk = npcdrv::npcEnemyTemplates[379].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * pTileSpeed = npcdrv::npcEnemyTemplates[379].moveScript;
        evtmgr_cmd::EvtScriptCode * kMuthAtk = npcdrv::npcEnemyTemplates[279].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * sGloombaAtk = npcdrv::npcEnemyTemplates[6].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * uranokoAtk = npcdrv::npcEnemyTemplates[10].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * fUranokoAtk = npcdrv::npcEnemyTemplates[13].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * bleepAtk = npcdrv::npcEnemyTemplates[277].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * wClubbaAtk = npcdrv::npcEnemyTemplates[350].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * sbbDirAtk = npcdrv::npcEnemyTemplates[340].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * sbbProjAtk = npcdrv::npcEnemyTemplates[343].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * sMagiProjSpeed = npcdrv::npcEnemyTemplates[347].moveScript;
        evtmgr_cmd::EvtScriptCode * sMagiProjAtk = npcdrv::npcEnemyTemplates[347].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * wClubbaTriple = npcdrv::npcEnemyTemplates[350].atkScript;
        evtmgr_cmd::EvtScriptCode * uranokoMvmt = npcdrv::npcEnemyTemplates[10].atkScript;
        evtmgr_cmd::EvtScriptCode * uranokoBrain = npcdrv::npcEnemyTemplates[10].moveScript;
        evtmgr_cmd::EvtScriptCode * uranokoOnDmgTaken = npcdrv::npcEnemyTemplates[10].onHitScript;
        evtmgr_cmd::EvtScriptCode * kamiKuriAtk = npcdrv::npcEnemyTemplates[330].atkScript;
        evtmgr_cmd::EvtScriptCode * bombBooAtk = npcdrv::npcEnemyTemplates[366].atkScript;
        evtmgr_cmd::EvtScriptCode * dLakAtk = npcdrv::npcEnemyTemplates[30].atkScript;
        evtmgr_cmd::EvtScriptCode * bawbAtk = npcdrv::npcEnemyTemplates[375].onSpawnScript;
        //    evtmgr_cmd::EvtScriptCode *kpStats = npcdrv::npcEnemyTemplates[7].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * goombaStats = npcdrv::npcEnemyTemplates[2].onSpawnScript;
        //    evtmgr_cmd::EvtScriptCode *skyblueAnimDefs = npcdrv::npcEnemyTemplates[336].onSpawnScript;
        evtmgr_cmd::EvtScriptCode * commonDeathScript = npcdrv::npcEnemyTemplates[350].deathScript;

        // Enemy stat overwrite
        evtpatch::hookEvtReplace(dPuffDirAtk, 2, d_puff_dir_atk);
        evtpatch::hookEvtReplace(dPuffAtk, 11, d_puff_atk);
        evtpatch::hookEvtReplace(iBroDirAtk, 1, i_bro_dir_atk);
        evtpatch::hookEvtReplace(iBroAtk, 1, i_bro_atk);
        evtpatch::hookEvtReplace(hGoombaAtk, 2, h_goomba_atk);
        evtpatch::hookEvtReplace(hpGoombaAtk, 2, h_pgoomba_atk);
        evtpatch::hookEvtReplace(rMagiAtk, 1, r_magi_atk);
        evtpatch::hookEvtReplace(pTileAtk, 1, p_tile_atk);
        evtpatch::hookEvt(pTileSpeed, 16, p_tile_speed);
        evtpatch::hookEvt(npc_tile::npc_tile_common_onspawn_evt, 6, p_tile_speed);
        evtpatch::hookEvtReplace(kMuthAtk, 1, k_muth_atk);
        evtpatch::hookEvtReplace(sGloombaAtk, 2, s_gloomba_atk);
        evtpatch::hookEvtReplace(uranokoAtk, 5, uranoko_atk);
        evtpatch::hookEvtReplace(fUranokoAtk, 2, uranoko_atk);
        evtpatch::hookEvtReplace(bleepAtk, 1, bleep_atk);
        evtpatch::hookEvtReplace(wClubbaAtk, 1, w_clubba_atk);
        evtpatch::hookEvtReplace(sbbDirAtk, 1, sbb_dir_atk);
        evtpatch::hookEvtReplace(sbbProjAtk, 1, sbb_proj_atk);
        evtpatch::hookEvtReplace(sMagiProjSpeed, 24, s_magi_proj_speed);
        evtpatch::hookEvtReplace(sMagiProjAtk, 1, s_magi_proj_atk);
        evtpatch::hookEvtReplace(wClubbaTriple, 36, w_clubba_mvmt);
        evtpatch::hookEvtReplace(wClubbaTriple, 50, w_clubba_mvmt);
        evtpatch::hookEvtReplaceBlock(wClubbaTriple, 52, w_clubba_triple, 59);
        evtpatch::hookEvtReplace(uranokoMvmt, 16, uranoko_mvmt);
        //    evtpatch::hookEvt(uranokoMvmt, 1, uranoko_mvmtcheck);
        evtpatch::hookEvt(uranokoBrain, 1, uranoko_brain);
        evtpatch::hookEvt(uranokoOnDmgTaken, 1, uranoko_dendrite);
        evtpatch::hookEvtReplace(kamiKuriAtk, 51, kami_no_dokkan);
        evtpatch::hookEvt(bombBooAtk, 34, bomb_boo_atk);
        evtpatch::hookEvtReplace(temp_unk::skellobits_unk7, 7, skello_ai);
        evtpatch::hookEvtReplace(temp_unk::skellobits_unk7, 25, skello_ai2);
        evtpatch::hookEvtReplace(temp_unk::skellobits_unk2, 16, skello_ai3);
        evtpatch::hookEvtReplace(temp_unk::skellobits_unk8, 16, skello_ai2);
        evtpatch::hookEvt(temp_unk::kameks_unk7, 18, s_magi_delay);
        evtpatch::hookEvtReplace(temp_unk::dark_broom_kamek_unk7, 17, s_magi_delay);
        evtpatch::hookEvtReplace(dLakAtk, 31, dlak_skyblue_2);
        evtpatch::hookEvtReplace(dLakAtk, 12, dlak_skyblue);
        evtpatch::hookEvtReplace(dLakAtk, 7, dlak_limit);
        evtpatch::hookEvtReplace(temp_unk::lakitu_onspawn_child, 8, dlak_stats);
        evtpatch::hookEvtReplace(bawbAtk, 1, bawb_atk);
        //    evtpatch::hookEvtReplace(kpStats, 4, kp_stats);
        evtpatch::hookEvtReplace(goombaStats, 2, heiho_stats);
        evtpatch::hookEvtReplaceBlock(temp_unk::goomba_unk2_child, 12, heiho_g_panic, 16);

        // Bullet William
        evtpatch::hookEvtReplace(npcdrv::npcEnemyTemplates[106].onSpawnScript, 1, william_new_atk);
        evtpatch::hookEvt(npcdrv::npcEnemyTemplates[109].onSpawnScript, 1, william_blaster_new_onspawn);
        patch::hookFunction(temp_unk::func_80203608, npc_killtai_spawn_child);

        // Ninjoe patched to only bomb 10% of the time, thanks Lily!
        writeWord(&npc_ninja::ninjoe_bomb_calc_chance, 0xDC, CMPWI(3, 10));

        // Shadoo
        evtpatch::hookEvtReplace(dan::dan_start_shadoo_evt, 1, fwd_new_shadoo_evt);
        evtpatch::hookEvtReplaceBlock(dan::dan_shadoo_defeat_evt, 13, shadoo_defeat_msg, 15);
        evtpatch::hookEvt(dan::dan_shadoo_fight_evt, 97, patch_shadoo_health);
        evtpatch::hookEvt(dan::dan_shadoo_fight_evt, 56, patch_shadoo_health);
        evtpatch::hookEvt(dan::dan_shadoo_fight_evt, 15, patch_shadoo_health);
        // Shadoo NPC patches
        evtpatch::hookEvtReplace(npcdrv::npcEnemyTemplates[287].onHitScript, 77, shadoo_reduce_wait_time);       // Dark Mario recovers much quicker
        evtpatch::hookEvtReplace(npcdrv::npcEnemyTemplates[287].onHitScript, 50, do_nothing);                    // Dark Mario no longer jumps back
        evtpatch::hookEvtReplace(npcdrv::npcEnemyTemplates[288].onHitScript, 88, shadoo_reduce_wait_time);       // Dark Peach recovers much quicker
        evtpatch::hookEvtReplace(npcdrv::npcEnemyTemplates[288].onHitScript, 62, do_nothing);                    // Dark Peach no longer jumps back
        evtpatch::hookEvtReplace(npcdrv::npcEnemyTemplates[286].onHitScript, 108, dark_luigi_reduce_wait_time);  // Dark Luigi recovers much quicker
        evtpatch::hookEvtReplace(npcdrv::npcEnemyTemplates[286].onHitScript, 76, dark_luigi_negate_knockback);   // Dark Luigi no longer jumps back
        evtpatch::hookEvtReplace(npcdrv::npcEnemyTemplates[285].onHitScript, 131, dark_bowser_reduce_wait_time); // Dark Bowser recovers quicker

        // Pit music replacement
        evtpatch::hookEvt(evt_door::evt_door_enter_dokan_down_evt, 79, determine_custom_music);
        evtpatch::hookEvt(evt_door::evt_door_enter_dokan_down_evt, 72, msg_continue);
        evtpatch::hookEvtReplace(evt_door::evt_door_enter_dokan_down_evt, 70, do_nothing);
        evtpatch::hookEvtReplace(dan::dan_exit_pipe_sign_interact_evt, 2, custom_music_sign);

        // Pit room pipe speedup
        evtpatch::hookEvtReplace(evt_door::evt_door_enter_dokan_left_same_map_evt, 108, do_almost_nothing);
        evtpatch::hookEvtReplace(evt_door::evt_door_enter_dokan_left_same_map_evt, 99, do_almost_nothing);
        evtpatch::hookEvtReplace(evt_door::evt_door_enter_dokan_right_same_map_evt, 119, do_almost_nothing);
        evtpatch::hookEvtReplace(evt_door::evt_door_enter_dokan_right_same_map_evt, 110, do_almost_nothing);

        // Quickstart
        evtpatch::hookEvt(aa1_01::aa1_01_mario_house_transition_evt, 10, determine_quickstart);

        // Item event patch
        evtpatch::hookEvt(temp_unk::default_item_use_evt, 4, itemEventEvtHook);

        // Custom NPCs
        evtpatch::hookEvtReplaceBlock(dan::dan_chest_room_init_evt, 65, rest_floor_npc_setup, 84);
        evtpatch::hookEvtReplace(map_data::mapDataPtr("mac_05")->initScript, 79, patch_mac_05);
        writeWord(&evt_shop::evt_shop_build_flimm_pit_item_tables, 0xB8, CMPWI(0, 1)); // Patch Pit Flimm's double pricing in the Flopside Pit

        // Blessings/Curses
        evtpatch::hookEvt(commonDeathScript, 4, spectre_logic);
        evtpatch::hookEvt(evt_door::door_init_evt, 1, run_global_operations);

        // Flipside Pit 3D Thoreau Lock patch
        writeWord(&mario_motion::marioCalcFramesToTerminalVel, 0x164, NOP);

        // Holo coin patch
        evtpatch::hookEvtReplace(temp_unk::npc_drop_item_evt, 3, npc_drop_item_patch);
    }

    /*void rotateCustomDokans() {
        mario::MarioWork * mario = mario::marioGetPtr();
        if (((mario->buttonsPressed & WPAD_BTN_C) == WPAD_BTN_C)) {
            for (u16 i = 0; i < BERO_DOKAN_MAX; i += 1) {
                if (bero::Dokans[i] == nullptr)
                    return;
                if (msl::string::strcmp(bero::Dokans[i]->Desc.mapName, spmario::gp->mapName) == 0) {
                    mobjdrv::mobjCalcMtx(bero::Dokans[i]->headMobj);
                    if (bero::Dokans[i]->baseMobj != nullptr)
                        mobjdrv::mobjCalcMtx(bero::Dokans[i]->baseMobj);
                }
            }
        }
        return;
    }*/

    void main() {
        // Allocate memory for LunaticPitWork
        // mempatch::memPatch();
        Lunatic = (LunaticPitWork *)memory::__memAlloc(0, sizeof(LunaticPitWork));
        msl::string::memset(Lunatic, 0, sizeof(LunaticPitWork));
        // Heronicus library inits
        globalop::GlobalOperationsInit();
        msgpatch::msgpatchMain();
        customwin::CustomWinMain();
        evtpatch::evtmgrExtensionInit();
        const char * wicon2 = "wicon2";
        tplpatch::iconPatch(wicon2);
        effpatch::effpatchInit();
        sndpatch::sndpatchInit();
        // bero::beroberoInit();
        // globalop::globalopAddEntry((void *)rotateCustomDokans, nullptr);
        acpatch::acpatchInit();
        // Add new BGM entries
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_100F8BIT", 1385, 50, 64, 0, 0);
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_100FSYNTH", 1378, 127, 64, 0, 0);
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_100FPIANO", 1379, 127, 64, 0, 0);
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_100FBEATS", 1384, 127, 64, 0, 0);
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_LUNATIC_A", 1380, 127, 64, 0, 0);
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_LUNATIC_B", 1381, 127, 64, 0, 0);
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_LUNATIC_C", 1382, 127, 64, 0, 0);
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_LUNATIC_D", 1383, 127, 64, 0, 0);
        sndpatch::sndpatchAddBGMEntryDirect("BGM_MAP_HARDBOSS", 1386, 127, 64, 0, 0);
        wii::os::OSReport("BGM slots taken: %d\n", spmario_snd::spsnd_work.bgmCount);
        // Mod functions
        danYouSuck();
        guiOverrides();
        rewrite_main();
        RFCDRVPatches();
        miscLambdas();
        hookNpcChildrenFromParentSpawnFuncs();
        danOverwrite();
        pluswinWhackaBump();
        danDontFuckingCrash();
        patchNpcRgbaFuncs();
        danPatchPitEnemies();
        npcMessagePatches();
        evtPatches();
        patchMarioDamage();
        exceptionPatch();
        romfontExpand();
        wii::os::OSReport(MOD_VERSION ": all modules successfully loaded.\n");
    }
}
