#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <vector>
#include <string>
#include <stdio.h>
#include <cstdint>

typedef unsigned char BYTE;

/**
 * @brief UDP socket wrapper class.
 * 
 * Provides basic UDP socket operations including reading and writing datagrams.
 * Socket is automatically initialized with receive timeout of 3 seconds.
 * 
 * @note This class uses blocking receive operations with timeout.
 * Socket is not bound to a specific port - sendto() will bind implicitly.
 * 
 * @example
 * UdpSocket socket;
 * if (socket.IsInitialized()) {
 *     std::vector<BYTE> data = {'H','i'};
 *     socket.WriteDatagram(data, "192.168.1.1", 69);
 * }
 */
class UdpSocket
{
public:
    UdpSocket();

    /**
     * Receives a UDP datagram (blocking, 3s timeout).
     * @param buffer Storage for received data (resized to datagram size)
     * @param max_len Maximum bytes to read
     * @param host [out] Sender's IP address
     * @param port [out] Sender's UDP port (or nullptr if not needed)
     * @return Bytes read, or -1 on error
     */
    virtual ssize_t ReadDatagram(
        std::vector<BYTE>& buffer, 
        size_t max_len,
        std::string& host,
        uint16_t* port);
        
     /**
     * Sends a UDP datagram (non-blocking).
     * @param data Bytes to send
     * @param host Target IP address (dotted-decimal)
     * @param port Target UDP port
     * @return Bytes sent, 0 on error, -1 on sendto() error
     */    
    virtual ssize_t WriteDatagram(
        const std::vector<BYTE>& data,
        const std::string& host,
        uint16_t port);
    
    bool IsInitialized();
    ~UdpSocket();

private:

    int m_socket {-1};
    bool m_initialized {false};
    bool Init();
    void Abort();
};

#endif // UDP_SOCKET_H