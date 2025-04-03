#ifndef U_DYNAMIC_MENU_H
#define U_DYNAMIC_MENU_H

#include <string>
#include <unordered_map>
#include <deque>
#include <vector>
#include <algorithm>

#include "uMenu.h"
#include "tConfiguration.h"
#include "nConfig.h"


namespace uDynamicMenu
{
    inline uMenu *GetOrCreateMenu(const std::string &menuName,
                                  std::unordered_map<std::string, uMenu *> &menuRegistry,
                                  uMenu *parentMenu = nullptr)
    {
        if (menuRegistry.find(menuName) == menuRegistry.end())
        {
            menuRegistry[menuName] = new uMenu(menuName.c_str());
            if (parentMenu)
            {
                tString subTitle = tString("Settings for ") + tString(menuName);
                new uMenuItemSubmenu(parentMenu, menuRegistry[menuName], subTitle.c_str());
            }
        }
        return menuRegistry[menuName];
    }

    inline void AddSettingToMenu(uMenu &parentMenu,
                                 const tString &settingTitle,
                                 tConfItemBase *confItem,
                                 tThemedText &menutheme,
                                 REAL &valueLocX)
    {
        tString helpText;
        helpText << "$"
                 << settingTitle
                 << "_help";
        tToLower(helpText);

        if (auto *boolItem = dynamic_cast<tConfItem<bool> *>(confItem))
        {
            new uMenuItemToggleThemed(&parentMenu,
                                      settingTitle.c_str(),
                                      helpText.c_str(),
                                      *const_cast<bool *>(boolItem->GetTarget()),
                                      menutheme,
                                      valueLocX);
        }
        else if (auto *intItem = dynamic_cast<tConfItem<int> *>(confItem))
        {
            new uMenuItemIntThemed(&parentMenu,
                                   settingTitle.c_str(),
                                   helpText.c_str(),
                                   *const_cast<int *>(intItem->GetTarget()),
                                   -9669966, 9669966, 1,
                                   menutheme,
                                   valueLocX);
        }
        else if (auto *realItem = dynamic_cast<tConfItem<REAL> *>(confItem))
        {
            new uMenuItemRealThemed(&parentMenu,
                                    settingTitle.c_str(),
                                    helpText.c_str(),
                                    *const_cast<REAL *>(realItem->GetTarget()),
                                    -9669966, 9669966, 0.1,
                                    menutheme,
                                    valueLocX);
        }
        else if (auto *strItem = dynamic_cast<tConfItem<tString> *>(confItem))
        {
            new uMenuItemStringThemed(&parentMenu,
                                      settingTitle.c_str(),
                                      helpText.c_str(),
                                      *const_cast<tString *>(strItem->GetTarget()),
                                      menutheme,
                                      valueLocX);
        }
    }

    inline void GenerateDynamicMenu(std::unordered_map<std::string, uMenu *> &menuRegistry,
                                    const tString &rootMenuName,
                                    tThemedText &menutheme,
                                    REAL &valueLocX,
                                    tString categoryConf = tString(""),
                                    tString excludeConf = tString(""))
    {

        for (auto &it : menuRegistry)
            delete it.second;
        menuRegistry.clear();

        uMenu *rootMenu = new uMenu(rootMenuName.c_str());

        tConfItemBase *catItem = nullptr;
        tConfItemBase *exItem = nullptr;

        if (!excludeConf.empty())
            exItem = tConfItemBase::GetConfigItem(excludeConf);

        if (!categoryConf.empty())
            catItem = tConfItemBase::GetConfigItem(categoryConf);

        tString categoryList = tString("");
        tString excludeList  = tString("");

        if (auto *catStr = dynamic_cast<tConfItem<tString> *>(catItem))
            categoryList = *catStr->GetTarget();

        if (auto *exStr = dynamic_cast<tConfItem<tString> *>(exItem))
            excludeList = *exStr->GetTarget();

        std::vector<tString> excludeTokens;
        if (!excludeList.empty())
        {
            tArray<tString> tokens = excludeList.ToUpper().Split(",");
            for (int i = 0; i < tokens.Len(); i++)
            {
                tString token = tokens[i].Trim();
                if (!token.empty())
                    excludeTokens.push_back(token);
            }
        }

        tArray<tString> headArray = categoryList.ToUpper().Split(",");
        std::vector<tString> headPrefixes;
        for (int i = 0; i < headArray.Len(); i++)
        {
            tString token = headArray[i].Trim();
            if (!token.empty())
                headPrefixes.push_back(token);
        }

        if (headPrefixes.empty()) // all commands
        {
            const auto &itemsMap = tConfItemBase::GetConfItemMap();
            std::vector<tString> allTitles;
            allTitles.reserve(itemsMap.size());

            for (auto &entry : itemsMap)
            {
                if (entry.second && entry.second->allowedChange())
                    allTitles.push_back(entry.first);
            }

            std::sort(allTitles.begin(), allTitles.end(), [](const tString &a, const tString &b)
                      {
                         return a > b; 
                     });

            for (const auto &confTitle : allTitles)
            {
                bool skip = false;
                if (!excludeTokens.empty())
                {
                    tString upperTitle = confTitle.ToUpper();
                    for (const auto &token : excludeTokens)
                    {
                        if (upperTitle.Contains(token))
                        {
                            skip = true;
                            break;
                        }
                    }
                }
                if (skip) 
                    continue;

                tConfItemBase *confItem = tConfItemBase::GetConfigItem(confTitle);
                if (confItem)
                    AddSettingToMenu(*rootMenu, confTitle, confItem, menutheme, valueLocX);
            }
        }
        else // specific command categories
        {
            std::reverse(headPrefixes.begin(), headPrefixes.end());

            for (const auto &head : headPrefixes)
            {
                uMenu *subMenu = GetOrCreateMenu(head.c_str(), menuRegistry, rootMenu);
                tString headReplaced = head;
                subMenu->title = headReplaced.Replace("_", " ");

                std::deque<tString> configTitles;
                tConfItemBase::FindConfigItems(head, configTitles);

                std::vector<tString> sortedTitles(configTitles.begin(), configTitles.end());
                std::sort(sortedTitles.begin(), sortedTitles.end(),
                        [](const tString &a, const tString &b) 
                        { 
                            return a > b; 
                        });

                for (const auto &confTitle : sortedTitles)
                {
                    bool skip = false;
                    if (!excludeTokens.empty())
                    {
                        tString upperTitle = confTitle.ToUpper();
                        for (const auto &token : excludeTokens)
                        {
                            if (upperTitle.Contains(token))
                            {
                                skip = true;
                                break;
                            }
                        }
                    }
                    if (skip) 
                        continue;

                    tConfItemBase *confItem = tConfItemBase::GetConfigItem(confTitle);
                    if (confItem && confItem->allowedChange())
                        AddSettingToMenu(*subMenu, confTitle, confItem, menutheme, valueLocX);
                }
            }
        }

        if (exItem)
            AddSettingToMenu(*rootMenu, exItem->GetTitle(),  exItem, menutheme, valueLocX);
        if (catItem)
            AddSettingToMenu(*rootMenu, catItem->GetTitle(), catItem, menutheme, valueLocX);

        rootMenu->Enter();
    }

}

