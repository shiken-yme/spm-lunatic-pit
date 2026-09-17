#include "patch.h"
#include "ymetools.h"

#include <customwin.h>
#include <msgpatch.h>
#include <globalop.h>

#include <common.h>
#include <spm/animdrv.h>
#include <spm/mario.h>
#include <spm/effdrv.h>
#include <spm/eff_sub.h>
#include <spm/eff/eff_spm_spindash.h>
#include <spm/eff/eff_3d_switch.h>
#include <spm/mobjdrv.h>
#include <spm/npcdrv.h>
#include <spm/evt_mobj.h>
#include <spm/eff/eff_ddtas_block.h>
#include <spm/evtmgr.h>
#include <spm/evtmgr_cmd.h>
#include <spm/evt_cam.h>
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
#include <spm/evt_msg.h>
#include <spm/evt_mobj.h>
#include <spm/evt_npc.h>
#include <spm/evt_offscreen.h>
#include <spm/evt_paper.h>
#include <spm/evt_pouch.h>
#include <spm/evt_shop.h>
#include <spm/evt_snd.h>
#include <spm/evt_seq.h>
#include <spm/evt_sub.h>
#include <spm/framedrv.h>
#include <spm/gxsub.h>
#include <spm/dispdrv.h>
#include <spm/hitdrv.h>
#include <spm/fontmgr.h>
#include <spm/filemgr.h>
#include <spm/spmario.h>
#include <spm/spmario_snd.h>
#include <spm/map_data.h>
#include <spm/mapdrv.h>
#include <spm/hitdrv.h>
#include <spm/itemdrv.h>
#include <spm/item_data.h>
#include <spm/winmgr.h>
#include <spm/pausewin.h>
#include <spm/msgdrv.h>
#include <spm/mario_motion.h>
#include <spm/mario_pouch.h>
#include <spm/mot_fairy_mario.h>
#include <spm/mario_fairy.h>
#include <spm/mot_walk.h>
#include <spm/seq_mapchange.h>
#include <spm/system.h>
#include <spm/seqdef.h>
#include <spm/seqdrv.h>
#include <spm/mario_sbr.h>
#include <spm/wpadmgr.h>
#include <wii/kpad.h>
#include <wii/wpad.h>
#include <wii/tpl.h>
#include <wii/os.h>
#include <msl/string.h>
#include <spm/swdrv.h>
#include <msl/math.h>
#include <msl/stdio.h>

namespace mod::yme
{
    /*
        Proprietary tool for debugging, reverse engineering, and feature testing
        You can use this tool if you want by entering Debug Mode (Z + B on title screen, then Z + B ingame) but the controls aren't intuitive
        I will not offer support for these tools, as they are not necessarily meant for public use... but maybe one day
    */

    using namespace spm;

    // Init variables for ObjDisp
    char hitObjName[64] = "Null";
    char mapObjName[64] = "Null";
    s32 curMapObj = 0;
    u8 mapObjStatus = 0;
    u8 hitObjStatus = 0;
    s32 plusHeldTimer = 0;
    s32 minusHeldTimer = 0;
    bool mapGrpMode = false;
    wii::gx::GXColor highlight = {255, 0, 0, 255};
    wii::gx::GXColor highlightInactive = {75, 150, 255, 255};
    wii::gx::GXColor clearHighlight = {255, 255, 255, 255};
    mapdrv::MapObj *mapObj = nullptr;
    bool objIsInactive = false;
    bool preserveObjStatus = false;
    const char *objStatusAddenda[] = {"", "(Inactive)", "(Active)", ""};

    // Init variables for MapAnimDisp
    char mapAnimName[64] = "Null";
    s32 curMapAnim = 0;
    mapdrv::MapAnimObj *mapAnim = nullptr;
    s32 mapAnimDispMode = 0;
    const char *animModeAddenda[] = {"Play Once", "Loop", "Finish Instantly", "3", "4", "5", "6", "7", "8", "9", "Play Reversed"};

    // Some init variables for MOBJ Editor
    //   s32 zHeldTimer = 0;
    u32 keypadFlagStorage = 0;
    const char *keypadFlagKeys[] =
        {
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "a",
            "b",
            "c",
            "d",
            "e",
            "f",
            "0",
            "0",
            "0",
            "0",
            "password_02"};
    const char *keypadCoordKeys[] =
        {
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "0",
            "-",
            "password_02"};

    enum YmeToolConfigs
    {
        /* 0x00 */ YMETOOLS_NULL,
        /* 0x01 */ YMETOOLS_OBJ_DISP,
        /* 0x02 */ YMETOOLS_MAPANIM_DISP,
        /* 0x03 */ YMETOOLS_MOBJ
    };

    s32 ActiveYmeTool = 0;

    // s32 curMap_n = 0;

    static seqdef::SeqFunc *seq_gameMainReal;

