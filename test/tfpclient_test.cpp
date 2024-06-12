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

    MOCK_METHOD(ssize_t, 
    WriteDatagram, (const std::vector<BYTE>& buf,const std::string& host, uint16_t port), 
    (override));
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

    std::vector<BYTE> buf;
    buf.push_back(0);
    buf.push_back(static_cast<char>(TFTPClient::OpCode::RRQ));
   
    // filename
    const std::string fname = "data.txt";
    buf.insert(buf.end(), fname.begin(), fname.end());
    buf.push_back(0);
    
    // mode
    std::string mode("octet");
    buf.insert(buf.end(), mode.begin(), mode.end());
    buf.push_back(0);

    
    //EXPECT_CALL(mock_socket, WriteDatagram(&buf[0], 10, server_addr.c_str(), port));
    //const auto packet_size = std::distance((char*)&buf[0], end);
    //EXPECT_CALL(mock_socket, WriteDatagram(&buf[0], packetSize, server_addr.c_str(), port));

    //cant compare pointers (buf and server addr)!!!
    EXPECT_CALL(mock_socket, WriteDatagram(buf, server_addr, port));

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