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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

***************************************************************************

*/
#include "nConfig.h"
#include "gZone.h"
#include "eRectangle.h"
#include "ePlayer.h"
#include "eTimer.h"
#include "eAdvWall.h"
#include "eGrid.h"
#include "gCycle.h"
#include "tDirectories.h"
#include "gSvgOutput.h"

static REAL clamp(REAL c) {
    if(c < 0) return 0;
    if(c > 1) return 1;
    return c;
}

std::ostream &operator<<(std::ostream &s, gSvgColor const &c) {
    s.fill('0');
    return s << '#' << std::hex
             << std::setw(2) << (int)(clamp(c.r)*255)
             << std::setw(2) << (int)(clamp(c.g)*255)
             << std::setw(2) << (int)(clamp(c.b)*255);
}

std::ofstream SvgOutput::svgFile;
long SvgOutput::afterRimWallsPos;
float SvgOutput::lx, SvgOutput::ly, SvgOutput::hx, SvgOutput::hy;
float SvgOutput::w, SvgOutput::h, SvgOutput::cx, SvgOutput::cy;
static bool sg_svgTransparentBackground=false;
static tSettingItem< bool > sg_svgTransparentBackgroundConf( "SVG_TRANSPARENT_BACKGROUND", sg_svgTransparentBackground);

void SvgOutput::WriteSvgHeader() {
    // Assume the file is already open
    svgFile << "<?xml version=\"1.0\" standalone=\"no\"?>\n"
               "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
               "<svg xmlns:xlink='http://www.w3.org/1999/xlink' width=\"100%\" height=\"100%\" version=\"1.1\" viewBox=\""
            << lx   << " " << -hy    << " "
            << hx-lx << " " << hy-ly
            << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
			if ( !sg_svgTransparentBackground ) {
				svgFile << "<rect x=\"" << lx << "\" y=\"" << -hy << "\" width=\"" << hx-lx << "\" height=\"" << hy-ly
					<< "\" stroke=\"none\" fill=\"#222\" />\n\n";
			}
     svgFile << "<defs>\n"
			   "\t<polygon id='cycle' stroke='none' points='4,0 0,2 0,-2' />\n"
               "\t<filter x=\"" << lx << "\" y=\"" << -hy << "\" width=\"" << hx-lx << "\" height=\"" << hy-ly << "\" id='wallsFilter' filterUnits='userSpaceOnUse'>\n"
               "\t\t<feGaussianBlur in='SourceAlpha' stdDeviation='1' />\n"
               "\t\t<feColorMatrix values='0 0 0 0 1  0 0 0 0 1  0 0 0 0 1  0 0 0 1 0  0 0 0 0 1' />\n"
               "\t\t<feMerge>\n"
               "\t\t\t<feMergeNode />\n"
               "\t\t\t<feMergeNode in='SourceGraphic' />\n"
               "\t\t</feMerge>\n"
               "\t</filter>\n"
               "</defs>\n\n";
}

void SvgOutput::WriteSvgFooter() {
	// Assume the file is already open
	svgFile << "\n</svg>\n";
}

void SvgOutput::DrawRimWalls( std::vector<ePolyLine> &list ) {
	int last_cr=0;
	for (unsigned int i=0; i < list.size(); ++i)
	{
		ePolyLine P = list[i];
		if (i-last_cr>=8) {
			svgFile << "\n\t";
			last_cr=i;
		}
		svgFile << P.op << P.pt.x << ',' << -P.pt.y << " ";
	}
}

