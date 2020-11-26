#include "mgos.h"
#include "mgos_gpio.h"
#include "mgos_adc.h"
#include "mgos_zsensor_gpio.h"

enum mg_zsensor_gpio_type {
  MG_ZSENSOR_GPIO_TYPE_STD,
  MG_ZSENSOR_GPIO_TYPE_ADC,
  MG_ZSENSOR_GPIO_TYPE_INT
};

struct mg_zsensor_gpio_ext {
  struct mgos_zsensor *handle;
  int pin;
  enum mg_zsensor_gpio_type type;
  void *cfg;
};

bool mg_zsensor_gpio_cfg_set(struct mgos_zsensor_gpio_cfg *cfg_src,
                             struct mgos_zsensor_gpio_cfg *cfg_dest) {
  if (!cfg_dest) return false;
  if (cfg_src != NULL) {
    cfg_dest->active_high = cfg_src->active_high;
    cfg_dest->pull_type = (cfg_src->pull_type == -1 ? MGOS_GPIO_PULL_UP : cfg_src->pull_type);
  } else {
    cfg_dest->active_high = true;
    cfg_dest->pull_type = MGOS_GPIO_PULL_UP;
  }
  return true;
}

bool mg_zsensor_gpio_adc_cfg_set(struct mgos_zsensor_gpio_adc_cfg *cfg_src,
                                 struct mgos_zsensor_gpio_adc_cfg *cfg_dest) {
  if (!cfg_dest) return false;
  if (cfg_src != NULL) {
    cfg_dest->is_voltage = cfg_src->is_voltage;
  } else {
    cfg_dest->is_voltage = false;
  }
  return true;
}

bool mg_zsensor_gpio_state_h(enum mgos_zthing_state_act act,
                             struct mgos_zsensor_state *state,
                             void *user_data) {
  struct mg_zsensor_gpio_ext *ext = (struct mg_zsensor_gpio_ext *)user_data;
  struct mgos_zsensor_gpio_cfg *cfg = (struct mgos_zsensor_gpio_cfg *)ext->cfg;

  if (act == MGOS_ZTHING_STATE_GET) {
    bool gpio_val = mgos_gpio_read(ext->pin);
    if (cfg->active_high) {
      mgos_zvariant_bool_set(state->value, (gpio_val ? true : false));
    } else {
      mgos_zvariant_bool_set(state->value, (gpio_val ? false : true));
    }
    return true;
  }
  return false;
}

bool mg_zsensor_gpio_adc_state_h(enum mgos_zthing_state_act act,
                                 struct mgos_zsensor_state *state,
                                 void *user_data) {
  struct mg_zsensor_gpio_ext *ext = (struct mg_zsensor_gpio_ext *)user_data;

  if (act == MGOS_ZTHING_STATE_GET) {
#if CS_PLATFORM == CS_P_ESP8266
    mgos_zvariant_integer_set(state->value, mgos_adc_read(ext->pin));
#else
    struct mgos_zsensor_gpio_adc_cfg *cfg = (struct mgos_zsensor_gpio_adc_cfg *)ext->cfg;
    mgos_zvariant_integer_set(state->value,
      (cfg->is_voltage ? mgos_adc_read_voltage(ext->pin) : mgos_adc_read(ext->pin)));
#endif //CS_PLATFORM == CS_P_ESP8266
    return true;
  }
  return false;
}

bool mg_zsensor_gpio_ext_init(struct mg_zsensor_gpio_ext *ext) {
  bool success = mgos_zsensor_state_handler_set(ext->handle,
    (ext->type == MG_ZSENSOR_GPIO_TYPE_ADC ? mg_zsensor_gpio_adc_state_h : mg_zsensor_gpio_state_h), ext); 

  if (success) {
    if (ext->type == MG_ZSENSOR_GPIO_TYPE_STD) {
      struct mgos_zsensor_gpio_cfg *cfg = (struct mgos_zsensor_gpio_cfg *)ext->cfg;
      if (mgos_gpio_setup_input(ext->pin, cfg->pull_type)) return true;
    } else if (ext->type == MG_ZSENSOR_GPIO_TYPE_ADC) {
      if (mgos_adc_enable(ext->pin)) return true;
    } else if (ext->type == MG_ZSENSOR_GPIO_TYPE_INT) {
      return true;
    }
  } else { 
    LOG(LL_ERROR, ("Error registering state handler for sensor '%s'.", ext->handle->id));
  }

  LOG(LL_ERROR, ("Error initializing GPIO %d for sensor '%s'.", ext->pin, ext->handle->id));
  return false;
}

