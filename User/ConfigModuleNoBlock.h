#ifndef CONFIGMODULENOBLOCK_H_
#define CONFIGMODULENOBLOCK_H_
#include "stm32f10x.h"
#ifndef CONFIGMODULENOBLOCK_C_
#define CONFIGMODULENOBLOCK_Ex_  extern
#else 
#define CONFIGMODULENOBLOCK_Ex_
#endif

CONFIGMODULENOBLOCK_Ex_ u32 ConfigModuleNoBlockCnt;        //配置函数延时变量
CONFIGMODULENOBLOCK_Ex_ char ConfigModuleNoBlockFlage;     //1-配置完 0-未配置完
CONFIGMODULENOBLOCK_Ex_ int  ConfigModuleNoBlockCaseValue; //控制执行哪一条Case 语句

CONFIGMODULENOBLOCK_Ex_ void (*ConfigConnectDispose)(char *data);//定义一个函数指针变量,用来处理模块返回的数据

void ConfigModuleNoBlock(void);

void FunctionParseCompare(char *data);//对比判断是否返回想要的数据
void SendConfigFunction(char *order,void (*FunctionSend)(),char *HopeReturn1,char *HopeReturn2,void (*FunctionParse)(char *data),u32 ConfigFunctionValue);


#endif

