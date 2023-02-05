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

int noTurns = 0;
class gHelperEmergencyTurn
{
    gHelperEmergencyTurn();

public:
    class Sensor : public gSensor
    {
    public:
        Sensor(gCycle *o, const eCoord &start, const eCoord &d)
            : gSensor(o, start, d), hitOwner_(0), hitTime_(0), hitDistance_(o->MaxWallsLength()), lrSuggestion_(0), windingNumber_(0)
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
                // unless it is an enemy, follow his wall instead (uncomment for a nasty cowardy campbot)
                // lrSuggestion *= -1;
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

                // gCycleChatBot & enemyChatBot = Get( hitOwner_ );

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
                if (hitTime_ > hitOwner_->LastTime() && hitOwner_ != owned)
                {
                    ehit = NULL;
                    hit = 1E+40;
                    return false;
                }

                // REAL cycleDistance = hitOwner_->GetDistance();

                // REAL wallStart = 0;

                /*
                if ( gCycle::WallsLength() > 0 )
                {
                    wallStart = cyclePos - playerWall->Cycle()->ThisWallsLength();
                    if ( wallStart < 0 )
                        wallStart = 0;
                }
                */
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

        gCycle *hitOwner_;   // the owner of the hit wall
        REAL hitTime_;       // the time the hit wall was built at
        REAL hitDistance_;   // the distance of the wall to the cycle that built it
        short lrSuggestion_; // sensor's oppinon on whether moving to the left or right of the hit wall is recommended (-1 for left, +1 for right)
        int windingNumber_;  // the number of turns (with sign) the cycle has taken
    };

    gHelperEmergencyTurn(gCycle *owner, gHelper *helper) : owner_(owner),
                                                           helper_(helper)
    {
    }

    // determines the distance between two sensors; the size should give the likelyhood
    // to survive if you pass through a gap between the two selected walls
    REAL Distance(Sensor const &a, Sensor const &b)
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
            REAL ret =
                a.hitDistance_ + b.hitDistance_;

            if (rim)
            {
                ret = bigDistance * .001 + ret * .01 + (a.before_hit - b.before_hit).Norm();

                // we love going between the rim and enemies
                if (!self)
                    ret = bigDistance * 2;
            }

            // minimal factor should be 1, this path should never return something smaller than the
            // paths where only one cycle's walls are hit
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
            return (fabsf(a.hitDistance_ - b.hitDistance_) + .25 * bigDistance) * selfHatred;
        }

        else if ((-2 * a.lr * (a.windingNumber_ - b.windingNumber_) > owner_->Grid()->WindingNumber()))
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

    int ActToTurn(uActionPlayer *action)
    {
        return action == &gCycle::se_turnLeft ? -1 : 1;
    }

    template <class T>
    void BotDebug(std::string description, T value = "")
    {
        HelperDebug::Debug("gHelperEmergencyTurn", description, &value);
    }

    void BotDebug(std::string description)
    {
        HelperDebug::Debug("gHelperEmergencyTurn", description, "");
    }

    // does the main thinking
    gTurnData *getTurn()
    {
        gTurnData *turn = (new gTurnData(false));

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
            return turn;
        }

        // owner_->enemyInfluence.AddSensor( front, 0, owner_ );

        REAL minMoveOn = 0, maxMoveOn = 0, moveOn = 0;

        // get extra time we get through rubber usage
        /*
        REAL rubberGranted, rubberEffectiveness;
        sg_RubberValues(owner_->Player(), speed, rubberGranted, rubberEffectiveness);
        REAL rubberTime = (rubberGranted - owner_->GetRubber()) * rubberEffectiveness / speed;
        REAL rubberRatio = owner_->GetRubber() / rubberGranted;
        */
        helper_->rubberData->calculate();
        REAL rubberTime = helper_->rubberData->rubberTimeLeft;
        REAL rubberRatio = helper_->rubberData->rubberUsedRatio;
        // turnedRecently_ = false;

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
                turn = new gTurnData(RIGHT, 2, "right by override rim hugging 1");
                return turn;

                // owner_->Act( &gCycle::se_turnRight, 1 );
                // owner_->Act( &gCycle::se_turnRight, 1 );
            }
            else if (leftOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
            {
                turn = new gTurnData(LEFT, 2, "left 2 by override rim hugging 1");
                return turn;
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
                turn = new gTurnData(LEFT, 2, "left 2 by override rim hugging 2");
                return turn;
                // owner_->Act( &gCycle::se_turnLeft, 1 );
                // owner_->Act( &gCycle::se_turnLeft, 1 );
            }
            else if (rightOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
            {
                turn = new gTurnData(RIGHT, 2, "right 2 by override rim hugging 2");
                return turn;
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

                turn = new gTurnData(ActToTurn(bestAction), 1, "frontOpen < bestOpen bestAction ");
                ;
                return turn;
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

                            turn = new gTurnData(ActToTurn(otherAction), 1, "rearOpen > bestSoFar ootherAction");
                            ;
                            return turn;
                        }
                        // // there needs to be space ahead to finish the maneuver correctly
                        // if ( maxMoveOn < speed * owner_->GetTurnDelay() )
                        // {
                        //     // there isn't. oh well, turn into the wrong direction completely, see if I care
                        //     owner_->Act( otherAction, 1 );
                        //     wait = true;
                        // }
                    }
                }

                if (!wait)
                {
                    turn = new gTurnData(ActToTurn(bestAction), 1, "not waiting bestAction");
                    ;
                    return turn;
                }

                minMoveOn = maxMoveOn = moveOn = 0;
            }
        }

        // execute brake command
        owner_->Act(&gCycle::s_brake, brake ? 1 : -1);

        noTurns++;
        // BotDebug("NO TURN FOUND", noTurns);
        return turn;
    }

