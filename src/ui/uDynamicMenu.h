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

class uMenuItemToggleThemed : public uMenuItemToggle
{
public:
    uMenuItemToggleThemed(uMenu *menu,
                          const char *title,
                          const char *help,
                          bool &target,
                          tThemedText &theTheme,
                          REAL valueLocX)
        : uMenuItemToggle(menu, "", help, target),
          customTitle(title),
          targetPtr(&target),
          menutheme(theTheme),
          locX(valueLocX)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1, bool selected = false) override
    {
        bool currentValue = *targetPtr;

        tString themedLabel = menutheme.HeaderColor() + customTitle;
        tString themedValue = currentValue
                                  ? (menutheme.MainColor() + tString("Enabled"))
                                  : (menutheme.ErrorColor() + tString("Disabled"));

        DisplayText(x, y, themedLabel.c_str(), selected, alpha, 1);
        DisplayText(x + locX, y, themedValue.c_str(), selected, alpha, -1);
    }

private:
    tString customTitle;
    bool *targetPtr;
    tThemedText &menutheme;
    REAL locX;
};

class uMenuItemIntThemed : public uMenuItemInt
{
public:
    uMenuItemIntThemed(uMenu *menu,
                       const char *title,
                       const char *help,
                       int &target,
                       int minVal,
                       int maxVal,
                       int stepVal,
                       tThemedText &theTheme,
                       REAL valueLocX)
        : uMenuItemInt(menu, "", help, target, minVal, maxVal, stepVal),
          customTitle(title),
          targetPtr(&target),
          menutheme(theTheme),
          locX(valueLocX)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1, bool selected = false) override
    {
        tString themedLabel = menutheme.HeaderColor() + customTitle;

        tString valueStr;
        valueStr << *targetPtr;
        tString themedValue = menutheme.ItemColor() + valueStr;

        DisplayText(x, y, themedLabel.c_str(), selected, alpha, 1);
        DisplayText(x + locX, y, themedValue.c_str(), selected, alpha, -1);
    }

private:
    tString customTitle;
    int *targetPtr;
    tThemedText &menutheme;
    REAL locX;
};

class uMenuItemRealThemed : public uMenuItemReal
{
public:
    uMenuItemRealThemed(uMenu *menu,
                        const char *title,
                        const char *help,
                        REAL &target,
                        REAL minVal,
                        REAL maxVal,
                        REAL stepVal,
                        tThemedText &theTheme,
                        REAL valueLocX)
        : uMenuItemReal(menu, "", help, target, minVal, maxVal, stepVal),
          customTitle(title),
          targetPtr(&target),
          menutheme(theTheme),
          locX(valueLocX)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1, bool selected = false) override
    {
        tString themedLabel = menutheme.HeaderColor() + customTitle;

        tString valueStr;
        valueStr << *targetPtr;
        tString themedValue = menutheme.ItemColor() + valueStr;

        DisplayText(x, y, themedLabel.c_str(), selected, alpha, 1);
        DisplayText(x + locX, y, themedValue.c_str(), selected, alpha, -1);
    }

private:
    tString customTitle;
    REAL *targetPtr;
    tThemedText &menutheme;
    REAL locX;
};

class uMenuItemStringThemed : public uMenuItemString
{
public:
    uMenuItemStringThemed(uMenu *menu,
                          const char *title,
                          const char *help,
                          tString &target,
                          tThemedText &theTheme,
                          REAL valueLocX,
                          int maxLength = 1024)
        : uMenuItemString(menu, tString(""), help, target, maxLength),
          customTitle(title),
          menutheme(theTheme),
          locX(valueLocX)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1, bool selected = false) override
    {
    #ifndef DEDICATED
        static int counter = 0;
        counter++;

        int cmode = 0;
        if (selected)
        {
            cmode = 1;
            if (counter & 32)
                cmode = 2;
        }

        rTextField::ColorMode currentColorMode = colorMode_;
        if (currentColorMode == rTextField::COLOR_SHOW && !selected)
            currentColorMode = rTextField::COLOR_USE;

        tString themedLabel = menutheme.HeaderColor() + customTitle;

        DisplayText(x, y, themedLabel.c_str(), selected, alpha, 1);
        DisplayText(x + locX, y, &((*content)[0]), selected, alpha, -1,
                    cmode, cursorPos, currentColorMode);
    #endif
    }

