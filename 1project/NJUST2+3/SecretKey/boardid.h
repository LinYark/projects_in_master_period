#ifndef BOARDID_H
#define BOARDID_H
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <fstream>

/********************************************************
 * @ClassName:  BoardId
 * @Brief:      主板ID获取类(未使用)
 * @date        2020/08/31
 ********************************************************/
class BoardId
{
public:
    BoardId();


    static void test();

private:

    static void parse_board_serial(const char * file_name, const char * match_words, std::string & board_serial);

    static bool get_board_serial_by_system(std::string & board_serial);

    static bool get_board_serial_number(std::string & board_serial);

};

#endif // BOARDID_H