private:
    gCycle *owner_; //!< owner of chatbot
    gHelper *helper_;
};

class SensorPub : public gSensor
{
public:
    SensorPub(gCycle *o, const eCoord &start, const eCoord &d)
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
    // determines the distance between two sensors; the size should give the
    // likelyhood to survive if you pass through a gap between the two selected
    // walls
    REAL Distance(SensorPub const &a, SensorPub const &b)
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

    if (sg_helperSmartTurningAutoTrace)
        smartTurningAutoTrace(data);

    if (sg_helperSmartTurningFollowTail)
        followTail(data);

    if (sg_helperSmartTurningFrontBot)
        smartTurningFrontBot(data);
}

bool gSmartTurning::isClose(eCoord pos, REAL closeFactor)
{
    eCoord distanceToPos = owner_->pos - pos;
    return distanceToPos.NormSquared() < closeFactor * closeFactor;
}

void gSmartTurning::smartTurningPlan(gHelperData &data)
{
    if (!helper_->aliveCheck())
    {
        return;
    }
    if (!helper_->leftCorner.exist && !helper_->rightCorner.exist)
    {
        helper_->findCorners(data);
        return;
    }
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

    bool close = isClose(cornerToUse.currentPos, sg_helperShowCornersBoundary);
    if (!close)
    {
        return;
    }

    if (cornerToUse.infront)
    {
        makeTurnIfPossible(data, dir, 5);
    }
}

void gSmartTurning::thinkSurvive(gHelperData &data)
{
}

