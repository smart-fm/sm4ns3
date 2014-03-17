sm4ns3
======
This plugin interfaces Sim Mobility and ns-3. It is built as a module in ns-3, and relies on 
Sim Mobility code in entities/commsim.

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




