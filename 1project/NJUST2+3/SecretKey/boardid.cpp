#include "boardid.h"
#include <unistd.h>

BoardId::BoardId()
{

}

void BoardId::parse_board_serial(const char *file_name, const char *match_words, std::string &board_serial)
{
    board_serial.c_str();

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

        const char * board = strstr(line, match_words);
        if (NULL == board)
        {
            continue;
        }
        board += strlen(match_words);

        while ('\0' != board[0])
        {
            if (' ' != board[0])
            {
                board_serial.push_back(board[0]);
            }
            ++board;
        }

        if ("None" == board_serial)
        {
            board_serial.clear();
            continue;
        }

        if (!board_serial.empty())
        {
            break;
        }
    }

    ifs.close();
}

bool BoardId::get_board_serial_by_system(std::string &board_serial)
{
    board_serial.c_str();

    const char * dmidecode_result = ".dmidecode_result.txt";
    char command[512] = { 0 };
    snprintf(command, sizeof(command), "dmidecode -t 2 | grep Serial > %s", dmidecode_result);

    if (0 == system(command))
    {
        parse_board_serial(dmidecode_result, "Serial Number:", board_serial);
    }

    unlink(dmidecode_result);

    return(!board_serial.empty());
}

bool BoardId::get_board_serial_number(std::string &board_serial)
{
    if (0 == getuid())
    {
        if (get_board_serial_by_system(board_serial))
        {
            return(true);
        }
    }
    return(false);
}

void BoardId::test()
{
    std::string board_serial;
    if (get_board_serial_number(board_serial))
    {
        printf("board_serial: [%s]\n", board_serial.c_str());
    }
    else
    {
        printf("can not get board id\n");
    }
}
