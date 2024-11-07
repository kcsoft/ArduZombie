# ArduZombie

* read up to 16 push buttons
* blink up to 16 LEDs (push button LEDs)
* up to 16 outputs
* connect to ethernet (DHCP)
* connect to MQTT server
* send mqtt message on output state change or state query
* subscribe to mqtt and output changes
* output state can be 0 = off, 1 = on, >1 = seconds till stop

### Requirements
- Arduino MEGA 2560 board with W5100 ethernet shield
- Arduino AVR Boards version 1.8.6

Libraries used:
- Ethernet by Arduino v2.0.2
- PubSubClient by Nick O'Leary v2.8.0 https://github.com/knolleary/pubsubclient
- CircularBuffer by AgileWare v1.4.0 https://github.com/rlogiacco/CircularBuffer

### Operation mode

#### Startup
- when connecing to MQTT server, send mqtt message `house/:id/start` with payload `1` (online)

#### Outputs
- can be set/toggled by mqtt message on `house/:id/light/:output` with payload `0`, `1` or number of seconds to keep output on
- can be set/toggled by button press (actions)
- on change, send mqtt message `house/:id/light/state/:output` with payload `0`, `1` or number of seconds to keep output on


#### Button has 3 functions, each can have a different action:
- short press (default action is toggle output for light X)
- medium press - 1.5 seconds (default action is toggle blink mode for button LED)
- long press - 6 seconds (default action is mqtt publish button number)


#### Actions that can be assigned to button short, medium and long press

Can be set by sending mqtt message `house/:id/set` with payload
- `asXYZ` - set action short for button X (0-F), action Y (0-5), param Z (0-F)
- `amXYZ` - set action medium for button X, action Y, param Z
- `alXYZ` - set action long for button X, action Y, param Z

eg: `as01F` - set action short for button 0 to toggle output 15 (will set output 16)

##### Actions:

- `0`: no action
- `1`: toggle output for light X
- `2`: toggle blink mode for button LED
- `3`: mqtt publish `1` on button number topic: `house/:id/button/:button`
- `4`: mqtt publish `2` on button number topic
- `5`: mqtt publish `3` on button number topic

##### On MQTT message:
  - `house/:id/light/:output` with payload `0` or `1`: set output on or off. If `:output` is `0`, turn off all outputs. If payload is a number > 1, set output on for that many seconds.
  - `house/:id` send status for all outputs that are on
  - `house/:id/reset` reset the device
  - `house/:id/set` update setting, when payload
    - `0` - reset settings to default
    - `?X` - send current settings via mqtt `house/:id/settings`, X can be i = id, h = hostname, a[sml] = actions, b = blink modes
    - `iX` - set id to `X` eg: `i1`
    - `hHOSTNAME` - set hostname to `HOSTNAME` eg: `hArdu`
    - `asXYZ` - set action short for button X (0-F), action Y (0-5), param Z (0-F)
    - `amXYZ` - set action medium for button X, action Y, param Z
    - `alXYZ` - set action long for button X, action Y, param Z
    - `bX`, X=0..4 - set blink mode


### Pinout

#### Inputs (Buttons) from 1 to 16 (PORTA, PORTC)

- multiplexed with button LED:
  - LED+ = 5V
  - LED- = pin
  - ButtonNO1 = pin
  - ButtonNO2 = GND

- Arduino PINS: 22 - 29, 37 - 30

#### Outputs (Relays) from 1 to 16 (ADC/PORTK, PORTL)

- Arduino PINS: A8 - A15, 42 - 49

### Arduino MEGA 2560 Pinout

![Arduino MEGA 2560 Pinout](/assets/arduino-mega-pinout.png)
