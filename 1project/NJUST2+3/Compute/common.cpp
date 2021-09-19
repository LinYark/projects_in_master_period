#include "common.h"

Common::Common(QObject *parent) : QObject(parent)
{
    Tmp_SavePath = "/media/jiangnanxiangmu/江南项目专属/jiangnan0918/tmp0921/";
}

string Common::int2string(int num)
{
    ostringstream out_stream;
    out_stream << num;
    return out_stream.str();
}

string Common::num2str56(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

string Common::num2str1(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

string Common::num2str1000(double i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

void Common::setTmp_SavePath(const string &value)
{
    Tmp_SavePath = value;
}


