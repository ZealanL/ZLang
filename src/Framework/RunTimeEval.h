#pragma once
#include "FrameworkBase.h"

// PURPOSE: Have lambda functions be executed at runtime initialization (prior to main)

struct RunTimeEvalCall {
	RunTimeEvalCall(std::function<void()> funcToCall) {
		funcToCall();
	}
};

#define RUNTIME(f) const auto NAMEMERGE_IDR(__RUNTIME_EVAL_CALL, __COUNTER__) = RunTimeEvalCall(f)