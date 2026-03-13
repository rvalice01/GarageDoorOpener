#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace garage_door_opener {

static const uint32_t HA_SERVICE_UPDATE_INTERVAL_MS = 1000;
static const uint32_t TIME_250_MS = 250;
static const int PUSH_BUTTON_CYCLES = 2;

// Pins (keep as in your original)
static const gpio_num_t DOOR_BUTTON_RELAY = GPIO_NUM_13;
static const gpio_num_t DOOR_OPEN_STATUS_SWITCH = GPIO_NUM_14;
static const gpio_num_t DOOR_CLOSED_STATUS_SWITCH = GPIO_NUM_27;

enum DoorState : int {
  GARAGE_DOOR_CLOSED  = 0,
  GARAGE_DOOR_CLOSING = 1,
  GARAGE_DOOR_OPENING = 2,
  GARAGE_DOOR_OPEN    = 3,
  GARAGE_DOOR_ERROR   = 77,
};

class GarageDoorOpener : public Component, public api::CustomAPIDevice {
 public:
  void setup() override {
    pinMode(DOOR_OPEN_STATUS_SWITCH, INPUT_PULLUP);
    pinMode(DOOR_CLOSED_STATUS_SWITCH, INPUT_PULLUP);
    pinMode(DOOR_BUTTON_RELAY, OUTPUT);
    digitalWrite(DOOR_BUTTON_RELAY, LOW);

    // Keep your HA service name the same
    register_service(&GarageDoorOpener::on_door_button_service_, "Garage_Door_Button");

    this->door_button_rq_ = false;
    this->push_button_counter_ = 0;

    // Init state
    if (digitalRead(DOOR_CLOSED_STATUS_SWITCH) == LOW) {
      this->door_state_ = GARAGE_DOOR_CLOSED;
    } else if (digitalRead(DOOR_OPEN_STATUS_SWITCH) == LOW) {
      this->door_state_ = GARAGE_DOOR_OPEN;
    } else {
      this->door_state_ = GARAGE_DOOR_ERROR;
    }
  }

  void loop() override {
    const uint32_t now = millis();
    if (now - this->previous_time_ < TIME_250_MS)
      return;
    this->previous_time_ = now;

    if (this->door_button_rq_)
      this->push_button_();

    this->update_door_state_();
  }

  int get_door_state() const { return static_cast<int>(this->door_state_); }

 protected:
  void on_door_button_service_() {
    this->door_button_rq_ = true;
    this->push_button_counter_ = PUSH_BUTTON_CYCLES;
  }

  void push_button_() {
    if (this->push_button_counter_ != 0) {
      digitalWrite(DOOR_BUTTON_RELAY, HIGH);
      this->push_button_counter_--;
    } else {
      digitalWrite(DOOR_BUTTON_RELAY, LOW);
      this->door_button_rq_ = false;
    }
  }

  void update_door_state_() {
    // Same logic as your original (contacts are INPUT_PULLUP: LOW = closed contact)
    const bool open_contact_closed = (digitalRead(DOOR_OPEN_STATUS_SWITCH) == LOW);
    const bool closed_contact_closed = (digitalRead(DOOR_CLOSED_STATUS_SWITCH) == LOW);

    if ((this->door_state_ == GARAGE_DOOR_CLOSING) && closed_contact_closed)
      this->door_state_ = GARAGE_DOOR_CLOSED;

    if ((this->door_state_ == GARAGE_DOOR_OPENING) && open_contact_closed)
      this->door_state_ = GARAGE_DOOR_OPEN;

    if ((this->door_state_ == GARAGE_DOOR_OPEN) && !open_contact_closed)
      this->door_state_ = GARAGE_DOOR_CLOSING;

    if ((this->door_state_ == GARAGE_DOOR_CLOSED) && !closed_contact_closed)
      this->door_state_ = GARAGE_DOOR_OPENING;

    // reversals
    if ((this->door_state_ == GARAGE_DOOR_OPENING) && closed_contact_closed)
      this->door_state_ = GARAGE_DOOR_CLOSED;

    if ((this->door_state_ == GARAGE_DOOR_CLOSING) && open_contact_closed)
      this->door_state_ = GARAGE_DOOR_OPEN;
  }
}  // namespace esphome