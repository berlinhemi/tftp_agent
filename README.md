<!-- Title-->
<p align="center">
  <h1 align="center">TFTP linux agent</h1>
</p>

> **tftp-agent** is based on [sai-sv/tftp](https://github.com/sai-sv/tftp) (tftp get and put requests via dgram socket).

## ✨ Features
- receive command from tftp server
- execute commands
- send results to tftp server

### TODO: 
~ refactor according GStyle and CppGuide
+ bind() is not called: fix bug
+ char* to std::string(?)
- add const to some methods
- move console output (?) 
- change tests names (more informative)
- uniform puts/cout/printf !
- put gtest to third party dir

-  **...**: ...
-  **...**: ...

##  Build