void gSmartTurning::followTail(gHelperData &data)
{

    if (!helper_->aliveCheck() || owner_->tailMoving != true)
    {
        return;
    }

    if (!isClose(owner_->tailPos, data.turnSpeedFactorF() * 3))
    {
        return;
    }

    if (owner_->tailMoving != true)
    {
        return;
    }
    REAL delay = data.turnSpeedFactorF() * sg_helperSmartTurningFollowTailDelayMult;
    bool drivingStraight = helper_->drivingStraight();
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    bool closedIn, blockedBySelf;

    canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningFollowTailFreeSpaceMult);
    if (closedIn || blockedBySelf)
    {
        return;
    }

    // if (owner_->tailMoving) {
    // // Calculate the direction from the cycle to the tail
    // eCoord directionToTail = owner_->tailPos - owner_->pos;
    // directionToTail.Normalize();

    // // Determine if the tail is in front of or behind the cycle
    // if (directionToTail * owner_->dir < 0) {
    //     // The tail is behind the cycle, turn towards it

    //     // Check if there are walls to the left or right of the cycle
    //     gSensor leftSensor(owner_, owner_->pos, owner_->dir.Turn(0, 1));
    //     leftSensor.detect(sg_helperSensorRange);
    //     gSensor rightSensor(owner_, owner_->pos, owner_->dir.Turn(0, -1));
    //     rightSensor.detect(sg_helperSensorRange);

    //     if (directionToTail * owner_->dir > 0 && canSurviveRightTurn && rightSensor.hit > .999999) {
    //         owner_->Act(&gCycle::se_turnRight, 1);
    //     }
    //     else if (canSurviveLeftTurn && leftSensor.hit > .999999) {
    //         owner_->Act(&gCycle::se_turnLeft, 1);
    //     }
    // }
    // else {
    //     // The tail is in front of the cycle, stay on current path

    //     // The tail is directly in front of or behind the cycle
    //     // Stay on current path
    // }
    // }
    // return;
    // bool turnedRecently = !(lastTailTurnTime < 0) && owner_->lastTurnTime + delay > CurrentTime();
    // con << owner_->lastTurnTime + delay << " > " << CurrentTime() << "\n" << turnedRecently << "\n";
    bool readyToTurn = helper_->CurrentTime() > lastTailTurnTime + delay;

    if (!readyToTurn)
    {
        return;
    }

    int turnDirection = thinkPath(owner_->tailPos, data);

    if (canSurviveRightTurn && turnDirection == -1)
    {
        owner_->Act(&gCycle::se_turnRight, 1);
        lastTailTurnTime = helper_->CurrentTime();
    }

    if (canSurviveLeftTurn && turnDirection == 1)
    {
        owner_->Act(&gCycle::se_turnLeft, 1);
        lastTailTurnTime = helper_->CurrentTime();
    }
}

int gSmartTurning::thinkPath(eCoord pos, gHelperData &data)
{

    eFace *posFace = owner_->grid->FindSurroundingFace(pos);
    eHalfEdge::FindPath(owner_->Position(), owner_->CurrentFace(),
                        pos, posFace,
                        owner_,
                        path);
    REAL mindist = 10;
    int lr = 0;
    eCoord dir = owner_->Direction();
    REAL ls = data.sensors.getSensor(LEFT)->hit;
    REAL rs = data.sensors.getSensor(RIGHT)->hit;

    for (int z = 5; z >= 0; z--)
    {
        if (!path.Proceed())
        {
            break;
        }
    }

    bool goon = path.Proceed();
    bool nogood = false;

    do
    {
        if (goon)
            goon = path.GoBack();
        else
            goon = true;

        eCoord pos = path.CurrentPosition() + path.CurrentOffset() * 0.1f;
        eCoord opos = owner_->Position();
        eCoord odir = pos - opos;

        eCoord intermediate = opos + dir * eCoord::F(odir, dir);

        gSensor p(owner_, opos, intermediate - opos);
        p.detect(1.1f);
        nogood = (p.hit <= .999999999 || eCoord::F(path.CurrentOffset(), odir) < 0);

        if (!nogood)
        {
            gSensor p(owner_, intermediate, pos - intermediate);
            p.detect(1);
            nogood = (p.hit <= .99999999 || eCoord::F(path.CurrentOffset(), odir) < 0);
        }
        // debugLine(0,1,1,5,4*data.speedFactorF(),pos,pos);

    } while (goon && nogood);

    if (goon)
    {
        // now we have found our next goal. Try to get there.
        eCoord pos = owner_->Position();
        eCoord target = path.CurrentPosition();

        gHelper::debugLine(gRealColor(.2, 1, 0), 5, data.speedFactorF(), target, target);
        gHelper::debugLine(gRealColor(.2, 1, 0), 5, data.speedFactorF(), pos, target);
        // look how far ahead the target is:
        REAL ahead = eCoord::F(target - pos, dir) + eCoord::F(path.CurrentOffset(), dir);

        if (ahead > 0)
        { // it is still before us. just wait a while.
            mindist = ahead;
        }
        else
        { // we have passed it. Make a turn towards it.
            REAL side = (target - pos) * dir;

            if (!((side > 0 && ls < 3) || (side < 0 && rs < 3)) && (fabs(side) > 3 || ahead < -10))
            {
                lr += (side > 0 ? 1 : -1);
                return lr;
            }
        }
    }
}

