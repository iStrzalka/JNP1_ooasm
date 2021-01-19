#include "ooasm.h"
#include "computer.h"


int main() {
    auto ooasm_move = program({
        mov(mem(num(0)), num(42))
    });
    Computer computer1(1);
    computer1.boot(ooasm_move);
    assert(memory_dump(computer1) == "42 ");
}