private:
    tString customTitle;
    tThemedText &menutheme;
    REAL locX;
};

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
                                 REAL valueLocX)
    {
        tString helpText;
        helpText << "$" << settingTitle << "_help";
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
                                    const tString &categoryList,
                                    tThemedText &menutheme,
                                    REAL valueLocX)
    {
        for (auto &it : menuRegistry)
            delete it.second;
        menuRegistry.clear();

        uMenu *rootMenu = new uMenu(rootMenuName.c_str());

        tArray<tString> headArray = categoryList.ToUpper().Split(",");
        std::vector<tString> headPrefixes;
        for (int i = 0; i < headArray.Len(); i++)
        {
            tString token = headArray[i].Trim();
            if (!token.empty())
                headPrefixes.push_back(token);
        }

        if (headPrefixes.empty())
        {
            const auto &itemsMap = tConfItemBase::GetConfItemMap();
            std::vector<tString> allTitles;
            allTitles.reserve(itemsMap.size());

            for (auto &entry : itemsMap)
            {
                if (entry.second && entry.second->allowedChange())
                    allTitles.push_back(entry.first);
            }

            std::sort(allTitles.begin(), allTitles.end(),
                      [](const tString &a, const tString &b)
                      {
                          return a > b;
                      });

            for (auto &confTitle : allTitles)
            {
                tConfItemBase *confItem = tConfItemBase::GetConfigItem(confTitle);
                if (confItem)
                {
                    uDynamicMenu::AddSettingToMenu(*rootMenu,
                                                   confTitle,
                                                   confItem,
                                                   menutheme,
                                                   valueLocX);
                }
            }
        }
        else
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
                    tConfItemBase *confItem = tConfItemBase::GetConfigItem(confTitle);
                    if (confItem && confItem->allowedChange())
                        AddSettingToMenu(*subMenu, confTitle, confItem, menutheme, valueLocX);
                }
            }
        }

        rootMenu->Enter();
    }

}

bool su_dynamicMenu = true;
static tConfItem<bool> su_dynamicMenuConf = HelperCommand::tConfItem("DYNAMIC_MENU", su_dynamicMenu);

REAL su_dynamicMenuValueLocX = 0.02;
static tConfItem<REAL> su_dynamicMenuValueLocXConf = HelperCommand::tConfItem("DYNAMIC_MENU_VALUE_LOCX", su_dynamicMenuValueLocX);

tString su_dynamicMenuColorHeader("0xffffff"); // White for labels
static tConfItem<tString> su_dynamicMenuColorHeaderConf = HelperCommand::tConfItem("DYNAMIC_MENU_COLOR_LABEL", su_dynamicMenuColorHeader);
tString su_dynamicMenuColorItem("0xaaaaaa"); // Light Gray for values
static tConfItem<tString> su_dynamicMenuColorItemConf = HelperCommand::tConfItem("DYNAMIC_MENU_COLOR_VALUES", su_dynamicMenuColorItem);
tString su_dynamicMenuColorMain("0x00dd00"); // Green for enabled
static tConfItem<tString> su_dynamicMenuColorMainConf = HelperCommand::tConfItem("DYNAMIC_MENU_COLOR_ENABLED", su_dynamicMenuColorMain);
tString su_dynamicMenuColorError("0xdd0000"); // Red for disabled
static tConfItem<tString> su_dynamicMenuColorErrorConf = HelperCommand::tConfItem("DYNAMIC_MENU_COLOR_DISABLED", su_dynamicMenuColorError);

tString su_dynamicMenuCategories("");
static tConfItem<tString> su_dynamicMenuCategoriesConf = HelperCommand::tConfItem("DYNAMIC_MENU_CATEGORIES", su_dynamicMenuCategories);

tThemedText dynamicMenuTheme(su_dynamicMenuColorHeader, su_dynamicMenuColorMain, su_dynamicMenuColorItem, su_dynamicMenuColorError);

std::unordered_map<std::string, uMenu *> dynamicMenuRegistry;

void LaunchDynamicMenu()
{

    static tString menuName("Dynamic Menu");
    uDynamicMenu::GenerateDynamicMenu(dynamicMenuRegistry,
                                      menuName,
                                      su_dynamicMenuCategories,
                                      dynamicMenuTheme,
                                      su_dynamicMenuValueLocX);
}

void LaunchDynamicMenuPub(std::istream &s)
{
    LaunchDynamicMenu();
}
static tConfItemFunc dynamicMenuConf = HelperCommand::tConfItemFunc("DYNAMIC_MENU_LAUNCH", &LaunchDynamicMenuPub);

#endif // U_DYNAMIC_MENU_H
