
#pragma once

#include <stdint.h>
#include "sh1107/sh110x.hpp"
#include "defines.hpp"
#include "CAN_module.hpp"
#include "mcp2515/mcp2515.h" 

void myOLED_init(void);
void myOLED_result(Result* res, Result* throttle, Result* rpm);


