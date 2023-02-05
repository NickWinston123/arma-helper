#include "../../gSensor.h"
#include "../../gCycle.h"
#include "../../gWall.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"

using namespace helperConfig;

extern REAL sg_cycleBrakeRefill;
extern REAL sg_cycleBrakeDeplete;
extern void sg_RubberValues(ePlayerNetID const *player, REAL speed, REAL &max, REAL &effectiveness);
extern REAL sg_brakeCycle;
extern REAL sg_cycleBrakeDeplete;

class Sensor : public gSensor
{
public:
    Sensor(gCycle *o, const eCoord &start, const eCoord &d)
        : gSensor(o, start, d), hitOwner_(0), hitTime_(0),
          hitDistance_(o->MaxWallsLength()), lrSuggestion_(0),
          windingNumber_(0), owner_(o)
    {
        if (hitDistance_ <= 0)
            hitDistance_ = o->GetDistance();
    }

    virtual void PassEdge(const eWall *ww, REAL time, REAL a, int r)
    {
        try
        {
            gSensor::PassEdge(ww, time, a, r);
        }
        catch (eSensorFinished &e)
        {
            if (DoExtraDetectionStuff())
                throw;
        }
    }

    bool DoExtraDetectionStuff()
    {
        // move towards the beginning of a wall
        lrSuggestion_ = -lr;

        switch (type)
        {
        case gSENSOR_NONE:
        case gSENSOR_RIM:
            lrSuggestion_ = 0;
            return true;
        default:
        // unless it is an enemy, follow his wall instead (uncomment for a nasty
        // cowardy campbot) lrSuggestion *= -1;
        case gSENSOR_SELF:
        {
            // determine whether we're hitting the front or back half of his wall
            if (!ehit)
                return true;
            eWall *wall = ehit->GetWall();
            if (!wall)
                return true;
            gPlayerWall *playerWall = dynamic_cast<gPlayerWall *>(wall);
            if (!playerWall)
                return true;
            hitOwner_ = playerWall->Cycle();
            if (!hitOwner_)
                return true;

            REAL wallAlpha = playerWall->Edge()->Ratio(before_hit);
            // that's an unreliable source
            if (wallAlpha < 0)
                wallAlpha = 0;
            if (wallAlpha > 1)
                wallAlpha = 1;
            hitDistance_ = hitOwner_->GetDistance() - playerWall->Pos(wallAlpha);
            hitTime_ = playerWall->Time(wallAlpha);
            windingNumber_ = playerWall->WindingNumber();

            // don't see new walls
            if (hitTime_ > hitOwner_->LastTime() &&
                hitOwner_ != owned)
            {
                ehit = NULL;
                hit = 1E+40;
                return false;
            }
        }
        }

        return true;
    }

    // check how far the hit wall extends straight into the given direction
    REAL HitWallExtends(eCoord const &dir, eCoord const &origin)
    {
        if (!ehit || !ehit->Other())
        {
            return 1E+30;
        }

        REAL ret = -1E+30;
        eCoord ends[2] = {*ehit->Point(), *ehit->Other()->Point()};
        for (int i = 1; i >= 0; --i)
        {
            REAL newRet = eCoord::F(dir, ends[i] - origin);
            if (newRet > ret)
                ret = newRet;
        }

        return ret;
    }

    gCycle *owner_;
    gCycle *hitOwner_;   // the owner of the hit wall
    REAL hitTime_;       // the time the hit wall was built at
    REAL hitDistance_;   // the distance of the wall to the cycle that built it
    short lrSuggestion_; // sensor's oppinon on whether moving to the left or
                         // right of the hit wall is recommended (-1 for left,
                         // +1 for right)
    int windingNumber_;  // the number of turns (with sign) the cycle has taken
};


gHelperEmergencyTurn &gHelperEmergencyTurn::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(helper);

    // create
    if (helper->smartTurning->emergencyTurn.get() == 0)
        helper->smartTurning->emergencyTurn.reset(new gHelperEmergencyTurn(helper, owner));

    return *helper->smartTurning->emergencyTurn;
}

int gHelperEmergencyTurn::ActToTurn(uActionPlayer *action)
{
    return action == &gCycle::se_turnLeft ? -1 : 1;
}

template <class T>
void gHelperEmergencyTurn::BotDebug(std::string description, T value)
{
    HelperDebug::Debug("gHelperEmergencyTurn", description, &value);
}

void gHelperEmergencyTurn::BotDebug(std::string description)
{
    HelperDebug::Debug("gHelperEmergencyTurn", description, "");
}

