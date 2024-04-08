#include "def.h"
#include "DataQueue.h"
#include "BcgManager.h"

class BcgDataProcess
{
public:
    void processData(bcg_parse_data_t* data);

private:
    messege_queue_data_t data;
}