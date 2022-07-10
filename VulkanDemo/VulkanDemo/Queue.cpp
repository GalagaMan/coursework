#include "Queue.h"

QueueFamily::QueueFamily(uint32_t familySize, vk::QueueFlags queueBits)
	:total_queue_count(familySize), free_queue_count(familySize), queue_bits(queueBits)
{
	
}
