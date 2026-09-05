#ifndef __FACTORY_H
#define __FACTORY_H

#include "Config.h"

/* 初始化工厂环境监控，并采集开机基准温度。 */
void Factory_Init(void);

/* 执行一次环境监控任务，建议在主循环中每10ms调用一次。 */
void Factory_Task(void);

#endif /* __FACTORY_H */
