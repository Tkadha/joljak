#pragma once
#include <chrono>
#include <concurrent_priority_queue.h>
enum class EVENT_TYPE
{
	E_P_SLOW_END = 0,
	E_P_REGENERATE_HP,
	E_P_REGENERATE_STAMINA,
	E_P_CONSUME_HUNGER,
	E_P_CONSUME_THIRST,
};

class EVENT
{
public:
	std::chrono::system_clock::time_point wakeup_time;
	EVENT_TYPE e_type;
	unsigned long long player_id;
	int obj_id;

public:
	EVENT() = default;
	~EVENT() = default;
	EVENT(EVENT_TYPE type, unsigned long long p_id, int o_id)
		: e_type(type), player_id(p_id), obj_id(o_id)
	{ }
	static void add_timer(EVENT& ev, int ms) {
		ev.wakeup_time = std::chrono::system_clock::now() + std::chrono::milliseconds(ms);
		event_queue.push(ev);
	}



public:
	static concurrency::concurrent_priority_queue<EVENT> event_queue;

	bool operator<(const EVENT& other) const
	{
		return wakeup_time > other.wakeup_time;
	}
};

