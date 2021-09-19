#ifndef CPUID_H
#define CPUID_H
#include <string>
#include <QString>

/********************************************************
 * @ClassName:  CpuId
 * @Brief:      Cpu ID获取类(未使用)
 * @date        2020/08/31
 ********************************************************/
class CpuId
{
public:
    CpuId();

    static QString get();

    static void test_1();
    static void test_2();

private:
    static bool get_cpu_id_by_asm(std::string & cpu_id);
    static void parse_cpu_id(const char * file_name, const char * match_words, std::string & cpu_id);
    static bool get_cpu_id_by_system(std::string & cpu_id);
    static bool get_cpu_id(std::string & cpu_id);

};

#endif // CPUID_H
