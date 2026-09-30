#pragma once
#include <common.h>
#include <spm/evtmgr.h>
#include <spm/evtmgr_cmd.h>
#include <spm/winmgr.h>
#include <wii/gx.h>
#include <wii/os.h>
#include <wii/tpl.h>

namespace mod::customwin {
    using namespace spm;

// Set to true to enable CustomWin debug osreports
#define CUSTOMWIN_DEBUG false

#define CWDEBUG_OSREPORT(message) \
    if (CUSTOMWIN_DEBUG)          \
        wii::os::OSReport(message);

#define CWDEBUG_OSREPORT_FMT(message, ...) \
    if (CUSTOMWIN_DEBUG)                   \
        wii::os::OSReport(message, __VA_ARGS__);

#define CWSELECT_ENTRY_MAX 4
#define CWSELECT_DESC_MAX 300
#define CWSELECT_PAGE_MAX 16
#define CWSELECT_PAGE_DESC_MAX 50

#define CWMSG_ENTRY_MAX 10

    typedef bool(CWSelectCallback)(winmgr::WinmgrSelect * select);

    enum CWSelectType {
        CWSELECT_DEFAULT = 0,
        CWSELECT_SHOP = 1,
        CWSELECT_INFOGRAPHIC = 2
    };

    struct CWKey {
        char * name;
        u32 id;
    };

    struct CWSelectItemDesc {
        s32 iconId; // only read if Listing, -1 to draw none
        s32 itemId; // only read if Item, -1 if not an item
        s32 cost;   // only read if select is Shop
        u8 page;    // only read if select is Info; index starts at 0
        wii::gx::GXColor nameColor;
        bool deallocStrings;
        char * nameTxt;
        char * descTxt;
    };

    struct CWSelectColorDef {
        wii::gx::GXColor colorTop;
        wii::gx::GXColor colorBottom;
        s32 transitionTime;
        s32 stayTime;
    };

    struct CWSelectColorization {
        bool animate;
        u32 timer;
        u8 subTimer;
        u32 numDefs;
        CWSelectColorDef * Colors;
        wii::gx::GXColor headerColor;
        wii::gx::GXColor selectTxtColor;
        wii::gx::GXColor headerTxtColor;
    };

    struct CWSelectPageDef {
        CWSelectItemDesc * Descs;
        u8 num;
        s32 itemTable[CWSELECT_PAGE_DESC_MAX];
        char * windowTitle;
        char * windowSelect;
    };

    struct CWSelectInfographic {
        u8 numPages; // max of 16 per select
        u8 currentPage;
        CWSelectPageDef * Pages;
    };

    struct CWSelectIconDef {
        s16 id; // wicon.tpl index. todo: expand functionality to support animPoseIds
        f32 scale;
    };

    struct CWSelectSfx {
        bool muteOpenSfx;
        bool muteCloseSfx;
        bool muteDecideSfx;
        char * openSfx;
        char * closeSfx;
        char * decideSfx;
    };

    struct CWSelectCB {
        CWSelectCallback * Btn1Behavior;
        CWSelectCallback * Btn2Behavior;
        CWSelectCallback * BtnPlusBehavior;
        CWSelectCallback * BtnMinusBehavior;
        CWSelectCallback * BtnABehavior;
        CWSelectCallback * BtnBBehavior;
    };

    enum CWSelectCBId : s32 {
        BTN_1,
        BTN_2,
        BTN_PLUS,
        BTN_MINUS,
        BTN_A,
        BTN_B,
        BTN_MAX
    };

    struct CWSelect {
        CWSelectType type;
        u32 num;
        u32 resetNum; // copied from num @ creation time, necessary for when num changes at reset time
        CWSelectItemDesc * Descs;
        CWSelectCallback * BtnOverrides[BTN_MAX]; // A currently not supported
        CWSelectColorization Colorize;
        CWSelectInfographic Info;
        CWSelectIconDef ShopIcon;    // icon at top-right of menu
        CWSelectIconDef PointerIcon; // hand icon that points at selected item
        CWSelectSfx Sfx;
        bool hideDescWin;
        bool instantOpen;
        bool instantClose;
        char * windowTitle;  // blue header, e.g. Items, Catch Cards
        char * windowSelect; // box to the left, e.g. Select an Item, Select a Card, What do you want to sell?
        s32 itemTable[CWSELECT_DESC_MAX];
        s32 resetItemTable[CWSELECT_DESC_MAX]; // copied from itemTable @ creation time, never changes
    };

