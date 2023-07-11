#include "app_data.h"

DevMngCtlStage_t devStagePolling = STAGE_IDLE;
MPU_IncomMsg_t MPU_IncomMsg;
uint32_t calibDelayScroll = 500;