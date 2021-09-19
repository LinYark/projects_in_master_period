#include "mainwindow.h"
#include <QApplication>
#include "Server/parameter.h"
#include "SecretKey/secretkeydialog.h"

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

    static QMutex mutex_log;
    mutex_log.lock();
    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);
    QDir dir(PP->save_path());
    if(!dir.exists()){
        dir.mkdir(PP->save_path());
    }
    QFile file(PP->save_path() + "/log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
    mutex_log.unlock();

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //注册MessageHandler
//    qInstallMessageHandler(outputMessage);            //保存日志至log文件

//    MainWindow w;
//    w.show();
//    SecretKeyDialog s;
//    s.show();

    SecretKeyDialog s;
    MainWindow w;
    if(SecretKeyDialog::key_check()){
        qDebug()<<"许可证检测通过！";
        w.show();
    }
    else{
        qDebug()<<"许可证检测失败！";
        if(s.exec() == QDialog::Accepted)
            w.show();
        else
            return 0;
    }


    return a.exec();
}
