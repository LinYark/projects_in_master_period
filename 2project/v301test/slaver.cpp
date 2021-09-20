#include "slaver.h"

slaver::slaver(QObject *parent) : QObject(parent)
{
    myChangeThread();
}
