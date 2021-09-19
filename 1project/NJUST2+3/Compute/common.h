#ifndef COMMON_H
#define COMMON_H

#include <QObject>
using namespace std;
using namespace cv;

struct point4d
{
    double x;
    double y;
    double z;
    double indexGongjian;
};

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = nullptr);

    static string int2string(int num);
    static string num2str56(int i);    
    static string num2str1(int i);
    static string num2str1000(double i);
    void setTmp_SavePath(const string &value);

signals:

public slots:

private:
    string Tmp_SavePath;

};

#endif // COMMON_H
