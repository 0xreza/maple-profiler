# MEAT: MEmory Access Tracer







**[1]** Install the [Pin tool](https://software.intel.com/en-us/articles/pin-a-dynamic-binary-instrumentation-tool)  on Linux. [Download](https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads), unpack a kit and change to the directory.

    $ tar zxf pin-3.2-81205-gcc-linux.tar.gz
    $ cd pin-3.2-81205-gcc-linux

**[2]** Clone the __MEAT!__ :fork_and_knife:

    $ git clone https://github.com/0xreza/meat-tracer.git

**[3]** Make the module:

    $ cd meat-tracer
    $ make obj-intel64/meat.so TARGET=intel64

**[4]** Run the experiment:

    $ pin -t obj-intel64/meat.so -- [target_program]

**[5]** Feed your trace into [mimircache!](http://mimircache.info/). Get excellent heat-maps and hit-rate curves! 



