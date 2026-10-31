#include "globalop.h"
#include "patch.h"

#include <common.h>
#include <msl/string.h>
#include <spm/evtmgr.h>
#include <spm/evtmgr_cmd.h>
#include <spm/mario.h>
#include <spm/memory.h>
#include <spm/system.h>
#include <wii/os.h>

namespace mod::globalop {
    using namespace spm;

    GlobalOperations * GlobalOp = nullptr;

    s32 globalopAddEntry(void * func, void * param) {
        GlobalOp->Operations[GlobalOp->entryCount].func = (GlobalOpFunc *)func;
        GlobalOp->Operations[GlobalOp->entryCount].param = param;
        GlobalOp->Operations[GlobalOp->entryCount].deleteIdx = GlobalOp->entryCount;
        GlobalOp->entryCount += 1;
        return GlobalOp->Operations[GlobalOp->entryCount].deleteIdx;
    }

    void globalopDelEntry(s32 deleteIdx) {
        GlobalOp->Operations[deleteIdx].func = nullptr;
        GlobalOp->Operations[deleteIdx].param = nullptr;
        GlobalOp->Operations[deleteIdx].deleteIdx = 0;
        deleteIdx += 1;
        // Sort
        for (s32 i = deleteIdx; i < GlobalOp->entryCount; i += 1) {
            if ((u32)GlobalOp->Operations[i].func == 0)
                break;
            GlobalOp->Operations[i - 1].func = GlobalOp->Operations[i].func;
            GlobalOp->Operations[i - 1].param = GlobalOp->Operations[i].param;
            GlobalOp->Operations[i - 1].deleteIdx = i - 1;
            GlobalOp->Operations[i].func = nullptr;
            GlobalOp->Operations[i].param = nullptr;
            GlobalOp->Operations[i].deleteIdx = 0;
        }
        GlobalOp->entryCount -= 1;
        return;
    }

    void globalopMain() {
        if (GlobalOp->entryCount == 0)
            return;
        for (s32 i = 0; i < GlobalOp->entryCount; i += 1) {
            if (GlobalOp->Operations[i].param == nullptr)
                (GlobalOp->Operations[i].func)(nullptr, GlobalOp->Operations[i].deleteIdx);
            else
                (GlobalOp->Operations[i].func)(GlobalOp->Operations[i].param, GlobalOp->Operations[i].deleteIdx);
        }
        mario::marioHandleSquash(); // Call the function the branchlink overwrites
    }

    void GlobalOperationsInit() {
        GlobalOp = (GlobalOperations *)memory::__memAlloc(memory::HEAP_MAIN, sizeof(GlobalOperations));
        msl::string::memset(GlobalOp, 0, sizeof(GlobalOperations));
        writeBranchLink(mario::marioMain, 0x48C, globalopMain);
    }
}
