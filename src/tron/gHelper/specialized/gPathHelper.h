#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"
#ifndef ArmageTron_GHELPER_PATHHELPER
#define ArmageTron_GHELPER_PATHHELPER

class gPathHelper
{
public:
    // Constructor
    gPathHelper(gHelper *helper, gCycle *owner);

    bool targetExist();

    void FindPath(gHelperData &data);

    void Activate(gHelperData &data);

    bool autoMode(gHelperData orig_data);
    bool enemyMode(gHelperData orig_data);
    bool tailMode(gHelperData orig_data);
    bool cornerMode(gHelperData orig_data);
    bool DistanceCheck(gHelperData &data);
    bool UpdateTimeCheck(gHelperData &data);
    void RenderPath(gHelperData &data);
    void RenderTurn(gHelperData &data);

    static gPathHelper &Get(gHelper *helper, gCycle *owner);

private:
    eCoord target;
    tJUST_CONTROLLED_PTR<eFace> targetCurrentFace_;
    gCycle *owner_; // Pointer to the owner cycle
    gHelper *helper_;
    REAL lastTime;
    REAL nextTime;
    REAL lastPath;
    eCoord lastPos;
    ePath path_;
    bool pathInvalid;
};

#endif
