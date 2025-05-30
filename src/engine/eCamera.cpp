/*

*************************************************************************

ArmageTron -- Just another Tron Lightcycle Game in 3D.
Copyright (C) 2000  Manuel Moos (manuel@moosnet.de)

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Geeneral Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

***************************************************************************

*/

#include "rSDL.h"

#include "eSensor.h"
#include "eCamera.h"
#include "eSound.h"
#include "rScreen.h"
#include "eGameObject.h"
#include "uInputQueue.h"
//#include "eTess.h"
#include "eTimer.h"
#include "tConfiguration.h"
#include "rSysdep.h"
#include "tConsole.h"
#include "ePlayer.h"
#include "eAdvWall.h"
#include "nConfig.h"
#include "eFloor.h"
#include "eGrid.h"
#include "eDebugLine.h"
#include "tMath.h"
#include "eNetGameObject.h"
#include "nObserver.h"

// camera visibility settings
static REAL se_hitCacheSpeed = 1; // the speed the hit cache for external visibility targets recovers from hits

static REAL se_visibilityWallDistance = .5f; // the distance the visibility targets keep from walls

static REAL se_visibilitySpeed = 40; // speed with wich the visibility targets is brought into view
static REAL se_visibilityExtension = 1; // distance (measured in seconds, gets multiplied by speed) of the visibility targets from the watched object
static REAL se_visibilitySidewaysSkew = .5; // extra forward component of the sideways visibility targets
static bool se_visibilityLowerWall = true; // flag indicating whether walls should be lowerd when they block the view
static bool se_visibilityLowerWallSmart = false; // same specially for the smart camera

static tConfItem<REAL> se_viscs("CAMERA_VISIBILITY_RECOVERY_SPEED", se_hitCacheSpeed );
static tConfItem<REAL> se_viswd("CAMERA_VISIBILITY_WALL_DISTANCE", se_visibilityWallDistance );
static tConfItem<REAL> se_viss("CAMERA_VISIBILITY_CLIP_SPEED", se_visibilitySpeed );
static tConfItem<REAL> se_vise("CAMERA_VISIBILITY_EXTENSION", se_visibilityExtension );
static tConfItem<REAL> se_vissk("CAMERA_VISIBILITY_SIDESKEW", se_visibilitySidewaysSkew );
static tConfItem<bool> se_vislw("CAMERA_VISIBILITY_LOWER_WALL", se_visibilityLowerWall );
static tConfItem<bool> se_vislws("CAMERA_VISIBILITY_LOWER_WALL_SMART", se_visibilityLowerWallSmart );

//static bool se_customGlance = true; // use the custom camera settings for glancing in the smart camera
//static tSettingItem<bool> se_cg("CAMERA_SMART_GLANCE_CUSTOM", se_customGlance );

// allow cameras [player independent; gets transferred over the network]
static bool forbid_camera[CAMERA_COUNT];

class eInitForbidCamera
{
public:
    eInitForbidCamera()
    {
        // allow all cameras
        for ( int i = CAMERA_COUNT-1; i>=0; --i )
        {
            forbid_camera[i] = false;
        }

        // except the server custom camera
        forbid_camera[ CAMERA_SERVER_CUSTOM ] = true;
    }
};
static eInitForbidCamera se_initForbid;

// forbid smart camerea
static nSettingItem<bool> a_s
("CAMERA_FORBID_SMART",
 forbid_camera[CAMERA_SMART]);

// forbid internal camerea
static nSettingItem<bool> a_i
("CAMERA_FORBID_IN",
 forbid_camera[CAMERA_IN]);

// forbid mer camera
static bool stc_forbidMerCam = false;
static nSettingItem<bool> cafmer("CAMERA_FORBID_MER", stc_forbidMerCam);

// forbid custom camerea
static nSettingItem<bool> a_c
("CAMERA_FORBID_CUSTOM",
 forbid_camera[CAMERA_CUSTOM]);

// forbid custom camerea
static nSettingItem<bool> a_sc
("CAMERA_FORBID_SERVER_CUSTOM",
 forbid_camera[CAMERA_SERVER_CUSTOM]);

// forbid free camerea
static nSettingItem<bool> a_f
("CAMERA_FORBID_FREE",
 forbid_camera[CAMERA_FREE]);

// forbid fixed ext. camerea
static nSettingItem<bool> a_fe
("CAMERA_FORBID_FOLLOW",
 forbid_camera[CAMERA_FOLLOW]);



// override custom glance settings with server glance settings
static bool se_overrideCustom=false;
static nSettingItem<bool> a_ocg
("CAMERA_OVERRIDE_CUSTOM_GLANCE",
 se_overrideCustom);

// override custom glance settings with server glance settings when using the server custom camera
static bool se_overrideCustomServerCustom=false;
static nSettingItem<bool> a_ocgs
("CAMERA_OVERRIDE_CUSTOM_GLANCE_SERVER_CUSTOM",
 se_overrideCustomServerCustom);

// forbid custom glancing alltogether
static bool se_forbidCustomGlance=false;
static nSettingItem<bool> a_fcg
("CAMERA_FORBID_CUSTOM_GLANCE",
 se_forbidCustomGlance);


//... hack for handling the hud map of 0.4 clients
bool stc_forbidHudMap = false;
static nSettingItem<bool> fcs("FORBID_HUD_MAP", stc_forbidHudMap);

#ifndef DEDICATED
#include "rGL.h"
#endif

static REAL lastTime=0;
static const REAL rimDistance = 0.01f;
static const REAL rimDistanceHeight = 0.1f;

REAL se_cameraRise=0;
REAL se_cameraZ=10;

// List<eCamera> se_cameras;

