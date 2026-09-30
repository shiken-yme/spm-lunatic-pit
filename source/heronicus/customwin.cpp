#include "customwin.h"
#include "patch.h"

#include <msgpatch.h>

#include <common.h>
#include <msl/math.h>
#include <msl/stdio.h>
#include <msl/string.h>
#include <spm/camdrv.h>
#include <spm/evt_msg.h>
#include <spm/evtmgr.h>
#include <spm/evtmgr_cmd.h>
#include <spm/fontmgr.h>
#include <spm/gxsub.h>
#include <spm/item_data.h>
#include <spm/mario.h>
#include <spm/mario_motion.h>
#include <spm/mario_pouch.h>
#include <spm/memory.h>
#include <spm/msgdrv.h>
#include <spm/pausewin.h>
#include <spm/spmario.h>
#include <spm/spmario_snd.h>
#include <spm/sptexture.h>
#include <spm/system.h>
#include <spm/windowdrv.h>
#include <spm/winmgr.h>
#include <spm/wpadmgr.h>
#include <wii/mem.h>
#include <wii/os.h>
#include <wii/tpl.h>
#include <wii/wpad.h>

namespace mod::customwin {
    /*
        CustomWin strives to provide a streamlined way to create window menus with custom options, icons, and text
        ...despite pausewin and winmgr's inherent inflexibilities.

        This library is largely powered by MsgPatch, a library that rewrites msgSearch functions to take input from
        MsgPatch entries, which can override any text entry in SPM without much user input.

        This library is also meant to be interfaced with primarily in EVT scripts, meaning that you will need evtpatch to
        run scripts with your custom menus defined within them.

        To use this library:
        * Add msgpatch to your project if it is not already present
        * Add customwin to your project too
        * You're also gonna need evtpatch
        * Call evtpatch::evtmgrExtensionInit() somewhere in your project
        * Call msgpatchMain() near the top of your main function in mod.cpp
        * Call customwinMain() somewhere below that

        CustomWin is a work-in-progress; more window menu types will be offered in the future, potentially including
        the ability to create your own windows and menus entirely.

        Here's a short guide on how to create a custom item select menu (CWSelect), for example:
        * Set up an EVT script in which you want the select menu to appear.
            (It'll probably look a little like this:)

            EVT_BEGIN()

            // Creates a CWSelect menu named "Example" with the default select menu type, the header title "My Menu," and "Something!!!" on the top-left.
            // The last two fields, which correspond to a CWSelectItemDesc array ptr & the size of the array respectively, are left as 0 here.
            // You can manually define these fields if you want a more complex select menu implementation!
                USER_FUNC(EvtCWSelectEntry, PTR("Example"), CWSELECT_DEFAULT, PTR("My Menu"), PTR("Something!!!"), 0, 0)

            // Adds a listing to the CWSelect entry with the name "Free Candy", the description "Just for you!", and Honey Candy's wicon iconId.
            // The last two fields correspond to a "cost" and a ptr to a custom text GXColor (if left blank, defaults to black).
            // Cost is only relevant for menus of type CWSELECT_SHOP and will not display on default select menus.
                USER_FUNC(EvtCWSelectAddListing, PTR("Example"), PTR("Free Candy"), PTR("Just for you!"), 0xFD, 0, 0)

            // Adds another listing directly below. This one has a price, but it doesn't display or do anything.
                USER_FUNC(EvtCWSelectAddListing, PTR("Example"), PTR("Expensive Candy"), PTR("It's also for you... for the\nright price!"), 0xFD, 9999, 0)

            // Add a vanilla item directly below with the item ID 0xA0 (Egg Bomb); cost is left as 0.
                USER_FUNC(EvtCWSelectAddItem, PTR("Example"), 0xA0, 0)

            // Prepares the specified CWSelect for use, then opens the menu. Takes an optional callback function ptr (not provided) and returns an index.
            // Cancelling the prompt results in -1 being returned as the index; selecting a listing returns the selection index (starting from 0).
            // If provided, the callback runs immediately after the CWSelect and WinmgrSelect entries are set up and uses the WinmgrSelect entry as the parameter.
                USER_FUNC(EvtCWSelectMenuStart, PTR("Example"), 0, LW(0))

            // Gets the item or listing name of the selected index, whichever applies to that item. Takes the index and returns the name as a char ptr.
                USER_FUNC(EvtCWSelectGetSelectionName, LW(0), LW(1))

            // Deactivates the current active CWSelect entry. Do NOT leave this out, but only use it after you're 100% done with the CWSelect!
                USER_FUNC(EvtCWSelectReset)

                RETURN()
            EVT_END()

        * Profit

        If you'd like to submit a bug report or feature request, contact Yme -- and submit a PR, if you'd like.
        If you'd like to submit a complaint about an inefficiency, don't
    */

    using namespace spm;

    CustomWinWork * GlobalCW = nullptr;

    bool CWInitBlockRerun = false;

    /*
        Initialize the global CustomWinWork struct in memory on mod start
    */
    void CWInit() {
        if (CWInitBlockRerun) {
            wii::os::OSReport("CustomWin::CWInit: Redundant initialization attempt has been terminated.\n");
            return;
        }
        CWInitBlockRerun = true;
        GlobalCW = (CustomWinWork *)memory::__memAlloc(memory::HEAP_MAIN, sizeof(CustomWinWork));
        msl::string::memset(GlobalCW, 0, sizeof(CustomWinWork));
        wii::os::OSReport("CustomWin::CWInit: GlobalCW has been allocated; GlobalCW is located at %p\n", GlobalCW);

        // Select
        GlobalCW->selectWinTitleMsgId = "msg_window_title_1";
        GlobalCW->selectWinSelectMsgId = "msg_window_select_1";
        GlobalCW->activeSelect = -1;

        // Msg
        GlobalCW->activeMsgGX = -1;
        return;
    }

    char * CWReallocString(const char * str) {
        char * newstr = (char *)memory::__memAlloc(memory::HEAP_MAIN, msl::string::strlen(str) + 1);
        msl::string::memset(newstr, 0, msl::string::strlen(str) + 1);
        msl::string::strcpy(newstr, str);
        return newstr;
    }

    /* --------------------------------------------------------------------------------------

                                            CWSelect

     -------------------------------------------------------------------------------------- */

    /*
        WindowDesc for custom select menus
    */
    winmgr::WindowDesc CWSelectWindowDescs[2] = {{0, 1, 11, {-24, 140}, {300, 250}, {255, 255, 255, 255}, CWSelect_Main, CWSelect_Disp}, {0, 0, 11, {-280, 120}, {240, 45}, {255, 255, 255, 255}, nullptr, CWSelect_Disp2}};
    winmgr::WindowDesc CWSelectWindowDescs_Info[2] = {{0, 1, 11, {-24, 140}, {300, 250}, {255, 255, 255, 255}, CWSelect_Main_Info, CWSelect_Disp}, {0, 0, 11, {-280, 120}, {240, 45}, {255, 255, 255, 255}, nullptr, CWSelect_Disp2}};

    /*
        Sets a CWSelect entry as the active one, meaning it will be used in EvtCWSelectReset
    */
    s32 CWSelectSetActive(const char * key) {
        for (u32 i = 0; i < CWSELECT_ENTRY_MAX; i += 1) {
            if (msl::string::strcmp("", GlobalCW->SelectKeys[i].name) != 0) {
                if (msl::string::strcmp(GlobalCW->SelectKeys[i].name, key) == 0) {
                    GlobalCW->activeSelect = i;
                    CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectSetActive: Select entry with key name \'%s\' (GlobalCW->Select[%d]) set as active select menu.\n", key, i);
                    return i;
                }
            }
        }
        CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectSetActive: Could not find select entry with key name \'%s\'.\n", key);
        return -1;
    }

    /*
        Gets the numerical ID from a CWSelect entry's key name
    */
    s32 CWSelectKeyToId(const char * key) {
        for (u32 i = 0; i < CWSELECT_ENTRY_MAX; i += 1) {
            if (GlobalCW->SelectKeys[i].name != nullptr) {
                if (msl::string::strcmp(GlobalCW->SelectKeys[i].name, key) == 0)
                    return i;
            }
        }
        CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectKeyToId: Could not find select entry with key name \'%s\'.\n", key);
        return -1;
    }

