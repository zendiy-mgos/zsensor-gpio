load('api_zsensor.js');
load('api_gpio.js');

ZenSensor.GPIO = {
  _att: ffi('bool mgos_zsensor_gpio_attach(void *, int, void *)'),
  _atta: ffi('bool mjs_zsensor_gpio_adc_attach(void *, int)'),
  _atti: ffi('bool mgos_zsensor_gpio_int_attach(void *, bool)'),
  _cfgc: ffi('void *mjs_zsensor_gpio_cfg_create(bool, int)'),
  
  _proto: {
    _sensor: null,
    _getHandle: function() {
      return this._sensor.handle;
    },

    // ## **`object.GPIO.attach(pin, cfg)`**
    //
    // Example:
    // ```javascript
    // if (btn.GPIO.attach(4, {activeHigh: false, pullType: GPIO.PULL_UP})) {
    //   /* success */
    // } else {
    //   /* error */
    // }
    // ```
    attach: function(pin, cfg) {
      let cfgo = null;
      if (cfg) {
        cfgo = ZenSensor.GPIO._cfgc(
          ZenThing._getSafe(cfg.activeHigh, true),
          ZenThing._getSafe(pullType, -1)
        );
        if (cfgo === null) return null;
      }
      let result = ZenSensor.GPIO._att(this._getHandle(), pin, cfgo);
      ZenThing._free(cfgo);
      return result;
    },

    attachAdc: function(pin) {
      return ZenSensor.GPIO._atta(this._getHandle(), pin);
    },
    attachInt: function(activeHigh) {
      return ZenSensor.GPIO._atti(this._getHandle(),
        ZenThing._getSafe(activeHigh, true));
    },
  }
};

ZenThing._onCreateSub(function(obj) {
  if ((obj.type & ZenThing.TYPE_SENSOR) === ZenThing.TYPE_SENSOR) {
    obj.GPIO = Object.create(ZenSensor.GPIO._proto);
    obj.GPIO._sensor = obj;
  }
});