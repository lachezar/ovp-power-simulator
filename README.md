OVP-Experiments
===============

The code is mess right now, but soon I will know which parts to cut out and which should remain. Then I will move it to the main repository.

#Code style

OVP related models and code is written using the OVP's style - camel case everything. \

The non-OVP code (e.g. radio state machine, cycles reference table) is written following the more common C code style - snake case everything. Also the non-OVP related code is using the standard types (e.g. int, unsigned int) instead of the OVP custom types (e.g. Int32, Uns32).

At the end they are mixed in some places, but it seems easier for me to differentiate between the OVP related code and the rest.
