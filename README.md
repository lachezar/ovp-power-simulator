# Power and execution simulator based on Open Virtual Platform for nrf51822

##Installation

The OVP is available for MS Windows and 32-bit version of Linux (with some 64-bit support). If you are a Windows user you will need Cygwin and MinGW. If you are a Linux user, try installing as much of the software as possible from the package manager. Note that you need to purchase an actual nrf51822 evaluation board to get access to the Nordic's website download section.

###Installing OVP:

1. Install GCC and GNU Make.
2. Install the GCC ARM toolchain.
3. Download and install "OVPsim Fast Simulator" and "OVPsim PSE Toolchains" from http://www.ovpworld.org/dlp/.
4. Purchase and install suitable license from http://www.ovpworld.org/likey/ by following the instructions on the page. It might take few hours to get it.

###Building the simulator:

1. Clone this repository inside the `Examples` folder of OVP's project installation.
2. Run `build_platform.{sh,bat}`.

###Installing nrf51822 SDK

1. Log in or create account in https://www.nordicsemi.com.
2. Register the nrf51822 as product if you have not done this before.
3. Go to "My Downloads".
4. Download and install "nRF518-SDK-zip", "S110-SD" and "nRF51-Tools".
5. Edit `Makefile.{posix,windows}` in `<nordic sdk>/nrf51822/Source/templates/gcc` and specify correct values for the variables. Here is an example:

```
GNU_INSTALL_ROOT := /usr
GNU_VERSION := 4.8.2
GNU_PREFIX := arm-none-eabi
```

###Building example software for nrf51822

1. Start a shell terminal and navigate to the folder containing the Makefile for the program you want to build (e.g. `<nordic sdk>/Board/pca10001/s110/ble_app_beacon/gcc`).
2. Execute `make -f <path to makefile> release`.
3. The result of the build should be located in `./_build/` folder as a file with extension "hex".
4. If the program you are building uses BLE then you need to merge the hex file with the SoftDevice containing the BLE protocol stack:

```
mergehex -m <path to the hex file> <path to the softdevice file> -o <path for the output file of the merge>
```

###Running the simulator

The simulator requires two command line parameters - path to the executable hex file to be simulated and disassembled version of the same hex file.

1. To disassemble executable file in hex format run: `arm-none-eabi-objdump -D -l -bihex -Mforce-thumb -marm <path to hex file> > application.asm`
2. Run the simulator: `./platform.{Linux32,Windows32}.exe <hex file> <assembly file> > execution.log`

The result of the simulation is an execution trace log and two CSV files with power estimations.


##Code style

OVP related models and code is written using the OVP's style - camel case everything.

The non-OVP code (e.g. radio state machine, cycles reference table) is written following the more common C code style - snake case everything. Also the non-OVP related code is using the standard types (e.g. int, unsigned int) instead of the OVP custom types (e.g. Int32, Uns32).

At the end they are mixed in some places, but it seems easier for me to differentiate between the OVP related code and the rest.
