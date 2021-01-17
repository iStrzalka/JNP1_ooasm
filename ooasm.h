#ifndef OOASM_H
#define OOASM_H

#include "computer_memory.h"
#include <cstdint>
#include <stdexcept>
#include <utility>

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
    virtual int get_value(ComputerMemory &m) const {
        throw std::exception();
    }
};

class LValue {
    friend class Computer;;
public:
    virtual memory_word_t& get_reference(ComputerMemory &m) const {
        throw std::exception();
    };

    virtual int get_value(ComputerMemory &m) const {
        throw std::exception();
    };
};

class num : public RValue {
private:
    int64_t value;
public:
    explicit num(int64_t val) : value(val) {}

    int get_value(ComputerMemory &mem) const override {
        return value;
    };
};

class lea : public RValue {
private:
    identifier_t id;
public:
    explicit lea(const char* _id) : id(Id(_id)) {}

    int get_value(ComputerMemory &mem) const override {
        return id;
    };
};

class mem : public LValue, public RValue {
private:
    RValue rval;
    bool is_lea = false;
public:
    explicit mem(RValue x) : rval(std::move(x)) {}
    explicit mem(lea x) : rval(std::move(x)) {
        is_lea = true;
    }

    int get_value(ComputerMemory &mem) const override {
        if (is_lea) {
            return mem.idx(rval.get_value(mem));
        } else {
            return rval.get_value(mem);
        }
    };

    memory_word_t& get_reference(ComputerMemory &m) const override {
        return m.mem.at(get_value(m));
    }
};

class Function {
protected:
    bool definition = false;
public:
    virtual void execute(ComputerMemory &m) {
        throw std::exception();
    }

    [[nodiscard]] bool is_definition() const noexcept {
        return definition;
    }
};

class data : public Function {
private:
    identifier_t data_id;
    num data_num;
public:
    data(const char* input, num _num) : data_id(Id(input)), data_num(std::move(_num)) {
        definition = true;
    }

    void execute(ComputerMemory &m) override {
        m.mem.at(m.add(data_id)) = data_num.get_value(m);
    }
};

class mov : public Function {
private:
    LValue lval;
    RValue rval;
public:
    explicit mov(LValue _lval, RValue _rval) : lval(std::move(_lval)), rval(std::move(_rval)) {}

    void execute(ComputerMemory &m) override {
        lval.get_reference(m) = rval.get_value(m);
    }
};

class Arithmetic : public Function {
protected:
    LValue lval;

    explicit Arithmetic(LValue _lval) : lval(std::move(_lval)) {}

    void change_flags(ComputerMemory &m) {
        m.set_flags(lval.get_reference(m));
    }
};

class ArithmeticTwoArgs : public Arithmetic {
protected:
    RValue rval;
    explicit ArithmeticTwoArgs(LValue _lval,  RValue _rval) : Arithmetic(std::move(_lval)), rval(std::move(_rval)) {}
};

class ArithmeticOneArg : public Arithmetic {
protected:

    explicit ArithmeticOneArg(LValue _lval) : Arithmetic(std::move(_lval)) {}
};

class add : public ArithmeticTwoArgs {
public:
    explicit add(LValue _lval,  RValue _rval) : ArithmeticTwoArgs(std::move(_lval), std::move(_rval)) {}

    void execute(ComputerMemory &m) override {
        lval.get_reference(m) += rval.get_value(m);
        change_flags(m);
    }
};

class sub : public ArithmeticTwoArgs {
public:
    explicit sub(LValue _lval,  RValue _rval) : ArithmeticTwoArgs(std::move(_lval), std::move(_rval)) {}

    void execute(ComputerMemory &m) override {
        lval.get_reference(m) -= rval.get_value(m);
        change_flags(m);
    }
};

class inc : public ArithmeticOneArg {
public:
    explicit inc(LValue _lval) : ArithmeticOneArg(std::move(_lval)) {}

    void execute(ComputerMemory &m) override {
        lval.get_reference(m)++;
        change_flags(m);
    }
};

class dec : public ArithmeticOneArg {
public:
    explicit dec(LValue _lval) : ArithmeticOneArg(std::move(_lval)) {}

    void execute(ComputerMemory &m) override {
        lval.get_reference(m)--;
        change_flags(m);
    }
};

class Flagged : public Function {
protected:
    LValue lval;
    explicit Flagged(LValue _lval) : lval(std::move(_lval)) {}
};

class one : public Flagged {
public:
    explicit one(LValue _lval) : Flagged(std::move(_lval)) {}

    void execute(ComputerMemory &m) override {
        lval.get_reference(m) = 1;
    }
};

class ones : public Flagged {
public:
    explicit ones(LValue _lval) : Flagged(std::move(_lval)) {}

    void execute(ComputerMemory &m) override {
        if (m.is_flag_SF_set()) {
            lval.get_reference(m) = 1;
        }
    }

};

class onez : public Flagged {
public:
    explicit onez(LValue _lval) : Flagged(std::move(_lval)) {}

    void execute(ComputerMemory &m) override {
        if (m.is_flag_ZF_set()) {
            lval.get_reference(m) = 1;
        }
    }
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