#include "gSmarterBot.h"
#include "../../gAIBase.h"
#include "../../gSensor.h"
#include "../../gCycle.h"
#include "../../gWall.h"
#include "eDebugLine.h"
#include "gHelperSensor.h"
#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "tRandom.h"
#include "eTimer.h"
#include "ePlayer.h"

#include "../gHelperHud.h"
#include "../gHelperUtilities.h"
REAL AI_EMERGENCY = 10;
static tConfItem<REAL> AI_EMERGENCYC("SMARTER_BOT_EMERGENCY", AI_EMERGENCY);
REAL AI_LOOP = 10;
static tConfItem<REAL> AI_LOOPC("SMARTER_BOT_LOOP", AI_LOOP);
REAL AI_ENEMY = 30;
static tConfItem<REAL> AI_ENEMYC("SMARTER_BOT_ENEMY", AI_ENEMY);
REAL AI_STATE_TRACE = 10;
static tConfItem<REAL> AI_STATE_TRACEC("SMARTER_BOT_TRACE", AI_STATE_TRACE);
REAL AI_STATE_CLOSECOMBAT = 30;
static tConfItem<REAL> AI_STATE_CLOSECOMBATC("SMARTER_BOT_CLOSECOMBAT", AI_STATE_CLOSECOMBAT);
REAL AI_STATE_PATH = 20;
static tConfItem<REAL> AI_STATE_PATHC("SMARTER_BOT_PATH", AI_STATE_PATH);
REAL AI_RANGE = 10;
static tConfItem<REAL> AI_RANGEC("SMARTER_BOT_RANGE", AI_RANGE);
REAL AI_TUNNEL = 5;
static tConfItem<REAL> AI_TUNNELC("SMARTER_BOT_TUNNEL", AI_TUNNEL);

REAL sg_smarterBotNextTimeMult = 1;
static tConfItem<REAL> sg_smarterBotNextTimeMultC("SMARTER_BOT_NEXT_TIME_MULT", sg_smarterBotNextTimeMult);
// int AI_REACTION = 10;
// static tConfItem<REAL> AI_REACTIONC("SMARTER_BOT_REACTION", AI_REACTION);

const REAL relax=25;

gHelperHudItem<tColoredString> smarterBotH("Smarter Bot",tColoredString("None"));


gAISensor::gAISensor(gCycle *c,
                     const eCoord & start, const eCoord & dir,
                     REAL sideScan,
                     REAL frontScan,
                     REAL corridorScan,
                     int winding)
    : cycle(c), distance(frontScan * 2)
{
    hit = false;

    gSensor ahead(cycle, start, dir);

    int count = 0;

    do
    {
        ahead.detect(frontScan);
        front.AddHit(start, dir, ahead, winding);
        if (front.Hit())
        {
            hit = true;
            distance = front.distance;
            if (AI_LOOP> 3 + abs(winding) * 3)
                DetectLoop(front, frontLoop);
        }
    } while (!front.Hit() && count++ < AI_RANGE);

    if (distance * .99f < corridorScan)
        corridorScan = distance * .99f;
    corridorScan -= sideScan * .02;
    if (corridorScan < 0.1f)
        corridorScan = 0.1f;

    if (Random() * 10 < AI_TUNNEL)
    {
        eCoord lookTunnel = start + dir * corridorScan;
        int i;

        for (i = 1; i >= 0; i--)
        {
            int lr = i + i - 1;
            eCoord lrDir = dir.Turn(eCoord(.01f, -lr));

            gSensor side(cycle, start, lrDir);
            side.detect(sideScan * 1.01f);
            REAL thisSideScan = side.hit * .99f;

            gSensor tunnel(cycle, lookTunnel, lrDir);
            tunnel.detect(thisSideScan);
            sides[i].AddHit(start, dir, tunnel, winding + lr);

            gSensor parallel(cycle, start + lrDir * thisSideScan, dir);
            parallel.detect(corridorScan);
            sides[i].AddHit(start, dir, parallel, winding);

            if (sides[i].Hit())
            {
                if (AI_LOOP> 6 + abs(winding) * 3)
                    DetectLoop(sides[i], sideLoop[i]);

                if (sideLoop[i][1 - i].loop ||
                    (AI_TUNNEL >= 10 &&
                     sides[i].otherCycle &&
                     sides[i].otherCycle->Team() != cycle->Team() &&
                     sides[i].lr * (i + i - 1) < 0 &&
                     sides[i].otherCycle->GetDistance() < sides[i].driveDistance + sides[i].otherCycle->Speed() * 20))
                {
                    if (sides[i].distance < distance)
                        distance = sides[i].distance;

                    hit = true;
                }
            }
        }
    }
    if (sides[1].otherCycle == sides[0].otherCycle && sides[0].otherCycle)
        hit = true;
}