// determines the distance between two sensors; the size should give the
// likelyhood to survive if you pass through a gap between the two selected
// walls
REAL gHelperEmergencyTurn::Distance(Sensor const &a, Sensor const &b)
{
    // make sure a is left from b
    if (a.Direction() * b.Direction() < 0)
        return Distance(b, a);

    bool self = a.type == gSENSOR_SELF || b.type == gSENSOR_SELF;
    bool rim = a.type == gSENSOR_RIM || b.type == gSENSOR_RIM;

    // avoid. own. walls.
    REAL selfHatred = 1;
    if (a.type == gSENSOR_SELF)
    {
        selfHatred *= .5;
        if (a.lr > 0)
        {
            selfHatred *= .5;
            if (b.type == gSENSOR_RIM)
                selfHatred *= .25;
        }
    }
    if (b.type == gSENSOR_SELF)
    {
        selfHatred *= .5;
        if (b.lr < 0)
        {
            selfHatred *= .5;
            if (a.type == gSENSOR_RIM)
                selfHatred *= .25;
        }
    }

    // some big distance to return if we don't know anything better
    REAL bigDistance = owner_->MaxWallsLength();
    if (bigDistance <= 0)
        bigDistance = owner_->GetDistance();

    if (a.hitOwner_ != b.hitOwner_)
    {
        // different owners? Great, there has to be a way through!
        REAL ret = a.hitDistance_ + b.hitDistance_;

        if (rim)
        {
            ret = bigDistance * .001 + ret * .01 +
                    (a.before_hit - b.before_hit).Norm();

            // we love going between the rim and enemies
            if (!self)
                ret = bigDistance * 2;
        }

        // minimal factor should be 1, this path should never return something
        // smaller than the paths where only one cycle's walls are hit
        ret *= 16;

        // or empty space
        if (a.type == gSENSOR_NONE || b.type == gSENSOR_NONE)
            ret *= 2;

        return ret * selfHatred;
    }
    else if (rim)
    {
        // at least one rim wall? Take the distance between the hit positions.
        return (a.before_hit - b.before_hit).Norm() * selfHatred;
    }
    else if (a.type == gSENSOR_NONE && b.type == gSENSOR_NONE)
    {
        // empty space! Woo!
        return owner_->GetDistance() * 256;
    }
    else if (a.lr != b.lr)
    {
        // different directions? Also great!
        return (fabsf(a.hitDistance_ - b.hitDistance_) + .25 * bigDistance) *
                selfHatred;
    }

    else if (-2 * a.lr * (a.windingNumber_ - b.windingNumber_) >
                owner_->Grid()->WindingNumber())
    {
        // this looks like a way out to me
        return fabsf(a.hitDistance_ - b.hitDistance_) * 10 * selfHatred;
    }

    else
    {
        // well, the longer the wall segment between the two points, the better.
        return fabsf(a.hitDistance_ - b.hitDistance_) * selfHatred;
    }

    // default: hit distance
    return (a.before_hit - b.before_hit).Norm() * selfHatred;
}

