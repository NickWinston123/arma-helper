#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"
#ifndef ArmageTron_GHELPER_TAILHELPER
#define ArmageTron_GHELPER_TAILHELPER

class gTailHelper
{
public:
    // Constructor
    gTailHelper(gHelper &helper, gCycle &owner);

    void Activate(gHelperData &data);
    std::vector<eCoord> getPathToTail();
    static gTailHelper &Get(gHelper &helper, gCycle &owner);

private:
    gCycle *owner_; // Pointer to the owner cycle
    gHelper *helper_;
    eCoord *ownerPos;
    eCoord *ownerDir;
    eCoord *tailPos;
    eCoord *tailDir;
    REAL turnDelay;
    REAL *ownerSpeed;
    std::vector<eCoord> path;
    REAL updateTime;

    static eCoord discretize(const eCoord& position, REAL gridSize)
    {
        return eCoord(std::round(position.x / gridSize) * gridSize, std::round(position.y / gridSize) * gridSize);
    }

};

#endif
