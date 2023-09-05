
#include "../../gCycle.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "gTailHelper.h"

using namespace helperConfig;
namespace helperConfig
{
    bool sg_tailHelper = false;
    static tConfItem<bool> sg_tailHelperC = HelperCommand::tConfItem("HELPER_SELF_TAIL", sg_tailHelper);
    REAL sg_tailHelperBrightness = 1;
    static tConfItem<REAL> sg_tailHelperBrightnessC = HelperCommand::tConfItem("HELPER_SELF_TAIL_BRIGHTNESS", sg_tailHelperBrightness);
    REAL sg_tailHelperHeight = 1;
    static tConfItem<REAL> sg_tailHelperHeightC = HelperCommand::tConfItem("HELPER_SELF_TAIL_HEIGHT", sg_tailHelperHeight);
    REAL sg_tailHelperGridSize = 1;
    static tConfItem<REAL> sg_tailHelperGridSizeC = HelperCommand::tConfItem("HELPER_SELF_TAIL_GRID_SIZE", sg_tailHelperGridSize);
    REAL sg_tailHelperDelay = 0;
    static tConfItem<REAL> sg_tailHelperDelayC = HelperCommand::tConfItem("HELPER_SELF_TAIL_DELAY", sg_tailHelperDelay);
    REAL sg_tailHelperUpdateTime = 1;
    static tConfItem<REAL> sg_tailHelperUpdateTimeC = HelperCommand::tConfItem("HELPER_SELF_TAIL_UPDATE_TIME", sg_tailHelperUpdateTime);
    REAL sg_tailHelperUpdateDistance = 1;
    static tConfItem<REAL> sg_tailHelperUpdateDistanceC = HelperCommand::tConfItem("HELPER_SELF_TAIL_UPDATE_DISTANCE", sg_tailHelperUpdateDistance);
};

gTailHelper::gTailHelper(gHelper &helper, gCycle &owner)
    : helper_(&helper),
      owner_(&owner)
{
    ownerPos = &owner_->pos;
    ownerDir = &owner_->dir;
    tailPos = &owner_->tailPos;
    tailDir = &owner_->tailDir;
    ownerSpeed = &owner_->verletSpeed_;
    turnDelay = owner_->GetTurnDelay();
    updateTime = -100;
}
#include <queue>
#include <unordered_map>
#include <tuple>

struct eCoordHash
{
    std::size_t operator()(const eCoord &coord) const
    {
        return std::hash<double>{}(coord.x) ^ std::hash<double>{}(coord.y);
    }
};

struct eCoordEqual
{
    bool operator()(const eCoord &lhs, const eCoord &rhs) const
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

struct CompareDist
{
    bool operator()(const std::tuple<int, eCoord> &lhs, const std::tuple<int, eCoord> &rhs) const
    {
        return std::get<0>(lhs) > std::get<0>(rhs);
    }
};

// Manhattan distance heuristic
int manhattanDist(const eCoord &a, const eCoord &b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

REAL angleBetween(const eCoord &a, const eCoord &b)
{
    REAL dotProduct = a * b;
    REAL lenA = a.Norm();
    REAL lenB = b.Norm();
    return acos(dotProduct / (lenA * lenB));
}

std::vector<eCoord> gTailHelper::getPathToTail()
{
    std::vector<eCoord> path;
    std::priority_queue<std::tuple<int, eCoord>, std::vector<std::tuple<int, eCoord>>, CompareDist> pq;
    std::unordered_map<eCoord, eCoord, eCoordHash, eCoordEqual> prev;

    eCoord startPos = discretize(*ownerPos, sg_tailHelperGridSize);
    eCoord targetPos = discretize(*tailPos, sg_tailHelperGridSize);

    int initialDist = manhattanDist(startPos, targetPos);
    pq.push(std::make_tuple(initialDist, startPos));
    prev[startPos] = startPos;

    std::array<eCoord, 4> directions = {
        eCoord(1, 0),
        eCoord(0, 1),
        eCoord(-1, 0),
        eCoord(0, -1)};

    while (!pq.empty())
    {
        eCoord current = std::get<1>(pq.top());
        pq.pop();

        if (current == targetPos)
        {
            break;
        }

        for (const eCoord &dir : directions)
        {
            eCoord next = current + dir;
            if (prev.find(next) == prev.end())
            {
                int dist = manhattanDist(next, targetPos);
                pq.push(std::make_tuple(dist, next));
                prev[next] = current;
            }
        }
    }

    if (prev.find(targetPos) == prev.end())
    {
        return path; // Path not found
    }

    // Reconstruct path
    for (eCoord pos = targetPos; pos != startPos; pos = prev[pos])
    {
        path.push_back(pos);
    }

    std::reverse(path.begin(), path.end());

    return path;
}

void gTailHelper::Activate(gHelperData &data)
{
    if (!owner_->tailMoving)
        return;

    if (updateTime <= helper_->CurrentTime() || path.size() < 1)
    {
        path = getPathToTail();
        updateTime = helper_->CurrentTime() + sg_tailHelperUpdateTime;
    }

    if (path.size() < 1)
        return;

    REAL timeout = data.ownerData.speedFactorF();

    if (sg_tailHelperUpdateTime > timeout)
        timeout = sg_tailHelperUpdateTime;

    eCoord lastPos = *ownerPos;
    for (int i = 0; i < path.size(); i++)
    {
        gHelperUtility::debugLine(tColor(1, 0, 0), sg_tailHelperHeight, timeout, lastPos, path[i], sg_tailHelperBrightness);
        lastPos = path[i];
    }
}

gTailHelper &gTailHelper::Get(gHelper &helper, gCycle &owner)
{
    tASSERT(&owner);
    // create
    if (helper.tailHelper.get() == 0)
        helper.tailHelper.reset(new gTailHelper(helper, owner));
    return *helper.tailHelper;
}
