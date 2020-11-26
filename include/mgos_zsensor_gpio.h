/*
 * Copyright (c) 2020 ZenDIY
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MGOS_ZSENSOR_GPIO_H_
#define MGOS_ZSENSOR_GPIO_H_

#include <stdio.h>
#include <stdbool.h>
#include "mgos_zsensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MGOS_ZSENSOR_GPIO_CFG {true, MGOS_GPIO_PULL_UP} 
struct mgos_zsensor_gpio_cfg {
  bool active_high;
  enum mgos_gpio_pull_type pull_type;
};

#define MGOS_ZSENSOR_GPIO_ADC_CFG {false} 
struct mgos_zsensor_gpio_adc_cfg {
  bool is_voltage;
};

bool mgos_zsensor_gpio_attach(struct mgos_zsensor *handle, int pin,
                              struct mgos_zsensor_gpio_cfg *cfg);

#if CS_PLATFORM == CS_P_ESP8266
bool mgos_zsensor_gpio_adc_attach(struct mgos_zsensor *handle, int pin);
#else
bool mgos_zsensor_gpio_adc_attach(struct mgos_zsensor *handle, int pin,
                                  struct mgos_zsensor_gpio_adc_cfg *cfg);
#endif //CS_PLATFORM == CS_P_ESP8266

bool mgos_zsensor_gpio_int_attach(struct mgos_zsensor *handle, bool active_high);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MGOS_ZSENSOR_GPIO_H_ */
