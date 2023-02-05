#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"
#ifndef ArmageTron_GHELPER_ZONEHELPER
#define ArmageTron_GHELPER_ZONEHELPER

class gZoneHelper
{
public:
    // Constructor
    gZoneHelper(gHelper *helper, gCycle *owner);
    void Activate(gHelperData &data);

    static gZoneHelper &Get(gHelper *helper, gCycle *owner);

private:
    gCycle *owner_; // Pointer to the owner cycle
    gHelper *helper_;
};
#endif