void gSmarterBot::Activate(){
     REAL time = se_GameTime();

     // when our turning distrupts its waiting time
        if (owner_->lastBotTurnTime < owner_->lastTurnTime) {
            nextTime = 0;
        }

        // if (owner_->lastBotTurnTime < time + 6) {
        //     nextStateChange = 0;
        // }

    tColoredString debug;
    debug << "State " << state << "\n";
    debug << "nextTime " << nextTime << "\n";

    smarterBotH << debug;

    // debug << "helperAI time " << time << "\n";
    // debug << "should quit? " << bool( Object() && Object()->LastTime() < time - EPS ) << "\n";
    // debug << Object()->LastTime() << " < " << time << " - " << EPS << " (" << time-EPS << ")" << "\n";
    // debug << "Activate ? " << (bool(Object() && Object()->LastTime() < time - EPS) == false) << "\n";
    // debug << nextTime << " < " << time << " ? " << bool(nextTime<time) << "\n";

    if (nextTime > se_GameTime())
        return;

    //smarterBotH << debug;
    REAL nextthought=Think();
    nextTime *= sg_smarterBotNextTimeMult;
    nextTime = se_GameTime() + nextthought;

    // REAL ts=time-lastTime;
    // lastTime=time;

    // if (concentration < 0)
    //     concentration = 0;

    // concentration += 4*(AI_REACTION) * ts/relax;
    // concentration=concentration/(1+ts/relax);


    // if (bool(Object()) && Object()->Alive()){ && ((nextTime<time) )){
    //     REAL nextthought=Think();
    //     //    if (nextthought>.9) nextthought=REAL(.9);

    //     if (nextthought<REAL(.6-concentration)) nextthought=REAL(.6-concentration);

    //     nextTime=nextTime+nextthought;

    //     //con << concentration << "\t" << nextthought << '\t' << ts << '\n';

    //     if(.1+4*nextthought<1)
    //         concentration*=REAL(.1+4*nextthought);
    // }

}
void gSmarterBot::Chat(gCHAT_ACTION type)
{
    gAIChat::Chat(type,owner_->Player());
}

void gSmarterBot::Chat(tString message)
{
    gAIChat::Chat(message,owner_->Player());
}

void gSmarterBot::ActOnData( ThinkData & data )
{
    // delegate
    ThinkDataBase & base = data;
    ActOnData( base );

    // sanitize next think time so it will be before we hit the next wall
    if ( Object()->Speed() > 0 && data.thinkAgain > 0 )
    {
        gSensor front( Object(), Object()->Position(), Object()->Direction() );
        front.detect( Object()->Speed() * data.thinkAgain * 1.5 );
        if ( front.ehit )
        {
            REAL thinkAgain = ( front.hit / Object()->Speed() ) * .8;
            if ( data.thinkAgain > thinkAgain )
                data.thinkAgain = thinkAgain;
        }
    }
}

void gSmarterBot::ActOnData( ThinkDataBase & data )
{
    // archive decision
    ThinkDataBase copy = data;

    // execute turn
    if ( data.turn ) {
            Object()->BotTurn(data.turn < 0 ? LEFT : RIGHT);
    }
}

gSmarterBot::gSmarterBot(gCycle *owner): owner_(owner),
    lastPath(se_GameTime()-100),
        lastTime(se_GameTime()),
        nextTime(0)
{
    ClearTarget();
    traceSide = 1;
    freeSide = 0;
    // log = tNEW(gAILog);

    lastTime = 0;
    lastPath = 0;
    lastChangeAttempt = 0;
    lazySideChange = 0;
    path.Clear();
    concentration = 1000;
    state = SURVIVE;
    nextStateChange = 0;
    //Chat(TRAPPED);
}

REAL gSmarterBot::Think(){
    // get the delay between two turns
    REAL delay = Delay();

    if (!Object()->Alive())
        return 100;

    emergency = false;
    //  return 1;

    // first, find close eWalls and evade them.
    REAL speed=Object()->Speed();
    REAL range=speed+100;
    eCoord dir=Object()->Direction();
    REAL side=speed*delay;

    REAL corridor = range;
    if (corridor < side * 2)
        corridor = side * 2;

     gAISensor front(Object(),Object()->Position(),dir, side * 2, range, corridor, 0);

    // get the sensors to the left and right with the most free space
    int currentDirectionNumber = Object()->Grid()->DirectionWinding( dir );
    REAL mindistLeft = 1E+30, mindistRight = 1E+30;
    std::unique_ptr< gAISensor > left  ( sg_GetSensor(currentDirectionNumber, *Object(), -1, side, range, corridor, mindistLeft ) );
    std::unique_ptr< gAISensor > right ( sg_GetSensor(currentDirectionNumber, *Object(), 1, side, range, corridor, mindistRight ) );

    // count intermediate walls to the left and right as if they were in front
    {
        REAL mindistFront = mindistLeft > mindistRight ? mindistLeft : mindistRight;
        if ( mindistFront < front.distance )
        {
            front.distance = mindistFront;
        }
    }

    // switch to survival state if our victim died:
    if (state != SURVIVE && state != TRACE && (!target || !target->Alive())) {
        SwitchToState(SURVIVE, 1);
    }

    triesLeft = 10;

    REAL ret = 1;

    //not the best solution, but still better than segfault...
    if(left.get() != 0 && right.get() != 0) {
        ThinkData data( front, *left, *right);
//        con << "STATE " << state << "\n";
        switch (state)
        {
        case SURVIVE:
            ThinkSurvive(data);
            break;
        case PATH:
            ThinkPath(data);
            break;
        // case AI_ROUTE:
        //     ThinkRoute(data);
        //     break;
        case TRACE:
            ThinkTrace(data);
            break;
        case CLOSECOMBAT:
            ThinkCloseCombat(data);
            break;
        }
        ActOnData( data );
        ret = data.thinkAgain;
    }


    REAL mindist = front.distance * front.distance * 8;

    const tList<eGameObject>& gameObjects = Object()->Grid()->GameObjects();

    // find the closest enemy
    for (int i=gameObjects.Len()-1;i>=0;i--){
        gCycle *other=dynamic_cast<gCycle *>(gameObjects(i));

        if (other && other != Object()){
            // then, enemy is realy an enemy
            eCoord otherpos=other->Position()-Object()->Position();
            REAL dist = otherpos.NormSquared();

            if (dist < mindist)
                mindist = dist;
        }
    }

    mindist = sqrt(mindist) / (3 * Object()->Speed());

    if (ret > mindist)
        ret = mindist;

    return ret;
}

