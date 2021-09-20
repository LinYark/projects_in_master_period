#include "master.h"

master::master(QObject *parent) : QObject(parent)
{
    myChangeThread();
}