bool su_dynamicMenu = true;
static tConfItem<bool> su_dynamicMenuConf("DYNAMIC_MENU", su_dynamicMenu);

REAL su_dynamicMenuValueLocX = 0.02;
static tConfItem<REAL> su_dynamicMenuValueLocXConf("DYNAMIC_MENU_VALUE_LOCX", su_dynamicMenuValueLocX);

tString su_dynamicMenuColorHeader("0xffffff"); // White for labels
static tConfItem<tString> su_dynamicMenuColorHeaderConf("DYNAMIC_MENU_COLOR_LABEL", su_dynamicMenuColorHeader);
tString su_dynamicMenuColorItem("0xaaaaaa");   // Light Gray for values
static tConfItem<tString> su_dynamicMenuColorItemConf("DYNAMIC_MENU_COLOR_VALUES", su_dynamicMenuColorItem);
tString su_dynamicMenuColorMain("0x00dd00");   // Green for enabled
static tConfItem<tString> su_dynamicMenuColorMainConf("DYNAMIC_MENU_COLOR_ENABLED", su_dynamicMenuColorMain);
tString su_dynamicMenuColorError("0xdd0000");  // Red for disabled
static tConfItem<tString> su_dynamicMenuColorErrorConf("DYNAMIC_MENU_COLOR_DISABLED", su_dynamicMenuColorError);

tString su_dynamicMenuCategories("");
static tConfItem<tString> su_dynamicMenuCategoriesConf("DYNAMIC_MENU_CATEGORIES", su_dynamicMenuCategories);

tString su_dynamicMenuCategoriesExcludeList("");
static tConfItem<tString> su_dynamicMenuCategoriesExcludeListConf("DYNAMIC_MENU_CATEGORIES_EXCLUDE", su_dynamicMenuCategoriesExcludeList);

tThemedText dynamicMenuTheme(su_dynamicMenuColorHeader, su_dynamicMenuColorMain, su_dynamicMenuColorItem, su_dynamicMenuColorError);

std::unordered_map<std::string, uMenu *> dynamicMenuRegistry;

void LaunchDynamicMenu()
{

    static tString menuName("Dynamic Menu");
    uDynamicMenu::GenerateDynamicMenu(dynamicMenuRegistry,
                                      menuName,
                                      dynamicMenuTheme,
                                      su_dynamicMenuValueLocX,
                                      su_dynamicMenuCategoriesConf.GetTitle(),
                                      su_dynamicMenuCategoriesExcludeListConf.GetTitle());
}

void LaunchDynamicMenuPub(std::istream &s)
{
    LaunchDynamicMenu();
}
static tConfItemFunc dynamicMenuConf = tConfItemFunc("DYNAMIC_MENU_LAUNCH", &LaunchDynamicMenuPub);

#endif // U_DYNAMIC_MENU_H