// state change:
void gSmarterBot::SwitchToState(gSTATE nextState, REAL minTime)
{
    lastState = state;
    lastStateChange = se_GameTime();
    int thisAbility = 10 - AI_STATE_TRACE;
    switch (state)
    {
    case TRACE:
        thisAbility = AI_STATE_TRACE;
        break;
    case CLOSECOMBAT:
        thisAbility = AI_STATE_CLOSECOMBAT;
        break;
    case PATH:
        thisAbility = AI_STATE_PATH;
        break;
    default:
        break;
    };

    int nextAbility = 10;
    switch (nextState)
    {
    case TRACE:
        nextAbility = AI_STATE_TRACE;
        break;
    case CLOSECOMBAT:
        nextAbility = AI_STATE_CLOSECOMBAT;
        break;
    case PATH:
        nextAbility = AI_STATE_PATH;
        break;
    default:
        break;
    };


    // if (nextAbility > thisAbility && Random() * 10 > nextAbility)
    //     return;


    // if (state != nextState) {
    //     tString message;
    //     message << "Switching to state " << nextState << "\n";
    //     con << message;
    // }

    state           = nextState;
    nextStateChange = se_GameTime() + minTime;
}


void gSmarterBot::SetTraceSide(int side)
{
    REAL time = se_GameTime();
    REAL ts   = time - lastChangeAttempt + 1;
    lastChangeAttempt = time;

    lazySideChange += ts * side;
    if (lazySideChange * traceSide <= 0)
    {
        // state change!
        traceSide = lazySideChange > 0 ? 1 : -1;
        lazySideChange = 10 * traceSide;
    }

    if (lazySideChange > 10)
        lazySideChange = 10;
    if (lazySideChange < -10)
        lazySideChange = -10;
}


void gSmarterBot::ThinkSurvive(  ThinkData & data )
{

    REAL random = 0;

    bool turnRight = data.left.front.wallType == gSENSOR_RIM;// || data.left.front.wallType == gSENSOR_RIM && data.right.front.wallType == gSENSOR_RIM && data.right.front.distance > data.left.front.distance;
    bool turnLeft  = data.right.front.wallType == gSENSOR_RIM;// || data.right.front.wallType == gSENSOR_RIM && data.left.front.wallType == gSENSOR_RIM && data.left.front.distance > data.right.front.distance;
    if ( turnRight )
        EmergencySurvive( data, 1);
    else if (turnLeft)
        EmergencySurvive( data, -1);
    else
        EmergencySurvive( data );

    //con << "nextStateChange " << nextStateChange << "\n";
    if (nextStateChange < se_GameTime())
    {
        data.thinkAgain = .5f;


    // switch from Survival to close combat if surviving is too boring
    random=10*Random();
    //con << "RANDOM = " << random << "\n";
    if (random <8)
    {
        // find a new victim:
        eCoord enemypos=eCoord(1000,100);

        const tList<eGameObject>& gameObjects = Object()->Grid()->GameObjects();
        gCycle *secondbest = NULL;

        // find the closest enemy
        for (int i=gameObjects.Len()-1;i>=0;i--){
            gCycle *other=dynamic_cast<gCycle *>(gameObjects(i));

            if (other && other->Team()!=Object()->Team() &&
                    !IsTrapped(other, Object())){
                // then, enemy is realy an enemy
                eCoord otherpos=other->Position()-Object()->Position();
                if (otherpos.NormSquared()<enemypos.NormSquared()){
                    // check if the path is clear
                    gSensor p(Object(),Object()->Position(),otherpos);
                    p.detect(REAL(.98));
                    secondbest = dynamic_cast<gCycle *>(other);
                    if (p.hit>=.98){
                        enemypos = otherpos;
                        target = secondbest;
                    }
                }
            }
        }

        if (!target)
            target = secondbest;

        if (!target)
            target = owner_;

        if (target)
            SwitchToState(CLOSECOMBAT, 1);
    }

    data.thinkAgain = 1;
}
}


