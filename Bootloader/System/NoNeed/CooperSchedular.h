#include "globol.h"
#include "System/TBManager.h"

#define MAX_TASK  5
#define PERIODIC  1
#define SINGLE    0

struct SCHEDULAR{
    void (*pfunc)(void);
    u8 Delay;
    u8 Period;
    u8 Active;
};

#define ERROR_SCH_TASK_ARRAY_FULL   1
