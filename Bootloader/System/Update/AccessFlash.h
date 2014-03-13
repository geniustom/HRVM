#include "globol.h"
#include "Flash.h"

void CLOCK_SET();	   								//set the system clock
void save_data(u32 Page_index,u8 *Databuff);
void read_data(u32 Page_index,u8 *Databuff);
void Block_Erase(u32 Page_index);