    struct CWMsgGX_Tile {
        s32 texBG;
        s32 texTL_Color;
        s32 texTR_Color;
        s32 texBL_Color;
        s32 texBR_Color;
        s32 texTL_Clear;
        s32 texTR_Clear;
        s32 texBL_Clear;
        s32 texBR_Clear;
    };

    struct CWMsgGX {
        wii::tpl::TPLHeader * tpl;
        bool animate;
        s32 type;
        union {
            CWMsgGX_Tile Tile;
        } Type;
    };

    struct CustomWinWork {
        // Select
        CWSelect * Select[CWSELECT_ENTRY_MAX];
        CWKey SelectKeys[CWSELECT_ENTRY_MAX];
        s32 activeSelect;
        const char * selectWinTitleMsgId;
        const char * selectWinSelectMsgId;
        // Msg
        CWMsgGX * MsgGX[CWMSG_ENTRY_MAX];
        CWKey MsgKeys[CWMSG_ENTRY_MAX];
        s32 activeMsgGX;
    };

    CWSelect * CWSelectGetActiveEntry();

    EVT_DECLARE_USER_FUNC(EvtCWSelectEntry, 6)
    EVT_DECLARE_USER_FUNC(EvtCWSelectAddListing, 7)
    EVT_DECLARE_USER_FUNC(EvtCWSelectAddItem, 4)
    EVT_DECLARE_USER_FUNC(EvtCWSelectRemoveListing, 2)
    EVT_DECLARE_USER_FUNC(EvtCWSelectAddInfoPage, 3)
    EVT_DECLARE_USER_FUNC(EvtCWSelectOverrideBtnBehavior, 3)
    EVT_DECLARE_USER_FUNC(EvtCWSelectHideDescWindow, 1)
    EVT_DECLARE_USER_FUNC(EvtCWSelectReset, 0)
    EVT_DECLARE_USER_FUNC(EvtCWSelectDelete, 1)
    EVT_DECLARE_USER_FUNC(EvtCWSelectSetBGColor, 3)
    EVT_DECLARE_USER_FUNC(EvtCWSelectSetHeaderColor, 2)
    EVT_DECLARE_USER_FUNC(EvtCWSelectSetSelectTextColor, 2)
    EVT_DECLARE_USER_FUNC(EvtCWSelectSetHeaderTextColor, 2)
    EVT_DECLARE_USER_FUNC(EvtCWSelectMenuStart, 3)
    EVT_DECLARE_USER_FUNC(EvtCWSelectGetSelectionName, 2)
    EVT_DECLARE_USER_FUNC(EvtCWSelectGetSelectionCost, 2)
    EVT_DECLARE_USER_FUNC(EvtCWSelectGetSelectionItemId, 2)
    EVT_DECLARE_USER_FUNC(EvtCWSelectSetShopIcon, 3)
    EVT_DECLARE_USER_FUNC(EvtCWSelectSetPointerIcon, 3)
    EVT_DECLARE_USER_FUNC(EvtCWSelectSetInstantOpenClose, 3)
    EVT_DECLARE_USER_FUNC(EvtCWSelectModifySfx, 4)

    CWMsgGX * CWMsgEntry(const char * key, s32 type, wii::tpl::TPLHeader * tpl, bool animate, bool setActive);
    void CWMsgGX_Tile_SetTplIndices(CWMsgGX * Entry, u32 bg, u32 tlCol, u32 blCol, u32 trCol, u32 brCol, u32 tlClear, u32 blClear, u32 trClear, u32 brClear);

    EVT_DECLARE_USER_FUNC(EvtCWMsgPrint, 5)

    extern CustomWinWork * GlobalCW;
    s32 CWSelectKeyToId(const char * key);
    void CWSelect_Main(winmgr::WinmgrEntry * win);
    void CWSelect_Main_Info(winmgr::WinmgrEntry * win);
    void CWSelect_Disp(winmgr::WinmgrEntry * win);
    void CWSelect_Disp2(winmgr::WinmgrEntry * win);
    void CWDebug_GetMainHeapFreeSpace(bool alloc);

    void CustomWinMain();
}
