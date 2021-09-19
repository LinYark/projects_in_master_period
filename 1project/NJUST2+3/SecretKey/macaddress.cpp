#include "macaddress.h"

MacAddress::MacAddress()
{

}

QString MacAddress::getHostMacAddress()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();
    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }
    return strMacAddr;
}

bool MacAddress::get_mac_address_by_ioctl(std::string &mac_address)
{
    mac_address.clear();

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return(false);
    }

    struct ifreq ifr = { 0 };
    strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name) - 1);
    bool ret = (ioctl(sock, SIOCGIFHWADDR, &ifr) >= 0);

    close(sock);

    const char hex[] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    char mac[16] = { 0 };
    for (int index = 0; index < 6; ++index)
    {
        size_t value = ifr.ifr_hwaddr.sa_data[index] & 0xFF;
        mac[2 * index + 0] = hex[value / 16];
        mac[2 * index + 1] = hex[value % 16];
    }
    std::string(mac).swap(mac_address);

    return(ret);
}

void MacAddress::parse_mac_address(const char *file_name, const char *match_words, std::string &mac_address)
{
    mac_address.c_str();

    std::ifstream ifs(file_name, std::ios::binary);
    if (!ifs.is_open())
    {
        return;
    }

    char line[4096] = { 0 };
    while (!ifs.eof())
    {
        ifs.getline(line, sizeof(line));
        if (!ifs.good())
        {
            break;
        }

        const char * mac = strstr(line, match_words);
        if (NULL == mac)
        {
            continue;
        }
        mac += strlen(match_words);

        while ('\0' != mac[0])
        {
            if (' ' != mac[0] && ':' != mac[0])
            {
                mac_address.push_back(mac[0]);
            }
            ++mac;
        }

        if (!mac_address.empty())
        {
            break;
        }
    }

    ifs.close();
}

bool MacAddress::get_mac_address_by_system(std::string &mac_address)
{
    mac_address.c_str();

    const char * lshw_result = ".lshw_result.txt";
    char command[512] = { 0 };
    snprintf(command, sizeof(command), "lshw -c network | grep serial | head -n 1 > %s", lshw_result);

    if (0 == system(command))
    {
        parse_mac_address(lshw_result, "serial:", mac_address);
    }

    unlink(lshw_result);

    return(!mac_address.empty());
}

bool MacAddress::get_mac_address(std::string &mac_address)
{
    if (get_mac_address_by_ioctl(mac_address))
    {
        return(true);
    }
    if (get_mac_address_by_system(mac_address))
    {
        return(true);
    }
    return(false);
}

void MacAddress::test_1()
{
    std::string mac_address;
    if (get_mac_address(mac_address))
    {
        printf("mac_address: [%s]\n", mac_address.c_str());
    }
    else
    {
        printf("can not get mac address\n");
    }
}

void MacAddress::test_2()
{
    {
        std::string mac_address;
        if (get_mac_address_by_ioctl(mac_address))
        {
            printf("mac_address: [%s]\n", mac_address.c_str());
        }
        else
        {
            printf("can not get mac address\n");
        }
    }
    {
        std::string mac_address;
        if (get_mac_address_by_system(mac_address))
        {
            printf("mac_address: [%s]\n", mac_address.c_str());
        }
        else
        {
            printf("can not get mac address\n");
        }
    }
}
