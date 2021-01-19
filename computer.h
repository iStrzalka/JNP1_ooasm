#ifndef OOASM_COMPUTER_H
#define OOASM_COMPUTER_H

#include <array>
#include <vector>
#include <ostream>
#include "ooasm.h"
#include "computer_memory.h"

class Computer {
private:
    ComputerMemory cm;

    //Executes all declarations of the program.
    void declare_vars(program &p) {
        for (const auto &command : p) {
            if (command->is_definition()) {
                command->execute(cm);
            }
        }
    }

    //Executes all functions that aren't declarations.
    void execute_functions(program &p) {
        for (const auto &command : p) {
            if (!command->is_definition()) {
                command->execute(cm);
            }
        }
    }

public:
    explicit Computer(const int &size) {
        cm.setup(size);
    }

    void boot(program &p) {
        declare_vars(p);
        execute_functions(p);
    }

    void memory_dump(std::ostream &os) const {
        for (auto a : cm.mem) {
            os << a << ' ';
        }
    }
};

#endif //OOASM_COMPUTER_H
