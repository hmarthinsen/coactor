#pragma once

#define RECEIVE() co_await receive()

#define ACT(body)                                                              \
	Handle act() override                                                      \
	{                                                                          \
		body co_return;                                                        \
	}

#define ACTOR(name, body)                                                      \
	class name : public coactor::Actor {                                       \
	public:                                                                    \
		ACT(body)                                                              \
	}

#define ACTOR_BEGIN(name)                                                      \
	class name : public coactor::Actor {                                       \
	public:                                                                    \
		Handle act() override                                                  \
		{
#define ACTOR_END                                                              \
	}                                                                          \
	}                                                                          \
	;
