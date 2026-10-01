#include <common.h>
#include <lp_common.h>
#include <mod.h>
#include <msgpatch.h>
#include <msl/math.h>
#include <spm/animdrv.h>
#include <spm/effdrv.h>
#include <spm/filemgr.h>
#include <spm/icondrv.h>
#include <spm/item_data.h>
#include <spm/mario.h>
#include <spm/mario_pouch.h>
#include <spm/memory.h>
#include <spm/msgdrv.h>
#include <spm/npcdrv.h>
#include <spm/rel/mi4.h>
#include <tplpatch.h>
#include <wii/os.h>
#include <wii/tpl.h>
#include <ymetools.h>

namespace mod {
    /*
        Library for basic utility functions used all across proprietary LP files
    */

    using namespace spm;

    wii::tpl::TPLHeader * LPTitleTPLHeader = nullptr;
    s32 LPTitleTPLCurIndex = 0;
    s32 LPTitleTPLCurN = 0;

    s32 round(f32 in) {
        bool neg = false;
        if (in < 0.0f) {
            neg = true;
            in *= -1.0f;
        }
        s32 left = (s32)in;
        in -= left;
        if (in >= 0.5f)
            in = 1.0f;
        else
            in = 0.0f;
        s32 out = left + in;
        if (neg)
            out *= -1;
        return out;
    }

    s32 round_to(s32 in, s32 to) {
        f32 fIn = (f32)in;
        f32 fTo = (f32)to;
        return (round(fIn / fTo) * fTo);
    }

    s32 clamp(s32 input, s32 min, s32 max) {
        if (input < min)
            return min;
        else if (input > max)
            return max;
        else
            return input;
    }

    // Splits a positive number into separate digits in order & returns number of digits
    s32 split(s32 in, s32 * out) {
        if (in == 0)
            return 1;
        s32 i = -1;
        for (s32 n = in; n > 0; n /= 10) {
            i += 1;
        }
        s32 num = i + 1;
        for (s32 n = in; i >= 0; i -= 1) {
            out[i] = n % 10;
            n /= 10;
        }
        return num;
    }

    f32 abs_value(f32 in) {
        if (in < 0.0f)
            in *= -1.0f;
        return in;
    }

    wii::tpl::TPLHeader * allocTPL(const char * fileName, const char * folderName, memory::Heap heap, bool bind) {
        filemgr::FileEntry * file = nullptr;
        if (folderName != nullptr)
            file = filemgr::fileAllocf(0, "./%s/%s", folderName, fileName);
        else
            file = filemgr::fileAllocf(0, "./%s", fileName);
        u32 heapSize = ((u32)memory::memory_wp->heapEnd[heap] - (u32)memory::memory_wp->heapStart[heap]);
        if (heap == memory::HEAP_MEM1_UNUSED && heapSize < file->length) {
            heap = memory::HEAP_EFFECT;
            heapSize = ((u32)memory::memory_wp->heapEnd[heap] - (u32)memory::memory_wp->heapStart[heap]);
        }
        assertf(file->length < heapSize, "Heap %d Overflow!! [%s] (0x%x > 0x%x)", heap, fileName, file->length, heapSize);
        wii::tpl::TPLHeader * tpl = (wii::tpl::TPLHeader *)memory::__memAlloc(heap, file->length);
        msl::string::memcpy(tpl, file->sp->data, file->length);
        filemgr::fileFree(file);
        if (bind)
            wii::tpl::TPLBind(tpl);
        return tpl;
    }

    void lpEnableDebugMode() {
        if (!DebugMode) {
            DebugMode = true;
            yme::ymeMain();
        }
    }

    void lpAddAtk(s32 atk) {
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        s32 preAtk = pouch->attack;
        pouch->attack += atk;
        Lunatic->Stats.DemiseATK += (pouch->attack - preAtk);
        return;
    }

    void lpAddHp(s32 maxHp, s32 hp) {
        mario_pouch::MarioPouchWork * pouch = mario_pouch::pouchGetPtr();
        s32 preHp = pouch->maxHp;
        pouch->maxHp += maxHp;
        Lunatic->Stats.DelightHP += (pouch->maxHp - preHp);
        pouch->hp += hp;
        if (pouch->hp > pouch->maxHp)
            pouch->hp = pouch->maxHp;
        return;
    }

    void lpAddCrit(s32 rate, f32 mult) {
        Lunatic->Stats.CritRate += rate;
        Lunatic->Stats.CritMult += mult;
        Lunatic->Stats.CritRate = clamp(Lunatic->Stats.CritRate, -100, 100);
        return;
    }

    s32 evt_lp_add_crit(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 rate = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        f32 mult = evtmgr_cmd::evtGetFloat(evtEntry, args[1]);
        lpAddCrit(rate, mult);
        return 2;
    }

    void lpAddChestKeys(s32 num) {
        Lunatic->RFC.chestKeysOwned += num;
        return;
    }

