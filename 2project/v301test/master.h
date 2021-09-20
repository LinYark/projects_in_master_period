#ifndef MASTER_H
#define MASTER_H

//结论，情况是这样，如果你先emit在修改false,如果中间没有什么操作，就可能会先返回了信号，再修改了false导致死锁。
//其次，如果有emit但是没有多线程，一定没有意义，这样完全是单线程。emit之后会一直等着，不会进入下一步
//然后，建议，必须要全部使用多线程，必须先修改标志位再emit
//最后，目前没有必要考虑线程的终止，因为如果是一个可以被终止的，那就相当于函数，要么就直接调用函数，大不了再开个c++类型的线程调用即可.
//最后的最后，一定不要在对象之间的槽函数链接时使用this，除了main窗口，其它的对象都不许使用this，因为这些this会在窗口类出现歧义，说到底还是一句话，qt垃圾

//情况时这样的，你不需要考虑线程的唤醒与挂起，因为这是QT4的产物，那个时候还是用run来做的多线程，主类和run不在同一线程上，所以可以在主类线程控制run线程，详情可以看看qwaitcondition与qmutex
//但是qt5官方并不推荐这个作多线程，而是用魔改或者直接全部转移的方式做多线程，不需要你挂起什么，只需要你做好事件之间的响应机制，就像人一样，没有必要一定要在某个人那里卡死。
//切记，这个技能点可以加，但是现在貌似没有必要，做好响应机制时可以避免这个问题的，到公司如果可以学就另说。可以去找同学学，不要自己学，顺便看一些鹏哥的消息队列。
//最后要注意线程的关闭，finish解deletalater


#include <QObject>
#include"common/commonheaders.h"

#include"slaver.h"
class master : public QObject
{
    Q_OBJECT
public:
    explicit master(QObject *parent = nullptr);
    void myChangeThread(){
        thread= new QThread;
        this->moveToThread(thread);
        thread->start();
    }
    void myconnect(){
        connect(this,&master::giveSlaver,s1,&slaver::giveBack);
        connect(s1,&slaver::back,this,&master::end);
    }

    void myturn(){
        while(1){
            if(flg ==true){
                flg=false;
                emit giveSlaver();
            }
            Sleep(1000);
            qDebug()<<"turn-flash";
        }
    }
    QThread *thread = new QThread;
    bool flg ;
    slaver *s1 =new slaver;

signals:
    void giveSlaver();
public slots:
    void end(){
        qDebug()<<"end-end";
        flg = true;
    }
};

#endif // MASTER_H