uActionCamera eCamera::se_moveBack("MOVE_BACK",
                                   -10,
                                   uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_moveForward("MOVE_FORWARD",
                                      -20,
                                      uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_moveDown("MOVE_DOWN",
                                   -30,
                                   uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_moveUp("MOVE_UP",
                                 -40,
                                 uAction::uINPUT_ANALOG);


uActionCamera eCamera::se_moveRight("MOVE_RIGHT",
                                    -50,
                                    uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_moveLeft("MOVE_LEFT",
                                   -60,
                                   uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_zoomOut("ZOOM_OUT",
                                  -70,
                                  uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_zoomIn("ZOOM_IN",
                                 -80,
                                 uAction::uINPUT_ANALOG);


uActionCamera eCamera::se_glanceForward("GLANCE_FORWARD",-85);

uActionCamera eCamera::se_glanceBack("GLANCE_BACK",-90);

uActionCamera eCamera::se_glanceRight("GLANCE_RIGHT",-100);

uActionCamera eCamera::se_glanceLeft("GLANCE_LEFT",-110);

uActionCamera eCamera::se_lookDown("BANK_DOWN",
                                   -120,
                                   uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_lookUp("BANK_UP",
                                 -130,
                                 uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_lookRight("LOOK_RIGHT",
                                    -140,
                                    uAction::uINPUT_ANALOG);

uActionCamera eCamera::se_lookLeft("LOOK_LEFT",
                                   -150,
                                   uAction::uINPUT_ANALOG);


uActionCamera eCamera::se_switchView("SWITCH_VIEW", -160);

uActionTooltip eCamera::se_glanceBackTooltip( eCamera::se_glanceBack, 1 );
uActionTooltip eCamera::se_glanceForwardTooltip( eCamera::se_glanceForward, 1 );
uActionTooltip eCamera::se_glanceRightTooltip( eCamera::se_glanceRight, 1 );
uActionTooltip eCamera::se_glanceLeftTooltip( eCamera::se_glanceLeft, 1 );
uActionTooltip eCamera::se_switchViewTooltip( eCamera::se_switchView, 1 );


static REAL s_startFollowX = -30, s_startFollowY = -30, s_startFollowZ = 80;
static REAL s_startSmartX = 10, s_startSmartY = 30, s_startSmartZ = 2;
static REAL s_startFreeX =  10, s_startFreeY = -70, s_startFreeZ = 100;

static tConfItem<REAL> s_foX("CAMERA_FOLLOW_START_X", s_startFollowX);
static tConfItem<REAL> s_smX("CAMERA_SMART_START_X", s_startSmartX);
static tConfItem<REAL> s_frX("CAMERA_FREE_START_X", s_startFreeX);

static tConfItem<REAL> s_foY("CAMERA_FOLLOW_START_Y", s_startFollowY);
static tConfItem<REAL> s_smY("CAMERA_SMART_START_Y", s_startSmartY);
static tConfItem<REAL> s_frY("CAMERA_FREE_START_Y", s_startFreeY);

static tConfItem<REAL> s_foZ("CAMERA_FOLLOW_START_Z", s_startFollowZ);
static tConfItem<REAL> s_smZ("CAMERA_SMART_START_Z", s_startSmartZ);
static tConfItem<REAL> s_frZ("CAMERA_FREE_START_Z", s_startFreeZ);

// custom camera displacement
static REAL s_customBack = 6, s_customRise = 4, s_customBackSpeed = .5, s_customRiseSpeed = .4, s_customPitch = -.58, s_customZoom = 0, s_customTurnSpeed=4, s_customTurnSpeed180 = 4;
static REAL s_glanceBack = 6, s_glanceRise = 4, s_glanceBackSpeed = .5, s_glanceRiseSpeed = .4 , s_glancePitch = -.58;
static REAL s_serverCustomBack = 30, s_serverCustomRise = 20, s_serverCustomBackSpeed = 0, s_serverCustomRiseSpeed = 0, s_serverCustomPitch = -.7, s_serverCustomTurnSpeed=-1, s_serverCustomTurnSpeed180 = 2;
static REAL s_serverGlanceBack = 30, s_serverGlanceRise = 20, s_serverGlanceBackSpeed = 0, s_serverGlanceRiseSpeed = 0, s_serverGlancePitch = -.7;


static tConfItem<REAL> s_iBack("CAMERA_CUSTOM_BACK", s_customBack);
static tConfItem<REAL> s_iRise("CAMERA_CUSTOM_RISE", s_customRise);
static tConfItem<REAL> s_iBackSpeed("CAMERA_CUSTOM_BACK_FROMSPEED", s_customBackSpeed);
static tConfItem<REAL> s_iRiseSpeed("CAMERA_CUSTOM_RISE_FROMSPEED", s_customRiseSpeed);
static tConfItem<REAL> s_iPitch("CAMERA_CUSTOM_PITCH", s_customPitch);
static tConfItem<REAL> s_iZoom("CAMERA_CUSTOM_ZOOM", s_customZoom);
static tConfItem<REAL> s_iCustomTurnSpeed("CAMERA_CUSTOM_TURN_SPEED", s_customTurnSpeed);
static tConfItem<REAL> s_iCustomTurnSpeed180("CAMERA_CUSTOM_TURN_SPEED_180", s_customTurnSpeed180);

static tConfItem<REAL> s_iGBack("CAMERA_GLANCE_BACK", s_glanceBack);
static tConfItem<REAL> s_iGRise("CAMERA_GLANCE_RISE", s_glanceRise);
static tConfItem<REAL> s_iGBackSpeed("CAMERA_GLANCE_BACK_FROMSPEED", s_glanceBackSpeed);
static tConfItem<REAL> s_iGRiseSpeed("CAMERA_GLANCE_RISE_FROMSPEED", s_glanceRiseSpeed);
static tConfItem<REAL> s_iGPitch("CAMERA_GLANCE_PITCH", s_glancePitch);

static nSettingItem<REAL> s_iSBack("CAMERA_SERVER_CUSTOM_BACK", s_serverCustomBack);
static nSettingItem<REAL> s_iSRise("CAMERA_SERVER_CUSTOM_RISE", s_serverCustomRise);
static nSettingItem<REAL> s_iSBackSpeed("CAMERA_SERVER_CUSTOM_BACK_FROMSPEED", s_serverCustomBackSpeed);
static nSettingItem<REAL> s_iSRiseSpeed("CAMERA_SERVER_CUSTOM_RISE_FROMSPEED", s_serverCustomRiseSpeed);
static nSettingItem<REAL> s_iSPitch("CAMERA_SERVER_CUSTOM_PITCH", s_serverCustomPitch);
static nSettingItem<REAL> s_iSCustomTurnSpeed("CAMERA_SERVER_CUSTOM_TURN_SPEED", s_serverCustomTurnSpeed);
static nSettingItem<REAL> s_iSCustomTurnSpeed180("CAMERA_SERVER_CUSTOM_TURN_SPEED_180", s_serverCustomTurnSpeed180);

static nSettingItem<REAL> s_iSGBack("CAMERA_SERVER_GLANCE_BACK", s_serverGlanceBack);
static nSettingItem<REAL> s_iSGRise("CAMERA_SERVER_GLANCE_RISE", s_serverGlanceRise);
static nSettingItem<REAL> s_iSGBackSpeed("CAMERA_SERVER_GLANCE_BACK_FROMSPEED", s_serverGlanceBackSpeed);
static nSettingItem<REAL> s_iSGRiseSpeed("CAMERA_SERVER_GLANCE_RISE_FROMSPEED", s_serverGlanceRiseSpeed);
static nSettingItem<REAL> s_iSGPitch("CAMERA_SERVER_GLANCE_PITCH", s_serverGlancePitch);

// turn speed of internal camera
static REAL s_inTurnSpeed=40;
static tConfItem<REAL> s_iInTurnSpeed("CAMERA_IN_TURN_SPEED", s_inTurnSpeed);

bool eCamera::InterestingToWatch(eGameObject const *g){
    return g &&
           (g->Alive() ||
            (lastTime - g->deathTime<1));
}

// pointer holding the last player watched actively
static nObserverPtr< ePlayerNetID > se_watchedPlayer[ MAX_PLAYERS ];

// returns the player a camera last watched for modification
static nObserverPtr< ePlayerNetID > & se_GetWatchedPlayer( eCamera * cam )
{
    static nObserverPtr< ePlayerNetID > dummy;

    ePlayer const * localPlayer = cam->LocalPlayer();
    if ( !localPlayer )
        return dummy;

    return se_watchedPlayer[ localPlayer->ID() ];
}

// returns the last watched game object
static eGameObject * se_GetWatchedObject( eCamera * cam )
{
    ePlayerNetID const * player = se_GetWatchedPlayer( cam );
    if ( player )
        return player->Object();

    return NULL;
}

static void se_SetWatchedObject( eCamera * cam, eGameObject * obj )
{
    nObserverPtr< ePlayerNetID > & player = se_GetWatchedPlayer( cam );

    // switch the favorite player to watch if we switch away from him deliberately
    if ( !player || ( player->Object() && player->Object()->Alive() ) )
    {
        // determine the player that controls the object, a bit awkward...
        for ( int i = se_PlayerNetIDs.Len()-1; i>=0; --i )
        {
            ePlayerNetID * p = se_PlayerNetIDs(i);
            if ( p->Object() == obj )
                player = p;
        }
    }
}


void eCamera::MyInit(){
    if (localPlayer){
        if (cameraMain_) mode = localPlayer->startCamera; //PENDING:
        fov=localPlayer->FOV;
    }

    // find center: the object our player controls
    if (bool(netPlayer) && !center)
    {
        center = netPlayer->Object();
    }
    else if ( grid->gameObjectsInteresting.Len() > 0 )
    {
        // or an arbitrary game object
        center = grid->gameObjectsInteresting[0];
    }

    // switch away from forbidden camera mode
    if (forbid_camera[mode] && bool(netPlayer) && netPlayer->Object()==Center())
        SwitchView();

    centerPos=eCoord(100,100);
    centerSpeedSmooth=0;
    if ( Center() )
    {
        centerPos = Center()->PredictPosition();
        centerSpeedSmooth = Center()->Speed();
    }

    pos=CenterPos();
    dir=CenterDir();
    glanceDir_ = eCoord(1,0);
    centerPosSmooth=pos;
    centerDirLast=centerDirSmooth=dir;
    lastPos=pos;
    zNear=0.1f;
    //  foot=tNEW(eGameObject)(pos,dir,0);
    distance=0;
    lastrendertime=se_GameTime();

    if ( grid )
    {
        if ( CameraMain() )
            grid->cameras.Add(this,id);
        else
            grid->subcameras.Add(this,id);
    }

    //  se_ResetVisibles(id);
    smoothTurning=turning=0;
    centerPosLast=CenterPos();
    userCameraControl=0;
    centerIncam=1;
    smartcamSkewSmooth=0;
    smartcamIncamSmooth=1;
    smartcamFrontSmooth=0;

    for(int i = hitCacheSize-1; i>=0; --i)
        hitCache_[i] = 1;

    switch (mode){
    case CAMERA_CUSTOM:
    case CAMERA_SERVER_CUSTOM:
    case CAMERA_IN:
        z=10;
        rise=0;
        break;
    case CAMERA_FOLLOW:
        pos=pos+dir.Turn(eCoord(s_startFollowX,s_startFollowY)) ;
        z=s_startFollowZ;
        break;
    case CAMERA_SMART:
        pos=pos+dir.Turn(eCoord(s_startSmartX,s_startSmartY)) ;
        z=s_startSmartZ;
        break;
    case CAMERA_FREE:
        pos=pos+dir.Turn(eCoord(s_startFreeX,s_startFreeY)) ;
        z=s_startFreeZ;
        break;
    default:
        break;
    }

    if ( mode != CAMERA_IN && mode != CAMERA_CUSTOM && mode != CAMERA_SERVER_CUSTOM ){
        dir=CenterPos()-pos;
        REAL dist=REAL(sqrt(dir.NormSquared()));
        if (dist<.001) dist=1;
        dir=dir*(1/dist);
        rise=(CenterZ()-z)/dist;
    }

    glancingForward=glancingBack=glancingLeft=glancingRight=false;
    glanceSmooth=glanceSmoothAbs=0;

    lastSwitch=-100;
}

const ePlayerNetID* eCamera::Player() const { return netPlayer; }
const ePlayer* eCamera::LocalPlayer() const { return localPlayer; }

eCamera::eCamera(eGrid *g, rViewport *view,ePlayerNetID *p,
                 ePlayer *lp,eCamMode m,  bool rMain)
        :id(-1),grid(g),netPlayer(p),localPlayer(lp),
        // centerID(0),
        mode(m),pos(0,0),dir(1,0),top(0,0),
        vp(view), cameraMain_(rMain) 
    {
    /*
      if (p->pID>=0)
      localPlayer=playerConfig[p->pID];
    */
    MyInit();
}




eCamera::~eCamera(){
    eSoundLocker locker;
    //  int ID=id;
    //  tDESTROY(foot);
    //  se_cameras.Remove(this,id);
    //  se_ResetVisibles(se_cameras.Len());
    //  if (ID!=se_cameras.Len()) se_ResetVisibles(ID);

    if (cameraMain_)
    	grid->cameras.Remove(this, id);
    else
		grid->subcameras.Remove(this, id);

    tCHECK_DEST;
}


//static eGameObject *dummy=NULL;

eGameObject * eCamera::Center() const{
    return center;
}

void eCamera::SwitchView(){
    zNear = 0.01f;

    int count=CAMERA_COUNT * 2;

    userCameraControl = 0;

    //  eCamMode pre=mode;

    bool imp=true,global_imp=true,both_imp=true;
    for (int i=CAMERA_COUNT-1;i>=0;i--){
        if (!localPlayer || localPlayer->allowCam[i])
            imp=false;
        if (!forbid_camera[i] || !netPlayer || netPlayer->Object()!=Center())
            global_imp=false;
        if ((!forbid_camera[i] || !netPlayer || netPlayer->Object()!=Center())
                && (!localPlayer || localPlayer->allowCam[i]))
            both_imp=false;
    }

    if (imp) con << "impossible to meet your needs.\n";
    if (global_imp) con << "impossible to meet global needs.\n";
    if (both_imp) con << "impossible to meet both needs.\n";

    if (both_imp && !global_imp)
        imp=true;

    do{
        // rotate the mode
        int m = mode;
        m--;
        if ( m<0 )
            m = CAMERA_SERVER_CUSTOM;
        mode = static_cast< eCamMode >( m );

        count--;
    }
    while ((!imp && count > CAMERA_COUNT && localPlayer && !localPlayer->allowCam[mode])
            || (count >0 && !global_imp && forbid_camera[mode] &&
                (bool( netPlayer ) && netPlayer->Object()==Center())));

    if ( mode == CAMERA_IN || mode == CAMERA_CUSTOM || mode == CAMERA_SERVER_CUSTOM )
        rise=0;

    // custom camera with turn speed 0: align it with the cycle once
    if( ( mode == CAMERA_CUSTOM && s_customTurnSpeed <= 0 ) || ( mode == CAMERA_SERVER_CUSTOM && s_serverCustomTurnSpeed <= 0 ) )
    {
        dir = CenterDir();
    }

    if(mode==CAMERA_SMART){
        smartcamIncamSmooth=1;
        z=z+1;
        pos=pos+dir.Turn(-1,.1);
    }
}

bool eCamera::Act(uActionCamera *Act,REAL x){
    eCoord objdir=CenterCamDir();

    int turn=0;
    bool takeOverGlance = false;
    if (eGameObject::se_turnLeft==*reinterpret_cast<uActionPlayer *>(Act)){
        //takeOverGlance = glancingLeft || glancingBack;
        turn=-1;
    }
    if (eGameObject::se_turnRight==*reinterpret_cast<uActionPlayer *>(Act)){
        //takeOverGlance = glancingRight || glancingBack;
        turn=1;
    }

    if ( takeOverGlance && /* mode != CAMERA_FOLLOW && */ mode != CAMERA_FREE )
    {
        // copy over position and direction, but reset glancing.
        // this will keep the camera as it was before the turn.
        glancingForward=glancingRight=glancingBack=glancingLeft=false;
        dir = dir.Turn( glanceDir_ );
        pos = Glance( pos, glanceDir_ );
        glanceSmooth = 0;
    }

    if (turn){
        eGameObject *cent=NULL;
        if (se_GameTime() <= 0 )
            turning+=.5;
        if (netPlayer) cent=netPlayer->Object();
        if (!InterestingToWatch(cent) && x>0)
        {
            SwitchCenter(turn);
            se_SetWatchedObject( this, center );
        }
    }

    REAL ll=0,lu=0,ml=0,mf=0,mu=0,zi=1;

    if (se_lookLeft==*Act && x>0)
        ll=x;
    else if (se_lookRight==*Act && x>0)
        ll=-x;
    else if (se_lookUp==*Act && x>0)
        lu=x;
    else if (se_lookDown==*Act && x>0)
        lu=-x;
    else if (se_zoomIn==*Act && x>0)
        zi*=1+zi*.1;
    else if (se_zoomOut==*Act && x>0)
        zi/=1+zi*.1;

    else if (se_moveLeft==*Act && x>0)
        ml=x;
    else if (se_moveRight==*Act && x>0)
        ml=-x;
    else if (se_moveForward==*Act && x>0)
        mf=x;
    else if (se_moveBack==*Act && x>0)
        mf=-x;
    else if (se_moveUp==*Act && x>0)
        mu=x;
    else if (se_moveDown==*Act && x>0)
        mu=-x;
    else if (se_switchView==*Act && x>0)
        SwitchView();
    else if (se_glanceForward==*Act)
        glancingForward=(x>0);
    else if (se_glanceBack==*Act)
        glancingBack=(x>0);
    else if (se_glanceLeft==*Act)
        glancingLeft=(x>0);
    else if (se_glanceRight==*Act)
        glancingRight=(x>0);
    else
        return false;


    userCameraControl+=sqrt(ll*ll+lu*lu+(1-zi)*(1-zi)+ml*ml+mf*mf+mu*mu)/20;

    switch(mode){
    case CAMERA_IN:
    case CAMERA_SMART_IN:
        lu+=mu*2;
        ll+=ml;
        mu=ml=0;
        break;
    case CAMERA_CUSTOM:
    case CAMERA_SERVER_CUSTOM:
    case CAMERA_FREE:
        break;
    case CAMERA_FOLLOW:
    case CAMERA_SMART:
        mu-=lu;
        ml-=ll;
        lu=ll=0;
        break;
    case CAMERA_COUNT:
        break;
    }

    // normal actions with the given data
    dir=dir+dir.Turn(eCoord(0,ll*.2));
    rise+=lu/80;
    z+=mu*.25;
    pos=pos+dir*mf*.25+dir.Turn(eCoord(0,ml*.25));

    fov/=zi;
    //if (fov>120) fov=120;
    if (fov>160 && zi < 1) fov=160;

    //if (fov<30) fov=30;
    if (fov<30 && zi > 1) fov=30;


    switch(mode){
    case CAMERA_IN:
    case CAMERA_SMART_IN:
        {
            int x=3;
            while (eCoord::F(dir,objdir)<-.51 && x>0){
                dir=dir-objdir*(eCoord::F(dir,objdir)+.5);
                dir=dir*(1/sqrt(dir.NormSquared()));
                x--;
            }
        }
        break;
    case CAMERA_CUSTOM:
    case CAMERA_SERVER_CUSTOM:
    case CAMERA_FREE:
    case CAMERA_FOLLOW:
    case CAMERA_SMART:
        break;
    case CAMERA_COUNT:
        break;
    }

    Bound(0);

    return true;
}

extern REAL upper_height,lower_height;

static bool se_ClampCamera( eCamMode mode )
{
    return !(mode == CAMERA_SMART && se_GameTime() > -.5 ? se_visibilityLowerWallSmart : se_visibilityLowerWall);
}

//! Sensor class that moves the camera so the view is not blocked
class eCameraSensor: public eSensor
{
public:
    //! constructor
    eCameraSensor(eGameObject *o,eCoord & camera, const eCoord & target )
            :eSensor(o,o->Position(),camera-target), moved_(false),
            camPos_(camera), target_(target), zLimit_(2), ratio_(0), camera_(0), lowerWall_( true )
    {
#ifdef DEBUG_VISIBILITY_TARGETS
        eDebugLine::SetColor( 1,1,1 );
        eDebugLine::SetTimeout(.005);
        eDebugLine::Draw( target, 0, target, 2 );
#endif
        Move( target,0,0 );
        //clip_ = true;
    }

    //! helper struct to get the best (smallest) position correction
    struct Correction
    {
        eCoord correctTo;
        REAL   distance;

        //! try to improve the correction suggestion
        void Improve( eCoord correctTo, REAL distance )
        {
            if ( this->distance > distance )
            {
                this->distance = distance;
                this->correctTo = correctTo;
            }
        }
    };

    //! looks around the given wall (push the camera so that the ray
    //! from the camera to the object passes beside the wall )
    //! @param w          the wall
    //! @param direction  whether to look "left" or "right" around the wall (relative to wall)
    //! @param correction the correction suggestion to store the result in
    //! @param heightLimit maximal height a wall may have before it counts as an obstacle
    //! @param recursion   maximum number of corners to follow the wall around
    //! @param hardRecursion   maximum number of walls to follow
    void LookAround( eWall const * w, int direction, Correction & correction, REAL heightLimit, int recursion, int hardRecursion )
    {
        // abort if recursion is too deep aleready
        if ( recursion < 0 || hardRecursion < 0 )
            return;

        // tell the wall that it is blocking the sight (if requested)
        if ( lowerWall_ )
        {
            w->BlocksCamera( camera_, heightLimit );
        }

        // NULL pointer checks
        if ( !w || !w->Edge() || !w->Edge()->Other() )
            return;

        // project camera position a bit to the other side of the wall:
        // get the two endpoints
        eHalfEdge const * edge = (direction == 0 ? w->Edge() : w->Edge()->Other());
        eCoord const & p1 = *edge->Point();
        eCoord const & p2 = *edge->Other()->Point();

        // it is ony valid if the order of points on an imaginary line is
        // camPos_ -> p1 -> targtet_
        // (otherwise, te wall is not blocking sight at all)
        if ( !lowerWall_ && eCoord::F( camPos_ - p1, target_ - p1 ) > 0 )
            return;

        // determine normal of target - p1 line
        eCoord diff = target_ - p1;
        diff.Normalize();
        eCoord normal = diff.Turn(0,-1);

        // determine which side the other endpoint of the line lies on
        REAL sideOther=eCoord::F(normal,p2-p1);

        // find other walls that end in p1
        {
            bool wallContinues = false;
            eHalfEdge const * run = edge;
            do
            {
                run = run->Other();
                if (run)
                    run = run->Next();

                if (!run || !run->Other() || run == edge)
                    break;

                if ( run->GetWall() || run->Other()->GetWall() )
                {
                    // got one! determine its other entpoint
                    eCoord const & p3 = *run->Other()->Point();

                    // determine which side of the ray the other endpoint of the wall lies on
                    REAL side3=eCoord::F(normal,p3-p1);

                    // if it lies on the same side as p1, the edgepoint is an outline point
                    // and needs to be projected. If not, we need to recurse.
                    if ( side3 * sideOther < 0 )
                    {
                        wallContinues = true;

                        // only on corners the recursion level should be decreased
                        int recursion2 = recursion;
                        if ( fabs( (p2 - p1)*(p1 - p3) ) >= 10 * EPS * sqrt( (p1-p2).NormSquared() * (p1-p3).NormSquared() ) )
                            recursion2--;

                        // recurse
                        if ( run->GetWall() )
                            LookAround( run->GetWall(), 1, correction, heightLimit, recursion2, hardRecursion-1 );
                        else if ( run->Other()->GetWall() )
                            LookAround( run->Other()->GetWall(), 0, correction, heightLimit, recursion2, hardRecursion-1 );
                    }
                }
            }
            while ( true );

            // if recursion took place, don't project on this wall.
            if ( wallContinues )
                return;
        }

        // No other wall found: project camera around this wall.
        REAL side=eCoord::F(normal,camPos_-p1) * (1-ratio_);
        correction.Improve( camPos_ - normal * side, fabs( side ) );
    }

    //! called when the sensor passes another wall
    //! @param w     the wall that is passed
    //! @param time  usually the game time of the event, here time simply is the fraction of the
    //!              distance from the object to the camera covered so far
    //! @param alpha relative coordinate of the collision point on the wall
    virtual void PassEdge(const eWall *w,REAL time,REAL alpha,int)
    {
        // determine the height limit (max. height at which walls will not be considered blockers)
        REAL objectZ = 1.5;
        if ( camera_ )
            objectZ = camera_->CenterCamZ() * 2;
        REAL heightLimit = ( .5 * zLimit_ * time + objectZ * ( 1 - time ) );

        // exit early if the wall does not obstruct view
        if ( moved_ || !w || !owned->EdgeIsDangerous(w, time, alpha) || w->Height() <= heightLimit )
            return;

        heightLimit *= .5f;

        // project camera position a bit to the other side of the wall:
        // get the two endpoints
        eCoord const & p1 = w->EndPoint(0);
        eCoord const & p2 = w->EndPoint(1);

        // calculate wall normal
        eCoord diff=p2-p1;
        diff=diff*(1/w->Len());
        eCoord normal=diff.Turn(0,-1);

        // project
        REAL side=eCoord::F(normal,camPos_-p1);

        // initialize correction suggestion; be very reluctant to project (it's confusing)
        Correction correction;
        correction.distance = fabs( side ) * 10;
        correction.correctTo = camPos_ - normal*(side);

        // try to look around the edge to the right and left instead
        int recursion = lowerWall_? 0 : 2;
        LookAround( w, 0, correction, heightLimit, recursion, 1000 );
        LookAround( w, 1, correction, heightLimit, recursion, 1000 );

        // execute the correction
        if ( !lowerWall_ )
        {
            moved_ = correction.distance > .001f;
            camPos_ = correction.correctTo;
        }
    }
    bool moved_; //!< flag indicating that the camera position was moved

    inline eCameraSensor & SetZLimit( REAL const & zLimit );	   //!< Sets height limit of walls to consider blockers
    inline REAL const & GetZLimit( void ) const;	               //!< Gets height limit of walls to consider blockers
    inline eCameraSensor const & GetZLimit( REAL & zLimit ) const; //!< Gets height limit of walls to consider blockers
    inline eCameraSensor & SetRatio( REAL const & ratio );	       //!< Sets mixing ratio of clipped position and old position
    inline REAL const & GetRatio( void ) const;	                   //!< Gets mixing ratio of clipped position and old position
    inline eCameraSensor const & GetRatio( REAL & ratio ) const;   //!< Gets mixing ratio of clipped position and old position
    inline eCameraSensor & SetCamera( eCamera * camera );	             //!< Sets the camera
    inline eCamera * GetCamera( void ) const;	                         //!< Gets the camera
    inline eCameraSensor const & GetCamera( eCamera * & camera ) const;	 //!< Gets the camera
    inline eCameraSensor & SetLowerWall( bool const & lowerWall );	     //!< Sets flag to lower blocking walls instead of moving the camera
    inline bool const & GetLowerWall( void ) const;	                     //!< Gets flag to lower blocking walls instead of moving the camera
    inline eCameraSensor const & GetLowerWall( bool & lowerWall ) const; //!< Gets flag to lower blocking walls instead of moving the camera
private:
    eCoord & camPos_; //!< the position of the camera
    eCoord target_; //!< the position of the target
    REAL zLimit_;   //!< height limit of walls to consider blockers
    REAL ratio_;    //!< mixing ratio of clipped position and old position (0: perfect clipping, ->1: smooth clipping)
    eCamera * camera_; //!< the camera
    bool lowerWall_; //!< flag to lower blocking walls instead of moving the camera
};

// *******************************************************************************************
// *
// *   GetZLimit
// *
// *******************************************************************************************
//!
//!        @return     height limit of walls to consider blockers
//!
// *******************************************************************************************

REAL const & eCameraSensor::GetZLimit( void ) const
{
    return this->zLimit_;
}

// *******************************************************************************************
// *
// *   GetZLimit
// *
// *******************************************************************************************
//!
//!        @param  zLimit  height limit of walls to consider blockers to fill
//!      @return     A reference to this to allow chaining
//!
// *******************************************************************************************

eCameraSensor const & eCameraSensor::GetZLimit( REAL & zLimit ) const
{
    zLimit = this->zLimit_;
    return *this;
}

// *******************************************************************************************
// *
// *   SetZLimit
// *
// *******************************************************************************************
//!
//!        @param  zLimit  height limit of walls to consider blockers to set
//!       @return     A reference to this to allow chaining
//!
// *******************************************************************************************

eCameraSensor & eCameraSensor::SetZLimit( REAL const & zLimit )
{
    this->zLimit_ = zLimit;
    return *this;
}

// *******************************************************************************************
// *
// *   GetRatio
// *
// *******************************************************************************************
//!
//!        @return     mixing ratio of clipped position and old position
//!
// *******************************************************************************************

REAL const & eCameraSensor::GetRatio( void ) const
{
    return this->ratio_;
}

// *******************************************************************************************
// *
// *   GetRatio
// *
// *******************************************************************************************
//!
//!        @param  ratio   mixing ratio of clipped position and old position to fill
//!       @return     A reference to this to allow chaining
//!
// *******************************************************************************************

eCameraSensor const & eCameraSensor::GetRatio( REAL & ratio ) const
{
    ratio = this->ratio_;
    return *this;
}

// *******************************************************************************************
// *
// *   SetRatio
// *
// *******************************************************************************************
//!
//!        @param  ratio   mixing ratio of clipped position and old position to set
//!        @return     A reference to this to allow chaining
//!
// *******************************************************************************************

eCameraSensor & eCameraSensor::SetRatio( REAL const & ratio )
{
    this->ratio_ = ratio;
    return *this;
}

// *******************************************************************************************
// *
// *   GetCamera
// *
// *******************************************************************************************
//!
//!        @return     the camera
//!
// *******************************************************************************************

eCamera * eCameraSensor::GetCamera( void ) const
{
    return this->camera_;
}

// *******************************************************************************************
// *
// *   GetCamera
// *
// *******************************************************************************************
//!
//!        @param  camera  the camera to fill
//!      @return     A reference to this to allow chaining
//!
// *******************************************************************************************

eCameraSensor const & eCameraSensor::GetCamera( eCamera * & camera ) const
{
    camera = this->camera_;
    return *this;
}

// *******************************************************************************************
// *
// *   SetCamera
// *
// *******************************************************************************************
//!
//!        @param  camera  the camera to set
//!       @return     A reference to this to allow chaining
//!
// *******************************************************************************************

eCameraSensor & eCameraSensor::SetCamera( eCamera * camera )
{
    this->camera_ = camera;
    return *this;
}

// *******************************************************************************************
// *
// *   GetLowerWall
// *
// *******************************************************************************************
//!
//!        @return     flag to lower blocking walls instead of moving the camera
//!
// *******************************************************************************************

bool const & eCameraSensor::GetLowerWall( void ) const
{
    return this->lowerWall_;
}

// *******************************************************************************************
// *
// *   GetLowerWall
// *
// *******************************************************************************************
//!
//!        @param  lowerWall   flag to lower blocking walls instead of moving the camera to fill
//!       @return     A reference to this to allow chaining
//!
// *******************************************************************************************

eCameraSensor const & eCameraSensor::GetLowerWall( bool & lowerWall ) const
{
    lowerWall = this->lowerWall_;
    return *this;
}

// *******************************************************************************************
// *
// *   SetLowerWall
// *
// *******************************************************************************************
//!
//!        @param  lowerWall   flag to lower blocking walls instead of moving the camera to set
//!        @return     A reference to this to allow chaining
//!
// *******************************************************************************************

eCameraSensor & eCameraSensor::SetLowerWall( bool const & lowerWall )
{
    this->lowerWall_ = lowerWall;
    return *this;
}

// *******************************************************************************************
// *
// *   Glance
// *
// *******************************************************************************************
//!
//!        @param  in
//!        @param  glanceDir
//!        @return
//!
// *******************************************************************************************

eCoord eCamera::Glance( eCoord const & in, eCoord const & glanceDir ) const
{
    eCoord pos_diff=in-CenterPos();
    if (mode != CAMERA_FREE)
    {
        pos_diff = pos_diff.Turn(glanceDir);
    }
    return pos_diff + CenterPos();
}

// *******************************************************************************************
// *
// *   Bound
// *
// *******************************************************************************************
//!
//!        @param  dt  the timestep taken
//!
// *******************************************************************************************

void eCamera::Bound( REAL dt )
{
    eCoord glancePos = Glance( pos, glanceDir_ );
    Bound( dt, glancePos );
    pos = Glance( glancePos, glanceDir_.Conj() );
}

// *******************************************************************************************
// *
// *   Bound
// *
// *******************************************************************************************
//!
//! make sure CenterPos() + dirFromTarget is visible from pos
//!        @param  ratio   mixing ratio of old and best position: 0 means to take the best position, 1 to leave the old.
//!        @param  pos     the camera positon to clamp for visibility
//!        @param  dirFromTarget   the point CenterPos() + dirFromTarget is supposed to be visible
//!        @param  hitCache    if the caller persists this number, the real visibility target will not
//!                            "snap" when it suddenly gets more room
//!        @return         true if the camera position was moved
//!
// *******************************************************************************************

bool eCamera::Bound( REAL ratio, eCoord & pos, eCoord const & dirFromTarget, REAL & hitCache )
{
    // the target position that should be visible
    eCoord target = CenterPos();

    // move it as requested, but not into walls
    if ( dirFromTarget.NormSquared() > 0.0001f )
    {
        eSensor test( Center(), target, dirFromTarget );
        test.detect( hitCache );
        target = test.before_hit * (1-se_visibilityWallDistance) + target * se_visibilityWallDistance;
        hitCache = test.hit;
    }

    // prepare camera clamping sensor
    eCameraSensor toObject( Center(), pos, target );
    toObject.SetZLimit( z ).SetRatio( ratio ).SetCamera( this );

    // if not glancing, switch from wall lowering to camera clipping if the user desires
    if ( !glancingBack && fabs( glanceSmooth ) < .001 )
    {
        toObject.SetLowerWall( !se_ClampCamera(mode) );

        // clamp at outer boundary
        if ( !toObject.GetLowerWall() )
        {
            REAL offset = rimDistance + rimDistanceHeight * z;
            eWallRim::Bound(pos,offset);
        }
    }

    // execute clamping
    toObject.detect( 1 );

    return toObject.moved_;
}

// *******************************************************************************************
// *
// *   Bound
// *
// *******************************************************************************************
//!
//!        @param  dt   the timestep taken
//!        @param  pos  the camera position to clamp for visibility
//!
// *******************************************************************************************

void eCamera::Bound( REAL dt, eCoord & pos )
{
    // make sure the camera is above the floor and inside the rim eWalls
    if (z<.1)
        z=.1;

    // don't waste time on the internal cameras, they don't need clamping
    if(mode!=CAMERA_IN && mode !=CAMERA_SMART_IN )
    {
        // the following is only meaningful if there is an active camera object
        if ( Center() )
        {
            // gently bring points in front of the cycle into view
            REAL smoothBound = 1/(1+se_visibilitySpeed*dt);
            eCoord direction = centerDirSmooth;
            //eCoord direction = Center()->Direction();
            //eCoord direction = CenterCamDir() + centerDirSmooth;
            direction.Normalize();
            // z-man: I can't yet decide which of the three direction calculations is best.

            Bound( smoothBound, pos, direction * ( CenterSpeed() * se_visibilityExtension ), hitCache_[0] );

            //            Bound( smoothBound, pos, direction.Turn(se_visibilitySidewaysSkew, 1) * ( CenterSpeed() * se_visibilityExtension ), hitCache_[1] );
            //            Bound( smoothBound, pos, direction.Turn(se_visibilitySidewaysSkew,-1) * ( CenterSpeed() * se_visibilityExtension ), hitCache_[2] );

            // force the object itself to be in full view, try several times
            int timeout = 4;
            bool goon = true;
            while (goon && timeout > 0)
            {
                --timeout;
                goon = false;

                REAL cache = 1; // no real cache needed here
                if ( Bound( 0, pos, eCoord(0,0), cache ) )
                {
                    goon = true;
                    break;
                }
            }
        }
    }
    /*
      if ((sr_upperSky) && z>upper_height-3)
      z=upper_height-3.0001;

      if (
    	(se_BlackSky() || (sr_lowerSky && !sr_upperSky))&&
    	z>lower_height-3)
    	z=lower_height-3.0001;
    */
}

bool eCamera::CenterIncamOnTurn(){
    if (localPlayer)
        return localPlayer->centerIncamOnTurn;
    else
        return false;
}
bool eCamera::WhobbleIncam(){
    if (localPlayer)
        return localPlayer->wobbleIncam;
    else
        return false;
}
bool eCamera::AutoSwitchIncam(){
    if (localPlayer)
        return localPlayer->autoSwitchIncam;
    else
        return false;
}

static inline void makefinite(REAL &x,REAL y=2){if (!std::isfinite(x)) x=y;}
static inline void makefinite(eCoord &x){makefinite(x.x);makefinite(x.y);}

// Smart camera settings

// distance scale for tests measured relative to cycle speed
static REAL se_cameraSmartDistanceScale = .2;
static tConfItem< REAL > se_confCameraSmartDistanceScale( "CAMERA_SMART_DISTANCESCALE", se_cameraSmartDistanceScale );

// minimal distance scale of tests in meters
static REAL se_cameraSmartMinDistanceScale = 5.0;
static tConfItem< REAL > se_confCameraSmartMinDistanceScale( "CAMERA_SMART_MIN_DISTANCESCALE", se_cameraSmartMinDistanceScale );

// minimal distance of the camera to the cycle in meters
static REAL se_cameraSmartMinDistance = 10.0;
static tConfItem< REAL > se_confCameraSmartMinDistance( "CAMERA_SMART_MIN_DISTANCE", se_cameraSmartMinDistance );

// typical cycle speed
static REAL se_cameraSmartCycleSpeed = 20.0;
static tConfItem< REAL > se_confCameraSmartCycleSpeed( "CAMERA_SMART_CYCLESPEED", se_cameraSmartCycleSpeed );

// typical height in speed units
static REAL se_cameraSmartHeight = 2.0;
static tConfItem< REAL > se_confCameraSmartHeight( "CAMERA_SMART_HEIGHT", se_cameraSmartHeight );
// typical height in speed units
static REAL se_cameraSmartDistance = 4.0;
static tConfItem< REAL > se_confCameraSmartDistance( "CAMERA_SMART_DISTANCE", se_cameraSmartDistance );
// extra factor for height
static REAL se_cameraSmartHeightExtra = .5f;
static tConfItem< REAL > se_confCameraSmartHeightExtra( "CAMERA_SMART_HEIGHT_EXTRA", se_cameraSmartHeightExtra );

// influence of turning
static REAL se_cameraSmartHeightTurning = .5;
static tConfItem< REAL > se_confCameraSmartHeightTurning( "CAMERA_SMART_HEIGHT_TURNING", se_cameraSmartHeightTurning );

// influence of grinding
static REAL se_cameraSmartHeightGrinding = 0;
static tConfItem< REAL > se_confCameraSmartHeightGrinding( "CAMERA_SMART_HEIGHT_GRINDING", se_cameraSmartHeightGrinding );

// influence of wall in front
static REAL se_cameraSmartHeightObstacle = 1.0;
static tConfItem< REAL > se_confCameraSmartHeightObstacle( "CAMERA_SMART_HEIGHT_OBSTACLE", se_cameraSmartHeightObstacle );

// factor moving the camera to the side if it is in front of the cycle
static REAL se_cameraSmartAvoidFront = 10.0;
static tConfItem< REAL > se_confCameraSmartAvoidFront( "CAMERA_SMART_AVOID_FRONT", se_cameraSmartAvoidFront );

// factor moving the camera to the side if it is in front of the cycle
static REAL se_cameraSmartAvoidFront2 = 0.1;
static tConfItem< REAL > se_confCameraSmartAvoidFront2( "CAMERA_SMART_AVOID_FRONT2", se_cameraSmartAvoidFront2 );

// amount of turning from grinding
static REAL se_cameraSmartTurn = 5.0;
static tConfItem< REAL > se_confCameraSmartTurn( "CAMERA_SMART_TURN_GRINDING", se_cameraSmartTurn );

// speed of center pos smoothing
static REAL se_cameraSmartCenterPosSmooth = 6.0;
static tConfItem< REAL > se_confCameraSmartCenterPosSmooth( "CAMERA_SMART_CENTER_POS_SMOOTH", se_cameraSmartCenterPosSmooth );

// speed of center dir smoothing
static REAL se_cameraSmartCenterDirSmooth = 3.0;
static tConfItem< REAL > se_confCameraSmartCenterDirSmooth( "CAMERA_SMART_CENTER_DIR_SMOOTH", se_cameraSmartCenterDirSmooth );

// amount of lookahead relative to speed
static REAL se_cameraSmartCenterLookahead = .5;
static tConfItem< REAL > se_confCameraSmartCenterLookahead( "CAMERA_SMART_CENTER_LOOKAHEAD", se_cameraSmartCenterLookahead );

// max amount of lookahead
static REAL se_cameraSmartCenterMaxLookahead = 5;
static tConfItem< REAL > se_confCameraSmartCenterMaxLookahead( "CAMERA_SMART_CENTER_MAX_LOOKAHEAD", se_cameraSmartCenterMaxLookahead );


/*


static REAL se_cameraSmart =;
static tSettingItem< REAL > se_confCameraSmart( "CAMERA_SMART", se_cameraSmart );
*/

#ifndef DEDICATED
bool displaying=false;

void eCamera::Render(){
    if (!sr_glOut)
        return;
    displaying=true;

    se_cameraRise=rise;
    se_cameraZ=z;

    //REAL  ts=ArmageTronTimer-lastrendertime;
    lastrendertime=se_GameTime();

    makefinite(pos);
    makefinite(lastPos);
    makefinite(top);
    makefinite(dir);
    makefinite(rise,0);
    makefinite(z,2);
    makefinite(distance,0);
    makefinite(smartcamSkewSmooth);
    makefinite(smartcamFrontSmooth);
    makefinite(smartcamIncamSmooth);
    makefinite(centerDirSmooth);
    makefinite(centerPosSmooth);
    makefinite(centerPosLast);
    makefinite(centerIncam);
    makefinite(userCameraControl);
    makefinite(glanceSmooth,0);
    makefinite(turning);
    makefinite(smoothTurning);
    makefinite(glanceSmooth);
    makefinite(fov);
    makefinite(distance);
    makefinite(lastrendertime);

    /*
    eCoord glancedir=dir.Turn(1,glanceSmooth).Turn(1,glanceSmooth);
    glancedir=glancedir*(1/sqrt(glancedir.NormSquared()));
    if (glancingBack)
        glancedir=glancedir*(-1);

    eCoord pos_diff=pos-CenterPos();
    if (mode != CAMERA_FREE){
        pos_diff = pos_diff.Turn(dir.Conj());
        pos_diff = pos_diff.Turn(glancedir);
    }
    pos_diff = pos_diff + CenterPos();
    */

    eCoord pos_diff = Glance( pos, glanceDir_ );
    eCoord glancedir = dir.Turn( glanceDir_ );

    if (!se_ClampCamera(mode))
        Bound(0, pos_diff);

    // Bound( dt );

    if (z>400) z=300;
    if (z<0) z=0;

    if (rise<-100) rise=-100;
    if (rise>100) rise=100;

    // camera control logic
    /*
      if (center)
      Timestep(ts);
    */

    //  foot->Move(pos_diff,0,1);

    /*
      if (foot->currentFace)
      foot->currentFace->SetVisHeight(id,0);
      foot->Move(pos+dir*.01,0,1);
      if (foot->currentFace)
      foot->currentFace->SetVisHeight(id,0);
    */

    distance+=sqrt((lastPos-pos_diff).NormSquared())*1.5;
    lastPos=pos_diff;

#ifdef DEBUG
    //  eEdge::UpdateVisAll(id);
#endif

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    if(CenterCockpitFixedBefore()){
        vp->Perspective(fov,zNear,1E+20);

        /*
          gluLookAt(pos.x,
             pos.y,
             z,

             pos.x+dir.x,
             pos.y+dir.y,
             z+rise,

             top.x,top.y,
             1);
        */

        gluLookAt(0,
                  0,
                  0,

                  glancedir.x,
                  glancedir.y,
                  rise,

                  top.x,top.y,
                  1);

        glTranslatef(-pos_diff.x,-pos_diff.y,-z);
        glMatrixMode(GL_MODELVIEW);

        bool draw_center=((CenterPos()-pos).NormSquared()>1 ||
                          fabs(CenterZ() - z)>1);

        tJUST_CONTROLLED_PTR< eGameObject > c=Center();
        if (!draw_center && c) c->RemoveFromList();

        eCoord poscopy = pos;
        zNear = - eWallRim::Bound( poscopy, 0.0f );
        if (zNear < -.1 )
            zNear = .1;

        grid->Render( this, id, zNear );

        zNear *= .3f;
        if ( zNear < 0.0001f )
        {
            zNear = 0.0001f;
        }

        if (c) c->RenderCockpitVirtual();
        if (!draw_center && c) c->AddToList();

        /*
          glDisable(GL_TEXTURE);
          glColor3f(1,1,1);
          glBegin(GL_LINES);
          glVertex3f(centerPosSmooth.x,centerPosSmooth.y,0);
          glVertex3f(centerPosSmooth.x,centerPosSmooth.y,10);
          glEnd();
        */

        CenterCockpitFixedAfter();
    }
    displaying=false;
}

#endif

static bool se_glanceSnap = false;
static tSettingItem< bool > se_glanceSnapConf( "GLANCE_SNAP", se_glanceSnap );

static bool se_holdGlancing = false;
static tSettingItem< bool > se_holdGlancingConf( "GLANCE_HOLD", se_holdGlancing );


void eCamera::SwitchCenter(int d){
    zNear = 0.01f;

    int centerID = 0;
    if (center)
        centerID = center->interestingID;
    center = NULL;

    if (centerID>=grid->gameObjectsInteresting.Len())
        centerID=0;
    if (centerID<0)
        centerID=grid->gameObjectsInteresting.Len()-1;

    int timeout=(grid->gameObjectsInteresting.Len()+1)*5;
    int oldid=centerID;
    if (grid->gameObjectsInteresting.Len()>0){
        if (!InterestingToWatch(grid->gameObjectsInteresting(centerID)))
            grid->gameObjectsInteresting.Remove
            (grid->gameObjectsInteresting(centerID),
             grid->gameObjectsInteresting(centerID)->interestingID);
        do{
            timeout--;
            centerID+=d;

            if (centerID<0)
                centerID=grid->gameObjectsInteresting.Len()-1;
            if (centerID>=grid->gameObjectsInteresting.Len())
                centerID=0;

        }while(timeout >0 && grid->gameObjectsInteresting.Len()>0 &&
                oldid!=centerID &&
                !InterestingToWatch(grid->gameObjectsInteresting(centerID)));
    }
    else centerID=0;
    // con << "swtiched view from " << oldid << " to " << centerID << '\n';

    if ( centerID >= 0 && centerID < grid->gameObjectsInteresting.Len() )
    {
        center = grid->gameObjectsInteresting(centerID);
        lastSwitch=lastTime;
    }
}


void eCamera::Timestep(REAL ts){
    // find net player
    if (!netPlayer && localPlayer)
    {
        netPlayer = localPlayer->netPlayer;
    }
    if (fov != localPlayer->FOV) {
        fov = localPlayer->FOV;
    }

    if ((!netPlayer || (netPlayer && !InterestingToWatch(netPlayer->Object())) ) && localPlayer->watchPlayer != nullptr)
    {
        gCycle *cycle = ePlayerNetID::NetPlayerToCycle(localPlayer->watchPlayer);
        if (center != cycle && InterestingToWatch(cycle))
        {
            center = cycle;
        }
    }

    // the best center is always our own vehicle. Focus on it if possible.
    if (netPlayer)
    {
        eGameObject * bestCenter = netPlayer->Object();
        if ( InterestingToWatch(bestCenter) )
        {
            if ( bestCenter != center )
            {
                center = bestCenter;
                // if ( mode != CAMERA_FREE )
                mode=localPlayer->startCamera;
            }
        }
    }

    // update blending factor for smooth glancing
    {
#define GLANCE_SPEED 20

        glanceSmoothAbs/=(1+GLANCE_SPEED*ts);

        REAL abs = fabs(glanceSmooth);
        glanceSmoothAbs = abs > glanceSmoothAbs ? abs : glanceSmoothAbs;

        // override: go all the way when glancing back
        if ( glancingBack || glancingForward )
        {
            glanceSmoothAbs = 1;
        }
    }

    // determine camera custom parameters based on speed, camera mode and camera settings
    REAL customBack = 0, customRise = 0, customPitch = 0, customTurnSpeed = s_customTurnSpeed, customTurnSpeed180 = s_customTurnSpeed180;
    REAL glanceBack = 0, glanceRise = 0, glancePitch = 0;
    {
        REAL speed = centerSpeedSmooth;
        if ( mode == CAMERA_SERVER_CUSTOM )
        {
            customBack = s_serverCustomBack + speed * s_serverCustomBackSpeed;
            customRise = s_serverCustomRise + speed * s_serverCustomRiseSpeed;
            customPitch = s_serverCustomPitch;

            if ( s_serverCustomTurnSpeed >= 0 )
            {
                customTurnSpeed = s_serverCustomTurnSpeed;
                customTurnSpeed180 = s_serverCustomTurnSpeed180;
            }
        }
        else
        {
            customBack = s_customBack + speed * s_customBackSpeed;
            customRise = s_customRise + speed * s_customRiseSpeed;
            customPitch = s_customPitch;
        }

        // select glance settings
        if ( ( mode == CAMERA_SERVER_CUSTOM && se_overrideCustomServerCustom ) ||
                se_overrideCustom )
        {
            glanceBack = s_serverGlanceBack + speed * s_serverGlanceBackSpeed;
            glanceRise = s_serverGlanceRise + speed * s_serverGlanceRiseSpeed;
            glancePitch = s_serverGlancePitch;
        }
        else
        {
            glanceBack = s_glanceBack + speed * s_glanceBackSpeed;
            glanceRise = s_glanceRise + speed * s_glanceRiseSpeed;
            glancePitch = s_glancePitch;
        }

        // blend the two according to the glance status
        REAL blend = glanceSmoothAbs;
        if ( mode != CAMERA_CUSTOM && mode != CAMERA_SERVER_CUSTOM )
            blend = 1;
        if ( se_forbidCustomGlance )
            blend = 0;
        customBack  = customBack  * ( 1-blend ) + glanceBack  * blend;
        customRise  = customRise  * ( 1-blend ) + glanceRise  * blend;
        customPitch = customPitch * ( 1-blend ) + glancePitch * blend;
    }

    // switch away from dead players
    if (lastSwitch>lastTime)
        lastSwitch=lastTime;
    if (!InterestingToWatch(center) && lastTime-lastSwitch>2){
        center = se_GetWatchedObject( this );
        if ( !center || !InterestingToWatch(center) )
            SwitchCenter(1);

        // Did not work as expected, more work needs to be done to reset the settings
        //        else
        //        {
        //            if (localPlayer)
        //            {
        //                mode=localPlayer->startCamera;
        //            }
        //        }
    }

    if (!Center())
        return;

    // watch for turns of the center game object
    if ( fabs( centerDirLast * Center()->Direction() ) > .01 )
    {
        turning+=.5;
        centerDirLast = Center()->Direction();
    }

    for(int i = hitCacheSize-1; i>=0; --i)
    {
        hitCache_[i] += ts * se_hitCacheSpeed;
        if (hitCache_[i] > 1)
            hitCache_[i] = 1;
    }

    // flag telling someone at the end of the function whether Bound() was already called
    bool bound = false;

    eCoord objdir=CenterCamDir();

    if (glancingLeft)
        glanceSmooth+=GLANCE_SPEED*ts;

    if (glancingRight)
        glanceSmooth-=GLANCE_SPEED*ts;

    glanceSmooth/=(1+GLANCE_SPEED*ts);

    glanceDir_ = eCoord(1,glanceSmooth).Turn(1,glanceSmooth);
    glanceDir_.Normalize();
    if (glancingBack)
        glanceDir_=glanceDir_*(-1);

    // update center positions
    if ( Center() )
    {
        #define SMOOTH_SPEED 1
        centerSpeedSmooth = ( centerSpeedSmooth + Center()->Speed() * ts * SMOOTH_SPEED)/( 1 + ts * SMOOTH_SPEED );

        centerPos = Center()->PredictPosition();

        // move it a bit to the side (disabled for now, does not have the desired effect of making walls visible)
        //eCoord side = Center()->Direction().Turn(0,1);
        //REAL displace = eCoord::F( Center()->CamPos() - centerPos, side );
        //centerPos = centerPos + side * displace;
    }
    centerPosSmooth=(centerPosSmooth+ CenterPos()*(ts*se_cameraSmartCenterPosSmooth))
                    *(1/(1+ts*se_cameraSmartCenterPosSmooth));

    //centerPosSmooth=centerPosition();

    //REAL dist_from_center=sqrt((centerPos-pos).NormSquared()+
    //(CenterZ() - z)*(CenterZ() - z));

    if (!CenterAlive() && (mode==CAMERA_IN || mode==CAMERA_SMART_IN)){// || mode==CAMERA_CUSTOM || mode==CAMERA_SERVER_CUSTOM)){
        pos=pos-dir.Turn(eCoord(5,1));
        z+=2;
        mode = localPlayer ? localPlayer->startCamera : CAMERA_SMART;
    }

    const REAL dirSmooth = se_cameraSmartCenterDirSmooth;
    centerDirSmooth=(centerDirSmooth+(CenterDir()*dirSmooth*ts))*
                    (1/(1+dirSmooth*ts));

    //	eCoord centerpos=centerPosSmooth+centerDirSmooth * ( this->CenterSpeed() * .05f );
    REAL speedFactor = se_GameTime() * this->CenterSpeed() * se_cameraSmartCenterLookahead;
    if ( speedFactor < 0.0f )
    {
        speedFactor = 0.0f;
    }
    if ( speedFactor > se_cameraSmartCenterMaxLookahead )
    {
        speedFactor = se_cameraSmartCenterMaxLookahead;
    }
    eCoord centerpos=centerPosSmooth + centerDirSmooth * speedFactor;


#define SMART_INCAM_SPEED 1
#define SMART_FRONT_SPEED 4

    userCameraControl/=(1+ts*5);
#define maxcontrol 10
    if (userCameraControl>maxcontrol)
        userCameraControl=maxcontrol;

    smartcamFrontSmooth/=(1+SMART_FRONT_SPEED*ts);
    smartcamSkewSmooth/=(1+2*ts);
    smartcamIncamSmooth/=(1+SMART_INCAM_SPEED*ts);

    eCoord newpos=pos,newdir=dir;
    REAL newz=z,newrise=rise;

    eCoord usernewpos=pos;
    eCoord usernewdir=dir;
    REAL usernewz=z;
    REAL usernewrise=rise;

    REAL relax=se_cameraSmartDistance;//1 + 34/(CenterSpeed() + 1);
    //	REAL wish_h=2*vp->UpDownFOV(fov)/60 * SpeedMultiplier();
    REAL wish_h=se_cameraSmartHeight*vp->UpDownFOV(fov)/60 * ( this->CenterSpeed() * .02 + SpeedMultiplier() );
    REAL min_dist=se_cameraSmartMinDistance;

    turning/=(1+2*ts);
    smoothTurning+=3*turning*ts;
    smoothTurning/=1+ts;

#define maxs 5
    if (smoothTurning>maxs) smoothTurning=maxs;

    REAL side;
    REAL eturn;

    top=eCoord(0,0);

    // temporarily use free cam code if nothing interesting to watch exists
    eCamMode effectiveMode = mode;
    if (!InterestingToWatch(Center()))
    {
        effectiveMode=CAMERA_FREE;
    }

    switch (effectiveMode){
    case CAMERA_FREE:
        newpos=pos;
        newdir=dir;
        newz=z;
        newrise=rise;
        break;
    case CAMERA_SMART_IN:
    case CAMERA_CUSTOM:
    case CAMERA_SERVER_CUSTOM:
    case CAMERA_IN:
        if (WhobbleIncam()){
            top=CenterCamTop();
            newpos=CenterCamPos();
        }
        else
            newpos=CenterPos();

        if (CenterIncamOnTurn() || mode==CAMERA_SMART_IN || mode == CAMERA_CUSTOM || mode == CAMERA_SERVER_CUSTOM )
        {
            // fetch the relevant turning speed
            REAL turnSpeed = ( mode == CAMERA_IN || mode == CAMERA_SMART_IN ) ? s_inTurnSpeed : customTurnSpeed;

            if ( se_glanceSnap && ( glancingForward || glancingBack || glancingLeft || glancingRight ) )
            {
                turnSpeed += GLANCE_SPEED;
            }

            eCoord cycleDir = CenterCycleDir();

            if( se_holdGlancing )
            {
                static bool glancingLast = false;
                static eCoord glancingPos;
                if( !glancingLast )
                {
                    if( glancingBack || glancingLeft || glancingRight || glancingForward )
                    {
                        glancingPos = cycleDir;
                        glancingLast = true;
                    }
                }
                else
                {
                    if( glancingBack || glancingLeft || glancingRight || glancingForward )
                    {
                        cycleDir = glancingPos;
                    }
                    else
                    {
                        glancingLast = false;
                    }
                }
            }

            newdir=dir+cycleDir*(turnSpeed*ts);

            // test if we're looking against the current driving direction
            REAL wrongDirection = -eCoord::F( cycleDir, newdir );
            if ( Center() &&  wrongDirection > 0 )
            {
                // if so, turn to the side using the last driving direction
                //newdir = newdir + Center()->LastDirection()*(wrongDirection*ts*turnSpeed*customTurnSpeed180);
                eCoord normedLastDir = Center()->LastDirection();
                REAL wrongWrongDirection = -eCoord::F(cycleDir, normedLastDir);
                if( wrongWrongDirection > 0)
                {
                    normedLastDir = normedLastDir + cycleDir * (wrongWrongDirection/cycleDir.NormSquared());
                    REAL n = normedLastDir.NormSquared();
                    if(n > 0)
                    {
                        normedLastDir *= 1/sqrt(n);
                    }
                }

                newdir = newdir + normedLastDir*(wrongDirection*ts*turnSpeed*customTurnSpeed180);
            }
        }
        else
            newdir=dir;

        if ( mode == CAMERA_IN || mode == CAMERA_SMART_IN )
        {
            const REAL forwardCheck = .1;

            // cast ray forward; don't be too close to a wall
            eSensor forward( Center(), newpos, newdir );
            forward.detect( forwardCheck );
            if ( forward.ehit )
            {
                REAL backwardCheck = ( forwardCheck - forward.hit ) * 2;
                eSensor backward( Center(), newpos, -newdir );
                backward.detect( backwardCheck );
                newpos = newpos - newdir * ( backward.hit * .5 );
            }
        }

        if (mode != CAMERA_CUSTOM && mode != CAMERA_SERVER_CUSTOM)
        {
            newz=CenterCamZ();
            newrise=rise;
            if (newrise>2) newrise=2;
            if (newrise<-2) newrise=-2;

            usernewpos=newpos;
            usernewz=newz;
        }


        if (mode==CAMERA_SMART_IN){
            REAL space[2];

            REAL dist = CenterSpeed() * .2f;
            if (dist < 5)
                dist = 5;

            for(int i=0;i<2;i++){
                eSensor s(Center(),CenterPos(),CenterDir().Turn(1,2*i-1));
                s.detect(dist);
                space[i]=s.hit;
            }
            smartcamIncamSmooth+=(space[0]+space[1])*ts*SMART_INCAM_SPEED/dist;

            if (smartcamIncamSmooth>.8){
                eSensor s(Center(),CenterPos(),CenterCycleDir());
                s.detect(5.5);

                if (s.hit>5){
                    mode=CAMERA_SMART;
                    usernewz=newz=z+.5;
                    usernewpos=newpos=pos+dir.Turn(-1,.1);
                }
            }
        }

        if (mode != CAMERA_CUSTOM && mode != CAMERA_SERVER_CUSTOM)
        {
            int x=3;
            while (eCoord::F(newdir,objdir)<-.5001 && x>0){
                newdir=newdir-objdir*(eCoord::F(newdir,objdir)+.5);
                newdir=newdir*(1/sqrt(newdir.NormSquared()));
                x--;
            }
        }
        else if ( mode == CAMERA_CUSTOM || mode == CAMERA_SERVER_CUSTOM )
        {
            REAL zoom = lastTime > 0 ? 1 : exp( s_customZoom * lastTime );

            newdir=newdir*(1/sqrt(newdir.NormSquared()));
            newpos     = newpos - newdir * customBack * zoom;
            usernewpos = usernewpos + centerpos - centerPosLast;
            newrise    = customPitch;
            newz       = CenterCamZ() + customRise * zoom;
        }

        break;
    case CAMERA_SMART:
        {
            REAL dist = CenterSpeed() * se_cameraSmartDistanceScale;
            if (dist < se_cameraSmartMinDistanceScale)
                dist = se_cameraSmartMinDistanceScale;

            REAL space[2];

            for(int i=0;i<2;i++){
                eSensor s(Center(),CenterPos(),CenterDir().Turn(1,2*i-1));
                s.detect(dist);
                space[i]=s.hit;
            }

            REAL slowFactor = this->CenterSpeed() / se_cameraSmartCycleSpeed;
            if ( slowFactor > 1.0f )
            {
                slowFactor = 1.0f;
            }

            eSensor front(Center(), CenterPos(), CenterDir());
            front.detect(dist * 4);
            REAL ff = (4 * dist - front.hit)/(3*dist);
            ff *= ff;
            smartcamFrontSmooth+=ff*SMART_FRONT_SPEED*ts;

            smartcamSkewSmooth+=(space[0]-space[1])*ts * slowFactor;
            smartcamIncamSmooth+=(space[0]+space[1])*ts*SMART_INCAM_SPEED/dist;

            REAL sk = fabs(smartcamSkewSmooth)/5;
            if (sk > 1)
                sk = 1;

            REAL rf = .15+.1*smoothTurning - sk * .15 - .05 * smartcamFrontSmooth;
            if (rf < .05)
                rf = .05;

            relax*=rf;
            relax/=slowFactor;
            // wish_h*=.5+1.5*smoothTurning + 2 * sk + smartcamFrontSmooth;
            wish_h*=se_cameraSmartHeightExtra + se_cameraSmartHeightTurning*smoothTurning + se_cameraSmartHeightGrinding * sk + smartcamFrontSmooth * se_cameraSmartHeightObstacle;
            min_dist/=3; // +smoothTurning;

            {
                if (!CenterAlive()) wish_h+=3;
                REAL front=eCoord::F(pos-centerpos,CenterDir());
                side=((pos-centerpos)*CenterDir()) * front;
                //eCoord::F(pos-centerpos,CenterDir);
                eturn=ts/relax * (1 + .5 * smartcamFrontSmooth);
                if (side>0) eturn*=-1;

                newz=z;

                // we do not want to look at the cycle front

                //eCoord skew;
                if (front>0){ // increase skew
                    if (front>2.5) front=2.5;
                    if (fabs(smartcamSkewSmooth)>1 || smartcamSkewSmooth*eturn>0)
                        smartcamSkewSmooth*=(1+ts);
                    if (fabs(smartcamSkewSmooth)<1)
                        smartcamSkewSmooth -= se_cameraSmartAvoidFront * eturn;
                    newz+=ts*front*.1;
                    //if ( Center() )
                    //    skew = -Center()->LastDirection()*(front*dist*.2);
                }

                if (se_GameTime()>0){
                    newpos=pos + CenterDir().Turn(eCoord(0,eturn*se_cameraSmartAvoidFront2));
                    newpos=newpos+CenterDir().Turn(0,-1)*smartcamSkewSmooth*ts*se_cameraSmartTurn;
                    //newpos=newpos+skew*ts*5;
                    newpos=newpos+centerpos*(ts/relax);
                    newpos=newpos*(1/(1+ts/relax));
                }
                else{
                    newpos=pos+ (pos-centerpos).Turn(-ts*.5,ts*.5);
                }

                if ( userCameraControl < .25  && se_ClampCamera(mode) )
                {
                    bound = true;
                    eCoord glancePos = Glance( newpos, glanceDir_ );
                    Bound( ts, glancePos );
                    newpos = Glance( glancePos, glanceDir_.Conj() );
                    // Bound( ts, newpos );
                }

                newz=newz+(CenterZ()+wish_h)*(ts/relax);
                newz=newz/(1+ts/relax);
                newdir=centerpos-newpos;
                REAL dist=sqrt(newdir.NormSquared());
                if (dist<.001) dist=.01;
                //					newdir=dir+(centerDirSmooth*16+newdir)*ts;
                newdir=dir+newdir*ts*5.0;
                newdir=newdir*(1/sqrt(newdir.NormSquared()));

                if (dist<min_dist){
                    //newpos=newpos-newdir*((min_dist-dist)*(min_dist-dist)*ts);
                    REAL dz=(min_dist*min_dist-dist*dist-.5*z*z);
                    if (dz>0)
                        newz+=dz*ts;
                }

                REAL d=eCoord::F(newdir,centerpos - newpos);
                if (d<.0001) d=.0001;
                newrise=(CenterZ()-newz)/d;

                usernewpos=pos + centerpos - centerPosLast;
                // usernewdir=newdir;
                // usernewrise=newrise;

                // use custom camera settings when glancing
                if ( localPlayer && !se_forbidCustomGlance && localPlayer->smartCustomGlance && ( glancingForward || glancingBack || glancingRight || glancingLeft || glanceSmoothAbs > .01 ))
                {
                    // calculate blending factor c. c=0 will take the smart cam position, c=1 the custom cam.
                    REAL b = 1 - glanceSmoothAbs;
                    REAL c = 1 - b/(b + ts * GLANCE_SPEED);

                    // the camera pitch is calculated anew every frame, blend it accordingly
                    usernewrise = newrise = newrise * ( 1-glanceSmoothAbs) + customPitch * glanceSmoothAbs;

                    // the other values are updated every frame, blend them softer
                    if ( glancingForward || glancingBack || glancingRight || glancingLeft )
                    {
                        usernewpos  =  newpos = pos * (1-c) + (CenterPos() - CenterCamDir() * customBack) * c;
                        usernewz    = newz    = z * (1-c) + (CenterCamZ() + customRise) * c;

                        newdir=centerpos-newpos;
                        REAL dist=sqrt(newdir.NormSquared());
                        if (dist<.001) dist=.01;
                        usernewdir=newdir=newdir*(1/sqrt(newdir.NormSquared()));
                    }
                }

                if (AutoSwitchIncam()){
                    if (smartcamIncamSmooth<.7 && CenterAlive()){
                        eSensor s(Center(),CenterPos(),CenterDir());
                        s.detect(5.5);
                        if (s.hit>5){
                            usernewrise=newrise=0;
                            mode=CAMERA_SMART_IN;
                            usernewdir=newdir=objdir;
                        }
                    }
                }
                else
                    if (smartcamIncamSmooth<.7)
                        newz+=ts*(.7-smartcamIncamSmooth);
            }
        }
        break;
    case CAMERA_FOLLOW:{
            newpos=usernewpos=pos + centerpos - centerPosLast;
            newz=z;
            newdir=centerpos-newpos;
            REAL dist=sqrt(newdir.NormSquared());
            newdir=newdir*(1/dist);
            newrise=(CenterZ()-newz)/dist;
        }
        break;
        /*
        case CAMERA_FOLLOW:{
           // meriton's camera hack starts here
        #define mercamxydist 10
        #define mercamz 5
           eCoord t = pos - centerpos;

           t=t*(mercamxydist/sqrt(t.NormSquared()));
           newpos=usernewpos=centerpos+t;
           newz=mercamz; //z;
           newdir=centerpos-newpos;
           REAL
           dist=sqrt(newdir.NormSquared());
           newdir=newdir*(1/dist);
           newrise=(CenterZ()-newz)/dist;       }
        break;
        */
    default:
        break;
    }


    // REAL ratio=1-exp(-100*exp(-userCameraControl)*ts);

    /*
      REAL ratio=1 - exp(-4*userCameraControl);
      ratio*=ts;
      ratio*=100;
      ratio=exp(-ratio);
    */

    // calcualte ratios under which user and automatic camera positions should be blended
    REAL aratio = exp(-4*userCameraControl);
    REAL ratio = 1 - aratio;

    // blend
    pos=newpos*aratio + usernewpos*ratio;
    dir=newdir*aratio + usernewdir*ratio;
    z  =newz  *aratio + usernewz  *ratio;

    // newrise rise is calculated anew every frame, use a different blending
    if (userCameraControl > .01)
    {
        rise=newrise + (usernewrise-newrise)*exp(-ts/userCameraControl);
    }
    else
    {
        rise=newrise;
    }

    // normalize direction
    dir=dir*(1/sqrt(dir.NormSquared()));

    centerPosLast=centerpos;

    // bound camera if that was not already done earlier
    if (!bound && se_ClampCamera(mode) )
        Bound( ts );
}



void eCamera::s_Timestep(eGrid *grid, REAL time){
    if (fabs(time-lastTime)>1) lastTime=time-.1;
    if (time>lastTime){
        eDebugLine::Update(time-lastTime);

        for(int i=grid->cameras.Len()-1;i>=0;i--){
            //con << time-lastTime<< '\n';
            eCamera *c = grid->cameras(i);
            c->Timestep(time-lastTime);
            su_FetchAndStoreSDLInput();
        }
        lastTime=time;
    }
}


#ifndef DEDICATED

void eCamera::SoundMix(Uint8 *dest,unsigned int len){
    tASSERT_THIS();

    if (id>=0){
        eGameObject *c=Center();
        for(int i=grid->gameObjects.Len()-1;i>=0;i--){
            eGameObject *go=grid->gameObjects(i);
            SoundMixGameObject(dest,len,go);
        }
        if (c && c->id<0)
            SoundMixGameObject(dest,len,c);
    }
}


void eCamera::SoundMixGameObject(Uint8 *dest,unsigned int len,eGameObject *go){
    if(!go)
        return;

    eCoord vec((go->pos-pos).Turn(dir.Conj()));
    REAL dist_squared=vec.NormSquared()+(z-go->z)*(z-go->z);

    //dist_squared*=.1;
    if (dist_squared<1)
        dist_squared=1;

    REAL dist=sqrt(dist_squared);

#define MAXVOL .4

    REAL l=(dist*.5+vec.y)/dist_squared;
    REAL r=(dist*.5-vec.y)/dist_squared;

    if (l<0) l=0;
    if (r<0) r=0;
    if (l>MAXVOL) l=MAXVOL;
    if (r>MAXVOL) r=MAXVOL;

    if (go==Center()){
        if (mode==CAMERA_IN || mode==CAMERA_SMART_IN)
            l=r=.2;
        else if (mode!=CAMERA_FREE){
            l*=.9;
            r*=.9;
        }
    }

    go->SoundMix(dest,len,id,r,l);
}


#endif

eCoord eCamera::CenterPos() const{
    return centerPos;
}

eCoord eCamera::CenterCycleDir() const{
    return CenterDir();
}

eCoord eCamera::CenterDir() const{
    eGameObject *go=Center();
    if (go)
        return go->Direction() ;
    else
        return eCoord(1,0);
}

eCoord eCamera::CenterCamDir() const{
    eGameObject *go=Center();
    if (go)
        return go->CamDir() ;
    else
        return eCoord(1,0);
}

eCoord eCamera::CenterCamTop() const{
    eGameObject *go=Center();
    if (go)
        return go->CamTop();
    else
        return eCoord(0,0);
}

eCoord eCamera::CenterCamPos() const{
    eGameObject *go=Center();
    if (go)
        return go->CamPos();
    else
        return eCoord(100,100);
}

REAL  eCamera::CenterCamZ() const{
    eGameObject *go=Center();
    if (go)
        return go-> CamZ();
    else
        return 1.5;
}

REAL  eCamera::CenterZ() const{
    eGameObject *go=Center();
    if (go)
        return go->z ;
    else
        return 1.5;
}

REAL  eCamera::CenterSpeed() const{
    eGameObject *go=Center();
    if (go)
        return go->Speed();
    else
        return 20;
}


bool eCamera::CenterAlive() const{
    eGameObject *go=Center();
    if (go)
        return go->Alive() ;
    else
        return false;
}


bool eCamera::CenterCockpitFixedBefore() const{
    eGameObject *go=Center();
    if (go)
        return go->RenderCockpitFixedBefore();
    else
        return true;
}

void eCamera::CenterCockpitFixedAfter() const{
    eGameObject *go=Center();
    if (go)
        go->RenderCockpitFixedAfter() ;
    else
        return;
}


bool eCamera::SetCamMode(eCamMode m){

    if ((localPlayer && localPlayer->allowCam[m] && (!forbid_camera[m])) && m != CAMERA_COUNT) {
    	mode = m;
		switch(mode){
		case CAMERA_IN:
			rise=0;
			break;
		case CAMERA_SMART_IN:
			break;
		case CAMERA_CUSTOM:
			rise=0;
			break;
		case CAMERA_SERVER_CUSTOM:
			rise=0;
			break;
		case CAMERA_FREE:
			break;
		case CAMERA_FOLLOW:
			break;
		case CAMERA_SMART:
			smartcamIncamSmooth=1;
			z=z+1;
			pos=pos+dir.Turn(-1,.1);
			break;
        case CAMERA_COUNT:
            tASSERT(0);
            break;
		}
		return true;
	}
	return false;
}