void SvgOutput::DrawWalls(tList<gNetPlayerWall> &list) {
    unsigned i, len=list.Len();
    double currentTime = se_GameTime();
    bool limitedLength = gCycle::WallsLength() > 0;
    double wallsStayUpDelay = gCycle::WallsStayUpDelay();
    for(i=0; i<len; i++) {
        gNetPlayerWall *wall = list[i];
        gCycle *cycle = wall->Cycle();
        if(!cycle) continue;
        double alpha = 1;
        if(!cycle->Alive() && wallsStayUpDelay >= 0) {
            alpha -= 2 * (currentTime - cycle->DeathTime() - wallsStayUpDelay);
            if(alpha <= 0) continue;
        }
        double wallsLength = cycle->ThisWallsLength();
        double cycleDist = cycle->GetDistance();
        double minDist = limitedLength && cycleDist > wallsLength ? cycleDist - wallsLength : 0;
        const eCoord &begPos = wall->EndPoint(0), &endPos = wall->EndPoint(1);
        tArray<gPlayerWallCoord> &coords = wall->Coords();
        double begDist = wall->BegPos();
        double lenDist = wall->EndPos() - begDist;
        unsigned j, numcoords = coords.Len();
        if(numcoords < 2) continue;
        bool prevDangerous = coords[0].IsDangerous;
        double prevDist = coords[0].Pos;
        if(prevDist < minDist) prevDist = minDist;
        prevDist = (prevDist - begDist) / lenDist;
        for(j=1; j<numcoords; j++) {
            bool curDangerous = coords[j].IsDangerous;
            double curDist = coords[j].Pos;
            if(curDist < minDist) curDist = minDist;
            curDist = (curDist - begDist) / lenDist;
            if(prevDangerous) {
                eCoord v = endPos - begPos, begin = begPos + v * prevDist, end = begPos + v * curDist;
                if( (end - begin).Norm() == 0 ) continue; // if 0-length, probably not a real wall
                svgFile << "\t<path d='M" << begin.x << ',' << -begin.y
                        << ' ' << end.x << ',' << -end.y << "' stroke='" << gSvgColor(cycle->color_) << '\'';
                if(alpha != 1) {
                    svgFile << " opacity='" << alpha << '\'';
                }
                svgFile << " />\n";
            }
            prevDangerous = curDangerous;
            prevDist = curDist;
        }
    }
}

void SvgOutput::DrawObjects() {
    tList<eGameObject> const &gameObjects = eGrid::CurrentGrid()->GameObjects();
    size_t len = gameObjects.Len();
    for(size_t i = 0; i < len; ++i) {
        eGameObject *obj = gameObjects(i);
        tASSERT(obj);
        obj->DrawSvg(svgFile);
    }
}

extern std::vector<ePolyLine> se_unsplittedRimWalls;

static bool sg_cycleWallsGlow = true;
static tSettingItem<bool> sg_cycleWallsGlowConf("SVG_CYCLE_WALLS_GLOW", sg_cycleWallsGlow);

static eLadderLogWriter sg_svgOutputWriter("SVG_CREATED", true);

void SvgOutput::Create() {
    // open file as a new one
    svgFile.clear();
    if ( !tDirectories::Var().Open(svgFile, "map.svg", std::ios::trunc) ) return;
    // get map limits
    const eRectangle &bounds = eWallRim::GetBounds();
    lx = round(bounds.GetLow().x) - 5;
    ly = round(bounds.GetLow().y) - 5;
    hx = round(bounds.GetHigh().x) + 5;
    hy = round(bounds.GetHigh().y) + 5;
    w = hx - lx;
    h = hy - ly;
    cx = (lx + hx)/2;
    cy = (ly + hy)/2;

    // add header, rim wall
    WriteSvgHeader();
    svgFile << "<!-- Rim Walls -->\n<path fill = 'none' stroke='#fff' stroke-width='1' stroke-linecap='round'\n\td='";
    DrawRimWalls(se_unsplittedRimWalls);
    svgFile << "'/>\n<!-- End of Rim Walls -->\n\n";
    // keep the current file offset
    afterRimWallsPos = svgFile.tellp();
    // add player walls and other game objects
    if(sg_cycleWallsGlow) {
        svgFile << "<g filter='url(#wallsFilter)'>\n";
    }
    svgFile << "<!-- Cycle's Walls -->\n<g stroke-width='1' stroke-linecap='round'";
    svgFile << ">\n";
    DrawWalls(sg_netPlayerWallsGridded);
    DrawWalls(sg_netPlayerWalls);
    svgFile << "\n</g>\n<!-- Game objects -->\n";
    DrawObjects();
    if(sg_cycleWallsGlow) {
        svgFile << "</g>\n";
    }
    // add the footer and close the filepp
    WriteSvgFooter();
    svgFile.close();
    sg_svgOutputWriter.write();
    //con << "Svg file created\n";
}

SvgOutput::SvgOutput() {
}

SvgOutput::~SvgOutput() {
}