// emergency functions:
bool gSmarterBot::EmergencySurvive( ThinkData & data, int enemyevade, int preferedSide)
{
    gAISensor const & front = data.front;
    gAISensor const & left = data.left;
    gAISensor const & right = data.right;

    triesLeft = AI_EMERGENCY;

    freeSide *= .95;

    int i, j;

    // don't do a thing if there may be a better way out of we drive on:
    if (triesLeft > 0 &&
            front.front.otherCycle &&
            front.front.otherCycle != Object() &&
            ((front.frontLoop[1].loop && front.front.otherCycle != left .front.otherCycle && left .front.otherCycle)||
             (front.frontLoop[0].loop && front.front.otherCycle != right.front.otherCycle && right.front.otherCycle ) )
       )
        return false;

    // get the delay between two turns
    REAL delay = Delay();
    REAL range = Object()->Speed() * delay;

    // nothing we can do if we cannot make a turn immediately
    if (!Object()->CanMakeTurn(1) || !Object()->CanMakeTurn(-1))
        return false;

    //  bool dontCheckForLoop[2] = { false, false };


    // look out if there is anything bad going on in one of the directions:
    // [signifficance: danger level of n: You'll be (as good as) dead in [10/n delay times] if you drive that way
    int sideDanger[DANGERLEVELS][2];
    int frontDanger[DANGERLEVELS];
    for(i = DANGERLEVELS-1; i>=0; i--)
    {
        sideDanger[i][0] = 0;
        sideDanger[i][1] = 0;
        frontDanger[i]   = 0;
    }

    bool canTrapEnemy = false;

    if (emergency)
    {
        frontDanger[SPACELEVEL] += 40;
    }

    const gAISensor* sides[2];
    sides[0] = &left;
    sides[1] = &right;

    // avoid loops:

    bool isTrapped = IsTrapped(Object(), NULL);

    /*
      if (front.front.wallType == gSENSOR_ENEMY)
      sideDanger[LOOPLEVEL][(1-front.front.lr*enemyevade) >> 1] += 5;
    */

    if (!isTrapped)
        for (i = 1; i>=0; i--)
        {
            if (front.frontLoop[i].loop && front.distance < 5*sides[i]->distance)
            {
                // if we would close ourself in, make the danger bigger
                if (front.front.otherCycle == Object() && i+i-1 == front.front.lr)
                    sideDanger[LOOPLEVEL][i]+=40;

                sideDanger[LOOPLEVEL][i]+=40;
                for (j = front.frontLoop[i].closedIn.Len()-1; j>=0; j--)
                    if (front.frontLoop[i].closedIn(j) == target)
                        canTrapEnemy = true;
            }

            for (j = 1; j>=0; j--)
                if (front.sideLoop[i][j].loop)
                    sideDanger[LOOPLEVEL][j]++;

            // if we would close ourselfs in by a zigzag in direction i,
            // but not by a u-turn and there is enough space for a u-turn,
            // do it.
            if (sides[i]->frontLoop[1-i].loop  &&
                    !sides[i]->frontLoop[i].loop)
            {
                if (sides[i]->distance > range)
                {
                    frontDanger[LOOPLEVEL]     += 20;
                    sideDanger[LOOPLEVEL][1-i] += 10;
                }
                else // try to make some room so we can evade:
                {
                    frontDanger[LOOPLEVEL]     += 20;

                    sideDanger[LOOPLEVEL][i]   += 10;
                }
            }

            // if we would close ourselves in by a U-Turn, don't do it.
            //	if (sides[i]->frontLoop[i].loop && sides[i].distance < range * 2)
            //	  sideDanger[LOOPLEVEL][i] += 40;
        }

    // try to trap the enemy
    if (AI_LOOP >= 10 && canTrapEnemy && !emergency)
        return false;


    /*
      // avoid closing yourself or a teammate in.
      if (front.type == gSENSOR_SELF || front.type == gSENSOR_TEAMMATE)
      {
         if (front.lr > 0)
      sideDanger[][1] +=2;
         else
      sideDanger[][0] +=2;
      }
    */



    {
        if (front.Hit() &&
                ( front.distance + range < sides[0]->distance ||
                  front.distance + range < sides[1]->distance) )
        {
            if ( front.front.wallType == gSENSOR_RIM)
                frontDanger[SPACELEVEL] += static_cast<int>(100 * range * gArena::SizeMultiplier() / (front.distance + range * .1));

            frontDanger[SPACELEVEL] += static_cast<int>(5 * range / (front.distance + range *.2));

            if (front.distance < range)
                frontDanger[SPACELEVEL] += static_cast<int>(20 * range / (front.distance + range *.2)) + 1;
        }


        // avoid close corners:
        for (i = 1; i>=0; i--)
        {
            if (sides[i]->Hit() && //sides[i]->distance < range * 3 &&
                    sides[i]->distance < front.distance + range)
            {
                if ( sides[i]->front.wallType == gSENSOR_RIM)
                    sideDanger[SPACELEVEL][i] += static_cast<int>(150 * range * gArena::SizeMultiplier() / (sides[i]->distance + range * .1));

                sideDanger[SPACELEVEL][i] += static_cast<int>
                                             (range * 5 / (sides[i]->distance + range * .1));

                if (sides[i]->distance < range)
                    sideDanger[SPACELEVEL][i] += static_cast<int>
                                                 (range * 20 / (sides[i]->distance + range * .1));
            }

            // give us a chance to turn around:
            if (frontDanger[SPACELEVEL] * 2 < sideDanger[SPACELEVEL][i])
                sideDanger[LOOPLEVEL][i-i] -= sideDanger[SPACELEVEL][i] * 2;
        }
    }

    // avoid close proximity to other cycles
    const gCycle* target = NULL;
    const tList<eGameObject>& gameObjects = Object()->Grid()->GameObjects();
    gCycle *secondbest = NULL;
    REAL closest = 1000000;
    eCoord dir     = Object()->Direction();

    // find the closest enemy
    for (i=gameObjects.Len()-1;i>=0;i--){
        gCycle *other=dynamic_cast<gCycle *>(gameObjects(i));

        if (other && other->Alive() && other != Object())
        {
            eCoord otherpos=other->Position()-Object()->Position();
            REAL otherNorm = otherpos.NormSquared();

            bool nothit = false;
            if (otherNorm < closest * 4)
            {
                gSensor p(other, other->Position(), -otherpos);
                p.detect(REAL(.9999));
                gSensor q(Object(), Object()->Position(), otherpos);
                q.detect(REAL(.9999));

                nothit = p.hit>=.999 && q.hit >=.999;
            }

            if (other->Team() != Object()->Team())
            {
                // then, enemy is realy an enemy
                //      REAL s = Object()->Speed() * 50;
                if (/* otherNorm < s*s && */ otherNorm < closest)
                {
                    // check if the path is clear
                    secondbest = dynamic_cast<gCycle *>(other);
                    if (nothit){
                        closest = otherNorm;
                        target = secondbest;
                    }
                }
            }
            else if (nothit)
            {
                // he is a teammate. Avoid him.

                eCoord friendpos=other->Position() - Object()->Position();

                // transform coordinates relative to us:
                friendpos=friendpos.Turn(dir.Conj()).Turn(0,1);

                if (friendpos.y > fabs(friendpos.x) * 1.5f)
                    frontDanger[TEAMLEVEL] += 10;
                if (friendpos.x * 2 > -friendpos.y)
                    sideDanger[TEAMLEVEL][1] += 10;
                else if (-friendpos.x * 2 > -friendpos.y)
                    sideDanger[TEAMLEVEL][0] += 10;
            }
        }
    }

    //  if (!target)
    //target = secondbest;

    if (target && AI_ENEMY > 0)
    {
        bool sdanger = false;
        for (i = DANGERLEVELS-1; i>=0; i--)
            sdanger |= sideDanger[i][0] > 4 || sideDanger[i][1] > 4;

        eCoord enemypos=target->Position() - Object()->Position();
        eCoord enemydir=target->Direction();
        REAL enemyspeed=target->Speed();


        // transform coordinates relative to us:
        enemypos=enemypos.Turn(dir.Conj()).Turn(0,1);
        enemydir=enemydir.Turn(dir.Conj()).Turn(0,1);

        if (AI_ENEMY > 7)
        {
            // would he be able to trap us if we drive straight on?
            // bool trap[2] = {false, false};

            if (!isTrapped)
                for (i = 1; i>=0; i--)
                {
                    // if the enemy comes racing towards us, check if he could
                    // close us in by touching our own line ON THE OPPOSITE side of i
                    tArray<const gCycle*> closedIn;
                    int winding = 0;

                    bool loop = CheckLoop(target, Object(),
                                          Object()->GetDistance() + 4 * TOL, i, 0,
                                          closedIn, winding);

                    winding -= Object()->WindingNumber();
                    winding += target->WindingNumber();


                    // yes! we shoult turn in direction 1-i to get the target
                    // to the other side.
                    if (loop)
                        if (winding * (i+i-1) < 0)
                        {
                            // trap[i] = true;
                            REAL x = enemypos.x * (i+i-1);
                            REAL y = enemypos.y;

                            bool canAccelerateByTurning =
                                ( sides[1-i]->Hit() &&
                                  sides[1-i]->distance < Object()->Speed() * delay * 5 &&
                                  sides[i-i]->distance > Object()->Speed() * delay &&
                                  !sides[i-i]->frontLoop[i].loop) ;

                            bool ohShit = target->Speed() > Object()->Speed() + sqrt(closest);

                            if (ohShit)
                            {
                                SetTraceSide(-(i+i-1));
                                SwitchToState(TRACE, 10);
                            }

                            bool turningIsFutile =
                                front.front.otherCycle == Object() &&
                                sides[1-i]->front.otherCycle == Object() &&
                                front.distance < sides[1-1]->distance * 10 ;

                            if (
                                x < 0 &&
                                (
                                    x * Object()->Speed() < -y * target->Speed() + 1000 ||
                                    canAccelerateByTurning || ohShit
                                )
                                &&
                                !turningIsFutile
                            )
                            {
                                if (enemydir.y < -.2f && y < 0)
                                    SetTraceSide(-(i+i-1));

                                frontDanger[TRAPLEVEL]    += 10;
                            }

                            if ( y > 0 || x < 0 || ohShit
                                    //			   ( y * Object()->Speed() > x * target->Speed()*.9 - 200 || enemyspeed.x * (i+i-1)
                               )
                                sideDanger[TRAPLEVEL][i] += 20;
                            // sideDanger[TRAPLEVEL][i] ++;
                        }
                }
        }

        if (AI_ENEMY > 0)
        {
            // imminent collision check
            REAL totalspeed = enemyspeed + Object()->Speed();

            if ((fabs(enemypos.y) < totalspeed * .3f && fabs(enemypos.x) < totalspeed * .3f))
            {
                REAL diffSpeed  = -enemydir.y * enemyspeed + Object()->Speed();
                if (diffSpeed > 0 && enemydir.y <= .2)
                {
                    REAL enemyFront = enemypos.y / diffSpeed;
                    REAL enemySide  = fabs(enemypos.x) / diffSpeed;
                    if (enemyFront > 0 && enemyFront < .4 + enemySide && fabs(enemypos.y) > fabs(enemypos.x))
                    {
                        frontDanger[COLIDELEVEL] += 1 + int(4 / (enemyFront + .01));
                        //		      SwitchToState( AI_SURVIVE, enemyFront * 4 + 2 );
                    }
                }

                int side = enemypos.x > 0 ? 1 : 0;

                // can we cut him instead of evade him?
                if (Object()->Team() != target->Team() &&
                        ( ( enemydir.y <= -.2 &&
                            enemypos.y*target->Speed()*1.1 > fabs(enemypos.x) * Object()->Speed() ) ||
                          sideDanger[COLIDELEVEL][side] > 0))
                    sideDanger[COLIDELEVEL][1-side]+=5;
                else if ( -(enemypos.y + .3f) * Object()->Speed() < fabs(enemypos.x) * target->Speed()*1.2)
                    sideDanger[COLIDELEVEL][side]+=10;
            }
        }
    }


    // determine the total danger levels by taking the max of the individual experts:
    int fDanger = 0;
    int sDanger[2] = { 0, 0 };
    for (i = 0; i<DANGERLEVELS; i++)
        // for (i = 1; i< 2; i++)
    {
        if (!fDanger || frontDanger[i] > fDanger + 2)
            fDanger = frontDanger[i];

        for (int j=1; j>=0; j--)
            if (!sDanger[j] || sideDanger[i][j] > sDanger[j] + 2)
                sDanger[j] = sideDanger[i][j];
    }

    // nothing to do if we are not in immediate danger.
    if (!fDanger && !preferedSide)
        return false;


    // decide about your direction:
    int turn = 0;

    turn += sDanger[0];
    turn -= sDanger[1];

    if (!turn)
        turn = (int) freeSide;

    if (!turn && front.front.wallType != gSENSOR_RIM)
        turn = front.front.lr * enemyevade;

    if (!turn)
        turn = (sides[0]->distance > sides[1]->distance ? -1 : 1);


    // switch to survival mode if we just trapped an enemy
    if (canTrapEnemy)
    {

        SwitchToState(TRACE, 10);

        if (turn)
            this->SetTraceSide(-turn);
    }



    int side = 1;
    if (preferedSide < 0)
    {
        for (i = DANGERLEVELS-1; i>=0; i--)
        {
            int dSwap = sideDanger[i][0];
            sideDanger[i][0] = sideDanger[i][1];
            sideDanger[i][1] = dSwap;
        }

        int dSwap = sDanger[0];
        sDanger[0] = sDanger[1];
        sDanger[1] = dSwap;

        sides[1] = &left;
        sides[0] = &right;
        side     = -1;
        preferedSide = 1;
    }


    // no problem in the preferred direction. Just take it.
    if (preferedSide)
    {
        if( fDanger  * 3 >= sDanger[1] * 2 - 5 &&
                sDanger[0] * 3 >= sDanger[1] * 2 - 5)
        {
            freeSide -= side*100;

            data.turn = side;
            return true;
        }

        if (fDanger * 2 <= sDanger[0] * 3 + 3)
        {
            return false;
        }
    }

    // it is safer driving straight on
    if (fDanger <= sDanger[0] + 3 && fDanger <= sDanger[1] + 3 && fDanger < 20)
    {

        return false;
    }


    if (turn)
    {
        freeSide -= side*100;
        data.turn = turn;
    }

    return turn;
}