bool mg_zsensor_gpio_x_attach(struct mgos_zsensor *handle, int pin,
                              enum mg_zsensor_gpio_type type, void *cfg) {
  if (handle == NULL || pin < 0) return false;
  struct mg_zsensor_gpio_ext *e = calloc(1, sizeof(struct mg_zsensor_gpio_ext));
  if (e != NULL) {
    e->handle = handle;
    e->pin = pin;
    e->type = type;
    e->cfg = NULL;

    bool success = false;
    if (e->type == MG_ZSENSOR_GPIO_TYPE_ADC) {
      e->cfg = calloc(1, sizeof(struct mgos_zsensor_gpio_adc_cfg));
      success = mg_zsensor_gpio_adc_cfg_set((struct mgos_zsensor_gpio_adc_cfg *)cfg, (struct mgos_zsensor_gpio_adc_cfg *)e->cfg);
    } else if (e->type == MG_ZSENSOR_GPIO_TYPE_STD || e->type == MG_ZSENSOR_GPIO_TYPE_INT) {
      e->cfg = calloc(1, sizeof(struct mgos_zsensor_gpio_cfg));
      success = mg_zsensor_gpio_cfg_set((struct mgos_zsensor_gpio_cfg *)cfg, (struct mgos_zsensor_gpio_cfg *)e->cfg);
    }

    if (success && mg_zsensor_gpio_ext_init(e) &&
        mgos_zthing_ext_attach(MGOS_ZTHING_CAST(handle), "zsensor-gpio", e)) {
      LOG(LL_INFO, ("Sensor '%s' successfully attacched to GPIO pin %d.", e->handle->id, e->pin));
      return true;
    }

    free(e->cfg);
    free(e);
    e = NULL;
  }
  
  return (e != NULL);
}

bool mgos_zsensor_gpio_attach(struct mgos_zsensor *handle, int pin,
                              struct mgos_zsensor_gpio_cfg *cfg) {
  return mg_zsensor_gpio_x_attach(handle, pin, MG_ZSENSOR_GPIO_TYPE_STD, cfg);
}

#if CS_PLATFORM == CS_P_ESP8266
bool mgos_zsensor_gpio_adc_attach(struct mgos_zsensor *handle, int pin) {
  return mg_zsensor_gpio_x_attach(handle, pin, MG_ZSENSOR_GPIO_TYPE_ADC, NULL);
}
#else
bool mgos_zsensor_gpio_adc_attach(struct mgos_zsensor *handle, int pin,
                                  struct mgos_zsensor_gpio_adc_cfg *cfg) {
  return mg_zsensor_gpio_x_attach(handle, pin, MG_ZSENSOR_GPIO_TYPE_ADC, cfg);
}
#endif //CS_PLATFORM == CS_P_ESP8266

bool mgos_zsensor_gpio_int_attach(struct mgos_zsensor *handle, bool active_high) {
  struct mgos_zsensor_int_cfg int_cfg;
  if (mgos_zsensor_int_cfg_get(handle, &int_cfg)) {
    struct mgos_zsensor_gpio_cfg cfg;
    cfg.active_high = active_high;
    cfg.pull_type = MGOS_GPIO_PULL_NONE;
    return mg_zsensor_gpio_x_attach(handle, int_cfg.pin, MG_ZSENSOR_GPIO_TYPE_INT, &cfg);
  }
  return false;
}

#ifdef MGOS_HAVE_MJS

struct mgos_zsensor_gpio_cfg *mjs_zsensor_gpio_cfg_create(bool active_high,
                                                          enum mgos_gpio_pull_type pull_type) {
  struct mgos_zsensor_gpio_cfg cfg_src = {
    active_high,
    pull_type
  };
  struct mgos_zsensor_gpio_cfg *cfg_dest = calloc(1, sizeof(struct mgos_zsensor_gpio_cfg));
  if (mg_zsensor_gpio_cfg_set(&cfg_src, cfg_dest)) return cfg_dest;
  free(cfg_dest);
  return NULL;
}

bool mjs_zsensor_gpio_adc_attach(struct mgos_zsensor *handle, int pin) {
#if CS_PLATFORM == CS_P_ESP8266
  return mgos_zsensor_gpio_adc_attach(handle, pin);
#else
  return mgos_zsensor_gpio_adc_attach(handle, pin, NULL);
#endif //CS_PLATFORM == CS_P_ESP8266
}

#endif /* MGOS_HAVE_MJS */


bool mgos_zsensor_gpio_init() { 
  return true;
}