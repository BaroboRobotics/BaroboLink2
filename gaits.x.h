GAIT(ARCH, "Arch", Mobot_motionArchNB((mobot_t*)arg, 30*3.14/180.0))
GAIT(INCHWORMLEFT, "Inchworm Left", Mobot_motionInchwormLeftNB((mobot_t*)arg, 1))
GAIT(INCHWORMRIGHT, "Inchworm Right",Mobot_motionInchwormRightNB((mobot_t*)arg, 1)) 
GAIT(SKINNY, "Skinny Pose",Mobot_motionSkinnyNB((mobot_t*)arg, 90*3.14159/180.0)) 
GAIT(STAND, "Stand",Mobot_motionStandNB((mobot_t*)arg)) 
GAIT(TUMBLELEFT, "Tumble Left",Mobot_motionTumbleLeftNB((mobot_t*)arg, 1)) 
GAIT(TUMBLERIGHT, "Tumble Right",Mobot_motionTumbleRightNB((mobot_t*)arg, 1)) 
GAIT(UNSTAND, "Unstand",Mobot_motionUnstandNB((mobot_t*)arg)) 

