#include "../../gZone.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "../gHelperHud.h"
using namespace helperConfig;

gHelperHudItem<tColoredString> zoneDebugH("Zone Debug", tColoredString("None"));

void debugZone(gZone *zone)
{
    REAL zoneVelocity = zone->GetVelocity().Norm();
    gHelper::debugBox(zone->GetColor().r, zone->GetColor().g, zone->GetColor().b, zone->Position(), zone->GetRadius(), zoneVelocity > 0 ? 1 / zoneVelocity : .005);
}

gZoneHelper::gZoneHelper(gHelper *helper, gCycle *owner)
    : helper_(helper),
      owner_(owner)
{
}

void gZoneHelper::Activate(gHelperData &data)
{
    if (sg_HelperTrackedZones.size() <= 0)
        return;
    tColoredString debug;
    debug << "Zones size " << sg_HelperTrackedZones.size() << "\n";
    for (std::deque<gZone *>::const_iterator i = sg_HelperTrackedZones.begin(); i != sg_HelperTrackedZones.end(); ++i)
    {
        gZone *zone = *i;
        if (!zone || zone->destroyed_)
            continue;
        bool zoneIsOurs = zone->Team() == owner_->Team();
        if (!sg_zoneHelperOwnerOnly || sg_zoneHelperOwnerOnly && zoneIsOurs)
        {
            debugZone(zone);
            // if (zoneIsOurs) {
            //     gBaseZoneHack *baseZone = dynamic_cast<gBaseZoneHack *>(zone);
            //     if (baseZone){
            //         debug << "Zone base State " << baseZone->currentState_ << "\n";
            //         debug << "Zone base conquered_ " << baseZone->conquered_ << "\n";
            //     }
            // }
        }
    }
    zoneDebugH << debug;
}

gZoneHelper &gZoneHelper::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(owner);

    // create
    if (helper->zoneHelper.get() == 0)
        helper->zoneHelper.reset(new gZoneHelper(helper, owner));

    return *helper->zoneHelper;
}