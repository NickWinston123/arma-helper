
// #include "../../gAINavigator.h"
// #include "gSmarterBot.h"

// REAL sg_smarterBotRange = 32;
// REAL sg_smarterBotNewWallBlindness = 2;
// REAL sg_smarterBotMinTimestep = .1f;
// REAL sg_smarterBotDelay = 2;
// REAL sg_smarterBotDecay = .05f;

// gSmarterBot::gSmarterBot(gCycle* owner)
//     : gAINavigator(owner)
//     , nextChatAI_(0)
//     , timeOnChatAI_(0)
// {
//     settings_.range = sg_smarterBotRange;
//     settings_.newWallBlindness = sg_smarterBotNewWallBlindness;
// }

// gSmarterBot& gSmarterBot::Get(gCycle* cycle)
// {
//     tASSERT(cycle);

//     // create
//     if (cycle->smarterBot_.get() == 0)
//         cycle->smarterBot_.reset(new gSmarterBot(cycle));

//     return *cycle->smarterBot_;
// }

// REAL gSmarterBot::Think(REAL minStep)
// {
//     UpdatePaths();
//     EvaluationManager manager(GetPaths());
//     manager.Evaluate(SuicideEvaluator(*Owner(), sg_smarterBotMinTimestep * 1.1), 1);
//     manager.Evaluate(SuicideEvaluator(*Owner(), minStep), 1);
//     manager.Reset();
//     manager.Evaluate(SpaceEvaluator(*Owner()), 1);
//     manager.Evaluate(PlanEvaluator(), .1);
//     CycleControllerAction controller;
//     return manager.Finish(controller, *Owner(), minStep);
// }

// void gSmarterBot::Activate(REAL currentTime)
//     {
//         // is it already time for activation?
//         if ( currentTime < nextChatAI_ )
//             return;

//         REAL minstep   = sg_smarterBotMinTimestep; // minimum timestep between thoughts in seconds
//         REAL maxstep   = sg_smarterBotMinTimestep * ( 1 + .5 * tReproducibleRandomizer::GetInstance().Get() );  // maximum timestep between thoughts in seconds

//         // chat AI wasn't active yet, so don't start immediately
//         if ( nextChatAI_ <= EPS )
//         {
//             nextChatAI_ = sg_smarterBotDelay + currentTime;
//             return;
//         }

//         timeOnChatAI_ += currentTime - nextChatAI_;

//         // make chat AI worse over time
//         if ( sn_GetNetState() != nSTANDALONE )
//         {
//             REAL qualityFactor = ( timeOnChatAI_ * sg_smarterBotDecay );
//             if ( qualityFactor > 1 )
//             {
//                 minstep *= qualityFactor;
//                 // maxstep *= qualityFactor;
//             }
//         }

//         REAL minTime = Think( maxstep*2 );

//         if( minTime < 0 )
//         {
//             // try right again
//             nextChatAI_ = currentTime;
//         }
//         else
//         {
//             if ( minTime < minstep )
//                 minTime = minstep;
//             if ( minTime > maxstep + minstep * 1.5 )
//             {
//                 minTime = maxstep;
//             }
            
//             nextChatAI_ = currentTime + minTime;
//             timeOnChatAI_ += minTime;
//         }
//     }