#pragma once
#include <common.h>
#include <msl/stdio.h>
#include <msl/string.h>
#include <spm/icondrv.h>
#include <spm/msgdrv.h>
#include <spm/npcdrv.h>
#include <spm/pausewin.h>
#include <spm/search.h>
#include <spm/spmario.h>
#include <wii/sc.h>

namespace mod {
    using namespace spm;

/*
    Macros to reduce redundancy somewhat
*/

// Start a new text box
#define BRK "<k>\n<p>\n"

// Initiate a system text box
#define INIT_SYS "<system>\n"

// Initiate a Tippi tattle text box
#define INIT_TATTLE "<fairy><keyyon>"

// Continue text after a selection is made
#define CONT "<p>\n"

// Continue text after a selection is made, specifying system type (possibly redundant)
#define CONT_SYS "<p><system>\n"

// End a text box
#define END "<k>"

// Suspend a text box with a text selection prompt
#define PROMPT "<o>"

// I don't think this will ever be used in its current state, but just in case
#define STR(text) \
    #text "\n"

    enum Tribe2Tattle_Types {
        /* 0x0 */ TATTLE,
        /* 0x1 */ CARD_NAME,
        /* 0x2 */ CARD_DESC
    };

    struct NPCMessagePatchData {
        npcdrv::NPCTribeId tribeId;
        const char * nameMsg;
        const char * cardMsg;
        const char * tattleMsg;
    };

    const char savefile_start_noversion[] =
        INIT_SYS
        "This save file is unmodded\n"
        "or from an old version of\n"
        "Lunatic Pit.\n" BRK
        "Playing this save file may\n"
        "result in unintended errors\n"
        "during gameplay.\n" BRK
        "It is recommended that you\n"
        "create a new save file to\n"
        "play this version.\n" BRK
        "Start save file %d anyway?\n" PROMPT;

    const char savefile_start_oldversion[] =
        INIT_SYS
        "This save file is from an\n"
        "old version of Lunatic Pit.\n" BRK
        "Playing this save file may\n"
        "result in unintended errors\n"
        "during gameplay.\n" BRK
        "It is recommended that you\n"
        "create a new save file to\n"
        "play this version.\n" BRK
        "Start save file %d anyway?\n" PROMPT;

    const char savefile_start_update[] =
        CONT_SYS
        "Do you want to update this\n"
        "save file's Lunatic Pit\n"
        "version?\n" PROMPT;

    const char backCursyaText[] =
        "<system>\nlmao skill issue\n<k>";

    const char npcGetNameFromTribeIdError[] =
        "Null";

    // Custom Selects, Difficulty Options, and RFC
    const char difficultyOptions[] =
        "<select 1 -1 160 45>\n"
        "Easy\n"
        "Normal\n"
        "Hard";

    const char difficultyText[] =
        CONT_SYS
        "Choose your difficulty.\n" PROMPT;

    const char musicSignRFText[] =
        INIT_SYS
        "You can choose new music to\n"
        "play in the Pit by reading\n"
        "this sign!\n" END;

    const char nyMusicName[] =
        "Modern Mix";

    const char nyMusicDesc[] =
        "Lively and insert descriptor!\n"
        "Composed by Nilyoshi.\n" BRK
        "This theme gets more intense\n"
        "as you traverse the Pit.";

    const char vMusicName[] =
        "Pit of 100 Trials";

    const char vMusicDesc[] =
        "Mysterious and entrancing!\n"
        "The classic Pit music.";

    const char ttMusicName[] =
        "8-Bit Mix";

    const char ttMusicDesc[] =
        "A chiptune jam! Composed by\n"
        "Tater-Tot Tunes.";

    const char plMusicName[] =
        "Synth Mix";

    const char plMusicDesc[] =
        "Synthy and orchestral!\n"
        "Composed by Plazzap.";

    const char noMusicName[] =
        "No Music";

    const char noMusicDesc[] =
        "Play your own music in the\n"
        "background, if you'd like!";

    const char jdMusicName[] =
        "Piano Mix";

    const char jdMusicDesc[] =
        "Jazzy and emotional! Performed\n"
        "live by Jdaster64.";

    const char zkMusicName[] =
        "Pit of 100 Beats";

    const char zkMusicDesc[] =
        "Insert descriptors here!\n"
        "Arranged by Zaku.";

    const char moverFeaturesName[] =
        "Movers";

    const char moverFeaturesDesc[] =
        "These guys can help you skip\n"
        "floors, if you have coin!";

    const char merlunaFeaturesName[] =
        "Merluna";

    const char merlunaFeaturesDesc[] =
        "The esoteric purveyor of Divine\n"
        "Judgement!... But not right now.\n";

    const char whackaFeaturesName[] =
        "Whacka";

    const char whackaFeaturesDesc[] =
        "Everyone's favorite blue mole,\n"
        "Whacka! Please don't whack him.";

    const char marioFeaturesName[] =
        "Mario";

    const char marioFeaturesDesc[] =
        "Our hero, Mario! You wouldn't\n"
        "disable him, right?";

    const char lockPatchesName[] =
        "Patched Locks";

    const char lockPatchesDesc[] =
        "Were you tired of selecting\n"
        "a key every time? I sure was.";

    const char hpPatchesName[] =
        "Mute Low HP Sounds";

    const char hpPatchesDesc[] =
        "The only sounds worse than your\n"
        "Wii crashing on a loading zone.";

    const char explosionShakeAccessName[] =
        "Lighter Tremors";

    const char explosionShakeAccessDesc[] =
        "When the screen shakes, it will\n"
        "be significantly more gentle.";

    const char aerodynamicAccessName[] =
        "Aerodynamic";

    const char aerodynamicAccessDesc[] =
        "Mario will become approximately\n"
        "2% more aerodynamic.";

    const char baldMarioName[] = "Baldio";

    const char baldMarioDesc[] =
        "Our hero, Baldio! Press <icon PAD_A 0.58 0 3 2> to\n"
        "flip between dimensions!\n"
        "Watch out: if his Flip Meter\n"
        "runs out, Baldio will get hurt.";

    const char statNamesAccessName[] =
        "Shorter Stat Names";

    const char statNamesAccessDesc[] =
        "Names on the stats display tab\n"
        "will be truncated.\n" BRK
        "Examples: Crit Mult -> CM;\n"
        "Damage Reduction -> DR";

    const char selectMusicBox[] =
        "Select a Tune";

    const char selectMusicBlueText[] =
        "Music";

    const char selectJimboBox[] =
        "Select your Settings";

    const char selectJimboBlueText[] =
        "Options";

    const char chestText[] =
        "<system>"
        "Open the %s Chest?\n"
        "<wait 300>You have <icon PAD_1 0.67 0 1 3>%d.\n"
        "<wait 300>Open: <icon PAD_1 0.67 0 1 3>%d    Reroll: <icon PAD_1 0.67 0 1 3>%d\n"
        "<dkey><wait 300></dkey><o>";

    const char chestOptions[] =
        "<select 0 -1 270 45>\n"
        "Open it!\n"
        "Leave it.\n"
        "Reroll rarity\n"
        "Huh? Whuh?";

    const char rfcChestName[] =
        "box";

    const char chestKeyDescPtr[] =
        "msg_chest_key_desc";

    const char chestKeyNamePtr[] =
        "msg_chest_key_name";

    const char chestKeyDesc[] =
        "A key that can open chests\n"
        "in the Pit of 100 Trials.";

    const char rfcHelp[] =
        CONT_SYS
        "Rest floor chests can be any\n"
        "of 4 rarity types,<wait 200> all with\n"
        "loot of increasing quality.\n" BRK
        "Chest rarities spawn like so:\n"
        "40% Common   30% Uncommon\n"
        "20% Rare       10% Legendary\n" BRK
        "In each chest,<wait 200> you <wave>may</wave> find\n"
        "special items that upgrade\n"
        "stats like Crit and Defense.\n" BRK
        "However, to open these chests,\n"
        "<wait 200>you will need a certain number\n"
        "of Chest Keys...\n" BRK
        "Chest keys have a chance to\n"
        "drop from any enemy in any\n"
        "floor, <wait 200><wave>but they're rare!\n"
        "</wave><k>\n<p>\n"
        "Up to 1 chest key may spawn\n"
        "randomly per floor, with 2+\n"
        "guaranteed every 10 floors.\n" BRK
        "Consider saving your chest\n"
        "keys for when you encounter\n"
        "a Rare or Legendary chest!\n" BRK
        "You can reroll the chest\n"
        "rarity, but only before you\n"
        "open the chest.\n" BRK
        "Rerolls cost one chest key,\n"
        "but each reroll will make the\n"
        "next one more expensive!\n" BRK
        "Oh, and just a warning...\n"
        "<wait 500><dynamic 3>Please</dynamic> don't open chests if\n"
        "you don't have the keys.\n" END;

    const char chestKeysToHp[] =
        INIT_SYS
        "%d chest keys were converted\n"
        "to %d HP!" END;

    const char smallChestCoins[] =
        "<system>\nYou received %d coins!\n<k>";

    const char smallChestHp[] =
        "<system>\nYou received %d HP!\n<k>";

    const char voucherIntro[] =
        INIT_SYS
        "You've obtained a Voucher!\n" BRK
        "When certain conditions are\n"
        "met, Vouchers will cause a\n"
        "positive effect to occur.\n" BRK
        "When a Voucher takes effect,\n"
        "you'll see its icon spin\n"
        "around briefly.\n" BRK
        "However, any time a Voucher\n"
        "activates, it might tear,\n"
        "making it spin rapidly.\n" BRK
        "When a Voucher tears, a\n"
        "secondary effect will occur\n"
        "before it disappears.\n" BRK
        "You can learn more about an\n"
        "active Voucher anytime in the\n"
        "<icon PAD_1 0.67 0 0 0> Active Effects menu.\n" END;

    const char chestKeyName[] =
        "Chest Key";

    const char cakeVName[] =
        "Cake Voucher";

    const char thunderVName[] =
        "Thunder Voucher";

    const char judgementVName[] =
        "Judgement Voucher";

    const char stellarVName[] =
        "Stellar Voucher";

    const char redVName[] =
        "Determination Voucher";

    const char orangeVName[] =
        "Bravery Voucher";

    const char yellowVName[] =
        "Justice Voucher";

    const char greenVName[] =
        "Kindness Voucher";

    const char cyanVName[] =
        "Patience Voucher";

    const char blueVName[] =
        "Integrity Voucher";

    const char purpleVName[] =
        "Perseverance Voucher";

    const char whiteVName[] =
        "Limerence Voucher";

    const char blackVName[] =
        "Contrivance Voucher";

    const char soul1Name[] =
        "Soul Drop";

    const char soul2Name[] =
        "Soul Boon";

