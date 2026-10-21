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

## 📦 Dependencies
openssl >= 3.0.0
zlib >= 1.3.2

### System packages for coverage
```bash
apt install -y build-essential lcov genhtml \
    libtimedate-perl libcapture-tiny-perl libdatetime-perl libjson-xs-perl
```

#### If your lcov version < 2.0
```bash
cd /tmp
wget https://github.com/linux-test-project/lcov/releases/download/v2.3.1/lcov-2.3.1.tar.gz
tar xfz lcov-2.3.1.tar.gz
cd lcov-2.3.1
sudo make install
```

## 🔨 Build
### Make sure that your conan profile contains Debug build type
```bash
mkdir build && cd build
conan install ..  --build=missing
cmake --preset conan-debug  .. -DENABLE_COVERAGE=ON
cd Debug && make
```

## 📊 Coverage report
```bash
cd Debug && make coverage
```

### 📝 TODO: 
 
~ add doxygen doc-strings
- check for same commands [?]
~ refactor according GStyle and CppGuide
- add const to some methods
- check arguments
- add UdpSocket statuses (?)
- m_received_block_id processing ?
- run clang-tidy
- add json-config

