#include "basecode/frameAllocator.h"

uint8*   FrameAllocator::smBuffer = NULL;
uint32   FrameAllocator::smWaterMark = 0;
uint32   FrameAllocator::smHighWaterMark = 0;