    s32 evt_lp_get_chest_keys(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)Lunatic->RFC.chestKeysOwned);
        return 2;
    }

    s32 evt_lp_add_chest_keys(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 num = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        lpAddChestKeys(num);
        return 2;
    }

    void * lpMakeEffTarget(effdrv::EffTargetType type) {
        effdrv::EffTarget_Coord * target = (effdrv::EffTarget_Coord *)memory::__memAlloc(memory::HEAP_MAP, sizeof(effdrv::EffTarget_Coord));
        target->type = type;
        return target;
    }

    void lpBaldioOnOff(bool onOff) {
        if (onOff) {
            mario::marioCharPoseNames[mario::PLAYER_MARIO][0] = "p_wii_bldio";
            mario::marioCharBackPoseNames[mario::PLAYER_MARIO][0] = "p_wii_bldio_r";
            msgpatch::msgpatchAddEntry("in_pc_mario", baldMarioName, false);
            msgpatch::msgpatchAddEntry("msg_pc_mario", baldMarioDesc, false);
            item_data::itemDataTable[item_data::ITEM_ID_CHAR_MARIO].iconId = TPLPATCH_ICON(ICON_BALDIO);
        } else {
            mario::marioCharPoseNames[mario::PLAYER_MARIO][0] = "p_wii_mario";
            mario::marioCharBackPoseNames[mario::PLAYER_MARIO][0] = "p_wii_mario_r";
            msgpatch::msgpatchDelEntry("in_pc_mario");
            msgpatch::msgpatchDelEntry("msg_pc_mario");
            item_data::itemDataTable[item_data::ITEM_ID_CHAR_MARIO].iconId = icondrv::ICON_MARIO;
        }
        return;
    }

    s32 lpGetDanLv() {
        s32 floor = swdrv::swByteGet(1), lv;
        if (floor < 25) {
            lv = 1;
        } else if (floor < 150) {
            lv = 2;
        } else if (floor < 175) {
            lv = 3;
        } else {
            lv = 4;
        }
        return lv;
    }

    s32 lpGetFloorNum() {
        s32 floor = swdrv::swByteGet(1);
        if (floor >= 100)
            floor -= 100;
        return floor;
    }

    s32 lpGetDifficulty() {
        return (s32)Lunatic->Misc.difficulty;
    }

    s32 evt_lp_get_difficulty(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], (s32)Lunatic->Misc.difficulty);
        return 2;
    }

    s32 evt_lp_set_difficulty(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        Lunatic->Misc.difficulty = (LPDifficulty)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        return 2;
    }

    bool npcIsShellEnemy(npcdrv::NPCEntry * npc) {
        if (npc->templateKouraKickScript != nullptr)
            return true;
        return false;
    }

    bool npcCheckHasKey(npcdrv::NPCEntry * npc, bool checkForChestKey, s32 itemIdToChk) {
        s32 item = itemIdToChk;
        if (item == 0)
            item = npc->dropItemId;
        if (checkForChestKey && item == ITEM_ID_KEY_MAC_KEY_00)
            return true;
        if (item == ITEM_ID_KEY_DAN_KEY)
            return true;
        return false;
    }

    bool npcCheckDanFlag(npcdrv::NPCEntry * npc, u32 flag) {
        if (((u32)npc->unkShellSfx & flag) != 0)
            return true;
        return false;
    }

    void npcSetDanFlag(npcdrv::NPCEntry * npc, u32 flag) {
        u32 f = (u32)npc->unkShellSfx;
        f |= flag;
        npc->unkShellSfx = (const char *)f;
        return;
    }

    void npcClearDanFlag(npcdrv::NPCEntry * npc, u32 flag) {
        u32 f = (u32)npc->unkShellSfx;
        f &= ~flag;
        npc->unkShellSfx = (const char *)f;
        return;
    }

    void npcMakeHolo(npcdrv::NPCEntry * npc) {
        npcSetDanFlag(npc, DAN_NPC_HOLOGRAPHIC);
        if (npc->maxHp > 1 || npc->tribeId == NPC_GOOMBA) {
            npc->maxHp = (npc->maxHp + 10) * 2;
            npc->hp = npc->maxHp;
        }
        if ((npc->m_Anim).m_nPoseId != -1)
            animdrv::animPoseSetDispCallback2((npc->m_Anim).m_nPoseId, (void *)mi4::mi4MimiHolographicEffect, nullptr);
        npcdrv::NPCPart * part = npc->parts;
        while (part != nullptr) {
            if (part->m_Anim.m_nPoseId != -1)
                animdrv::animPoseSetDispCallback2((part->m_Anim).m_nPoseId, (void *)mi4::mi4MimiHolographicEffect, nullptr);
            part = part->nextPart;
        }
        return;
    }

    void npcMakeNegative(npcdrv::NPCEntry * npc) {
        npcSetDanFlag(npc, DAN_NPC_NEGATIVE);
        if (npc->maxHp >= 2) {
            npc->maxHp = round((f32)npc->maxHp / 3.0f);
            if (npc->maxHp == 0)
                npc->maxHp = 1;
            npc->hp = npc->maxHp;
        }
        if ((npc->m_Anim).m_nPoseId != -1)
            animdrv::animPoseSetDispCallback2((npc->m_Anim).m_nPoseId, (void *)DanEnemyNegativeDispCb, nullptr);
        npcdrv::NPCPart * part = npc->parts;
        while (part != nullptr) {
            if (part->m_Anim.m_nPoseId != -1)
                animdrv::animPoseSetDispCallback2((part->m_Anim).m_nPoseId, (void *)DanEnemyNegativeDispCb, nullptr);
            part = part->nextPart;
        }
        return;
    }

    const char * npcTribeToName(s32 tribeId) {
        return msgdrv::msgSearch(item_data::itemDataTable[npcdrv::npcTribes[tribeId].catchCardItemId].nameMsg);
    }

    s32 evt_mario_set_invincibility(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        f32 frames = evtmgr_cmd::evtGetFloat(evtEntry, args[0]);
        s32 type = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        mario::marioSetInvincibility(frames, type);
        return 2;
    }

    s32 ToggleGSWF(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 gswf = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        if (swdrv::swGet(gswf) == false)
            swdrv::swSet(gswf);
        else
            swdrv::swClear(gswf);
        return 2;
    }

    s32 MsgIconReplaceIdx(evtmgr::EvtEntry * evtEntry, bool firstRun) {
        (void)firstRun;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 idx = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 iconId = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        msgdrv::msgdrv_msgIcon[idx].iconId = iconId;
        return 2;
    }

}