    const char soul3Name[] =
        "Soul Epiphany";

    const char soul4Name[] =
        "Soul Legacy";

    const char spirit1Name[] =
        "Spirit Drop";

    const char spirit2Name[] =
        "Spirit Boon";

    const char spirit3Name[] =
        "Spirit Epiphany";

    const char spirit4Name[] =
        "Spirit Legacy";

    const char aegis1Name[] =
        "Aegis Endowment";

    const char aegis2Name[] =
        "Aegis Invocation";

    const char auspice1Name[] =
        "Auspice Endowment";

    const char auspice2Name[] =
        "Auspice Invocation";

    const char artiSoulName[] =
        "Artifact of Soul";

    const char artiSpiritName[] =
        "Artifact of Spirit";

    const char artiAegisName[] =
        "Artifact of Aegis";

    const char artiAuspiceName[] =
        "Artifact of Auspice";

    const char artiDelightName[] =
        "Artifact of Delight";

    const char artiDemiseName[] =
        "Artifact of Demise";

    const char artiSoulDesc[] =
        "Permanently gain +12%\n"
        "Crit Rate while in the Pit.";

    const char artiSpiritDesc[] =
        "Permanently gain +100%\n"
        "Crit Mult while in the Pit.";

    const char artiAegisDesc[] =
        "Permanently gain +1 Defense\n"
        "while in the Pit.";

    const char artiAuspiceDesc[] =
        "Permanently gain +20% Damage\n"
        "Reduction while in the Pit.";

    const char artiDelightDesc[] =
        "Permanently gain +30\n"
        "Max HP while in the Pit.";

    const char artiDemiseDesc[] =
        "Permanently gain +2 Attack\n"
        "while in the Pit.";

    const char cakeVDesc[] =
        "Max HP is increased by 2 for\n"
        "every enemy room completed.\n" BRK
        "%d%% chance to tear after %d\n"
        "activations.\n" BRK
        "Tear: Halves total Max HP\n"
        "bonus and adds it to HP.";

    const char thunderVDesc[] =
        "An offensive stat is increased\n"
        "for every 30 enemies defeated.\n" BRK
        "You may get either +8%% Crit Mult,\n"
        "+2%% Crit Rate, or +1 Attack.\n" BRK
        "%d%% chance to tear after %d\n"
        "activations.\n" BRK
        "Tear: Halves all offensive stat\n"
        "bonuses, then adds a final bonus.";

    const char judgementVDesc[] =
        "Judgement Voucher";

    const char stellarVDesc[] =
        "Increases item drop rates\n"
        "from most enemies.\n" BRK
        "Common Chests will not appear\n"
        "on rest floors.\n" BRK
        "Chest reroll cost is lowered\n"
        "from 1 to 0.\n" BRK
        "%d%% chance to tear after %d\n"
        "activations.\n" BRK
        "Tear: Instantly obtain a\n"
        "Chest Key.";

    const char redVDesc[] =
        "Determination Voucher";

    const char orangeVDesc[] =
        "Bravery Voucher";

    const char yellowVDesc[] =
        "Justice Voucher";

    const char greenVDesc[] =
        "Kindness Voucher";

    const char cyanVDesc[] =
        "Patience Voucher";

    const char blueVDesc[] =
        "Integrity Voucher";

    const char purpleVDesc[] =
        "Perseverance Voucher";

    const char whiteVDesc[] =
        "Limerence Voucher";

    const char blackVDesc[] =
        "A world without him is empty...\n"
        "A life without him is empty...\n" BRK
        "...so let there be light.\n";

    const char soul1Desc[] =
        "Increases Crit Rate by 4%.";

    const char soul2Desc[] =
        "Increases Crit Rate by 8%.";

    const char soul3Desc[] =
        "Increases Crit Rate by 16%.";

    const char soul4Desc[] =
        "Increases Crit Rate by 24%.";

    const char spirit1Desc[] =
        "Increases Crit Mult by 25%.";

    const char spirit2Desc[] =
        "Increases Crit Mult by 50%.";

    const char spirit3Desc[] =
        "Increases Crit Mult by 100%.";

    const char spirit4Desc[] =
        "Increases Crit Mult by 150%.";

    const char auspice1Desc[] =
        "Increases Damage Reduction\n"
        "by 15%.";

    const char auspice2Desc[] =
        "Increases Damage Reduction\n"
        "by 25%.";

    const char aegis1Desc[] =
        "Increases Defense by 1.";

    const char aegis2Desc[] =
        "Increases Defense by 1 and\n"
        "lowers minimum damage to 0.";

    const char soul1Get[] =
        "<system>\nYour Crit Rate has been\nincreased by 4%!\n<k>";

    const char soul2Get[] =
        "<system>\nYour Crit Rate has been\nincreased by 8%!\n<k>";

    const char soul3Get[] =
        "<system>\nYour Crit Rate has been\nincreased by 16%!\n<k>";

    const char soul4Get[] =
        "<system>\nYour Crit Rate has been\nincreased by 24%!\n<k>";

    const char spirit1Get[] =
        "<system>\nYour Crit Mult has been\nincreased by 25%!\n<k>";

    const char spirit2Get[] =
        "<system>\nYour Crit Mult has been\nincreased by 50%!\n<k>";

    const char spirit3Get[] =
        "<system>\nYour Crit Mult has been\nincreased by 100%!\n<k>";

    const char spirit4Get[] =
        "<system>\nYour Crit Mult has been\nincreased by 150%!\n<k>";

    const char auspice1Get[] =
        "<system>\nYour Damage Reduction has\nbeen increased by 15%!\n<k>";

    const char auspice2Get[] =
        "<system>\nYour Damage Reduction has\nbeen increased by 25%!\n<k>";

    const char aegis1Get[] =
        "<system>\nYour Defense has been\nincreased by 1!\n<k>";

    const char aegis2Get[] =
        "<system>\nYour Defense has been\nincreased by 1!\n<k>\n<p>\nAdditionally, you can now take\nzero damage if enemies are\nweak enough.<k>";

    // Shadoo
    const char shadooIntro[] =
        "<housou><wave>Hiiiii, Heroes!</wave><wait 500> Did you have a\n"
        "fun time in this here dungeon?\n"
        "<shake>Hehehehehe!!!\n"
        "</shake><k>\n<p>\n"
        "<dynamic 3>The name is Shadoo!</dynamic><wait 350> I've been\n"
        "trapped here for 2,000 <wave>long,\n"
        "<wait 200>lo<wait 150>o<wait 150>o<wait 150>o<wait 150>o<wait 150>ng years...\n"
        "</wave><k>\n<p>\n"
        "Oh,<wait 350> won't you help me,<wait 200> Heroes?\n"
        "<wait 350>Indeed,<wait 200> allow me to reveal my\n"
        "form to you...\n" END;

    const char shadooIntroRamp[] =
        "<housou><wave>Welcome back, Heroes!</wave><wait 500> Looks\n"
        "like you've upped the ante,\n"
        "<wait 200>yes?<wait 400><shake> Hehehehe!!\n"
        "</shake><k>\n<p>\n"
        "<wave>I shall entertain you once\n"
        "more,<wait 200> great Heroes!\n"
        "</wave><wait 500><shake><dynamic 3>Here I cooooome!\n" END;

    const char shadooIntroMock[] =
        "<housou><wave>Welcome back, Heroes!</wave><wait 500> Oh my,\n"
        "<wait 200>how interesting...<wait 350> You've\n"
        "returned, <wait 200><wave>but this is no fun!\n"
        "</wave><k>\n<p>\n"
        "Fighting me on the same\n"
        "difficulty <wait 100><shake>twice?</shake><wait 500><wave> Are you trying\n"
        "to cheese me? <wait 500>How crude!\n"
        "</wave><k>\n<p>\n"
        "<wave>Oh,<wait 200> but I shall entertain you\n"
        "nevertheless,<wait 200> great Heroes!\n"
        "<wait 500><shake><dynamic 3>Here I cooooome!\n" END;

    const char shadooDoBattle[] =
        "<housou><shake>Let us do battle,<wait 300> accursed\n"
        "Heroes of Light!\n"
        "</shake><k>";

    const char shadooDefeated[] =
        "<housou><wave>Defeated by the prophesied\n"
        "Heroes of Light and the Pixls...\n"
        "<wait 600>How very shamefuuuul...\n"
        "</wave><k>\n<p>\n"
        "<shake>Yet I won't be sealed away by\n"
        "a battle of such low caliber!\n"
        "<wait 500></shake><dynamic 3>Visit me again soon, Heroes!\n" END;

    const char tippiDS[] =
        "<fairy>Shadoo...<wait 250> Who are they?\n"
        "<wait 400>Why do they want us to come\n"
        "back here...\n" END;

    const char peachDS[] =
        "This Shadoo wants us to come\n"
        "back?<wait 300> How mysterious...<wait 300> Just who\n"
        "are they?\n" END;

    const char bowserDS[] =
        "Huh?<wait 300> You want me to come\n"
        "back?<wait 300> Pffft,<wait 100> I'll kick your\n"
        "butt anytime you want!\n" END;

    const char luigiDS[] =
        "Come back?<wait 300> Does that mean we'll\n"
        "have to do all 100 floors\n"
        "over again?\n" BRK
        "<shake>I don't wanna...\n"
        "</shake><k>";

    const char tippiDSAgain[] =
        "<fairy>Well, we've defeated Shadoo\n"
        "again...<wait 250> What exactly are\n"
        "they?\n" BRK
        "It's like there's no end to\n"
        "this...<wait 250> Should we even keep\n"
        "coming back here?\n" END;

    const char tippiDS_Sus[] =
        "<fairy>Now that I think about it...\n"
        "<wait 250>Doesn't Shadoo seem weirdly\n"
        "familiar to you?\n" BRK
        "Maybe it's the way they talk...\n"
        "<wait 250>Ugh,<wait 100> I just can't remember...\n" END;

    const char hardShadooBadFeeling[] =
        INIT_SYS
        "You have an ominous feeling\n"
        "that something much worse\n"
        "was meant to happen.\n" BRK
        "An unshakeable sensation of\n"
        "impending doom rattles you\n"
        "to your core.\n" BRK
        "But such a terrible fate will\n"
        "not befall you...\n"
        "<wait 500><shake>Not yet, it won't.\n</shake>" BRK
        "<wave>The end is never the end.\n"
        "</wave><wait 500>Perhaps you should take\n"
        "some solace in that.\n" BRK
        "This is far from the end of\n"
        "this saga.<wait 500> Please look forward\n"
        "to future updates.\n" END;

    // Blessings and Curses

    const char spectreName[] =
        "Spectre";

    const char houraiName[] =
        "Hourai Doll";

    const char paramitaName[] =
        "Paramita";

