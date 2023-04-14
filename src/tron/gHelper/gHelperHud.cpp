#include "gHelperVar.h"
#include "gHelperHud.h"
#include "rFont.h"
#include "../gHud.h"
#include "nConfig.h"
#include "tString.h"
#include <vector>
#include <map>

namespace helperConfig {
bool sg_helperHud = false; // Helper Hud
static tConfItem<bool> sg_helperHudConf("HELPER_HUD", sg_helperHud);
bool sg_helperHudFreeze = false; // Helper Hud
static tConfItem<bool> sg_helperHudFreezeConf("HELPER_HUD_FREEZE", sg_helperHudFreeze);
REAL sg_helperHudX = 0.755; // Helper Hud Y Position
static tConfItem<REAL> sg_helperHudXC("HELPER_HUD_LOCX", sg_helperHudX);
REAL sg_helperHudY = -0.01; // Helper Hud Y Position
static tConfItem<REAL> sg_helperHudYC("HELPER_HUD_LOCY", sg_helperHudY);
REAL sg_helperHudSize = .055; // Size of Helper Hud
static tConfItem<REAL> sg_helperHudSizeC("HELPER_HUD_SIZE", sg_helperHudSize);
tString sg_helperHudIgnoreList = tString("");
static tConfItem<tString> sg_helperHudIgnoreListConf("HELPER_HUD_IGNORE_LIST", sg_helperHudIgnoreList);

}

using namespace helperConfig;

static std::map< std::string, gHelperHudBase * > * st_confMap = 0;
gHelperHudBase::gHelperHudMap & gHelperHudBase::GetHelperHudMap()
{
    if (!st_confMap)
        st_confMap = tNEW( gHelperHudMap );

    return *st_confMap;
}

gHelperHudBase::gHelperHudBase(int id_, std::string label_, std::string parent_)
{
    std::map<std::string, gHelperHudBase *> &hudMap = gHelperHudBase::GetHelperHudMap();

    if (hudMap.find(label_) != hudMap.end())
        tERR_ERROR_INT("Two gHelperHudBase with the same name " << label_ << "!");

    hudMap[label_] = this;
    parent = parent_;
}

void gHelperHudBase::Render() {
    if (!sg_helperHud)
        return;

    std::map<std::string, std::vector<gHelperHudBase*>> hudMap;
    gHelperHudMap &items = gHelperHudBase::GetHelperHudMap();

    // First, populate the hudMap with all items and their parent relationships
    for (auto iter = items.begin(); iter != items.end(); iter++) {
        gHelperHudBase *item = iter->second;

        std::string parent = item->getParent();

        if (tIsInList(sg_helperHudIgnoreList,item->getLabelStr())){
            continue;
        }

        if (parent.empty()) {
            parent = "";
        }

        if (hudMap.find(parent) == hudMap.end()) {
            hudMap[parent] = std::vector<gHelperHudBase*>();
        }

        hudMap[parent].push_back(item);
    }

    rTextField hudDebug(sg_helperHudX - .15 * sg_helperHudSize / 2.0, sg_helperHudY, .15 * sg_helperHudSize, .3 * sg_helperHudSize);

    // Next, iterate through the hudMap and display parent items first, followed by their child items
    for (auto iter = hudMap.begin(); iter != hudMap.end(); iter++) {
        if (iter->first != "") {
            hudDebug << iter->first << ":\n";
        }
        for (auto item : iter->second) {
            gTextCache<tString,tString> cache;
            if (!(cache.Call(item->getValue(), item->getLastValue()))) {
                rDisplayListFiller filler(cache.list_);
                hudDebug << item->displayString();
                item->setLastValue();
            }
        }
    }
}

