#pragma once
#include <iostream>
#include <vector>

#include <vulkan/vulkan.hpp>


class QueueFamily
{
public:
	uint32_t total_queue_count;
	uint32_t free_queue_count;
	vk::QueueFlags queue_bits;
	QueueFamily(uint32_t familySize, vk::QueueFlags queueBits);
};

class Queue
{
};

