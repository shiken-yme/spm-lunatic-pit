#pragma once

#include <common.h>
#include <spm/npcdrv.h>
#include <spm/mario.h>
#include <spm/effdrv.h>

CPP_WRAPPER(spm::temp_unk)

using namespace spm;

UNKNOWN_FUNCTION(luigi_boss_set_stats);
UNKNOWN_FUNCTION(bowser_spawn_fire);
UNKNOWN_FUNCTION(luigi_superjump_atk);
EVT_DECLARE_USER_FUNC(uranoko_idk, 4)

EVT_DECLARE(kameks_unk7)
EVT_DECLARE(dark_broom_kamek_unk7)
EVT_DECLARE(skellobits_unk7)
EVT_DECLARE(skellobits_unk2)
EVT_DECLARE(skellobits_unk8)
EVT_DECLARE(lakitu_onspawn_child)
EVT_DECLARE(goomba_unk2_child)
UNKNOWN_FUNCTION(shellPart1UpdateFunc);
UNKNOWN_FUNCTION(shellPart2UpdateFunc);
EVT_DECLARE(dimen_unk_fight_script_1)
EVT_DECLARE(dimen_unk_fight_script_2)
EVT_DECLARE(dimen_unk_fight_script_3)
EVT_DECLARE_USER_FUNC(dimen_determine_move_pos, 3)
EVT_DECLARE(whacka_move_evt)
EVT_DECLARE(whacka_onhit_evt)
EVT_DECLARE(default_item_use_evt)
EVT_DECLARE(npc_drop_item_evt)
EVT_DECLARE(evt_8043bc68)

// template id
EVT_DECLARE_USER_FUNC(npc_jugemu_toss_spiny, 1)

// tribe id, num
EVT_DECLARE_USER_FUNC(lakitu_count_spinies, 2)

DECOMP_STATIC(spm::npcdrv::NPCTribeAnimDef whacka_anim_defs[])

extern spm::npcdrv::NPCDefense whacka_defenses;

// should go in npcdrv.h, but compiler doesn't like recursive includes
s32 npcHandleHitXp(spm::mario::MarioWork * marioWork, spm::npcdrv::NPCEntry * npcEntry, s32 killXp, s32 unk_variant);

EVT_DECLARE_USER_FUNC(backCursyaHit, 0)

// too lazy to make a header for these lol
effdrv::EffEntry * effSpmVoltEntry(f32, f32, void * target, s32);
effdrv::EffEntry * effItemThunderEntry(f32, f32, f32, f32, s32 variant, s32, s32, s32);
void effItemThunderSetTargetMario(effdrv::EffEntry * eff);

// nw4r::lyt::TexMap::Set
void func_802f2860(void * _this, wii::tpl::TPLHeader * tpl, u32 idx);

/*
    Child NPC spawn functions for various NPC types
    Each one applies to all members of a given NPC family unless otherwise stated
*/
EVT_DECLARE_USER_FUNC(func_801f9294, 1) // Hammer Bros
EVT_DECLARE_USER_FUNC(func_801f9cfc, 2) // Boomerang Bros
EVT_DECLARE_USER_FUNC(func_801fa3d0, 1) // Fire Bros
EVT_DECLARE_USER_FUNC(func_80202ea8, 1) // Dull Bones
EVT_DECLARE_USER_FUNC(func_80200f5c, 0) // Magikoopas
EVT_DECLARE_USER_FUNC(func_802052fc, 0) // Koopa Strikers
EVT_DECLARE_USER_FUNC(func_80203608, 1) // Bill Blaster
EVT_DECLARE_USER_FUNC(func_80203808, 1) // Bombshell Blaster (Why is this separate?)
EVT_DECLARE_USER_FUNC(func_8022e008, 0) // Pokeys
EVT_DECLARE_USER_FUNC(func_80206a84, 2) // Ruff Puffs
EVT_DECLARE_USER_FUNC(func_802232c0, 1) // Dayzees
EVT_DECLARE_USER_FUNC(func_80221cf4, 0) // Squigs
EVT_DECLARE_USER_FUNC(func_801d9a88, 2) // Boomboxers
EVT_DECLARE_USER_FUNC(func_802355f8, 2) // Sproing-Oings (into Mini variants)
EVT_DECLARE_USER_FUNC(func_801d8d94, 0) // Mr. Is
EVT_DECLARE_USER_FUNC(func_802259f0, 2) // Foton
EVT_DECLARE_USER_FUNC(func_801f6514, 2) // Hooligons/Eeligons
EVT_DECLARE_USER_FUNC(func_801d7d74, 0) // Barribads
EVT_DECLARE_USER_FUNC(func_8021eaac, 1) // Ninjoes (Bomb)
EVT_DECLARE_USER_FUNC(func_8021e8ac, 0) // Ninjoes (Shurikens)
EVT_DECLARE_USER_FUNC(func_80234368, 0) // Skellobomber, Head-Type
EVT_DECLARE_USER_FUNC(func_80234d3c, 0) // Skellobomber, Bait-Type
EVT_DECLARE_USER_FUNC(func_80234fd4, 0) // Skellobait (Breath)
EVT_DECLARE_USER_FUNC(func_8021259c, 0) // Magiblots
EVT_DECLARE_USER_FUNC(func_8024b198, 2) // Cherbils
EVT_DECLARE_USER_FUNC(func_8024bc8c, 2) // Wrackle
EVT_DECLARE_USER_FUNC(func_801f8220, 0)

CPP_WRAPPER_END()