// does the main thinking
gTurnData* gHelperEmergencyTurn::getTurn()
{
    turnData->set(false);

    REAL lookahead = 100; // seconds to plan ahead
    REAL minstep = 0;     // minimum timestep between thoughts in seconds

    // cylce data
    REAL speed = owner_->Speed();
    eCoord dir = owner_->Direction();
    eCoord pos = owner_->Position();

    REAL range = speed * lookahead;
    eCoord scanDir = dir * range;

    REAL frontFactor = 1;

    Sensor front(owner_, pos, scanDir);
    front.detect(frontFactor);

    if (!front.ehit)
    {
        return turnData;
    }

    // owner_->enemyInfluence.AddSensor( front, 0, owner_ );

    REAL minMoveOn = 0, maxMoveOn = 0, moveOn = 0;

    // get extra time we get through rubber usage
    helper_->rubberData->calculate();
    REAL rubberTime = helper_->rubberData->rubberTimeLeft;
    REAL rubberRatio = helper_->rubberData->rubberUsedRatio;

    // these checks can hit our last wall and fail. Temporarily set it to NULL.
    tJUST_CONTROLLED_PTR<gNetPlayerWall> lastWall = owner_->getLastWall();
    owner_->setLastWall(NULL);

    REAL narrowFront = 1;

    // cast four diagonal rays
    Sensor forwardLeft(owner_, pos, scanDir.Turn(+1, +1));
    forwardLeft.detect(5);

    Sensor backwardLeft(owner_, pos, scanDir.Turn(-1, +narrowFront));
    backwardLeft.detect(5);

    Sensor forwardRight(owner_, pos, scanDir.Turn(+1, -1));
    forwardRight.detect(5);

    Sensor backwardRight(owner_, pos, scanDir.Turn(-1, -narrowFront));
    backwardRight.detect(5);

    // determine survival chances in the four directions
    REAL frontOpen = Distance(forwardLeft, forwardRight);
    REAL leftOpen = Distance(forwardLeft, backwardLeft);
    REAL rightOpen = Distance(forwardRight, backwardRight);
    REAL rearOpen = Distance(backwardLeft, backwardRight);

    Sensor self(owner_, pos, scanDir.Turn(-1, 0));

    // fake entries
    self.before_hit = pos;
    self.windingNumber_ = owner_->getWindingNumber();
    self.type = gSENSOR_SELF;
    self.hitDistance_ = 0;
    self.hitOwner_ = owner_;
    self.hitTime_ = owner_->LastTime(); // localCurrentTime;
    self.lr = -1;
    REAL rearLeftOpen = Distance(backwardLeft, self);
    self.lr = 1;
    REAL rearRightOpen = Distance(backwardRight, self);

    {
        // override: don't camp (too much)
        if (forwardRight.type == gSENSOR_SELF &&
            forwardLeft.type == gSENSOR_SELF &&
            backwardRight.type == gSENSOR_SELF &&
            backwardLeft.type == gSENSOR_SELF &&
            front.type == gSENSOR_SELF &&
            forwardRight.lr == front.lr &&
            forwardLeft.lr == front.lr &&
            backwardRight.lr == front.lr &&
            backwardLeft.lr == front.lr &&
            frontOpen + leftOpen + rightOpen < owner_->GetDistance() * .5)
        {
            // turnedRecently_ = true;
            if (front.lr > 0)
            {
                if (leftOpen > minstep * speed)
                    // force a turn to the left
                    rightOpen = 0;
                else if (front.hit * range < 2 * minstep)
                    // force a preliminary turn to the right that will allow us to reverse
                    frontOpen = 0;
            }
            else
            {
                if (rightOpen > minstep * speed)
                    // force a turn to the right
                    leftOpen = 0;
                else if (front.hit * range < 2 * minstep)
                    // force a preliminary turn to the left that will allow us to reverse
                    frontOpen = 0;
            }
        }
    }

    // override rim hugging
    if (forwardRight.type == gSENSOR_SELF &&
        forwardLeft.type == gSENSOR_RIM &&
        backwardRight.type == gSENSOR_SELF &&
        backwardLeft.type == gSENSOR_RIM &&
        // backwardLeft.hit < .1 &&
        forwardRight.lr == -1 &&
        backwardRight.lr == -1)
    {
        // turnedRecently_ = true;
        if (rightOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
        {
            turnData->set(RIGHT, 2, "right by override rim hugging 1");
            return turnData;

            // owner_->Act( &gCycle::se_turnRight, 1 );
            // owner_->Act( &gCycle::se_turnRight, 1 );
        }
        else if (leftOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
        {
            turnData->set(LEFT, 2, "left 2 by override rim hugging 1");
            return turnData;
            // owner_->Act( &gCycle::se_turnLeft, 1 );
            // owner_->Act( &gCycle::se_turnLeft, 1 );
        }
    }

    if (forwardLeft.type == gSENSOR_SELF &&
        forwardRight.type == gSENSOR_RIM &&
        backwardLeft.type == gSENSOR_SELF &&
        backwardRight.type == gSENSOR_RIM &&
        // backwardRight.hit < .1 &&
        forwardLeft.lr == 1 &&
        backwardLeft.lr == 1)
    {
        // turnedRecently_ = true;
        if (leftOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
        {
            turnData->set(LEFT, 2, "left 2 by override rim hugging 2");
            return turnData;
            // owner_->Act( &gCycle::se_turnLeft, 1 );
            // owner_->Act( &gCycle::se_turnLeft, 1 );
        }
        else if (rightOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
        {
            turnData->set(RIGHT, 2, "right 2 by override rim hugging 2");
            return turnData;
        }
    }

    // get the best turn direction
    uActionPlayer *bestAction = (leftOpen > rightOpen) ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
    int bestDir = (leftOpen > rightOpen) ? 1 : -1;
    REAL bestOpen = (leftOpen > rightOpen) ? leftOpen : rightOpen;
    Sensor &bestForward = (leftOpen > rightOpen) ? forwardLeft : forwardRight;
    Sensor &bestBackward = (leftOpen > rightOpen) ? backwardLeft : backwardRight;

    Sensor direct(owner_, pos, scanDir.Turn(0, bestDir));
    direct.detect(5);

    // restore last wall
    owner_->setLastWall(lastWall);

    // only turn if the hole has a shape that allows better entry after we do a zig-zag, or if we're past the good turning point
    // see how the survival chance is distributed between forward and backward half
    REAL forwardHalf = Distance(direct, bestForward);
    REAL backwardHalf = Distance(direct, bestBackward);

    REAL forwardOverhang = bestForward.HitWallExtends(bestForward.Direction(), pos);
    REAL backwardOverhang = bestBackward.HitWallExtends(bestForward.Direction(), pos);

    // we have to move forward this much before we can hope to turn
    minMoveOn = bestBackward.HitWallExtends(dir, pos);

    // maybe the direct to the side sensor is better?
    REAL minMoveOnOther = direct.HitWallExtends(dir, pos);

    // determine how far we can drive on
    maxMoveOn = bestForward.HitWallExtends(dir, pos);
    REAL maxMoveOnOther = front.HitWallExtends(dir, pos);
    if (maxMoveOn > maxMoveOnOther)
        maxMoveOn = maxMoveOnOther;

    if (maxMoveOn > minMoveOnOther && forwardHalf > backwardHalf && direct.hitOwner_ == bestBackward.hitOwner_)
    {
        backwardOverhang = direct.HitWallExtends(bestForward.Direction(), pos);
        minMoveOn = minMoveOnOther;
    }

    // best place to turn
    moveOn = .5 * (minMoveOn * (1 + rubberRatio) + maxMoveOn * (1 - rubberRatio));

    // hit the brakes before you hit anything and if it's worth it
    bool brake = sg_brakeCycle > 0 &&
                    front.hit * lookahead * sg_cycleBrakeDeplete < owner_->GetBrakingReservoir() &&
                    sg_brakeCycle * front.hit * lookahead < 2 * speed * owner_->GetBrakingReservoir() &&
                    (maxMoveOn - minMoveOn) > 0 &&
                    owner_->GetBrakingReservoir() * (maxMoveOn - minMoveOn) < speed * owner_->GetTurnDelay();

    if (frontOpen < bestOpen &&
        (forwardOverhang <= backwardOverhang || (minMoveOn < 0 && moveOn < minstep * speed)))
    {
        minMoveOn = maxMoveOn = moveOn = 0;
        {

            turnData->set(ActToTurn(bestAction), 1, "frontOpen < bestOpen bestAction ");
            return turnData;
        }

        brake = false;
    }
    else
    {
        // the best
        REAL bestSoFar = frontOpen > bestOpen ? frontOpen : bestOpen;
        bestSoFar *= (10 * (1 - rubberRatio) + 1);

        if (rearOpen > bestSoFar && (rearLeftOpen > bestSoFar || rearRightOpen > bestSoFar))
        {
            brake = false;
            // turnedRecently_ = true;

            bool goLeft = rearLeftOpen > rearRightOpen;

            // dead end. reverse into the opposite direction of the front wall
            uActionPlayer *bestAction = goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
            uActionPlayer *otherAction = !goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
            Sensor &bestForward = goLeft ? forwardLeft : forwardRight;
            Sensor &bestBackward = goLeft ? backwardLeft : backwardRight;
            Sensor &otherForward = !goLeft ? forwardLeft : forwardRight;
            Sensor &otherBackward = !goLeft ? backwardLeft : backwardRight;

            // space in the two directions available for turns
            REAL bestHit = bestForward.hit > bestBackward.hit ? bestBackward.hit : bestForward.hit;
            REAL otherHit = otherForward.hit > otherBackward.hit ? otherBackward.hit : otherForward.hit;

            bool wait = false;

            // well, after a short turn to the right if space is tight
            if (bestHit * lookahead < owner_->GetTurnDelay() + rubberTime)
            {
                if (otherHit < bestForward.hit * 2 && front.hit * lookahead > owner_->GetTurnDelay() * 2)
                {
                    // wait a bit, perhaps there will be a better spot
                    wait = true;
                    BotDebug("WAITING");
                }
                else if (!wait)
                {
                    {

                        turnData->set(ActToTurn(otherAction), 1, "rearOpen > bestSoFar ootherAction");
                        return turnData;
                    }
                }
            }

            if (!wait)
            {
                turnData->set(ActToTurn(bestAction), 1, "not waiting bestAction");
                return turnData;
            }

            minMoveOn = maxMoveOn = moveOn = 0;
        }
    }

    // execute brake command
    owner_->Act(&gCycle::s_brake, brake ? 1 : -1);

    // BotDebug("NO TURN FOUND", noTurns);
    return turnData;
}


// SmartTurning
gSmartTurning::gSmartTurning(gHelper *helper, gCycle *owner)
    : helper_(helper),
      owner_(owner),
      lastTurnAttemptTime(owner_->lastTurnAttemptTime),
      lastTurnAttemptDir(owner_->lastTurnAttemptDir),
      lastTurnTime(owner_->lastTurnTime),
      turnIgnoreTime(owner_->turnIgnoreTime),
      lastTurnDir(owner_->lastTurnDir), // 0 none, -1 left , 1 right
      blockTurn(owner_->blockTurn),     // 0 = NONE, -1 = LEFT, 1 = RIGHT, 2 = BOTH
      forceTurn(owner_->forceTurn),
      lastTailTurnTime(-999) // 0 = NONE, -1 = LEFT, 1 = RIGHT

{
    gHelperEmergencyTurn::Get(helper_, owner_);
}

void gSmartTurning::Activate(gHelperData &data)
{
    if (sg_helperSmartTurningSurvive)
    {
        sg_helperSmartTurningOpposite = 0;
        sg_helperSmartTurningPlan = 0;
        smartTurningSurvive(data);
    }

    if (sg_helperSmartTurningOpposite)
    {
        sg_helperSmartTurningSurvive = 0;
        sg_helperSmartTurningPlan = 0;
        smartTurningOpposite(data);
    }

    if (sg_helperSmartTurningPlan)
    {
        sg_helperSmartTurningSurvive = 0;
        sg_helperSmartTurningOpposite = 0;
        smartTurningPlan(data);
    }

    if (sg_helperSmartTurningFollowTail)
        followTail(data);

    if (sg_helperSmartTurningFrontBot)
        smartTurningFrontBot(data);
}

// Function: isClose
// Check if the distance between a point and the player is within a certain range.
//
// Parameters:
// pos: The position to check distance to.
// closeFactor: The maximum allowed distance to be considered close.
//
// Returns:
// bool: True if the distance is within the closeFactor range, False otherwise.
bool gSmartTurning::isClose(eCoord pos, REAL closeFactor)
{
    eCoord distanceToPos = owner_->pos - pos;
    return distanceToPos.NormSquared() < closeFactor * closeFactor;
}

// Automatically turn to trace corners
void gSmartTurning::smartTurningPlan(gHelperData &data)
{
    // Check if the player is alive.
    if (!helper_->aliveCheck())
    {
        return;
    }

    // If there are no corners found, find them.
    if (!helper_->leftCorner.exist && !helper_->rightCorner.exist)
    {
        helper_->findCorners(data);
        return;
    }

    // Select the corner closest to the player.
    gSmartTurningCornerData cornerToUse = helper_->leftCorner;
    int dir = LEFT;
    if (cornerToUse.exist && helper_->rightCorner.exist)
    {
        bool useLeftCorner = cornerToUse.distanceFromPlayer < helper_->rightCorner.distanceFromPlayer ? true : false;
        cornerToUse = useLeftCorner ? cornerToUse : helper_->rightCorner;
        dir = useLeftCorner ? LEFT : RIGHT;
    }
    else if (!cornerToUse.exist && helper_->rightCorner.exist)
    {
        cornerToUse = helper_->rightCorner;
        dir = RIGHT;
    }

    // Check if the player is close to the selected corner.
    bool close = isClose(cornerToUse.currentPos, sg_helperShowCornersBoundary);
    if (!close)
    {
        return;
    }

    // Make a turn if possible.
    if (cornerToUse.infront)
    {
        if (makeTurnIfPossible(data, dir, 5))
        {
            HelperDebug::Debug("SMART TURNING PLAN", std::string("MADE TURN: ") + ((dir == LEFT) ? "LEFT" : "RIGHT"), "");
        }
    }
}


/**
 * This function implements the behavior of following the owner's tail.
 * If the tail is behind the cycle, the cycle will turn towards the tail if there is enough space.
 * If the tail is in front of the cycle, the cycle will continue on its current path.
 *
 * @param data A reference to a gHelperData object that contains information about the cycle and the game state.
 */
void gSmartTurning::followTail(gHelperData &data)
{
    // If the cycle is not alive, or the tail is not moving, or the tail is not close to the cycle, return
    if (!helper_->aliveCheck() || !owner_->tailMoving ||
       (!isClose(owner_->tailPos, data.turnSpeedFactorF() * 3)))
    {
        return;
    }

    // Calculate the delay between turning actions
    REAL delay = data.turnSpeedFactorF() * sg_helperSmartTurningFollowTailDelayMult;

    // Check if the cycle is driving straight
    bool drivingStraight = helper_->drivingStraight();

    // Check if the cycle can survive turning left or right
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    bool closedIn, blockedBySelf;
    canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningFollowTailFreeSpaceMult);

    // If the cycle is closed in or blocked by itself, return
    if (closedIn || blockedBySelf)
    {
        return;
    }

    // Calculate the direction from the cycle to the tail
    eCoord directionToTail = owner_->tailPos - owner_->pos;
    directionToTail.Normalize();
    int turnDirection = NONE;
    // Determine if the tail is in front of or behind the cycle
    if (directionToTail * owner_->dir < 0)
    {
        // The tail is behind the cycle, turn towards it

        // Check if there are walls to the left or right of the cycle
        gSensor leftSensor(owner_, owner_->pos, owner_->dir.Turn(0, 1));
        leftSensor.detect(sg_helperSensorRange);
        gSensor rightSensor(owner_, owner_->pos, owner_->dir.Turn(0, -1));
        rightSensor.detect(sg_helperSensorRange);

        // Check if the tail is to the right or left of the cycle
        if (directionToTail * owner_->dir > 0 && canSurviveRightTurn && rightSensor.hit > .999999)
        {
            turnDirection = RIGHT;
        }
        else if (canSurviveLeftTurn && leftSensor.hit > .999999)
        {
            turnDirection = LEFT;
        }
        if (turnDirection == NONE)
            return;
    }
    else
    {
        // The tail is in front of the cycle, stay on current path

        // The tail is directly in front of or behind the cycle
        // Stay on current path
    }

    // Check if the cycle has turned recently
    bool turnedRecently = !(lastTailTurnTime < 0) && owner_->lastTurnTime + delay > helper_->CurrentTime();

    // Check if the cycle is ready to turn again
    bool readyToTurn = helper_->CurrentTime() > lastTailTurnTime + delay;

    // If the cycle has not turned recently or if the cycle is not ready to turn, return
    if (!readyToTurn)
    {
        return;
    }

    // If the turn direction is LEFT and the cycle can survive a left turn,
    // turn the cycle left and update the last tail turn time
    if (canSurviveRightTurn && turnDirection == LEFT)
    {
        owner_->ActTurnBot(LEFT);
        lastTailTurnTime = helper_->CurrentTime();
    }

    // If the turn direction is RIGHT and the cycle can survive a right turn,
    // turn the cycle right and update the last tail turn time
    if (canSurviveLeftTurn && turnDirection == RIGHT)
    {
        owner_->ActTurnBot(RIGHT);
        lastTailTurnTime = helper_->CurrentTime();
    }
}

void gSmartTurning::smartTurningOpposite(gHelperData &data)
{
    // Check if the cycle is alive
    if (!helper_->aliveCheck())
    {
        // If the cycle is not alive, return
        return;
    }

    // Variables to store the result of the canSurviveTurn function
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    bool closedIn, blockedBySelf;

    // Determine if the cycle can survive a turn to the left or right according to rubber and space
    canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningSpace);

    // Check if the cycle is closed in or blocked by itself
    if ((closedIn) || (closedIn) && blockedBySelf)
    {
        // Skip the survive logic turn if the cycle is closed in or blocked by itself
        goto SKIP_FORCETURN;
    }

    // Check if the cycle can survive a right turn but not a left turn
    if (!canSurviveLeftTurn && canSurviveRightTurn)
    {
        // Set the force turn to RIGHT
        owner_->forceTurn = RIGHT;
        // Set the block turn to NONE
        owner_->blockTurn = NONE;
        // Return from the function
        return;
    }
    // Check if the cycle can survive a left turn but not a right turn
    else if (canSurviveLeftTurn && !canSurviveRightTurn)
    {
        // Set the force turn to LEFT
        owner_->forceTurn = LEFT;
        // Set the block turn to NONE
        owner_->blockTurn = NONE;
        // Return from the function
        return;
    }

    SKIP_FORCETURN:
    {
        // Set the force turn to NONE
        this->forceTurn = NONE;
        // Return from the function
        return;
    }
}

/**
 * @brief gSmartTurning::smartTurningSurvive function checks if the player can survive the turn.
 * If the player cannot survive either direction, block both turns and if enabled, turn as soon as possible using trace.
 *
 * @param data The reference to the gHelperData struct that holds all the helper data.
 */
void gSmartTurning::smartTurningSurvive(gHelperData &data)
{
    // Check if the player is alive. If not, return.
    // Check if the player has any pending turns. If yes, return.
    if (!helper_->aliveCheck() || !owner_->pendingTurns.empty())
    {
        return;
    }

    // Variables to store the result of whether the player can survive a left or right turn.
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    // Variables to store if the player is closed in or blocked by themselves.
    bool closedIn, blockedBySelf;

    // Check if the player can survive a turn.
    canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningSpace);

    // If the player has a turn ignore time set and it is not over, block both turns and return.
    if (owner_->turnIgnoreTime > owner_->localCurrentTime)
    {
        this->blockTurn = BOTH;
        return;
    }
    // If the player is closed in or blocked by themselves and closedIn is set, unblock both turns and return.
    if ((closedIn && sg_helperSmartTurningClosedIn) || (closedIn) && blockedBySelf)
    {
        goto SKIP_BLOCKTURN;
    }

    // If the player cannot survive either of the turn, block both turns.
    if (!canSurviveLeftTurn && !canSurviveRightTurn)
    {
        if (this->blockTurn != BOTH)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING BOTH TURNS", "");

        this->blockTurn = BOTH;
        return;
    }

    // If the player cannot survive the left turn, block the left turn.
    if (!canSurviveLeftTurn)
    {
        if (this->blockTurn != LEFT)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING LEFT TURNS", "");

        this->blockTurn = LEFT;
        return;
    }

    // If the player cannot survive the right turn, block the right turn.
    if (!canSurviveRightTurn)
    {
        if (this->blockTurn != RIGHT)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING RIGHT TURN", "");

        this->blockTurn = RIGHT;
            return;
    }

    SKIP_BLOCKTURN:
        // Unblock the turns if the player can survive a turn.
        owner_->lastBlockedTurn = this->blockTurn;

        if (this->blockTurn != NONE)
            HelperDebug::Debug("SMART TURNING SURVIVE", "UNBLOCKING TURNS", "");

        // Set the blockTurn to NONE to indicate that the player can now turn in either direction.
        this->blockTurn = NONE;

        // Check if the smartTurningSurviveTrace is enabled and call the function if it is.
        if (sg_helperSmartTurningSurviveTrace)
            smartTurningSurviveTrace(data);
}

/**
 * @brief smartTurningSurviveTrace - performs the logic for the survive trace functionality of the smart turning module
 *
 * The function checks if the bot is alive and driving straight. If not, the function returns immediately.
 * The function then retrieves the last blocked direction the bot tried to turn in. If the bot did not try to turn
 * in the last time step, the function returns immediately.
 * The function then retrieves the corner data of the blocked direction. If the corner is close, it has been noticed
 * recently, the turn time is positive and the turn time factor is less than or equal to the survive trace turn time,
 * the function calls the makeTurnIfPossible function to attempt to turn in
 * the blocked direction as soon as possible.
 @param data - the helper data struct containing information used by the helper logic

 @return - the function returns void and has no return value
*/
void gSmartTurning::smartTurningSurviveTrace(gHelperData &data)
{
    // Return if the bot is not alive or not driving straight
    // Return if the last turn attempt was more recent than the last successful turn
    if (!helper_->aliveCheck() ||
        !helper_->drivingStraight() ||
        (owner_->lastTurnAttemptTime < owner_->lastTurnTime))
    {
        return;
    }

    // Get the direction of the last blocked turn attempt
    int lastBlockedDir = owner_->lastTurnAttemptDir;

    // Get the information about the blocked corner
    gSmartTurningCornerData* corner = helper_->getCorner(lastBlockedDir);

    // Calculate the time factor for the turn
    REAL turnTimeFactor = corner->getTimeUntilTurn(owner_->Speed());

    // If the following conditions are met, try to make a turn:
    // 1. The corner exists
    // 2. The bot is close enough to the corner
    // 3. The time since the last turn attempt is greater than a certain threshold
    // 4. The time until the turn is greater than 0
    // 5. The time until the turn is less than a certain threshold
    if (corner->exist && isClose(corner->currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
        owner_->lastTurnAttemptTime + (sg_helperSmartTurningSurviveTraceActiveTime / (10 * data.turnSpeedFactorF())) > corner->noticedTime &&
        corner->getTimeUntilTurn(owner_->Speed()) > 0 &&
        (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime))
    {
        // Attempt to make a turn in the direction of the blocked attempt
        if (makeTurnIfPossible(data, lastBlockedDir, 1))
        {
            // Print the turn
            HelperDebug::Debug("SMART TURNING TRACE", std::string("TURNED") + ((lastBlockedDir == LEFT) ? "LEFT" : "RIGHT"), "");}
    }
}

// This function checks if a turn is possible in the specified direction (dir)
// by comparing the sensor hit value to a spaceFactor or the current rubberFactor
// If the owner is not driving straight, return false
// If the turn is possible (canSurviveTurnSpecific returns true), call the owner's ActTurnBot function with the specified direction
// Return the result of the call to ActTurnBot (i.e., if the turn was successfully made)
bool gSmartTurning::makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor)
{
    if (!helper_->drivingStraight())
        return false;

    if (canSurviveTurnSpecific(data, dir, spaceFactor))
    {
        return owner_->ActTurnBot(dir);
    }
    return false;
}

// The function returns whether the cycle can survive a turn in the specific direction
bool gSmartTurning::canSurviveTurnSpecific(gHelperData &data, int dir, REAL spaceFactor)
{
    // compareFactor is the value that the gap distance should be compared to
    REAL compareFactor;

    // If spaceFactor > 0, it's used as the compareFactor
    if (spaceFactor > 0)
    {
        // spaceFactor is multiplied by the turn speed factor
        compareFactor = spaceFactor * data.turnSpeedFactorF();
    }
    else
    {
        // If spaceFactor <= 0, rubberFactor is calculated and used as compareFactor
        helper_->rubberData->calculate();
        compareFactor = helper_->rubberData->rubberFactor;
    }

    // If the direction is LEFT, return whether the gap distance is greater than compareFactor
    if (dir == LEFT)
    {
        return data.sensors.getSensor(LEFT)->hit > compareFactor;
    }
    // If the direction is RIGHT, return whether the gap distance is greater than compareFactor
    else if (dir == RIGHT)
    {
        return data.sensors.getSensor(RIGHT)->hit > compareFactor;
    }
}


// Function that checks if a turn can be survived by the player.
void gSmartTurning::canSurviveTurn(gHelperData &data, REAL &canSurviveLeftTurn, REAL &canSurviveRightTurn, bool &closedIn, bool &blockedBySelf, REAL freeSpaceFactor)
{
    // If the player is not alive, return.
    if (!helper_->aliveCheck())
    {
        return;
    }

    // Calculate the rubber factor.
    helper_->rubberData->calculate();

    // Initialize the values to assume both turns are survivable.
    canSurviveLeftTurn = true;
    canSurviveRightTurn = true;

    // Get the front, left and right sensors.
    gSensor *front = data.sensors.getSensor(FRONT);
    gSensor *left = data.sensors.getSensor(LEFT);
    gSensor *right = data.sensors.getSensor(RIGHT);

    // Initialize the variables to assume the turns can be survived.
    bool canTurnLeftRubber = true, canTurnRightRubber = true, canTurnLeftSpace = true, canTurnRightSpace = true;
    // Calculate the closed-in factor based on the turn speed factor and the closed-in mult.
    REAL closedInFactor = data.turnSpeedFactorF() * sg_helperSmartTurningClosedInMult;
    // Check if the player is closed in on the front, left and right sides.
    closedIn = (front->hit <= closedInFactor && left->hit <= closedInFactor && right->hit <= closedInFactor);
    // Check if the player is blocked by itself on the front, left and right sides.
    blockedBySelf = (left->type == gSENSOR_SELF && right->type == gSENSOR_SELF && front->type == gSENSOR_SELF);

    // If free space factor is greater than 0, check the free space factor.
    if (freeSpaceFactor > 0)
    {
        // If the left hit is less than the turn speed factor multiplied by the free space factor and the front and right hits are greater than the turn speed factor multiplied by the free space factor, the turn left cannot be survived.
        if (left->hit < data.turnSpeedFactorF() * freeSpaceFactor && front->hit > data.turnSpeedFactorF() * freeSpaceFactor && right->hit > data.turnSpeedFactorF() * freeSpaceFactor)
        {
            canTurnLeftSpace = false;
        }
        // If the right hit is less than the turn speed factor multiplied by the free space factor and the front and left hits are greater than the turn speed factor multiplied by the free space factor, the turn right cannot be survived.
        if (right->hit < data.turnSpeedFactorF() * freeSpaceFactor && front->hit > data.turnSpeedFactorF() * freeSpaceFactor && left->hit > data.turnSpeedFactorF() * freeSpaceFactor)
        {
            canTurnRightSpace = false;
        }
    }

    // If the left hit is less than the rubber factor, the turn left cannot be survived.
    if (left->hit < helper_->rubberData->rubberFactor)
    {
        canTurnLeftRubber = false;
    }

    // If the right hit is less than the rubber factor, the turn right cannot be survived.
    if (right->hit < helper_->rubberData->rubberFactor)
    {
        canTurnRightRubber = false;
    }

    // If freeSpaceFactor is greater than 0, calculate canSurviveLeftTurn and canSurviveRightTurn based on closedIn and freeSpaceFactor
    if (freeSpaceFactor > 0)
    {
        // If the player is not closed in and both canTurnLeftRubber and canTurnLeftSpace are true,
        // then the left turn can be survived.
        canSurviveLeftTurn = !closedIn ? canTurnLeftRubber && canTurnLeftSpace : canTurnLeftRubber;
        // If the player is not closed in and both canTurnRightRubber and canTurnRightSpace are true,
        // then the right turn can be survived.
        canSurviveRightTurn = !closedIn ? canTurnRightRubber && canTurnRightSpace : canTurnRightRubber;
    }
    else
    {
        // If freeSpaceFactor is not greater than 0, calculate canSurviveLeftTurn and canSurviveRightTurn
        // based on canTurnLeftRubber and canTurnRightRubber.
        canSurviveLeftTurn = canTurnLeftRubber;
        canSurviveRightTurn = canTurnRightRubber;
    }
}

/**
 * @brief gSmartTurning::smartTurningFrontBot handles the front bot actions by getting an emergecy turning when rubber usage
 * or space factor reaches a current threashold.
 * This function checks if the bot has any pending turns and if the smartTurningFrontBotTurnOnce flag is set and the
 * last bot turn was made more recently than the last turn, the function returns without any action.
 * If the rubber used ratio is greater than or equal to sg_helperSmartTurningFrontBotActivationRubber or the range to the
 * front bot is less than or equal to  sg_helperSmartTurningFrontBotActivationSpace, the function attempts to make a turn in
 * the direction determined by the emergency turn. If a turn is made, new turns by the owner will be ignored for a specified
 * time determined by setting the turnIgnoreTime to the current time plus sg_helperSmartTurningFrontBotDisableTime.
 *
 * @param data gHelperData object containing sensor data and other relevant information.
 */
void gSmartTurning::smartTurningFrontBot(gHelperData &data)
{
    // Return if there are any pending turns
    // or the smartTurningFrontBotTurnOnce flag is set and
    // the last bot turn was made more recently than the last turn

    if (!owner_->pendingTurns.empty() ||
        (sg_helperSmartTurningFrontBotTurnOnce && owner_->lastBotTurnTime > owner_->lastTurnTime))
    {
        return;
    }

    // Get the range to the front bot
    REAL hitRange = data.sensors.getSensor(FRONT)->hit;

    // If the range to the front bot is within the sg_helperSmartTurningFrontBotThinkRange
    if (hitRange <= sg_helperSmartTurningFrontBotThinkRange * data.turnSpeedFactorF())
    {
        // Calculate the rubber data
        helper_->rubberData->calculate();

        // If the rubber used ratio is greater than or equal to sg_helperSmartTurningFrontBotActivationRubber
        // or the range to the front bot is less than or equal to sg_helperSmartTurningFrontBotActivationSpace
        if (helper_->rubberData->rubberUsedRatio >= sg_helperSmartTurningFrontBotActivationRubber ||
            hitRange <= sg_helperSmartTurningFrontBotActivationSpace)
        {
            // Flag to check if a turn was made
            bool turnMade = false;

            // Get the turn data from the emergency turn object
            gTurnData* turnData = emergencyTurn->getTurn();

            // If the turn data exists and the number of turns is greater than 0
            if (turnData->exist && turnData->numberOfTurns > 0)
            {
                // Loop through the number of turns
                for (int i = 0; i < turnData->numberOfTurns; i++)
                {
                    // Try to make the turn in the specified direction (left or right)
                    if (makeTurnIfPossible(data, turnData->direction, 0))
                    {
                        // Log the direction of the turn made
                        HelperDebug::Debug("smartTurningFrontBot",
                                           "Turn made: " + std::string(turnData->direction == -1 ? "LEFT" : "RIGHT") + " Reason: " + (turnData->reason), "");
                        turnMade = true;
                    }
                }
            } else if (turnData->noTurns > 100) {
                HelperDebug::Debug("smartTurningFrontBot",
                                   "Failed to find turn in " + std::to_string(turnData->noTurns) + " tries!", "");
            }
            // If smartTurningFrontBotDisableTime is greater than 0 and a turn has been made
            if (sg_helperSmartTurningFrontBotDisableTime > 0 && turnMade)
            {
                // Set the turnIgnoreTime for the owner
                owner_->turnIgnoreTime = helper_->CurrentTime() + (sg_helperSmartTurningFrontBotDisableTime);
            }
        }
    }
}

gSmartTurning &gSmartTurning::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(helper);

    // create
    if (helper->smartTurning.get() == 0)
        helper->smartTurning.reset(new gSmartTurning(helper, owner));

    return *helper->smartTurning;
}