void gSmarterBot::ThinkCloseCombat( ThinkData & data )
{
    int lr=0;

    REAL nextThought = 0;

    const gAISensor* sides[2];
    sides[0] = &data.left;
    sides[1] = &data.right;

    eCoord dir = Object()->Direction();
    REAL fs=data.front.distance;
    //  REAL ls=left.hit;
    //  REAL rs=right.hit;

    if (nextStateChange > se_GameTime())
    {
        data.thinkAgain = .5f;

        // return;
    }

    if ( bool( target ) && !IsTrapped(target, Object()) && nextStateChange < se_GameTime() )
    {
        gSensor p(Object(),Object()->Position(),target->Position() - Object()->Position());
        p.detect(REAL(1));
        if (p.hit <=  .999999)  // no free line of sight to victim. Switch to path mode.
        {
            SwitchToState(PATH, 5);
            EmergencySurvive( data );
            return;
        }
    }

    REAL ed = 0;

    const REAL fear=REAL(.01);
    const REAL caution=.001;
    const REAL evasive=100;
    const REAL attack=100;
    const REAL seek=REAL(1);
    const REAL trap=REAL(.01);
    const REAL ffar=20;
    //    const REAL close=1000;

    REAL random=10*Random()*Random();

    if ( bool( target ) && target->Alive()){

        eCoord enemypos=target->Position()-Object()->Position();
        eCoord enemydir=target->Direction();
        REAL enemyspeed=target->Speed();

        ed=REAL(fabs(enemypos.x)+fabs(enemypos.y));
        ed/=enemyspeed;

        // transform coordinates relative to us:
        enemypos=enemypos.Turn(dir.Conj()).Turn(0,1);
        enemydir=enemydir.Turn(dir.Conj()).Turn(0,1);

    // Check if the enemy is facing opposite direction of ours
    bool oppositeDirectionofEnemy = directionsAreClose(enemydir, dir.Turn(LEFT).Turn(LEFT), 0.001);

    // Check if the enemy is on the left side of us
    bool enemyIsOnLeft = enemypos.x < 0;

    // Check if the enemy is on the right side of us
    bool enemyIsOnRight = !enemyIsOnLeft;

    // Check if the enemy is facing right direction of ours
    bool enemyIsFacingOurRight = directionsAreClose(enemydir, dir.Turn(RIGHT), 0.1);

    // Check if the enemy is facing left direction of ours
    bool enemyIsFacingOurLeft = directionsAreClose(enemydir, dir.Turn(LEFT), 0.1);

    // If the enemy is facing opposite direction of ours, flip the relative position
    if (oppositeDirectionofEnemy)
    {
        enemypos.y *= -1;
    }
    // If the enemy is facing right direction of ours and on the left side, rotate the relative position
    else if (enemyIsOnLeft && enemyIsFacingOurRight)
    {
        enemypos = enemypos.Turn(LEFT);
    }
    // If the enemy is facing left direction of ours and on the right side, rotate the relative position
    else if (enemyIsOnRight && enemyIsFacingOurLeft)
    {
        enemypos = enemypos.Turn(RIGHT);
    }


        // now we are at the center of the coordinate system facing
        // in direction (0,1).

        // rules are symmetrical: exploit that.
        int side=1;
        if (enemypos.x<0){
            side*=-1;
            enemypos.x*=-1;
            enemydir.x*=-1;

            sides[1] = &data.left;
            sides[0] = &data.right;
            //      Swap(ls,rs);
        }
        // now we can even assume the enemy is on our right side.

        // consider his ping and our reaction time
#define REACTION .2


        //REAL enemyspeed=target->speed;
        REAL ourspeed=Object()->Speed();

        REAL enemydist=target->Lag();

        enemydist+=2*REACTION *enemyspeed;

        REAL ourdist=REACTION*ourspeed;;


        // now we consider the worst case: we drive straight on,
        enemypos.y-=ourdist;
        // while the enemy cuts us: he goes in front of us
        REAL forward=-enemypos.y+.01;
        if (forward<0) // no need to go to much ahead
            forward=0;
        if (forward>enemydist)
            forward=enemydist;

        enemypos.y+=forward;
        enemydist-=forward;

        // and then he turns left.
        enemypos.x-=enemydist;

        if (enemypos.y*enemyspeed>enemypos.x*ourspeed){ // he is right ahead of us.
            if (random<fear){ // evade him
                // Chat(tString("NO!!!!!"));
                lr+=side;

                nextThought += 1;
            }
            if (enemypos.y<=ffar &&
                    ((enemydir.x<0 && random<evasive) ||
                     (enemydir.y>0 && random<caution) ||
                     (enemydir.y<0 && random<attack))){
                // Chat(tString("Caution!!!!!"));
                lr+=side;

                nextThought += 1;

                if (enemyspeed > ourspeed)
                {

	            {
                        SetTraceSide(-side);
                        SwitchToState(TRACE, 10);
		    }
                }
            }
        }
        else if (enemypos.y*ourspeed<-enemypos.x*enemyspeed){
            /*
              // good attack position
              if (enemypos.x<rs && rs < range*.99){
              #ifdef DEBUG
              con << "BOX!\n";
              #endif
              turn+=10;
              lr-=side;
              }

            */

            REAL canCutIfDriveOn = enemypos.x*ourspeed - fs * (enemyspeed - ourspeed);
            canCutIfDriveOn -= enemypos.y*enemyspeed;

            REAL canCutIfAttack  = - sides[1]->distance * enemyspeed
                                   - (sides[1]->distance - enemypos.x -enemypos.y*ourspeed) * ourspeed;

            if (random<attack && (!(data.front.Hit() && data.front.distance < 20) || canCutIfAttack > canCutIfDriveOn)){
#ifdef DEBUG
                con << "attack!\n";
#endif
                lr-=side;

                nextThought += 1;
            }
        }
        else if(enemypos.x>ffar*4){
            if(random<seek){
                // Chat(tString("im coming for you"));
                lr-=side;

                nextThought += 1;
            }
        }
        else if (enemypos.x<ffar*2 && fabs(enemypos.y)<ffar){
            if(random<trap){
            // Chat(tString("trap!!!!!"));
                lr+=side;

                nextThought += 1;
            }
        }
    }

    if (!EmergencySurvive(data, 1, -lr))
        nextThought = 0;

    data.thinkAgain = ed/2 + nextThought;
}


