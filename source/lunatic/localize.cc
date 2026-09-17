#include <common.h>
#include <cutscene_helpers.h>
#include <evt_cmd.h>
#include <gen.h>
#include <lunatic/localize.h>
#include <mod.h>
#include <msgpatch.h>
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
#include <wii/cx.h>
#include <wii/gx.h>
#include <wii/os/OSError.h>
#include <wii/tpl.h>
#include <wii/wpad.h>

namespace mod {
    using namespace spm;
    using namespace npcdrv;

    NPCMessagePatchData data[] =
        {
            {NPC_DARK_GOOMBA, "Hyper Goomba", desc_hyper_goomba, tattle_hyper_goomba},
            {NPC_UNUSED_SPIKED_GOOMBA, "Spiked Gloomba", desc_spiked_gloomba, tattle_spiked_gloomba},
            {NPC_DARK_SPIKED_GOOMBA, "Spiked Hyper Goomba", desc_spiked_hyper_goomba, tattle_spiked_hyper_goomba},
            {NPC_DARK_PARAGOOMBA, "Hyper Paragoomba", desc_hyper_paragoomba, tattle_hyper_paragoomba},
            {NPC_DARK_HEADBONK_GOOMBA, "Kamikaze Goomba", desc_kamikaze_goomba, tattle_kamikaze_goomba},
            {NPC_DARK_KOOPA, "Dark Koopa", desc_dark_koopa, tattle_dark_koopa},
            {NPC_DARK_KOOPATROL, "Dark Koopatrol", desc_dark_koopatrol, tattle_dark_koopatrol},
            {NPC_DARK_PARATROOPA, "Dark Paratroopa", desc_dark_paratroopa, tattle_dark_paratroopa},
            {NPC_DARK_SPIKE_TOP, "Red Spike Top", desc_red_spike_top, tattle_red_spike_top},
            {NPC_DARK_SPINY, "Sky-Blue Spiny", desc_sky_blue_spiny, tattle_sky_blue_spiny},
            {NPC_DARK_DULL_BONES, "Dark Bones", desc_dark_bones, tattle_dark_bones},
            {NPC_DARK_DULL_BONES_PROJ, "Dark Bones", desc_dark_bones, tattle_dark_bones},
            {NPC_DARK_HAMMER_BRO, "Shady Hammer Bro", desc_shady_hammer_bro, tattle_shady_hammer_bro},
            {NPC_DARK_HAMMER_BRO_PROJ, "Shady Hammer Bro", desc_shady_hammer_bro, tattle_shady_hammer_bro},
            {NPC_DARK_BOOMERANG_BRO, "Shady Boomerang Bro", desc_shady_boomerang_bro, tattle_shady_boomerang_bro},
            {NPC_DARK_BOOMERANG_BRO_PROJ, "Shady Boomerang Bro", desc_shady_boomerang_bro, tattle_shady_boomerang_bro},
            {NPC_DARK_FIRE_BRO, "Ice Bro", desc_ice_bro, tattle_ice_bro},
            {NPC_DARK_FIRE_BRO_PROJ, "Ice Bro", desc_ice_bro, tattle_ice_bro},
            {NPC_DARK_MAGIKOOPA, "Shady Magikoopa", desc_shady_magikoopa, tattle_shady_magikoopa},
            {NPC_DARK_BROOM_MAGIKOOPA, "Shady Magikoopa", desc_shady_magikoopa, tattle_shady_magikoopa},
            {NPC_DARK_MAGIKOOPA_PROJ, "Shady Magikoopa", desc_shady_magikoopa, tattle_shady_magikoopa},
            {NPC_DARK_STRIKER, "Shady Striker", desc_shady_striker, tattle_shady_striker},
            {NPC_DARK_STRIKER_PROJ, "Shady Striker", desc_shady_striker, tattle_shady_striker},
            {NPC_DARK_DARK_BOO, "Bomb Boo", desc_bomb_boo, tattle_bomb_boo},
            {NPC_DARK_CLUBBA, "White Clubba", desc_white_clubba, tattle_white_clubba},
            {NPC_DARK_FUZZY, "Green Fuzzy", desc_green_fuzzy, tattle_green_fuzzy},
            {NPC_DARK_CLEFT, "Hyper Cleft", desc_hyper_cleft, tattle_hyper_cleft},
            {NPC_DARK_RUFF_PUFF, "Dark Puff", desc_dark_puff, tattle_dark_puff},
            {NPC_DARK_RUFF_PUFF_PROJ, "Dark Puff", desc_dark_puff, tattle_dark_puff},
            {NPC_DARK_CHOMP, "Gold Chomp", desc_gold_chomp, tattle_gold_chomp},
            {NPC_DARK_TILEOID, "Tileoid PU", desc_tileoid_pu, tattle_tileoid_pu},
            {NPC_DARK_JAWBUS, "Bawbus", desc_bawbus, tattle_bawbus},
            {NPC_DARK_NINJOE, "Ninjeremiah", desc_ninjeremiah, tattle_ninjeremiah},
            {NPC_DARK_NINJOE_SHURIKEN, "Ninjeremiah", desc_ninjeremiah, tattle_ninjeremiah},
            {NPC_DARK_NINJOE_BOMB, "Ninjeremiah", desc_ninjeremiah, tattle_ninjeremiah},
            {NPC_DARK_SKELLOBIT, "Skellobyte", desc_skellobyte, tattle_skellobyte},
            {NPC_DARK_SPIKY_SKELLOBIT, "Spiky Skellobyte", desc_spiky_skellobyte, tattle_spiky_skellobyte},
            {NPC_DARK_CHERBIL, "Ash Cherbil", desc_ash_cherbil, tattle_ash_cherbil},
            {NPC_SHADY_KOOPA, "Shady Koopa", desc_shady_koopa, tattle_shady_koopa},
            {NPC_FLIP_SHADY_KOOPA, "Shady Koopa", desc_shady_koopa, tattle_shady_koopa},
            {NPC_FLIP_BUZZY_BEETLE, "Buzzy Beetle", desc_flip_buzzy_beetle, tattle_flip_buzzy_beetle},
            {NPC_FLIP_SPIKE_TOP, "Spike Top", desc_flip_spike_top, tattle_flip_spike_top},
            {NPC_GREEN_MAGIKOOPA, "Green Magikoopa", desc_green_magikoopa, tattle_green_magikoopa},
            {NPC_GREEN_BROOM_MAGIKOOPA, "Green Magikoopa", desc_green_magikoopa, tattle_green_magikoopa},
            {NPC_GREEN_MAGIKOOPA_PROJ, "Green Magikoopa", desc_green_magikoopa, tattle_green_magikoopa},
            {NPC_WHITE_MAGIKOOPA, "White Magikoopa", desc_white_magikoopa, tattle_white_magikoopa},
            {NPC_WHITE_BROOM_MAGIKOOPA, "White Magikoopa", desc_white_magikoopa, tattle_white_magikoopa},
            {NPC_WHITE_MAGIKOOPA_PROJ, "White Magikoopa", desc_white_magikoopa, tattle_white_magikoopa},
            {NPC_RED_MAGIKOOPA, "Red Magikoopa", desc_red_magikoopa, tattle_red_magikoopa},
            {NPC_RED_BROOM_MAGIKOOPA, "Red Magikoopa", desc_red_magikoopa, tattle_red_magikoopa},
            {NPC_RED_MAGIKOOPA_PROJ, "Red Magikoopa", desc_red_magikoopa, tattle_red_magikoopa},
            {NPC_SPINIA, "Spinia", desc_spinia, tattle_spinia},
            {NPC_SPUNIA, "Spunia", desc_spunia, tattle_spunia},
            {NPC_BOMBSHELL_BILL, "Bullet William", desc_bullet_william, tattle_bullet_william},
            {NPC_BOMBSHELL_BILL_BLASTER, "William Blaster", desc_william_blaster, tattle_william_blaster},
            {NPC_GREEN_BOOMBOXER, "Bleepboxer", desc_bleepboxer, tattle_bleepboxer},
            {NPC_GREEN_BOOMBOXER_PROJ, "Bleepboxer", desc_bleepboxer, tattle_bleepboxer},
            {NPC_PURPLE_MUTH, "Kilo Muth", desc_kilo_muth, tattle_kilo_muth},
            {NPC_DRY_BONES_UNUSED, "Shy Guy", desc_yellow_shy_guy, tattle_yellow_shy_guy},
            {NPC_GOLD_FUZZY, "Shy Guy", desc_green_shy_guy, tattle_green_shy_guy},
            {NPC_GREEN_FUZZY, "Shy Guy", desc_blue_shy_guy, tattle_blue_shy_guy},
            {NPC_SWAMPIRE, "Shy Guy", desc_shy_guy, tattle_shy_guy},
            {NPC_PHANTOM_EMBER, "Dark Lakitu", desc_dark_lakitu, tattle_dark_lakitu},
            {NPC_DARK_MARIO, "Phantom Mario", desc_phantom_mario, tattle_phantom_mario},
            {NPC_DARK_PEACH, "Phantom Peach", desc_phantom_peach, tattle_phantom_peach},
            {NPC_DARK_BOWSER, "Phantom Bowser", desc_phantom_bowser, tattle_phantom_bowser},
            {NPC_DARK_LUIGI, "Phantom Luigi", desc_phantom_luigi, tattle_phantom_luigi},
            {(NPCTribeId)-1, nullptr, nullptr, nullptr},
    };

