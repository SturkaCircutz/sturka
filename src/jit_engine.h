#pragma once

#include "ast.h"

class JitEngine {
public:
    void execute(const Program& program);
};