    /*
        Removes a CWSelect entry from memory
        Does not remove message patches
    */
    void CWSelectDelete(const char * key) {
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectDelete: Entry with key \'%s\' not found; aborting process.\n", key);
            return;
        }
        if (GlobalCW->Select[id]->type == CWSELECT_INFOGRAPHIC) {
            for (s32 i = 0; i < CWSELECT_PAGE_MAX; i += 1) {
                msl::string::memset(GlobalCW->Select[id]->Info.Pages[i].Descs, 0, (GlobalCW->Select[id]->Info.Pages[i].num * sizeof(CWSelectItemDesc)));
                memory::__memFree(memory::HEAP_MAIN, GlobalCW->Select[id]->Info.Pages[i].Descs);
                msl::string::memset(&GlobalCW->Select[id]->Info.Pages[i], 0, (GlobalCW->Select[id]->Info.Pages[i].num * sizeof(CWSelectPageDef)));
                memory::__memFree(memory::HEAP_MAIN, &GlobalCW->Select[id]->Info.Pages[i]);
            }
        }
        if (GlobalCW->Select[id]->windowTitle != nullptr)
            memory::__memFree(memory::HEAP_MAIN, GlobalCW->Select[id]->windowTitle);
        if (GlobalCW->Select[id]->windowSelect != nullptr)
            memory::__memFree(memory::HEAP_MAIN, GlobalCW->Select[id]->windowSelect);
        if (GlobalCW->Select[id]->Sfx.closeSfx != nullptr)
            memory::__memFree(memory::HEAP_MAIN, GlobalCW->Select[id]->Sfx.closeSfx);
        for (u32 i = 0; i < GlobalCW->Select[id]->resetNum; i += 1) {
            if (GlobalCW->Select[id]->Descs[i].deallocStrings) {
                if (GlobalCW->Select[id]->Descs[i].nameTxt != nullptr)
                    memory::__memFree(memory::HEAP_MAIN, GlobalCW->Select[id]->Descs[i].nameTxt);
                if (GlobalCW->Select[id]->Descs[i].descTxt != nullptr)
                    memory::__memFree(memory::HEAP_MAIN, GlobalCW->Select[id]->Descs[i].descTxt);
            }
        }
        memory::__memFree(memory::HEAP_MAIN, GlobalCW->SelectKeys[id].name);
        GlobalCW->SelectKeys[id].name = nullptr;
        GlobalCW->SelectKeys[id].id = -1;
        msl::string::memset(GlobalCW->Select[id]->Descs, 0, (GlobalCW->Select[id]->num * sizeof(CWSelectItemDesc)));
        memory::__memFree(memory::HEAP_MAIN, GlobalCW->Select[id]->Descs);
        msl::string::memset(GlobalCW->Select[id], 0, sizeof(CWSelect));
        memory::__memFree(memory::HEAP_MAIN, GlobalCW->Select[id]);
        GlobalCW->Select[id] = nullptr;
        CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectDelete: Entry with key \'%s\' has been removed.\n", key);
        return;
    }

    /*
        Returns the current active CWSelect entry
    */
    CWSelect * CWSelectGetActiveEntry() {
        if (GlobalCW->activeSelect == -1)
            return nullptr;
        return GlobalCW->Select[GlobalCW->activeSelect];
    }

    /*
        Creates a CustomWin select entry
    */
    CWSelect * CWSelectEntry(const char * key, CWSelectType type, bool setActive) {
        /*if (msl::string::strlen(key) > CWKEY_NAME_LENGTH) {
            CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectEntry: Key \'%s\' is longer than CWKEY_NAME_LENGTH (%d). Failed to create select entry.\n", key, CWKEY_NAME_LENGTH);
        }*/
        for (u32 i = 0; i < CWSELECT_ENTRY_MAX; i += 1) {
            if (GlobalCW->Select[i] == nullptr) {
                GlobalCW->Select[i] = (CWSelect *)memory::__memAlloc(memory::HEAP_MAIN, sizeof(CWSelect));
                msl::string::memset(GlobalCW->Select[i], 0, sizeof(CWSelect));
                GlobalCW->SelectKeys[i].id = i;
                GlobalCW->Select[i]->type = type;
                GlobalCW->SelectKeys[i].name = CWReallocString(key);
                if (setActive) {
                    GlobalCW->activeSelect = i;
                    CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectEntry: New select entry with key \'%s\' (GlobalCW->Select[%d]) created. Set as active select menu.\n", key, i);
                } else
                    CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectEntry: New select entry with key \'%s\' (GlobalCW->Select[%d]) created.\n", key, i);
                return GlobalCW->Select[i];
            }
        }
        CWDEBUG_OSREPORT_FMT("CustomWin::CWSelectEntry: Select entries maxed out at CWSELECT_ENTRY_MAX (%d), cannot create new entry.\n", CWSELECT_ENTRY_MAX);
        return nullptr;
    }

    /*
        ** User Function **
        Creates a CustomWin select entry if one with the provided key name does not exist
        If it does exist, deletes it from memory and recreates it using provided parameters
    */
    s32 EvtCWSelectEntry(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        CWSelectType type = (CWSelectType)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        char * title = (char *)evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        char * select = (char *)evtmgr_cmd::evtGetValue(evtEntry, args[3]);
        CWSelectItemDesc * descs = (CWSelectItemDesc *)evtmgr_cmd::evtGetValue(evtEntry, args[4]);
        s32 numDescs = evtmgr_cmd::evtGetValue(evtEntry, args[5]);
        // Check to see if it exists first, then delete entirely
        s32 id = CWSelectKeyToId(key);
        if (id != -1)
            CWSelectDelete(key);
        CWSelect * entry = CWSelectEntry(key, type, false);
        if (title != nullptr)
            entry->windowTitle = CWReallocString(title);
        if (select != nullptr)
            entry->windowSelect = CWReallocString(select);
        // Establish listing descs in memory
        entry->Descs = (CWSelectItemDesc *)memory::__memAlloc(memory::HEAP_MAIN, sizeof(CWSelectItemDesc) * CWSELECT_DESC_MAX);
        msl::string::memset(entry->Descs, 0, sizeof(CWSelectItemDesc) * CWSELECT_DESC_MAX);
        if (descs != nullptr) {
            msl::string::memcpy(entry->Descs, descs, sizeof(CWSelectItemDesc) * numDescs);
            entry->num = numDescs;
            entry->resetNum = numDescs;
        }
        // Establish page defs if select is type Info
        if (type == CWSELECT_INFOGRAPHIC) {
            entry->Info.numPages = 1;
            entry->Info.Pages = (CWSelectPageDef *)memory::__memAlloc(memory::HEAP_MAIN, sizeof(CWSelectPageDef) * CWSELECT_PAGE_MAX);
            msl::string::memset(entry->Info.Pages, 0, sizeof(CWSelectPageDef) * CWSELECT_PAGE_MAX);
            entry->Info.Pages[0].windowTitle = entry->windowTitle;
            entry->Info.Pages[0].windowSelect = entry->windowSelect;
        }
        return 2;
    }

    /*
        ** User Function **
        Adds a listing to the bottom of the provided CWSelect entry
        Listings can have any wicon iconId, name, description, cost, and line color you want
        Line color defaults to black; cost only relevant if CWSelect is Shop; page only relevant if CWSelect is Info
    */
    s32 EvtCWSelectAddListing(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        char * name = (char *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        char * desc = (char *)evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        s32 iconId = evtmgr_cmd::evtGetValue(evtEntry, args[3]);
        s32 cost = evtmgr_cmd::evtGetValue(evtEntry, args[4]);
        u8 page = (u8)evtmgr_cmd::evtGetValue(evtEntry, args[5]);
        wii::gx::GXColor * color = (wii::gx::GXColor *)evtmgr_cmd::evtGetValue(evtEntry, args[6]);
        wii::gx::GXColor colorFallback = {0, 0, 0, 255};
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectAddListing: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        if (color == nullptr)
            color = &colorFallback;
        CWSelect * Entry = GlobalCW->Select[id];
        Entry->Descs[Entry->num].cost = cost;
        Entry->Descs[Entry->num].iconId = iconId;
        Entry->Descs[Entry->num].itemId = -1;
        Entry->Descs[Entry->num].nameTxt = CWReallocString(name);
        Entry->Descs[Entry->num].descTxt = CWReallocString(desc);
        Entry->Descs[Entry->num].deallocStrings = true;
        Entry->Descs[Entry->num].nameColor = *color;
        Entry->Descs[Entry->num].page = page;
        Entry->num += 1;
        Entry->resetNum = Entry->num;
        return 2;
    }

    /*
        ** User Function **
        Adds a regular item to the bottom of the provided CWSelect entry
        Items can have any cost; leave it as -1 for default buy price
    */
    s32 EvtCWSelectAddItem(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 itemId = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 cost = evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        u8 page = (u8)evtmgr_cmd::evtGetValue(evtEntry, args[3]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectAddItem: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        if (cost == -1)
            cost = item_data::itemDataTable[itemId].buyPrice;
        GlobalCW->Select[id]->Descs[GlobalCW->Select[id]->num].cost = cost;
        GlobalCW->Select[id]->Descs[GlobalCW->Select[id]->num].iconId = -1;
        GlobalCW->Select[id]->Descs[GlobalCW->Select[id]->num].itemId = itemId;
        GlobalCW->Select[id]->Descs[GlobalCW->Select[id]->num].page = page;
        GlobalCW->Select[id]->num += 1;
        return 2;
    }

    /*
        ** User Function **
        Overrides default behavior when pressing 2 in a select menu
        Automatically takes the current WinmgrSelect entry as a parameter
        Return true to continue selecting the item, return false to do nothing
    */
    s32 EvtCWSelectOverrideBtnBehavior(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        CWSelectCBId type = (CWSelectCBId)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        CWSelectCallback * callback = (CWSelectCallback *)evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectOverrideBtnBehavior: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        if (type >= BTN_MAX) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectOverrideBtnBehavior: Entry with key \'%s\' has invalid CWSelectCBId type %d; aborting process.\n", key, type);
            return 2;
        }
        GlobalCW->Select[id]->BtnOverrides[type] = callback;
        return 2;
    }

    /*
        ** User Function **
        Free Parking! Does what it says on the board!
        (Refers to the window at the top-left)
    */
    s32 EvtCWSelectHideDescWindow(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectHideDescWindow: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        GlobalCW->Select[id]->hideDescWin = true;
        return 2;
    }

    /*
        ** User Function **
        Sets SFX names for Open, Close, and Decide respectively
        Set to -1 to mute the sfx entirely, 0 or EVT_NULLPTR to leave as default
        Defaults to vanilla SFX names
    */
    s32 EvtCWSelectModifySfx(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        const char * openSfxName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        const char * closeSfxName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        const char * decideSfxName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[3]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectModifySfx: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        CWSelect * Entry = GlobalCW->Select[id];
        if ((s32)openSfxName == -1)
            Entry->Sfx.muteOpenSfx = true;
        else if (openSfxName != nullptr && (s32)openSfxName != EVT_NULLPTR)
            Entry->Sfx.openSfx = CWReallocString(openSfxName);
        if ((s32)closeSfxName == -1)
            Entry->Sfx.muteCloseSfx = true;
        else if (closeSfxName != nullptr && (s32)closeSfxName != EVT_NULLPTR)
            Entry->Sfx.closeSfx = CWReallocString(closeSfxName);
        if ((s32)decideSfxName == -1)
            Entry->Sfx.muteDecideSfx = true;
        else if (decideSfxName != nullptr && (s32)decideSfxName != EVT_NULLPTR)
            Entry->Sfx.decideSfx = CWReallocString(decideSfxName);
        return 2;
    }

    /*
        ** User Function **
        Makes an icon appear at the top-right of the CWSelect menu
        If select type is shop, this icon will default to a spinning coin if not specified
        If select type is not shop and shop icon *is* specified, it will appear
    */
    s32 EvtCWSelectSetShopIcon(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 iconId = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        f32 scale = evtmgr_cmd::evtGetFloat(evtEntry, args[2]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetShopIcon: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        GlobalCW->Select[id]->ShopIcon.id = iconId;
        GlobalCW->Select[id]->ShopIcon.scale = scale;
        return 2;
    }

    /*
        ** User Function **
        Changes the cursor icon & allowsyou to set its scale
        If not called, defaults to the regular pointer icon
    */
    s32 EvtCWSelectSetPointerIcon(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 iconId = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        f32 scale = evtmgr_cmd::evtGetFloat(evtEntry, args[2]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetPointerIcon: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        GlobalCW->Select[id]->PointerIcon.id = iconId;
        GlobalCW->Select[id]->PointerIcon.scale = scale;
        return 2;
    }

    /*
        ** User Function **
        Tells a select entry to instantly open and/or close
        Defaults to regular select menu behavior
    */
    s32 EvtCWSelectSetInstantOpenClose(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        bool instantOpen = (bool)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        bool instantClose = (bool)evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetInstantOpenClose: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        GlobalCW->Select[id]->instantOpen = instantOpen;
        GlobalCW->Select[id]->instantClose = instantClose;
        return 2;
    }

    s32 EvtCWSelectAddInfoPage(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        char * pageName = (char *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        char * pageDesc = (char *)evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectAddInfoPage: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        if (GlobalCW->Select[id]->type != CWSELECT_INFOGRAPHIC) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectAddInfoPage: Entry with key \'%s\' is not of type CWSELECT_INFOGRAPHIC; aborting process.\n", key);
            return 2;
        }
        CWSelect * entry = GlobalCW->Select[id];
        entry->Info.numPages += 1;
        if (pageName != nullptr)
            entry->Info.Pages[entry->Info.numPages - 1].windowTitle = CWReallocString(pageName);
        if (pageDesc != nullptr)
            entry->Info.Pages[entry->Info.numPages - 1].windowSelect = CWReallocString(pageDesc);
        return 2;
    }

    /*
        ** User Function **
        Sets a solid or gradient background color for a CWSelect entry's windows
        * Takes a CWSelectColorDef ptr as input
        * If alpha field is left out, will default to alpha 255 (fully opaque)
        * If the CWSelectColorDef provided has more than 1 entry, it will animate
        * Completely optional; if not called, all windows default to the classic white
    */
    s32 EvtCWSelectSetBGColor(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        CWSelectColorDef * color = (CWSelectColorDef *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 numDefs = evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetBGColor: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        if (color == nullptr) {
            CWDEBUG_OSREPORT("CustomWin::EvtCWSelectSetBGColor: CWSelectColorDef argument provided is a null pointer; aborting process.\n");
            return 2;
        }
        GlobalCW->Select[id]->Colorize.Colors = color;
        GlobalCW->Select[id]->Colorize.numDefs = numDefs;
        if (numDefs > 1) {
            GlobalCW->Select[id]->Colorize.animate = true;
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetBGColor: Set animated BG gradient for entry with key \'%s\'.\n", key);
        } else {
            GlobalCW->Select[id]->Colorize.animate = false;
            if (msl::string::memcmp(&color->colorTop, &color->colorBottom, 4) == 0) {
                CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetBGColor: Set BG color for entry with key \'%s\' to RGBA #%.2X%.2X%.2X%.2X.\n", key, color->colorTop.r, color->colorTop.g, color->colorTop.b, color->colorTop.a);
            } else
                CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetBGColor: Set BG gradient for entry with key \'%s\' to RGBA #%.2X%.2X%.2X%.2X -> #%.2X%.2X%.2X%.2X.\n", key, color->colorTop.r, color->colorTop.g, color->colorTop.b, color->colorTop.a, color->colorBottom.r, color->colorBottom.g, color->colorBottom.b, color->colorBottom.a);
        }
        return 2;
    }

    /*
        ** User Function **
        Sets a solid color as the title header's background color
    */
    s32 EvtCWSelectSetHeaderColor(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        wii::gx::GXColor * color = (wii::gx::GXColor *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetHeaderColor: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        if (color == nullptr) {
            CWDEBUG_OSREPORT("CustomWin::EvtCWSelectSetHeaderColor: GXColor argument provided is a null pointer; aborting process.\n");
            return 2;
        }
        GlobalCW->Select[id]->Colorize.headerColor = *color;
        CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetHeaderColor: Set header color for entry with key \'%s\' to RGBA #%.2X%.2X%.2X%.2X.\n", key, color->r, color->g, color->b, color->a);
        return 2;
    }

    /*
        ** User Function **
        Sets a solid color for the left box's text
    */
    s32 EvtCWSelectSetSelectTextColor(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        wii::gx::GXColor * color = (wii::gx::GXColor *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetSelectTextColor: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        if (color == nullptr) {
            CWDEBUG_OSREPORT("CustomWin::EvtCWSelectSetSelectTextColor: GXColor argument provided is a null pointer; aborting process.\n");
            return 2;
        }
        GlobalCW->Select[id]->Colorize.selectTxtColor = *color;
        CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetSelectTextColor: Set select box text color for entry with key \'%s\' to RGBA #%.2X%.2X%.2X%.2X.\n", key, color->r, color->g, color->b, color->a);
        return 2;
    }

    /*
        ** User Function **
        Sets a solid color for the title header's text
    */
    s32 EvtCWSelectSetHeaderTextColor(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        wii::gx::GXColor * color = (wii::gx::GXColor *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetHeaderTextColor: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        if (color == nullptr) {
            CWDEBUG_OSREPORT("CustomWin::EvtCWSelectSetHeaderTextColor: GXColor argument provided is a null pointer; aborting process.\n");
            return 2;
        }
        GlobalCW->Select[id]->Colorize.headerTxtColor = *color;
        CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectSetHeaderTextColor: Set header text color for entry with key \'%s\' to RGBA #%.2X%.2X%.2X%.2X.\n", key, color->r, color->g, color->b, color->a);
        return 2;
    }

    /*
        ** User Function **
        Prepares a provided CustomWin select entry for use and makes it appear
        Use EvtCWSelectReset once you are done with the select entry to remove all message patches
    */
    s32 EvtCWSelectMenuStart(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        winmgr::WinmgrSelect * menu;
        mario::MarioWork * mario = mario::marioGetPtr();
        wii::mtx::Vec3 pos;
        camdrv::CamEntry * cam;
        s32 selection;
        s32 listingIdx;
        pausewin::PausewinEntry * pausewin;
        winmgr::WinmgrEntry * winEnt;

        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        CWSelectCallback * callback = (CWSelectCallback *)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        if (firstCall) {
            // Prepare the CWSelect entry
            const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
            CWSelectSetActive(key);
            u32 blacklist[CWSelectGetActiveEntry()->num];
            u32 i = 0, j = 0, itemId = 65, idx = 0, size = 0;

            // blacklist autogenning item ids that belong to items
            // CWDEBUG_OSREPORT("CustomWin::EvtCWSelectPrepare: !!! DEBUG !!! Beginning Item ID Blacklist Process\n");
            for (i = 0; i < CWSelectGetActiveEntry()->num; i += 1) {
                if (CWSelectGetActiveEntry()->Descs[i].itemId > 0) {
                    blacklist[size] = CWSelectGetActiveEntry()->Descs[i].itemId;
                    // CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectPrepare: !!! DEBUG !!! Item ID %d has been reserved!\n", blacklist[size]);
                    size += 1;
                }
            }
            // generate item ids, avoid anything in blacklist
            for (i = 0; i < CWSelectGetActiveEntry()->num; i += 1) {
                // For non-items, iterate through blacklist to ensure itemId isn't taken. if it is, increment itemId and reiterate
                if (CWSelectGetActiveEntry()->Descs[i].itemId < 1) {
                    while (j < size) {
                        if (itemId == blacklist[j]) {
                            // CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectPrepare: !!! DEBUG !!! Listing \'%s\' (i = %d) attempted to assign ID %d and encountered blacklisted ID %d. Incrementing ID...\n", CWSelectGetActiveEntry()->Descs[i].nameTxt, i, itemId, blacklist[j]);
                            itemId += 1;
                            j = 0;
                        } else
                            j += 1;
                    }
                    // CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectPrepare: !!! DEBUG !!! Listing \'%s\' (i = %d) has successfully assigned ID %d.\n", CWSelectGetActiveEntry()->Descs[i].nameTxt, i, itemId);
                    CWSelectGetActiveEntry()->itemTable[i] = itemId;
                    msgpatch::msgpatchAddEntry(item_data::itemDataTable[itemId].nameMsg, CWSelectGetActiveEntry()->Descs[i].nameTxt, 1);
                    msgpatch::msgpatchAddEntry(item_data::itemDataTable[itemId].descMsg, CWSelectGetActiveEntry()->Descs[i].descTxt, 1);
                    itemId += 1;
                } else {
                    // CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectPrepare: !!! DEBUG !!! Item \'%s\' (i = %d) has successfully assigned ID %d.\n", msgdrv::msgSearch(item_data::itemDataTable[CWSelectGetActiveEntry()->Descs[i].itemId].nameMsg), i, blacklist[idx]);
                    CWSelectGetActiveEntry()->itemTable[i] = blacklist[idx];
                    idx += 1;
                }
                j = 0;
            }
            CWSelectGetActiveEntry()->itemTable[i] = -1;
            msl::string::memcpy(CWSelectGetActiveEntry()->resetItemTable, CWSelectGetActiveEntry()->itemTable, sizeof(CWSelectGetActiveEntry()->itemTable));
            if (CWSelectGetActiveEntry()->windowTitle != nullptr)
                msgpatch::msgpatchAddEntry(GlobalCW->selectWinTitleMsgId, CWSelectGetActiveEntry()->windowTitle, 1);
            if (CWSelectGetActiveEntry()->windowSelect != nullptr)
                msgpatch::msgpatchAddEntry(GlobalCW->selectWinSelectMsgId, CWSelectGetActiveEntry()->windowSelect, 1);
            // Reset colorize timer just in case
            CWSelectGetActiveEntry()->Colorize.timer = 0;
            // CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectPrepare: Select menu item table prepared; custom menu is ready to display. Item Table: %p\n", &CWSelectGetActiveEntry()->itemTable);

            // Prepare the WinmgrSelect entry
            menu = winmgr::winMgrSelectEntry((s32)&CWSelectGetActiveEntry()->itemTable, 0, 1);
            if (CWSelectGetActiveEntry()->type == CWSELECT_INFOGRAPHIC) {
                winmgr::winmgr_wp->entries[menu->entryIds[0]].desc = &CWSelectWindowDescs_Info[0];
                winmgr::winmgr_wp->entries[menu->entryIds[1]].desc = &CWSelectWindowDescs_Info[1];
            } else {
                winmgr::winmgr_wp->entries[menu->entryIds[0]].desc = &CWSelectWindowDescs[0];
                winmgr::winmgr_wp->entries[menu->entryIds[1]].desc = &CWSelectWindowDescs[1];
            }
            // CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectMenuStart: WinmgrSelect entry is @ %p\n", menu);
            evtEntry->tempU[0] = (u32)menu;
            evtEntry->tempF[1] = 0.0;
            evtEntry->tempS[2] = 0;
            evtEntry->tempU[3] = (u32)mario->motionId;

            // Remove the white box from the pausewin entry (description box)
            pausewin = pausewin::pausewinGetEntry(menu->pausewinId);
            pausewin->boxOnly = 2;
            // Run callback if it exists
            if (callback != nullptr) {
                (callback)(menu);
            }
            // Init pre-menu appear effects
            if (!CWSelectGetActiveEntry()->Sfx.muteOpenSfx) {
                if (CWSelectGetActiveEntry()->Sfx.openSfx == nullptr)
                    spmario_snd::spsndSFXOn("SFX_SYS_MENU_OPEN1");
                else
                    spmario_snd::spsndSFXOn(CWSelectGetActiveEntry()->Sfx.openSfx);
            }
            mario::marioKeyOff();
            if ((mario->miscFlags & MARIO_MISC_FLAG_SPACE_SWIM) == 0)
                mario::marioChgPose("P_1");
            mario->dispFlags |= MARIO_DISP_FLAG_LOCK_POSE;

            if (CWSelectGetActiveEntry()->instantOpen) {
                winEnt = &winmgr::winmgr_wp->entries[menu->entryIds[0]];
                winEnt->rgba.a = 255;
                winEnt->scalarScale = 1.0f;
                pausewin->state = 12;        // STATE_NEUTRAL
                pausewin->stateProgress = 0; // jic
                pausewin->alpha = 255;
            }
        }
        menu = (winmgr::WinmgrSelect *)evtEntry->tempU[0];
        winEnt = &winmgr::winmgr_wp->entries[menu->entryIds[0]];
        pausewin = pausewin::pausewinGetEntry(menu->pausewinId);
        cam = camdrv::camGetPtr(mario->camId);
        pos.x += (f32)(msl::math::cos(cam->targetAngle * 57.29578) * 50.0);
        pos.y = mario->position.y;
        pos.z += -(f32)(msl::math::sin(cam->targetAngle * 57.29578)) * 50.0;
        switch (evtEntry->tempS[2]) {
        case 0:
            if (!CWSelectGetActiveEntry()->instantOpen) {
                evtEntry->tempF[1] += 0.05;
                if (1.0 < evtEntry->tempF[1])
                    evtEntry->tempF[1] = 1.0;
                camdrv::func_800586c8(2.0, evtEntry->tempF[1], 0.2, 5, &pos);
            }
            if (!CWSelectGetActiveEntry()->instantClose) {
                selection = winmgr::winMgrSelect(menu);
                if (selection != 0)
                    evtEntry->tempS[2] = 1;
            } else if (pausewin->alpha == 0 && pausewin->state == 21) {
                evtEntry->tempS[2] = 2;
                goto case2;
            }
            break;
        case 1:
            evtEntry->tempF[1] -= 0.05;
            if (0.0 > evtEntry->tempF[1]) {
                evtEntry->tempF[1] = 0.0;
                evtEntry->tempS[2] = 2;
            }
            camdrv::func_800586c8(2.0, evtEntry->tempF[1], 0.2, 5, &pos);
            break;
        case 2:
        case2:
            selection = winmgr::winMgrSelect(menu);
            listingIdx = winmgr::winMgrSelectGetIndex(menu);
            mario::marioKeyOn();
            mario->dispFlags &= ~MARIO_DISP_FLAG_LOCK_POSE;
            mario_motion::marioChgMot(evtEntry->tempU[3]);
            if (selection <= 0) {
                evtmgr_cmd::evtSetValue(evtEntry, args[2], -1);
            } else
                evtmgr_cmd::evtSetValue(evtEntry, args[2], listingIdx);
            winmgr::winMgrSelectDelete(menu);
            return 2;
            break;
        }
        return 0;
    }

    /*
        ** User Function **
        Gets the item or listing name for a selected index, whichever applies
    */
    s32 EvtCWSelectGetSelectionName(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 idx = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        if (idx < 0) {
            evtmgr_cmd::evtSetValue(evtEntry, args[1], -1);
        } else if (CWSelectGetActiveEntry()->Descs[idx].itemId > 0) {
            evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)msgdrv::msgSearch(item_data::itemDataTable[CWSelectGetActiveEntry()->Descs[idx].itemId].nameMsg));
        } else
            evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)&CWSelectGetActiveEntry()->Descs[idx].nameTxt);
        return 2;
    }

    /*
        ** User Function **
        Gets an item or listing's cost for a selected index
    */
    s32 EvtCWSelectGetSelectionCost(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 idx = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        if (idx < 0) {
            evtmgr_cmd::evtSetValue(evtEntry, args[1], -1);
        } else
            evtmgr_cmd::evtSetValue(evtEntry, args[1], CWSelectGetActiveEntry()->Descs[idx].cost);
        return 2;
    }

    /*
        ** User Function **
        Gets an item's ID from the selected index
        Only meant for listings that are specifically items
    */
    s32 EvtCWSelectGetSelectionItemId(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        s32 idx = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        if (idx < 0) {
            evtmgr_cmd::evtSetValue(evtEntry, args[1], -1);
        } else
            evtmgr_cmd::evtSetValue(evtEntry, args[1], CWSelectGetActiveEntry()->Descs[idx].itemId);
        return 2;
    }

    /*
        ** User Function **
        Removes a listing/item from the provided Descs/itemTable index in a CWSelect entry
        Immediately sorts all listing data
        Not yet compatible with CWSELECT_INFOGRAPHIC
        Entirely untested; I cannot assure intended functionality in any use case
    */
    s32 EvtCWSelectRemoveListing(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        u32 idx = (u32)evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 id = CWSelectKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWSelectRemoveListing: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        GlobalCW->Select[id]->num -= 1;
        for (; idx < GlobalCW->Select[id]->num; idx += 1) {
            msl::string::memcpy(&GlobalCW->Select[id]->Descs[idx], &GlobalCW->Select[id]->Descs[idx + 1], sizeof(CWSelectItemDesc));
            GlobalCW->Select[id]->itemTable[idx] = GlobalCW->Select[id]->itemTable[idx + 1];
        }
        return 2;
    }

    /*
        ** User Function **
        Clears all live message patches for the current CWSelect instance
    */
    s32 EvtCWSelectReset(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        (void)evtEntry;
        u32 i = 0;
        s32 id = GlobalCW->activeSelect;
        if (id == -1) {
            CWDEBUG_OSREPORT("CustomWin::EvtCWSelectReset: There is no active CWSelect entry; aborting process.\n");
            return 2;
        }
        for (i = 0; i < GlobalCW->Select[id]->resetNum; i += 1) {
            // Clear msgpatches only if type is not item
            if (GlobalCW->Select[id]->Descs[i].itemId < 1) {
                msgpatch::msgpatchDelEntry(item_data::itemDataTable[GlobalCW->Select[id]->resetItemTable[i]].nameMsg);
                msgpatch::msgpatchDelEntry(item_data::itemDataTable[GlobalCW->Select[id]->resetItemTable[i]].descMsg);
            }
        }
        if (GlobalCW->Select[id]->windowSelect != nullptr)
            msgpatch::msgpatchDelEntry(GlobalCW->selectWinSelectMsgId);
        if (GlobalCW->Select[id]->windowTitle != nullptr)
            msgpatch::msgpatchDelEntry(GlobalCW->selectWinTitleMsgId);
        GlobalCW->activeSelect = -1;
        CWDEBUG_OSREPORT("CustomWin::EvtCWSelectReset: All live CWSelect message patches have been reset.\n");
        return 2;
    }

    /*
        ** User Function **
        Completely removes a CWSelect entry from memory.
        Please use EvtCWSelectReset first to properly clear msgpatches!
    */
    s32 EvtCWSelectDelete(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        CWSelectDelete(key);
        return 2;
    }

    /*

        !!! MISC HELPER FUNCTIONS AND VANILLA FUNCTION REWRITES !!!

    */

    void CWDebug_GetMainHeapFreeSpace(bool alloc) {
        if (CUSTOMWIN_DEBUG) {
            wii::mem::MEMEXPHeap * exph = (wii::mem::MEMEXPHeap *)memory::memory_wp->heapHandle[0];
            u32 size = (u32)exph->end - (u32)exph->start;
            u32 rem = 0;
            for (wii::mem::MEMAllocation * p = exph->firstFree; p; p = p->next)
                rem += p->size;
            if (size == (rem + sizeof(wii::mem::MEMAllocation)))
                rem += sizeof(wii::mem::MEMAllocation);
            if (alloc) {
                CWDEBUG_OSREPORT_FMT("CustomWin::CWDebug_GetMainHeapFreeSpace: mem allocation! main heap has %d/%d space remaining (%.4f)\n", rem, size, (f32)((rem / size) * 100));
            } else
                CWDEBUG_OSREPORT_FMT("CustomWin::CWDebug_GetMainHeapFreeSpace: mem free! main heap has %d/%d space remaining (%.4f)\n", rem, size, (f32)((rem / size) * 100));
        }
        return;
    }

    wii::gx::GXColor CWColorOverrideRecalcAlpha(wii::gx::GXColor * color, pausewin::PausewinEntry * entry) {
        wii::gx::GXColor ret = *color;
        ret.a = (u8)(msl::math::floor(((f32)entry->alpha / 255.0) * color->a));
        return ret;
    }

    void CWColorOverrideAnimateAction(f32 posX, f32 posY, f32 scaleX, f32 scaleY, pausewin::PausewinEntry * pausewin, s32 id) {
        // Init colors & define the colorize field for readability
        wii::gx::GXColor top;
        wii::gx::GXColor bottom;
        CWSelectColorization * colorize = &GlobalCW->Select[id]->Colorize;

        // Determine current def, as well as whether to interpolate colors based on timer
        u32 currentDef;
        u32 nextDef;
        s32 timer = colorize->timer;
        bool intpl = false;
        for (currentDef = 0; currentDef < colorize->numDefs; currentDef += 1) {
            timer -= colorize->Colors[currentDef].stayTime;
            if (timer < 0) {
                timer += colorize->Colors[currentDef].stayTime;
                break;
            }
            timer -= colorize->Colors[currentDef].transitionTime;
            if (timer < 0) {
                timer += colorize->Colors[currentDef].transitionTime;
                intpl = true;
                break;
            }
            // Assert currentDef = 0 and global timer = 0 if all timer checks fail
            if (currentDef == (colorize->numDefs - 1)) {
                currentDef = 0;
                colorize->timer = 0;
                break;
            }
        }

        // Assert nextDef, and reset nextDef to 0 if currentDef is max
        nextDef = currentDef + 1;
        if (currentDef == (colorize->numDefs - 1))
            nextDef = 0;

        // Display gradient of currentDef if in stayTime
        if (!intpl) {
            top = CWColorOverrideRecalcAlpha(&colorize->Colors[currentDef].colorTop, pausewin);
            bottom = CWColorOverrideRecalcAlpha(&colorize->Colors[currentDef].colorBottom, pausewin);
            gxsub::gxsubInit_Colour();
            gxsub::gxsubDrawQuadGradient(posX, posY, scaleX, scaleY, &top, &bottom);
        } else // Interpolate between current and next color defs
        {
            u8 r = (u8)system::intplGetValue(system::IntplMode::INTPL_MODE_LINEAR, (f32)colorize->Colors[currentDef].colorTop.r, (f32)colorize->Colors[nextDef].colorTop.r, timer, colorize->Colors[currentDef].transitionTime);
            u8 g = (u8)system::intplGetValue(system::IntplMode::INTPL_MODE_LINEAR, (f32)colorize->Colors[currentDef].colorTop.g, (f32)colorize->Colors[nextDef].colorTop.g, timer, colorize->Colors[currentDef].transitionTime);
            u8 b = (u8)system::intplGetValue(system::IntplMode::INTPL_MODE_LINEAR, (f32)colorize->Colors[currentDef].colorTop.b, (f32)colorize->Colors[nextDef].colorTop.b, timer, colorize->Colors[currentDef].transitionTime);
            u8 a = (u8)system::intplGetValue(system::IntplMode::INTPL_MODE_LINEAR, (f32)colorize->Colors[currentDef].colorTop.a, (f32)colorize->Colors[nextDef].colorTop.a, timer, colorize->Colors[currentDef].transitionTime);
            top = {r, g, b, a};
            top = CWColorOverrideRecalcAlpha(&top, pausewin);
            r = (u8)system::intplGetValue(system::IntplMode::INTPL_MODE_LINEAR, (f32)colorize->Colors[currentDef].colorBottom.r, (f32)colorize->Colors[nextDef].colorBottom.r, timer, colorize->Colors[currentDef].transitionTime);
            g = (u8)system::intplGetValue(system::IntplMode::INTPL_MODE_LINEAR, (f32)colorize->Colors[currentDef].colorBottom.g, (f32)colorize->Colors[nextDef].colorBottom.g, timer, colorize->Colors[currentDef].transitionTime);
            b = (u8)system::intplGetValue(system::IntplMode::INTPL_MODE_LINEAR, (f32)colorize->Colors[currentDef].colorBottom.b, (f32)colorize->Colors[nextDef].colorBottom.b, timer, colorize->Colors[currentDef].transitionTime);
            a = (u8)system::intplGetValue(system::IntplMode::INTPL_MODE_LINEAR, (f32)colorize->Colors[currentDef].colorBottom.a, (f32)colorize->Colors[nextDef].colorBottom.a, timer, colorize->Colors[currentDef].transitionTime);
            bottom = {r, g, b, a};
            bottom = CWColorOverrideRecalcAlpha(&bottom, pausewin);
            gxsub::gxsubInit_Colour();
            gxsub::gxsubDrawQuadGradient(posX, posY, scaleX, scaleY, &top, &bottom);
        }
        // CWDEBUG_OSREPORT_FMT("CustomWin::CWColorOverrideAnimateAction: Timer is %d.\n", timer);

        // Increment timer by 1 when subTimer reaches 3 (necessary because function runs 3 times a frame)
        colorize->subTimer += 1;
        if (colorize->subTimer == 3) {
            colorize->timer += 1;
            colorize->subTimer = 0;
        }
        return;
    }

    void CWSelectSetDescriptionMsg(winmgr::WinmgrSelect * select, CWSelect * Entry) {
        pausewin::PausewinEntry * pausewin = pausewin::pausewinGetEntry(select->pausewinId);
        s16 item = Entry->Descs[select->selectedItemIdx].itemId;
        if (item > 0) // Is item
        {
            if (item < 0x11A) // Not a card
            {
                pausewin::pausewinSetMessage(pausewin, item, 0);
            } else
                pausewin::pausewinSetMessage(pausewin, 0, "msg_window_help_card");
        } else // Is listing
            pausewin::pausewinSetMessage(pausewin, Entry->itemTable[select->selectedItemIdx], 0);
    }

    /*
        Rewrite of select_disp, incorporating only necessary elements for CWSelect menus
    */
    void CWSelect_Disp(winmgr::WinmgrEntry * win) {
        if (win->rgba.r == 0 && win->rgba.g == 0 && win->rgba.b == 0 && win->rgba.a == 0)
            return;
        winmgr::WinmgrSelect * select = win->select;
        wii::gx::GXColor hdrTxtCol;
        wii::gx::GXColor iconCol = {255, 255, 255, 255};
        wii::gx::GXColor fogCol = {255, 255, 255, 255};
        wii::gx::GXColor txtCol = {0, 0, 0, 255};
        pausewin::PausewinEntry * pausewin = pausewin::pausewinGetEntry(win->select->pausewinId);
        char cost[8];
        u32 sciX, sciY, sciW, sciH;
        wii::mtx::Mtx34 mtxPos, mtxScale, mtxAnim, mtxRot;
        f32 msgW, xScl;
        CWSelect * CW = CWSelectGetActiveEntry();
        CWSelectItemDesc * Descs = CW->Descs;
        msl::string::memcpy(&hdrTxtCol, &CW->Colorize.headerTxtColor, sizeof(hdrTxtCol));
        if (hdrTxtCol.r == 0 && hdrTxtCol.g == 0 && hdrTxtCol.b == 0 && hdrTxtCol.a == 0)
            hdrTxtCol = {255, 255, 255, 255};
        txtCol.a = win->rgba.a;
        iconCol.a = win->rgba.a;

        wii::gx::GXSetFog(0, 0, 0, 0, 0, &fogCol);
        wii::gx::GXGetScissor(&sciX, &sciY, &sciW, &sciH);
        wii::gx::GXSetScissor(win->pos.x + 304, 274 - win->pos.y, win->scale.x, win->scale.y - 50);

        // Iterate through all select menu items
        for (u32 i = 0; i < CW->num; i += 1) {
            // Determine whether to draw next item
            f32 y = select->scrollProgress - ((25.0 * i) - (win->pos.y - 44));
            if (((y - 32.0) <= win->pos.y) && ((win->pos.y - win->scale.y) <= (y + 32.0))) {
                wii::mtx::PSMTXTrans(mtxPos, win->pos.x + 25, select->scrollProgress - ((25.0 * i) - (win->pos.y - 44)) - 14.0, 0.0);
                wii::mtx::PSMTXScale(mtxScale, 0.6, 0.6, 1.0);
                wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
                // Draw listing icon
                if (Descs[i].iconId != -1) {
                    icondrv::iconDispGxCol(mtxPos, 0x10, Descs[i].iconId, iconCol);
                } else if (Descs[i].itemId > 0)
                    icondrv::iconDispGxCol(mtxPos, 0x10, item_data::itemDataTable[Descs[i].itemId].iconId, iconCol);
                // Draw listing name
                wii::mtx::PSMTXTrans(mtxPos, win->pos.x + 60, select->scrollProgress - ((25.0 * i) - (win->pos.y - 32)), 0.0);
                const char * name = msgdrv::msgSearch(item_data::itemDataTable[CW->itemTable[i]].nameMsg);
                msgW = fontmgr::FontGetMessageWidth(name);
                xScl = win->scale.x - 110.0;
                if (msgW <= xScl) {
                    wii::mtx::PSMTXScale(mtxScale, 1.0, 1.0, 1.0);
                } else
                    wii::mtx::PSMTXScale(mtxScale, (xScl / msgW), 1.0, 1.0);
                wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
                fontmgr::FontDrawStart();
                // Check if custom color is specified; if not, use default
                if (Descs[i].nameColor.r == 0 && Descs[i].nameColor.g == 0 && Descs[i].nameColor.b == 0 && Descs[i].nameColor.a == 0) {
                    fontmgr::FontDrawColor(&txtCol);
                } else
                    fontmgr::FontDrawColor(&Descs[i].nameColor);
                fontmgr::FontDrawMessageMtx(mtxPos, name);
                // If shop, draw listing price
                if (CW->type == CWSELECT_SHOP) {
                    msl::stdio::sprintf(cost, "%d", Descs[i].cost);
                    msgW = fontmgr::FontGetMessageWidth(cost);
                    if (msgW <= 30.0) {
                        wii::mtx::PSMTXScale(mtxScale, 1.0, 1.0, 1.0);
                    } else {
                        wii::mtx::PSMTXScale(mtxScale, (30.0 / msgW), 1.0, 1.0);
                        msgW = 30.0;
                    }
                    wii::mtx::PSMTXTrans(mtxPos, (win->pos.x + win->scale.x - 10.0 - msgW), select->scrollProgress - ((25.0 * i) - (win->pos.y - 32)), 0.0);
                    wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
                    fontmgr::FontDrawStart();
                    fontmgr::FontDrawColor(&txtCol);
                    fontmgr::FontDrawMessageMtx(mtxPos, cost);
                }
            }
        }
        wii::gx::GXSetScissor(sciX, sciY, sciW, sciH);
        // Draw window title in header
        const char * title = msgdrv::msgSearch(GlobalCW->selectWinTitleMsgId);
        msgW = fontmgr::FontGetMessageWidth(title);
        if (msgW <= 120.0) {
            wii::mtx::PSMTXScale(mtxScale, 1.0, 1.0, 1.0);
        } else {
            wii::mtx::PSMTXScale(mtxScale, (120.0 / msgW), 1.0, 1.0);
            msgW = 120.0;
        }
        wii::mtx::PSMTXTrans(mtxPos, (win->scale.x - msgW) * 0.5 + win->pos.x, win->pos.y + 14, 0.0);
        wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
        fontmgr::FontDrawStart_alpha(CWColorOverrideRecalcAlpha(&hdrTxtCol, pausewin).a);
        fontmgr::FontDrawColor(&hdrTxtCol);
        fontmgr::FontDrawMessageMtx(mtxPos, title);
        // Figure out which icon to draw at top-right, if any
        // Assert default shop icon as spinning coin
        if (CW->type == CWSELECT_SHOP && CW->ShopIcon.id == 0) {
            CW->ShopIcon.id = icondrv::ICON_COIN;
            CW->ShopIcon.scale = 0.3;
        }
        // Assert default shop icon as nothing if type is not shop
        if (CW->type != CWSELECT_SHOP && CW->ShopIcon.id == 0)
            CW->ShopIcon.id = -1;
        // Draw icon if shop icon is not -1, even if type is not shop, unless type is INFO
        if (CW->ShopIcon.id != -1 && CW->type != CWSELECT_INFOGRAPHIC) {
            wii::mtx::PSMTXTrans(mtxPos, (win->pos.x + win->scale.x - 25.0), win->pos.y - 16, 0.0);
            wii::mtx::PSMTXScale(mtxScale, CW->ShopIcon.scale, CW->ShopIcon.scale, CW->ShopIcon.scale);
            wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
            icondrv::iconDispGx2(mtxPos, 0x10, CW->ShopIcon.id);
        }
        // Draw up/down arrows to indicate scroll
        if (CW->num > 8) {
            wii::mtx::PSMTXTrans(mtxAnim, 0.0, winmgr::select_disp_arrow_move[spmario::gp->frameCounter % 20], 0.0);
            if (select->scrollIdx != 0) {
                wii::mtx::PSMTXTrans(mtxPos, (win->scale.x * 0.5) + win->pos.x, win->pos.y - 14, 0.0);
                wii::mtx::PSMTXRotRad(PI, mtxRot, 122); // "z", just avoiding a compile warning lol
                wii::mtx::PSMTXScale(mtxScale, 0.6, 0.6, 0.6);
                wii::mtx::PSMTXConcat(mtxPos, mtxRot, mtxPos);
                wii::mtx::PSMTXConcat(mtxPos, mtxAnim, mtxPos);
                wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
                icondrv::iconDispGxCol(mtxPos, 0x4010, icondrv::ICON_POINTING_ARROW, iconCol);
            }
            if ((u32)select->scrollIdx != (CW->num - 8)) {
                wii::mtx::PSMTXTrans(mtxPos, (win->scale.x * 0.5) + win->pos.x, (win->pos.y - win->scale.y), 0.0);
                wii::mtx::PSMTXScale(mtxScale, 0.6, 0.6, 0.6);
                wii::mtx::PSMTXConcat(mtxPos, mtxAnim, mtxPos);
                wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
                icondrv::iconDispGxCol(mtxPos, 0x4010, icondrv::ICON_POINTING_ARROW, iconCol);
            }
        }
        // Draw left/right arrows if we have pages
        if (CW->type == CWSELECT_INFOGRAPHIC) {
            wii::mtx::PSMTXTrans(mtxAnim, 0.0, winmgr::select_disp_arrow_move[spmario::gp->frameCounter % 20], 0.0);
            wii::mtx::PSMTXTrans(mtxPos, (win->scale.x * 0.5) + win->pos.x - 120.0, win->pos.y, 0.0);
            wii::mtx::PSMTXRotRad(-(PI / 2), mtxRot, 122); // "z"
            wii::mtx::PSMTXScale(mtxScale, 0.6, 0.6, 0.6);
            wii::mtx::PSMTXConcat(mtxPos, mtxRot, mtxPos);
            wii::mtx::PSMTXConcat(mtxPos, mtxAnim, mtxPos);
            wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
            icondrv::iconDispGxCol(mtxPos, 0x4010, icondrv::ICON_POINTING_ARROW, iconCol);
            wii::mtx::PSMTXTrans(mtxPos, (win->scale.x * 0.5) + win->pos.x + 120.0, win->pos.y, 0.0);
            wii::mtx::PSMTXRotRad((PI / 2), mtxRot, 122); // "z"
            wii::mtx::PSMTXScale(mtxScale, 0.6, 0.6, 0.6);
            wii::mtx::PSMTXConcat(mtxPos, mtxRot, mtxPos);
            wii::mtx::PSMTXConcat(mtxPos, mtxAnim, mtxPos);
            wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
            icondrv::iconDispGxCol(mtxPos, 0x4010, icondrv::ICON_POINTING_ARROW, iconCol);
        }
        // Draw cursor
        wii::mtx::Vec3 cursorPos = {select->pos.x, select->pos.y, 1.0};
        if (CW->PointerIcon.id == 0) {
            CW->PointerIcon.id = icondrv::ICON_FINGER_POINT_HORIZONTAL;
            CW->PointerIcon.scale = 1.0;
        }
        icondrv::iconDispGx(CW->PointerIcon.scale, &cursorPos, 0x14, CW->PointerIcon.id);
        return;
    }

    void CWSelect_Disp2(winmgr::WinmgrEntry * win) {
        if (CWSelectGetActiveEntry()->hideDescWin)
            return;
        pausewin::PausewinEntry * pausewin = pausewin::pausewinGetEntry(win->select->pausewinId);
        wii::gx::GXColor col;
        wii::mtx::Mtx34 mtxTrans, mtxScale;
        u16 outlines;
        winmgr::WinmgrSelect * select = win->select;
        const char * msg = msgdrv::msgSearch(GlobalCW->selectWinSelectMsgId);
        f32 width = (f32)fontmgr::FontGetMessageWidthLine(msg, &outlines);
        if (width <= (win->scale.x - 30)) {
            wii::mtx::PSMTXScale(mtxScale, 1.0, 1.0, 1.0);
        } else {
            wii::mtx::PSMTXScale(mtxScale, (f32)((win->scale.x - 30) / width), 1.0, 1.0);
            width = (win->scale.x - 30);
        }
        f32 textY = (f32)((win->pos).y - (s32)((win->scale).y + (outlines + 1) * -22) / 2) + (outlines * 4);
        f32 textX = (f32)((win->scale.x - width) * 0.5);
        if (textX < 0.0)
            textX = -textX;
        textX += win->pos.x;
        winmgr::winmgr_wp->entries[select->entryIds[1]].pos.x = win->pos.x;
        winmgr::winmgr_wp->entries[select->entryIds[1]].pos.y = win->desc->pos.y + (s32)(((u32)outlines * 30) / 2);
        winmgr::winmgr_wp->entries[select->entryIds[1]].scale.x = win->scale.x;
        winmgr::winmgr_wp->entries[select->entryIds[1]].scale.y = win->desc->scale.y + ((u32)outlines * 30);
        if (win->rgba.r == 0 && win->rgba.g == 0 && win->rgba.b == 0 && win->rgba.a == 0)
            return;
        if (pausewin->alpha == 0 && CWSelectGetActiveEntry()->instantClose && pausewin->state == 21)
            return;
        msl::string::memcpy(&col, &CWSelectGetActiveEntry()->Colorize.selectTxtColor, sizeof(col));
        if (col.r == 0 && col.g == 0 && col.b == 0 && col.a == 0)
            msl::string::memcpy(&col, &win->rgba, sizeof(win->rgba));
        wii::mtx::PSMTXTrans(mtxTrans, textX, textY, 0.0);
        wii::mtx::PSMTXConcat(mtxTrans, mtxScale, mtxTrans);
        fontmgr::FontDrawStart_alpha(CWColorOverrideRecalcAlpha(&col, pausewin).a);
        fontmgr::FontDrawStart();
        fontmgr::FontDrawColor(&col);
        fontmgr::FontDrawMessageMtx(mtxTrans, msg);
        return;
    }

    void CWWinMgrDisp(s32 camId, winmgr::WinmgrEntry * entry) {
        (void)camId;
        s32 id = GlobalCW->activeSelect;
        if (id > -1) {
            if (entry->desc->type == 0 && CWSelectGetActiveEntry()->hideDescWin)
                return;
        }
        wii::mtx::Mtx34 mtx;
        wii::gx::GXColor selectCol = {entry->desc->color.r, entry->desc->color.g, entry->desc->color.b, entry->rgba.a};
        wii::gx::GXColor black = {0, 0, 0, 255};
        wii::gx::GXColor blue = {43, 164, 255, entry->rgba.a};
        s32 hdrPosX;
        s32 hdrPosY;
        wii::mtx::PSMTXScale(mtx, entry->scalarScale, entry->scalarScale, entry->scalarScale);
        // Add support for select menu GX overrides
        if (id > -1) {
            pausewin::PausewinEntry * pause = pausewin::pausewinGetEntry(entry->select->pausewinId);
            if (pause->alpha == 0 && CWSelectGetActiveEntry()->instantClose && pause->state == 21)
                return;
            // Replace header color (expand functionality later)
            if (GlobalCW->Select[id]->Colorize.headerColor.a != 0) {
                blue.r = GlobalCW->Select[id]->Colorize.headerColor.r;
                blue.g = GlobalCW->Select[id]->Colorize.headerColor.g;
                blue.b = GlobalCW->Select[id]->Colorize.headerColor.b;
                blue.a = GlobalCW->Select[id]->Colorize.headerColor.a;
                blue = CWColorOverrideRecalcAlpha(&blue, pause);
            }
            if (GlobalCW->Select[id]->Colorize.Colors != nullptr) // If CWSelectColorDefs are defined for the current active CWSelect, draw color
            {
                if (!GlobalCW->Select[id]->Colorize.animate) // If not set to animate, just takes from the first CWSelectColorDef
                {
                    wii::gx::GXColor top = CWColorOverrideRecalcAlpha(&GlobalCW->Select[id]->Colorize.Colors->colorTop, pause);
                    wii::gx::GXColor bottom = CWColorOverrideRecalcAlpha(&GlobalCW->Select[id]->Colorize.Colors->colorBottom, pause);
                    gxsub::gxsubInit_Colour();
                    gxsub::gxsubDrawQuadGradient(entry->pos.x, entry->pos.y, entry->scale.x, entry->scale.y, &top, &bottom);
                } else // Interpolate between the current & next color defs OR stay the same
                {
                    CWColorOverrideAnimateAction(entry->pos.x, entry->pos.y, entry->scale.x, entry->scale.y, pause, id);
                }
            } else // Draws a normal white box
                windowdrv::windowDispGX2_Waku_col(&mtx, 0, &selectCol, entry->pos.x, entry->pos.y, entry->scale.x, entry->scale.y, 0);
        } else // Draws a normal white box
            windowdrv::windowDispGX2_Waku_col(&mtx, 0, &selectCol, entry->pos.x, entry->pos.y, entry->scale.x, entry->scale.y, 0);
        gxsub::gxsubInit();
        gxsub::gxsubDrawLineSquare(entry->pos.x, entry->pos.y, (entry->scale.x * entry->scalarScale), (entry->scale.y * entry->scalarScale), 0x10, &black);
        switch (entry->desc->type) {
        case 1:
            hdrPosX = entry->pos.x + (entry->scale.x - 150) / 2;
            hdrPosY = entry->pos.y + 14;
            windowdrv::windowDispGX2_Waku_col(&mtx, 0, &blue, hdrPosX, hdrPosY, 150, 28, 0);
            gxsub::gxsubInit();
            gxsub::gxsubDrawLineSquare(hdrPosX, hdrPosY, (150 * entry->scalarScale), (28 * entry->scalarScale), 0x10, &black);
            break;
        case 2:
            hdrPosX = entry->pos.x + (entry->scale.x - 200) / 2;
            hdrPosY = entry->pos.y + 14;
            windowdrv::windowDispGX2_Waku_col(&mtx, 0, &blue, hdrPosX, hdrPosY, 200, 28, 0);
            gxsub::gxsubInit();
            gxsub::gxsubDrawLineSquare(hdrPosX, hdrPosY, (200 * entry->scalarScale), (28 * entry->scalarScale), 0x10, &black);
            break;
        }
        // CWDEBUG_OSREPORT_FMT("CustomWin::CWWinMgrDisp: WinmgrEntry scale fields are... x = %d, y = %d, scalar = %.5f\n", entry->scale.x, entry->scale.y, entry->scalarScale);
        // Override default disp funcs
        if (entry->desc->dispFunc != nullptr)
            (entry->desc->dispFunc)(entry);
        return;
    }

    void (*pausewinMsgBoxDisp)(pausewin::PausewinEntry * entry);
    static void CWPauseWinMsgBoxDisp_SetLambda() {
        pausewinMsgBoxDisp = patch::hookFunction(pausewin::pausewinMsgBoxDisp,
                                                 [](pausewin::PausewinEntry * entry) {
                                                     s32 id = GlobalCW->activeSelect;
                                                     if (id > -1) {
                                                         wii::gx::GXColor black = {0, 0, 0, 255};
                                                         wii::gx::GXColor white = {231, 227, 231, 255};
                                                         if (GlobalCW->Select[id]->Colorize.Colors != nullptr) {
                                                             if (!GlobalCW->Select[id]->Colorize.animate) {
                                                                 wii::gx::GXColor top = CWColorOverrideRecalcAlpha(&GlobalCW->Select[id]->Colorize.Colors->colorTop, entry);
                                                                 wii::gx::GXColor bottom = CWColorOverrideRecalcAlpha(&GlobalCW->Select[id]->Colorize.Colors->colorBottom, entry);
                                                                 gxsub::gxsubInit_Colour();
                                                                 gxsub::gxsubDrawQuadGradient(entry->pos.x, entry->pos.y, entry->width, entry->height, &top, &bottom);
                                                             } else {
                                                                 CWColorOverrideAnimateAction(entry->pos.x, entry->pos.y, entry->width, entry->height, entry, id);
                                                             }
                                                         } else {
                                                             gxsub::gxsubInit();
                                                             white = CWColorOverrideRecalcAlpha(&white, entry);
                                                             gxsub::gxsubDrawQuad(entry->pos.x, entry->pos.y, entry->width, entry->height, &white);
                                                         }
                                                         gxsub::gxsubInit();
                                                         gxsub::gxsubDrawLineSquare(entry->pos.x, entry->pos.y, entry->width, entry->height, 0x10, &black);
                                                     }
                                                     pausewinMsgBoxDisp(entry);
                                                 });
    }

    void CWSelectInfoActivatePage(CWSelect * Entry, winmgr::WinmgrSelect * sel, u8 newPage) {
        // temp?
        (void)sel;
        // Overwrite Select's Descs
        Entry->Info.currentPage = newPage;
        CWSelectPageDef * Page = &Entry->Info.Pages[newPage];
        msl::string::memset(Entry->Descs, 0, sizeof(CWSelectItemDesc) * CWSELECT_DESC_MAX);
        msl::string::memcpy(Entry->Descs, Page->Descs, sizeof(CWSelectItemDesc) * Page->num);
        // Copy over the item table and msgpatches for each item
        for (u8 i = 0; i < Page->num && i < CWSELECT_PAGE_DESC_MAX; i += 1) {
            Entry->itemTable[i] = Page->itemTable[i];
            if (msl::string::strcmp("", Entry->Descs[i].nameTxt) != 0) {
                msgpatch::msgpatchDelEntry(item_data::itemDataTable[Entry->itemTable[i]].nameMsg);
                msgpatch::msgpatchAddEntry(item_data::itemDataTable[Entry->itemTable[i]].nameMsg, Entry->Descs[i].nameTxt, 1);
            }
            if (msl::string::strcmp("", Entry->Descs[i].descTxt) != 0) {
                msgpatch::msgpatchDelEntry(item_data::itemDataTable[Entry->itemTable[i]].descMsg);
                msgpatch::msgpatchAddEntry(item_data::itemDataTable[Entry->itemTable[i]].descMsg, Entry->Descs[i].descTxt, 1);
            }
        }
        Entry->itemTable[Page->num] = -1;
        Entry->num = Page->num;
        // Reapply title/select msgpatches
        msgpatch::msgpatchDelEntry(GlobalCW->selectWinTitleMsgId);
        msgpatch::msgpatchDelEntry(GlobalCW->selectWinSelectMsgId);
        if (Page->windowTitle != nullptr)
            msgpatch::msgpatchAddEntry(GlobalCW->selectWinTitleMsgId, Page->windowTitle, true);
        if (Page->windowSelect != nullptr)
            msgpatch::msgpatchAddEntry(GlobalCW->selectWinSelectMsgId, Page->windowSelect, true);
        return;
    }

    void CWSelectInfoSetup(CWSelect * Entry) {
        u8 i, num, page;
        // Allocate Descs to memory for numPages
        for (i = 0; i < Entry->Info.numPages; i += 1) {
            Entry->Info.Pages[i].Descs = (CWSelectItemDesc *)memory::__memAlloc(memory::HEAP_MAIN, sizeof(CWSelectItemDesc) * Entry->num);
            msl::string::memset(Entry->Info.Pages[i].Descs, 0, sizeof(CWSelectItemDesc) * Entry->num);
        }
        // For each listing desc's page definition, copy that item's desc to its respective page descs
        for (i = 0; i < Entry->num; i += 1) {
            page = Entry->Descs[i].page;
            if (page < Entry->Info.numPages) {
                num = Entry->Info.Pages[page].num;
                msl::string::memcpy(&Entry->Info.Pages[page].Descs[num], &Entry->Descs[i], sizeof(CWSelectItemDesc));
                Entry->Info.Pages[page].itemTable[num] = Entry->itemTable[i];
                Entry->Info.Pages[page].itemTable[num + 1] = -1;
                Entry->Info.Pages[page].num += 1;
            }
        }
        return;
    }

    /*
        Rewrite of select_main, just in case we need special user-defined behavior
    */
    void CWSelect_Main(winmgr::WinmgrEntry * win) {
        winmgr::WinmgrSelect * select = win->select;
        u32 held = wpadmgr::wpadGetButtonsHeldRepeat(0);
        u32 pressed = wpadmgr::wpadGetButtonsPressed(0);
        s32 i, ogSelIdx, item;
        winmgr::WinmgrEntry * curEnt;
        pausewin::PausewinEntry * pausewin;
        CWSelect * Entry = CWSelectGetActiveEntry();
        switch (select->state) {
        case 0:
            for (i = 0; i < select->entryCount; i += 1) {
                curEnt = &winmgr::winmgr_wp->entries[select->entryIds[i]];
                if ((curEnt->flags & 1) != 0) {
                    if (!Entry->instantOpen) {
                        curEnt->seqState = 10;
                        curEnt->seqProgress = 0;
                    } else {
                        curEnt->seqState = 11;
                        curEnt->seqProgress = 999;
                        curEnt->flags |= 0x6;
                    }
                }
            }
            CWDEBUG_OSREPORT_FMT("CustomWin::CWSelect_Main: WinmgrSelect is located at %p\n", select);
            select->state += 1;
            break;
        case 1:
            ogSelIdx = select->selectedItemIdx;
            if ((winmgr::winmgr_wp->entries[select->entryIds[0]].flags & 4) != 0)
                return;
            if ((held & WPAD_BTN_LEFT) != 0) // H-DOWN
            {
                select->selectedItemIdx += 1;
                if (select->itemCount <= select->selectedItemIdx) {
                    select->selectedItemIdx = 0;
                    select->scrollIdx = 0;
                }
            } else if ((held & WPAD_BTN_RIGHT) != 0) // H-UP
            {
                select->selectedItemIdx -= 1;
                if (select->selectedItemIdx < 0) {
                    select->selectedItemIdx = select->itemCount - 1;
                    if (8 < select->itemCount)
                        select->scrollIdx = select->itemCount - 8;
                }
            }
            if ((pressed & WPAD_BTN_2) != 0) {
                if (Entry->BtnOverrides[BTN_2] != nullptr) {
                    bool proceedWithSelect = (Entry->BtnOverrides[BTN_2])(select);
                    if (!proceedWithSelect)
                        break;
                }
                if (select->itemCount == 0) // ??????????
                {
                    select->flag4 |= 0x2000;
                    if (!CWSelectGetActiveEntry()->Sfx.muteCloseSfx) {
                        if (CWSelectGetActiveEntry()->Sfx.closeSfx == nullptr)
                            spmario_snd::spsndSFXOn("SFX_SYS_SELECT_NG1");
                        else
                            spmario_snd::spsndSFXOn(CWSelectGetActiveEntry()->Sfx.closeSfx);
                    }
                } else if (!CWSelectGetActiveEntry()->Sfx.muteDecideSfx) {
                    if (CWSelectGetActiveEntry()->Sfx.decideSfx == nullptr)
                        spmario_snd::spsndSFXOn("SFX_SYS_MENU_DESIDE1");
                    else
                        spmario_snd::spsndSFXOn(CWSelectGetActiveEntry()->Sfx.decideSfx);
                }
                select->state += 1;
            } else if ((pressed & WPAD_BTN_1) != 0 && ((select->flag0 & 1) != 0)) {
                if (Entry->BtnOverrides[BTN_1] != nullptr) {
                    bool proceedWithCancel = (Entry->BtnOverrides[BTN_1])(select);
                    if (!proceedWithCancel)
                        break;
                }
                select->flag4 |= 0x2000;
                select->state += 1;
            } else if ((pressed & WPAD_BTN_PLUS) != 0 && Entry->BtnOverrides[BTN_PLUS] != nullptr) {
                (Entry->BtnOverrides[BTN_PLUS])(select);
            } else if ((pressed & WPAD_BTN_MINUS) != 0 && Entry->BtnOverrides[BTN_MINUS] != nullptr) {
                (Entry->BtnOverrides[BTN_MINUS])(select);
            } else if ((pressed & WPAD_BTN_B) != 0 && Entry->BtnOverrides[BTN_B] != nullptr) {
                (Entry->BtnOverrides[BTN_B])(select);
            } // A is left out for now as it'd be most appropriate to hijack the function responsible for scrolling text. CWSelect has yet to need that for any other purpose thus far.
            if (ogSelIdx != select->selectedItemIdx) {
                if ((ogSelIdx < select->selectedItemIdx) && (5 < (select->selectedItemIdx - select->scrollIdx)) && (select->scrollIdx < (select->itemCount - 8)))
                    select->scrollIdx += 1;
                else if ((ogSelIdx > select->selectedItemIdx) && (2 > (select->selectedItemIdx - select->scrollIdx)) && (select->scrollIdx > 0))
                    select->scrollIdx -= 1;
                pausewin = pausewin::pausewinGetEntry(select->pausewinId);
                item = Entry->Descs[select->selectedItemIdx].itemId;
                if (item > 0) // Is item
                {
                    if (item < 0x11A) // Not a card
                    {
                        pausewin::pausewinSetMessage(pausewin, item, 0);
                    } else
                        pausewin::pausewinSetMessage(pausewin, 0, "msg_window_help_card");
                } else // Is listing
                    pausewin::pausewinSetMessage(pausewin, Entry->itemTable[select->selectedItemIdx], 0);
                spmario_snd::spsndSFXOn("SFX_SYS_MENU_CURSOR_MOVE2");
            }
            select->scrollProgress += (f32)(((f32)select->scrollIdx * 25.0 - select->scrollProgress) / 6.0);
            select->pos.x += (f32)(((f32)win->pos.x - select->pos.x) / 6.0);
            select->pos.y += (f32)((-((f32)(select->selectedItemIdx - select->scrollIdx) * 25.0 - (f32)((win->pos.y - 54))) - select->pos.y) / 6.0);
            break;
        case 2:
            if ((select->flag4 & 0x2000) != 0 && !CWSelectGetActiveEntry()->Sfx.muteCloseSfx) {
                if (CWSelectGetActiveEntry()->Sfx.closeSfx == nullptr)
                    spmario_snd::spsndSFXOn("SFX_SYS_MENU_CLOSE1");
                else
                    spmario_snd::spsndSFXOn(CWSelectGetActiveEntry()->Sfx.closeSfx);
            }
            for (i = 0; i < select->entryCount; i += 1) {
                curEnt = &winmgr::winmgr_wp->entries[select->entryIds[i]];
                if ((curEnt->flags & 1) != 0) {
                    if (!Entry->instantClose) {
                        curEnt->seqState = 20;
                        curEnt->seqProgress = 0;
                    } else {
                        curEnt->seqState = 21;
                        curEnt->seqProgress = 999;
                        curEnt->flags &= ~0x6;
                        pausewin = pausewin::pausewinGetEntry(select->pausewinId);
                        pausewin->alpha = 0;
                        pausewin->state = 21; // STATE_SHRINK
                        pausewin->stateProgress = 999;
                        pausewin->flags |= 0x1;
                        // wii::os::OSReport("pausewin located @ %p\n", pausewin);
                        pausewin::_pausewinDelete(pausewin->id);
                        break;
                    }
                }
            }
            pausewin::pausewinDelete(select->pausewinId);
            select->state += 1;
            break;
        case 3:
            if ((winmgr::winmgr_wp->entries[select->entryIds[0]].flags & 4) != 0)
                return;
            select->flag4 |= 0x1000;
            select->state += 1;
            break;
        }
        return;
    }

    /*
        Resizes the main window based on item count
    */
    void CWSelectResizeWindow(winmgr::WinmgrEntry * win, CWSelect * Entry) {
        if (Entry->num < 8)
            win->scale.y = (s32)(CWSelectWindowDescs[0].scale.y - ((8 - Entry->num) * 25.0));
        return;
    }

    /*
        Rewrite of select_main specifically for info menus
    */
    void CWSelect_Main_Info(winmgr::WinmgrEntry * win) {
        winmgr::WinmgrSelect * select = win->select;
        u32 held = wpadmgr::wpadGetButtonsHeldRepeat(0);
        u32 pressed = wpadmgr::wpadGetButtonsPressed(0);
        s32 i, ogSelIdx;
        u8 ogPgIdx;
        winmgr::WinmgrEntry * curEnt;
        CWSelect * Entry = CWSelectGetActiveEntry();
        CWSelectPageDef * Page = &Entry->Info.Pages[Entry->Info.currentPage];
        s32 numItems = (s32)Page->num;
        switch (select->state) {
        case 0:
            for (i = 0; i < select->entryCount; i += 1) {
                curEnt = &winmgr::winmgr_wp->entries[select->entryIds[i]];
                if ((curEnt->flags & 1) != 0) {
                    curEnt->seqState = 10;
                    curEnt->seqProgress = 0;
                }
            }
            Entry->Info.currentPage = 0;
            CWSelectInfoSetup(Entry);
            CWSelectInfoActivatePage(Entry, select, 0);
            CWSelectSetDescriptionMsg(select, Entry);
            CWSelectResizeWindow(win, Entry);
            select->state += 1;
            break;
        case 1:
            ogSelIdx = select->selectedItemIdx;
            if ((winmgr::winmgr_wp->entries[select->entryIds[0]].flags & 4) != 0)
                return;
            if ((held & WPAD_BTN_LEFT) != 0) // H-DOWN
            {
                select->selectedItemIdx += 1;
                if (numItems <= select->selectedItemIdx) {
                    select->selectedItemIdx = 0;
                    select->scrollIdx = 0;
                }
            } else if ((held & WPAD_BTN_RIGHT) != 0) // H-UP
            {
                select->selectedItemIdx -= 1;
                if (select->selectedItemIdx < 0) {
                    select->selectedItemIdx = numItems - 1;
                    if (8 < numItems)
                        select->scrollIdx = numItems - 8;
                }
            } else if (Entry->type == CWSELECT_INFOGRAPHIC) {
                ogPgIdx = Entry->Info.currentPage;
                if ((held & WPAD_BTN_UP) != 0) // H-LEFT
                {
                    if (Entry->Info.currentPage < 1) {
                        Entry->Info.currentPage = Entry->Info.numPages - 1;
                    } else
                        Entry->Info.currentPage -= 1;
                } else if ((held & WPAD_BTN_DOWN) != 0) // H-RIGHT
                {
                    if (Entry->Info.currentPage == (Entry->Info.numPages - 1)) {
                        Entry->Info.currentPage = 0;
                    } else
                        Entry->Info.currentPage += 1;
                }
                if (ogPgIdx != Entry->Info.currentPage) {
                    CWSelectInfoActivatePage(Entry, select, Entry->Info.currentPage);
                    select->scrollIdx = 0;
                    select->scrollProgress = 0.0;
                    select->selectedItemIdx = 0;
                    CWSelectResizeWindow(win, Entry);
                    CWSelectSetDescriptionMsg(select, Entry);
                    spmario_snd::spsndSFXOn("SFX_SYS_MENU_CURSOR_MOVE2");
                }
            }
            if ((pressed & WPAD_BTN_2) != 0 && Entry->type != CWSELECT_INFOGRAPHIC) {
                if (numItems == 0) // ??????????
                {
                    select->flag4 |= 0x2000;
                    spmario_snd::spsndSFXOn("SFX_SYS_SELECT_NG1");
                } else
                    spmario_snd::spsndSFXOn("SFX_SYS_MENU_DESIDE1");
                select->state += 1;
            } else if ((pressed & WPAD_BTN_1) != 0 && ((select->flag0 & 1) != 0)) {
                select->flag4 |= 0x2000;
                select->state += 1;
            }
            if (ogSelIdx != select->selectedItemIdx) {
                if ((ogSelIdx < select->selectedItemIdx) && (5 < (select->selectedItemIdx - select->scrollIdx)) && (select->scrollIdx < (numItems - 8)))
                    select->scrollIdx += 1;
                else if ((ogSelIdx > select->selectedItemIdx) && (2 > (select->selectedItemIdx - select->scrollIdx)) && (select->scrollIdx > 0))
                    select->scrollIdx -= 1;
                CWSelectSetDescriptionMsg(select, Entry);
                spmario_snd::spsndSFXOn("SFX_SYS_MENU_CURSOR_MOVE2");
            }
            select->scrollProgress += (f32)(((f32)select->scrollIdx * 25.0 - select->scrollProgress) / 6.0);
            select->pos.x += (f32)(((f32)win->pos.x - select->pos.x) / 6.0);
            select->pos.y += (f32)((-((f32)(select->selectedItemIdx - select->scrollIdx) * 25.0 - (f32)((win->pos.y - 54))) - select->pos.y) / 6.0);
            break;
        case 2:
            if ((select->flag4 & 0x2000) != 0)
                spmario_snd::spsndSFXOn("SFX_SYS_MENU_CLOSE1");
            for (i = 0; i < select->entryCount; i += 1) {
                curEnt = &winmgr::winmgr_wp->entries[select->entryIds[i]];
                if ((curEnt->flags & 1) != 0) {
                    curEnt->seqState = 20;
                    curEnt->seqProgress = 0;
                }
            }
            pausewin::pausewinDelete(select->pausewinId);
            select->state += 1;
            break;
        case 3:
            if ((winmgr::winmgr_wp->entries[select->entryIds[0]].flags & 4) != 0)
                return;
            select->flag4 |= 0x1000;
            select->state += 1;
            break;
        }
        return;
    }

    /* --------------------------------------------------------------------------------------

                                            CWKeypad

     -------------------------------------------------------------------------------------- */

    /* --------------------------------------------------------------------------------------

                                            CWMsg

    -------------------------------------------------------------------------------------- */

    CWMsgGX * CWMsgGetActiveEntry() {
        if (GlobalCW->activeMsgGX == -1)
            return nullptr;
        return GlobalCW->MsgGX[GlobalCW->activeMsgGX];
    }

    s32 CWMsgKeyToId(const char * key) {
        for (u32 i = 0; i < CWMSG_ENTRY_MAX; i += 1) {
            if (msl::string::strcmp("", GlobalCW->MsgKeys[i].name) != 0) {
                if (msl::string::strcmp(GlobalCW->MsgKeys[i].name, key) == 0)
                    return i;
            }
        }
        CWDEBUG_OSREPORT_FMT("CustomWin::CWMsgKeyToId: Could not find msg entry with key name \'%s\'.\n", key);
        return -1;
    }

    CWMsgGX * CWMsgEntry(const char * key, s32 type, wii::tpl::TPLHeader * tpl, bool animate, bool setActive) {
        /*if (msl::string::strlen(key) > CWKEY_NAME_LENGTH) {
            CWDEBUG_OSREPORT_FMT("CustomWin::CWMsgEntry: Key \'%s\' is longer than CWKEY_NAME_LENGTH (%d). Failed to create select entry.\n", key, CWKEY_NAME_LENGTH);
            return nullptr;
        }*/
        for (u32 i = 0; i < CWMSG_ENTRY_MAX; i += 1) {
            if (GlobalCW->MsgGX[i] == nullptr) {
                GlobalCW->MsgGX[i] = (CWMsgGX *)memory::__memAlloc(memory::HEAP_MAIN, sizeof(CWMsgGX));
                msl::string::memset(GlobalCW->MsgGX[i], 0, sizeof(CWMsgGX));
                GlobalCW->MsgKeys[i].id = i;
                GlobalCW->MsgGX[i]->type = type;
                GlobalCW->MsgGX[i]->tpl = tpl;
                GlobalCW->MsgGX[i]->animate = animate;
                GlobalCW->MsgKeys[i].name = CWReallocString(key);
                if (setActive) {
                    GlobalCW->activeMsgGX = i;
                    CWDEBUG_OSREPORT_FMT("CustomWin::CWMsgEntry: New msg entry with key \'%s\' (GlobalCW->MsgGX[%d]) created. Set as active msg entry.\n", key, i);
                } else
                    CWDEBUG_OSREPORT_FMT("CustomWin::CWMsgEntry: New msg entry with key \'%s\' (GlobalCW->MsgGX[%d]) created.\n", key, i);
                return GlobalCW->MsgGX[i];
            }
        }
        CWDEBUG_OSREPORT_FMT("CustomWin::CWMsgEntry: Msg entries maxed out at CWMSG_ENTRY_MAX (%d), cannot create new entry.\n", CWMSG_ENTRY_MAX);
        return nullptr;
    }

    void CWMsgGX_Tile_SetTplIndices(CWMsgGX * Entry, u32 bg, u32 tlCol, u32 blCol, u32 trCol, u32 brCol, u32 tlClear, u32 blClear, u32 trClear, u32 brClear) {
        Entry->Type.Tile.texBG = bg;
        Entry->Type.Tile.texTL_Color = tlCol;
        Entry->Type.Tile.texBL_Color = blCol;
        Entry->Type.Tile.texTR_Color = trCol;
        Entry->Type.Tile.texBR_Color = brCol;
        Entry->Type.Tile.texTL_Clear = tlClear;
        Entry->Type.Tile.texBL_Clear = blClear;
        Entry->Type.Tile.texTR_Clear = trClear;
        Entry->Type.Tile.texBR_Clear = brClear;
        return;
    }

    /*
        ** User Function **
        Activates a CWMsg entry, then prints a msg
    */
    s32 EvtCWMsgPrint(evtmgr::EvtEntry * evtEntry, bool firstCall) {
        (void)firstCall;
        evtmgr::EvtVar * args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char * key = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 flags = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        const char * msg = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[2]);
        void * mainFunc = (void *)evtmgr_cmd::evtGetValue(evtEntry, args[3]);
        char * speaker = (char *)evtmgr_cmd::evtGetValue(evtEntry, args[4]);
        s32 id = CWMsgKeyToId(key);
        if (id == -1) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWMsgPrint: Entry with key \'%s\' not found; aborting process.\n", key);
            return 2;
        }
        GlobalCW->activeMsgGX = id;
        s32 ret = evt_msg::_evt_msg_print(evtEntry, firstCall, flags, msg, mainFunc, speaker);
        if (ret == 2) {
            CWDEBUG_OSREPORT_FMT("CustomWin::EvtCWMsgPrint: Printed CWMsg with key \'%s\'.\n", key);
            GlobalCW->activeMsgGX = -1;
        }
        return ret;
    }

    bool sptexCWMode = false;

    void CWMsgGX_SptextureGetNew(u32 id, wii::gx::GXTexObj * dest) {
        CWMsgGX * Entry = CWMsgGetActiveEntry();
        if (id == 0x7777) {
            if (Entry == nullptr)
                id = 6;
            else
                id = Entry->Type.Tile.texBG;
        }
        if (Entry != nullptr && sptexCWMode) {
            wii::tpl::TPLGetGXTexObjFromPalette(Entry->tpl, dest, id);
            return;
        }
        if (sptexture::sptexture_wp->loaded) {
            wii::tpl::TPLGetGXTexObjFromPalette((wii::tpl::TPLHeader *)sptexture::sptexture_wp->tpl->sp->data, dest, id);
            return;
        }
        wii::gx::GXInitTexObj(dest, &sptexture::sptexture_image, 1, 1, 0, 0, 0, 0);
    }

    // System text-style "tiled texture" textbox
    void CWMsgGX_Tile_Main(s32 type, u8 alpha, f32 x, f32 y, f32 p3, f32 p4) {
        u32 tlIdx_Col = 62, blIdx_Col = 63, trIdx_Col = 64, brIdx_Col = 65, tlIdx_Clear = 66, blIdx_Clear = 67, trIdx_Clear = 68, brIdx_Clear = 69;
        f32 animOffset = 0.0f;
        wii::mtx::Mtx34 pos;
        camdrv::CamEntry * cam = camdrv::camGetCurPtr();
        CWMsgGX * Entry = CWMsgGetActiveEntry();

        if (windowdrv::windowdrv_frm_ctr != spmario::gp->frameCounter)
            windowdrv::windowdrv_anim_time += 0.005f;
        windowdrv::windowdrv_frm_ctr = spmario::gp->frameCounter;
        if (windowdrv::windowdrv_anim_time > 10.0f)
            windowdrv::windowdrv_anim_time -= 10.0f;
        windowdrv::func_80038b08();
        wii::gx::GXColor col = {255, 255, 255, alpha};
        wii::gx::GXSetTevColor(1, &col);
        wii::gx::GXSetNumTevStages(4);
        wii::gx::GXSetTevOrder(0, 0, 0, 255);
        wii::gx::GXSetTevColorOp(0, 0, 0, 0, 1, 0);
        wii::gx::GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
        wii::gx::GXSetTevColorIn(0, 15, 15, 15, 8);
        wii::gx::GXSetTevAlphaIn(0, 7, 7, 7, 4);
        wii::gx::GXSetTevSwapMode(0, 0, 0);
        wii::gx::GXSetTevOrder(1, 1, 1, 255);
        wii::gx::GXSetTevColorOp(1, 0, 0, 0, 1, 0);
        wii::gx::GXSetTevAlphaOp(1, 0, 0, 0, 1, 0);
        wii::gx::GXSetTevColorIn(1, 15, 15, 15, 0);
        wii::gx::GXSetTevAlphaIn(1, 7, 0, 4, 7);
        wii::gx::GXSetTevSwapMode(1, 0, 0);
        wii::gx::GXSetTevOrder(2, 1, 2, 255);
        wii::gx::GXSetTevColorOp(2, 0, 0, 0, 1, 0);
        wii::gx::GXSetTevAlphaOp(2, 0, 0, 0, 1, 0);
        wii::gx::GXSetTevColorIn(2, 0, 8, 9, 15);
        wii::gx::GXSetTevAlphaIn(2, 7, 7, 7, 0);
        wii::gx::GXSetTevSwapMode(2, 0, 0);
        wii::gx::GXSetTevOrder(3, 255, 255, 255);
        wii::gx::GXSetTevColorOp(3, 0, 0, 0, 1, 0);
        wii::gx::GXSetTevAlphaOp(3, 0, 0, 0, 1, 0);
        wii::gx::GXSetTevColorIn(3, 15, 0, 2, 15);
        wii::gx::GXSetTevAlphaIn(3, 7, 0, 1, 7);
        wii::gx::GXSetTevSwapMode(3, 0, 0);
        wii::mtx::PSMTXTrans(pos, x, y, 0.0f);
        wii::mtx::PSMTXConcat(cam->viewMtx, pos, pos);
        wii::gx::GXLoadPosMtxImm(pos, 0);
        wii::gx::GXSetCurrentMtx(0);
        if (Entry != nullptr) {
            if (Entry->animate)
                animOffset = windowdrv::windowdrv_anim_time;
            tlIdx_Col = Entry->Type.Tile.texTL_Color;
            blIdx_Col = Entry->Type.Tile.texBL_Color;
            trIdx_Col = Entry->Type.Tile.texTR_Color;
            brIdx_Col = Entry->Type.Tile.texBR_Color;
            tlIdx_Clear = Entry->Type.Tile.texTL_Clear;
            blIdx_Clear = Entry->Type.Tile.texBL_Clear;
            trIdx_Clear = Entry->Type.Tile.texTR_Clear;
            brIdx_Clear = Entry->Type.Tile.texBR_Clear;
        } else if (type != 13) {
            animOffset = windowdrv::windowdrv_anim_time;
            tlIdx_Col = 42;
            blIdx_Col = 43;
            trIdx_Col = 44;
            brIdx_Col = 45;
            tlIdx_Clear = 46;
            blIdx_Clear = 47;
            trIdx_Clear = 48;
            brIdx_Clear = 49;
        }
        sptexCWMode = true;
        windowdrv::windowDispGX_System_LoadTex(0, 0, (p3 - 32.0f), (p4 - 32.0f), animOffset, tlIdx_Col, tlIdx_Clear);
        windowdrv::windowDispGX_System_LoadTex(0, -(p4 - 32.0f), (p3 - 32.0f), 32.0f, animOffset, blIdx_Col, blIdx_Clear);
        windowdrv::windowDispGX_System_LoadTex((p3 - 32.0f), 0, 32.0f, (p4 - 32.0f), animOffset, trIdx_Col, trIdx_Clear);
        windowdrv::windowDispGX_System_LoadTex((p3 - 32.0f), -(p4 - 32.0f), 32.0f, 32.0f, animOffset, brIdx_Col, brIdx_Clear);
        sptexCWMode = false;
        return;
    }

    void CustomWinMain() {
        CWInit();
        // Select
        patch::hookFunction(winmgr::winMgrDisp, CWWinMgrDisp);
        CWPauseWinMsgBoxDisp_SetLambda();
        // Msg
        patch::hookFunction(windowdrv::windowDispGX_System, CWMsgGX_Tile_Main);
        patch::hookFunction(sptexture::sptextureGet, CWMsgGX_SptextureGetNew);
        writeWord(windowdrv::windowDispGX_System_LoadTex, 0x6C, LI(3, 0x7777));
    }
}
