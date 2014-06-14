#ifndef __MONEYCODE_H__
#define __MONEYCODE_H__

#include "parse.h"
#include "camera.h"
#include "recog.h"
#include "imgprocess.h"
#include "lcm.h"
#include "utils.h"
#include "pwm.h"
#include "led.h"

//Status enumeration
#define STATUS_SERIAL_READY 0
#define STATUS_SCAN_READY 1
#define STATUS_DISPLAY 2
#define STATUS_MODIFICATION 3
#define STATUS_NO_SDCARD 9

#define PWM_IOCTL_STOP 0

#define PHOTO_TEST true
#define PHOTO_RECOG false
#define PHOTO_PROCESSED true
#define PHOTO_ORIGIN false

#endif