    /*
        This gxsubDrawLine pattern is pretty much copied from npcHitboxDisp.
        I'll try to make something a little better one day.
    */
    void hitboxDisplay(s32 camId, void *hitEnt)
    {
        hitdrv::HitObj *hitEntry = (hitdrv::HitObj *)hitEnt;
        (void)camId;
        wii::mtx::Vec3 start, end, a, b;
        wii::gx::GXColor color = highlight;
        if ((hitEntry->flags & 0x1) != 0)
            color = highlightInactive;
        f32 width = (hitEntry->joint->bboxMax.x - hitEntry->joint->bboxMin.x) * 10.0f;
        f32 height = (hitEntry->joint->bboxMax.y - hitEntry->joint->bboxMin.y) * 10.0f;
        f32 length = (hitEntry->joint->bboxMax.z - hitEntry->joint->bboxMin.z) * 10.0f;
        hitdrv::hitObjGetPos(hitdrv::hitGetName(hitEntry), &start);
        start = {start.x - (width / 2.0f), start.y - (height / 2.0f), start.z - (length / 2.0f)};
        end.x = start.x + width;
        end.y = start.y + height;
        end.z = start.z + length;
        gxsub::gxsubInit_Cam(camdrv::camGetPtr(camdrv::CAM_ID_3D));
        a = {start.x, end.y, end.z};
        b = {end.x, end.y, end.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {start.x, start.y, end.z};
        b = {start.x, end.y, end.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {end.x, start.y, end.z};
        b = {start.x, start.y, end.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {end.x, end.y, end.z};
        b = {end.x, start.y, end.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {start.x, end.y, start.z};
        b = {end.x, end.y, start.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {start.x, start.y, start.z};
        b = {start.x, end.y, start.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {end.x, start.y, start.z};
        b = {start.x, start.y, start.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {end.x, end.y, start.z};
        b = {end.x, start.y, start.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {end.x, end.y, start.z};
        b = {end.x, start.y, start.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {end.x, end.y, start.z};
        b = {end.x, end.y, end.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {start.x, end.y, start.z};
        b = {start.x, end.y, end.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {end.x, start.y, start.z};
        b = {end.x, start.y, end.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        a = {start.x, start.y, start.z};
        b = {start.x, start.y, end.z};
        gxsub::gxsubDrawLine(&a, &b, 10, &color);
        return;
    }

    void hitobj9HitText()
    {
        if (ActiveYmeTool == YMETOOLS_OBJ_DISP)
        {
            wii::gx::GXColor funnyColor = {255, 255, 255, 255};
            f32 scale = 0.64f;
            char buffer[100];
            const char *hitObjStatusAddendum = objStatusAddenda[hitObjStatus];
            msl::stdio::sprintf(buffer, "(Z1) HitObj Name: %s %s", hitObjName, hitObjStatusAddendum);
            const char *msg = buffer;
            fontmgr::FontDrawStart();
            fontmgr::FontDrawEdge();
            fontmgr::FontDrawColor(&funnyColor);
            fontmgr::FontDrawScale(scale);
            fontmgr::FontDrawNoiseOff();
            fontmgr::FontDrawRainbowColorOff();
            fontmgr::FontDrawString(-360, -80.0f, msg);
            if (msl::string::strcmp(hitObjName, "Null") != 0)
            {
                wii::mtx::Vec3 hitPos;
                hitdrv::hitObjGetPos(hitObjName, &hitPos);
                dispdrv::dispEntry(camdrv::CAM_ID_3D, 11, dispdrv::dispCalcZ(&hitPos), hitboxDisplay, (void *)hitdrv::hitNameToPtr(hitObjName));
            }
            /*
                Debug
            */
            /*hitdrv::HitObj *OneF4 = mario::marioGetPtr()->unknown_0x1f4;
            if (OneF4 != nullptr)
            {
                char buf2[64];
                msl::stdio::sprintf(buf2, "HitObj 0x1F4: %s", hitdrv::hitGetName(OneF4));
                const char *msg2 = buf2;
                fontmgr::FontDrawStart();
                fontmgr::FontDrawEdge();
                fontmgr::FontDrawColor(&funnyColor);
                fontmgr::FontDrawScale(scale);
                fontmgr::FontDrawNoiseOff();
                fontmgr::FontDrawRainbowColorOff();
                fontmgr::FontDrawString(-360, 0.0f, msg2);
            }
            hitdrv::HitObj *OneF8 = mario::marioGetPtr()->unknown_0x1f8;
            if (OneF8 != nullptr)
            {
                char buf3[64];
                msl::stdio::sprintf(buf3, "HitObj 0x1F8: %s", hitdrv::hitGetName(OneF8));
                const char *msg3 = buf3;
                fontmgr::FontDrawStart();
                fontmgr::FontDrawEdge();
                fontmgr::FontDrawColor(&funnyColor);
                fontmgr::FontDrawScale(scale);
                fontmgr::FontDrawNoiseOff();
                fontmgr::FontDrawRainbowColorOff();
                fontmgr::FontDrawString(-360, -20.0f, msg3);
            }
            */
        }
    }

    void mapObjDispText()
    {
        if (ActiveYmeTool == YMETOOLS_OBJ_DISP)
        {
            wii::gx::GXColor funnyColor = {255, 255, 255, 255};
            f32 scale = 0.64f;
            char buffer[100];
            if (!preserveObjStatus)
            {
                if (!objIsInactive)
                    mapObjStatus = 2;
                else
                    mapObjStatus = 1;
            }
            const char *mapObjStatusAddendum = objStatusAddenda[mapObjStatus];
            if (!mapGrpMode)
            {
                msl::stdio::sprintf(buffer, "(Z2) MapObj Name: %s %s", mapObjName, mapObjStatusAddendum);
            }
            else
                msl::stdio::sprintf(buffer, "(Z2) MapGrp Name: %s %s", mapObjName, mapObjStatusAddendum);
            const char *msg = buffer;
            fontmgr::FontDrawStart();
            fontmgr::FontDrawEdge();
            fontmgr::FontDrawColor(&funnyColor);
            fontmgr::FontDrawScale(scale);
            fontmgr::FontDrawNoiseOff();
            fontmgr::FontDrawRainbowColorOff();
            fontmgr::FontDrawString(-360, -100.0f, msg);
        }
    }

    void mapAnimDispText()
    {
        if (ActiveYmeTool == YMETOOLS_MAPANIM_DISP)
        {
            wii::gx::GXColor funnyColor = {255, 255, 255, 255};
            f32 scale = 0.64f;
            char buffer[100];
            const char *mapAnimModeAddendum = animModeAddenda[mapAnimDispMode];
            msl::stdio::sprintf(buffer, "MapFileAnim Name: %s  (dispMode: %s)", mapAnimName, mapAnimModeAddendum);
            const char *msg = buffer;
            fontmgr::FontDrawStart();
            fontmgr::FontDrawEdge();
            fontmgr::FontDrawColor(&funnyColor);
            fontmgr::FontDrawScale(scale);
            fontmgr::FontDrawNoiseOff();
            fontmgr::FontDrawRainbowColorOff();
            fontmgr::FontDrawString(-360, -100.0f, msg);
        }
    }

    void textDisplay(seqdrv::SeqWork *wp)
    {
        hitobj9HitText();
        mapObjDispText();
        mapAnimDispText();
        seq_gameMainReal(wp);
    }

    static void postTextDisplay()
    {
        seq_gameMainReal = seqdef::seq_data[seqdrv::SEQ_GAME].main;
        seqdef::seq_data[seqdrv::SEQ_GAME].main = &textDisplay;
    }

    void ymetoolsMapSetColor(mapdrv::MapObj *obj, u8 mode)
    {
        const char *name = obj->joint->name;
        if (mode == 0) // Highlight
        {
            if (objIsInactive)
            {
                if (mapGrpMode)
                {
                    mapdrv::mapGrpSetColor(name, &highlightInactive);
                }
                else
                    mapdrv::mapObjSetColor(name, &highlightInactive);
            }
            else
            {
                if (mapGrpMode)
                {
                    mapdrv::mapGrpSetColor(name, &highlight);
                }
                else
                    mapdrv::mapObjSetColor(name, &highlight);
            }
        }
        else if (mode == 1) // Clear highlight
        {
            if (mapGrpMode)
            {
                mapdrv::mapGrpSetColor(name, &clearHighlight);
            }
            else
                mapdrv::mapObjSetColor(name, &clearHighlight);
        }
        return;
    }

    void ymetoolsMapFlagOnOff(const char *name, u8 mode)
    {
        if (mode == 0) // FlagOn; HIDES the object
        {
            if (mapGrpMode)
            {
                mapdrv::mapGrpFlagOn(0, name, 1);
            }
            else
            {
                mapdrv::mapObjFlagOn(0, name, 1);
            }
        }
        else // FlagOff; SHOWS the object
        {
            if (mapGrpMode)
            {
                mapdrv::mapGrpFlagOff(0, name, 1);
            }
            else
            {
                mapdrv::mapObjFlagOff(0, name, 1);
            }
        }
        return;
    }

    s32 evt_ymetools_handle_config_select(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        u8 select = evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        switch (select)
        {
        case YMETOOLS_MAPANIM_DISP:
            curMapAnim = 0;
            break;
        case YMETOOLS_OBJ_DISP:
            curMapObj = 0;
            break;
        case YMETOOLS_NULL:
            if (mapObj != nullptr)
            {
                ymetoolsMapSetColor(mapObj, 1);
                if (objIsInactive)
                {
                    ymetoolsMapFlagOnOff(mapObj->joint->name, 0);
                    objIsInactive = false;
                }
            }
        }
        ActiveYmeTool = select;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_ymetools_handle_config_select, 1)

    const char ymetoolsConfigText[] =
        "<housou>\n"
        "Select a configuration.\n"
        "<o>";

    const char ymetoolsConfigOptions[] =
        "<select 0 -1 300 45>\n"
        "None\n"
        "Object Display\n"
        "Map Anim Viewer\n"
        "MOBJ Manip";

    EVT_BEGIN(ymetoolsConfigMenu)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_npc::evt_npc_freeze_all)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(ymetoolsConfigText), 0, 0)
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(ymetoolsConfigOptions))
    USER_FUNC(evt_msg::evt_msg_continue)
    USER_FUNC(evt_ymetools_handle_config_select, LW(0))
    USER_FUNC(evt_mario::evt_mario_key_on)
    USER_FUNC(evt_npc::evt_npc_unfreeze_all)
    SET(GSWF(1700), 0)
    RETURN()
    EVT_END()

    EVT_BEGIN(delayClearHighlight)
    WAIT_FRM(1)
    USER_FUNC(evt_map::evt_mapobj_color, LW(1), LW(0), 255, 255, 255, 255)
    RETURN()
    EVT_END()

    const char ymetoolsMobjMenu_Main_Text[] =
        "<housou>\n"
        "<wave>Hiiii, onii-chan! x3</wave><wait 500> Do you want\n"
        "to create a new object or\n"
        "modify one on-field?\n"
        "<o>";

    const char ymetoolsMobjMenu_Main_Select[] =
        "<select 0 -1 300 45>\n"
        "New Object\n"
        "Existing Object";

    const char ymetoolsMobjMenu_ExistingMobjs[] =
        "Model name: %s\n"
        "Pos: (%d, %d, %d)";

    const char ymetoolsMobjMenu_ExObj_NoExObj[] =
        "<housou>\n"
        "<wave>Onii-chaaaan,</wave><wait 500> you're so silly!\n"
        "There aren't any MOBJs in\n"
        "this entire room yet! <wave>D:\n"
        "</wave><k>";

    const char ymetoolsMobjMenu_Modify[] =
        "<p><housou>\n"
        "<wave>Got it!</wave><wait 300> What do you want to\n"
        "modify, onii-chan? <wait 300><wave>OwO\n"
        "</wave><o>";

    const char ymetoolsMobjMenu_Modify_Select[] =
        "<select 0 -1 300 45>\n"
        "Coordinates\n"
        "Flags\n"
        "Object Type\n"
        "All done!";

    const char ymetoolsMobjMenu_Coords[] =
        "<p><housou>\n"
        "<wave>Okaaaay!</wave><wait 300> The object's position\n"
        "is currently (%d, %d, %d)!\n"
        "<wait 300>Want to change it? <wait 300><wave>:3\n"
        "</wave><o>";

    const char ymetoolsMobjMenu_Coords_Select[] =
        "<select 0 -1 200 45>\n"
        "X Pos\n"
        "Y Pos\n"
        "Z Pos\n"
        "All done!";

    const char ymetoolsMobjMenu_Flags[] =
        "<p><housou>\n"
        "Which flags to change? <wave>:3c\n"
        "</wave><wait 300>Flag0 is currently 0x%s;\n"
        "<wait 300>Flag4 is currently 0x%s~\n"
        "</wave><o>";

    const char ymetoolsMobjMenu_Flags_Select[] =
        "<select 0 -1 200 45>\n"
        "Flag0\n"
        "Flag4\n"
        "All done!";

    const char ymetoolsMobjMenu_InvalidFlags[] =
        "<housou>\n"
        "<wave>Onii-chaaaan,</wave><wait 250> this'll delete the\n"
        "object!<wait 250> Flag 0x1 is missing...\n"
        "<wait 250>Do you wanna try again? <wave>:o\n"
        "</wave><o>";

    const char ymetoolsMobjMenu_InvalidFlags_Select[] =
        "<select 0 -1 270 45>\n"
        "Let's try again\n"
        "Delete it!";

    customwin::CWSelectItemDesc cwDescs[300];

    // Moving *downward*
    customwin::CWSelectColorDef cwColors_Gay[] =
        {
            {{255, 0, 0, 155}, {255, 128, 0, 155}, 60, 0},   // Red-Orange
            {{255, 0, 255, 155}, {255, 0, 0, 155}, 60, 0},   // Magenta-Red
            {{128, 0, 255, 155}, {255, 0, 255, 155}, 60, 0}, // Purple-Magenta
            {{0, 0, 255, 155}, {128, 0, 255, 155}, 60, 0},   // Blue-Purple
            {{0, 255, 255, 155}, {0, 0, 255, 155}, 60, 0},   // Cyan-Blue
            {{0, 255, 0, 155}, {0, 255, 255, 155}, 60, 0},   // Green-Cyan
            {{255, 255, 0, 155}, {0, 255, 0, 155}, 60, 0},   // Yellow-Green
            {{255, 128, 0, 155}, {255, 255, 0, 155}, 60, 0}  // Orange-Yellow
    };

    wii::gx::GXColor cwHeaderBrown = {66, 114, 255, 255};

    wii::gx::GXColor cwHeaderTxtYellow = {255, 255, 0, 255};

    s32 createMobjWinDefs(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        u32 i = 0;
        s32 n = 0;
        mobjdrv::MobjEntry *mobj = mobjdrv::mobjdrv_wp->entries;
        msl::string::memset(&cwDescs, 0, sizeof(cwDescs));
        while (i < 256)
        {
            if ((mobj->flag0 & 1) != 0)
            {
                cwDescs[n].iconId = icondrv::ICON_YELLOW_CHOCO;
                msl::string::strcpy(cwDescs[n].nameTxt, mobj->instanceName);
                msl::stdio::sprintf(cwDescs[n].descTxt, ymetoolsMobjMenu_ExistingMobjs, mobjdrv::mobjGetModelName(mobj), (s32)mobj->pos.x, (s32)mobj->pos.y, (s32)mobj->pos.z);
                n = n + 1;
            }
            i = i + 1;
            mobj = mobj + 1;
        }
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        evtmgr_cmd::evtSetValue(evtEntry, args[0], n);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(createMobjWinDefs, 1)

    s32 readMobjCustomwinExObj(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char *mobjName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        mobjdrv::MobjEntry *mobj = mobjdrv::mobjNameToPtr(mobjName);
        evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)mobj);
        evtmgr_cmd::evtSetValue(evtEntry, args[2], (s32)mobjName);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(readMobjCustomwinExObj, 3)

    s32 keypadStrToCoord(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char *input = evt_sub::keypadInputStorage;
        s32 coord = 0;
        msl::stdio::sscanf(input, "%d", &coord);
        evtmgr_cmd::evtSetValue(evtEntry, args[0], coord);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(keypadStrToCoord, 1)

    s32 keypadInsertCurrentCoords(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char *mobjName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 orientation = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        s32 x = (s32)evtmgr_cmd::evtGetFloat(evtEntry, args[2]);
        s32 y = (s32)evtmgr_cmd::evtGetFloat(evtEntry, args[3]);
        s32 z = (s32)evtmgr_cmd::evtGetFloat(evtEntry, args[4]);
        s32 id = (s32)evtmgr_cmd::evtGetFloat(evtEntry, args[5]);
        evtmgr::EvtWork *evtWrk = evtmgr::evtGetWork();
        evtmgr::EvtEntry *brother = evtWrk->entries; // basically copied in full from evtCheckID
        s32 cur = evtWrk->entryCount;
        if (0 < evtWrk->entryCount)
        {
            do
            {
                if ((brother->flags & 1) != 0)
                {
                    if (brother->id == id)
                    {
                        break;
                    }
                }
                brother = brother + 1;
                cur = cur - 1;
            } while (cur != 0);
        }
        winmgr::WinmgrKeypad *keypad = (winmgr::WinmgrKeypad *)brother->tempU[0];
        keypad->keyDefs = keypadCoordKeys;
        mobjdrv::MobjEntry *mobj = mobjdrv::mobjNameToPtr(mobjName);
        char buf[6];
        switch (orientation)
        {
        case 0:
            mobj->pos.x = x;
            msl::stdio::sprintf(buf, "%d", x);
            break;
        case 1:
            mobj->pos.y = y;
            msl::stdio::sprintf(buf, "%d", y);
            break;
        default:
            mobj->pos.z = z;
            msl::stdio::sprintf(buf, "%d", z);
            break;
        }
        const char *pBuf = buf;
        // &brother->tempU[0]);
        char *pInput = keypad->input;
        msl::string::strcpy(pInput, pBuf);
        keypad->digitsInputted = (msl::string::strlen(pBuf));
        return 2;
    }
    EVT_DECLARE_USER_FUNC(keypadInsertCurrentCoords, 6)

    char keypadFlag0Buf[16];
    char keypadFlag4Buf[16];

    s32 mobjGetFlagsAsStr(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char *mobjName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        mobjdrv::MobjEntry *mobj = mobjdrv::mobjNameToPtr(mobjName);
        msl::string::memset(keypadFlag0Buf, 0, sizeof(keypadFlag0Buf));
        msl::stdio::sprintf(keypadFlag0Buf, "%x", mobj->flag0);
        char *flag0Str = keypadFlag0Buf;
        msl::string::memset(keypadFlag4Buf, 0, sizeof(keypadFlag4Buf));
        msl::stdio::sprintf(keypadFlag4Buf, "%x", mobj->flag4);
        char *flag4Str = keypadFlag4Buf;
        evtmgr_cmd::evtSetValue(evtEntry, args[1], (s32)flag0Str);
        evtmgr_cmd::evtSetValue(evtEntry, args[2], (s32)flag4Str);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(mobjGetFlagsAsStr, 3)

    s32 keypadInsertCurrentFlags(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char *flags = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 id = (s32)evtmgr_cmd::evtGetFloat(evtEntry, args[1]);
        evtmgr::EvtWork *evtWrk = evtmgr::evtGetWork();
        evtmgr::EvtEntry *brother = evtWrk->entries; // basically copied in full from evtCheckID
        s32 cur = evtWrk->entryCount;
        if (0 < evtWrk->entryCount)
        {
            do
            {
                if ((brother->flags & 1) != 0)
                {
                    if (brother->id == id)
                    {
                        break;
                    }
                }
                brother = brother + 1;
                cur = cur - 1;
            } while (cur != 0);
        }
        winmgr::WinmgrKeypad *keypad = (winmgr::WinmgrKeypad *)brother->tempU[0];
        // Overwrite key defs to add the remaining digits
        keypad->rows = 4;
        keypad->columns = 5;
        keypad->keyDefNum = 20;
        keypad->keyDefs = keypadFlagKeys;
        winmgr::winMgrSetSize(keypad->padWinEntryId, -165, 50, 335, 200);
        //    wii::os::OSReport("keypad: %p\n", keypad->input);
        // Get string for the flags and insert it into the keypad entry
        char buf[9];
        msl::stdio::sprintf(buf, "%s", flags);
        const char *pBuf = buf;
        char *pInput = keypad->input;
        msl::string::strcpy(pInput, pBuf);
        keypad->digitsInputted = (msl::string::strlen(pBuf));
        return 2;
    }
    EVT_DECLARE_USER_FUNC(keypadInsertCurrentFlags, 2)

    s32 keypadStrToFlag(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char *input = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 length = msl::string::strlen(input);
        char flexBuf[length];
        msl::string::memset(flexBuf, 0, sizeof(flexBuf));
        msl::string::strncpy(flexBuf, input, (length - 3));
        const char *input2 = flexBuf;
        keypadFlagStorage = 0;
        msl::stdio::sscanf(input2, "%x", &keypadFlagStorage);
        wii::os::OSReport("input: %s; input2: %s; keypadFlagStorage: %d\n", input, input2, keypadFlagStorage);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(keypadStrToFlag, 1)

    s32 mobjSetFlags(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char *mobjName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        s32 flag0Or4 = evtmgr_cmd::evtGetValue(evtEntry, args[1]);
        mobjdrv::MobjEntry *mobj = mobjdrv::mobjNameToPtr(mobjName);
        if (flag0Or4 == 0)
        {
            mobj->flag0 = keypadFlagStorage;
        }
        else
            mobj->flag4 = keypadFlagStorage;
        return 2;
    }
    EVT_DECLARE_USER_FUNC(mobjSetFlags, 2)

    s32 evt_mobj_calc_mtx(evtmgr::EvtEntry *evtEntry, bool firstRun)
    {
        (void)firstRun;
        evtmgr::EvtVar *args = (evtmgr::EvtVar *)evtEntry->pCurData;
        const char *mobjName = (const char *)evtmgr_cmd::evtGetValue(evtEntry, args[0]);
        mobjdrv::MobjEntry *mobj = mobjdrv::mobjNameToPtr(mobjName);
        mobjdrv::mobjCalcMtx2(mobj);
        return 2;
    }
    EVT_DECLARE_USER_FUNC(evt_mobj_calc_mtx, 1)

    EVT_BEGIN(ymetoolsMobjMenu_Main)
    ALLOC_USER_WRK(8)
    USER_FUNC(evt_mario::evt_mario_key_off, 1)
    USER_FUNC(evt_npc::evt_npc_freeze_all)
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(ymetoolsMobjMenu_Main_Text), 0, 0)
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(ymetoolsMobjMenu_Main_Select))
    USER_FUNC(evt_msg::evt_msg_continue)
    IF_EQUAL(LW(0), 1) // Existing Object
    LBL(0)
    USER_FUNC(createMobjWinDefs, LW(0))
    IF_EQUAL(LW(0), 0) // No existing objects in the map
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(ymetoolsMobjMenu_ExObj_NoExObj), 0, 0)
    GOTO(99)
    END_IF()
    USER_FUNC(customwin::EvtCWSelectEntry, PTR("Mobj"), customwin::CWSELECT_DEFAULT, PTR("MOBJs"), PTR("Select an MOBJ! ^w^\n:3 I love you!"), PTR(&cwDescs), LW(0))
    USER_FUNC(customwin::EvtCWSelectSetHeaderTextColor, PTR("Mobj"), PTR(&cwHeaderTxtYellow))
    USER_FUNC(customwin::EvtCWSelectMenuStart, PTR("Mobj"), 0, LW(0))
    IF_EQUAL(LW(0), -1)
    USER_FUNC(customwin::EvtCWSelectReset)
    GOTO(99)
    END_IF()
    USER_FUNC(customwin::EvtCWSelectGetSelectionName, LW(0), LW(0))
    USER_FUNC(readMobjCustomwinExObj, LW(0), LW(10), LW(11))
    USER_FUNC(customwin::EvtCWSelectReset)
    LBL(10)
    IF_EQUAL(UW(2), 1)
    SET(UW(2), 0)
    USER_FUNC(evt_msg::evt_msg_print_add, 1, PTR(ymetoolsMobjMenu_Modify))
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(ymetoolsMobjMenu_Modify), 0, 0)
    END_IF()
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(ymetoolsMobjMenu_Modify_Select))
    SET(LW(5), LW(0))
    SWITCH(LW(5))
    CASE_EQUAL(0) // Coordinates
    LBL(11)
    USER_FUNC(evt_mobj::evt_mobj_get_position, LW(11), LW(7), LW(8), LW(9))
    IF_EQUAL(UW(2), 1)
    SET(UW(2), 0)
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(ymetoolsMobjMenu_Coords), 0, 0, LW(7), LW(8), LW(9))
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print_add_insert, 1, PTR(ymetoolsMobjMenu_Coords), LW(7), LW(8), LW(9))
    END_IF()
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(ymetoolsMobjMenu_Coords_Select))
    SET(UW(2), 1)
    IF_EQUAL(LW(0), 3) // Return
    GOTO(10)
    ELSE()
    USER_FUNC(evt_msg::evt_msg_continue)
    BROTHER_EVT_ID(LW(15))
    USER_FUNC(evt_sub::evt_sub_keypad, 0xA, 6, UW(0), UW(1))
    END_BROTHER()
    WAIT_FRM(2)
    USER_FUNC(keypadInsertCurrentCoords, LW(11), LW(0), LW(7), LW(8), LW(9), LW(15))
    USER_FUNC(msgpatch::evt_msgpatch_add_entry, PTR("password_01"), PTR("Coords"), 1)
    DO(0)
    CHK_EVT(LW(15), LW(14))
    IF_EQUAL(LW(14), 0)
    DO_BREAK()
    END_IF()
    WAIT_FRM(1)
    WHILE()
    USER_FUNC(msgpatch::evt_msgpatch_del_entry, PTR("password_01"))
    IF_EQUAL(UW(0), 0)
    GOTO(11)
    ELSE()
    USER_FUNC(keypadStrToCoord, UW(1))
    END_IF()
    SWITCH(LW(0))
    CASE_EQUAL(0)
    USER_FUNC(evt_mobj::evt_mobj_set_position, LW(11), UW(1), LW(8), LW(9))
    CASE_EQUAL(1)
    USER_FUNC(evt_mobj::evt_mobj_set_position, LW(11), LW(7), UW(1), LW(9))
    CASE_EQUAL(2)
    USER_FUNC(evt_mobj::evt_mobj_set_position, LW(11), LW(7), LW(8), UW(1))
    END_SWITCH()
    USER_FUNC(evt_mobj_calc_mtx, LW(11))
    SET(UW(0), 0)
    SET(UW(1), 0)
    GOTO(11)
    END_IF()
    CASE_EQUAL(1) // Flags
    LBL(12)
    USER_FUNC(mobjGetFlagsAsStr, LW(11), LW(7), LW(8))
    IF_EQUAL(UW(2), 1)
    SET(UW(2), 0)
    USER_FUNC(evt_msg::evt_msg_print_insert, 1, PTR(ymetoolsMobjMenu_Flags), 0, 0, LW(7), LW(8))
    ELSE()
    USER_FUNC(evt_msg::evt_msg_print_add_insert, 1, PTR(ymetoolsMobjMenu_Flags), LW(7), LW(8))
    END_IF()
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(ymetoolsMobjMenu_Flags_Select))
    SET(UW(2), 1)
    IF_EQUAL(LW(0), 2) // Return
    GOTO(10)
    ELSE()
    USER_FUNC(evt_msg::evt_msg_continue)
    LBL(13)
    BROTHER_EVT_ID(LW(15))
    USER_FUNC(evt_sub::evt_sub_keypad, 0xA, 9, UW(0), UW(1))
    END_BROTHER()
    WAIT_FRM(2)
    IF_EQUAL(LW(0), 0)
    USER_FUNC(keypadInsertCurrentFlags, LW(7), LW(15))
    ELSE()
    USER_FUNC(keypadInsertCurrentFlags, LW(8), LW(15))
    END_IF()
    USER_FUNC(msgpatch::evt_msgpatch_add_entry, PTR("password_01"), PTR("Flags (Hex)"), 1)
    DO(0)
    CHK_EVT(LW(15), LW(14))
    IF_EQUAL(LW(14), 0)
    DO_BREAK()
    END_IF()
    WAIT_FRM(1)
    WHILE()
    USER_FUNC(msgpatch::evt_msgpatch_del_entry, PTR("password_01"))
    IF_EQUAL(UW(0), 0)
    GOTO(12)
    ELSE()
    USER_FUNC(keypadStrToFlag, UW(1))
    END_IF()
    IF_EQUAL(LW(0), 0)
    IF_FLAG((s32)keypadFlagStorage, 1)
    USER_FUNC(mobjSetFlags, LW(11), LW(0))
    ELSE()
    SET(LW(1), LW(0))
    USER_FUNC(evt_msg::evt_msg_print, 1, PTR(ymetoolsMobjMenu_InvalidFlags), 0, 0)
    USER_FUNC(evt_msg::evt_msg_select, 1, PTR(ymetoolsMobjMenu_InvalidFlags_Select))
    USER_FUNC(evt_msg::evt_msg_continue)
    SWITCH(LW(0))
    CASE_EQUAL(0) // Try Again
    GOTO(13)
    CASE_EQUAL(1) // Confirm Delete
    USER_FUNC(mobjSetFlags, LW(11), LW(1))
    SET(UW(2), 0)
    GOTO(0)
    END_SWITCH()
    END_IF()
    ELSE()
    USER_FUNC(mobjSetFlags, LW(11), LW(0))
    END_IF()
    SET(UW(0), 0)
    SET(UW(1), 0)
    GOTO(12)
    END_IF()
    CASE_EQUAL(2) // Object Type
    USER_FUNC(evt_msg::evt_msg_continue)
    GOTO(99)
    CASE_EQUAL(3) // All done!
    USER_FUNC(evt_msg::evt_msg_continue)
    GOTO(99)
    END_SWITCH()
    END_IF()
    LBL(99)
    USER_FUNC(evt_mario::evt_mario_key_on)
    USER_FUNC(evt_npc::evt_npc_unfreeze_all)
    RETURN()
    EVT_END()

    s32 iconEnum_FrmCtr = 0;

    // Handles checking for certain button combinations every frame
    void ymetoolsMain()
    {
        mario::MarioWork *mario = mario::marioGetPtr();
        mapdrv::MapWorkGroup *mapWkGrp = mapdrv::mapGetWork();
        mapdrv::MapEntry *mapEnt = &mapWkGrp->entries[0];
        // Handle opening the config menu
        u8 configMenuOpen = swdrv::swGet(1700);
        if (configMenuOpen == 0)
        {
            if ((mario->buttonsHeld & (WPAD_BTN_B | WPAD_BTN_Z)) == (WPAD_BTN_B | WPAD_BTN_Z))
            {
                swdrv::swSet(1700);
                evtmgr::evtEntryType(ymetoolsConfigMenu, 0, 0, 0);
            }
        }
        // Handle the mapobj display and hitobj on/off features
        if (ActiveYmeTool == YMETOOLS_OBJ_DISP)
        {
            if (curMapObj >= mapEnt->mapObjCount)
                curMapObj = 0;
            if (curMapObj < 0)
                curMapObj = (mapEnt->mapObjCount - 1);
            mapObj = mapEnt->mapObjs + curMapObj;
            // Ensure only map groups in MapGrp mode, and only map objects in MapObj mode
            while (((msl::string::strcmp(mapObj->joint->type, "mesh") == 0) && mapGrpMode) || ((msl::string::strcmp(mapObj->joint->type, "null") == 0) && !mapGrpMode))
            {
                curMapObj = curMapObj + 1;
                if (curMapObj >= mapEnt->mapObjCount)
                    curMapObj = 0;
                if (curMapObj < 0)
                    curMapObj = (mapEnt->mapObjCount - 1);
                mapObj = mapEnt->mapObjs + curMapObj;
            }
            msl::stdio::sprintf(mapObjName, "%s", mapObj->joint->name);
            if ((mapObj->flag0 & 0x1) != 0)
            {
                ymetoolsMapFlagOnOff(mapObj->joint->name, 1);
                objIsInactive = true;
            }
            ymetoolsMapSetColor(mapObj, 0);
            preserveObjStatus = false;
            if ((mario->buttonsHeld & (WPAD_BTN_C)) == (WPAD_BTN_C) && (mario->buttonsPressed & (WPAD_BTN_2)) == (WPAD_BTN_2)) // C held & 2 pressed
            {
                if (objIsInactive)
                {
                    ymetoolsMapFlagOnOff(mapObj->joint->name, 0);
                    objIsInactive = false;
                    preserveObjStatus = true;
                }
                evtmgr::EvtEntry *evt = evtmgr::evtEntryType(delayClearHighlight, 0, 0, 0);
                evt->lw[0] = (s32)mapObj->joint->name;
                evt->lw[1] = (s32)mapGrpMode;
                curMapObj = curMapObj + 1;
                if (curMapObj >= mapEnt->mapObjCount)
                    curMapObj = 0;
                if (curMapObj < 0)
                    curMapObj = (mapEnt->mapObjCount - 1);
                mapObj = mapEnt->mapObjs + curMapObj;
                while (((msl::string::strcmp(mapObj->joint->type, "mesh") == 0) && mapGrpMode) || ((msl::string::strcmp(mapObj->joint->type, "null") == 0) && !mapGrpMode))
                {
                    curMapObj = curMapObj + 1;
                    if (curMapObj >= mapEnt->mapObjCount)
                        curMapObj = 0;
                    if (curMapObj < 0)
                        curMapObj = (mapEnt->mapObjCount - 1);
                    mapObj = mapEnt->mapObjs + curMapObj;
                }
            }
            if ((mario->buttonsHeld & (WPAD_BTN_C)) == (WPAD_BTN_C) && (mario->buttonsPressed & (WPAD_BTN_1)) == (WPAD_BTN_1)) // C held & 1 pressed
            {
                if (objIsInactive)
                {
                    ymetoolsMapFlagOnOff(mapObj->joint->name, 0);
                    objIsInactive = false;
                    preserveObjStatus = true;
                }
                evtmgr::EvtEntry *evt = evtmgr::evtEntryType(delayClearHighlight, 0, 0, 0);
                evt->lw[0] = (s32)mapObj->joint->name;
                evt->lw[1] = (s32)mapGrpMode;
                curMapObj = curMapObj - 1;
                if (curMapObj >= mapEnt->mapObjCount)
                    curMapObj = 0;
                if (curMapObj < 0)
                    curMapObj = (mapEnt->mapObjCount - 1);
                mapObj = mapEnt->mapObjs + curMapObj;
                while (((msl::string::strcmp(mapObj->joint->type, "mesh") == 0) && mapGrpMode) || ((msl::string::strcmp(mapObj->joint->type, "null") == 0) && !mapGrpMode))
                {
                    curMapObj = curMapObj - 1;
                    if (curMapObj >= mapEnt->mapObjCount)
                        curMapObj = 0;
                    if (curMapObj < 0)
                        curMapObj = (mapEnt->mapObjCount - 1);
                    mapObj = mapEnt->mapObjs + curMapObj;
                }
            }
            if ((mario->buttonsHeld & (WPAD_BTN_Z)) == (WPAD_BTN_Z) && (mario->buttonsPressed & (WPAD_BTN_2)) == (WPAD_BTN_2)) // Z held & 2 pressed
            {
                if (objIsInactive)
                {
                    objIsInactive = false;
                }
                else if ((mapObj->flag0 & 0x1) == 0)
                {
                    objIsInactive = true;
                    ymetoolsMapFlagOnOff(mapObj->joint->name, 1);
                }
                wii::os::OSReport("MapObj Ptr: 0x%p\n", mapObj);
            }
            if ((mario->buttonsHeld & (WPAD_BTN_Z)) == (WPAD_BTN_Z) && (mario->buttonsPressed & (WPAD_BTN_1)) == (WPAD_BTN_1)) // Z held & 1 pressed
            {
                const char *msg = hitObjName;
                if (msl::string::strcmp(msg, "Null") != 0)
                {
                    hitdrv::HitObj *hitObj = hitdrv::hitNameToPtr(msg);
                    if ((hitObj->flags & 0x1) == 0)
                    {
                        hitdrv::hitGrpFlagOn(0, msg, 1);
                        hitObjStatus = 1;
                    }
                    else
                    {
                        hitdrv::hitGrpFlagOff(0, msg, 1);
                        hitObjStatus = 2;
                    }
                }
            }
            if ((mario->buttonsHeld & (WPAD_BTN_Z)) == (WPAD_BTN_Z) && (mario->buttonsPressed & (WPAD_BTN_PLUS)) == (WPAD_BTN_PLUS)) // Z held & + pressed
            {
                if (msl::string::strcmp(hitObjName, "Null") != 0)
                {
                    hitdrv::HitObj *hObj = hitdrv::hitNameToPtr(hitObjName);
                    if (hObj->parent != nullptr)
                        msl::stdio::sprintf(hitObjName, "%s", hObj->parent->joint->name);
                }
            }
            if ((mario->buttonsHeld & (WPAD_BTN_C | WPAD_BTN_2)) == (WPAD_BTN_C | WPAD_BTN_2)) // C and 2 held to scroll
            {
                plusHeldTimer = plusHeldTimer + 1;
                if (plusHeldTimer > 30 && spmario::gp->frameCounter % 6 == 0)
                {
                    if (objIsInactive)
                    {
                        ymetoolsMapFlagOnOff(mapObj->joint->name, 0);
                        objIsInactive = false;
                        preserveObjStatus = true;
                    }
                    evtmgr::EvtEntry *evt = evtmgr::evtEntryType(delayClearHighlight, 0, 0, 0);
                    evt->lw[0] = (s32)mapObj->joint->name;
                    evt->lw[1] = (s32)mapGrpMode;
                    curMapObj = curMapObj + 1;
                    if (curMapObj >= mapEnt->mapObjCount)
                        curMapObj = 0;
                    if (curMapObj < 0)
                        curMapObj = (mapEnt->mapObjCount - 1);
                    mapObj = mapEnt->mapObjs + curMapObj;
                    while (((msl::string::strcmp(mapObj->joint->type, "mesh") == 0) && mapGrpMode) || ((msl::string::strcmp(mapObj->joint->type, "null") == 0) && !mapGrpMode))
                    {
                        curMapObj = curMapObj + 1;
                        if (curMapObj >= mapEnt->mapObjCount)
                            curMapObj = 0;
                        if (curMapObj < 0)
                            curMapObj = (mapEnt->mapObjCount - 1);
                        mapObj = mapEnt->mapObjs + curMapObj;
                    }
                }
            }
            else
                plusHeldTimer = 0;
            if ((mario->buttonsHeld & (WPAD_BTN_C | WPAD_BTN_1)) == (WPAD_BTN_C | WPAD_BTN_1)) // C and 1 held to scroll
            {
                minusHeldTimer = minusHeldTimer + 1;
                if (minusHeldTimer > 30 && spmario::gp->frameCounter % 6 == 0)
                {
                    if (objIsInactive)
                    {
                        ymetoolsMapFlagOnOff(mapObj->joint->name, 0);
                        objIsInactive = false;
                        preserveObjStatus = true;
                    }
                    evtmgr::EvtEntry *evt = evtmgr::evtEntryType(delayClearHighlight, 0, 0, 0);
                    evt->lw[0] = (s32)mapObj->joint->name;
                    evt->lw[1] = (s32)mapGrpMode;
                    curMapObj = curMapObj - 1;
                    if (curMapObj >= mapEnt->mapObjCount)
                        curMapObj = 0;
                    if (curMapObj < 0)
                        curMapObj = (mapEnt->mapObjCount - 1);
                    mapObj = mapEnt->mapObjs + curMapObj;
                    while (((msl::string::strcmp(mapObj->joint->type, "mesh") == 0) && mapGrpMode) || ((msl::string::strcmp(mapObj->joint->type, "null") == 0) && !mapGrpMode))
                    {
                        curMapObj = curMapObj - 1;
                        if (curMapObj >= mapEnt->mapObjCount)
                            curMapObj = 0;
                        if (curMapObj < 0)
                            curMapObj = (mapEnt->mapObjCount - 1);
                        mapObj = mapEnt->mapObjs + curMapObj;
                    }
                }
            }
            else
                minusHeldTimer = 0;
            if ((mario->buttonsHeld & (WPAD_BTN_C)) == (WPAD_BTN_C) && (mario->buttonsPressed & (WPAD_BTN_Z)) == (WPAD_BTN_Z)) // C held & Z pressed
            {
                ymetoolsMapSetColor(mapObj, 1);
                if (mapGrpMode)
                    mapGrpMode = false;
                else
                    mapGrpMode = true;
            }
        }
        else if (ActiveYmeTool == YMETOOLS_MAPANIM_DISP)
        {
            if (curMapAnim >= mapEnt->animCount)
                curMapAnim = 0;
            if (curMapAnim < 0)
                curMapAnim = (mapEnt->animCount - 1);
            mapAnim = mapEnt->animData + curMapAnim;
            if (mapAnim == nullptr)
            {
                msl::stdio::sprintf(mapAnimName, "None here!");
                return;
            }
            else
                msl::stdio::sprintf(mapAnimName, "%s", mapAnim->fileData->name);
            if ((mario->buttonsHeld & (WPAD_BTN_Z)) == (WPAD_BTN_Z) && (mario->buttonsPressed & (WPAD_BTN_1)) == (WPAD_BTN_1)) // Z held & 1 pressed
            {
                mapdrv::mapPlayAnimationLv(mapAnim->fileData->name, mapAnimDispMode, 0);
            }
            if ((mario->buttonsHeld & (WPAD_BTN_Z)) == (WPAD_BTN_Z) && (mario->buttonsPressed & (WPAD_BTN_2)) == (WPAD_BTN_2)) // Z held & 2 pressed
            {
                switch (mapAnimDispMode)
                {
                case 0:
                    mapAnimDispMode = 1;
                    break;
                case 1:
                    mapAnimDispMode = 2;
                    break;
                case 2:
                    mapAnimDispMode = 10;
                    break;
                default:
                    mapAnimDispMode = 0;
                    break;
                }
            }
            if ((mario->buttonsHeld & (WPAD_BTN_C)) == (WPAD_BTN_C) && (mario->buttonsPressed & (WPAD_BTN_1)) == (WPAD_BTN_1)) // C held & 1 pressed
            {
                curMapAnim = curMapAnim - 1;
                if (curMapAnim >= mapEnt->animCount)
                    curMapAnim = 0;
                if (curMapAnim < 0)
                    curMapAnim = (mapEnt->animCount - 1);
                mapAnim = mapEnt->animData + curMapAnim;
            }
            if ((mario->buttonsHeld & (WPAD_BTN_C)) == (WPAD_BTN_C) && (mario->buttonsPressed & (WPAD_BTN_2)) == (WPAD_BTN_2)) // C held & 2 pressed
            {
                curMapAnim = curMapAnim + 1;
                if (curMapAnim >= mapEnt->animCount)
                    curMapAnim = 0;
                if (curMapAnim < 0)
                    curMapAnim = (mapEnt->animCount - 1);
                mapAnim = mapEnt->animData + curMapAnim;
            }
            if ((mario->buttonsHeld & (WPAD_BTN_C | WPAD_BTN_2)) == (WPAD_BTN_C | WPAD_BTN_2)) // C & 2 held
            {
                plusHeldTimer = plusHeldTimer + 1;
                if (plusHeldTimer > 30 && spmario::gp->frameCounter % 6 == 0)
                {
                    curMapAnim = curMapAnim + 1;
                    if (curMapAnim >= mapEnt->animCount)
                        curMapAnim = 0;
                    if (curMapAnim < 0)
                        curMapAnim = (mapEnt->animCount - 1);
                    mapAnim = mapEnt->animData + curMapAnim;
                }
            }
            else
                plusHeldTimer = 0;
            if ((mario->buttonsHeld & (WPAD_BTN_C | WPAD_BTN_1)) == (WPAD_BTN_C | WPAD_BTN_1)) // C & 1 held
            {
                minusHeldTimer = minusHeldTimer + 1;
                if (minusHeldTimer > 30 && spmario::gp->frameCounter % 6 == 0)
                {
                    curMapAnim = curMapAnim - 1;
                    if (curMapAnim >= mapEnt->animCount)
                        curMapAnim = 0;
                    if (curMapAnim < 0)
                        curMapAnim = (mapEnt->animCount - 1);
                    mapAnim = mapEnt->animData + curMapAnim;
                }
            }
            else
                minusHeldTimer = 0;
        }
        else if (ActiveYmeTool == YMETOOLS_MOBJ)
        {
            if ((mario->buttonsPressed & (WPAD_BTN_C)) == (WPAD_BTN_C)) // C pressed
            {
                evtmgr::evtEntryType(ymetoolsMobjMenu_Main, 0, 0, 0);
            }
        }
        /*
            SLIM GAP VISUALIZER TEST!

        if (seqdrv::seqGetSeq() == seqdrv::SEQ_GAMEOVER || seqdrv::seqGetSeq() == seqdrv::SEQ_TITLE)
            seqdrv::seqSetSeq(seqdrv::SEQ_MAPCHANGE, map_data::mapData[curMap_n++]->name, "");
        else if (seqdrv::seqGetSeq() == seqdrv::SEQ_GAME)
        {
            const char *name = seq_mapchange::seq_mapchange_wp->mapName;
            s32 n = 0;
            for (s32 i = 0; i < mapEnt->hitObjCount; i += 1)
            {
                hitdrv::HitObj *hitEntry = &mapEnt->hitObjs[i];
                if ((hitEntry->attr & 0x10) != 0) // Is a slim gap
                {
                    n += 1;
                    if ((hitEntry->attr & 0x40000000) != 0) // Is 2D
                        wii::os::OSReport("[%s] Full Match: %s has attr 0x%x\n", name, hitdrv::hitGetName(hitEntry), hitEntry->attr);
                    else
                        wii::os::OSReport("[%s] Partial Match: %s has attr 0x%x.\n", name, hitdrv::hitGetName(hitEntry), hitEntry->attr);
                }
            }
            if (n == 0)
                wii::os::OSReport("[%s] No Slim gaps were found in this map.\n", name);
            seqdrv::seqSetSeq(seqdrv::SEQ_MAPCHANGE, map_data::mapData[curMap_n++]->name, "");
        }*/

        /*
             Code directly stolent from SPMIIWG to make these controls less awful
             Hammer with B any time instead of Cudge and 1
        */
        if ((mario->buttonsPressed & (WPAD_BTN_B)) == (WPAD_BTN_B)) // B pressed
        {
            if ((mario->flags & 0x40000000) != 0 || (mario->flags & 0x10000000) != 0)
                return; // Mega Star; Lock Motion ID
            if ((mario->dispFlags & MARIO_DISP_FLAG_CAM_ID) == 0 && (mario->dispFlags & MARIO_DISP_FLAG_INVISIBLE) == 0 && (mario->flags & 8) == 0)
            {
                u16 motion = mario->motionId;
                if ((motion == MOT_WALK || motion == MOT_DASH || motion == MOT_STAY || motion == MOT_SPACE_SWIM || motion == MOT_SPINDASH) && (msl::string::strcmp(mario->curPoseName, "D_2") != 0))
                    mario_motion::marioChgMot(MOT_HAMMER);
            }
        }
        /*
            ICON ID ENUM TEST, REMOVE LATER

        if ((mario->buttonsHeld & (WPAD_BTN_B)) == (WPAD_BTN_B)) // B held
        {
            wii::mtx::Mtx34 mtxPos, mtxScale;
            wii::mtx::PSMTXTrans(mtxPos, 0.0f, 0.0f, 0.0f);
            wii::mtx::PSMTXScale(mtxScale, 1.0f, 1.0f, 1.0f);
            wii::mtx::PSMTXConcat(mtxPos, mtxScale, mtxPos);
            icondrv::iconDispGxCol(mtxPos, 0x18, iconEnum_FrmCtr++, {255, 255, 255, 255});
        }*/
        return;
    }

    mario::MarioWork *HitobjDispTool()
    {
        mario::MarioWork *mario = mario::marioGetPtr();
        if (ActiveYmeTool == YMETOOLS_OBJ_DISP)
        {
            if (mario->hitObjHammeredWall != nullptr && mario->hitObjHammeredWall != 0)
            {
                msl::stdio::sprintf(hitObjName, "%s", mario->hitObjHammeredWall->joint->name);
                wii::os::OSReport("HitObj Ptr: %x (Attr: %x)\n", (u32)mario->hitObjHammeredWall, mario->hitObjHammeredWall->attr);
            }
            else if (mario->hitObjHammeredFloor != nullptr && mario->hitObjHammeredFloor != 0)
            {
                msl::stdio::sprintf(hitObjName, "%s", mario->hitObjHammeredFloor->joint->name);
                wii::os::OSReport("HitObj Ptr: %x (Attr: %x)\n", (u32)mario->hitObjHammeredFloor, mario->hitObjHammeredFloor->attr);
            }
            else
            {
                msl::stdio::sprintf(hitObjName, "Null");
            }
            hitObjStatus = 0;
        }
        return mario;
    }

    const char *ymetoolsResetVars()
    {
        hitObjStatus = 0;
        mapObjStatus = 0;
        curMapObj = 0;
        msl::stdio::sprintf(hitObjName, "Null");
        curMapAnim = 0;
        return "./map";
    }

    void implementTools()
    {
        globalop::globalopAddEntry((void *)ymetoolsMain, nullptr);
        writeBranchLink(mot_fairy_mario::mot_hammer, 0x604, HitobjDispTool);
        writeBranchLink(seq_mapchange::seq_mapChangeMain, 0x304, ymetoolsResetVars);
        postTextDisplay();
    }

    void mot_hammer_post_new(mario::MarioWork *mario)
    {
        mario->flags = mario->flags & ~0x980;
    }

    void patchHammerFuncs()
    {
        // Patch mot_hammer with NOPS. Thank you for the help John
        writeWord(mot_fairy_mario::mot_hammer, 0x38, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x2B0, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x2B4, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x2B8, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x2BC, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x3D4, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x3D8, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x3DC, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x3E0, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x654, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x658, NOP);
        writeWord(mot_fairy_mario::mot_hammer, 0x65C, NOP);
        patch::hookFunction(mot_fairy_mario::mot_hammer_post, mot_hammer_post_new);
    }

    void ymeMain()
    {
        implementTools();
        patchHammerFuncs();
    }
}