    // Array-size message IDs * identifier size limit of 15
    char tattleMsgs[sizeof(data) / sizeof(NPCMessagePatchData)][15];

    void npcMessagePatches() {
        // Overwrite all enemy tattles, card names, and card descriptions
        for (s32 i = 0; data[i].tribeId != -1; i += 1) {
            npcdrv::NPCTribe * tribe = npcdrv::npcGetTribe(data[i].tribeId);
            msgpatch::msgpatchAddEntry(item_data::itemDataTable[tribe->catchCardItemId].nameMsg, data[i].nameMsg, true);
            msgpatch::msgpatchAddEntry(item_data::itemDataTable[tribe->catchCardItemId].descMsg, data[i].cardMsg, true);
            msl::stdio::sprintf(tattleMsgs[i], "anna_%s", item_data::itemDataTable[tribe->catchCardItemId].descMsg);
            msgpatch::msgpatchAddEntry(tattleMsgs[i], data[i].tattleMsg, true);
        };
        // Misc
        msgpatch::msgpatchAddEntry("D100_entrance_03", D100_entrance_03, true);
        msgpatch::msgpatchAddEntry("mac_kanban_004", mac_kanban_004, true);
        msgpatch::msgpatchAddEntry("m_noroi", backCursyaText, true);
        msgpatch::msgpatchAddEntry(mystBumpNamePtr, mystBumpName, true);
        msgpatch::msgpatchAddEntry(mystBumpDescPtr, mystBumpDesc, true);
        msgpatch::msgpatchAddEntry(chestKeyNamePtr, chestKeyName, true);
        msgpatch::msgpatchAddEntry(chestKeyDescPtr, chestKeyDesc, true);
        return;
    }