void gSmartTurning::smartTurningOpposite(gHelperData &data)
{
    if (!helper_->aliveCheck())
    {
        return;
    }
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    bool closedIn, blockedBySelf;

    canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningSpace);

    if ((closedIn) || (closedIn) && blockedBySelf)
    {
        goto SKIP_FORCETURN;
    }

    if (!canSurviveLeftTurn && canSurviveRightTurn)
    {
        owner_->forceTurn = 1;
        owner_->blockTurn = 0;
        return;
    }
    else if (canSurviveLeftTurn && !canSurviveRightTurn)
    {
        owner_->forceTurn = -1;
        owner_->blockTurn = 0;
        return;
    }

SKIP_FORCETURN:
{
    this->forceTurn = 0;
    return;
}
}

void gSmartTurning::smartTurningSurvive(gHelperData &data)
{
    if (!helper_->aliveCheck())
    {
        return;
    }
    if (!owner_->pendingTurns.empty())
    {
        return;
    }
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    bool closedIn, blockedBySelf;

    canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningSpace);
    // con << canSurviveLeftTurn << " | " << canSurviveRightTurn << " | " << closedIn << " | " << blockedBySelf << " | " << sg_helperSmartTurningSpace << "\n";

    if (owner_->turnIgnoreTime > owner_->localCurrentTime)
    {
        this->blockTurn = BOTH;
        return;
    }
    if ((closedIn && sg_helperSmartTurningClosedIn) || (closedIn) && blockedBySelf)
    {
        goto SKIP_BLOCKTURN;
    }

    if (!canSurviveLeftTurn && !canSurviveRightTurn)
    {
        if (this->blockTurn != BOTH)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING BOTH TURNS", "");
        this->blockTurn = BOTH;
        return;
    }

    if (!canSurviveLeftTurn)
    {

        if (this->blockTurn != LEFT)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING LEFT TURNS", "");
        this->blockTurn = LEFT;
        return;
    }

    if (!canSurviveRightTurn)
    {
        if (this->blockTurn != RIGHT)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING RIGHT TURN", "");
        this->blockTurn = RIGHT;
        return;
    }

SKIP_BLOCKTURN:
{
    // this->turnIgnoreTime = -999;
    owner_->lastBlockedTurn = this->blockTurn;
    if (this->blockTurn != 0)
        HelperDebug::Debug("SMART TURNING SURVIVE", "UNBLOCKING TURNS", "");
    this->blockTurn = 0;
    if (sg_helperSmartTurningSurviveTrace)
    {
        smartTurningSurviveTrace(data);
    }
    return;
}
}

