#include "libicalflusher.h"
#include <libical/ical.h>

LibIcalFlusher::LibIcalFlusher()
{
}
LibIcalFlusher::~LibIcalFlusher(){
    icalmemory_free_ring();
}

