#include "../../gSensor.h"
#include "gHelperSensor.h"
#include "../../gCycle.h"
#include "../../gWall.h"
#include "../../gAIBase.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"

using namespace helperConfig;

extern REAL sg_cycleBrakeRefill;
extern REAL sg_cycleBrakeDeplete;
extern void sg_RubberValues(ePlayerNetID const *player, REAL speed, REAL &max, REAL &effectiveness);
extern REAL sg_brakeCycle;

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
        // cowardy campbot)
        lrSuggestion_ *= -1;
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


// The function returns whether the cycle can survive a turn in the specific direction
bool gTurnHelper::canSurviveTurnSpecific(gHelperData &data, int dir, REAL spaceFactor)
{
    // compareFactor is the value that the gap distance should be compared to
    REAL compareFactor;
    REAL *hit = &data.sensors.getSensor(dir)->hit;
    // If spaceFactor > 0, it's used as the compareFactor
    if (spaceFactor > 0)
    {
        // spaceFactor is multiplied by the turn speed factor
        compareFactor = spaceFactor * data.ownerData.turnSpeedFactorF();
    }
    else
    {
        // If spaceFactor <= 0, rubberFactor is calculated and used as compareFactor
        data.rubberData.calculate();
        compareFactor = data.rubberData.rubberFactor;
    }

    // return whether the hit distance is greater than compareFactor
    return *hit > compareFactor;
}

bool gTurnHelper::makeTurnIfPossible(int dir, REAL spaceFactor)
{
    return makeTurnIfPossible(*helper_->data_stored,dir,spaceFactor);
}

// This function checks if a turn is possible in the specified direction (dir)
// by comparing the sensor hit value to a spaceFactor or the current rubberFactor
// If the owner is not driving straight, return false
// If the turn is possible (canSurviveTurnSpecific returns true), call the owner's ActTurnBot function with the specified direction
// Return the result of the call to ActTurnBot (i.e., if the turn was successfully made)
bool gTurnHelper::makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor)
{
    if (!helper_->drivingStraight())
        return false;

    if (canSurviveTurnSpecific(data, dir, spaceFactor))
        return owner_->ActTurnBot(dir);

    return false;
}


// Function that checks if a turn can be survived by the player.
gSurviveData gTurnHelper::canSurviveTurn(gHelperData &data, REAL freeSpaceFactor)
{
    // If the player is not alive, return.
    if (!helper_->aliveCheck())
        return gSurviveData(false);

    gSurviveData surviveData;

    // Calculate the rubber factor.
    data.rubberData.calculate();

    // Initialize the values to assume both turns are survivable.
    surviveData.canSurviveLeftTurn = true;
    surviveData.canSurviveRightTurn = true;

    // Get the front, left and right sensors.
    gSensor *front = data.sensors.getSensor(FRONT);
    gSensor *left = data.sensors.getSensor(LEFT);
    gSensor *right = data.sensors.getSensor(RIGHT);

    // Initialize the variables to assume the turns can be survived.
    bool canTurnLeftRubber = true, canTurnRightRubber = true, canTurnLeftSpace = true, canTurnRightSpace = true;
    // Calculate the closed-in factor based on the turn speed factor and the closed-in mult.
    REAL closedInFactor = data.ownerData.turnSpeedFactorF() * sg_helperSmartTurningClosedInMult;
    // Check if the player is closed in on the front, left and right sides.
    surviveData.closedIn = (front->hit <= closedInFactor && left->hit <= closedInFactor && right->hit <= closedInFactor);
    // Check if the player is blocked by itself on the front, left and right sides.
    surviveData.blockedBySelf = (left->type == gSENSOR_SELF && right->type == gSENSOR_SELF && front->type == gSENSOR_SELF);

    surviveData.trapped =  IsTrapped(owner_, NULL);

    // If free space factor is greater than 0, check the free space factor.
    if (freeSpaceFactor > 0)
    {
        // If the left hit is less than the turn speed factor multiplied by the free space factor and the front and right hits are greater than the turn speed factor multiplied by the free space factor, the turn left cannot be survived.
        if (left->hit < data.ownerData.turnSpeedFactorF() * freeSpaceFactor && front->hit > data.ownerData.turnSpeedFactorF() * freeSpaceFactor && right->hit > data.ownerData.turnSpeedFactorF() * freeSpaceFactor)
        {
            canTurnLeftSpace = false;
        }
        // If the right hit is less than the turn speed factor multiplied by the free space factor and the front and left hits are greater than the turn speed factor multiplied by the free space factor, the turn right cannot be survived.
        if (right->hit < data.ownerData.turnSpeedFactorF() * freeSpaceFactor && front->hit > data.ownerData.turnSpeedFactorF() * freeSpaceFactor && left->hit > data.ownerData.turnSpeedFactorF() * freeSpaceFactor)
        {
            canTurnRightSpace = false;
        }
    }

    // If the left hit is less than the rubber factor, the turn left cannot be survived.
    if (left->hit < data.rubberData.rubberFactor)
    {
        canTurnLeftRubber = false;
    }

    // If the right hit is less than the rubber factor, the turn right cannot be survived.
    if (right->hit < data.rubberData.rubberFactor)
    {
        canTurnRightRubber = false;
    }

    // If freeSpaceFactor is greater than 0, calculate canSurviveLeftTurn and canSurviveRightTurn based on closedIn and freeSpaceFactor
    if (freeSpaceFactor > 0)
    {
        // If the player is not closed in and both canTurnLeftRubber and canTurnLeftSpace are true,
        // then the left turn can be survived.
        surviveData.canSurviveLeftTurn = !surviveData.closedIn ? canTurnLeftRubber && canTurnLeftSpace : canTurnLeftRubber;
        // If the player is not closed in and both canTurnRightRubber and canTurnRightSpace are true,
        // then the right turn can be survived.
        surviveData.canSurviveRightTurn = !surviveData.closedIn ? canTurnRightRubber && canTurnRightSpace : canTurnRightRubber;
    }
    else
    {
        // If freeSpaceFactor is not greater than 0, calculate canSurviveLeftTurn and canSurviveRightTurn
        // based on canTurnLeftRubber and canTurnRightRubber.
        surviveData.canSurviveLeftTurn = canTurnLeftRubber;
        surviveData.canSurviveRightTurn = canTurnRightRubber;
    }
    
    return surviveData;
}


int gTurnHelper::ActToTurn(uActionPlayer *action)
{
    return action == &gCycle::se_turnLeft ? -1 : 1;
}

template <class T>
void gTurnHelper::BotDebug(std::string description, T value)
{
    gHelperUtility::Debug("gTurnHelper", description, &value);
}

void gTurnHelper::BotDebug(std::string description)
{
    gHelperUtility::Debug("gTurnHelper", description, "");
}


// determines the distance between two sensors; the size should give the
// likelyhood to survive if you pass through a gap between the two selected
// walls
REAL gTurnHelper::Distance(Sensor const &a, Sensor const &b)
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
gTurnData* gTurnHelper::getTurn()
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
//    helper_->rubberData->calculate();
    REAL rubberTime = helper_->data_stored->rubberData.rubberTimeLeft;
    REAL rubberRatio = helper_->data_stored->rubberData.rubberUsedRatio;

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


gTurnHelper &gTurnHelper::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(helper);

    // create
    if (helper->turnHelper.get() == 0)
        helper->turnHelper.reset(new gTurnHelper(helper, owner));

    return *helper->turnHelper;
}
