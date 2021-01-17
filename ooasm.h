#ifndef OOASM_H
#define OOASM_H

#include "computer_memory.h"
#include <cstdint>
#include <stdexcept>

using identifier_t = uint64_t;

// Returns identifier for provided [input]. Big and small letters are undifferentiated.
identifier_t Id(const char *input) {
    identifier_t base = 601;
    identifier_t x = 1;

    if (input[0] == '\0')
        throw std::invalid_argument("Input should be not empty");
    identifier_t base2 = 1;
    for (std::size_t i = 0; input[i] != '\0'; i++) {
        if (i == 10)
            throw std::invalid_argument("Input's size is over the limit of 6 characters");

        int integer = input[i];

        x += base2 * integer;
        base2 *= base;
    }

    const identifier_t result = x;
    return result;
}

class RValue {
    friend class Computer;
public:
    virtual int get_value(ComputerMemory &m){};
};

class LValue {
    friend class Computer;
    virtual memory_word_t& get_reference(ComputerMemory &m){};
};

class num : public RValue {
private:
    int value;
public:

};

class lea : public RValue {
private:
    identifier_t id;
public:

};

class mem : public LValue, RValue {
private:
    RValue rval;
public:
    int get_value(ComputerMemory &mem){};
};

class Function {
protected:
    bool definition = false;
public:
    void execute(ComputerMemory &m) {};
    bool is_definition() {
        return definition;
    };
};

class data : public Function {
private:
    identifier_t data_id;
    num data_num;
public:
    data(const char* input, num _num) : data_id(Id(input)), data_num(_num) {
        definition = true;
    };
};

class mov : public Function {

};

class Arithmetic : public Function {

};

class ArithmeticTwoArgs : public Arithmetic {

};

class ArithmeticOneArg : public Arithmetic {

};

class add : public ArithmeticTwoArgs {

};

class sub : public ArithmeticTwoArgs {

};

class inc : public ArithmeticOneArg {

};

class dec : public ArithmeticOneArg {

};

class Flagged : public Function {

};

class one : public Flagged {

};

class ones : public Flagged {

};

class onez : public Flagged {

};

class program {
private:
    std::vector<Function> vec;
public:
    program(std::initializer_list<Function> init_list) : vec(init_list) {}
    using iterator = typename std::vector<Function>::iterator;

    iterator begin() noexcept {
        return vec.begin();
    };

    iterator end() noexcept {
        return vec.end();
    };
};

#endif //OOASM_H
