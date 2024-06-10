#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include "tftp_client.h"    
#include "udp_socket.h"


using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::Return;

//class MockUdpSocket: public UdpSocket
class MockUdpSocket: public UdpSocket
{
public:

    MOCK_METHOD(ssize_t, WriteDatagram, (const char *data, size_t len, const char *host, uint16_t port), (override));
    //MOCK_METHOD(ssize_t,  WriteDatagram, (const char *data, size_t len, const char *host, uint16_t port));

};

TEST(SimpleTest, check)
{
    // Создаем mock-объект, который заменит Database
    //MockUdpSocket* mock_socket = new MockUdpSocket(); 
    MockUdpSocket mock_socket;

    const std::string server_addr = "1.1.1.1";
    uint16_t port = 69;
    //TODO FIXTURE ?
    TFTPClient tftp_cli(&mock_socket, server_addr,  port);

    std::array<char, 516UL> buf;
    buf[0] = 0;
    buf[1] = static_cast<char>(TFTPClient::OpCode::RRQ);
    // filename
    const std::string fname = "data.txt";
    char *end = std::strncpy(&buf[2], fname.c_str(), fname.size()) + fname.size();
    *end++ = '\0';
    // mode
    std::string mode("octet");
    end = std::strncpy(end, mode.c_str(), mode.size()) + mode.size();
    *end++ = '\0';

    
    //EXPECT_CALL(mock_socket, WriteDatagram(&buf[0], 10, server_addr.c_str(), port));
    const auto packetSize = std::distance(&buf[0], end);
    //EXPECT_CALL(mock_socket, WriteDatagram(&buf[0], packetSize, server_addr.c_str(), port));

    //cant compare pointers (buf and server addr)!!!
    EXPECT_CALL(mock_socket, WriteDatagram(_,packetSize,_,port));

     std::cout << "RUN CODE:";
    std::cout << (int)tftp_cli.Get(fname);
     std::cout << "\n";

    //mock_socket.WriteDatagram("ff",1,"ff",44); WORKS FINE
    
    //delete mock_socket;
    //mock_socket = nullptr;
    // Макрос EXPECT_CALL позволяет описать, 
    // что должно произойти с методом make_query за время теста:
    //
    // а) метод make_query должен быть вызван строго один раз (WillOnce)
    // б) методу make_query в качестве первого аргумента передадут "query"
    // в) что передадут в качестве второго аргумента не важно
    // г) метод make_query установит второй аргумент в значение "result"
    // д) метод make_query вернет true
    
    // Запомните: это только описание того, что еще должно произойти в тесте!
    

     
    // Создаем экземпляр класса Order
    //Order<MockDatabase> order(&database); 
    
    
    // Проверяем метод check и все сделанные предположения относительно 
    // метода make_query. При этом make_query внутри метода check будет
    // вести себя так, как мы ему указали. А все предположения относительно
    // количества вызовов и аргмументов будут строго проверены.
    //order.check(100); // ... здесь начинаются реальные действия
}

int main(int argc, char** argv)
{   
    ::testing::InitGoogleTest(&argc, argv);
     ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}