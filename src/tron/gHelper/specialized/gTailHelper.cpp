
#include "../../gCycle.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "gTailHelper.h"

using namespace helperConfig;

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

struct eCoordHash {
    std::size_t operator()(const eCoord& coord) const
    {
        return std::hash<double>{}(coord.x) ^ std::hash<double>{}(coord.y);
    }
};

struct eCoordEqual {
    bool operator()(const eCoord& lhs, const eCoord& rhs) const
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

struct CompareDist {
    bool operator()(const std::tuple<int, eCoord>& lhs, const std::tuple<int, eCoord>& rhs) const {
        return std::get<0>(lhs) > std::get<0>(rhs);
    }
};


// Manhattan distance heuristic
int manhattanDist(const eCoord& a, const eCoord& b)
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

    eCoord startPos = discretize(*ownerPos,sg_tailHelperGridSize);
    eCoord targetPos = discretize(*tailPos,sg_tailHelperGridSize);

    int initialDist = manhattanDist(startPos, targetPos);
    pq.push(std::make_tuple(initialDist, startPos));
    prev[startPos] = startPos;

    std::array<eCoord, 4> directions = {
        eCoord(1, 0),
        eCoord(0, 1),
        eCoord(-1, 0),
        eCoord(0, -1)
    };

    while (!pq.empty())
    {
        eCoord current = std::get<1>(pq.top());
        pq.pop();

        if (current == targetPos)
        {
            break;
        }

        for (const eCoord& dir : directions)
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

    eCoord lastPos = *ownerPos;
    for (int i = 0; i < path.size(); i++)
    {
        gHelperUtility::debugLine(gRealColor(1, 0, 0), sg_tailHelperHeight, data.ownerData.speedFactorF(), lastPos, path[i], sg_tailHelperBrightness);
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