    const char shionName[] =
        "Tatarian Aster";

    const char hexName[] =
        "Vulnerability Hex";

    const char migraineName[] =
        "Migraine";

    // Merluna & blessing/curse text
    const char houraiFirstUse[] =
        "<system>"
        "Hourai Doll has taken effect!\n"
        "HP has been fully restored,\n"
        "but the Blessing has cleared.\n" BRK
        "You now have an Affliction:\n"
        "Phoenix's Tail. Attack is\n"
        "halved; HP increased by 10.\n" BRK
        "Hourai Doll's effects are no\n"
        "longer active, so be careful!\n" BRK
        "These effects will remain\n"
        "until you exit the Pit or\n"
        "the Affliction has cleared.\n" END;

    const char houraiWearOff[] =
        "<system>"
        "The temporary HP boost and\n"
        "Attack cut from Phoenix's\n"
        "Tail have now worn off.\n" END;

    // TEMP

    const char IMISSMYBROTHER[] =
        "<housou>"
        "<center><wave>Ahhh...<wait 500> Now's not the time,</center>\n"
        "<center><wait 300>o great Hero!<wait 500> You've caught</center>\n"
        "<center>me 'lacking',<wait 200> as they say...</center>\n" BRK
        "<center>Oh, I truly envy you lot...</center>\n"
        "<center><wait 1000></wave><shake>You'll never know the same</center>\n"
        "<center>grief as I...<wait 500> Hah...</center>\n" BRK
        "<center></shake><wave>Well! We shall meet again</center>\n"
        "<center>in another time and place!</center>\n"
        "<center></wave><shake><dkey><wait 500>Wah-<wait 200>hah-<wait 200>haaaaah!</center>\n<wait 1000></dkey>" END;

    // END TEMP

    const char merlunaIntro[] =
        "<shake>Wah-hah-hah!</shake><wait 500> You've found\n"
        "me, the oh-so-wonderful...<wait 250> the\n"
        "dearly accursed... <wait 250><wave>Merluna!</wave>\n" END;

    const char merlunaIntro2[] =
        "What am I doing here, you ask\n"
        "me?<wait 250> Well, now, aren't you an\n"
        "<shake>inquisitive</shake> one, indeed...\n" BRK
        "If you must know, I am in\n"
        "the business of <wave>distributing\n"
        "the Ancients' unyielding will!\n"
        "</wave>\n<k>\n<p>\n"
        "\"What does she mean, truly?\"<wait 500> \n"
        "Well, if you'd like, I <wave>cooould\n"
        "</wave>show you my great powers...\n"
        "<dkey></dkey><k>\n<p>\n"
        "<wait 1500>...So I will!<wait 500> For a nominal\n"
        "fee, I shall render Divine\n"
        "Judgement upon your person!\n" BRK
        "Worry not;<wait 250> quite often, fortune\n"
        "may very well be in your\n"
        "favor...<wait 750> it's all up to chance!\n" BRK
        "You have a chance to be\n"
        "<wave>blessed,</wave><wait 500> but you could also\n"
        "be <shake>cursed...\n"
        "</shake><k>\n<p>\n"
        "<wave>But dearest Merluna,<wait 250> how likely\n"
        "is a curse?</wave><wait 400> That depends on\n"
        "your Difficulty setting!\n"
        "</wave><k>\n<p>\n"
        "If you do get cursed,<wait 250> you can\n"
        "always come back to me<wait 100> and\n"
        "I'll make it <wave>allll better...\n"
        "</wave><k>\n<p>\n"
        "You have a %d percent chance\n"
        "to be blessed, and a %d percent\n"
        "chance to be cursed!\n" BRK
        "Now, allow me to divine the\n"
        "latest in the Tribe's bidding\n"
        "for you,<wait 500> <wave>o great Hero...\n"
        "</wave><k>";

    const char merlunaIntro3[] =
        "Hah-hah,<wait 300> so you've come back\n"
        "for me, <wave>haven't you, Hero?\n"
        "<wait 700></wave><shake>Well, then...\n"
        "</shake><k>\n<p>\n"
        "You have a %d percent chance\n"
        "to be blessed, and a %d percent\n"
        "chance to be cursed!\n" BRK
        "You know how the rest goes,<wait 300> so\n"
        "I shall spare you the lecture\n"
        "and hear the Ancients' will...\n" END;

    const char merlunaJudgement[] =
        "The Blessing is %s;\n"
        "<wait 300>the Curse is %s.\n"
        "<wait 700><wave>Just 10 coins...<wait 500> wanna risk it?\n"
        "</wave><o>";

    const char merlunaOptions[] =
        "<select 3 -1 360 55>\n"
        "Let's do it!\n"
        "More about Blessing...\n"
        "More about Curse...\n"
        "No, thanks.";

    const char merlunaDecline[] =
        "<wave>Okay, okay, okayyyy... </wave><wait 600><shake>Well,\n"
        "</shake><wait 300>if you change your mind,\n"
        "<wait 300><wave>I'll always be heeeere!\n"
        "</wave><k>";

    const char merlunaBless[] =
        "<wave>Yay!</wave><wait 300> You've just earned the\n"
        "blessing of the Tribe!\n"
        "<wait 500><wave>Enjoy it while you can!\n"
        "</wave><k>";

    const char merlunaCurse[] =
        "<wave>Oh no!</wave><wait 300> Looks like you weren't\n"
        "so lucky...<wait 300> What a shame...\n"
        "<wait 700><shake>Well, good luck now, Hero!\n"
        "</shake><k>";

    const char merlunaScam[] =
        "<shake><dynamic 3>Ugh!!!</shake><wait 500> Have you no respect\n"
        "for the powers of my tribe!?\n"
        "<wait 500>You can't afford me!\n"
        "</dynamic></shake><k>\n<p>\n"
        "<shake>Honestly, </shake><wait 300> some people just\n"
        "have NO respect for us...\n"
        "<wait 700><dynamic 3><shake>DON'T WASTE MY TIME!\n"
        "</dynamic></shake><k>";

    const char merlunaRelieveCurse[] =
        "<shake>Wah-hah-hah,</shake><wait 300> hello, Hero...\n"
        "<wave><wait 800>So, have you been enjoying\n"
        "yourself? </wave><wait 700><shake>Hehehe...\n"
        "</shake><k>\n<p>\n"
        "Well, <wait 300>that's just how it goes!\n"
        "<wait 700>Sometimes,<wait 300> you get lucky,\n"
        "<wait 300>and other times...<wait 800> <wave>You don't!\n"
        "</wave><k>\n<p>\n"
        "I'm just <shake>sure</shake> you've had enough\n"
        "by now, <wait 300>haven't you? <wait 700><wave>But worry\n"
        "not,<wait 200> I can take care of you...\n"
        "</wave><k>\n<p>\n"
        "<wait 1000>For the measly price of... <wait 500><wave>100\n"
        "coins!</wave><wait 500> Surely you can afford\n"
        "that...<wait 700> <wave>Wanna clear the curse?\n"
        "</wave><o>";

    const char merlunaRelieveOptions[] =
        "<select 0 -1 400 40>\n"
        "Yes, please, ma'am!\n"
        "I rather like this curse.";

    const char merlunaRelieveScam[] =
        "<shake><dynamic 3>Unbelievable!</shake><wait 500> You incurred the\n"
        "wrath of my tribe and didn't\n"
        "even prepare an offering!?\n"
        "</dynamic></shake><k>\n<p>\n"
        "<shake>Honestly, </shake><wait 300> some people just\n"
        "have NO respect for us...\n"
        "<wait 700><dynamic 3><shake>DON'T WASTE MY TIME!\n"
        "</dynamic></shake><k>";

    const char merlunaRelieveDecline[] =
        "<wait 1000><wave>...<wait 1400>Well, aren't you strange?\n"
        "</wave><wait 500>Are you what they call,<wait 200> mm,\n"
        "<wait 200>a <shake>\"masochist?\"\n"
        "</shake><k>\n<p>\n"
        "I must say,<wait 200> I don't <wait 100><shake>quite\n"
        "<wait 100></shake>understand your tastes,<wait 200> <wave>but\n"
        "I hope you have fun!\n"
        "</wave><k>";

    const char merlunaRelieveReal[] =
        "<wave>All done!</wave><wait 500> Your curse has been\n"
        "cleared!<wait 200> <shake>Now...</shake><wait 200> <wave>would you like\n"
        "to retry Divine Judgement?\n"
        "</wave><o>";

    const char merlunaRetryOptions[] =
        "<select 0 -1 400 30>\n"
        "I won't lose twice!\n"
        "Maybe not...";

    const char merlunaRetryAccept[] =
        "Hehehe,<wait 200> well,<wait 200> <wave>aren't you a\n"
        "brave little Hero?</wave><wait 800> So be it...\n"
        "<wait 200><shake>Here we go!\n"
        "</shake><k>";

    const char merlunaRetryDecline[] =
        "<wave>Hehehe, I understand... </wave><wait 600><shake>Well,\n"
        "</shake><wait 300>if you change your mind,\n"
        "<wait 300><wave>I'll always be heeeere!\n"
        "</wave><k>";

    const char merlunaSpectre[] =
        "The Blessing known as Spectre\n"
        "<wait 100>is<wait 100> <wave>oh-so very convenient!\n"
        "</wave><wait 200>You'll <shake>certainly</shake> love it...\n" BRK
        "Each enemy you defeat will\n"
        "have a 20 percent chance to\n"
        "heal you a certain amount!\n" BRK
        "The HP obtained depends on\n"
        "the enemy's HP as well as\n"
        "your progress in the Pit.\n" BRK
        "<wave>Doesn't that sound great?\n"
        "<wait 700>It<wait 100> could<wait 100> be<wait 100> yoooours...\n"
        "<wait 100></wave><k>";

    const char merlunaAster[] =
        "The Curse,<wait 200> Tatarian Aster,\n"
        "<wait 200>is simply <shake>terrible...</shake><wait 700> You\n"
        "certainly don't want it!\n" BRK
        "Known to some in my tribe as\n"
        "\"Shion,\"<wait 300> it will ensure you have\n"
        "terrible luck...<wait 300> <shake>FOR LIFE!\n"
        "</shake><k>\n<p>\n"
        "In the Pit,<wait 200> you'll have a 30\n"
        "percent chance to be forced\n"
        "to defeat <dynamic 2>every enemy.\n"
        "</dynamic><k>\n<p>\n"
        "Although tedious early on,<wait 200> it\n"
        "can become dangerous in later\n"
        "floors,<wait 200> <wave>so be careful!\n"
        "</wave><k>\n<p>\n"
        "Of course,<wait 200> if you get cursed,\n"
        "<wait 200>I can always make it better\n"
        "for you...<wait 600> <shake>Hehehe...\n"
        "</shake><k>";