void gSmarterBot::ThinkTrace( ThinkData & data )
{
    gAISensor const & front = data.front;
    gAISensor const & left = data.left;
    gAISensor const & right = data.right;

    bool inverse = front.Hit() && front.distance < Object()->Speed() * Delay();

    if (left.front.wallType == gSENSOR_RIM)
        SetTraceSide(1);

    if (right.front.wallType == gSENSOR_RIM)
        SetTraceSide(-1);

    bool success = EmergencySurvive(data, 0, traceSide * ( inverse ? -1 : 1));

    REAL & nextTurn = data.thinkAgain;
    nextTurn = 100;
    if (left.front.edge)
    {
        REAL a = eCoord::F(Object()->Direction(), *left.front.edge->Point() - Object()->Position());
        REAL b = eCoord::F(Object()->Direction(), *left.front.edge->Other()->Point() - Object()->Position());

        if (a < b)
            a = b;
        if ( a > 0 )
            nextTurn = a;
    }

    if (right.front.edge)
    {
        REAL a = eCoord::F(Object()->Direction(), *right.front.edge->Point() - Object()->Position());
        REAL b = eCoord::F(Object()->Direction(), *right.front.edge->Other()->Point() - Object()->Position());

        if (a < b)
            a = b;
        if ( ( a > 0 && a < nextTurn ) || !left.front.edge)
            nextTurn = a;
    }

    nextTurn/= Object()->Speed() * .98f;

    REAL delay = Delay() * 1.5f;
    if ((!Object()->CanMakeTurn(1) || !Object()->CanMakeTurn(-1) || success) && nextTurn > delay)
        nextTurn = delay;

    if (nextTurn > .3f)
        nextTurn = .3f;

    if (nextStateChange > se_GameTime())
        return;

    // find a new victim:
    eCoord enemypos=eCoord(1000,100);

    const tList<eGameObject>& gameObjects = Object()->Grid()->GameObjects();
    gCycle *secondbest = NULL;

    // find the closest enemy
    for (int i=gameObjects.Len()-1;i>=0;i--){
        gCycle *other=dynamic_cast<gCycle *>(gameObjects(i));

        if (other && other->Team()!=Object()->Team() &&
                !IsTrapped(other, Object())){
            // then, enemy is realy an enemy
            eCoord otherpos=other->Position()-Object()->Position();
            if (otherpos.NormSquared()<enemypos.NormSquared()){
                // check if the path is clear
                gSensor p(Object(),Object()->Position(),otherpos);
                p.detect(REAL(.98));
                secondbest = dynamic_cast<gCycle *>(other);

                if (!target)
                    enemypos = otherpos;

                if (p.hit>=.98){
                    enemypos = otherpos;
                    target = secondbest;
                }
            }
        }
    }

    eCoord relpos=enemypos.Turn(Object()->Direction().Conj()).Turn(0,1);


    if (!target)
        target = secondbest;
    else
        SwitchToState(CLOSECOMBAT, 1);

    if (target)
        SetTraceSide((relpos.x  > 0 ? 10 : -10) *
                     (target->Speed() > Object()->Speed() ? -1 : 1));

    nextStateChange = se_GameTime() + 10;

    //*  SwitchToState(AI_SURVIVE, 1);
    return;
}


