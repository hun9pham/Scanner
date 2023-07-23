#include "app_data.h"

DevMngCtlStage_t devStagePolling = STAGE_IDLE;
MPU_IncomMsg_t MPU_IncomMsg;
UserSetting usrAdjust = {
    .magicNum = 0,
    .delayVal = 500
};