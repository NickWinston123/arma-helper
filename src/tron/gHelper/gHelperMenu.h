#ifndef ArmageTron_GHELPER_MENU
#define ArmageTron_GHELPER_MENU

#include "gHelper.h"
#include "uDynamicMenu.h"

using namespace helperConfig;

std::unordered_map<std::string, uMenu *> menuRegistry;

tString sg_helperMenuColorHeader("0xffffff"); // White for labels
static tConfItem<tString> sg_helperMenuColorHeaderConf = HelperCommand::tConfItem("HELPER_MENU_COLOR_LABEL", sg_helperMenuColorHeader);
tString sg_helperMenuColorItem("0xaaaaaa"); // Light Gray for values
static tConfItem<tString> sg_helperMenuColorItemConf = HelperCommand::tConfItem("HELPER_MENU_COLOR_VALUES", sg_helperMenuColorItem);
tString sg_helperMenuColorMain("0x00dd00"); // Green for enabled
static tConfItem<tString> sg_helperMenuColorMainConf = HelperCommand::tConfItem("HELPER_MENU_COLOR_ENABLED", sg_helperMenuColorMain);
tString sg_helperMenuColorError("0xdd0000"); // Red for disabled
static tConfItem<tString> sg_helperMenuColorErrorConf = HelperCommand::tConfItem("HELPER_MENU_COLOR_DISABLED", sg_helperMenuColorError);

tString sg_helperMenuCategories = tString("HELPER,HELPER_MENU,HELPER_CONF,HELPER_DEBUG,HELPER_HUD,HELPER_SMART_TURNING,HELPER_SELF_PATH,HELPER_SELF_SHOW_HIT,") +
                                  tString("HELPER_SELF_AUTO_BRAKE,HELPER_SELF_RUBBER_RATIO_BRAKE,HELPER_SELF_SHOW_CORNERS,HELPER_SELF_SIMPLE_BOT,") +
                                  tString("HELPER_SELF_SHOW_TAIL,HELPER_SELF_SHOW_TAIL_TRACER,HELPER_SELF_TRACE,HELPER_ENEMY_DETECT_CUT,") +
                                  tString("HELPER_ENEMY_SHOW_TAIL,HELPER_ENEMY_TRACERS,HELPER_ZONE,HELPER_CAN_SURVIVE");
static tConfItem<tString> sg_helperMenuCategoriesConf = HelperCommand::tConfItem("HELPER_MENU_CATEGORIES", sg_helperMenuCategories);

REAL sg_helperMenuValueLocX = 0.02;
static tConfItem<REAL> sg_helperMenuValueLocXConf = HelperCommand::tConfItem("HELPER_MENU_VALUE_LOCX", sg_helperMenuValueLocX);

tThemedText menutheme(sg_helperMenuColorHeader, sg_helperMenuColorMain, sg_helperMenuColorItem, sg_helperMenuColorError);

void helperDynamicMenu()
{
    if (!helperConfig::sghuk)
        return;
    static tString menuName("Helper Settings");
    uDynamicMenu::GenerateDynamicMenu(menuRegistry,
                                      menuName,
                                      sg_helperMenuCategories,
                                      menutheme,
                                      sg_helperMenuValueLocX);
}

void helperMenuDynPub(std::istream &s)
{
    helperDynamicMenu();
}
static tConfItemFunc HelperMenuDynConf = HelperCommand::tConfItemFunc("HELPER_MENU_LAUNCH", &helperMenuDynPub);

#endif