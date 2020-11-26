# ZenSensor GPIO
## Overview
Mongoose-OS library for attaching a [ZenSensor](https://github.com/zendiy-mgos/zsensor) to a GPIO value.
## GET STARTED
Build up your own device in few minutes just starting from the following sample. Start including following libraries into your `mos.yml` file.
```yaml
libs:
  - origin: https://github.com/zendiy-mgos/zsensor-gpio
```
**C/C++ sample code**
```c
#include "mgos.h"
#include "mgos_zsensor_gpio.h"

void mg_zsensor_state_updated_cb(int ev, void *ev_data, void *userdata) {
  struct mgos_zsensor_state_upd *state = (struct mgos_zsensor_state_upd *)ev_data;
  struct mgos_zsensor *handle = state->handle;

  bool cur_val = MGOS_ZVARIANT_PTR_CAST(state->value, bool);
  if(mgos_zvariant_is_nav(state->prev_value)) {
    LOG(LL_INFO, ("Updating '%s' value to '%s'(%d)", handle->id, 
      mgos_zsensor_state_name_by_val(handle, cur_val), cur_val));
  } else {
    bool prev_val = MGOS_ZVARIANT_PTR_CAST(state->prev_value, bool);
    LOG(LL_INFO, ("Updating '%s' value from '%s'(%d) to '%s'(%d)", handle->id, 
      mgos_zsensor_state_name_by_val(handle, prev_val), prev_val,
      mgos_zsensor_state_name_by_val(handle, cur_val), cur_val));
  }

  (void) ev;
  (void) userdata;
}

enum mgos_app_init_result mgos_app_init(void) {
  struct mgos_zsensor *sens1 = mgos_zsensor_create(mgos_sys_config_get_app_sens1_id(),
    ZSENSOR_TYPE_BINARY, NULL);

  if (sens1) {
    mgos_zsensor_state_name_set(sens1, false, "UP");
    mgos_zsensor_state_name_set(sens1, true, "DOWN");
    mgos_event_add_handler(MGOS_EV_ZTHING_STATE_UPDATED, mg_zsensor_state_updated_cb, NULL);

    int pin =  mgos_sys_config_get_app_sens1_gpio_pin();
    if (mgos_zsensor_int_set(sens1, pin, MGOS_GPIO_PULL_UP, MGOS_GPIO_INT_EDGE_ANY, 50)) {
      if (mgos_zsensor_gpio_int_attach(sens1, false)) {
         return MGOS_APP_INIT_SUCCESS;
      }
    }
  }

  return MGOS_APP_INIT_ERROR;
}
```
**JavaScript sample code**
```js
load("api_zsensor_gpio.js")

/* Create sensor using defualt configuration. */   
let sens1 = ZenSensor.create('sens1', ZenSensor.TYPE.BINARY);

if (sens1) {
  sens1.setStateName(false, 'UP');
  sens1.setStateName(true, 'DOWN');

  if (sens1.setInt(14, GPIO.PULL_UP, GPIO.INT_EDGE_ANY, 50)) {
    if (sens1.GPIO.attachInt(false)) {
      
      sens1.onStateUpd(function(state, ud) {
        let sens = state.thing;
        let curVal = ZenVar.bool(state.value);
        if (ZenVar.isNaV(state.prevValue)) {
          print('Updating', sens.id,
            "to", sens.getStateNameByVal(curVal), "(", curVal, ")");    
        } else {
          let prevVal = ZenVar.bool(state.prevValue);
          print('Updating', sens.id,
            'from', sens.getStateNameByVal(prevVal), "(", prevVal, ")",
            "to", sens.getStateNameByVal(curVal), "(", curVal, ")");
        }
      }, null);

    }
  }
}
```
## C/C++ API Reference
### mgos_zsensor_gpio_cfg
```c
struct mgos_zsensor_gpio_cfg {
  bool active_high;
};
```
GPIO configuration settings (e.g.: used by `mgos_zsensor_gpio_attach()`).

|Field||
|--|--|
|active_high|Set to `true` if the GPIO input is high (1) when the button is pressed.|

**Example** - Create and initialize configuration settings.
```c
// create and initialize cfg using defaults
struct mgos_zsensor_gpio_cfg cfg = MGOS_ZBUTTON_GPIO_CFG;
```
### mgos_zsensor_gpio_attach()
```c
bool mgos_zsensor_gpio_attach(struct mgos_zsensor *handle, int pin, struct mgos_zsensor_gpio_cfg *cfg);
```
Attaches the button to the GPIO. Returns `true` on success, `false` otherwise.

|Parameter||
|--|--|
|handle|Button handle.|
|pin|GPIO pin.|
|cfg|Optional. [GPIO configuration](https://github.com/zendiy-mgos/zsensor-gpio#mgos_zsensor_gpio_cfg). If `NULL`, default configuration values are used.|
### mgos_zsensor_gpio_detach()
```c
bool mgos_zsensor_gpio_detach(struct mgos_zsensor *handle);
```
Detaches the button from the GPIO that was previously attached using `mgos_zsensor_gpio_attach()`. Returns `true` on success, `false` otherwise.

|Parameter||
|--|--|
|handle|Button handle.|
## JS API Reference
### .GPIO.attach()
```js
let success = btn.GPIO.attach(pin, cfg);
```
Attaches the button to the GPIO. Returns `true` on success, `false` otherwise.

|Parameter|Type||
|--|--|--|
|pin|numeric|GPIO pin.|
|cfg|object|Optional. GPIO configuration. If missing, default configuration values are used. For more details, see 'GPIO configuration properties' section below.<br><br>{&nbsp;activeHigh: true&nbsp;}|

**GPIO configuration properties**
|Property|Type||
|--|--|--|
|activeHigh|boolean|Optional. Set to `true` if the GPIO input is high (1) when the button is pressed. Default value `true`.|
### .GPIO.detach()
```js
let success = btn.GPIO.detach();
```
Detaches the button from the GPIO that was previously attached using `.GPIO.attach()`. Returns `true` on success, `false` otherwise.
## Additional resources
Take a look to some other samples or libraries.

|Reference|Type||
|--|--|--|
|[zsensor-mqtt](https://github.com/zendiy-mgos/zsensor-mqtt)|Library|Mongoose-OS library for publishing ZenButtons events as MQTT messages.|
|[zsensor-mqtt-demo](https://github.com/zendiy-mgos/zsensor-mqtt-demo)|Firmware|Mongoose-OS demo firmware that uses ZenButtons ecosystem for publishing pushbutton events as MQTT messages.|