    const char merlunaHex[] =
        "The Curse,<wait 200> Vulnerability Hex,\n"
        "<wait 200>will cause you to take more\n"
        "damage from all sources.\n" BRK
        "Historically,<wait 200> this curse was used\n"
        "to weaken our enemies and\n"
        "defeat them swiftly...\n" BRK
        "It's not so terrible early on,\n"
        "<wait 200><wave>but you'll take increased\n"
        "damage in later floors...\n"
        "</wave><k>\n<p>\n"
        "You'll take 1 extra damage in\n"
        "Floors 1-50,<wait 200> 2 extra through 75,\n"
        "<wait 200>and 3 extra after that!\n" BRK
        "Of course,<wait 200> if you get cursed,\n"
        "<wait 200>I can always make it better\n"
        "for you...<wait 600> <shake>Hehehe...\n"
        "</shake><k>";

    const char merlunaHourai[] =
        "The Blessing,<wait 200> Hourai Doll,<wait 200> allows\n"
        "the user to return from the\n"
        "dead,<wait 200> <wave>fully revitalized!\n" BRK
        "Weeellll....</wave><wait 400> maybe that's not\n"
        "entirely true...\n" BRK
        "If you run out of HP with this\n"
        "Blessing active,<wait 200> you'll revive\n"
        "at full HP,<wait 200> <wave>and then some!\n"
        "</wave><k>\n<p>\n"
        "After Hourai Doll takes effect,\n"
        "<wait 200>the Blessing will end,<wait 200> and you'll\n"
        "be afflicted by Phoenix's Tail.\n" BRK
        "Phoenix's Tail increases your max\n"
        "HP by 10,<wait 200> <shake>but halves your attack\n"
        "power!<wait 400></shake> <wave>How dangerous...\n"
        "</wave><k>\n<p>\n"
        "Do know that I cannot remove\n"
        "Phoenix's Tail...<wait 200> <wave>You'll have to\n"
        "put up with it for 20 floors!\n"
        "</wave><k>\n<p>\n"
        "<wave>Doesn't that sound great?\n"
        "<wait 700>It<wait 100> could<wait 100> be<wait 100> yoooours...\n"
        "<wait 100></wave><k>";

    const char merlunaParamita[] =
        "The Blessing,<wait 200> Paramita,<wait 200> is\n"
        "incredibly powerful,<wait 200> but\n"
        "<wave>oh-so-very costly!\n"
        "</wave><k>\n<p>\n"
        "At will,<wait 200> you can multiply your\n"
        "attack power...<wait 400> but it'll take\n"
        "<shake>20 percent</shake> of your current HP!\n" BRK
        "Paramita will multiply attack\n"
        "by two in Floors 1-50,<wait 200> by three\n"
        "in 51-75,<wait 200> and by four in 75-99!\n" BRK
        "This buff will last for twenty\n"
        "seconds,<wait 200> with an equally long\n"
        "cooldown after it ends.\n" BRK
        "<wave>Doesn't that sound great?\n"
        "<wait 700>It<wait 100> could<wait 100> be<wait 100> yoooours...\n"
        "<wait 100></wave><k>";

    const char merlunaMigraine[] =
        "The Curse,<wait 200> Migraine,<wait 200> is <wave>soooooo\n"
        "very annoying!</wave><wait 400> You'll be\n"
        "frustrated in no time flat!\n" BRK
        "When you are hit by an enemy,\n"
        "<wait 200>you will have a 75 percent\n"
        "chance to be <shake>debuffed...\n"
        "</shake><k>\n<p>\n"
        "<wave>You might forget how to walk,\n"
        "or you might forget how to use\n"
        "your abilities...\n" BRK
        "...but sometimes,<wait 200> you might\n"
        "become as slow as a snail,<wait 200> or\n"
        "as heavy as a boulder!\n"
        "</wave><k>\n<p>\n"
        "The amount of time the debuff\n"
        "lasts is <shake>random!</shake><wait 300> It can last\n"
        "up to one to ten seconds...\n" BRK
        "Of course,<wait 200> if you get cursed,\n"
        "<wait 200>I can always make it better\n"
        "for you...<wait 600> <shake>Hehehe...\n"
        "</shake><k>";

    // Mover
    const char moverIntro[] =
        "Hey.<wait 250> Wassup?<wait 250> I'm a Mover.\n"
        "I know a few things about\n"
        "getting through this dungeon.\n" BRK
        "For a few coins, I can move\n"
        "you where you wanna go.\n"
        "<wait 250>Where do you wanna go?\n" PROMPT;

    const char moverChoices[] =
        "<select 2 -1 400 40>\n"
        "%d Coins: Down 2 Floors\n"
        "%d Coins: Down 5 Floors\n"
        "Nah, I'm fine.";

    const char moverNo[] =
        "Whatever, man. Have it your\n"
        "way. It's not like I do this\n"
        "FOR A LIVING or anything!\n" END;

    const char moverScam[] =
        "<wave>Maaan,</wave> you don't have the cash\n"
        "money! How about you talk\n"
        "to me when you can PAY UP?\n" END;

    const char moverYes[] =
        "<wave>Alriiiight!</wave> Appreciate the\n"
        "business, man. Let's getcha\n"
        "outta here. <wave>Close your eyes!</wave>\n" END;

    // Quickstart
    const char quickstartOptions[] =
        "<select 0 -1 360 40>\n"
        "New Save\n"
        "Start from 2-1\n"
        "Start from 6-1\n"
        "Start from Postgame";

    const char quickstartText[] =
        "<system>"
        "Do you want to quickstart\n"
        "the Lunatic Pit or create\n"
        "a new save file?\n" PROMPT;

    // Whacka

    const char whackaHi[] =
        "Whaaack-hoo! <wait 200>I'm Whacka!\n"
        "<wait 250>Didn't expect to see someone\n"
        "else down here, <wait 200>whack-a-do!\n" BRK
        "It's soooo quiet in this room,\n"
        "<wait 200><wave>whack-a-hooo!</wave><wait 300> And this\n"
        "is MY spot, whack-doo!\n" BRK
        "It suuuuure looked dangerous\n"
        "in those other rooms!<wait 200> You\n"
        "should be careful, whack-roo!\n" END;

    const char whackaHiBlueLong1[] =
        "Whack-bluuuuue!<wait 250> Check it out!\n"
        "<wait 150>I'm blue now,<wait 100> whack-a-yoooo!\n"
        "<wait 150>Never felt better,<wait 100> whack-roo!\n" END;

    const char whackaHiBlueLong2[] =
        "Whack-hoo?<wait 250> Why am I blue?\n"
        "<wait 500>Well.......<wait 1200> <wave>That's a secret,\n"
        "<wait 300>whack-a-bluuuuuuuue!\n"
        "</wave><k>\n<p>\n"
        "I bet you want to be blue,\n"
        "<wait 100>too,<wait 100> don't you?<wait 300> I can help,\n"
        "<wait 200>whack-a-yooo!\n" BRK
        "But you have to do me a big\n"
        "favor,<wait 100> yep,<wait 100> whack-a-hoo!\n"
        "<k>\n<wait 500><p>\n"
        "Give me my bump back!<wait 250> Yeah,\n"
        "<wait 100>the one you whacked right\n"
        "out of me,<wait 200> whack-a-do!\n" BRK
        "If you give it back to me,\n"
        "<wait 200>you can be blue!<wait 250> <wave>That's a\n"
        "promise,<wait 150> whack-bluuuue!\n"
        "</wave><k>";

    const char whackaHmph[] =
        "You don't wanna give me my\n"
        "bump back?<wait 250> Hmph, fine!<wait 250> <wave>No\n"
        "blue for you,<wait 100> whackety-hoo!\n"
        "</wave><k>";

    const char whackaThanks[] =
        "<wave>Yay!</wave><wait 250> Thanks for giving me\n"
        "my bump back,<wait 100> whackety-woo!\n"
        "<wait 250><wave>I'm so happy,<wait 100> whack-a-rooo!\n"
        "</wave><k>\n<p>\n"
        "As promised,<wait 100> I'll help make\n"
        "you blue,<wait 100> but you'll have to\n"
        "wait,<wait 100> whackety-hoo!\n" BRK
        "Give me about 15 seconds to\n"
        "make your present!<wait 250> Be right\n"
        "back,<wait 100> whackety-yack-a-dooo!\n" END;

    const char whackaGiveBump1[] =
        "Hoooo...<wait 250> I'm back!<wait 200> Did you\n"
        "miss me?<wait 200> I hope that wasn't\n"
        "too loud,<wait 100> whack-hoooo...\n" BRK
        "Here you go!<wait 250> It's your present,\n"
        "<wait 100>especially from me to you,\n"
        "<wait 100><wave>whack-a-bluuuuue!\n"
        "</wave><k>";

    const char whackaGiveBump2[] =
        "You can use this to become\n"
        "blue anytime!<wait 250> It can suck\n"
        "all the blue back out,<wait 100> too!\n" BRK
        "Think of me every time you\n"
        "go blue,<wait 100> whack-a-bluuuue!\n"
        "<wait 250>Stay safe,<wait 100> friend!\n" END;

    const char whackaHiBlueShort[] =
        "Give me my bump back!<wait 250> Yeah,\n"
        "<wait 100>the one you whacked right\n"
        "out of me,<wait 200> whack-a-do!\n" BRK
        "If you give it back to me,\n"
        "<wait 200>you can be blue!<wait 250> <wave>That's a\n"
        "promise,<wait 150> whack-bluuuue!\n"
        "</wave><k>";

    const char whackaDontHaveBump[] =
        "...You don't have my bump?\n"
        "<wait 200>What did you do with it?\n"
        "<wait 500>You didn't eat it,<wait 100> did you?\n" BRK
        "Well,<wait 100> then you don't get to\n"
        "be blue like me!<wait 250> <wave>What a shame,\n"
        "whack-a-bluuuuue!"
        "</wave><k>";

    const char tippiBlueWhacka[] =
        "<fairy>...<wait 500>Weren't you already blue\n"
        "before?<wait 250> How did you become\n"
        "even more blue...?\n" END;

    const char peachBlueWhacka[] =
        "I thought you were already\n"
        "blue,<wait 100> though?<wait 250> How'd you get\n"
        "to be more blue?\n" END;

    const char bowserBlueWhacka[] =
        "What's your problem,<wait 100> you\n"
        "weird mole?<wait 250> Why are you so\n"
        "darn blue?\n" END;

    const char luigiBlueWhacka[] =
        "Weren't you already blue,\n"
        "little Whacka?<wait 250> I hope I'm\n"
        "not misremembering...\n" END;

