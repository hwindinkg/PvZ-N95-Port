#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <e32def.h>
#include "engine/Image.h"

// Global images referenced by the game code
namespace Sexy {

extern Image* IMAGE_BACKGROUND1;
extern Image* IMAGE_BACKGROUND2;
extern Image* IMAGE_BACKGROUND3;
extern Image* IMAGE_BACKGROUND4;
extern Image* IMAGE_BACKGROUND5;
extern Image* IMAGE_BACKGROUND6;
extern Image* IMAGE_BUGSPRAY;
extern Image* IMAGE_CHOCOLATE;
extern Image* IMAGE_FERTILIZER;
extern Image* IMAGE_FLAG;
extern Image* IMAGE_FOG;
extern Image* IMAGE_FOG_SOFTWARE;
extern Image* IMAGE_GLOVE;
extern Image* IMAGE_GREENHOUSE;
extern Image* IMAGE_HOUSEDOOR_BOTTOM;
extern Image* IMAGE_HOUSEDOOR_TOP;
extern Image* IMAGE_ICE;
extern Image* IMAGE_MONEY_SIGN;
extern Image* IMAGE_MUSHROOM_GARDEN;
extern Image* IMAGE_PHONOGRAPH;
extern Image* IMAGE_ROOFDOOR_BOTTOM;
extern Image* IMAGE_ROOFDOOR_TOP;
extern Image* IMAGE_SEEDBANK;
extern Image* IMAGE_SEEDBANK_BLANK;
extern Image* IMAGE_SEEDBANK_CONVEYORBELT;
extern Image* IMAGE_SEEDBANK_ICE;
extern Image* IMAGE_SEEDBANK_ZOMBIE;
extern Image* IMAGE_SHOVEL;
extern Image* IMAGE_SHOVELBANK;
extern Image* IMAGE_SLOT_MACHINE_BACKGROUND;
extern Image* IMAGE_SODBANK;
extern Image* IMAGE_SUN;
extern Image* IMAGE_PLANTSHADOW;
extern Image* IMAGE_SUNBANK;
extern Image* IMAGE_TREE_FOOD;
extern Image* IMAGE_TREEOFWISDOM;
extern Image* IMAGE_WATERINGCAN;
extern Image* IMAGE_WHEELBARROW;
extern Image* IMAGE_ZENSHOPBUTTON;
extern Image* IMAGE_ZENSHOPBUTTON_HIGHLIGHT;
extern Image* IMAGE_ZOMBIQUARIUM;
extern Image* IMAGE_SEEDCHOOSER_BUTTON;

// Coin / award images
extern Image* IMAGE_SEEDS;
extern Image* IMAGE_TROPHY;
extern Image* IMAGE_SUNFLOWER_TROPHY;
extern Image* IMAGE_CARKEYS;
extern Image* IMAGE_ALMANAC;
extern Image* IMAGE_SCARY_POT;
extern Image* IMAGE_TACO;
extern Image* IMAGE_ZOMBIE_NOTE_SMALL;
extern Image* IMAGE_PRESENT;
extern Image* IMAGE_PRESENTOPEN;
extern Image* IMAGE_MONEYBAG;
extern Image* IMAGE_MONEYBAG_HI_RES;
extern Image* IMAGE_TROPHY_HI_RES;
extern Image* IMAGE_SHOVEL_HI_RES;
extern Image* IMAGE_AWARDPICKUPGLOW;
extern Image* IMAGE_REANIM_COINGLOW;

// Reanimation image overrides
extern Image* IMAGE_REANIM_ZOMBIE_BUCKET1;
extern Image* IMAGE_REANIM_ZOMBIE_BUCKET2;
extern Image* IMAGE_REANIM_ZOMBIE_BUCKET3;
extern Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR1;
extern Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR2;
extern Image* IMAGE_REANIM_ZOMBIE_SCREENDOOR3;
extern Image* IMAGE_REANIM_ZOMBIE_LADDER_1;
extern Image* IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE1;
extern Image* IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE2;
extern Image* IMAGE_REANIM_ZOMBIE_LADDER_5;
extern Image* IMAGE_REANIM_ZOMBIE_JACKBOX_BOX;
extern Image* IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE;
extern Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET;
extern Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET2;
extern Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET3;
extern Image* IMAGE_REANIM_WALLNUT_BODY;
extern Image* IMAGE_REANIM_WALLNUT_CRACKED1;
extern Image* IMAGE_REANIM_WALLNUT_CRACKED2;
extern Image* IMAGE_REANIM_TALLNUT_CRACKED1;
extern Image* IMAGE_REANIM_TALLNUT_CRACKED2;
extern Image* IMAGE_REANIM_GARLIC_BODY2;
extern Image* IMAGE_REANIM_GARLIC_BODY3;
extern Image* IMAGE_REANIM_PUMPKIN_DAMAGE1;
extern Image* IMAGE_REANIM_PUMPKIN_DAMAGE3;
extern Image* IMAGE_REANIM_COIN_SILVER_DOLLAR;
extern Image* IMAGE_REANIM_COIN_GOLD_DOLLAR;
extern Image* IMAGE_REANIM_DIAMOND;
extern Image* IMAGE_PLANTSHADOW2;
extern Image* IMAGE_ZOMBIEPOGO;
extern Image* IMAGE_BLANK;
extern Image* IMAGE_REANIM_ZOMBIE_GARGANTUAR_ZOMBIE;
extern Image* IMAGE_REANIM_ZOMBIE_GARGANTUAR_DUCKXING;
extern Image* IMAGE_REANIM_ZOMBIE_GARGANTUAR_HEAD_REDEYE;
extern Image* IMAGE_REANIM_ZOMBIE_PAPER_MADHEAD;
extern Image* IMAGE_REANIM_ZOMBIE_FOOTBALL_LEFTARM_UPPER2;
extern Image* IMAGE_REANIM_ZOMBIE_PAPER_LEFTARM_UPPER2;
extern Image* IMAGE_REANIM_ZOMBIE_OUTERARM_UPPER2;

// Projectile images
extern Image* IMAGE_REANIM_COBCANNON_COB;
extern Image* IMAGE_PROJECTILEPEA;
extern Image* IMAGE_PROJECTILESNOWPEA;
extern Image* IMAGE_PROJECTILECACTUS;
extern Image* IMAGE_PROJECTILE_STAR;
extern Image* IMAGE_PUFFSHROOM_PUFF1;
extern Image* IMAGE_REANIM_ZOMBIE_CATAPULT_BASKETBALL;
extern Image* IMAGE_REANIM_CABBAGEPULT_CABBAGE;
extern Image* IMAGE_REANIM_CORNPULT_KERNAL;
extern Image* IMAGE_REANIM_CORNPULT_BUTTER;
extern Image* IMAGE_REANIM_MELONPULT_MELON;
extern Image* IMAGE_REANIM_WINTERMELON_PROJECTILE;
extern Image* IMAGE_PEA_SHADOWS;

// GridItem images
extern Image* IMAGE_PLANTSPEECHBUBBLE;
extern Image* IMAGE_BRAIN;
extern Image* IMAGE_TOMBSTONES;
extern Image* IMAGE_TOMBSTONE_MOUNDS;
extern Image* IMAGE_CRATER;
extern Image* IMAGE_CRATER_WATER_NIGHT;
extern Image* IMAGE_CRATER_WATER_DAY;
extern Image* IMAGE_CRATER_ROOF_LEFT;
extern Image* IMAGE_CRATER_ROOF_CENTER;
extern Image* IMAGE_CRATER_FADING;

// TitleScreen and loading images
extern Image* IMAGE_TITLESCREEN;
extern Image* IMAGE_PARTNER_LOGO;
extern Image* IMAGE_POPCAP_LOGO;
extern Image* IMAGE_LOADBAR_DIRT;
extern Image* IMAGE_LOADBAR_GRASS;
extern Image* IMAGE_PVZ_LOGO;
extern Image* IMAGE_REANIM_SODROLLCAP;

// Image access helpers
inline Image* GetImageById(int id) { (void)id; return NULL; }
inline Image* GetImageByName(const char* name) { (void)name; return NULL; }

// Extract resource group functions (stubs unless called via ResourceManager)
bool ExtractResourcesByName(class ResourceManager* theResourceManager, const char* theName);
bool ExtractInitResources(class ResourceManager* theResourceManager);
bool ExtractLoaderBarResources(class ResourceManager* theResourceManager);

} // namespace Sexy

#endif // __RESOURCES_H__
