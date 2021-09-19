#include "secretkeydialog.h"
#include "ui_secretkeydialog.h"
#include "cpuid.h"
#include "macaddress.h"
#include <QMessageBox>
#include "Server/parameter.h"

SecretKeyDialog::SecretKeyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SecretKeyDialog)
{
    ui->setupUi(this);
    ui->plainTextEdit_hardwar_code->appendPlainText(hardware_code_gen());
}

SecretKeyDialog::~SecretKeyDialog()
{
    delete ui;
}

/*
 * @Brief:  秘钥检测,开机时使用
 * @Return: bool,TRUE为检测通过,FALSE为不通过
 */
bool SecretKeyDialog::key_check()
{
    if(key_gen(hardware_code_gen()) == PP->key_code())
        return true;
    else
        return false;
}

/*
 * @Brief:  响应[注册]按钮,并检测输入秘钥是否正确
 * @Return: NULL
 */
void SecretKeyDialog::on_pbt_register_clicked()
{
    //    this->setResult(QDialog::Accepted);
    if(key_gen(ui->plainTextEdit_hardwar_code->toPlainText()) == ui->plainTextEdit_key->toPlainText()){
        PP->setKey_code(ui->plainTextEdit_key->toPlainText());
        this->accept();
    }
    else{
        QMessageBox::warning(this, tr("密钥错误"),
                             tr("输入的密钥不正确，无法完成注册!\n"
                                "请向您的软件提供商索要正确的密钥！"),
                             QMessageBox::Ok);
    }
}

/*
 * @Brief:  响应[放弃]按钮
 * @Return: NULL
 */
void SecretKeyDialog::on_pbt_giveup_clicked()
{
    //    this->setResult(QDialog::Accepted);
    this->reject();
}

/*
 * @Brief:
 * @Param:  code,硬件码
 * @Return: QString,生成的秘钥
 */
QString SecretKeyDialog::key_gen(QString code)
{
    QByteArray byteArray = code.toLatin1();
    QCryptographicHash hash(QCryptographicHash::Keccak_256);
    hash.addData(byteArray);  // 添加数据到加密哈希值
    QByteArray result = hash.result();  // 返回最终的哈希值
    QString strMD5 = result.toHex().toUpper();
    return strMD5;
}

/*
 * @Brief:  硬件码生成,可选生成方式有很多,但是目前只使用了Mac地址的方式
 * @Return: QString,生成的硬件码
 */
QString SecretKeyDialog::hardware_code_gen()
{
    //ok半小时
//    QString code1 = CpuId::get();
    QString code1 = MacAddress::getHostMacAddress();
    QString result1 = key_gen(code1);
    QString code2 = code1 + "NJUST";
    QString result2 = key_gen(code2);
    return result1 + result2;
}
