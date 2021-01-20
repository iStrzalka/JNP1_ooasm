#include "ooasm.h"
#include "computer.h"
#include <string>
#include <sstream>
#include <cassert>

namespace {
    std::string memory_dump(Computer const& computer) {
        std::stringstream ss;
        computer.memory_dump(ss);
        return ss.str();
    }
}

int main() {
    auto ooasm_move = program({
        mov(mem(num(0)), num(42))
    });
    Computer computer1(1);
    computer1.boot(ooasm_move);
    assert(memory_dump(computer1) == "42 ");

    auto ooasm_ones = program({
        dec(mem(num(1))),
        ones(mem(num(0)))
    });
    Computer computer2(2);
    computer2.boot(ooasm_ones);
    assert(memory_dump(computer2) == "1 -1 ");

    auto ooasm_data = program({
        inc(mem(lea("a"))),
        data("a", num(0)),
        data("b", num(2)),
        data("c", num(3))
    });
    Computer computer3(4);
    computer3.boot(ooasm_data);
    assert(memory_dump(computer3) == "1 2 3 0 ");

    auto ooasm_operations = program({
        data("a", num(4)),
        data("b", num(3)),
        data("c", num(2)),
        data("d", num(1)),
        add(mem(lea("a")), mem(lea("c"))),
        sub(mem(lea("b")), mem(lea("d"))),
        mov(mem(lea("c")), num(0)),
        mov(mem(lea("d")), num(0))
    });
    Computer computer4(5);
    computer4.boot(ooasm_operations);
    assert(memory_dump(computer4) == "6 2 0 0 0 ");

    auto oneki = program({
        onez(mem(num(18))),
        ones(mem(num(17))),
        data("xd1", num(4)),
        data("xd2", num(3)),
        data("xd3", num(2)),
        data("xd4", num(1)),
        data("xd11", num(4)),
        data("xd22", num(3)),
        data("xd33", num(2)),
        data("xd44", num(1)),
        data("xd111", num(4)),
        data("xd222", num(3)),
        data("xd333", num(2)),
        data("xd444", num(1)),
        data("xd1111", num(4)),
        data("xd2222", num(3)),
        data("xd3333", num(2)),
        data("xd4444", num(1)),
        sub(mem(lea("xd1")), mem(lea("xd11"))),
        one(mem(num(19))),
        onez(mem(lea("xd1"))),
        add(mem(lea("xd2")), mem(num(19))),
        onez(mem(lea("xd1111"))),
        ones(mem(lea("xd111"))),
        dec(mem(lea("xd4"))),
        onez(mem(lea("xd2"))),
        ones(mem(lea("xd3"))),
        inc(mem(lea("xd4"))),
        onez(mem(lea("xd11"))),
        ones(mem(lea("xd3"))),
        dec(mem(lea("xd3"))),
        onez(mem(lea("xd11"))),
        ones(mem(lea("xd111"))),
        dec(mem(num(19))),
        onez(mem(lea("xd11"))),
        ones(mem(lea("xd111"))),
        dec(mem(num(19))),
        onez(mem(lea("xd111"))),
        ones(mem(lea("xd1111"))),
        ones(mem(lea("xd111"))),
        mov(mem(lea("xd22")), mem(lea("xd22")))
    });
    Computer computer5(20);
    computer5.boot(oneki);
    assert(memory_dump(computer5) == "1 1 1 1 1 3 2 1 1 3 2 1 1 3 2 1 0 0 0 -1 ");
}
