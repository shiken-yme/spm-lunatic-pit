#include "patch.h"
#include "sndpatch.h"

#include <common.h>
#include <spm/memory.h>
#include <spm/spmario_snd.h>
#include <spm/system.h>
#include <msl/string.h>
#include <wii/os.h>

namespace mod::sndpatch
{
    using namespace spm;
    using namespace spm::spmario_snd;
    using namespace spm::memory;
    using namespace msl::string;

    void sndpatchExpandBGMSlots()
    {
        // writeWord(readWiiMarioSndDat, 0x1C8, (u32)(0x28000000 + SNDPATCH_BGM_MAX));
        // writeWord(spsndInit, 0x158, (u32)(0x38800000 + (u32)(sizeof(BgmEntry) * SNDPATCH_BGM_MAX)));

        /*
            Patch checks that cause BGMOn to stop iterating after 150 entries
        */
        writeWord(spsndBGMOn_f_d, 0xB8, CMPWI(29, SNDPATCH_BGM_MAX));
        writeWord(spsndBGMOn_f_d, 0xC0, CMPWI(29, SNDPATCH_BGM_MAX));
        /*
            Reallocate spsbgmlist to a larger block
        */
        BgmEntry *newBgmList = (BgmEntry *)__memAlloc(HEAP_SOUND, (u32)(sizeof(BgmEntry) * SNDPATCH_BGM_MAX));
        memcpy(&newBgmList[0], &spsbgmlist[0], (u32)(sizeof(BgmEntry) * spsnd_work.bgmCount));
        __memFree(HEAP_SOUND, &spsbgmlist[0]);
        spsbgmlist = newBgmList;
        return;
    }

    void sndpatchAddBGMEntry(BgmEntry *entry)
    {
        assert(spsnd_work.bgmCount < SNDPATCH_BGM_MAX, "Fix your shit or enlarge SNDPATCH_BGM_MAX in sndpatch.h lmao");
        memcpy(&spsbgmlist[spsnd_work.bgmCount++], entry, sizeof(BgmEntry));
    }

    void sndpatchAddBGMEntryDirect(const char *name, s16 brsarIdx, u8 volume, u8 pan, u8 flags, u8 otoyaId)
    {
        BgmEntry entry = {name, brsarIdx, 6, volume, pan, flags, otoyaId};
        sndpatchAddBGMEntry(&entry);
    }

    /*void fuckthewiimotelowkeybruh() {
        SfxEntry * sfx = spssfxlist;
        for (s32 i = 0; i < spsnd_work.sfxCount; i += 1) {
            sfx->setupFlags &= ~0x18;
            sfx++;
        }
        return;
    }*/

    void sndpatchInit()
    {
        sndpatchExpandBGMSlots();
        //fuckthewiimotelowkeybruh();
        return;
    }
}
