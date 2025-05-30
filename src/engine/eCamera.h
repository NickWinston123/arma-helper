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

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

***************************************************************************

*/

#ifndef ArmageTron_CAMERA_H
#define ArmageTron_CAMERA_H

typedef enum {CAMERA_IN=0, CAMERA_CUSTOM=1, CAMERA_FREE=2, CAMERA_FOLLOW=3,
              CAMERA_SMART=4,  CAMERA_SERVER_CUSTOM=5, CAMERA_SMART_IN=6, CAMERA_COUNT=7 } eCamMode;

#include "eCoord.h"
//#include "uInput.h"
#include "rViewport.h"
#include "tList.h"
#include "nObserver.h"

class ePlayer;
class ePlayerNetID;
class eGameObject;
class eNetGameObject;
class eCamera;
class eGrid;
class uActionCamera;
class uActionTooltip;

//extern REAL se_cameraRise; // how far down does the current camera look?
//extern REAL se_cameraZ;

// extern List<eCamera> se_cameras;

class eCamera{
protected:
    static uActionCamera se_lookUp,se_lookDown,se_lookLeft,se_lookRight,
    se_moveLeft,se_moveRight,se_moveUp,se_moveDown,se_moveForward,se_moveBack,
    se_zoomIn,se_zoomOut,
    se_glanceLeft,se_glanceRight,se_glanceBack,se_glanceForward,
    se_switchView;
    static uActionTooltip se_glanceLeftTooltip, se_glanceRightTooltip, se_glanceBackTooltip, se_glanceForwardTooltip, se_switchViewTooltip;

    int id;
    //  tCHECKED_PTR(eGameObject) foot;
    tCHECKED_PTR(eGrid)           grid;
    //eGameObject *center;
    nObserverPtr< ePlayerNetID>	netPlayer;
    tCHECKED_PTR(ePlayer)         localPlayer;
  public:
    // mutable nObserverPtr<eNetGameObject> center; // the game object we watch
    tJUST_CONTROLLED_PTR<eGameObject> center; // the game object we watch
protected:
    // mutable int  centerID; // the game id of the object we are watching
    //int        lastCenterID; // the game id of the object we are watching
    REAL         lastSwitch; // the last center switch
    REAL         zNear	;	 // near clipping plane
    eCamMode     mode   ;    // current view mode

    eCoord pos;       // Position
    eCoord lastPos;  // last rendered position
    eCoord dir;       // direction
    eCoord top;       // vector (top,1) is top of camera (limits views)
    REAL  z,rise;    // height above the floor and whether we look up or down
    REAL  fov;       // field of vision;

    REAL turning;   // number of turns in the last seconds
    REAL smoothTurning; //that value smoothed


    REAL distance;    // distance travelled so far
    REAL lastrendertime; // the time this was last rendered

    REAL smartcamSkewSmooth, smartcamIncamSmooth, smartcamFrontSmooth, centerSpeedSmooth;
    eCoord centerDirSmooth;
    eCoord centerDirLast;
    eCoord centerPos;
    eCoord centerPosSmooth;

    tCHECKED_PTR(rViewport) vp;

    eCoord centerPosLast;
    REAL  userCameraControl;
    REAL  centerIncam;
public:
    bool glancingLeft,glancingRight,glancingBack,glancingForward;
protected:
    REAL glanceSmooth, glanceSmoothAbs;
    eCoord glanceDir_;

    bool renderingMain_;	// flag indicating whether the current rendering process is the main process or just a mirror effect

    bool cameraMain_;		// flag indicating whether the camera is a main camera or a widget view

    static bool InterestingToWatch(eGameObject const * g);

    eCoord Glance( eCoord const & in, eCoord const & glanceDir ) const;

    void Bound( REAL dt ); //!< make sure the camera is inside the arena and has clear line of sight
    void Bound( REAL dt, eCoord & pos ); //!< make sure pos is inside the arena and has clear line of sight

    void MyInit();
public:
    bool CenterIncamOnTurn();
    bool WhobbleIncam();
    bool AutoSwitchIncam();

    bool RenderingMain() const { return renderingMain_;  }
    void SetRenderingMain( bool f ){ renderingMain_ = f; }

    bool CameraMain() const { return cameraMain_;  }
    void SetCameraMain( bool f ){ cameraMain_ = f; }

    const ePlayerNetID* Player() const;
    const ePlayer* LocalPlayer() const;

    eCamera(eGrid *grid, rViewport *vp,ePlayerNetID *owner,ePlayer *lp,eCamMode m=CAMERA_IN, bool rMain=true);

    virtual ~eCamera();

    eGameObject * Center() const;

    eCoord CenterPos() const;
    eCoord CenterDir() const;
    virtual eCoord CenterCycleDir() const;

    virtual REAL SpeedMultiplier() const { return 1; }

    eCoord CenterCamDir() const;
    eCoord CenterCamTop() const;
    eCoord CenterCamPos() const;
    REAL  CenterCamZ() const;

    REAL  CenterZ() const;
    REAL  CenterSpeed() const;

    const eCoord& CameraDir() const {return dir;}
    const eCoord& CameraPos() const {return pos;}
    eCoord        CameraGlancePos() const {return Glance(pos, glanceDir_);}
    REAL          CameraZ  () const {return z;}

    bool SetCamMode(eCamMode m);		//! Set camera mode, return true if change is allowed and complete, otherwise false
    void SetCameraPos(eCoord const& pPos) {pos = pPos;}
    void SetCameraZ  (REAL pZ) {z = pZ;}
    
    bool CenterAlive() const;

    bool CenterCockpitFixedBefore() const;
    void CenterCockpitFixedAfter() const;

    void SwitchView();
    void SwitchCenter(int d);
    bool Act(uActionCamera *act,REAL x);
    
#ifndef DEDICATED
    void Render();
    void SoundMix(unsigned char *dest,unsigned int len);
private:
    void SoundMixGameObject(unsigned char *dest,unsigned int len,eGameObject *go);
public:
#endif


    virtual void Timestep(REAL ts);

    REAL Dist(){return distance;}

    /*
    static int    Number(){return se_cameras.Len();}
    static const eCoord& PosNum(int i){
      if (i<se_cameras.Len())
        return se_cameras(i)->lastPos;
      else
        return se_zeroCoord;
    }
    static const eCoord& DirNum(int i){
      if (i<se_cameras.Len())
        return se_cameras(i)->dir;
      else
        return se_zeroCoord;
    }

    static REAL HeightNum(int i){
      if (i<se_cameras.Len())
        return se_cameras(i)->z;
      else
        return 0;
    }
    */

    static void s_Timestep(eGrid *grid, REAL time);
    
private:
    //! make sure CenterPos() + dirFromTarget() is visible from pos
    bool Bound( REAL ratio, eCoord & pos, eCoord const & dirFromTarget, REAL & hitCache );

    enum HitCacheSize{ hitCacheSize = 3 };
    REAL hitCache_[hitCacheSize];
};

/*
inline int    NumberOfCameras(){return eCamera::Number();}
inline const eCoord& CameraPos(int i){return eCamera::PosNum(i);}
inline const eCoord& CameraDir(int i){return eCamera::DirNum(i);}
inline REAL CameraHeight(int i){return eCamera::HeightNum(i);}
*/


#endif
