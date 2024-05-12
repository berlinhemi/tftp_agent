#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tftp_client.h"    
#include "udp_socket.h"


using ::testing::_;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::Return;

class MockUdpSocet: public UdpSocket
{
public:

    //MOCK_METHOD(ssize_t, WriteDatagram, (const char *data, size_t len, const char *host, uint16_t port), (override));
        MOCK_METHOD4( WriteDatagram, ssize_t(const char *data, size_t len, const char *host, uint16_t port));

};

TEST(SimpleTest, check)
{
    // Создаем mock-объект, который заменит Database
    MockUdpSocet mock_socket; 
    
    // Макрос EXPECT_CALL позволяет описать, 
    // что должно произойти с методом make_query за время теста:
    //
    // а) метод make_query должен быть вызван строго один раз (WillOnce)
    // б) методу make_query в качестве первого аргумента передадут "query"
    // в) что передадут в качестве второго аргумента не важно
    // г) метод make_query установит второй аргумент в значение "result"
    // д) метод make_query вернет true
    
    // Запомните: это только описание того, что еще должно произойти в тесте!
    
    // EXPECT_CALL(mock_socket, WriteDatagram("some_data", 10, "1.1.1.1", 5555))
    //      .WillOnce(DoAll(Return(100)));
    
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