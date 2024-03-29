#ifndef OOASM_COMPUTER_MEMORY_H
#define OOASM_COMPUTER_MEMORY_H

// Memory of the computer.
#include <vector>
#include <unordered_map>

namespace ooasm {
    using flag_t = bool;
    using memory_word_t = int64_t;
    using identifier_t = std::string;

    struct ComputerMemory {
        using memory_t = std::vector<memory_word_t>;
        using vars_memory_t = std::unordered_map<identifier_t, memory_t::size_type>;
        using vars_size_t = typename vars_memory_t::size_type;

        vars_memory_t vars;
        memory_t mem;
        flag_t ZF = false;
        flag_t SF = false;
        vars_size_t last_index = 0;
        vars_size_t size = -1;

        void setup() {
            vars = vars_memory_t();
            mem = memory_t(size);
        }

        // Assigns the identifier to one of memory's cells.
        // Throws an error if there are more assigned identifiers than memory's cells.
        vars_size_t add(const identifier_t& id) {
            if (last_index == size)
                throw std::invalid_argument("Too many variables");

            if (vars.find(id) == vars.end())
                vars[id] = last_index;

            return last_index++;
        }

        // Finds which index of the memory identifier is assigned to and returns it.
        // Throws an error if it can't find it.
        [[nodiscard]] vars_size_t idx(const identifier_t& id) const {
            auto it = vars.find(id);

            if (it != vars.end())
                return it->second;
            else
                throw std::invalid_argument("Variable not found");
        }

        // Returns memory at index [index]. Throws an error if it is out of bounds.
        memory_word_t &at(vars_size_t index) {
            if (index > size)
                throw std::invalid_argument("Out of bounds");

            return mem[index];
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
        bool is_flag_ZF_set() const {
            return ZF;
        }

        // Checks if flag SF is set.
        bool is_flag_SF_set() const {
            return SF;
        }
    };
}

#endif //OOASM_COMPUTER_MEMORY_H

