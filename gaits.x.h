/*
   Copyright 2013 Barobo, Inc.

   This file is part of BaroboLink.

   Foobar is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Foobar is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

GAIT(ARCH, "Arch", Mobot_motionArchNB((mobot_t*)arg, 30*3.14/180.0))
GAIT(INCHWORMLEFT, "Inchworm Left", Mobot_motionInchwormLeftNB((mobot_t*)arg, 1))
GAIT(INCHWORMRIGHT, "Inchworm Right",Mobot_motionInchwormRightNB((mobot_t*)arg, 1)) 
GAIT(SKINNY, "Skinny Pose",Mobot_motionSkinnyNB((mobot_t*)arg, 90*3.14159/180.0)) 
GAIT(STAND, "Stand",Mobot_motionStandNB((mobot_t*)arg)) 
GAIT(TUMBLELEFT, "Tumble Left",Mobot_motionTumbleLeftNB((mobot_t*)arg, 1)) 
GAIT(TUMBLERIGHT, "Tumble Right",Mobot_motionTumbleRightNB((mobot_t*)arg, 1)) 
GAIT(UNSTAND, "Unstand",Mobot_motionUnstandNB((mobot_t*)arg)) 

