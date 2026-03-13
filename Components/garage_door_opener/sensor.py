import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import UNIT_EMPTY, ICON_GATE, STATE_CLASS_MEASUREMENT

garage_ns = cg.esphome_ns.namespace("garage_door_opener")
GarageDoorOpener = garage_ns.class_("GarageDoorOpener", cg.Component, cg.CustomAPIDevice)
DoorStateSensor = garage_ns.class_("DoorStateSensor", sensor.Sensor, cg.PollingComponent)

CONF_DOOR_STATE = "door_state"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(GarageDoorOpener),

    cv.Optional(CONF_DOOR_STATE): sensor.sensor_schema(
        unit_of_measurement=UNIT_EMPTY,
        icon=ICON_GATE,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend({
        cv.GenerateID("door_state_sensor_id"): cv.declare_id(DoorStateSensor),
        cv.Optional("update_interval", default="1s"): cv.update_interval,
    }),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    opener = cg.new_Pvariable(config[cv.CONF_ID])
    await cg.register_component(opener, config)

    if CONF_DOOR_STATE in config:
        sconf = config[CONF_DOOR_STATE]
        sens = cg.new_Pvariable(sconf["door_state_sensor_id"], opener)
        await sensor.register_sensor(sens, sconf)
        await cg.register_component(sens, sconf)
        cg.add(sens.set_update_interval(sconf["update_interval"]))