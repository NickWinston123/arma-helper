#include "gHelperVar.h"
#include "gHelperHud.h"
#include "rFont.h"
#include "../gHud.h"
#include "nConfig.h"
#include "tString.h"
#include <vector>
#include <map>
#include "gHelperUtilities.h"

namespace helperConfig
{
    bool sg_helperHud = false; // Helper Hud
    static tConfItem<bool> sg_helperHudConf = HelperCommand::tConfItem("HELPER_HUD", sg_helperHud);
    bool sg_helperHudFreeze = false; // Helper Hud
    static tConfItem<bool> sg_helperHudFreezeConf = HelperCommand::tConfItem("HELPER_HUD_FREEZE", sg_helperHudFreeze);
    REAL sg_helperHudX = 0.755; // Helper Hud Y Position
    static tConfItem<REAL> sg_helperHudXC = HelperCommand::tConfItem("HELPER_HUD_LOCX", sg_helperHudX);
    REAL sg_helperHudY = -0.01; // Helper Hud Y Position
    static tConfItem<REAL> sg_helperHudYC = HelperCommand::tConfItem("HELPER_HUD_LOCY", sg_helperHudY);
    REAL sg_helperHudSize = .055; // Size of Helper Hud
    static tConfItem<REAL> sg_helperHudSizeC = HelperCommand::tConfItem("HELPER_HUD_SIZE", sg_helperHudSize);
    tString sg_helperHudIgnoreList = tString("");
    static tConfItem<tString> sg_helperHudIgnoreListConf = HelperCommand::tConfItem("HELPER_HUD_IGNORE_LIST", sg_helperHudIgnoreList);

    tString sg_helperHudColorHeader("0xff0033"); // Red for labels
    static tConfItem<tString> sg_helperHudColorHeaderConf = HelperCommand::tConfItem("HELPER_HUD_COLOR_LABEL", sg_helperHudColorHeader);
    tString sg_helperHudColorItem("0xeeffff");   // White for values
    static tConfItem<tString> sg_helperHudColorItemConf = HelperCommand::tConfItem("HELPER_HUD_COLOR_VALUES", sg_helperHudColorItem);
    tString sg_helperHudColorMain("0x00dd00");   // Green for enabled
    static tConfItem<tString> sg_helperHudColorMainConf = HelperCommand::tConfItem("HELPER_HUD_COLOR_ENABLED", sg_helperHudColorMain);
    tString sg_helperHudColorError("0xdd0000");  // Red for disabled
    static tConfItem<tString> sg_helperHudColorErrorConf = HelperCommand::tConfItem("HELPER_HUD_COLOR_DISABLED", sg_helperHudColorError);
}
tThemedText gHelperHudBase::theme(sg_helperHudColorHeader, sg_helperHudColorMain, sg_helperHudColorItem, sg_helperHudColorError);

using namespace helperConfig;

static std::map<std::string, gHelperHudBase *> *st_confMap = 0;

gHelperHudBase::gHelperHudMap &gHelperHudBase::GetHelperHudMap()
{
    if (!st_confMap)
        st_confMap = tNEW(gHelperHudMap);

    return *st_confMap;
}

gHelperHudBase::gHelperHudBase(int id_, std::string label_, std::string parent_)
    : id(id_)
    , label(label_)         
    , parent(parent_)       
{
    auto &hudMap = gHelperHudBase::GetHelperHudMap();

    std::string compositeKey = parent_ + "||" + label_;

    if (hudMap.find(compositeKey) != hudMap.end())
        tERR_ERROR_INT("Duplicate HUD item with parent+label = " << compositeKey);

    hudMap[compositeKey] = this;
}


void gHelperHudBase::Render()
{
    if (!sg_helper || !sg_helperHud || !sghuk)
        return;

    std::map<std::string, std::vector<gHelperHudBase *>> hudMap;
    gHelperHudMap &items = gHelperHudBase::GetHelperHudMap();

    for (auto &kv : items)
    {
        gHelperHudBase *item = kv.second;

        if (!sg_helperHudIgnoreList.empty() 
            && tIsInList(sg_helperHudIgnoreList, item->getLabelStr()))
            continue;

        std::string parentName = item->getParent();
        hudMap[parentName].push_back(item);
    }

    for (auto &kv : hudMap)
    {
        auto &vec = kv.second; 

        std::sort(vec.begin(), vec.end(),
        [](gHelperHudBase *a, gHelperHudBase *b)
        {
            bool aIsStatus = (a->getLabel() == "Status");
            bool bIsStatus = (b->getLabel() == "Status");

            if (aIsStatus && !bIsStatus) 
                return true;
            if (bIsStatus && !aIsStatus) 
                return false;
                
            return a->getLabel() < b->getLabel();
        });
    }

    rTextField hudDebug(sg_helperHudX - .15 * sg_helperHudSize / 2.0, 
                        sg_helperHudY, 
                        .15 * sg_helperHudSize, 
                        .3 * sg_helperHudSize);

   
    for (auto &kv : hudMap)
    {
        const std::string &parent = kv.first;
        if (!parent.empty())
            hudDebug << "0xRESETT" << parent << ":\n";

        for (auto item : kv.second)
        {
            gTextCache<tString> cache;
            if (!cache.Call(item->getValue(), item->getLastValue()))
            {
                rDisplayListFiller filler(cache.list_);
                hudDebug << item->displayString();
                item->setLastValue();
            }
        }
    }
}