void gSmartTurning::smartTurningAutoTrace(gHelperData &data)
{
    if (!helper_->aliveCheck())
    {
        return;
    }
    if (!helper_->drivingStraight())
    {
        return;
    }
    int lastTurn = owner_->lastTurnAttemptDir;

    if (owner_->lastTurnAttemptTime < owner_->lastTurnTime)
    {
        return;
    }

    if (!(helper_->rightCorner.exist || helper_->rightCorner.exist))
        return;

    switch (lastTurn)
    {
    case LEFT:
    {
        REAL turnTimeFactor = helper_->leftCorner.getTimeUntilTurn(owner_->Speed());

        if (helper_->leftCorner.exist && isClose(helper_->leftCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
            helper_->leftCorner.getTimeUntilTurn(owner_->Speed()) > 0 &&
            helper_->leftCorner.distanceFromPlayer < sg_helperSmartTurningAutoTraceDistance &&
            (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime))
        {
            if (makeTurnIfPossible(data, LEFT, 1))
                HelperDebug::Debug("SMART TURNING AUTO TRACE", "TURNED LEFT", "");
        }
        return;
    }
    case RIGHT:
    {
        REAL turnTimeFactor = helper_->rightCorner.getTimeUntilTurn(owner_->Speed());

        if (helper_->rightCorner.exist && isClose(helper_->rightCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
            helper_->rightCorner.getTimeUntilTurn(owner_->Speed()) > 0 &&
            helper_->rightCorner.distanceFromPlayer < sg_helperSmartTurningAutoTraceDistance &&
            (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime))
        {
            if (makeTurnIfPossible(data, RIGHT, 1))
                ;
            HelperDebug::Debug("SMART TURNING TRACE", "TURNED RIGHT", "");
        }
        return;
    }
    }
}

void gSmartTurning::smartTurningSurviveTrace(gHelperData &data)
{
    if (!helper_->aliveCheck())
    {
        return;
    }
    if (!helper_->drivingStraight())
    {
        return;
    }
    int lastTurn = owner_->lastTurnAttemptDir;

    if (owner_->lastTurnAttemptTime < owner_->lastTurnTime)
    {
        return;
    }

    switch (lastTurn)
    {
    case LEFT:
    {
        REAL turnTimeFactor = helper_->leftCorner.getTimeUntilTurn(owner_->Speed());

        if (helper_->leftCorner.exist && isClose(helper_->leftCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
            owner_->lastTurnAttemptTime + (sg_helperSmartTurningSurviveTraceActiveTime / (10 * data.turnSpeedFactorF())) > helper_->leftCorner.noticedTime &&
            helper_->leftCorner.getTimeUntilTurn(owner_->Speed()) > 0 &&
            (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime))
        {
            if (makeTurnIfPossible(data, LEFT, 1))
                HelperDebug::Debug("SMART TURNING TRACE", "TURNED LEFT", "");
        }
        return;
    }
    case RIGHT:
    {
        REAL turnTimeFactor = helper_->rightCorner.getTimeUntilTurn(owner_->Speed());

        if (helper_->rightCorner.exist && isClose(helper_->rightCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
            owner_->lastTurnAttemptTime + (sg_helperSmartTurningSurviveTraceActiveTime / (10 * data.turnSpeedFactorF())) > helper_->rightCorner.noticedTime &&
            helper_->rightCorner.getTimeUntilTurn(owner_->Speed()) > 0 &&
            (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime))
        {
            if (makeTurnIfPossible(data, RIGHT, 1))
                HelperDebug::Debug("SMART TURNING TRACE", "TURNED LEFT", "");
        }
        return;
    }
    }
}

bool gSmartTurning::makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor)
{
    if (!helper_->drivingStraight())
        return false;

    switch (dir)
    {
    case LEFT:
    {
        if (canSurviveTurnSpecific(data, dir, spaceFactor))
        {
            return owner_->ActBot(&gCycle::se_turnLeft, 1);
        }
        return false;
    }
    case RIGHT:
    {
        if (canSurviveTurnSpecific(data, dir, spaceFactor))
        {
            return owner_->ActBot(&gCycle::se_turnRight, 1);
        }
        return false;
    }
    default:
    {
        return false;
    }
    }
}

bool gSmartTurning::canSurviveTurnSpecific(gHelperData &data, int dir, REAL spaceFactor)
{
    REAL compareFactor;

    if (spaceFactor > 0)
    {
        compareFactor = spaceFactor * data.turnSpeedFactorF();
    }
    else
    {
        // calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);
        helper_->rubberData->calculate();
        compareFactor = helper_->rubberData->rubberFactor;
    }

    if (dir == LEFT)
    {
        return data.sensors.getSensor(LEFT)->hit > compareFactor;
    }
    else if (dir == RIGHT)
    {
        return data.sensors.getSensor(RIGHT)->hit > compareFactor;
    }
}

void gSmartTurning::canSurviveTurn(gHelperData &data, REAL &canSurviveLeftTurn, REAL &canSurviveRightTurn, bool &closedIn, bool &blockedBySelf, REAL freeSpaceFactor)
{
    if (!helper_->aliveCheck())
    {
        return;
    }

    // calculateRubberFactor(sg_helperSmartTurningRubberTimeMult,sg_helperSmartTurningRubberFactorMult);
    helper_->rubberData->calculate();
    canSurviveLeftTurn = true;
    canSurviveRightTurn = true;

    gSensor *front = data.sensors.getSensor(FRONT);
    gSensor *left = data.sensors.getSensor(LEFT);
    gSensor *right = data.sensors.getSensor(RIGHT);

    bool canTurnLeftRubber = true, canTurnRightRubber = true, canTurnLeftSpace = true, canTurnRightSpace = true;
    REAL closedInFactor = data.turnSpeedFactorF() * sg_helperSmartTurningClosedInMult;
    closedIn = (front->hit <= closedInFactor && left->hit <= closedInFactor && right->hit <= closedInFactor);
    blockedBySelf = (left->type == gSENSOR_SELF && right->type == gSENSOR_SELF && front->type == gSENSOR_SELF);

    if (freeSpaceFactor > 0)
    {
        if (left->hit < data.turnSpeedFactorF() * freeSpaceFactor && front->hit > data.turnSpeedFactorF() * freeSpaceFactor && right->hit > data.turnSpeedFactorF() * freeSpaceFactor)
        {
            canTurnLeftSpace = false;
        }
        if (right->hit < data.turnSpeedFactorF() * freeSpaceFactor && front->hit > data.turnSpeedFactorF() * freeSpaceFactor && left->hit > data.turnSpeedFactorF() * freeSpaceFactor)
        {
            canTurnRightSpace = false;
        }
    }

    if (left->hit < helper_->rubberData->rubberFactor)
    {
        canTurnLeftRubber = false;
    }

    if (right->hit < helper_->rubberData->rubberFactor)
    {
        canTurnRightRubber = false;
    }

    if (freeSpaceFactor > 0)
    {
        canSurviveLeftTurn = !closedIn ? canTurnLeftRubber && canTurnLeftSpace : canTurnLeftRubber;
        canSurviveRightTurn = !closedIn ? canTurnRightRubber && canTurnRightSpace : canTurnRightRubber;
    }
    else
    {
        canSurviveLeftTurn = canTurnLeftRubber;
        canSurviveRightTurn = canTurnRightRubber;
    }
}

// void gSmartTurning::calculateRubberFactor(REAL rubberMult, REAL rubberFactorMult) {
//     if (!helper_->aliveCheck()) { return; }
//     REAL rubberGranted, rubberEffectiveness;
//     sg_RubberValues( owner_->Player(), owner_->verletSpeed_, rubberGranted, rubberEffectiveness );
//     rubberTime = ( rubberGranted - owner_->GetRubber() )*rubberEffectiveness/owner_->verletSpeed_;
//     rubberRatio = owner_->GetRubber()/rubberGranted;
//     rubberFactor = owner_->verletSpeed_ * (  owner_->GetTurnDelay() - rubberTime * sg_helperSmartTurningRubberTimeMult );
//     rubberFactor *= rubberFactorMult;
// }

void gSmartTurning::smartTurningFrontBot(gHelperData &data)
{
    if (!owner_->pendingTurns.empty())
    {
        return;
    }
    if (sg_helperSmartTurningFrontBotTurnOnce && owner_->lastBotTurnTime > owner_->lastTurnTime)
    {
        return;
    }
    REAL hitRange = data.sensors.getSensor(FRONT)->hit;
    if (hitRange <= sg_helperSmartTurningFrontBotThinkRange * data.turnSpeedFactorF())
    {
        // calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);
        helper_->rubberData->calculate();
        gHelperEmergencyTurn emergencyTurn(owner_, helper_);

        if (helper_->rubberData->rubberUsedRatio >= sg_helperSmartTurningFrontBotActivationRubber || hitRange <= sg_helperSmartTurningFrontBotActivationSpace)
        {
            bool turnMade = false;
            gTurnData *turnData = emergencyTurn.getTurn();
            if (turnData->exist && turnData->numberOfTurns > 0)
            {
                for (int i = 0; i < turnData->numberOfTurns; i++)
                {
                    if (makeTurnIfPossible(data, turnData->direction, 0) == true)
                    {

                        HelperDebug::Debug("smartTurningFrontBot",
                                           "Turn made: " + std::string(turnData->direction == -1 ? "LEFT" : "RIGHT") + " Reason: " + (turnData->reason), "");
                        turnMade = true;
                    }
                }
            }
            if (sg_helperSmartTurningFrontBotDisableTime > 0 && turnMade)
            {
                owner_->turnIgnoreTime = helper_->CurrentTime() + (sg_helperSmartTurningFrontBotDisableTime);
            }
        }
    }
}

REAL Distance(SensorPub const &a, SensorPub const &b, gCycle *owner_)
{
    // make sure a is left from b
    if (a.Direction() * b.Direction() < 0)
        return Distance(b, a, owner_);

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
        REAL ret =
            a.hitDistance_ + b.hitDistance_;

        if (rim)
        {
            ret = bigDistance * .001 + ret * .01 + (a.before_hit - b.before_hit).Norm();

            // we love going between the rim and enemies
            if (!self)
                ret = bigDistance * 2;
        }

        // minimal factor should be 1, this path should never return something smaller than the
        // paths where only one cycle's walls are hit
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
        return (fabsf(a.hitDistance_ - b.hitDistance_) + .25 * bigDistance) * selfHatred;
    }

    else if ((-2 * a.lr * (a.windingNumber_ - b.windingNumber_) > owner_->Grid()->WindingNumber()))
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

// gTurnData * gSmartTurning::getEmergencyTurn(gHelperData &data)
//     {
//         REAL chatBotNewWallBlindness = 0;
//         REAL chatBotMinTimestep      = 0;
//         REAL chatBotDelay            = 0;
//         REAL chatBotRange            = 15;
//         REAL chatBotDecay            = 0;
//         REAL chatBotEnemyPenalty     = 0;

//         REAL lookahead = chatBotRange;  // seconds to plan ahead
//         REAL minstep   = chatBotMinTimestep; // minimum timestep between thoughts in seconds

//         // cylce data
//         REAL speed = owner_->Speed();
//         eCoord dir = owner_->Direction();
//         eCoord pos = owner_->Position();

//         REAL range = speed * lookahead;
//         eCoord scanDir = dir * range;

//         REAL frontFactor = .5;

//         SensorPub front(owner_,pos,scanDir);
//         front.detect(frontFactor);
//         owner_->enemyInfluence.AddSensor( front, chatBotEnemyPenalty, owner_ );

//         REAL minMoveOn = 0, maxMoveOn = 0, moveOn = 0;

//         // get extra time we get through rubber usage
//         calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);

//         if ( !front.ehit )
//             return (new gTurnData(false));

//         // these checks can hit our last wall and fail. Temporarily set it to NULL.
//         tJUST_CONTROLLED_PTR< gNetPlayerWall > lastWall = owner_->lastWall;
//         owner_->lastWall = NULL;

//         REAL narrowFront = 1;

//         // cast four diagonal rays
//         SensorPub forwardLeft ( owner_, pos, scanDir.Turn(+1,+1 ) );
//         SensorPub backwardLeft( owner_, pos, scanDir.Turn(-1,+narrowFront) );
//         forwardLeft.detect(1);
//         backwardLeft.detect(1);
//         SensorPub forwardRight ( owner_, pos, scanDir.Turn(+1,-1 ) );
//         SensorPub backwardRight( owner_, pos, scanDir.Turn(-1,-narrowFront) );
//         forwardRight.detect(1);
//         backwardRight.detect(1);

//         // determine survival chances in the four directions
//         REAL frontOpen = Distance ( forwardLeft, forwardRight,owner_ );
//         REAL leftOpen  = Distance ( forwardLeft, backwardLeft,owner_ );
//         REAL rightOpen = Distance ( forwardRight, backwardRight,owner_ );
//         REAL rearOpen = Distance ( backwardLeft, backwardRight ,owner_);

//         SensorPub self( owner_, pos, scanDir.Turn(-1, 0) );
//         // fake entries
//         self.before_hit = pos;
//         self.windingNumber_ = owner_->windingNumber_;
//         self.type = gSENSOR_SELF;
//         self.hitDistance_ = 0;
//         self.hitOwner_ = owner_;
//         self.hitTime_ = helper_->CurrentTime();
//         self.lr = -1;
//         REAL rearLeftOpen = Distance( backwardLeft, self ,owner_);
//         self.lr = 1;
//         REAL rearRightOpen = Distance( backwardRight, self ,owner_);
//         bool sg_localBot = true;

//         // get the best turn direction
//         uActionPlayer * bestAction = ( leftOpen > rightOpen ) ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
//         int             bestDir      = ( leftOpen > rightOpen ) ? LEFT : RIGHT;
//         REAL            bestOpen     = ( leftOpen > rightOpen ) ? leftOpen : rightOpen;
//         SensorPub &        bestForward  = ( leftOpen > rightOpen ) ? forwardLeft : forwardRight;
//         SensorPub &        bestBackward = ( leftOpen > rightOpen ) ? backwardLeft : backwardRight;

//         SensorPub direct ( owner_, pos, scanDir.Turn( 0, bestDir) );
//         direct.detect( 1 );

//         // restore last wall
//         owner_->lastWall = lastWall;

//         // only turn if the hole has a shape that allows better entry after we do a zig-zag, or if we're past the good turning point
//         // see how the survival chance is distributed between forward and backward half
//         REAL forwardHalf  = Distance ( direct, bestForward,owner_ );
//         REAL backwardHalf = Distance ( direct, bestBackward,owner_ );

//         REAL forwardOverhang  = bestForward.HitWallExtends( bestForward.Direction(), pos );
//         REAL backwardOverhang  = bestBackward.HitWallExtends( bestForward.Direction(), pos );

//         // we have to move forward this much before we can hope to turn
//         minMoveOn = bestBackward.HitWallExtends( dir, pos );

//         // maybe the direct to the side sensor is better?
//         REAL minMoveOnOther = direct.HitWallExtends( dir, pos );

//         // determine how far we can drive on
//         maxMoveOn      = bestForward.HitWallExtends( dir, pos );
//         REAL maxMoveOnOther = front.HitWallExtends( dir, pos );
//         if ( maxMoveOn > maxMoveOnOther )
//             maxMoveOn = maxMoveOnOther;

//         if ( maxMoveOn > minMoveOnOther && forwardHalf > backwardHalf && direct.hitOwner_ == bestBackward.hitOwner_ )
//         {
//             backwardOverhang  = direct.HitWallExtends( bestForward.Direction(), pos );
//             minMoveOn = minMoveOnOther;
//         }

//         // best place to turn
//         moveOn = .5 * ( minMoveOn * ( 1 + rubberRatio ) + maxMoveOn * ( 1 - rubberRatio ) );

//         if ( frontOpen < bestOpen &&
//                 ( forwardOverhang <= backwardOverhang || ( minMoveOn < 0 && moveOn < minstep * speed ) ) )
//         {
//             minMoveOn = maxMoveOn = moveOn = 0;
//             {
//                 return (new gTurnData(bestDir,2,"frontOpen < bestOpen"));
//             }

//         }
//         else
//         {
//             // the best
//             REAL bestSoFar = frontOpen > bestOpen ? frontOpen : bestOpen;
//             bestSoFar *= ( 10 * ( 1 - rubberRatio ) + 1 );

//             if ( rearOpen > bestSoFar && ( rearLeftOpen > bestSoFar || rearRightOpen > bestSoFar ) )
//             {
//                 bool goLeft = rearLeftOpen > rearRightOpen;

//                 // dead end. reverse into the opposite direction of the front wall
//                 uActionPlayer * bestAction = goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
//                 int bestDir = goLeft ? LEFT : RIGHT;
//                 uActionPlayer * otherAction = !goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
//                 int otherDir = bestDir * -1;
//                 SensorPub &        bestForward  = goLeft ? forwardLeft : forwardRight;
//                 SensorPub &        bestBackward  = goLeft ? backwardLeft : backwardRight;
//                 SensorPub &        otherForward  = !goLeft ? forwardLeft : forwardRight;
//                 SensorPub &        otherBackward  = !goLeft ? backwardLeft : backwardRight;

//                 // space in the two directions available for turns
//                 REAL bestHit = bestForward.hit > bestBackward.hit ? bestBackward.hit : bestForward.hit;
//                 REAL otherHit = otherForward.hit > otherBackward.hit ? otherBackward.hit : otherForward.hit;

//                 bool wait = false;

//                 // well, after a short turn to the right if space is tight
//                 if ( bestHit * lookahead < owner_->GetTurnDelay() + rubberTime )
//                 {
//                     if ( otherHit < bestForward.hit * 2 && front.hit * lookahead > owner_->GetTurnDelay() * 2 )
//                     {
//                         // wait a bit, perhaps there will be a better spot
//                         wait = true;
//                     }
//                     else if (!wait)
//                     {

//                         //return (new gTurnData(otherDir,2));;

//                         // there needs to be space ahead to finish the maneuver correctly
//                         if (!( maxMoveOn < speed * owner_->GetTurnDelay() )) {
//                                 return (new gTurnData(bestDir,1,"maxMoveOn < speed"));
//                         }
//                     }
//                 }

//                 if (!wait)
//                     return (new gTurnData(bestDir,1,"!wait"));

//             }
//         }

//         return (new gTurnData(false));
//     }

bool gSmartTurning::CanMakeTurn(uActionPlayer *action)
{
    return owner_->CanMakeTurn((action == &gCycle::se_turnRight) ? 1 : -1);
}

gSmartTurning &gSmartTurning::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(helper);

    // create
    if (helper->smartTurning.get() == 0)
        helper->smartTurning.reset(new gSmartTurning(helper, owner));

    return *helper->smartTurning;
}