    const char * msgSearchCustomNpc(spm::npcdrv::NPCEntry * npc) {
        if (npc == nullptr)
            return nullptr;
        const char * tattle = nullptr;
        if (msl::string::strstr(npc->name, "rebear") != nullptr) {
            tattle = tattle_merluna;
        } else if (msl::string::strstr(npc->name, "dan_card") != nullptr) {
            tattle = spm::msgdrv::msgSearch("mac_19_card");
        } else if (msl::string::strstr(npc->name, "mover") != nullptr) {
            tattle = tattle_mover;
        } else if (msl::string::strstr(npc->name, "dan_koburon") != nullptr) {
            tattle = tattle_whacka;
        } else if (msl::string::strstr(npc->name, "jimbo") != nullptr) {
            tattle = tattle_jimbo;
        } else if (msl::string::strstr(npc->name, "mitch") != nullptr) {
            tattle = tattle_mitch;
        } else if (msl::string::strstr(npc->name, "gabbi") != nullptr) {
            tattle = tattle_gabbi;
        } else
            return nullptr;
        msl::stdio::sprintf(spm::search::search_wp->msgBuf, "<gsearch>\n%s", tattle);
        return search::search_wp->msgBuf;
    }

    const char * npcGetNameFromTribeId(s32 tribeId) {
        if (tribeId < 0)
            return npcGetNameFromTribeIdError;
        npcdrv::NPCTribe * tribe = npcdrv::npcGetTribe(tribeId);
        return msgdrv::msgSearch(item_data::itemDataTable[tribe->catchCardItemId].nameMsg);
    }

}
