

## Hardware

The T2 Micros are connected via bluetooth, and should be named via the AT+NAME command to be SAMMONS_# where # varies.

Devices should be labeled (with a sharpie?) to help clarify what does what.

Code that interfaces with bluetooth is flakey, but CLIs exist, so there are some prereqs that will need to be installed for our code to work:

- `apt install blueman`
- 