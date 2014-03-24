sm4ns3
======
This plugin interfaces Sim Mobility and ns-3. It is built as a module in ns-3, and relies on 
Sim Mobility code in entities/commsim.


Modifying ns-3
======
You need to add the following lines to ns-3's source (typically: ns-allinone-3.19/ns-3.19/wscript).
    conf.env['boost_system'] = conf.check(mandatory=True, lib='boost_system', uselib_store='BOOST_SYSTEM')
    conf.env['boost_thread'] = conf.check(mandatory=True, lib='boost_thread', uselib_store='BOOST_THREAD')
    conf.env['jsoncpp'] = conf.check(mandatory=True, lib='jsoncpp', uselib_store='JSONCPP')

Add these right *after* the following lines:
    env['APPNAME'] = wutils.APPNAME
    env['VERSION'] = wutils.VERSION

We are currently working to remove dependencies on these libraries, but for now they are necessary.


Installation instructions.
======
Perform the following:
* Download ns-3's source code (https://www.nsnam.org). This plugin was tested on "ns-allinone-3.19".
* Unzip the source code into a directory. Now, cd into that directory (cd ns-allinone-3.19/ns-3.19/src).
* Checkout this repository there (git clone https://github.com/smart-fm/sm4ns3.git).
* Change back to the ns-3 directory: (cd ns-allinone-3.19/ns-3.19)
* Configure, using ns-3's "waf" tool. You will need examples enabled. (./waf configure --enable-examples)
* Build, using ns-3's "waf" tool. (./waf)
* Run, using ns-3's "waf" tool. (./waf --run "sm4ns3-example --application=Default")


Running a Trace File (Baseline)
======
You can get a baseline performance for the ns-3 component alone by running the following:

./waf --run "sm4ns3-trace --trace=/tmp/full_trace_100k_with_time.txt --output=out.txt --nAgent=500"

This will also display the total runtime (minus the time spent loading the trace file). 