    const char whackaHiBluePost[] =
        "Whack-a-bluuuue! It sure is\n"
        "nice being blue, whack-hoo!\n" BRK
        "Do you like being blue?\n"
        "I sure hope so, yahooo!\n" END;

    const char whackaOww[] =
        "<wave>Whack-OWWWWWWWWWWW!\n"
        "</wave><wait 250>That really hurt!<wait 200> I'm going\n"
        "somewhere else now...\n" END;

    const char whackaCantHurt[] =
        "<wave>Whack-BLUUUUUUUE!\n"
        "</wave><wait 250>You can't hurt me anymore,\n"
        "<wait 200>because I'm blue!<wait 250> Whack-hoo!\n" END;

    const char tippiHitWhacka[] =
        "<fairy>I don't think being extra\n"
        "blue necessarily makes one\n"
        "invincible...\n" END;

    const char peachHitWhacka[] =
        "Are you sure?<wait 200> I can't imagine\n"
        "being more blue should make\n"
        "you more powerful...\n" END;

    const char bowserHitWhacka[] =
        "Being blue doesn't make you\n"
        "invulnerable, you little freak!\n"
        "<wait 250>What's your deal?\n" END;

    const char luigiHitWhacka[] =
        "Huh?<wait 250> So being all blue and\n"
        "stuff makes you stronger?\n"
        "<wait 250>I'm not so sure...\n" END;

    const char whackaCantHurt2[] =
        "Really?<wait 250> Well, I sure do <wave>feel\n"
        "</wave>a whole lot stronger now,\n"
        "<wait 200>wicky-whackety-whack-a-do!\n" END;

    const char mystBumpDescPtr[] =
        "msg_blue_bump_desc";

    const char mystBumpNamePtr[] =
        "msg_blue_bump_name";

    const char mystBumpDesc[] =
        "A weird bump with an odd ability.\n"
        "Given to you by a \"blue\" Whacka.";

    const char mystBumpName[] =
        "Mysterious Whacka Bump";

    const char bumpUseMsg[] =
        "<system>"
        "Would you like to be blue?\n" PROMPT;

    const char bumpUnuseMsg[] =
        "<system>"
        "Would you like to stop\n"
        "being blue?\n" PROMPT;

    const char bumpYoureBlueNow[] =
        "<system>\n* You're blue now.\n" END;

    const char bumpYoureNotBlueNow[] =
        "<system>\n* You're no longer blue.\n" END;

    // Interface

    const char activeConditionsNone[] =
        INIT_SYS
        "There are no active\n"
        "effects to view right now!\n" END;

    const char interfaceCM1[] =
        "CM";

    const char interfaceCM2[] =
        "Crit Mult";

    const char interfaceCR1[] =
        "CR";

    const char interfaceCR2[] =
        "Crit Rate";

    const char interfaceDEF1[] =
        "DEF";

    const char interfaceDEF2[] =
        "Defense";

    const char interfaceDR1[] =
        "DR";

    const char interfaceDR2[] =
        "Dmg Reduction";

    // Disorders

    const char disorderIntro[] =
        INIT_SYS
        "You've been afflicted with\n"
        "a Disorder!\n" BRK
        "Disorders have varying\n"
        "negative effects that always\n"
        "last for five rooms.\n" BRK
        "You can learn more about an\n"
        "active Disorder anytime in the\n"
        "<icon PAD_1 0.67 0 0 0> Active Effects menu.\n" END;

    const char apathyName[] =
        "Apathy";

    const char dreadName[] =
        "Dread";

    const char prejudiceName[] =
        "Prejudice";

    const char indifferenceName[] =
        "Indifference";

    const char recalcitranceName[] =
        "Recalcitrance";

    const char depravityName[] =
        "Depravity";

    const char indolenceName[] =
        "Indolence";

    const char melancholyName[] =
        "Melancholy";

    const char ruinName[] =
        "Ruin";

    const char apathyDesc[] =
        "Disorder: Apathy\n"
        "\"This is taking way too long...\"\n" BRK
        "Decreases max HP by %d%%.\n"
        "Increases enemy HP by %d%%.\n" BRK
        "Increases damage taken by %d.\n"
        "Decreases damage dealt by %d.\n" BRK
        "Halves Crit Rate.\n"
        "Halves Crit Mult.\n" BRK
        "All stats are restored when\n"
        "the Disorder ends.";

    const char dreadDesc[] =
        "Disorder: Dread\n"
        "\"I'm never getting out of here...\"\n" BRK
        "Enemies drop no coins or items.\n"
        "XP gain from enemies is disabled.\n" BRK
        "%d%% chance for enemies to\n"
        "heal instead of taking damage.\n" BRK
        "Enemy healing is silent and\n"
        "based on your Attack.";

    const char prejudiceDesc[] =
        "Disorder: Prejudice\n"
        "\"Wretched, audacious beasts...\"\n" BRK
        "Instantly lose about %d%% of all\n"
        "coins when entering a room.\n" BRK
        "%d%% chance to lose a coin when\n"
        "the timer counts down.\n" BRK
        "If coins drop below %d, halves\n"
        "damage dealt to enemies.";

    const char indifferenceDesc[] =
        "Disorder: Indifference\n"
        "\"What a waste of energy...\"\n" BRK
        "A random junk item is added\n"
        "to the inventory in all rooms.\n" BRK
        "If there is no space, a random\n"
        "item will be replaced.\n" BRK
        "67%% chance to receive another\n"
        "item %d time(s).\n" BRK
        "Every room will have a\n"
        "complex layout.";

    const char recalcitranceDesc[] =
        "Disorder: Recalcitrance\n"
        "\"I will make them pay tenfold...\"\n" BRK
        "Enemies defeated with jumps\n"
        "will give %d%% XP.\n" BRK
        "Instant healing items will be\n"
        "%d%% less effective.";

    const char depravityDesc[] =
        "Disorder: Depravity\n"
        "\"Torture of the highest degree...\"\n" BRK
        "A very difficult enemy will spawn\n"
        "plentifully in each room.\n" BRK
        "All enemies are guaranteed to\n"
        "spawn at very high rates.\n" BRK
        "All enemies will be very\n"
        "difficult after Floor %d.";

    const char indolenceDesc[] =
        "Disorder: Indolence\n"
        "\"Guess I'll just stay put...\"\n" BRK
        "Upon taking damage, %d%% chance\n"
        "to receive a status effect.\n" BRK
        "You may be Frozen or\n"
        "Slowed for %d seconds.\n" BRK
        "Additionally, the entire world\n"
        "slows down by 18%%.";

    const char melancholyDesc[] =
        "Disorder: Melancholy\n"
        "\"I'm so over this...\"\n" BRK
        "Gain 1 stack of Atrophy every\n"
        "10 seconds spent in one room.\n" BRK
        "Atrophy increases damage taken\n"
        "by 1 to the power of # stacks.\n" BRK
        "Atrophy stacks up to 5 times\n"
        "or until damage is taken.\n" BRK
        "All Voucher tear chances are\n"
        "doubled.";

    const char ruinDesc[] =
        "Disorder: Ruin\n"
        "\"Please... turn the lights back on...\"\n" BRK
        "Placeholder\n";

    const char apathyIntro[] =
        "<system>A wave of uncertainty strikes\n"
        "you,<wait 200> leading you to question\n"
        "your every ability.\n" BRK
        "As you doubt yourself,<wait 200> you\n"
        "begin to forget why you\n"
        "ever chose to be a Hero.\n" BRK
        "Gradually,<wait 200> you find yourself\n"
        "losing your motivation to\n"
        "forge on.\n" BRK
        "You're now bound by pure,\n"
        "<wait 200>unfettered <se_on SFX_EVT_SHOCK1><shake>apathy.\n"
        "</shake><k>\n<p>\n"
        "Your HP, CM, and CR have\n"
        "been decreased;<wait 200> enemy HP\n"
        "has been raised.\n" BRK
        "Damage taken is increased;\n"
        "<wait 200>damage dealt is decreased.\n" END;

    const char apathyIntro2[] =
        "<system><se_on SFX_EVT_SHOCK1><shake>Apathy:</shake><wait 500> Stats and DMG dealt\n"
        "are decreased; enemies have\n"
        "more HP and deal more DMG.\n" END;

    const char dreadIntro[] =
        "<system>A wave of terror strikes you,\n"
        "<wait 200>slowly draining you of all\n"
        "the hopes you ever had.\n" BRK
        "Every step you take sends\n"
        "unbearable chills down your\n"
        "spine.\n" BRK
        "You're now bound by pure,\n"
        "<wait 200>unfettered <se_on SFX_EVT_SHOCK1><shake>dread.\n"
        "</shake><k>\n<p>\n"
        "Enemies will no longer drop\n"
        "coins, items, or XP.\n" BRK
        "Additionally, enemies may\n"
        "silently heal instead of\n"
        "taking damage.\n" END;

    const char dreadIntro2[] =
        "<system><se_on SFX_EVT_SHOCK1><shake>Dread:</shake><wait 500> Enemies have no\n"
        "loot and may heal instead\n"
        "of taking DMG.\n" END;

    const char prejudiceIntro[] =
        "<system>A wave of contempt strikes\n"
        "you,<wait 200> making you wonder why\n"
        "you're bothering with this.\n" BRK
        "You bemoan the tedium of\n"
        "this Pit and the inferior\n"
        "enemies before you...\n" BRK
        "...<wait 200>yet you sense that the\n"
        "one watching you may feel\n"
        "that very way towards you.\n" BRK
        "You're now bound by pure,\n"
        "<wait 200>unfettered <se_on SFX_EVT_SHOCK1><shake>prejudice.\n"
        "</shake><k>\n<p>\n"
        "Coins will be lost upon\n"
        "entering a room and as\n"
        "time passes.\n" BRK
        "ATK will be nerfed if you\n"
        "lose too many coins.\n" END;

    const char prejudiceIntro2[] =
        "<system><se_on SFX_EVT_SHOCK1><shake>Prejudice:</shake><wait 500> Coins are drained\n"
        "constantly; ATK decreases\n"
        "if you lose too many.\n" END;

    const char indifferenceIntro[] =
        "<system>A wave of numbness strikes\n"
        "you,<wait 200> leaving your mind\n"
        "entirely vacant.\n" BRK
        "Suddenly,<wait 200> you feel detached\n"
        "from everything and\n"
        "everyone around you.\n" BRK
        "You feel your energy being\n"
        "sapped away as your grip on\n"
        "reality rapidly fades.\n" BRK
        "You're now bound by pure,\n"
        "<wait 200>unfettered <se_on SFX_EVT_SHOCK1><shake>indifference.\n"
        "</shake><k>\n<p>\n"
        "Some junk items will be\n"
        "forced into your inventory\n"
        "in every room.\n" BRK
        "If you don't have enough\n"
        "space for all of the items,\n"
        "<wait 200>space will be made.\n" END;

