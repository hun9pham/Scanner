#include "app_bsp.h"

#include "io_cfg.h"

led_t LedLife;
led_t LedFlash;
led_t LedStatus;
led_t LedDir;
led_t LedFault;

void appBspInit() {
    led_init(&LedLife, ledLifeInit, ledLifeOn, ledLifeOff);
    led_init(&LedFlash, ledFlashInit, ledFlashOn, ledFlashOff);
    led_init(&LedStatus, ledStatusInit, ledStatusOn, ledStatusOff);
    led_init(&LedDir, ledDirInit, ledDirOn, ledDirOff);
    led_init(&LedFault, ledFaultInit, ledFaultOn, ledFaultOff);
    led_off(&LedLife);
    led_off(&LedFlash);
    led_off(&LedStatus);
    led_off(&LedDir);
    led_off(&LedFault);
}