<!-- Title-->
<p align="center">
  <h1 align="center">TFTP linux agent</h1>
</p>

> **tftp-agent** is based on [sai-sv/tftp](https://github.com/sai-sv/tftp) (tftp get and put requests via dgram socket).

## ✨ Brief
  This tool implements client functionality in a client-server application. 
  Any TFTP server can act as server side of the application.
## ✨ Key features
- receive shell commands from tftp server
- execute commands
- send results to tftp server

## Dependencies
openssl >= 3.0.0

## Build
*  Make sure that your conan profile contains Debug build type
* `mkdir build && cd build`
* `conan install ..  --build=missing`
* `cmake --preset conan-debug  ..`
* `cd Debug && make`

### TODO: 
- add class doc-strings
- add readable statuses
~ add verbose output
- check for same commands !
~ refactor according GStyle and CppGuide
- add const to some methods
- check arguments
- add doxygen comments (?)
- add UdpSocket statuses (?)
- m_received_block_id processing ?
- add coverage
- run clang-tidy
- add json-config


-  **...**: ...
-  **...**: ...

##  Build