    const char indifferenceIntro2[] =
        "<system><se_on SFX_EVT_SHOCK1><shake>Indifference:</shake><wait 500> Receive some\n"
        "junk items in every room.\n"
        "Make sure to have space!\n" END;

    const char recalcitranceIntro[] =
        "<system>A wave of unease strikes\n"
        "you,<wait 200> overwhelming you with\n"
        "anxiety.\n" BRK
        "You have become infinitely\n"
        "more aware of how finite\n"
        "this space is.\n" BRK
        "A singular thought rings\n"
        "repeatedly in your head:\n"
        "<wait 500>\"find a way out of here.\"\n" BRK
        "Rationally, you know that\n"
        "isn't so hard,<wait 200> yet it bothers\n"
        "you all the same.\n" BRK
        "You feel an unquellable urge\n"
        "to escape, no matter what\n"
        "may try to stop you.\n" BRK
        "You're now bound by pure,\n"
        "<wait 200>unfettered <se_on SFX_EVT_SHOCK1><shake>recalcitrance.\n"
        "</shake><k>\n<p>\n"
        "XP gained from stomping\n"
        "or kicking will reduce\n"
        "your score.\n" BRK
        "Additionally, instant healing\n"
        "items are temporarily less\n"
        "effective.\n" END;

    const char recalcitranceIntro2[] =
        "<system><se_on SFX_EVT_SHOCK1><shake>Recalcitrance:</shake><wait 500> Reduced XP from\n"
        "stomps/kicks. Healing items\n"
        "are less effective.\n" END;

    const char depravityIntro[] =
        "<system>A wave of perversion strikes\n"
        "you,<wait 200> totally derailing your\n"
        "train of thought.\n" BRK
        "Numbness is now iniquity;\n"
        "<wait 200>you've come to revel in\n"
        "these trials.\n" BRK
        "As if to grant your newfound\n"
        "desire,<wait 200> countless foes have\n"
        "come to block your path.\n" BRK
        "The abyss stares back into\n"
        "you,<wait 200> rotting your soul like\n"
        "a terminal cancer.\n" BRK
        "You're now bound by pure,\n"
        "<wait 200>unfettered <se_on SFX_EVT_SHOCK1><shake>depravity.\n"
        "</shake><k>\n<p>\n"
        "Enemy spawn rates have\n"
        "significantly increased.\n" BRK
        "Difficult enemies are now\n"
        "guaranteed to spawn in\n"
        "every floor.\n" END;

    const char depravityIntro2[] =
        "<system><se_on SFX_EVT_SHOCK1><shake>Depravity:</shake><wait 500> Increased enemy\n"
        "spawn rates and difficult\n"
        "enemies in every floor.\n" END;

    const char indolenceIntro[] =
        "<system>A wave of lethargy strikes\n"
        "you, <wait 200>making you feel an order\n"
        "of magnitude heavier.\n" BRK
        "Ngl bro I don't really want\n"
        "to finish this blurb rn<wait 2000> I\n"
        "got purple disorder irl\n" BRK
        "You're now bound by pure,\n"
        "<wait 200>unfettered <se_on SFX_EVT_SHOCK1><shake>indolence.\n"
        "</shake><k>\n<p>\n"
        "Getting hit may result in\n"
        "being frozen or slowed down\n"
        "for some time.\n" BRK
        "Additionally, the world\n"
        "slows down slightly while\n"
        "the Disorder is active.\n" END;

    const char indolenceIntro2[] =
        "<system><se_on SFX_EVT_SHOCK1><shake>Indolence:</shake><wait 500> Getting hit can\n"
        "trigger Slow or Frozen.\n"
        "<wait 1000><wave>Eeeverything is slooower.\n"
        "</wave><k>";

    const char melancholyIntro[] =
        "<system>A wave of misery strikes\n"
        "you, <wait 200>doing this or that\n"
        "man idk\n" BRK
        "I'll do this later lol\n" BRK
        "You're now bound by pure,\n"
        "<wait 200>unfettered <se_on SFX_EVT_SHOCK1><shake>Melancholy.\n"
        "</shake><k>\n<p>\n"
        "Every 10 seconds, you will\n"
        "be marked with 1 stack\n"
        "of <wave>Atrophy.\n"
        "</wave><k>\n<p>\n"
        "<wave>Atrophy</wave> increases damage\n"
        "taken based on the number\n"
        "of stacks, up to 5 stacks.\n" BRK
        "<wave>Atrophy</wave> <dynamic 3>exponentially</dynamic> stacks\n"
        "damage; don't lose track of it!\n"
        "(+1, +2, +4, +8, +16)\n" BRK
        "Stacks and internal timer will\n"
        "reset when damage is taken\n"
        "or when the Disorder ends.\n" BRK
        "Additionally, all Vouchers\n"
        "are twice as likely to tear.\n" END;

    const char disorderIndifferenceItemNotif[] =
        "<system><center>Your inventory is full!</center>\n" BRK
        "<center>%s has been</center>\n"
        "<center>replaced with</center>\n"
        "<center>%s!</center>\n" PROMPT;

    const char disorderIndifferenceItemNotif2[] =
        "\n<p><system>\n"
        "<center>%s has been</center>\n"
        "<center>replaced with</center>\n"
        "<center>%s!</center>\n" PROMPT;

    // Jimbo
    const char jimboIntro[] =
        "Sup.<wait 300> I'm Jimbo.<wait 200> Nice to meet\n"
        "ya.<wait 300> I sorta guard the entrance\n"
        "to this here Pit.\n" BRK
        "Yeah, I'm kind of a big deal.\n"
        "<wait 300>Warnin' the people of the\n"
        "perils that lie in wait...\n" BRK
        "Yep, that's just what I do.\n"
        "<wait 300>No one even asks me to do\n"
        "this.<wait 300> I'm just <wave>that cool, yo.\n"
        "</wave><k>\n<p>\n"
        "Anyway, you don't look like\n"
        "just any ordinary citizen, so\n"
        "I can let ya go down there.\n" BRK
        "Matter of fact, I'll even help\n"
        "ya out if ya want.<wait 300> Yep, I can\n"
        "do that.\n" BRK
        "Well, hey, I won't go IN there\n"
        "with ya...<wait 300> though I would totally\n"
        "survive, yo...\n"
        "<k><wait 1000>\n<p>\n"
        "...<wait 1000>Uh, anyway, I can help ya\n"
        "with a few things. Preferences.\n"
        "Options, if ya will.<wait 300> Yo.\n" BRK
        "Y'know what they called me\n"
        "back at Shy High?<wait 300> \"The Options\n"
        "Menu.\"<wait 300> Yeah. That's right, yo.\n" BRK
        "I'm not lyin'.<wait 300> Ya went to a\n"
        "different school, so ya wouldn't\n"
        "know.<wait 300> C'mon, seriously, yo.\n" BRK
        "I can totally change some parts\n"
        "of your experience in the Pit.\n"
        "<wait 300>Don't ask how.<wait 300> <wave>I just can.\n"
        "</wave><k>\n<p>\n"
        "So, with allat out of the way,\n"
        "what can I do ya for?\n" PROMPT;

    const char jimboIntro2[] =
        "Sup.<wait 300> What can I do ya for?\n" PROMPT;

    const char jimboWarn[] =
        CONT
        "Whoa.<wait 300> Hoooold up.<wait 300> Ya haven't\n"
        "beaten the Pit yet, so I\n"
        "gotta warn ya...\n" BRK
        "Openin' this particular menu\n"
        "might spoil the experience for\n"
        "ya.<wait 300> Sure ya wanna proceed?\n" PROMPT;

    const char jimboAnythingElse2[] =
        CONT
        "Need anything else?\n" PROMPT;

    const char jimboAnythingElse[] =
        "Need anything else?\n" PROMPT;

    const char jimboBye[] =
        CONT
        "Bet. Catch ya later, yo.\n" END;

    const char jimboSettings[] =
        "<select 0 -1 280 40>\n"
        "Features\n"
        "Game Patches\n"
        "Accessibility\n"
        "I'm all good, yo.";

    // Pit Boodin
    const char boodinIntro[] =
        "Yo!<wait 400> Name's Boodin,<wait 200> owner an'\n"
        "operator of Fondest Hopes,\n"
        "<wait 200>your #1 Flopside card shop.\n" BRK
        "You got luck!<wait 300> You caught\n"
        "me right in the middle of\n"
        "restockin' my shop.\n" BRK
        "Go ahead and take a look\n"
        "at what I got on hand.\n"
        "<wait 200>I'll hook you up.\n" END;

    const char boodinItemSelected[] =
        "The %s card?\n"
        "<wait 200>That basically goes for\n"
        "%d coins.<wait 200> How 'bout it?\n"
        "<wait 200><o>";

    const char boodinSelect[] =
        "<select 0 -1 160 40>\n"
        "Sure\n"
        "Nah";

    const char boodinDecline[] =
        "Change your mind?<wait 300> That's\n"
        "chill.<wait 300> Thanks for stoppin'\n"
        "by!<wait 300> Stay safe out there.\n" END;

    const char boodinSatisfied[] =
        "Thanks for the buyin'!\n"
        "<wait 300>Stay safe out there.\n" END;

    const char boodinWantMore[] =
        "Want some more cards?\n" PROMPT;

    const char boodinNoSpace[] =
        "Yo, tough luck on that.\n"
        "<wait 200>You can't hold any more\n"
        "Catch Card SPs.\n" BRK
        "Maybe you'll catch me at\n"
        "another time. Thanks for\n"
        "stoppin' by!\n" END;

    const char boodinClassism[] =
        "Yo, tough luck on that.\n"
        "<wait 200>You can't afford that card.\n" BRK
        "Maybe you'll catch me at\n"
        "another time. Thanks for\n"
        "stoppin' by!\n" END;

    // Gabbi
    const char gabbiSleeping[] =
        "<system>\nThe Nimbi girl appears to\n"
        "be asleep.<wait 1000> You should try\n"
        "waking her up."
        "</wave></dynamic><k>";

    const char gabbiSleeping2[] =
        "<system>\nShe's still asleep.\n"
        "<wait 1000>Maybe you should try\n"
        "<se_on SFX_EVT_SHOCK1><dynamic 3><col c00000ff><wave>blowing her up."
        "</wave></col></dynamic><k>";

