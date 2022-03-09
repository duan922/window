#ifndef CONFIGMODULEBLOCK_H_
#define CONFIGMODULEBLOCK_H_

#include <stm32f10x.h>

#ifndef CONFIGMODULEBLOCK_C_//如果没有定义
#define CONFIGMODULEBLOCK_Ex_ extern
#else
#define CONFIGMODULEBLOCK_Ex_
#endif

CONFIGMODULEBLOCK_Ex_ int ConfigModuleBlockDelay;

char ConfigModuleBlock(char *dat,char *returnc,char *returncc);
#endif

