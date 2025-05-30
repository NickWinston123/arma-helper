#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"
#ifndef ArmageTron_GHELPER_PATHHELPER
#define ArmageTron_GHELPER_PATHHELPER


enum gPathHelperMode
{
    AUTO = 0,
    TAIL = 1,
    ENEMY = 2,
    CORNER = 3,
};

class gPathHelper
{
public:
    // Constructor
    gPathHelper(gHelper &helper, gCycle &owner);

    bool targetExist();

    void FindPath(gHelperData &data);

    bool autoMode(gHelperData &orig_data);
    bool enemyMode(gHelperData &orig_data);
    bool tailMode(gHelperData &orig_data);
    bool cornerMode(gHelperData &orig_data);
    bool DistanceCheck(gHelperData &data);
    bool UpdateTimeCheck(gHelperData &data);
    void RenderPath(gHelperData &data);
    void RenderTurn(gHelperData &data);

    void Activate(gHelperData &data);

    tColoredString ModeString();
    REAL DistanceToTarget();

    static gPathHelper &Get(gHelper &helper, gCycle &owner);

private:
    gCycle &owner_; 
    gHelper &helper_;

    eCoord target;
    tJUST_CONTROLLED_PTR<eFace> targetCurrentFace_;

    REAL pathUpdateTime;
    REAL pathUpdatedTime;

    eCoord lastPos;
    ePath path_;
};

#endif