void gSmarterBot::ThinkPath( ThinkData & data )
{
    int lr = 0;
    REAL mindist = 10;

    eCoord dir = Object()->Direction();
    // REAL fs=front.distance;
    REAL ls=data.left.distance;
    REAL rs=data.right.distance;

    if (data.front.distance < 1) {
        SwitchToState(SURVIVE);
    }

    if (!target->CurrentFace() || IsTrapped(target, Object()))
    {
        SwitchToState(SURVIVE, 1);
        EmergencySurvive( data );

        data.thinkAgain = 4;
        return;
    }

    eCoord tDir = target->Position() - Object()->Position();

    if ( nextStateChange < se_GameTime() )
    {
        gSensor p(Object(),Object()->Position(), tDir);
        p.detect(REAL(.9999999));
        if (p.hit >=  .9999999)  // free line of sight to victim. Switch to close combat.
        {
            SwitchToState(CLOSECOMBAT, 5);
            EmergencySurvive( data );

            return;
        }
    }

    // find a new path if the one we got is outdated:
    if (lastPath < se_GameTime() - 10)
        if (target->CurrentFace())
        {

            Object()->FindCurrentFace();
            REAL before = tRealSysTimeFloat();
            eHalfEdge::FindPath(Object()->Position(), Object()->CurrentFace(),
                                target->Position(), target->CurrentFace(),
                                Object(),
                                path);

            // calculate (and archive) time used for pathfinding
            REAL used = tRealSysTimeFloat() - before;
            static char const * section = "PATH_TIME";
            static REAL usedAverage = 0;
            const REAL decay = .1;
            usedAverage = (usedAverage+used*decay)/(1+decay);

            // disable pathfinding if it just takes too long.
            if ( used > .06 || usedAverage > .03 )
            {
#ifdef DEBUG
                con << "Path finding is too expensive for this PC. Disabling it.\n";
#endif
                return;
            }
            lastPath = se_GameTime();
        }

    if (!path.Valid())
    {
        data.thinkAgain = 1;
        return;
    }

    // find the most advanced path point that is in our viewing range:

    for (int z = 10; z>=0; z--)
        path.Proceed();

    bool goon   = path.Proceed();
    bool nogood = false;

    do
    {
        if (goon)
            goon = path.GoBack();
        else
            goon = true;

        eCoord pos   = path.CurrentPosition() + path.CurrentOffset() * 0.1f;
        eCoord opos  = Object()->Position();
        eCoord odir  = pos - opos;

        eCoord intermediate = opos + dir * eCoord::F(odir, dir);

        gSensor p(Object(), opos, intermediate - opos);
        p.detect(1.1f);
        nogood = (p.hit <= .999999999 || eCoord::F(path.CurrentOffset(), odir) < 0);

        if (!nogood)
        {
            gSensor p(Object(), intermediate, pos - intermediate);
            p.detect(1);
            nogood = (p.hit <= .99999999 || eCoord::F(path.CurrentOffset(), odir) < 0);
        }

    }
    while (goon && nogood);

    if (goon)
    {
        // now we have found our next goal. Try to get there.
        eCoord pos    = Object()->Position();
        eCoord target = path.CurrentPosition();

        // look how far ahead the target is:
        REAL ahead = eCoord::F(target - pos, dir)
                     + eCoord::F(path.CurrentOffset(), dir);

        if ( ahead > 0)
        {	  // it is still before us. just wait a while.
            mindist = ahead;
        }
        else
        { // we have passed it. Make a turn towards it.
            REAL side = (target - pos) * dir;

            if ( !((side > 0 && ls < 3) || (side < 0 && rs < 3))
                    && (fabs(side) > 3 || ahead < -10) )
            {
            // Chat(tString("Let's go this way"));
                lr += (side > 0 ? 1 : -1);
            }
        }
    }
    else // nogood
    {
        lastPath -= 1;
        SwitchToState(SURVIVE);
    }

    EmergencySurvive( data, 1, -lr );

    REAL d = sqrt(tDir.NormSquared()) * .2f;
    if (d < mindist)
        mindist = d;

    data.thinkAgain = mindist / Object()->Speed();
    if (data.thinkAgain > .4)
        data.thinkAgain *= .7;
}

