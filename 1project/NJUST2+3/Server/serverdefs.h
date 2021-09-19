#ifndef SERVERDEFS_H
#define SERVERDEFS_H

#pragma pack(push,1)//设定为1字节对齐
struct MessageHeader
{
     qint32 sig;    // 消息标志，固定为0x59535953
     qint16 type;   // 消息类型
     qint32 length; // 消息总长度（包括消息头）
 };

struct LongmenData
{
    MessageHeader header;
    qint16 commandtype;
    double dLongmenPosX;
};


struct SpfJsonData
{
    MessageHeader header;
    qint16 commandtype;
    qint32 sumLength;
    char chContent[1000]; // 部件ID
};


struct WeldSeamData
{
    MessageHeader header;
    qint16 commandtype;
    char partId[50] ;      // 部件ID
    qint16 seamQuantity;  // 轮廓路径点数量
    char seamId[50];       // 轮廓ID
    qint16 pointQuantity;  // 轮廓路径点数量
    qint64 *points;        // 轮廓路径点
};

struct weld_same_data
{
    char partId[50] ;      // 部件ID
    qint16 seamQuantity;  // 轮廓路径点数量
    char seamId[50];       // 轮廓ID
    qint16 pointQuantity;  // 轮廓路径点数量
    qint64 *points;        // 轮廓路径点
};


struct ControlData
{
   MessageHeader header;
   qint16 commandtype;
};

struct ShutdownData
{
    MessageHeader header;
    qint16 commandtype;
};
#pragma pack(pop)//恢复对齐状态

//sig
#define MSG_SIGNATURE  0x59535953 // 消息标志

//type
#define  MSG_TYPE_LMPOS     1000       //龙门数据包
#define  MSG_TYPE_SPFJSON   2000      //spf数据包
#define  MSG_TYPE_WELDSEAM  3000       // 焊缝信息包
#define  MSG_TYPE_CONTROL   4000       //
//command
#define COMMAND_TYPE_REQUEST_START  1100     // 消息标志
#define COMMAND_TYPE_REQUEST_END    2100    //
#define COMMAND_TYPE_DATA           3100     //

//////
#define COMMAND_TYPE_SYSTEM_START             4100     //龙门到达开始位置
#define COMMAND_TYPE_SYSTEM_START_OK          4101     //程序初始化完成，可以开始工作

#define COMMAND_TYPE_SYSTEM_SHUTDOWN          5100     //
#define COMMAND_TYPE_SYSTEM_SHUTDOWN_OK       5101     //龙门到达结束位置，拍摄完成


#endif // SERVERDEFS_H