    const char gabbiSleeping3[] =
        "<system>\n<dynamic 3><col c00000ff><shake><se_on SFX_EVT_SHOCK1>Bomb her.<wait 500> <se_on SFX_EVT_SHOCK1>Bomb her.<wait 500> <se_on SFX_EVT_SHOCK1>Bomb her.\n"
        "<wait 500><se_on SFX_EVT_SHOCK1>Bomb her.<wait 500> <se_on SFX_EVT_SHOCK1>Bomb her.<wait 500> <se_on SFX_EVT_SHOCK1>Bomb her.\n"
        "<wait 500><se_on SFX_EVT_SHOCK1>Bomb her.<wait 500> <se_on SFX_EVT_SHOCK1>Bomb her.<wait 500> <se_on SFX_EVT_SHOCK1>Bomb her.\n"
        "</shake></col></dynamic><k>";

    const char gabbiMadge[] =
        "<dynamic 3>O-ouch...!!<wait 500> Why didst thou d-do\n"
        "that?<wait 500> <shake>W-what is thy problem,\n"
        "<wait 200>thee wretched knave!?\n"
        "</shake><k>\n<p>\n"
        "P-p-prithee,<wait 200> tellest me why\n"
        "I should not <shake>o-obliterate\n"
        "</shake>thee h-hither and now...!?\n" END;

    const char gabbiSorry[] =
        "<shake>U-um...</shake><wait 500> I am s-sorry for mine\n"
        "outburst...<wait 500> verily, I was quite\n"
        "sensorily o-overwhelmed...\n" END;

    const char gabbiIntro[] =
        "I am...<wait 500> um, yea, I am Gabbi,\n"
        "and I...<wait 500> well,<wait 200> t-thou canst buy\n"
        "chest keys from me...!\n"
        "<wait 200><k>\n<p>\n"
        "I could really use the money...\n"
        "<wait 500>so, um...<wait 200> ahhh...<wait 200> d-dost thou\n"
        "wish to buy some keys...?\n"
        "<wait 200>" PROMPT;

    const char gabbiIntro_N[] =
        "G-greetings...!<wait 500> I hope thee\n"
        "art well...<wait 500> Dost thou need\n"
        "some m-more chest keys?\n" PROMPT;

    const char gabbiIntro_S[] =
        "<shake>Ah, h-hello again...<wait 500> D-dost\n"
        "thou wish to purchase mine\n"
        "w-wares... <wait 500><scale 0.67>Sniffle...<wait 300> Uwehh...\n"
        "</shake></scale>" PROMPT;

    const char gabbiIntro_B[] =
        "Ah, g-greetings...!<wait 500> I am ever\n"
        "so g-glad to see you again...\n" BRK
        "Needst t-thou any more chest\n"
        "keys today...?<wait 500> I have p-plenty!\n" PROMPT;

    const char gabbiSayYes[] =
        CONT
        "O-okay...\n"
        "<wait 500><scale 0.67>Thank thee..." END;

    const char gabbiSayNo[] =
        CONT
        "Oh...<wait 500> oh, well...<wait 500> T-thank thee\n"
        "for t-talking to me anyway...\n"
        "<wait 500><shake><scale 0.67>I-I-I am sorry to waste thy time...\n" END;

    const char gabbiBroke[] =
        CONT
        "Ah, u-umm...<wait 500> I am sorry...\n"
        "<wait 500>It doth appear that thou\n"
        "havest not enough coins...\n" BRK
        "I-it is not thy fault...<wait 500> the\n"
        "e-economy doth be rough,<wait 200> I\n"
        "know s-such...\n" BRK
        "I am so sorry, verily...<wait 500> I hope\n"
        "t-things improveth for thee...\n"
        "<wait 500><shake><scale 0.67>Yea,<wait 200> w-we art in this t-together...\n" END;

    const char gabbiBuyNothing[] =
        "Ah...<wait 500> ahh, a-all right then...\n"
        "<wait 500>Havest safe travels, prithee...\n"
        "<wait 500><shake><scale 0.67>Sniffle...\n" END;

    const char gabbiThanks_N[] =
        "Thanks for thy patronage...!\n"
        "<wait 500>F-fare thee well!\n" END;

    const char gabbiThanks_B[] =
        "O-oh, my...<wait 500> t-t-thank you so\n"
        "much, mine d-dearest customer...!\n" BRK
        "Thy generosity must know no\n"
        "bounds, v-verily... Prithee,\n"
        "enjoyest thy keys...! <scale 0.67>Heehee... <icon HM 0.6 -20 31 0>\n" END;

    const char gabbiSelectInstructions[] =
        "Buy some keys!\n"
        "(Try using <icon PAD_PLUS 0.67 0 0 0> and <icon PAD_MINUS 0.67 0 0 0>)";

    const char gabbiSysGetKeys[] =
        "<system>\nYou got %d chest keys!\n" END;

    // Mitch
    const char mitchIntro_1[] =
        "Hey, alright, we've got a\n"
        "new customer here, yah?\n"
        "<wait 500>Good to see ya, uh-huh.\n" BRK
        "The name's Mitch.<wait 500> It's short\n"
        "for D-Mitch, and that's just\n"
        "'nuff 'bout me, alright?\n" BRK
        "I'm all 'bout my biz, man.\n"
        "<wait 500>Ya gimme chest keys, I give\n"
        "ya <wave>stuff.</wave><wait 300> Name o' the game.\n" BRK
        "See, we've already learned\n"
        "so much 'bout each other,<wait 200> so\n"
        "we'll do some biz, yah?\n" END;

    const char mitchIntro_2[] =
        "Oh, you like the banana pin,\n"
        "huh?<wait 500> Yah, my partner got it\n"
        "for me,<wait 200> cuz I like bananas.\n" BRK
        "...Serious, though,<wait 200> 'nuff\n"
        "personal chit-chat.<wait 500> Buy my\n"
        "stuff or get outta here.\n" END;

    const char mitchIntro_B[] =
        "Hey, what's goin' on, huh?\n"
        "<wait 300>Nice to see ya again, yah.\n"
        "<wait 300>How's Jimbo treatin' ya?\n" BRK
        "Just kiddin', I don't really\n"
        "care.<wait 500> Ya know the drill:<wait 200> buy\n"
        "my stuff or get outta here.\n" END;

    const char mitchPoor[] =
        "What's the big idea, huh?<wait 500> Yuh'\n"
        "too broke to afford the good\n"
        "merchandise, buddy!\n" BRK
        "Get outta here and get yuh'\n"
        "chest keys up, lazybones!\n"
        "<dynamic 3><wait 300>Freeloader!<wait 300> BROKIE!\n"
        "</dynamic><k>";

    const char mitchBuyNothing[] =
        "Nothin'?<wait 500> Right on, man, so ya\n"
        "chose \"get outta here.\" <wait 500><dynamic 3>OKAY!\n"
        "<wait 500>Beat it!<wait 200> Scram!<wait 200> Shoo!<wait 200> NOW!\n"
        "</dynamic><k>";

    const char mitchThanks[] =
        "Alriiiight, thanks for the\n"
        "keys,<wait 200> sucka.<wait 500> I'll use the\n"
        "item on ya now, yah?\n" END;

    const char mitchEnjoy[] =
        "Enjoy it now, uh-huh?\n"
        "<wait 500>Alright, now get outta here.\n" END;

    const char mitchNoInventory[] =
        "Look, pal, ya ran me outta my\n"
        "ENTIRE STOCK.<wait 500> I gots nothin'\n"
        "left for ya right now.\n" BRK
        "I can't sell ya nothin', so\n"
        "<wait 200>get outta my face already,\n"
        "<wait 200><dynamic 2>yah? SA-<wait 200>YO-<wait 200>NA-<wait 200>RA,<wait 200> PAL!\n"
        "</dynamic><k>";

    // Misc
    const char yesNoSelect_NoByDefault[] =
        "<select 1 -1 160 40>\n"
        "Yes\n"
        "No";

    const char yesNoSelect_YesByDefault[] =
        "<select 0 -1 160 40>\n"
        "Yes\n"
        "No";

    const char mac_kanban_004[] =
        "<kanban>\n"
        "<center>Flopside's #1 Useless Pipe</center>\n"
        "<col c00000ff><center>IT'S USELESS! Try it out!</center>\n"
        "</col><center>-Flopside City Hall-</center>\n" END;

    const char D100_entrance_03[] =
        INIT_SYS
        "I can't promise this will\n"
        "work correctly, but hey,\n"
        "whatever floats your boat.\n" PROMPT;

    const char hardDifficultyFirst[] =
        INIT_SYS
        "On Hard Difficulty, shells\n"
        "will no longer deal damage\n"
        "to enemies in the Pit.\n" BRK
        "However, shells that are\n"
        "stomped or kicked will be \n"
        "vulnerable to shell damage.\n" BRK
        "<wave>G<wait 300>o<wait 300>o<wait 300>o<wait 300>o<wait 300>o<wait 300>o<wait 300>d<wait 600> l<wait 300>u<wait 300>c<wait 300>k<wait 300>!\n"
        "</wave><k>";

    // Tattles and Catch Cards :(

    const char tattle_phantom_mario[] =
        "That's Phantom Mario.<wait 150> This\n"
        "fake Mario lives in the Pit\n"
        "of 100 Trials...\n" BRK
        "These HP and Attack readings\n"
        "are highly abnormal...<wait 150> I can't\n"
        "tell what they are...\n" BRK
        "He has high attack power\n"
        "and jumps and hammers well...\n" BRK
        "If you don't plan your stomp,\n"
        "he might hit you with his\n"
        "hammer.\n" BRK
        "You share weaknesses, so\n"
        "if you can exploit that, you'll\n"
        "win...\n" BRK
        "After all, he's not the real\n"
        "Mario...<wait 150> He's just a bad copy...\n" END;

    const char desc_phantom_mario[] =
        "Max HP: ???\n"
        "Attack: ???\n" BRK
        "It's Mario's dark-power\n"
        "doppelganger!\n" BRK
        "Supposedly, one of the final\n"
        "foes in the Pit of 100 Trials.";

    const char tattle_phantom_peach[] =
        "That's Phantom Peach, a phony\n"
        "Peach that dwells in the\n"
        "Pit of 100 Trials...\n" BRK
        "These HP and Attack readings\n"
        "are highly abnormal...<wait 150> I can't\n"
        "tell what they are...\n" BRK
        "This Peach uses her parasol\n"
        "just like the original.\n" BRK
        "And if she uses it in midair,\n"
        "she might mess up the timing\n"
        "of your attack...\n" BRK
        "But that might also give you\n"
        "an opportunity to attack...\n" BRK
        "After all, she's not the real\n"
        "Peach...<wait 150> She's just a bad\n"
        "copy...\n" END;

    const char desc_phantom_peach[] =
        "Max HP: ???\n"
        "Attack: ???\n" BRK
        "It's a dark duplicate of Peach!\n"
        "\n" BRK
        "Supposedly, one of the final\n"
        "foes in the Pit of 100 Trials.";

