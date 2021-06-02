# Arduino software

The three ATmega328p scripts are as follows.

- PCRControl

  Manages the moment to moment temperature. Control of the system is performed through serial UART communication.

- Serial Commands

  - `whoami\n` prints an identification string.
  - `d\n` request a data point form the system of the format `[Block Temperature] [PWM] [Lid Temperature]\n`.
  - `verbose\n` toggles sending a data point of every tic of the format `[Block Temperature] [PWM] [Lid Temperature]\n`.
  - `pid\n` toggles sending a data point of every tic of the format `[Target Temperature] [Block Temperature] [PWM]\n`.
  - `state\n` sends `0\n` of peltier off, `1\n` for peltier on.
  - `offl\n` Turns the lid off.
  - `offp\n` Turns the peltier off.
  - `off\n` Turns the peltier and lid off.
  - `onl\n` Turns the lid on.
  - `onp\n` Turns the peltier on.
  - `on\n` Turns the peltier and lid on.
  - `pt[value]\n` Sets the target block temperature
  - `pia[value]\n` Sets the weighting for the Block Temperature moving average. 1 is minimum.
  - `poa[value]\n` Sets the weighting for the PWM moving average. 1 is the minimum.
  - `plc[value]\n` Sets the low bound for PWM. -255 is the minimum.
  - `plh[value]\n` Sets the high bound for PWM. 255 is the maximum.

- PCRDummy

  Fakes PCRControl missing some commands. Out of data

- EfficiencyTest

  For measuring System power consumption and thermal output. Does not work with FLC-PCR PCB rev1.
