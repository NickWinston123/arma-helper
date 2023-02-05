
#include "../../gCycle.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "gTailHelper.h"

using namespace helperConfig;

gTailHelper::gTailHelper(gHelper *helper, gCycle *owner)
    : helper_(helper),
      owner_(owner)
{
    ownerPos = &owner_->pos;
    ownerDir = &owner_->dir;
    tailPos = &owner_->tailPos;
    tailDir = &owner_->tailDir;
    ownerSpeed = &owner_->verletSpeed_;
    turnDelay = owner_->GetTurnDelay();
    updateTime = -100;
}

// std::vector<eCoord> gTailHelper::getPathToTail() {
//     std::vector<eCoord> path;

//     REAL delay = turnDelay + sg_tailHelperDelay;
//     eCoord direction = (*tailPos - *ownerPos);  // Initial direction towards tail
//     double dist = direction.Norm();
//     int numSegments = (int)(dist / delay);

//     // Get the angle between the tail direction and the initial direction
//     double tailAngle = atan2(tailDir->y, tailDir->x);
//     double initAngle = atan2(direction.y, direction.x);
//     double deltaAngle = tailAngle - initAngle;

//     // Divide the total angle by the number of segments to get the angle for each segment
//     double segmentAngle = deltaAngle / numSegments;

//     // Add the first point to the path
//     path.push_back(*ownerPos);
//     eCoord currentPos = *ownerPos;

//     // Create a path of turns to the tail
//     for (int i = 1; i < numSegments; i++) {
//         double currentAngle = atan2(direction.y, direction.x);
//         currentPos += direction*delay;
//         path.push_back(currentPos);
//         direction = eCoord(cos(currentAngle + segmentAngle), sin(currentAngle + segmentAngle));  // rotate direction by the segment angle
//     }

//     // Check if last segment is shorter than delay
//     if ((*tailPos - currentPos).Norm() > 1e-6) {
//         // Add the last point to the path
//         path.push_back(*tailPos);
//     }
//     return path;
// }

std::vector<eCoord> gTailHelper::getPathToTail()
{
    std::vector<eCoord> path;

    eCoord direction = (*tailPos - *ownerPos); // Initial direction towards tail
    double dist = direction.Norm();
    int numSegments = (int)(dist / sg_tailHelperDelay);

    // align initial direction with the grid
    double angle = atan2(direction.y, direction.x);
    angle = round(angle / (M_PI / 2)) * (M_PI / 2);
    direction = eCoord(cos(angle), sin(angle));

    direction.Normalize();

    // Add the first point to the path
    path.push_back(*ownerPos);
    eCoord currentPos = *ownerPos;

    // Create a path of 90-degree turns to the tail
    for (int i = 1; i < numSegments; i++)
    {
        double segmentAngle = atan2(direction.y, direction.x);
        currentPos += direction * sg_tailHelperDelay;
        path.push_back(currentPos);
        direction = eCoord(cos(segmentAngle + (i % 2 == 1 ? M_PI / 2 : -M_PI / 2)), sin(segmentAngle + (i % 2 == 1 ? M_PI / 2 : -M_PI / 2))); // rotate direction by 90 degrees
    }

    // Check if last segment is shorter than delay
    if ((*tailPos - currentPos).Norm() > 1e-6)
    {
        // Add the last point to the path
        path.push_back(*tailPos);
    }
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
        gHelper::debugLine(gRealColor(1, 0, 0), 1, data.speedFactorF(), lastPos, path[i], sg_tailHelperBrightness);
        lastPos = path[i];
    }
}

gTailHelper &gTailHelper::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(owner);
    // create
    if (helper->tailHelper.get() == 0)
        helper->tailHelper.reset(new gTailHelper(helper, owner));
    return *helper->tailHelper;
}