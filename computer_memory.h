#ifndef OOASM_COMPUTER_MEMORY_H
#define OOASM_COMPUTER_MEMORY_H

// Memory of the computer.
#include <cstdint>
#include <vector>
#include <stdexcept>

using flag_t = bool;
using memory_word_t = int64_t;
using identifier_t = uint64_t;

struct ComputerMemory {
    using memory_t = std::vector<memory_word_t>;
    using vars_memory_t = std::vector<identifier_t>;
    using vars_size_t = typename vars_memory_t::size_type;

    vars_memory_t vars;
    memory_t mem;
    flag_t ZF = false;
    flag_t SF = false;
    vars_size_t last_index = 0;
    vars_size_t size = -1;

    void setup(const int &_size) {
        size = _size;
        vars.reserve(_size);
        mem.reserve(_size);
    }

    // Assigns the identifier to one of memory's cells.
    // Throws an error if there are more assigned identifiers than memory's cells.
    vars_size_t add(identifier_t id) {
        if (last_index == size)
            throw std::invalid_argument("Too many variables");

        vars[last_index] = id;
        return last_index++;
    }

    // Finds which index of the memory identifier is assigned to and returns it.
    // Throws an error if it can't find it.
    [[nodiscard]] vars_size_t idx(identifier_t id) const {
        for (vars_size_t i = 0; i < size; i++) {
            if (vars[i] == id) {
                return i;
            }
        }
        throw std::invalid_argument("Variable not found");
    }

    // Sets the flag ZF based on last changed value.
    void set_flag_ZF(memory_word_t value) {
        ZF = value == 0;
    }

    // Sets the flag SF based on last changed value.
    void set_flag_SF(memory_word_t value) {
        SF = value < 0;
    }

    // Sets up both flags based on last changed value.
    void set_flags(memory_word_t value) {
        set_flag_ZF(value);
        set_flag_SF(value);
    }

    // Checks if flag ZF is set.
    bool is_flag_ZF_set() {
        return ZF;
    }

    // Checks if flag SF is set.
    bool is_flag_SF_set() {
        return SF;
    }
};

#endif //OOASM_COMPUTER_MEMORY_H