gSmarterBot &gSmarterBot::Get(gCycle * cycle)
{
    tASSERT(cycle);

    // create
    if (cycle->smarterBot_.get() == 0)
        cycle->smarterBot_.reset(new gSmarterBot(cycle));

    return *cycle->smarterBot_;
}


const tString gAIChat::killedMessages[NUM_KILLED_MESSAGES] = {
    tString("Gosh Darnit!"),
    tString("Darn it!"),
    tString("Aw man!")
};

const tString gAIChat::trappedMessages[NUM_TRAPPED_MESSAGES] = {
    tString("Hehe! Got you!"),
    tString("You're trapped!")
};

tString gAIChat::lastMessage = tString("");
REAL gAIChat::lastMessageTime = 0;

int gAIChat::lastKilledMessage = -1;
int gAIChat::lastTrappedMessage = -1;

void gAIChat::Chat(tString message, ePlayerNetID * player_)
{
    if (message != lastMessage && lastMessageTime < se_GameTime() + 5) {
        player_->Chat(message);
        lastMessage = message;
    }
}

void gAIChat::Chat(gCHAT_ACTION type, ePlayerNetID * player_)
{
    int numMessages = 0;
    const tString* messages = NULL;
    int lastMessage = 0;

    switch (type)
    {
        case KILLED:
            numMessages = NUM_KILLED_MESSAGES;
            messages = killedMessages;
            lastMessage = lastKilledMessage;
            break;

        case TRAPPED:
            numMessages = NUM_TRAPPED_MESSAGES;
            messages = trappedMessages;
            lastMessage = lastTrappedMessage;
            break;
    }

    int nextMessage = lastMessage;
    while (nextMessage == lastMessage)
    {
        nextMessage = rand() % numMessages;
    }

    lastMessage = nextMessage;
    player_->Chat(messages[nextMessage]);
}
