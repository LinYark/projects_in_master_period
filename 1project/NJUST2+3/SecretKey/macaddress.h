#ifndef MACADDRESS_H
#define MACADDRESS_H
#include <QNetworkInterface>
#include <QList>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <fstream>

/********************************************************
 * @ClassName:  MacAddress
 * @Brief:      Mac地址获取类
 * @date        2020/08/31
 ********************************************************/
class MacAddress
{
public:
    MacAddress();

    static QString getHostMacAddress();

    static void test_1();
    static void test_2();
private:

    static bool get_mac_address_by_ioctl(std::string & mac_address);

    static void parse_mac_address(const char * file_name, const char * match_words, std::string & mac_address);

    static bool get_mac_address_by_system(std::string & mac_address);

    static bool get_mac_address(std::string & mac_address);


};

#endif // MACADDRESS_H