    const char tattle_phantom_bowser[] =
        "That's Phantom Bowser. <wait 150>This\n"
        "fake Bowser rules the Pit\n"
        "of 100 Trials...\n" BRK
        "These HP and Attack readings\n"
        "are highly abnormal...<wait 150> I can't\n"
        "tell what they are...\n" BRK
        "I know his Defense is 4...<wait 150> \n"
        "He has spikes, so you can't\n"
        "stomp on him...\n" BRK
        "Because of his high Defense, you\n"
        "might need to use Boomer and\n"
        "Cudge...\n" BRK
        "Watch out for his ferocious\n"
        "fire attack...\n" BRK
        "But after all, he's not the\n"
        "real Bowser...<wait 150> He's just a bad\n"
        "copy...\n" END;

    const char desc_phantom_bowser[] =
        "Max HP: ???\n"
        "Attack: ???\n" BRK
        "It's a dark duplicate of\n"
        "Bowser!\n" BRK
        "Supposedly, one of the final\n"
        "foes in the Pit of 100 Trials.";

    const char tattle_phantom_luigi[] =
        "That's Phantom Luigi. <wait 150>He lurks in\n"
        "the Pit of 100 Trials...\n" BRK
        "These HP and Attack readings\n"
        "are highly abnormal...<wait 150> I can't\n"
        "tell what they are...\n" BRK
        "<wait 150>He has the same incredible\n"
        "jumping ability as Luigi.\n" BRK
        "Watch out for that super\n"
        "jump of his when you go to\n"
        "stomp him...\n" BRK
        "But his jump could be an\n"
        "interesting opportunity for an\n"
        "attack as well...\n" BRK
        "After all, he's not the real\n"
        "Luigi...<wait 150> He's just a bad\n"
        "copy...\n" END;

    const char desc_phantom_luigi[] =
        "Max HP: ???\n"
        "Attack: ???\n" BRK
        "It's a dark duplicate of Luigi!\n"
        "\n" BRK
        "Supposedly, one of the final\n"
        "foes in the Pit of 100 Trials.";

    const char tattle_hyper_goomba[] =
        INIT_TATTLE
        "It's a Hyper Goomba...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_hyper_goomba[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_spiked_gloomba[] =
        INIT_TATTLE
        "It's a Spiked Gloomba...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_spiked_gloomba[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_spiked_hyper_goomba[] =
        INIT_TATTLE
        "It's a Spiked Hyper Goomba...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_spiked_hyper_goomba[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_hyper_paragoomba[] =
        INIT_TATTLE
        "It's a Hyper Paragoomba...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_hyper_paragoomba[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_kamikaze_goomba[] =
        INIT_TATTLE
        "It's a Kamikaze Goomba...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_kamikaze_goomba[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_dark_koopa[] =
        INIT_TATTLE
        "It's a Dark Koopa...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_dark_koopa[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_dark_koopatrol[] =
        INIT_TATTLE
        "It's a Dark Koopatrol...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_dark_koopatrol[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_dark_paratroopa[] =
        INIT_TATTLE
        "It's a Dark Paratroopa...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_dark_paratroopa[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_red_spike_top[] =
        INIT_TATTLE
        "It's a Red Spike Top...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_red_spike_top[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_sky_blue_spiny[] =
        INIT_TATTLE
        "It's a Sky-Blue Spiny...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_sky_blue_spiny[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_dark_bones[] =
        INIT_TATTLE
        "It's a Dark Bones...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_dark_bones[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_shady_hammer_bro[] =
        INIT_TATTLE
        "It's a Shady Hammer Bro...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_shady_hammer_bro[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_shady_boomerang_bro[] =
        INIT_TATTLE
        "It's a Shady Boomerang Bro...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_shady_boomerang_bro[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_ice_bro[] =
        INIT_TATTLE
        "It's an Ice Bro...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_ice_bro[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_shady_magikoopa[] =
        INIT_TATTLE
        "It's a Shady Magikoopa...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_shady_magikoopa[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_shady_striker[] =
        INIT_TATTLE
        "It's a Shady Striker...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_shady_striker[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_bomb_boo[] =
        INIT_TATTLE
        "It's a Bomb Boo...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_bomb_boo[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_white_clubba[] =
        INIT_TATTLE
        "It's a White Clubba...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_white_clubba[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_green_fuzzy[] =
        INIT_TATTLE
        "It's a Green Fuzzy...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_green_fuzzy[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_hyper_cleft[] =
        INIT_TATTLE
        "It's a Hyper Cleft...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_hyper_cleft[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_dark_puff[] =
        INIT_TATTLE
        "It's a Dark Puff...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_dark_puff[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_tileoid_pu[] =
        INIT_TATTLE
        "It's a Tileoid PU...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_tileoid_pu[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_bawbus[] =
        INIT_TATTLE
        "It's a Bawbus...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_bawbus[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_ninjeremiah[] =
        INIT_TATTLE
        "It's a Ninjeremiah...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_ninjeremiah[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_skellobyte[] =
        INIT_TATTLE
        "It's a Skellobyte...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_skellobyte[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_spiky_skellobyte[] =
        INIT_TATTLE
        "It's a Spiky Skellobyte...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_spiky_skellobyte[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_ash_cherbil[] =
        INIT_TATTLE
        "It's an Ash Cherbil...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_ash_cherbil[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_shady_koopa[] =
        INIT_TATTLE
        "It's a Shady Koopa...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_shady_koopa[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_flip_shady_koopa[] =
        INIT_TATTLE
        "It's a Shady Koopa...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_flip_shady_koopa[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_flip_buzzy_beetle[] =
        INIT_TATTLE
        "It's a Buzzy Beetle...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_flip_buzzy_beetle[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_flip_spike_top[] =
        INIT_TATTLE
        "It's a Spike Top...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_flip_spike_top[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_green_magikoopa[] =
        INIT_TATTLE
        "It's a Green Magikoopa...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_green_magikoopa[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_white_magikoopa[] =
        INIT_TATTLE
        "It's a White Magikoopa...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_white_magikoopa[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_red_magikoopa[] =
        INIT_TATTLE
        "It's a Red Magikoopa...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_red_magikoopa[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_spinia[] =
        INIT_TATTLE
        "It's a Spinia...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_spinia[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_spunia[] =
        INIT_TATTLE
        "It's a Spunia...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_spunia[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_bullet_william[] =
        INIT_TATTLE
        "It's a Bullet William...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_bullet_william[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_william_blaster[] =
        INIT_TATTLE
        "It's a William Blaster...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_william_blaster[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_bleepboxer[] =
        INIT_TATTLE
        "It's a Bleepboxer...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_bleepboxer[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_kilo_muth[] =
        INIT_TATTLE
        "It's a Kilo Muth...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_kilo_muth[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_yellow_shy_guy[] =
        INIT_TATTLE
        "It's a Yellow Shy Guy...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_yellow_shy_guy[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_green_shy_guy[] =
        INIT_TATTLE
        "It's a Green Shy Guy...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_green_shy_guy[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_blue_shy_guy[] =
        INIT_TATTLE
        "It's a Blue Shy Guy...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n"
        "<wait 250>Defense is 3.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_blue_shy_guy[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_shy_guy[] =
        INIT_TATTLE
        "It's a Shy Guy...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_shy_guy[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_gold_chomp[] =
        INIT_TATTLE
        "It's a Gold Chomp...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n"
        "Defense is 5.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_gold_chomp[] =
        "Max HP: %d   Attack: %d\n"
        "Defense: 5\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_dark_lakitu[] =
        INIT_TATTLE
        "It's a Dark Lakitu...\n"
        "<wait 250>Max HP is %d. Attack is %d.\n" BRK
        "This is placeholder text...\n"
        "<wait 250>The developer doesn't want\n"
        "to write much else...\n" END;

    const char desc_dark_lakitu[] =
        "Max HP: %d\n"
        "Attack: %d\n" BRK
        "Placeholder alert!\n"
        "The dev is very lazy.";

    const char tattle_merluna[] =
        INIT_TATTLE
        "That's Merluna...<wait 250> She's a\n"
        "mysterious sage that claims to\n"
        "render \"Divine Judgement\"...\n" BRK
        "That is, she can bless you or\n"
        "she can curse you...<wait 250> You never\n"
        "know which one it'll be...\n" BRK
        "Apparently, she's recorded as\n"
        "having been alive 2000 years\n"
        "ago...\n" BRK
        "Has she really been around\n"
        "for so long...?\n" END;

    const char tattle_whacka[] =
        INIT_TATTLE
        "That's a Whacka...<wait 250> It's rare to\n"
        "see one...<wait 250> They're a delicate\n"
        "species...\n" BRK
        "I'd advise not stomping on\n"
        "it, unless you want to be\n"
        "responsible for its tears...\n" END;

    const char tattle_gabbi[] =
        INIT_TATTLE
        "That's Gabbi...<wait 250> She's a delicate\n"
        "and shy Nimbi...<wait 250> She sells chest\n"
        "keys for a living...\n" BRK
        "She has a bad habit of falling\n"
        "asleep...<wait 250> Especially when it\n"
        "matters most not to...\n" BRK
        "She's saving her money up\n"
        "for something...<wait 250> I think it's\n"
        "called 'estrogen'...\n" END;

    const char tattle_mitch[] =
        INIT_TATTLE
        "That's D-Mitch, better known\n"
        "as just Mitch...<wait 250> He upsells\n"
        "special items from chests...\n" BRK
        "He only accepts chest keys\n"
        "as payment...<wait 250> And an exorbitant\n"
        "amount of them, too...\n" BRK
        "Apparently,<wait 250> there's a very\n"
        "lucrative aftermarket for\n"
        "chest keys around here...\n" BRK
        "He's gruff and rude to almost\n"
        "everyone... Just not to his\n"
        "'partner', Jimbo...\n" END;

    const char tattle_mover[] =
        INIT_TATTLE
        "That's a Mover...<wait 250> These guys\n"
        "dwell in the Pit and can help\n"
        "you skip a few floors...\n" BRK
        "They can get a little pricey,\n"
        "though...<wait 250> It's up to you if you\n"
        "want to use them...\n" END;

    const char tattle_jimbo[] =
        INIT_TATTLE
        "That's Jimbo...<wait 250> He's an Anti Guy\n"
        "that guards civilians from\n"
        "the Pit of 100 Trials...\n" BRK
        "Well, that's what he claims...\n"
        "<wait 250>Do people even come down\n"
        "here, anyway?\n" BRK
        "He's definitely more powerful\n"
        "than he looks... <wait 250>You should ask\n"
        "what he can do for you...\n" END;

    void npcMessagePatches();
    const char * msgSearchCustomNpc(spm::npcdrv::NPCEntry * npc);
    const char * npcGetNameFromTribeId(s32 tribeId);

}
