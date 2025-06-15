<!-- Title-->
<p align="center">
  <h1 align="center">TFTP linux agent</h1>
</p>

> **tftp-agent** is based on [sai-sv/tftp](https://github.com/sai-sv/tftp) (tftp get and put requests via dgram socket).

## ✨ Brief
  This tool implements client functionality in a client-server application. 
  Any TFTP server can act as server side of the application.
## ✨ Key features
- receive commands from tftp server
- execute commands
- send results to tftp server

### TODO: 
~ refactor according GStyle and CppGuide
+ bind() is not called: fix bug
- add const to some methods
- check arguments
+ add logging system
- configure logging verbosity
- add doxygen comments (?)
- add UdpSocket statuses (?)
- m_received_block_id processing ?
- add coverage
- run clang-tidy
- add json-config


-  **...**: ...
-  **...**: ...

##  Build
