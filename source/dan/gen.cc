#include <common.h>
#include <cutscene_helpers.h>
#include <evt_cmd.h>
#include <evtpatch.h>
#include <gen.h>
#include <lp_common.h>
#include <mod.h>
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
    /*
        This file contains most of the code for Lunatic Pit room generation and enemy data
        NPCDATA is something I'm very scared to refactor. I know it's awful. I'll get around to it one day.
        NPCDATA used to be part of a header with no C file. I was a different fellow. I promise I've changed.
    */

    using namespace spm;
    using namespace spm::npcdrv;
    using namespace spm::item_data;

    /*
        NPCDATA
    */

    // All npcStructs are named after their "name" field, 1 plus their tribe ID
    DanNPCData npcStruct1 = {1, 70, 0, 4, 7};
    DanNPCData npcStruct2 = {2, 65, 0, 3, 6};
    DanNPCData npcStruct3 = {3, 40, 0, 3, 6};
    DanNPCData npcStruct4 = {4, 85, 0, 4, 6};
    DanNPCData npcStruct5 = {5, 60, 0, 3, 6};
    DanNPCData npcStruct6 = {6, 50, 0, 3, 5};
    DanNPCData npcStruct7 = {7, 30, 0, 3, 6};
    DanNPCData npcStruct8 = {8, 60, 0, 3, 5};
    DanNPCData npcStruct9 = {9, 30, 0, 3, 6};
    DanNPCData npcStruct10 = {10, 80, 0, 4, 6};
    DanNPCData npcStruct11 = {11, 70, 0, 4, 6};
    DanNPCData npcStruct12 = {12, 25, 0, 3, 5};
    DanNPCData npcStruct13 = {13, 30, 0, 3, 5};
    DanNPCData npcStruct15 = {15, 35, 0, 3, 5};
    DanNPCData npcStruct16 = {16, 25, 0, 3, 5};
    DanNPCData npcStruct17 = {17, 20, 0, 2, 5};
    DanNPCData npcStruct19 = {19, 35, 0, 2, 5};
    DanNPCData npcStruct20 = {20, 30, 0, 1, 4};
    DanNPCData npcStruct21 = {21, 25, 0, 3, 4};
    DanNPCData npcStruct22 = {22, 25, 0, 3, 4};
    DanNPCData npcStruct23 = {23, 40, 0, 3, 5};
    DanNPCData npcStruct24 = {24, 40, 0, 3, 4};
    DanNPCData npcStruct25 = {25, 25, 0, 2, 5};
    DanNPCData npcStruct26 = {26, 20, 0, 3, 5};
    DanNPCData npcStruct28 = {28, 40, 0, 3, 5};
    DanNPCData npcStruct30 = {30, 70, 0, 1, 2};
    DanNPCData npcStruct31 = {31, 25, 2, 3, 4};
    DanNPCData npcStruct32 = {32, 30, 2, 2, 4};
    DanNPCData npcStruct33 = {33, 45, 0, 3, 6};
    DanNPCData npcStruct35 = {35, 60, 0, 1, 2};
    DanNPCData npcStruct36 = {36, 50, 0, 1, 2};
    DanNPCData npcStruct37 = {37, 45, 0, 3, 6};
    DanNPCData npcStruct38 = {38, 35, 0, 3, 6};
    DanNPCData npcStruct39 = {39, 75, 0, 2, 6};
    DanNPCData npcStruct40 = {40, 30, 0, 3, 5};
    DanNPCData npcStruct42 = {42, 65, 0, 2, 5};
    DanNPCData npcStruct44 = {44, 60, 0, 1, 3};
    DanNPCData npcStruct46 = {46, 70, 0, 2, 4};
    DanNPCData npcStruct48 = {48, 45, 0, 1, 3};
    DanNPCData npcStruct50 = {50, 50, 0, 1, 2};
    DanNPCData npcStruct52 = {52, 50, 0, 3, 4};
    DanNPCData npcStruct54 = {54, 30, 0, 2, 5};
    DanNPCData npcStruct56 = {56, 50, 0, 1, 2};
    DanNPCData npcStruct58 = {58, 50, 0, 3, 4};
    DanNPCData npcStruct60 = {60, 30, 0, 2, 5};
    DanNPCData npcStruct62 = {62, 50, 0, 2, 5};
    DanNPCData npcStruct64 = {64, 70, 0, 2, 5};
    DanNPCData npcStruct65 = {65, 50, 0, 2, 4};
    DanNPCData npcStruct67 = {67, 66, 0, 1, 3};
    DanNPCData npcStruct68 = {68, 44, 0, 1, 3};
    DanNPCData npcStruct70 = {70, 65, 0, 2, 5};
    DanNPCData npcStruct74 = {74, 50, 0, 2, 4};
    DanNPCData npcStruct76 = {76, 50, 0, 2, 5};
    DanNPCData npcStruct84 = {84, 30, 0, 2, 4};
    DanNPCData npcStruct85 = {85, 50, 0, 3, 6};
    DanNPCData npcStruct86 = {86, 60, 0, 3, 6};
    DanNPCData npcStruct87 = {87, 60, 0, 2, 4};
    DanNPCData npcStruct89 = {89, 50, 0, 2, 4};
    DanNPCData npcStruct90 = {90, 40, 0, 2, 4};
    DanNPCData npcStruct91 = {91, 20, 0, 2, 3};
    DanNPCData npcStruct95 = {95, 75, 0, 3, 5};
    DanNPCData npcStruct96 = {96, 70, 0, 1, 3};
    DanNPCData npcStruct97 = {97, 80, 0, 3, 6};
    DanNPCData npcStruct98 = {98, 65, 0, 4, 6};
    DanNPCData npcStruct99 = {99, 35, 0, 5, 8};
    DanNPCData npcStruct100 = {100, 40, 0, 3, 6};
    DanNPCData npcStruct101 = {101, 75, 0, 3, 5};
    DanNPCData npcStruct102 = {102, 35, 0, 3, 5};
    DanNPCData npcStruct103 = {103, 50, 0, 2, 4};
    DanNPCData npcStruct105 = {105, 80, 0, 1, 3};
    DanNPCData npcStruct110 = {110, 75, 0, 3, 6};
    DanNPCData npcStruct112 = {112, 70, 0, 2, 6};
    DanNPCData npcStruct114 = {114, 65, 0, 3, 6};
    DanNPCData npcStruct116 = {116, 69, 0, 3, 5};
    DanNPCData npcStruct118 = {118, 7, 0, 1, 2};
    DanNPCData npcStruct123 = {123, 50, 0, 1, 3};
    DanNPCData npcStruct124 = {124, 30, 0, 1, 3};
    DanNPCData npcStruct125 = {125, 10, 0, 1, 2};
    DanNPCData npcStruct126 = {126, 60, 0, 3, 6};
    DanNPCData npcStruct127 = {127, 61, 0, 3, 6};
    DanNPCData npcStruct129 = {129, 90, 0, 3, 6};
    DanNPCData npcStruct131 = {131, 80, 0, 2, 6};
    DanNPCData npcStruct135 = {135, 55, 0, 3, 5};
    DanNPCData npcStruct137 = {137, 55, 0, 2, 4};
    DanNPCData npcStruct139 = {139, 55, 0, 3, 5};
    DanNPCData npcStruct143 = {143, 50, 0, 2, 4};
    DanNPCData npcStruct145 = {145, 60, 0, 3, 4};
    DanNPCData npcStruct147 = {147, 80, 0, 2, 4};
    DanNPCData npcStruct151 = {151, 40, 0, 1, 3};
    DanNPCData npcStruct153 = {153, 15, 1, 1, 2};
    DanNPCData npcStruct157 = {157, 10, 0, 2, 4};
    DanNPCData npcStruct158 = {158, 20, 0, 1, 2};
    DanNPCData npcStruct160 = {160, 45, 0, 1, 2};
    DanNPCData npcStruct161 = {161, 40, 0, 1, 2};
    DanNPCData npcStruct162 = {162, 70, 0, 1, 2};
    DanNPCData npcStruct164 = {164, 40, 0, 2, 4};
    DanNPCData npcStruct165 = {165, 40, 0, 2, 4};
    DanNPCData npcStruct166 = {166, 40, 0, 2, 4};
    DanNPCData npcStruct167 = {167, 40, 0, 2, 4};
    DanNPCData npcStruct168 = {168, 74, 0, 2, 4};
    DanNPCData npcStruct169 = {169, 50, 1, 1, 3};
    DanNPCData npcStruct170 = {170, 20, 1, 1, 3};
    DanNPCData npcStruct171 = {171, 20, 1, 1, 2};
    DanNPCData npcStruct172 = {172, 20, 1, 1, 2};
    DanNPCData npcStruct174 = {174, 55, 0, 3, 5};
    DanNPCData npcStruct178 = {178, 10, 0, 2, 5};
    DanNPCData npcStruct179 = {179, 40, 0, 2, 5};
    DanNPCData npcStruct180 = {180, 30, 0, 2, 4};
    DanNPCData npcStruct185 = {185, 50, 0, 2, 3};
    DanNPCData npcStruct189 = {189, 80, 0, 3, 6};
    DanNPCData npcStruct190 = {190, 50, 0, 2, 5};
    DanNPCData npcStruct193 = {193, 60, 0, 3, 5};
    DanNPCData npcStruct201 = {201, 16, 0, 1, 3};
    DanNPCData npcStruct202 = {202, 10, 0, 1, 3};
    DanNPCData npcStruct206 = {206, 30, 0, 3, 5};
    DanNPCData npcStruct208 = {208, 30, 0, 3, 5};
    DanNPCData npcStruct210 = {210, 30, 0, 3, 5};
    DanNPCData npcStruct212 = {212, 40, 0, 2, 4};
    DanNPCData npcStruct215 = {215, 40, 0, 2, 4};
    DanNPCData npcStruct218 = {218, 40, 0, 2, 4};
    DanNPCData npcStruct221 = {221, 40, 0, 2, 4};
    DanNPCData npcStruct225 = {225, 50, 0, 3, 6};
    DanNPCData npcStruct226 = {226, 40, 0, 4, 6};
    DanNPCData npcStruct228 = {228, 50, 0, 2, 4};
    DanNPCData npcStruct229 = {229, 40, 0, 2, 5};
    DanNPCData npcStruct231 = {231, 30, 0, 3, 5};
    DanNPCData npcStruct234 = {234, 30, 0, 1, 4};
    DanNPCData npcStruct238 = {238, 40, 0, 2, 5};
    DanNPCData npcStruct240 = {240, 40, 0, 2, 5};
    DanNPCData npcStruct242 = {242, 40, 0, 2, 5};
    DanNPCData npcStruct246 = {246, 60, 0, 2, 5};
    DanNPCData npcStruct247 = {247, 4, 0, 1, 4};
    DanNPCData npcStruct248 = {248, 30, 0, 2, 4};
    DanNPCData npcStruct249 = {249, 25, 0, 2, 4};
    DanNPCData npcStruct250 = {250, 35, 0, 2, 4};
    DanNPCData npcStruct256 = {256, 10, 0, 1, 2};
    DanNPCData npcStruct258 = {258, 10, 1, 1, 1};
    DanNPCData npcStruct259 = {259, 10, 0, 1, 2};
    DanNPCData npcStruct441 = {441, 65, 0, 2, 4};
    DanNPCData npcStruct443 = {443, 70, 0, 2, 5};
    DanNPCData npcStruct445 = {445, 75, 0, 2, 4};
    DanNPCData npcStruct447 = {447, 73, 0, 2, 5};
    DanNPCData npcStruct451 = {451, 60, 0, 2, 4};
    DanNPCData npcStruct466 = {466, 60, 0, 2, 5};
    DanNPCData npcStruct467 = {467, 60, 0, 2, 5};
    DanNPCData npcStruct471 = {471, 25, 0, 2, 5};
    DanNPCData npcStruct472 = {472, 30, 0, 3, 5};
    DanNPCData npcStruct474 = {474, 20, 0, 2, 4};
    DanNPCData npcStruct475 = {475, 20, 0, 2, 4};
    DanNPCData npcStruct477 = {477, 20, 0, 2, 4};
    DanNPCData npcStruct478 = {478, 20, 0, 2, 4};
    DanNPCData npcStruct480 = {480, 20, 0, 2, 4};
    DanNPCData npcStruct481 = {481, 20, 0, 2, 4};
    DanNPCData npcStruct496 = {496, 40, 0, 2, 6};
    DanNPCData npcStruct497 = {497, 45, 0, 2, 5};
    DanNPCData npcStruct504 = {504, 20, 0, 3, 5}; // Bullet William Blaster
    DanNPCData npcStruct505 = {505, 55, 0, 2, 4};
    DanNPCData npcStruct507 = {507, 16, 0, 1, 2};
    DanNPCData npcStruct531 = {531, 25, 0, 1, 3}; // Green Shy Guy
    DanNPCData npcStruct532 = {532, 30, 0, 2, 4}; // Blue Shy Guy
    DanNPCData npcStruct533 = {533, 40, 0, 2, 5}; // Red Shy Guy
    DanNPCData npcStruct535 = {535, 15, 0, 1, 2}; // Dark Lakitu

    DanNPCData * DanNPC[] = {
        // Enemy tribe as [arg] aligns with "name"
        &npcStruct1,   // 1,
        &npcStruct2,   // 2,
        &npcStruct3,   // 3,
        &npcStruct4,   // 4,
        &npcStruct5,   // 5,
        &npcStruct6,   // 6,
        &npcStruct7,   // 7,
        &npcStruct8,   // 8,
        &npcStruct9,   // 9,
        &npcStruct10,  // 10,
        &npcStruct11,  // 11,
        &npcStruct12,  // 12,
        &npcStruct13,  // 13,
        nullptr,       // 14,
        &npcStruct15,  // 15,
        &npcStruct16,  // 16,
        &npcStruct17,  // 17,
        nullptr,       // 18,
        &npcStruct19,  // 19,
        &npcStruct20,  // 20,
        &npcStruct21,  // 21,
        &npcStruct22,  // 22,
        &npcStruct23,  // 23,
        &npcStruct24,  // 24,
        &npcStruct25,  // 25,
        &npcStruct26,  // 26,
        nullptr,       // 27,
        &npcStruct28,  // 28,
        nullptr,       // 29,
        &npcStruct30,  // 30,
        &npcStruct31,  // 31,
        &npcStruct32,  // 32,
        &npcStruct33,  // 33,
        nullptr,       // 34,
        &npcStruct35,  // 35,
        &npcStruct36,  // 36,
        &npcStruct37,  // 37,
        &npcStruct38,  // 38,
        &npcStruct39,  // 39,
        &npcStruct40,  // 40,
        nullptr,       // 41,
        &npcStruct42,  // 42,
        nullptr,       // 43,
        &npcStruct44,  // 44,
        nullptr,       // 45,
        &npcStruct46,  // 46,
        nullptr,       // 47,
        &npcStruct48,  // 48,
        nullptr,       // 49,
        &npcStruct50,  // 50,
        nullptr,       // 51,
        &npcStruct52,  // 52,
        nullptr,       // 53,
        &npcStruct54,  // 54,
        nullptr,       // 55,
        &npcStruct56,  // 56,
        nullptr,       // 57,
        &npcStruct58,  // 58,
        nullptr,       // 59,
        &npcStruct60,  // 60,
        nullptr,       // 61,
        &npcStruct62,  // 62,
        nullptr,       // 63,
        &npcStruct64,  // 64,
        &npcStruct65,  // 65,
        nullptr,       // 66,
        &npcStruct67,  // 67,
        &npcStruct68,  // 68,
        nullptr,       // 69,
        &npcStruct70,  // 70,
        nullptr,       // 71,
        nullptr,       // 72,
        nullptr,       // 73,
        &npcStruct74,  // 74,
        nullptr,       // 75,
        &npcStruct76,  // 76,
        nullptr,       // 77,
        nullptr,       // 78,
        nullptr,       // 79,
        nullptr,       // 80,
        nullptr,       // 81,
        nullptr,       // 82,
        nullptr,       // 83,
        &npcStruct84,  // 84,
        &npcStruct85,  // 85,
        &npcStruct86,  // 86,
        &npcStruct87,  // 87,
        nullptr,       // 88,
        &npcStruct89,  // 89,
        &npcStruct90,  // 90,
        &npcStruct91,  // 91,
        nullptr,       // 92,
        nullptr,       // 93,
        nullptr,       // 94,
        &npcStruct95,  // 95,
        &npcStruct96,  // 96,
        &npcStruct97,  // 97,
        &npcStruct98,  // 98,
        &npcStruct99,  // 99,
        &npcStruct100, // 100,
        &npcStruct101, // 101,
        &npcStruct102, // 102,
        &npcStruct103, // 103,
        nullptr,       // 104,
        &npcStruct105, // 105,
        nullptr,       // 106,
        nullptr,       // 107,
        nullptr,       // 108,
        nullptr,       // 109,
        &npcStruct110, // 110,
        nullptr,       // 111,
        &npcStruct112, // 112,
        nullptr,       // 113,
        &npcStruct114, // 114,
        nullptr,       // 115,
        &npcStruct116, // 116,
        nullptr,       // 117,
        &npcStruct118, // 118,
        nullptr,       // 119,
        nullptr,       // 120,
        nullptr,       // 121,
        nullptr,       // 122,
        &npcStruct123, // 123,
        &npcStruct124, // 124,
        &npcStruct125, // 125,
        &npcStruct126, // 126,
        &npcStruct127, // 127,
        nullptr,       // 128,
        &npcStruct129, // 129,
        nullptr,       // 130,
        &npcStruct131, // 131,
        nullptr,       // 132,
        nullptr,       // 133,
        nullptr,       // 134,
        &npcStruct135, // 135,
        nullptr,       // 136,
        &npcStruct137, // 137,
        nullptr,       // 138,
        &npcStruct139, // 139,
        nullptr,       // 140,
        nullptr,       // 141,
        nullptr,       // 142,
        &npcStruct143, // 143,
        nullptr,       // 144,
        &npcStruct145, // 145,
        nullptr,       // 146,
        &npcStruct147, // 147,
        nullptr,       // 148,
        nullptr,       // 149,
        nullptr,       // 150,
        &npcStruct151, // 151,
        nullptr,       // 152,
        &npcStruct153, // 153,
        nullptr,       // 154,
        nullptr,       // 155,
        nullptr,       // 156,
        &npcStruct157, // 157,
        &npcStruct158, // 158,
        nullptr,       // 159,
        &npcStruct160, // 160,
        &npcStruct161, // 161,
        &npcStruct162, // 162,
        nullptr,       // 163,
        &npcStruct164, // 164,
        &npcStruct165, // 165,
        &npcStruct166, // 166,
        &npcStruct167, // 167,
        &npcStruct168, // 168,
        &npcStruct169, // 169,
        &npcStruct170, // 170,
        &npcStruct171, // 171,
        &npcStruct172, // 172,
        nullptr,       // 173,
        &npcStruct174, // 174,
        nullptr,       // 175,
        nullptr,       // 176,
        nullptr,       // 177,
        &npcStruct178, // 178,
        &npcStruct179, // 179,
        &npcStruct180, // 180,
        nullptr,       // 181,
        nullptr,       // 182,
        nullptr,       // 183,
        nullptr,       // 184,
        &npcStruct185, // 185,
        nullptr,       // 186,
        nullptr,       // 187,
        nullptr,       // 188,
        &npcStruct189, // 189,
        &npcStruct190, // 190,
        nullptr,       // 191,
        nullptr,       // 192,
        &npcStruct193, // 193,
        nullptr,       // 194,
        nullptr,       // 195,
        nullptr,       // 196,
        nullptr,       // 197,
        nullptr,       // 198,
        nullptr,       // 199,
        nullptr,       // 200,
        &npcStruct201, // 201,
        &npcStruct202, // 202,
        nullptr,       // 203,
        nullptr,       // 204,
        nullptr,       // 205,
        &npcStruct206, // 206,
        nullptr,       // 207,
        &npcStruct208, // 208,
        nullptr,       // 209,
        &npcStruct210, // 210,
        nullptr,       // 211,
        &npcStruct212, // 212,
        nullptr,       // 213,
        nullptr,       // 214,
        &npcStruct215, // 215,
        nullptr,       // 216,
        nullptr,       // 217,
        &npcStruct218, // 218,
        nullptr,       // 219,
        nullptr,       // 220,
        &npcStruct221, // 221,
        nullptr,       // 222,
        nullptr,       // 223,
        nullptr,       // 224,
        &npcStruct225, // 225,
        &npcStruct226, // 226,
        nullptr,       // 227,
        &npcStruct228, // 228,
        &npcStruct229, // 229,
        nullptr,       // 230,
        &npcStruct231, // 231,
        nullptr,       // 232,
        nullptr,       // 233,
        &npcStruct234, // 234,
        nullptr,       // 235,
        nullptr,       // 236,
        nullptr,       // 237,
        &npcStruct238, // 238,
        nullptr,       // 239,
        &npcStruct240, // 240,
        nullptr,       // 241,
        &npcStruct242, // 242,
        nullptr,       // 243,
        nullptr,       // 244,
        nullptr,       // 245,
        &npcStruct246, // 246,
        &npcStruct247, // 247,
        &npcStruct248, // 248,
        &npcStruct249, // 249,
        &npcStruct250, // 250,
        nullptr,       // 251,
        nullptr,       // 252,
        nullptr,       // 253,
        nullptr,       // 254,
        nullptr,       // 255,
        &npcStruct256, // 256,
        nullptr,       // 257,
        &npcStruct258, // 258,
        &npcStruct259, // 259,
        nullptr,       // 260,
        nullptr,       // 261,
        nullptr,       // 262,
        nullptr,       // 263,
        nullptr,       // 264,
        nullptr,       // 265,
        nullptr,       // 266,
        nullptr,       // 267,
        nullptr,       // 268,
        nullptr,       // 269,
        nullptr,       // 270,
        nullptr,       // 271,
        nullptr,       // 272,
        nullptr,       // 273,
        nullptr,       // 274,
        nullptr,       // 275,
        nullptr,       // 276,
        nullptr,       // 277,
        nullptr,       // 278,
        nullptr,       // 279,
        nullptr,       // 280,
        nullptr,       // 281,
        nullptr,       // 282,
        nullptr,       // 283,
        nullptr,       // 284,
        nullptr,       // 285,
        nullptr,       // 286,
        nullptr,       // 287,
        nullptr,       // 288,
        nullptr,       // 289,
        nullptr,       // 290,
        nullptr,       // 291,
        nullptr,       // 292,
        nullptr,       // 293,
        nullptr,       // 294,
        nullptr,       // 295,
        nullptr,       // 296,
        nullptr,       // 297,
        nullptr,       // 298,
        nullptr,       // 299,
        nullptr,       // 300,
        nullptr,       // 301,
        nullptr,       // 302,
        nullptr,       // 303,
        nullptr,       // 304,
        nullptr,       // 305,
        nullptr,       // 306,
        nullptr,       // 307,
        nullptr,       // 308,
        nullptr,       // 309,
        nullptr,       // 310,
        nullptr,       // 311,
        nullptr,       // 312,
        nullptr,       // 313,
        nullptr,       // 314,
        nullptr,       // 315,
        nullptr,       // 316,
        nullptr,       // 317,
        nullptr,       // 318,
        nullptr,       // 319,
        nullptr,       // 320,
        nullptr,       // 321,
        nullptr,       // 322,
        nullptr,       // 323,
        nullptr,       // 324,
        nullptr,       // 325,
        nullptr,       // 326,
        nullptr,       // 327,
        nullptr,       // 328,
        nullptr,       // 329,
        nullptr,       // 330,
        nullptr,       // 331,
        nullptr,       // 332,
        nullptr,       // 333,
        nullptr,       // 334,
        nullptr,       // 335,
        nullptr,       // 336,
        nullptr,       // 337,
        nullptr,       // 338,
        nullptr,       // 339,
        nullptr,       // 340,
        nullptr,       // 341,
        nullptr,       // 342,
        nullptr,       // 343,
        nullptr,       // 344,
        nullptr,       // 345,
        nullptr,       // 346,
        nullptr,       // 347,
        nullptr,       // 348,
        nullptr,       // 349,
        nullptr,       // 350,
        nullptr,       // 351,
        nullptr,       // 352,
        nullptr,       // 353,
        nullptr,       // 354,
        nullptr,       // 355,
        nullptr,       // 356,
        nullptr,       // 357,
        nullptr,       // 358,
        nullptr,       // 359,
        nullptr,       // 360,
        nullptr,       // 361,
        nullptr,       // 362,
        nullptr,       // 363,
        nullptr,       // 364,
        nullptr,       // 365,
        nullptr,       // 366,
        nullptr,       // 367,
        nullptr,       // 368,
        nullptr,       // 369,
        nullptr,       // 370,
        nullptr,       // 371,
        nullptr,       // 372,
        nullptr,       // 373,
        nullptr,       // 374,
        nullptr,       // 375,
        nullptr,       // 376,
        nullptr,       // 377,
        nullptr,       // 378,
        nullptr,       // 379,
        nullptr,       // 380,
        nullptr,       // 381,
        nullptr,       // 382,
        nullptr,       // 383,
        nullptr,       // 384,
        nullptr,       // 385,
        nullptr,       // 386,
        nullptr,       // 387,
        nullptr,       // 388,
        nullptr,       // 389,
        nullptr,       // 390,
        nullptr,       // 391,
        nullptr,       // 392,
        nullptr,       // 393,
        nullptr,       // 394,
        nullptr,       // 395,
        nullptr,       // 396,
        nullptr,       // 397,
        nullptr,       // 398,
        nullptr,       // 399,
        nullptr,       // 400,
        nullptr,       // 401,
        nullptr,       // 402,
        nullptr,       // 403,
        nullptr,       // 404,
        nullptr,       // 405,
        nullptr,       // 406,
        nullptr,       // 407,
        nullptr,       // 408,
        nullptr,       // 409,
        nullptr,       // 410,
        nullptr,       // 411,
        nullptr,       // 412,
        nullptr,       // 413,
        nullptr,       // 414,
        nullptr,       // 415,
        nullptr,       // 416,
        nullptr,       // 417,
        nullptr,       // 418,
        nullptr,       // 419,
        nullptr,       // 420,
        nullptr,       // 421,
        nullptr,       // 422,
        nullptr,       // 423,
        nullptr,       // 424,
        nullptr,       // 425,
        nullptr,       // 426,
        nullptr,       // 427,
        nullptr,       // 428,
        nullptr,       // 429,
        nullptr,       // 430,
        nullptr,       // 431,
        nullptr,       // 432,
        nullptr,       // 433,
        nullptr,       // 434,
        nullptr,       // 435,
        nullptr,       // 436,
        nullptr,       // 437,
        nullptr,       // 438,
        nullptr,       // 439,
        nullptr,       // 440,
        &npcStruct441, // 441,
        nullptr,       // 442,
        &npcStruct443, // 443,
        nullptr,       // 444,
        &npcStruct445, // 445,
        nullptr,       // 446,
        &npcStruct447, // 447,
        nullptr,       // 448,
        nullptr,       // 449,
        nullptr,       // 450,
        &npcStruct451, // 451,
        nullptr,       // 452,
        nullptr,       // 453,
        nullptr,       // 454,
        nullptr,       // 455,
        nullptr,       // 456,
        nullptr,       // 457,
        nullptr,       // 458,
        nullptr,       // 459,
        nullptr,       // 460,
        nullptr,       // 461,
        nullptr,       // 462,
        nullptr,       // 463,
        nullptr,       // 464,
        nullptr,       // 465,
        &npcStruct466, // 466,
        &npcStruct467, // 467,
        nullptr,       // 468,
        nullptr,       // 469,
        nullptr,       // 470,
        &npcStruct471, // 471,
        &npcStruct472, // 472,
        nullptr,       // 473,
        &npcStruct474, // 474,
        &npcStruct475, // 475,
        nullptr,       // 476,
        &npcStruct477, // 477,
        &npcStruct478, // 478,
        nullptr,       // 479,
        &npcStruct480, // 480,
        &npcStruct481, // 481,
        nullptr,       // 482,
        nullptr,       // 483,
        nullptr,       // 484,
        nullptr,       // 485,
        nullptr,       // 486,
        nullptr,       // 487,
        nullptr,       // 488,
        nullptr,       // 489,
        nullptr,       // 490,
        nullptr,       // 491,
        nullptr,       // 492,
        nullptr,       // 493,
        nullptr,       // 494,
        nullptr,       // 495,
        &npcStruct496, // 496,
        &npcStruct497, // 497,
        nullptr,       // 498,
        nullptr,       // 499,
        nullptr,       // 500,
        nullptr,       // 501,
        nullptr,       // 502,
        nullptr,       // 503,
        &npcStruct504, // 504,
        &npcStruct505, // 505,
        nullptr,       // 506,
        &npcStruct507, // 507
        nullptr,       // 508
        nullptr,       // 509
        nullptr,       // 510
        nullptr,       // 511
        nullptr,       // 512
        nullptr,       // 513
        nullptr,       // 514
        nullptr,       // 515
        nullptr,       // 516
        nullptr,       // 517
        nullptr,       // 518
        nullptr,       // 519
        nullptr,       // 520
        nullptr,       // 521
        nullptr,       // 522
        nullptr,       // 523
        nullptr,       // 524
        nullptr,       // 525
        nullptr,       // 526
        nullptr,       // 527
        nullptr,       // 528
        nullptr,       // 529
        nullptr,       // 530
        &npcStruct531, // 531
        &npcStruct532, // 532
        &npcStruct533, // 533
        nullptr,       // 534
        &npcStruct535  // 535
    };

    DanLevelData level1 = {45, 85, 100, 0, 0};
    DanLevelData level2 = {25, 40, 80, 0, 1};
    DanLevelData level3 = {10, 25, 60, 0, 2};
    DanLevelData level4 = {0, 15, 40, 1, 3};

    // Enemies are selected from these arrays based on the current floor, which determines which DanLevelData to use
    NPCTribeId lv1Tribes[] = {
        NPC_GOOMBA,
        NPC_FLIP_GOOMBA,
        NPC_PARAGOOMBA,
        NPC_GLOOMBA,
        NPC_SPIKED_GOOMBA,
        NPC_KOOPA_TROOPA,
        NPC_KOOPA_TROOPA_STG3,
        NPC_RED_KOOPA,
        NPC_FLIP_RED_KOOPA,
        NPC_PARATROOPA,
        NPC_PARATROOPA_STG3,
        NPC_BUZZY_BEETLE,
        NPC_STONE_BUZZY,
        NPC_BOO,
        NPC_GREEN_CHEEP_CHEEP,
        NPC_RED_CHEEP_CHEEP,
        NPC_FLIP_GREEN_CHEEP_CHEEP,
        NPC_BALD_CLEFT,
        NPC_SQUIGLET,
        NPC_SWAMPIRE, // Red Shy Guy
        NPC_SQUIG,
        NPC_SPROING_OING,
        NPC_BOING_OING,
        NPC_BOOMBOXER,
        NPC_GROWMEBA,
        NPC_JELLIEN,
        NPC_CURSYA,
        NPC_CHERBIL,
        NPC_POISON_CHERBIL,
        NPC_FRACKLE,
        NPC_SPINIA,
        NPC_FLIP_BUZZY_BEETLE,
        NPC_DARK_RUFF_PUFF,
        NPC_JAWBUS};

    NPCTribeId lv2Tribes[] = {
        NPC_UNUSED_SPIKED_GOOMBA,
        NPC_RED_PARATROOPA,
        NPC_FLIP_RED_PARATROOPA,
        NPC_SPIKE_TOP,
        NPC_PARABUZZY,
        NPC_SPINY,
        NPC_FLIP_SPINY,
        NPC_HAMMER_BRO,
        NPC_MAGIKOOPA,
        NPC_FUZZY,
        NPC_POKEY,
        NPC_SPANIA,
        NPC_CHAIN_CHOMP,
        NPC_SQUOG,
        NPC_BEEPBOXER,
        NPC_MISTER_I,
        NPC_SHLURP,
        NPC_TILEOID_G,
        NPC_TILEOID_B,
        NPC_GREEN_FUZZY, // Blue Shy Guy
        NPC_RAWBUS,
        NPC_PATROLMEOW,
        NPC_LONGATOR,
        NPC_PIGARITHM,
        NPC_FLORO_SAPIEN_YELLOW,
        NPC_FLORO_SAPIEN_PURPLE,
        NPC_FLORO_SAPIEN_RED,
        NPC_NINJOE,
        NPC_TECH_CURSYA,
        NPC_HEAVY_CURSYA,
        NPC_REVERSYA_CURSYA,
        NPC_ICE_CHERBIL,
        NPC_FLIP_SPIKE_TOP,
        NPC_MOON_CLEFT,
        NPC_PURPLE_MUTH,
        NPC_GREEN_BOOMBOXER};

    NPCTribeId lv3Tribes[] = {
        NPC_HEADBONK_GOOMBA,
        NPC_KOOPATROL,
        NPC_SPIKY_PARABUZZY,
        NPC_DRY_BONES,
        NPC_FLIP_HAMMER_BRO,
        NPC_BOOMERANG_BRO,
        NPC_FIRE_BRO,
        NPC_BROOM_MAGIKOOPA,
        NPC_KOOPA_STRIKER,
        NPC_BILL_BLASTER,
        NPC_DARK_BOO,
        NPC_CLUBBA,
        NPC_PINK_FUZZY,
        NPC_DARK_CLEFT,
        NPC_RUFF_PUFF,
        NPC_CRAZEE_DAYZEE,
        NPC_ZOING_OING,
        NPC_BLOMEBA,
        NPC_TILEOID_R,
        NPC_TILEOID_Y,
        NPC_MEOWBOMB_SURPRISEMEOW,
        NPC_FOTON,
        NPC_HOOLIGON,
        NPC_GOLD_FUZZY, // Green Shy Guy
        NPC_COPTA,
        NPC_HOGARITHM,
        NPC_NINJOHN,
        NPC_FLIP_SKELLOBIT,
        NPC_FLIP_SKELLOBOMBER_HEAD,
        NPC_MUTH,
        NPC_SPUNIA,
        NPC_DARK_GOOMBA,
        NPC_DARK_SPIKED_GOOMBA,
        NPC_DARK_PARAGOOMBA,
        NPC_DARK_JAWBUS,
        NPC_DARK_KOOPA,
        NPC_DARK_PARATROOPA,
        NPC_FLIP_BOOMERANG_BRO,
        NPC_FLIP_FIRE_BRO,
        NPC_SOOPA_STRIKER,
        NPC_DARK_FUZZY,
        NPC_DARK_FIRE_BRO,
        NPC_RED_MAGIKOOPA,
        NPC_RED_BROOM_MAGIKOOPA,
        NPC_WHITE_MAGIKOOPA,
        NPC_WHITE_BROOM_MAGIKOOPA,
        NPC_GREEN_MAGIKOOPA,
        NPC_LAKITU,
        NPC_FLIP_LAKITU,
        NPC_GREEN_BROOM_MAGIKOOPA};

    NPCTribeId lv4Tribes[] = {
        NPC_DULL_BONES,
        NPC_PHANTOM_EMBER, // Dark Lakitu
        NPC_POISON_POKEY,
        NPC_AMAZY_DAYZEE,
        NPC_RED_CHOMP,
        NPC_DARK_CHOMP, // Gold Chomp
        NPC_SQUOINKER,
        NPC_BLASTBOXER,
        NPC_RED_I,
        NPC_SHLORP,
        NPC_CHROMEBA,
        NPC_GAWBUS,
        NPC_LONGADILE,
        NPC_NINJERRY,
        NPC_FLIP_SPIKY_SKELLOBIT,
        NPC_FLIP_SKELLOBOMBER_WITH_SKELLOBAIT,
        NPC_MAGIBLOT_B,
        NPC_MAGIBLOT_R,
        NPC_MAGIBLOT_Y,
        NPC_BACK_CURSYA,
        NPC_GIGABITE,
        NPC_MEGA_MUTH,
        NPC_SHADY_KOOPA,
        NPC_FLIP_SHADY_KOOPA,
        NPC_DARK_CHERBIL,           // Ash Cherbil
        NPC_DARK_SPINY,             // Sky-Blue Spiny
        NPC_DARK_SPIKE_TOP,         // Red Spike Top
        NPC_DARK_DULL_BONES,        // Dark Bones
        NPC_DARK_CLUBBA,            // White Clubba
        NPC_DARK_DARK_BOO,          // Bomb Boo
        NPC_DARK_MAGIKOOPA,         // Shady Magikoopa
        NPC_DARK_BROOM_MAGIKOOPA,   // Shady Broom Magikoopa
        NPC_DARK_TILEOID,           // Tileoid PU
        NPC_DARK_NINJOE,            // Ninjeremiah
        NPC_DARK_KOOPATROL,         // ditto lulw
        NPC_DARK_HEADBONK_GOOMBA,   // Kamikaze Goomba
        NPC_DARK_SKELLOBIT,         // Skellobyte
        NPC_DARK_SPIKY_SKELLOBIT,   // Spiky Skellobyte
        NPC_DARK_HAMMER_BRO,        // Shady Hammer Bro
        NPC_DARK_BOOMERANG_BRO,     // Shady Boomerang Bro
        NPC_BOMBSHELL_BILL_BLASTER, // William Blaster
        NPC_DARK_STRIKER            // Shady Striker
    };

    DanNPCData ** danNpcGetPtr() {
        return (DanNPCData **)DanNPC;
    }

    DanLevelData * danLevelDataPtr(s32 level) {
        switch (level) {
        case 1:
            return &level1;
            break;
        case 2:
            return &level2;
            break;
        case 3:
            return &level3;
            break;
        default:
            return &level4;
            break;
        }
        return nullptr;
    }

    s32 npcGetDanLv(s32 tribeId) {
        NPCTribeId * arrays[4] = {lv1Tribes, lv2Tribes, lv3Tribes, lv4Tribes};
        s32 sizes[4] = {ARRAY32_COUNT(lv1Tribes), ARRAY32_COUNT(lv2Tribes), ARRAY32_COUNT(lv3Tribes), ARRAY32_COUNT(lv4Tribes)};
        for (s32 i = 0; i < 4; i += 1) {
            NPCTribeId * array = arrays[i];
            for (s32 j = 0; j < sizes[i]; j += 1) {
                if (tribeId == array[j])
                    return i + 1;
            }
        }
        return 0;
    }

    /*
        GEN
    */

    static double boobies = 0;

    // Set the Pit Flimm inventory
    s32 newRotenShopItems[] = {
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        -1};

    // Pit Flimm item pool to iterate from
    s32 rotenShopItemPool[] = {81, 82, 84, 89, 90, 95, 99, 106, 106, 106, 106, 118, 123, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140,
                               141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 160, 161, 162, 163, 164, 166, 168, 169, 170,
                               171, 173, 174, 177, 178, 180, 181, 182, 183, 184, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 198, 199, 200,
                               201, 202, 203, 204, 205, 207, 208, 209, 211, 212, 214, 215};

    s32 rotenShopLowerClassItemPool[] = {65, 66, 67, 68, 69, 70, 73, 74, 75, 76, 77, 78, 79, 83, 86, 98, 104, 109, 113};

    bool DanGen_Items_VerifyNotInBlocklist(s32 itemId, s32 * blocklist, s32 blocklistCnt) {
        for (s32 j = 0; j < blocklistCnt; j += 1) {
            if (itemId == blocklist[j])
                return false;
        }
        return true;
    }

    s32 DanGen_Items_SelectItem(s32 itemRarity, s32 * blocklist, s32 blocklistCnt, bool guaranteeSpecial) {
        s32 itemId = 0, i = 0;
        for (; i < 50; i += 1) {
            switch (itemRarity) {
            case 0:
                itemId = RFCItems_Common[system::rand() % RFCItems_Common_Size];
                break;
            case 1:
                itemId = RFCItems_Uncommon[system::rand() % RFCItems_Uncommon_Size];
                break;
            case 2:
                itemId = RFCItems_Rare[system::rand() % RFCItems_Rare_Size];
                break;
            default:
                itemId = RFCItems_Legendary[system::rand() % RFCItems_Legendary_Size];
                break;
            }
            if (!DanGen_Items_VerifyNotInBlocklist(itemId, blocklist, blocklistCnt))
                continue;
            if (itemId >= RFC_SPECIAL_START) {
                if ((itemId - RFC_SPECIAL_START) >= (s32)VOUCHER_RED && (itemId - RFC_SPECIAL_START) <= (s32)VOUCHER_BLACK) // instantly reroll, rework to check for that disorder completed later
                    continue;
                if (Lunatic->RFC.rfcSpecialObtained[itemId - RFC_SPECIAL_START] == true)
                    continue;
                if ((system::rand() % 100) < 20 && !guaranteeSpecial) // 20% chance to fail at rolling the special item
                    continue;
            } else if (itemId < RFC_SPECIAL_START && guaranteeSpecial)
                continue;
            break;
        }
        if (i >= 50)
            return -1;
        return itemId;
    }

    void DanGen_Items(bool onRoomLoad) {
        s32 rarity = 0, itemRarity = 0, odds = 0, i = 0, itemId = 0;
        VoucherState vState = VoucherGetStateById(VOUCHER_STELLAR);
        if (onRoomLoad) {
            Lunatic->RFC.rerolls = 0;
            Lunatic->RFC.rerollCost = 1;
            if (vState == V_ACTIVE) {
                VoucherProc(VOUCHER_STELLAR);
                Lunatic->RFC.rerollCost = 0;
            }
        } else {
            Lunatic->RFC.rerolls += 1;
            Lunatic->RFC.rerollCost += 1;
        }
        // Reset existing items
        msl::string::memset(Lunatic->RFC.rfcItemIds, 0, sizeof(Lunatic->RFC.rfcItemIds));
        // Decides chest rarity; tries again if a Reroll hits the same rarity OR Stellar is active
        do {
            odds = system::rand() % 100;
            rarity = 0;
            if (odds < 60) // 30% for Uncommon
                rarity = 1;
            if (odds < 30) // 20% for Rare
                rarity = 2;
            if (odds < 10) // 10% for Legendary
                rarity = 3;
        } while ((!onRoomLoad && rarity == Lunatic->RFC.chestRarity) || (vState == V_ACTIVE && (rarity == Lunatic->RFC.chestRarity || rarity == 0)));
        // First rest floor should start with a common or uncommon chest
        if (onRoomLoad && swdrv::swByteGet(1) == 10) {
            rarity = 0;
            odds = system::rand() % 100;
            if (odds < 40)
                rarity = 1;
        }
        // Sets chest rarity and open cost
        Lunatic->RFC.chestRarity = rarity;
        Lunatic->RFC.chestKeys = rarity + 1;
        // Select 3 items
        for (i = 0; i < 3; i += 1) {
            itemRarity = rarity;
            // Small chance for item rarity to increment if rarity isn't legendary
            if (itemRarity != 3) {
                odds = system::rand() % 100;
                // 10% chance to select a higher-tier item
                if (odds < 10)
                    itemRarity += 1;
            }
            // Pull item from array
            itemId = DanGen_Items_SelectItem(itemRarity, Lunatic->RFC.rfcItemIds, 3, false);
            if (itemId == -1 || (!onRoomLoad && DanGen_Items_VerifyNotInBlocklist(itemId, Lunatic->Mitch.itemIds, 6) == false)) {
                i -= 1;
                continue;
            }
            Lunatic->RFC.rfcItemIds[i] = itemId;
            customwin::CWSelectItemDesc * Desc = &Lunatic->RFC.rfcItems[i];
            RFCItemData * RFC_SpecialItems = (RFCItemData *)RFCSpecialGetPtr();
            RFCColorDef * RFC_Colors = (RFCColorDef *)RFCColorsGetPtr();
            msl::string::memset(Desc, 0, sizeof(customwin::CWSelectItemDesc));
            if (itemId >= RFC_SPECIAL_START) {
                Desc->nameTxt = (char *)memory::__memAlloc(memory::HEAP_MAP, 32);
                Desc->descTxt = (char *)memory::__memAlloc(memory::HEAP_MAP, 300);
                Desc->iconId = TPLPATCH_ICON((s32)RFC_SpecialItems[itemId - RFC_SPECIAL_START].iconId);
                msl::string::strcpy(Desc->nameTxt, RFC_SpecialItems[itemId - RFC_SPECIAL_START].name);
                if ((itemId - RFC_SPECIAL_START) >= VOUCHER_CAKE && (itemId - RFC_SPECIAL_START) <= VOUCHER_BLACK) // Check if voucher
                    msl::stdio::sprintf(Desc->descTxt, RFC_SpecialItems[itemId - RFC_SPECIAL_START].description, VoucherGetTearChance(VoucherTearChances[itemId - RFC_SPECIAL_START]), VoucherGuaranteeTrigs[itemId - RFC_SPECIAL_START]);
                else
                    msl::string::memcpy(Desc->descTxt, RFC_SpecialItems[itemId - RFC_SPECIAL_START].description, msl::string::strlen(RFC_SpecialItems[itemId - RFC_SPECIAL_START].description));
                OSREPORTF("RFC: Special item %d generated. Icon id set to %d. Name: %s\n", itemId, Desc->iconId, Desc->nameTxt);
            } else {
                Desc->itemId = itemId;
                Desc->iconId = -1;
            }
            Desc->nameColor = RFC_Colors[itemRarity].textCol;
            Lunatic->RFC.rfcItemData[i] = &RFC_SpecialItems[itemId - RFC_SPECIAL_START];
        }
        OSREPORTF("RFC: Chest rarity is %d. Items are %d, %d, %d.\n", rarity, Lunatic->RFC.rfcItemIds[0], Lunatic->RFC.rfcItemIds[1], Lunatic->RFC.rfcItemIds[2]);
        return;
    }

    void DanGen_SegmentsAndDoors(s32 currentFloor) {
        s32 activeDoorCount = 19; // Actually 20, but let's not complicate things
        // s32 doorOn[31] = {2, 3, 6, 7, 9, 12, 13, 16, 18, 19, 22, 23, 25, 26, 27, 28, 29, 30, 31, 32};
        s32 roomDecCode = 0;
        s32 segmentCount = 0;
        bool structureGenerated = true;
        bool segment400 = false;
        bool segment1 = false;
        bool segment800 = false;
        bool segment80 = false;
        bool segment40 = false;
        bool segment2 = false;
        bool segment2000 = false;
        bool segment1000 = false;
        bool segment8 = false;
        bool segment4 = false;
        bool segment4000 = false;
        bool segment200 = false;
        bool segment100 = false;
        bool segment10 = false;
        bool segment8000 = false;
        bool segment20 = false;
        bool segment10000 = false;
        bool segment20000 = false;
        bool segment40000 = false;
        bool segment80000 = false;

        if (currentFloor != 198) {
            // Determine # of segments to generate for current room
            s32 segmentRNG = system::rand() % 152;
            if (segmentRNG >= 0 && segmentRNG < 42) {
                segmentCount = 3;
            } else if (segmentRNG >= 42 && segmentRNG < 45) {
                segmentCount = 1;
            } else if (segmentRNG >= 45 && segmentRNG < 109) {
                segmentCount = 4;
            } else if (segmentRNG >= 109 && segmentRNG < 122) {
                segmentCount = 2;
            } else if (segmentRNG >= 122 && segmentRNG < 124) {
                segmentCount = 0;
            } else
                segmentCount = 5;
            s32 disorderId = Lunatic->Luna.disorder;
            if (disorderId == DisorderId::DISORDER_GREEN) // Guarantee a maximally complex layout while Indifference is active
                segmentCount = 16;
            // OSREPORTF("Rolled segmentRNG = %d, queueing %d segment generations for this room.\n", segmentRNG, segmentCount);

            /*
                ENCLOSED STRUCTURE GENERATION
                (If more than 2 segments are generated at any time, the remainder is subtracted from segmentCount)
            */
            s32 i = 0;
            s32 structureRNG = system::rand() % 180;
            s32 structureMiscRNG = system::rand() % 100;
            if (disorderId == DisorderId::DISORDER_GREEN && structureRNG > 30) // Guarantee an enclosed structure while Indifference is active
                structureRNG = system::rand() % 31;

            // SPLIT
            if (structureRNG >= 0 && structureRNG < 8) {
                // OSREPORTF("!!! SPLIT Structure is generating !!!\n");
                segment2000 = true;
                segment8 = true;
                if (structureMiscRNG < 70) {
                    segment20000 = true;
                } else if (structureMiscRNG < 92) {
                    segment80000 = true;
                } else {
                    segment1 = true;
                    segment20 = true;
                    segment400 = true;
                    segment8000 = true;
                    segment10000 = true;
                    segment20000 = true;
                    segment40000 = true;
                    segment80000 = true;
                    segmentCount = segmentCount - 4;
                }
            }

            // Quarter
            else if (structureRNG >= 8 && structureRNG < 19) {
                // OSREPORTF("!!! QUARTER Structure is generating !!!\n");
                if (structureMiscRNG < 31) {
                    segment100 = true;
                    segment8 = true;
                    if (segment40000) {
                        if (boobies < 50)
                            segment80000 = true;
                    } else
                        segment80000 = true;
                } else if (structureMiscRNG < 54) {
                    segment40 = true;
                    segment8 = true;
                    if (segment40000) {
                        if (boobies < 50)
                            segment80000 = true;
                    } else
                        segment80000 = true;
                } else if (structureMiscRNG < 77) {
                    segment100 = true;
                    segment2000 = true;
                    if (segment10000) {
                        if (boobies < 50)
                            segment20000 = true;
                    } else
                        segment20000 = true;
                } else {
                    segment40 = true;
                    segment2000 = true;
                    if (segment10000) {
                        if (boobies < 50)
                            segment20000 = true;
                    } else
                        segment20000 = true;
                }
            }

            // Cube
            else if (structureRNG >= 19 && structureRNG < 21) {
                // OSREPORTF("!!! CUBE Structure is generating !!!\n");
                segmentCount = segmentCount - 2;
                if (structureMiscRNG < 50) {
                    segment400 = true;
                    segment80 = true;
                    segment1 = true;
                    segment20 = true;
                    segment20000 = true;
                    segment40000 = true;
                } else {
                    segment8000 = true;
                    segment200 = true;
                    segment20 = true;
                    segment1 = true;
                    segment20000 = true;
                    segment40000 = true;
                }
            }

            // Pillar
            else if (structureRNG >= 21 && structureRNG < 23) {
                // OSREPORTF("!!! PILLAR Structure is generating !!!\n");
                if (structureMiscRNG < 60) {
                    segment800 = true;
                    segment80 = true;
                    segment2 = true;
                } else {
                    segment4000 = true;
                    segment200 = true;
                    segment10 = true;
                }
                if (boobies < 50) {
                    segment20000 = true;
                } else
                    segment80000 = true;
            }

            // Small
            else if (structureRNG >= 23 && structureRNG < 26) {
                // OSREPORTF("!!! SMALL Structure is generating !!!\n");
                if (structureMiscRNG < 33) {
                    segment800 = true;
                    segment400 = true;
                    segment10 = true;
                    segment20 = true;
                    segment8000 = true;
                    segment10000 = true;
                    segment80000 = true;
                    segmentCount = segmentCount - 3;
                } else if (structureMiscRNG < 67) {
                    segment8000 = true;
                    segment4000 = true;
                    segment1 = true;
                    segment2 = true;
                    segment400 = true;
                    segment10000 = true;
                    segment80000 = true;
                    segmentCount = segmentCount - 3;
                } else {
                    if (boobies < 25) {
                        segment8000 = true;
                        segment4000 = true;
                        segment400 = true;
                        segment10000 = true;
                        segmentCount = segmentCount - 1;
                    } else if (boobies < 50) {
                        segment800 = true;
                        segment400 = true;
                        segment8000 = true;
                        segment10000 = true;
                        segmentCount = segmentCount - 1;
                    } else if (boobies < 75) {
                        segment10 = true;
                        segment20 = true;
                        segment80000 = true;
                    } else {
                        segment1 = true;
                        segment2 = true;
                        segment80000 = true;
                    }
                }
            }

            // I'm sorry Tartt but I am leaving the stream during Lobates. They bore the hell out of me and frankly I can think of a whole host of things I'd rather be doing, including playing with a rubber band. Please
            else if (structureRNG >= 26 && structureRNG < 32) {
                // OSREPORTF("!!! LOBATE Structure is generating !!!\n");
                segmentCount = segmentCount - 2;
                if (structureMiscRNG <= 66) {
                    if (boobies < 25) {
                        segment4 = true;
                        segment8 = true;
                        segment200 = true;
                        segment8000 = true;
                        segment20000 = true;
                    } else if (boobies < 50) {
                        segment4 = true;
                        segment8 = true;
                        segment80 = true;
                        segment400 = true;
                        segment20000 = true;
                    } else if (boobies < 75) {
                        segment1000 = true;
                        segment2000 = true;
                        segment200 = true;
                        segment20 = true;
                        segment20000 = true;
                    } else {
                        segment1000 = true;
                        segment2000 = true;
                        segment80 = true;
                        segment1 = true;
                        segment20000 = true;
                    }
                } else {
                    if (structureMiscRNG >= 80) {
                        segment20000 = true;
                    } else {
                        segment80000 = true;
                    }
                    if (boobies < 12.5) {
                        segment4 = true;
                        segment8 = true;
                        segment80 = true;
                        segment800 = true;
                    } else if (boobies < 25) {
                        segment4 = true;
                        segment8 = true;
                        segment200 = true;
                        segment4000 = true;
                    } else if (boobies < 37.5) {
                        segment1000 = true;
                        segment2000 = true;
                        segment80 = true;
                        segment2 = true;
                    } else if (boobies < 50) {
                        segment1000 = true;
                        segment2000 = true;
                        segment200 = true;
                        segment10 = true;
                    } else if (boobies < 62.5) {
                        segment2 = true;
                        segment4 = true;
                        segment200 = true;
                        segment4000 = true;
                    } else if (boobies < 75) {
                        segment10 = true;
                        segment200 = true;
                        segment800 = true;
                        segment1000 = true;
                    } else if (boobies < 87.5) {
                        segment4 = true;
                        segment10 = true;
                        segment80 = true;
                        segment800 = true;
                    } else {
                        segment2 = true;
                        segment80 = true;
                        segment1000 = true;
                        segment4000 = true;
                    }
                }
            }
            // Structure does not generate
            else
                structureGenerated = false;

            // OSREPORTF("Structure check passed with RNG val %d, %d segments to generate remaining.\n", structureRNG, segmentCount);

            /*
                SEGMENT GENERATION PROCEDURE & EVERY NECESSARY CHECK
                (This made me want to kill myself. Badly. It took weeks back in summer 2024 but it was worth)
            */
            while (segmentCount > 0 && i < 20) {
                s32 segmentID = system::rand() % 16;
                switch (segmentID) {
                case 0:
                    // Segment 400
                    if ((segment400) ||
                        (segment800) ||
                        (segment1000 && segment2000) ||
                        (segment1000 && segment4000) ||
                        (segment1000 && segment8000) ||
                        (segment10 && segment200 && segment1000) ||
                        (segment2 && segment80) ||
                        (segment1 && segment4 && segment200 && segment1000) ||
                        (segment1 && segment4 && segment8 && segment2000) ||
                        (segment1 && segment80) ||
                        (segment40 && segment80) ||
                        (segment4 && segment8 && segment80) ||
                        (segment4 && segment10 && segment80) ||
                        (segment4 && segment20 && segment80) ||
                        (segment10 && segment200 && segment1000) ||
                        (segment20 && segment200 && segment1000) ||
                        (segment80 && segment4 && segment200 && segment4000) ||
                        (segment80 && segment4 && segment200 && segment8000) ||
                        (segment80 && segment4 && segment200 && segment100) ||
                        (segment1000 && segment200 && segment4 && segment2) ||
                        (segment1000 && segment200 && segment4 && segment8) ||
                        (segment100 && segment200 && segment1000)) {
                        break;
                    } else {
                        segment400 = true;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 1;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg400 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 1:
                    // Segment 1
                    if ((segment1) ||
                        (segment2) ||
                        (segment4 && segment8) ||
                        (segment4 && segment10) ||
                        (segment4 && segment20) ||
                        (segment4 && segment200 && segment4000) ||
                        (segment80 && segment800) ||
                        (segment4 && segment200 && segment400 && segment1000) ||
                        (segment8 && segment400 && segment1000 && segment2000) ||
                        (segment80 && segment400) ||
                        (segment40 && segment80) ||
                        (segment80 && segment1000 && segment2000) ||
                        (segment80 && segment1000 && segment4000) ||
                        (segment80 && segment1000 && segment8000) ||
                        (segment4 && segment200 && segment4000) ||
                        (segment4 && segment200 && segment8000) ||
                        (segment80 && segment1000 && segment200 && segment10) ||
                        (segment80 && segment1000 && segment200 && segment20) ||
                        (segment80 && segment1000 && segment200 && segment100) ||
                        (segment800 && segment1000 && segment200 && segment4) ||
                        (segment2000 && segment1000 && segment200 && segment4) ||
                        (segment4 && segment100 && segment200)) {
                        break;
                    } else {
                        segment1 = true;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 17;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg1 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 2:
                    // Segment 40
                    if ((segment80 && segment800) ||
                        (segment40) ||
                        (segment2 && segment80) ||
                        (segment80 && segment400) ||
                        (segment1 && segment80) ||
                        (segment2000) ||
                        (segment8) ||
                        (segment100) ||
                        (segment4 && segment20 && segment80) ||
                        (segment4 && segment10 && segment80) ||
                        (segment80 && segment1000 && segment8000) ||
                        (segment80 && segment1000 && segment200 && segment10) ||
                        (segment80 && segment1000 && segment200 && segment20) ||
                        (segment80 && segment1000 && segment200 && segment100) ||
                        (segment80 && segment4 && segment200 && segment4000) ||
                        (segment80 && segment4 && segment200 && segment8000) ||
                        (segment80 && segment4 && segment200 && segment100) ||
                        (segment80 && segment1000 && segment4000)) {
                        break;
                    } else {
                        segment40 = true;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 10;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 11;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg40 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 3:
                    // Segment 1000
                    if ((segment400 && segment8000) ||
                        (segment400 && segment4000) ||
                        (segment800 && segment8000) ||
                        (segment1000) ||
                        (segment800 && segment4000) ||
                        (segment800 && segment2000) ||
                        (segment2000 && segment4000) ||
                        (segment2000 && segment8000) ||
                        (segment400 && segment2000) ||
                        (segment2 && segment10 && segment80 && segment200) ||
                        (segment4 && segment80 && segment200) ||
                        (segment40 && segment80 && segment2000) ||
                        (segment4 && segment8 && segment80 && segment2000) ||
                        (segment4 && segment8 && segment200 && segment2000) ||
                        (segment2 && segment8 && segment80 && segment2000) ||
                        (segment8 && segment10 && segment200 && segment2000) ||
                        (segment2 && segment80 && segment8000) ||
                        (segment2 && segment80 && segment4000) ||
                        (segment2 && segment80 && segment2000) ||
                        (segment1 && segment80 && segment8000) ||
                        (segment1 && segment80 && segment4000) ||
                        (segment1 && segment80 && segment2000) ||
                        (segment40 && segment80 && segment8000) ||
                        (segment40 && segment80 && segment4000) ||
                        (segment40 && segment80 && segment2000) ||
                        (segment10 && segment200 && segment800) ||
                        (segment10 && segment200 && segment400) ||
                        (segment10 && segment200 && segment2000) ||
                        (segment20 && segment200 && segment800) ||
                        (segment20 && segment200 && segment400) ||
                        (segment20 && segment200 && segment2000) ||
                        (segment100 && segment200 && segment800) ||
                        (segment100 && segment200 && segment2000) ||
                        (segment1 && segment4 && segment200 && segment400) ||
                        (segment4 && segment20 && segment80 && segment8000) ||
                        (segment2 && segment80 && segment200 && segment20) ||
                        (segment2 && segment80 && segment200 && segment100) ||
                        (segment1 && segment80 && segment200 && segment10) ||
                        (segment1 && segment80 && segment200 && segment20) ||
                        (segment1 && segment80 && segment200 && segment100) ||
                        (segment40 && segment80 && segment200 && segment10) ||
                        (segment40 && segment80 && segment200 && segment20) ||
                        (segment40 && segment80 && segment200 && segment100) ||
                        (segment400 && segment200 && segment4 && segment2) ||
                        (segment400 && segment200 && segment4 && segment8) ||
                        (segment800 && segment200 && segment4 && segment1) ||
                        (segment800 && segment200 && segment4 && segment2) ||
                        (segment800 && segment200 && segment4 && segment8) ||
                        (segment2000 && segment200 && segment4 && segment1) ||
                        (segment2000 && segment200 && segment4 && segment2) ||
                        (segment2000 && segment200 && segment4 && segment8) ||
                        (segment8000 && segment80 && segment4 && segment10) ||
                        (segment8000 && segment80 && segment4 && segment8) ||
                        (segment4000 && segment80 && segment4 && segment20) ||
                        (segment4000 && segment80 && segment4 && segment10) ||
                        (segment4000 && segment80 && segment4 && segment8) ||
                        (segment2000 && segment80 && segment4 && segment20) ||
                        (segment2000 && segment80 && segment4 && segment10) ||
                        (segment2000 && segment80 && segment4 && segment8) ||
                        (segment100 && segment200 && segment400)) {
                        break;
                    } else {
                        segment1000 = true;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 4;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 5;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg1000 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 4:
                    // Segment 4
                    if ((segment1 && segment20) ||
                        (segment4) ||
                        (segment2 && segment10) ||
                        (segment1 && segment10) ||
                        (segment2 && segment20) ||
                        (segment2 && segment8) ||
                        (segment8 && segment10) ||
                        (segment8 && segment20) ||
                        (segment1 && segment8) ||
                        (segment80 && segment200 && segment800 && segment4000) ||
                        (segment80 && segment200 && segment1000) ||
                        (segment8 && segment40 && segment80) ||
                        (segment8 && segment100 && segment200) ||
                        (segment8 && segment80 && segment1000 && segment2000) ||
                        (segment8 && segment200 && segment1000 && segment2000) ||
                        (segment8 && segment80 && segment800 && segment2000) ||
                        (segment8 && segment200 && segment2000 && segment4000) ||
                        (segment20 && segment80 && segment800) ||
                        (segment10 && segment80 && segment800) ||
                        (segment8 && segment80 && segment800) ||
                        (segment20 && segment80 && segment400) ||
                        (segment10 && segment80 && segment400) ||
                        (segment8 && segment80 && segment400) ||
                        (segment20 && segment40 && segment80) ||
                        (segment10 && segment40 && segment80) ||
                        (segment1 && segment200 && segment4000) ||
                        (segment2 && segment200 && segment4000) ||
                        (segment8 && segment200 && segment4000) ||
                        (segment1 && segment200 && segment8000) ||
                        (segment2 && segment200 && segment8000) ||
                        (segment8 && segment200 && segment8000) ||
                        (segment1 && segment100 && segment200) ||
                        (segment1 && segment200 && segment400 && segment1000) ||
                        (segment20 && segment80 && segment1000 && segment8000) ||
                        (segment800 && segment80 && segment200 && segment8000) ||
                        (segment800 && segment80 && segment200 && segment100) ||
                        (segment400 && segment80 && segment200 && segment4000) ||
                        (segment400 && segment80 && segment200 && segment8000) ||
                        (segment400 && segment80 && segment200 && segment100) ||
                        (segment40 && segment80 && segment200 && segment4000) ||
                        (segment40 && segment80 && segment200 && segment8000) ||
                        (segment40 && segment80 && segment200 && segment100) ||
                        (segment400 && segment1000 && segment200 && segment2) ||
                        (segment400 && segment1000 && segment200 && segment8) ||
                        (segment800 && segment1000 && segment200 && segment1) ||
                        (segment800 && segment1000 && segment200 && segment2) ||
                        (segment800 && segment1000 && segment200 && segment8) ||
                        (segment2000 && segment1000 && segment200 && segment1) ||
                        (segment2000 && segment1000 && segment200 && segment2) ||
                        (segment2000 && segment1000 && segment200 && segment8) ||
                        (segment8000 && segment1000 && segment80 && segment10) ||
                        (segment8000 && segment1000 && segment80 && segment8) ||
                        (segment4000 && segment1000 && segment80 && segment20) ||
                        (segment4000 && segment1000 && segment80 && segment10) ||
                        (segment4000 && segment1000 && segment80 && segment8) ||
                        (segment2000 && segment1000 && segment80 && segment20) ||
                        (segment2000 && segment1000 && segment80 && segment10) ||
                        (segment2000 && segment1000 && segment80 && segment8) ||
                        (segment2 && segment100 && segment200)) {
                        break;
                    } else {
                        segment4 = true;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 20;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 21;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg4 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 5:
                    // Segment 100
                    if ((segment200 && segment4000) ||
                        (segment100) ||
                        (segment10 && segment200) ||
                        (segment100 && segment200) ||
                        (segment20 && segment200) ||
                        (segment200 && segment8000) ||
                        (segment2000) ||
                        (segment8) ||
                        (segment40) ||
                        (segment1 && segment4 && segment200) ||
                        (segment2 && segment4 && segment200) ||
                        (segment200 && segment400 && segment1000) ||
                        (segment2 && segment80 && segment1000 && segment200) ||
                        (segment1 && segment80 && segment1000 && segment200) ||
                        (segment40 && segment80 && segment1000 && segment200) ||
                        (segment800 && segment80 && segment4 && segment200) ||
                        (segment400 && segment80 && segment4 && segment200) ||
                        (segment40 && segment80 && segment4 && segment200) ||
                        (segment200 && segment800 && segment1000)) {
                        break;
                    } else {
                        segment100 = true;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 14;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 15;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg100 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 6:
                    // Segment 8000
                    if ((segment4000) ||
                        (segment8000) ||
                        (segment1000 && segment2000) ||
                        (segment1000 && segment800) ||
                        (segment1000 && segment400) ||
                        (segment2 && segment80 && segment1000) ||
                        (segment10 && segment200) ||
                        (segment4 && segment20 && segment80 && segment1000) ||
                        (segment4 && segment8 && segment20 && segment2000) ||
                        (segment20 && segment200) ||
                        (segment100 && segment200) ||
                        (segment4 && segment8 && segment200) ||
                        (segment2 && segment4 && segment200) ||
                        (segment1 && segment4 && segment200) ||
                        (segment2 && segment80 && segment1000) ||
                        (segment1 && segment80 && segment1000) ||
                        (segment800 && segment80 && segment4 && segment200) ||
                        (segment400 && segment80 && segment4 && segment200) ||
                        (segment40 && segment80 && segment4 && segment200) ||
                        (segment1000 && segment80 && segment4 && segment10) ||
                        (segment1000 && segment80 && segment4 && segment8) ||
                        (segment40 && segment80 && segment1000)) {
                        break;
                    } else {
                        segment8000 = true;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 8;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg8000 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 7:
                    // Segment 20
                    if ((segment10) ||
                        (segment20) ||
                        (segment4 && segment8) ||
                        (segment2 && segment4) ||
                        (segment1 && segment4) ||
                        (segment4 && segment80 && segment800) ||
                        (segment200 && segment4000) ||
                        (segment4 && segment80 && segment1000 && segment8000) ||
                        (segment8 && segment1000 && segment2000 && segment8000) ||
                        (segment200 && segment8000) ||
                        (segment100 && segment200) ||
                        (segment200 && segment1000 && segment2000) ||
                        (segment200 && segment800 && segment1000) ||
                        (segment200 && segment400 && segment1000) ||
                        (segment4 && segment80 && segment800) ||
                        (segment4 && segment80 && segment400) ||
                        (segment2 && segment80 && segment1000 && segment200) ||
                        (segment1 && segment80 && segment1000 && segment200) ||
                        (segment40 && segment80 && segment1000 && segment200) ||
                        (segment4000 && segment1000 && segment80 && segment4) ||
                        (segment2000 && segment1000 && segment80 && segment4) ||
                        (segment4 && segment40 && segment80)) {
                        break;
                    } else {
                        segment20 = true;
                        activeDoorCount = activeDoorCount + 1;
                        // doorOn[activeDoorCount] = 24;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg20 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 8:
                    // Segment 800
                    if ((segment400) ||
                        (segment800) ||
                        (segment80 && segment2) ||
                        (segment80 && segment40) ||
                        (segment80 && segment1) ||
                        (segment1000 && segment2000) ||
                        (segment1000 && segment4000) ||
                        (segment1000 && segment8000) ||
                        (segment4 && segment8 && segment80) ||
                        (segment4 && segment10 && segment80) ||
                        (segment4 && segment20 && segment80) ||
                        (segment4 && segment80 && segment200 && segment4000) ||
                        (segment10 && segment200 && segment1000) ||
                        (segment20 && segment200 && segment1000) ||
                        (segment80 && segment4 && segment200 && segment8000) ||
                        (segment80 && segment4 && segment200 && segment100) ||
                        (segment1000 && segment200 && segment4 && segment1) ||
                        (segment1000 && segment200 && segment4 && segment2) ||
                        (segment1000 && segment200 && segment4 && segment8) ||
                        (segment100 && segment200 && segment1000)) {
                        break;
                    } else {
                        segment800 = true;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg800 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 9:
                    // Segment 80
                    if ((segment2 && segment800) ||
                        (segment80) ||
                        (segment1 && segment40) ||
                        (segment40 && segment400) ||
                        (segment2 && segment400) ||
                        (segment1 && segment800) ||
                        (segment1 && segment400) ||
                        (segment40 && segment1000 && segment2000) ||
                        (segment40 && segment800) ||
                        (segment4 && segment8 && segment40) ||
                        (segment2 && segment40) ||
                        (segment4 && segment200 && segment1000) ||
                        (segment4 && segment20 && segment1000 && segment8000) ||
                        (segment2 && segment1000 && segment8000) ||
                        (segment4 && segment200 && segment800 && segment4000) ||
                        (segment2 && segment10 && segment200 && segment1000) ||
                        (segment2 && segment1000 && segment8000) ||
                        (segment2 && segment1000 && segment4000) ||
                        (segment2 && segment1000 && segment2000) ||
                        (segment1 && segment1000 && segment8000) ||
                        (segment1 && segment1000 && segment4000) ||
                        (segment1 && segment1000 && segment2000) ||
                        (segment40 && segment1000 && segment8000) ||
                        (segment40 && segment1000 && segment4000) ||
                        (segment4 && segment20 && segment800) ||
                        (segment4 && segment20 && segment400) ||
                        (segment4 && segment20 && segment40) ||
                        (segment4 && segment10 && segment800) ||
                        (segment4 && segment10 && segment400) ||
                        (segment4 && segment10 && segment40) ||
                        (segment4 && segment8 && segment800) ||
                        (segment2 && segment1000 && segment200 && segment20) ||
                        (segment2 && segment1000 && segment200 && segment100) ||
                        (segment1 && segment1000 && segment200 && segment10) ||
                        (segment1 && segment1000 && segment200 && segment20) ||
                        (segment1 && segment1000 && segment200 && segment100) ||
                        (segment40 && segment1000 && segment200 && segment10) ||
                        (segment40 && segment1000 && segment200 && segment20) ||
                        (segment40 && segment1000 && segment200 && segment100) ||
                        (segment800 && segment4 && segment200 && segment8000) ||
                        (segment800 && segment4 && segment200 && segment100) ||
                        (segment400 && segment4 && segment200 && segment4000) ||
                        (segment400 && segment4 && segment200 && segment8000) ||
                        (segment400 && segment4 && segment200 && segment100) ||
                        (segment40 && segment4 && segment200 && segment4000) ||
                        (segment40 && segment4 && segment200 && segment8000) ||
                        (segment40 && segment4 && segment200 && segment100) ||
                        (segment8000 && segment1000 && segment4 && segment10) ||
                        (segment8000 && segment1000 && segment4 && segment8) ||
                        (segment4000 && segment1000 && segment4 && segment20) ||
                        (segment4000 && segment1000 && segment4 && segment10) ||
                        (segment4000 && segment1000 && segment4 && segment8) ||
                        (segment2000 && segment1000 && segment4 && segment20) ||
                        (segment2000 && segment1000 && segment4 && segment10) ||
                        (segment2000 && segment1000 && segment4 && segment8) ||
                        (segment4 && segment8 && segment400)) {
                        break;
                    } else {
                        segment80 = true;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg80 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 10:
                    // Segment 2
                    if ((segment1) ||
                        (segment2) ||
                        (segment80 && segment800) ||
                        (segment40 && segment80) ||
                        (segment80 && segment400) ||
                        (segment4 && segment8) ||
                        (segment4 && segment10) ||
                        (segment4 && segment20) ||
                        (segment80 && segment1000 && segment2000) ||
                        (segment80 && segment1000 && segment4000) ||
                        (segment80 && segment1000 && segment8000) ||
                        (segment4 && segment200 && segment4000) ||
                        (segment10 && segment80 && segment200 && segment1000) ||
                        (segment4 && segment200 && segment8000) ||
                        (segment80 && segment1000 && segment200 && segment20) ||
                        (segment80 && segment1000 && segment200 && segment100) ||
                        (segment400 && segment1000 && segment200 && segment4) ||
                        (segment800 && segment1000 && segment200 && segment4) ||
                        (segment2000 && segment1000 && segment200 && segment4) ||
                        (segment4 && segment100 && segment200)) {
                        break;
                    } else {
                        segment2 = true;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg2 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 11:
                    // Segment 2000
                    if ((segment8) ||
                        (segment2000) ||
                        (segment800 && segment1000) ||
                        (segment1000 && segment4000) ||
                        (segment400 && segment1000) ||
                        (segment1000 && segment8000) ||
                        (segment40) ||
                        (segment100) ||
                        (segment2 && segment80 && segment1000) ||
                        (segment1 && segment80 && segment1000) ||
                        (segment10 && segment200 && segment1000) ||
                        (segment1000 && segment200 && segment4 && segment1) ||
                        (segment1000 && segment200 && segment4 && segment2) ||
                        (segment1000 && segment200 && segment4 && segment8) ||
                        (segment1000 && segment80 && segment4 && segment20) ||
                        (segment1000 && segment80 && segment4 && segment10) ||
                        (segment1000 && segment80 && segment4 && segment8) ||
                        (segment20 && segment200 && segment1000)) {
                        break;
                    } else {
                        segment2000 = true;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg2000 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 12:
                    // Segment 8
                    if ((segment2000) ||
                        (segment8) ||
                        (segment2 && segment4) ||
                        (segment4 && segment10) ||
                        (segment1 && segment4) ||
                        (segment4 && segment20) ||
                        (segment40) ||
                        (segment100) ||
                        (segment4 && segment80 && segment800) ||
                        (segment4 && segment80 && segment400) ||
                        (segment4 && segment200 && segment4000) ||
                        (segment400 && segment1000 && segment200 && segment4) ||
                        (segment800 && segment1000 && segment200 && segment4) ||
                        (segment2000 && segment1000 && segment200 && segment4) ||
                        (segment8000 && segment1000 && segment80 && segment4) ||
                        (segment4000 && segment1000 && segment80 && segment4) ||
                        (segment2000 && segment1000 && segment80 && segment4) ||
                        (segment4 && segment200 && segment8000)) {
                        break;
                    } else {
                        segment8 = true;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg8 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 13:
                    // Segment 4000
                    if ((segment8000) ||
                        (segment4000) ||
                        (segment10 && segment200) ||
                        (segment100 && segment200) ||
                        (segment20 && segment200) ||
                        (segment1000 && segment2000) ||
                        (segment1000 && segment800) ||
                        (segment1000 && segment400) ||
                        (segment4 && segment8 && segment200) ||
                        (segment2 && segment4 && segment200) ||
                        (segment1 && segment4 && segment200) ||
                        (segment4 && segment80 && segment200 && segment800) ||
                        (segment2 && segment80 && segment1000) ||
                        (segment1 && segment80 && segment1000) ||
                        (segment400 && segment80 && segment4 && segment200) ||
                        (segment40 && segment80 && segment4 && segment200) ||
                        (segment1000 && segment80 && segment4 && segment20) ||
                        (segment1000 && segment80 && segment4 && segment10) ||
                        (segment1000 && segment80 && segment4 && segment8) ||
                        (segment40 && segment80 && segment1000)) {
                        break;
                    } else {
                        segment4000 = true;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg4000 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 14:
                    // Segment 200
                    if ((segment10 && segment4000) ||
                        (segment200) ||
                        (segment20 && segment100) ||
                        (segment100 && segment8000) ||
                        (segment10 && segment8000) ||
                        (segment20 && segment4000) ||
                        (segment20 && segment8000) ||
                        (segment100 && segment1000 && segment2000) ||
                        (segment100 && segment200 && segment2000) ||
                        (segment4 && segment8 && segment100) ||
                        (segment10 && segment100) ||
                        (segment100 && segment4000) ||
                        (segment4 && segment80 && segment1000) ||
                        (segment1 && segment4 && segment400 && segment1000) ||
                        (segment10 && segment400 && segment1000) ||
                        (segment4 && segment80 && segment800 && segment4000) ||
                        (segment2 && segment10 && segment80 && segment1000) ||
                        (segment10 && segment400 && segment1000) ||
                        (segment10 && segment800 && segment1000) ||
                        (segment10 && segment1000 && segment2000) ||
                        (segment20 && segment400 && segment1000) ||
                        (segment20 && segment800 && segment1000) ||
                        (segment20 && segment1000 && segment2000) ||
                        (segment100 && segment400 && segment1000) ||
                        (segment100 && segment800 && segment1000) ||
                        (segment1 && segment4 && segment4000) ||
                        (segment1 && segment4 && segment8000) ||
                        (segment1 && segment4 && segment100) ||
                        (segment2 && segment4 && segment4000) ||
                        (segment2 && segment4 && segment8000) ||
                        (segment2 && segment4 && segment100) ||
                        (segment4 && segment8 && segment4000) ||
                        (segment2 && segment80 && segment1000 && segment20) ||
                        (segment2 && segment80 && segment1000 && segment100) ||
                        (segment1 && segment80 && segment1000 && segment10) ||
                        (segment1 && segment80 && segment1000 && segment20) ||
                        (segment1 && segment80 && segment1000 && segment100) ||
                        (segment40 && segment80 && segment1000 && segment10) ||
                        (segment40 && segment80 && segment1000 && segment20) ||
                        (segment40 && segment80 && segment1000 && segment100) ||
                        (segment800 && segment80 && segment4 && segment8000) ||
                        (segment800 && segment80 && segment4 && segment100) ||
                        (segment400 && segment80 && segment4 && segment4000) ||
                        (segment400 && segment80 && segment4 && segment8000) ||
                        (segment400 && segment80 && segment4 && segment100) ||
                        (segment40 && segment80 && segment4 && segment4000) ||
                        (segment40 && segment80 && segment4 && segment8000) ||
                        (segment40 && segment80 && segment4 && segment100) ||
                        (segment400 && segment1000 && segment4 && segment2) ||
                        (segment400 && segment1000 && segment4 && segment8) ||
                        (segment800 && segment1000 && segment4 && segment1) ||
                        (segment800 && segment1000 && segment4 && segment2) ||
                        (segment800 && segment1000 && segment4 && segment8) ||
                        (segment2000 && segment1000 && segment4 && segment1) ||
                        (segment2000 && segment1000 && segment4 && segment2) ||
                        (segment2000 && segment1000 && segment4 && segment8) ||
                        (segment4 && segment8 && segment8000)) {
                        break;
                    } else {
                        segment200 = true;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg200 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;

                case 15:
                    // Segment 10
                    if ((segment20) ||
                        (segment10) ||
                        (segment200 && segment4000) ||
                        (segment100 && segment200) ||
                        (segment200 && segment8000) ||
                        (segment4 && segment8) ||
                        (segment2 && segment4) ||
                        (segment1 && segment4) ||
                        (segment200 && segment1000 && segment2000) ||
                        (segment200 && segment800 && segment1000) ||
                        (segment200 && segment400 && segment1000) ||
                        (segment2 && segment80 && segment200 && segment1000) ||
                        (segment4 && segment80 && segment800) ||
                        (segment4 && segment80 && segment400) ||
                        (segment1 && segment80 && segment1000 && segment200) ||
                        (segment40 && segment80 && segment1000 && segment200) ||
                        (segment8000 && segment1000 && segment80 && segment4) ||
                        (segment4000 && segment1000 && segment80 && segment4) ||
                        (segment2000 && segment1000 && segment80 && segment4) ||
                        (segment4 && segment40 && segment80)) {
                        break;
                    } else {
                        segment10 = true;
                        segmentCount = segmentCount - 1;
                        // OSREPORTF("#%d: Seg10 successful. %d remaining.\n", i, segmentCount);
                    }
                    break;
                }
                ++i;
            }

            /*
                FINISH ROOM LAYOUT
            */

            // Pipes
            if (!structureGenerated) {
                s32 pipeRNG = system::rand() % 133;
                if (pipeRNG >= 0 && pipeRNG < 39)
                    segment20000 = true;
                else if (pipeRNG >= 39 && pipeRNG < 76)
                    segment80000 = true;
                else if (pipeRNG >= 76 && pipeRNG < 80 && segment1 && segment20)
                    segment40000 = true;
                else if (pipeRNG >= 80 && pipeRNG < 82 && segment400 && segment8000)
                    segment10000 = true;
                else if (pipeRNG >= 82 && pipeRNG < 83) {
                    segment20000 = true;
                    segment80000 = true;
                } else if (pipeRNG >= 83 && pipeRNG < 86 && segment1 && segment20 && segment400 && segment8000) {
                    segment10000 = true;
                    segment20000 = true;
                    segment40000 = true;
                    segment80000 = true;
                }
            }

            // Add all segment values
            if (segment400)
                roomDecCode = roomDecCode + 0x400;
            if (segment800)
                roomDecCode = roomDecCode + 0x800;
            if (segment1000)
                roomDecCode = roomDecCode + 0x1000;
            if (segment2000)
                roomDecCode = roomDecCode + 0x2000;
            if (segment4000)
                roomDecCode = roomDecCode + 0x4000;
            if (segment8000)
                roomDecCode = roomDecCode + 0x8000;
            if (segment40)
                roomDecCode = roomDecCode + 0x40;
            if (segment80)
                roomDecCode = roomDecCode + 0x80;
            if (segment100)
                roomDecCode = roomDecCode + 0x100;
            if (segment200)
                roomDecCode = roomDecCode + 0x200;
            if (segment1)
                roomDecCode = roomDecCode + 0x1;
            if (segment2)
                roomDecCode = roomDecCode + 0x2;
            if (segment4)
                roomDecCode = roomDecCode + 0x4;
            if (segment8)
                roomDecCode = roomDecCode + 0x8;
            if (segment10)
                roomDecCode = roomDecCode + 0x10;
            if (segment20)
                roomDecCode = roomDecCode + 0x20;

            // OSREPORTF("Finished generating. Roomcode w/o pipes is %d.\n", roomDecCode);

            if (segment10000)
                roomDecCode = roomDecCode + 0x10000;
            if (segment20000)
                roomDecCode = roomDecCode + 0x20000;
            if (segment40000)
                roomDecCode = roomDecCode + 0x40000;
            if (segment80000)
                roomDecCode = roomDecCode + 0x80000;

            if (currentFloor == 198) {
                roomDecCode = 0;
                activeDoorCount = 19;
            }
        }

        /*
            DOORS
            (I would make this a separate function, but it relies on previously established vars a lot so it can stay here)
        */
        s32 successfulDoors = 0;
        s32 doorExit = 0;
        s32 doorEntrance = 0;
        s32 doorId = 0;
        bool currentDoorSuccess = false;
        bool doorOnToggleableSegment = false;
        while (successfulDoors != 2) {
            currentDoorSuccess = false;
            doorOnToggleableSegment = false;
            doorId = system::rand() % 32 + 1;
            while (doorEntrance == doorId) {
                doorId = system::rand() % 32 + 1;
            }
            // OSREPORTF("Generated doorId %d.\n", doorId);
            if (doorId == 1) {
                doorOnToggleableSegment = true;
                if (segment400) {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId %d passed segment check.\n", doorId);
                }
            }
            if (doorId == 4 || doorId == 5) {
                doorOnToggleableSegment = true;
                if (segment1000) {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId %d passed segment check.\n", doorId);
                }
            }
            if (doorId == 8) {
                doorOnToggleableSegment = true;
                if (segment8000) {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId %d passed segment check.\n", doorId);
                }
            }
            if (doorId == 10 || doorId == 11) {
                doorOnToggleableSegment = true;
                if (segment40) {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId %d passed segment check.\n", doorId);
                }
            }
            if (doorId == 14 || doorId == 15) {
                doorOnToggleableSegment = true;
                if (segment100) {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId %d passed segment check.\n", doorId);
                }
            }
            if (doorId == 17) {
                doorOnToggleableSegment = true;
                if (segment1) {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId %d passed segment check.\n", doorId);
                }
            }
            if (doorId == 20 || doorId == 21) {
                doorOnToggleableSegment = true;
                if (segment4) {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId %d passed segment check.\n", doorId);
                }
            }
            if (doorId == 24) {
                doorOnToggleableSegment = true;
                if (segment20) {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId %d passed segment check.\n", doorId);
                }
            }
            // OSREPORTF("All toggleable segment checks finished; doorOnToggleableSegment is %d, successfulDoors is %d, currentDoorSuccess is %d.\n", doorOnToggleableSegment, successfulDoors, currentDoorSuccess);
            if (!doorOnToggleableSegment) {
                if (doorId >= 25) {
                    if (boobies > 35) {
                        successfulDoors = successfulDoors + 1;
                        currentDoorSuccess = true;
                        // OSREPORTF("doorId was not on a toggleable segment; successfulDoors is %d.\n", successfulDoors);
                    }
                } else {
                    successfulDoors = successfulDoors + 1;
                    currentDoorSuccess = true;
                    // OSREPORTF("doorId was not on a toggleable segment; successfulDoors is %d.\n", successfulDoors);
                }
            }
            if (currentDoorSuccess) {
                if (successfulDoors == 1) {
                    doorEntrance = doorId;
                } else if (successfulDoors == 2) {
                    doorExit = doorId;
                }
            }
            // OSREPORTF("doorEntrance: %d. doorExit: %d.\n", doorEntrance, doorExit);
        }

        for (s32 i = 0; i < 200; ++i) {
            // Placeholder item, true items are determined by above protocol and chosen upon opening chest
            dan::dan_wp->dungeons[i].item = itemdrv::itemTypeNameToId("ITEM_ID_COOK_TRIAL_PAN");

            // Read map (bitflags for parts of the map to enable and disable in enemy rooms, 0 & unused elsewhere)
            dan::dan_wp->dungeons[i].map = roomDecCode;

            // Door substitution
            dan::dan_wp->dungeons[i].doors[0].enter = doorEntrance;
            dan::dan_wp->dungeons[i].doors[0].exit = doorExit;
            dan::dan_wp->dungeons[i].doorCount = 1;
        }
        return;
    }

    // Returns true if the eGenTR loop needs to reroll, returns false if it can proceed
    bool DanGen_Enemies_CatchExceptions(s32 name, s32 e1, s32 e2, s32 e3, s32 e4, s32 enemyTypes) {
        // Catch duplicate enemy entries
        if (name == e1 || name == e2 || name == e3 || name == e4)
            return true;
        // Never allow pigarithms and hogarithms to coincide
        if (name == DAN_ENEMY(NPC_PIGARITHM) || name == DAN_ENEMY(NPC_HOGARITHM)) {
            if (DAN_ENEMY(NPC_PIGARITHM) == e1 || DAN_ENEMY(NPC_PIGARITHM) == e2 || DAN_ENEMY(NPC_PIGARITHM) == e3 || DAN_ENEMY(NPC_PIGARITHM) == e4)
                return true;
            if (DAN_ENEMY(NPC_HOGARITHM) == e1 || DAN_ENEMY(NPC_HOGARITHM) == e2 || DAN_ENEMY(NPC_HOGARITHM) == e3 || DAN_ENEMY(NPC_HOGARITHM) == e4)
                return true;
        }
        // Only 2 Ninjoe variants at once with 3 enemy types, 1 ninjoe variant with 4 enemy types
        if (name >= DAN_ENEMY(NPC_NINJOE) && name <= DAN_ENEMY(NPC_DARK_NINJOE)) {
            s32 ninjoeTypeCnt = 0;
            if (e1 >= DAN_ENEMY(NPC_NINJOE) && e1 <= DAN_ENEMY(NPC_DARK_NINJOE))
                ninjoeTypeCnt += 1;
            if (e2 >= DAN_ENEMY(NPC_NINJOE) && e2 <= DAN_ENEMY(NPC_DARK_NINJOE))
                ninjoeTypeCnt += 1;
            if (e3 >= DAN_ENEMY(NPC_NINJOE) && e3 <= DAN_ENEMY(NPC_DARK_NINJOE))
                ninjoeTypeCnt += 1;
            if (e4 >= DAN_ENEMY(NPC_NINJOE) && e4 <= DAN_ENEMY(NPC_DARK_NINJOE))
                ninjoeTypeCnt += 1;
            if (enemyTypes == 4 && ninjoeTypeCnt >= 1)
                return true;
            if (enemyTypes != 4 && ninjoeTypeCnt >= 2)
                return true;
        }
        return false;
    }

    // These filters are for simultaneous lag reduction & crash prevention
    s32 DanGen_Enemies_MiscFilters(s32 name, s32 amt) {
        // Pigarithm family is extreme animpose-heavy; 10 in one room crashes the game to a subheap overflow
        if ((name == DAN_ENEMY(NPC_PIGARITHM) || name == DAN_ENEMY(NPC_HOGARITHM)) && amt >= 4)
            return 4;
        // Ninjoes are extremely script-heavy and will lag the game with a lot of other enemies in a room
        if ((name >= DAN_ENEMY(NPC_NINJOE) && name <= DAN_ENEMY(NPC_DARK_NINJOE)) && amt >= 5)
            return 5;
        return 0;
    }

    void DanGen_Enemies(s32 roomGens, bool depravity) {
        // Enemy generation setup
        s32 enemyTypes = 0;
        s32 arrayRNG = 0;
        s32 enemyAmt = 0;
        s32 enemyDifference = 0;
        s32 enemyMin = 0;
        s32 enemyMax = 0;
        //  s32 spDoorRNG = 0;
        s32 tribeArray = 0;
        s32 vsOdds = 0;
        s32 eGenTR[4] = {-1, -1, -1, -1}; // "Enemies Generated This Room"
        s32 enemyTribe = 0;
        s32 enemyBonus = 0;
        s32 limiterMod = 0;
        s32 enemyGenLim1 = 0;
        s32 enemyGenLim2 = 0;
        s32 enemyGenRNG = system::rand() % 100;
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            enemyGenLim1 = 25;
            enemyGenLim2 = 75;
            break;
        case 1:
            enemyGenLim1 = 20;
            enemyGenLim2 = 65;
            break;
        case 2:
            enemyGenLim1 = 15;
            enemyGenLim2 = 45;
            break;
        }
        if (enemyGenRNG < enemyGenLim1) {
            enemyTypes = 1;
        } else if (enemyGenRNG < enemyGenLim2) {
            enemyTypes = 2;
        } else {
            enemyTypes = 3;
            // Flat 25% chance to become 4 enemies instead
            {
                s32 raiseTheStakesBabygirlYeahhhhh = system::rand() % 100;
                if (raiseTheStakesBabygirlYeahhhhh < 25)
                    enemyTypes = 4;
            }
        }
        if (enemyTypes < 3 && roomGens >= 174)
            enemyTypes += 1;
        if (depravity)
            enemyTypes = 4;
        Lunatic->Floor[roomGens].enemyTypes = enemyTypes;
        DanLevelData * danLevelData = nullptr;
        // Enemy Substition Protocol v4.0: Generate enemy data for each floor on Pit entry and store that data competently.
        // v4 makes use of the new Lunatic session pointer & new internal file structure, and maintains the rest of the v3 process as legacy code.
        for (s32 e = 0; e < 4; e += 1) {
            // Get "level data," referencing the scaling nature of the Pit. These provide thresholds for enemy Lv & bonus enemies to spawn, independent of Difficulty Options.
            if (roomGens < 25) {
                danLevelData = danLevelDataPtr(1);
            } else if (roomGens < 150) {
                danLevelData = danLevelDataPtr(2);
            } else if (roomGens < 175) {
                danLevelData = danLevelDataPtr(3);
            } else
                danLevelData = danLevelDataPtr(4);
            // Set enemy Lv threshold modifiers based on Difficulty.
            difficulty = lpGetDifficulty();
            limiterMod = system::rand() % 11;
            if (difficulty == 0) {
                limiterMod -= 5; // Ranges from -5 to 5
            } else if (difficulty == 1) {
                limiterMod -= 10; // Ranges from -10 to +0
            } else
                limiterMod -= 20; // Ranges from -10 to -20
        // Roll through each limiter. Lower enemyGenRNG values tend toward lower Lv enemies.
        nopeBuffThatShit:
            enemyGenRNG = system::rand() % 100;
            if (enemyGenRNG < ((danLevelData->lv1Limiter) + limiterMod)) {
                arrayRNG = system::rand() % ARRAY32_COUNT(lv1Tribes);
                tribeArray = 1;
                enemyTribe = lv1Tribes[arrayRNG];
                // OSREPORTF("Room #%d: tribeArray %d selected; (enemyGenRNG = %d) < (lv1Lim + limMod = %d). limMod = %d, lim = %d.\n", roomGens, tribeArray, enemyGenRNG, ((danLevelData->lv1Limiter) + limiterMod), limiterMod, danLevelData->lv1Limiter);
            } else if (enemyGenRNG < ((danLevelData->lv2Limiter) + limiterMod)) {
                arrayRNG = system::rand() % ARRAY32_COUNT(lv2Tribes);
                tribeArray = 2;
                enemyTribe = lv2Tribes[arrayRNG];
                // OSREPORTF("Room #%d: tribeArray %d selected; (enemyGenRNG = %d) < (lv2Lim + limMod = %d). limMod = %d, lim = %d.\n", roomGens, tribeArray, enemyGenRNG, ((danLevelData->lv2Limiter) + limiterMod), limiterMod, danLevelData->lv2Limiter);
            } else if (enemyGenRNG < ((danLevelData->lv3Limiter) + limiterMod)) {
                arrayRNG = system::rand() % ARRAY32_COUNT(lv3Tribes);
                tribeArray = 3;
                enemyTribe = lv3Tribes[arrayRNG];
                // OSREPORTF("Room #%d: tribeArray %d selected; (enemyGenRNG = %d) < (lv3Lim + limMod = %d). limMod = %d, lim = %d.\n", roomGens, tribeArray, enemyGenRNG, ((danLevelData->lv3Limiter) + limiterMod), limiterMod, danLevelData->lv3Limiter);
            } else {
                arrayRNG = system::rand() % ARRAY32_COUNT(lv4Tribes);
                tribeArray = 4;
                enemyTribe = lv4Tribes[arrayRNG];
                // OSREPORTF("Room #%d: tribeArray %d selected; (enemyGenRNG = %d) > (lv3Lim + limMod = %d). limMod = %d, lim = %d.\n", roomGens, tribeArray, enemyGenRNG, ((danLevelData->lv3Limiter) + limiterMod), limiterMod, danLevelData->lv3Limiter);
            }
            if (depravity && (e == 0 || roomGens >= DepravityGetAllLv4Threshold(difficulty)))
                tribeArray = 4;
            // Limit piss-easy rooms on Hard and above
            if (difficulty >= 2) {
                if (enemyTypes == 1 && tribeArray == 1) // 1 enemy type, easy enemy - buff it 100% of the time
                    goto nopeBuffThatShit;
                enemyGenRNG = system::rand() % 100;
                if (enemyGenRNG > 50 && enemyTypes < 3 && tribeArray == 1) // 1-2 enemy types, rolls an easy enemy - reroll 50% of the time
                    goto nopeBuffThatShit;
            }
            // Pulls danEnemy from the static array of DanNPCData structs.
            DanNPCData ** danEnemies = danNpcGetPtr();
            DanNPCData * danEnemy = danEnemies[enemyTribe];
            vsOdds = system::rand() % 100;
            // This loop runs through enemies of the same tribe array until it finds one that passes an odds check.
            // It MUST NOT be an enemy that has already generated in this room.
            bool goAgain = false;
            do {
                if (tribeArray == 1) {
                    arrayRNG = system::rand() % (sizeof(lv1Tribes) / 4);
                    enemyTribe = lv1Tribes[arrayRNG];
                } else if (tribeArray == 2) {
                    arrayRNG = system::rand() % (sizeof(lv2Tribes) / 4);
                    enemyTribe = lv2Tribes[arrayRNG];
                } else if (tribeArray == 3) {
                    arrayRNG = system::rand() % (sizeof(lv3Tribes) / 4);
                    enemyTribe = lv3Tribes[arrayRNG];
                } else {
                    arrayRNG = system::rand() % (sizeof(lv4Tribes) / 4);
                    enemyTribe = lv4Tribes[arrayRNG];
                }
                vsOdds = system::rand() % 100;
                danEnemy = danEnemies[enemyTribe];
                goAgain = DanGen_Enemies_CatchExceptions(danEnemy->name, eGenTR[0], eGenTR[1], eGenTR[2], eGenTR[3], enemyTypes);
            } while ((danEnemy->odds < vsOdds || goAgain));
            // Once an enemy is determined, the number of times it will spawn in the Pit are determined by its min and max fields, as well as some other factors.
            // This data is all thrown into enemyConfigArray, which is a master array of all NPC data read during the final generation step.
            // The enemy's name is also thrown into eGenTR so that it doesn't generate in this room again.
            eGenTR[e] = danEnemy->name;
            Lunatic->Floor[roomGens].Enemies[e].name = danEnemy->name;
            // First enemy type gets a relative enemy boost.
            if (e == 0 || depravity) {
                enemyDifference = (danEnemy->max - danEnemy->min) + danLevelData->bonusMax;
                if (enemyDifference == 0)
                    enemyDifference = 1;
                enemyAmt = (system::rand() % enemyDifference) + 1;
                enemyAmt = enemyAmt + danEnemy->min;
            }
            // All other enemies get a relative enemy nerf, followed by a potential bonus.
            else {
                if (danEnemy->min <= 3) {
                    enemyMin = 1;
                } else if (danEnemy->min <= 5)
                    enemyMin = 2;
                if (danEnemy->max <= 3) {
                    enemyMax = 1;
                } else if (danEnemy->max <= 5) {
                    enemyMax = 2;
                } else
                    enemyMax = 3;
                enemyDifference = enemyMax - enemyMin + 1;
                enemyBonus = system::rand() % (danLevelData->bonusMax + 1);
                enemyBonus += danLevelData->bonusMin;
                enemyAmt = system::rand() % enemyDifference;
                enemyAmt += danEnemy->min + enemyBonus;
            }
            // If there's only one enemy type, it gets an extra bonus.
            if (enemyTypes == 1) {
                if (boobies <= 25) {
                    enemyAmt += 2;
                } else if (boobies <= 75) {
                    enemyAmt += 3;
                } else
                    enemyAmt += 4;
                if (difficulty == 2) {
                    enemyAmt += (system::rand() % 3) + 2;
                }
            }
            // One final boost to the enemy amount on Hard Difficulty!
            if (difficulty == 2) {
                f32 vsOdds2 = system::rand() % 100;
                if (vsOdds2 > 50) {
                    enemyBonus = system::rand() % 3;
                    enemyAmt += enemyBonus;
                }
            }
            s32 adjEnemyAmt = DanGen_Enemies_MiscFilters(danEnemy->name, enemyAmt);
            if (adjEnemyAmt != 0) {
                enemyAmt = adjEnemyAmt;
                if (enemyTypes == 1)
                    enemyTypes += 1;
            }
            Lunatic->Floor[roomGens].Enemies[e].num = enemyAmt;
            OSREPORTF("Room #%d: %s generated %d times. lv1Limiter for this floor: %d. e = %d, enemyTypes = %d. %d (vsOdds) < %d (danEnemy odds).\n", roomGens, msgdrv::msgSearch(item_data::itemDataTable[npcdrv::npcGetTribe((danEnemy->name) - 1)->catchCardItemId].nameMsg), enemyAmt, danLevelData->lv1Limiter, e, enemyTypes, vsOdds, danEnemy->odds);
        }
    }

    void DanGen_Enemies_Apply() {
        for (s32 floor = 0; floor < 200; floor += 1) {
            dan::dan_wp->dungeons[floor].enemyCount = Lunatic->Floor[floor].enemyTypes;
            dan::dan_wp->dungeons[floor].enemies[1].name = Lunatic->Floor[floor].Enemies[0].name;
            dan::dan_wp->dungeons[floor].enemies[1].num = Lunatic->Floor[floor].Enemies[0].num;
            dan::dan_wp->dungeons[floor].enemies[1].pos = Lunatic->Floor[floor].Enemies[0].pos;
            if (Lunatic->Floor[floor].enemyTypes >= 2) {
                dan::dan_wp->dungeons[floor].enemies[2].name = Lunatic->Floor[floor].Enemies[1].name;
                dan::dan_wp->dungeons[floor].enemies[2].num = Lunatic->Floor[floor].Enemies[1].num;
                dan::dan_wp->dungeons[floor].enemies[2].pos = Lunatic->Floor[floor].Enemies[1].pos;
                if (Lunatic->Floor[floor].enemyTypes >= 3) {
                    dan::dan_wp->dungeons[floor].enemies[3].name = Lunatic->Floor[floor].Enemies[2].name;
                    dan::dan_wp->dungeons[floor].enemies[3].num = Lunatic->Floor[floor].Enemies[2].num;
                    dan::dan_wp->dungeons[floor].enemies[3].pos = Lunatic->Floor[floor].Enemies[2].pos;
                    if (Lunatic->Floor[floor].enemyTypes == 4) {
                        dan::dan_wp->dungeons[floor].enemies[4].name = Lunatic->Floor[floor].Enemies[3].name;
                        dan::dan_wp->dungeons[floor].enemies[4].num = Lunatic->Floor[floor].Enemies[3].num;
                        dan::dan_wp->dungeons[floor].enemies[4].pos = Lunatic->Floor[floor].Enemies[3].pos;
                    }
                }
            }
        }
    }

    s32 evt_dan_read_data_new(evtmgr::EvtEntry * entry, bool isFirstCall) {
        (void)entry;
        if (isFirstCall) {
            dan::dan_wp = (dan::DanWork *)memory::__memAlloc(memory::Heap::HEAP_MAP, sizeof(dan::DanWork));
            msl::string::memset(dan::dan_wp, 0, sizeof(*dan::dan_wp));
            dan::dan_wp->dungeons = (dan::DanDungeon *)memory::__memAlloc(memory::Heap::HEAP_MAP, sizeof(dan::DanDungeon[DUNGEON_MAX]));
            msl::string::memset(dan::dan_wp->dungeons, 0, sizeof(dan::DanDungeon[DUNGEON_MAX]));
        }

        /*
            SET UP VARIABLES AND DO RANDOM PIT/ROOM ENTRY THINGS
            (Some of this CAN be thrown into a new entry-only function later!)
        */
        // Reset Pit chests
        for (u16 ThakoGswf = 433; ThakoGswf <= 450; ++ThakoGswf) {
            swdrv::swClear(ThakoGswf);
        }

        // Clear Merluna judgement flag
        swdrv::swClear(1669);

        s32 i = 0;
        s32 currentFloor = swdrv::swByteGet(1);
        s32 shadooEntries = swdrv::swByteGet(24);

        // Global room generation RNG value (1-100, 5 decimal places)
        // Hi, it's Yme from November 2025. I should hang myself for doing this. I'm not changing it though
        double thighs = system::rand() % 99 + 1;
        double divisor = 10;
        double ass = (system::rand() % 10) / divisor;
        divisor = 100;
        double booty = (system::rand() % 10) / divisor;
        divisor = 1000;
        double butt = (system::rand() % 10) / divisor;
        divisor = 10000;
        double bottom = (system::rand() % 10) / divisor;
        divisor = 100000;
        double buttocks = (system::rand() % 10) / divisor;
        boobies = (thighs + ass + booty + butt + bottom + buttocks);

        // Check if room 49, then shove you into Flopside Pit (Doesn't really update until Floor 50)
        if (currentFloor == 48) {
            swdrv::swByteSet(1, 148);
        }

        // Ensure Shadoo is fought on first room entry
        if (shadooEntries == 0) {
            swdrv::swByteSet(24, 1);
        }

        /*
            INIT PIT ROOM RANDOMIZATION PROCESS
        */

        DanGen_SegmentsAndDoors(currentFloor);

        // DanGen_Items(true);

        if (currentFloor == 0) {
            /*// Customwin msg gx test
            customwin::CWMsgGX * Entry = customwin::CWMsgEntry("Test", 0, tplpatch::TPLPatchIconTPLHeader, true, false);
            customwin::CWMsgGX_Tile_SetTplIndices(Entry, 1, 2, 3, 4, 5, 6, 7, 8, 9);*/
            for (s32 roomGens = 0; roomGens < 200; roomGens += 1)
                DanGen_Enemies(roomGens, false);
        }

        // Determine enemy data for current room
        if (Lunatic->Mover.moverRNG > 14)
            DanGen_Enemies_Apply();
        //  Uncomment this and replace with any enemy name to add enemy to first 3 Floors. May break stuff sometimes
        /*dan::dan_wp->dungeons[0].enemies[0].name = DAN_ENEMY(NPC_BOMBSHELL_BILL_BLASTER);
        dan::dan_wp->dungeons[0].enemies[0].num = 1;
        dan::dan_wp->dungeons[0].enemies[1].name = DAN_ENEMY(NPC_GOOMBA);
        dan::dan_wp->dungeons[0].enemies[1].num = 1;
        dan::dan_wp->dungeons[0].enemies[2].name = DAN_ENEMY(NPC_SHLORP);
        dan::dan_wp->dungeons[0].enemies[2].num = 1;
        dan::dan_wp->dungeons[0].enemies[3].name = DAN_ENEMY(NPC_FLORO_SAPIEN_PURPLE);
        dan::dan_wp->dungeons[0].enemies[3].num = 1;
        dan::dan_wp->dungeons[0].enemyCount = 4;*/

        // Replace Flimm inventory every floor; this sets a number of random items from the custom rotenShopItemPools.
        f32 flimmMult = 0;
        s32 difficulty = lpGetDifficulty();
        switch (difficulty) {
        case 0:
            flimmMult = 1.2f;
            break;
        case 1:
            flimmMult = 1.5f;
            break;
        case 2:
            flimmMult = 1.8f;
            break;
        }
        if (VoucherChkTorn(VOUCHER_YELLOW) == true)
            flimmMult *= 0.7f;
        s32 poolItem;
        u16 cost;
        for (i = 0; i < 67; i = i + 3) {
            if (i <= 15) {
                poolItem = system::rand() % 86;
                newRotenShopItems[i] = rotenShopItemPool[poolItem];
                cost = (u16)msl::math::floor(((u16)item_data::itemDataTable[rotenShopItemPool[poolItem]].buyPrice) * flimmMult);
            } else {
                poolItem = system::rand() % 19;
                newRotenShopItems[i] = rotenShopLowerClassItemPool[poolItem];
                cost = (u16)msl::math::floor(((u16)item_data::itemDataTable[rotenShopLowerClassItemPool[poolItem]].buyPrice) * flimmMult);
            }
            newRotenShopItems[i + 1] = clamp(cost, 0, 999);
        }
        for (i = 0; i < 70; i++) {
            dan::dan_rotenShopItems[i] = newRotenShopItems[i];
        }

        // Mover logic
        if (Lunatic->Mover.moverRNG <= 14) {
            for (i = 0; i < 200; ++i) {
                dan::dan_wp->dungeons[i].enemies[1].name = 1;
                dan::dan_wp->dungeons[i].enemies[1].num = 1;
                dan::dan_wp->dungeons[i].enemies[1].pos = 26;
                dan::dan_wp->dungeons[i].enemyCount = 1;
                dan::dan_wp->dungeons[i].doors[0].enter = 28;
                dan::dan_wp->dungeons[i].doors[0].exit = 29;
                dan::dan_wp->dungeons[i].map = 0;
            }
            danClearMapParts();
        }

        s32 blessingRoomCounter = swdrv::swByteGet(1602);
        if (blessingRoomCounter > 0) {
            blessingRoomCounter = blessingRoomCounter - 1;
            swdrv::swByteSet(1602, blessingRoomCounter);
            if (blessingRoomCounter == 0) {
                swdrv::swByteSet(1600, 0);
            }
        }

        s32 paramitaTimer = swdrv::swByteGet(1610);
        if (paramitaTimer > 0) {
            paramitaTimer = paramitaTimer + 2;
            swdrv::swByteSet(1610, paramitaTimer);
        }
        return EVT_RET_CONTINUE;
    }

    // Force unused enemies to not crash the fucking game when spawned in the Pit
    s32 evt_dan_get_enemy_info_new(evtmgr::EvtEntry * entry, bool isFirstCall) {
        (void)isFirstCall;

        // Get dungeon and enemy index
        evtmgr::EvtScriptCode * args = entry->pCurData;
        s32 enemyTribex = evtmgr_cmd::evtGetValue(entry, args[1]);
        s32 currentFloor = swdrv::swByteGet(1);
        dan::DanDungeon * dungeon = dan::dan_wp->dungeons + currentFloor;

        if ((enemyTribex < 0) || (enemyTribex >= 16)) {
            // Return 0 for invalid enemies
            evtmgr_cmd::evtSetValue(entry, args[2], 0);
            evtmgr_cmd::evtSetValue(entry, args[3], 0);

            return EVT_RET_CONTINUE;
        } else {
            // Get enemy
            dan::DanEnemy * enemy = dungeon->enemies + enemyTribex;

            if (enemy->num > 0) {
                // Find template with correct tribe id
                s32 tribeId = enemy->name - 1;
                s32 i;
                npcdrv::NPCEnemyTemplate * curTemplate = npcdrv::npcEnemyTemplates;
                for (i = 0; i < NPCTEMPLATE_MAX; i++, curTemplate++) {
                    npcdrv::npcEnemyTemplates[10].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[13].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[74].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[76].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[55].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[101].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[321].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[322].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[323].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[324].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[82].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[316].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[317].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[318].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[319].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[320].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[276].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[277].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[279].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[23].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[125].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[30].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[113].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[200].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[332].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[108].unknown_0x8 = 0;
                    npcdrv::npcEnemyTemplates[109].unknown_0x8 = 0;

                    if (((curTemplate->unknown_0x8 & 1) == 0) && (curTemplate->tribeId == tribeId))
                        break;
                }
                assertf(i < NPCTEMPLATE_MAX, "みつかりませんでした[%d]", tribeId);

                // Return template id and num
                evtmgr_cmd::evtSetValue(entry, args[2], i);
                evtmgr_cmd::evtSetValue(entry, args[3], enemy->num);
            } else {
                // Return 0 for empty enemy slots
                evtmgr_cmd::evtSetValue(entry, args[2], 0);
                evtmgr_cmd::evtSetValue(entry, args[3], 0);
            }

            return EVT_RET_CONTINUE;
        }
    }
} // turtle
