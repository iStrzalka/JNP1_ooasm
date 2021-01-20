#ifndef OOASM_H
#define OOASM_H

#include "computer_memory.h"
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <memory> // For the shared_ptr<> template

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
    virtual int get_value([[maybe_unused]] ComputerMemory &mem) const = 0;

    virtual ~RValue() = default;
};

class LValue { // są pure pure virtual wiec nie potrzebują chyba wirtualnych destruktorów
    friend class Computer;

public:
    virtual memory_word_t &get_reference(ComputerMemory &mem) const = 0;

    virtual ~LValue() = default;

    virtual int get_value([[maybe_unused]] ComputerMemory &mem) const = 0;
};

class Num : public RValue {
private:
    int64_t value;
public:
    explicit Num(int64_t val) : value(val) {}

    int get_value([[maybe_unused]] ComputerMemory &mem) const override {
        return value;
    };
};

std::shared_ptr<Num> num(int64_t val) {
    return std::make_shared<Num>(val);
}

class Lea : public RValue {
private:
    identifier_t id;
public:
    explicit Lea(const char *_id) : id(Id(_id)) {}

    int get_value(ComputerMemory &mem) const override {
        return mem.idx(id);
    };
};

std::shared_ptr<Lea> lea(const char *_id) {
    return std::make_shared<Lea>(_id);
}

class Mem : public LValue, public RValue {
private:
    std::shared_ptr<RValue> rval;
public:
    explicit Mem(std::shared_ptr<RValue>& x) : rval(std::move(x)) {}

    int get_value(ComputerMemory &mem) const override {
        return mem.at(rval->get_value(mem));
    };

    memory_word_t &get_reference(ComputerMemory &mem) const override {
        return mem.at(rval->get_value(mem));
    }
};

std::shared_ptr<Mem> mem(std::shared_ptr<RValue> x) {
    return std::make_shared<Mem>(x);
}

class Function {
protected:
    bool definition = false;
public:
    virtual void execute(ComputerMemory &mem) = 0;

    virtual ~Function() = default;

    [[nodiscard]] bool is_definition() const noexcept {
        return definition;
    }
};

class Data : public Function {
private:
    identifier_t data_id;
    std::shared_ptr<Num> data_num;
public:
    Data(const char *input, std::shared_ptr<Num>& _num) : data_id(Id(input)), data_num(_num) {
        definition = true;
    }

    void execute(ComputerMemory &mem) override {
        mem.at(mem.add(data_id)) = data_num->get_value(mem);
    }
};

std::shared_ptr<Data> data(const char *input, std::shared_ptr<Num> _num) {
    return std::make_shared<Data>(input, _num);
}

class Mov : public Function {
private:
    std::shared_ptr<LValue> lval;
    std::shared_ptr<RValue> rval;
public:
    explicit Mov(std::shared_ptr<LValue>& _lval, std::shared_ptr<RValue>& _rval) : lval(_lval),
                                                                                 rval(_rval) {}

    void execute(ComputerMemory &mem) override {
        lval->get_reference(mem) = rval->get_value(mem);
    }
};

std::shared_ptr<Mov> mov(std::shared_ptr<LValue> _lval, std::shared_ptr<RValue> _rval) {
    return std::make_shared<Mov>(_lval, _rval);
}

class Arithmetic : public Function {
protected:
    std::shared_ptr<LValue> lval;
    std::shared_ptr<RValue> rval;
    bool negate = false;

    explicit Arithmetic(std::shared_ptr<LValue>& _lval, std::shared_ptr<RValue>& _rval) : lval(_lval),
                                                                                        rval(_rval) {}
    explicit Arithmetic(std::shared_ptr<LValue>& _lval, std::shared_ptr<RValue>&& _rval) : lval(_lval),
                                                                                          rval(_rval) {}

    void execute(ComputerMemory &mem) override {
        auto &lref = lval->get_reference(mem);
        if (negate) {
            lref -= rval->get_value(mem);
        } else {
            lref += rval->get_value(mem);
        }
        mem.set_flags(lref);
    }
};

class Add : public Arithmetic {
public:
    explicit Add(std::shared_ptr<LValue>& _lval, std::shared_ptr<RValue>& _rval) : Arithmetic(_lval,
                                                                                            _rval) {}
};

std::shared_ptr<Add> add(std::shared_ptr<LValue> _lval, std::shared_ptr<RValue> _rval) {
    return std::make_shared<Add>(_lval, _rval);
}

class Sub : public Arithmetic {
public:
    explicit Sub(std::shared_ptr<LValue>& _lval, std::shared_ptr<RValue>& _rval) : Arithmetic(_lval,
                                                                                            _rval) {
        negate = true;
    }
};

std::shared_ptr<Sub> sub(std::shared_ptr<LValue> _lval, std::shared_ptr<RValue> _rval) {
    return std::make_shared<Sub>(_lval, _rval);
}

class Inc : public Arithmetic {
public:
    explicit Inc(std::shared_ptr<LValue>& _lval) : Arithmetic(_lval, num(1)) {}
};

std::shared_ptr<Inc> inc(std::shared_ptr<LValue> _lval) {
    return std::make_shared<Inc>(_lval);
}

class Dec : public Arithmetic {
public:
    explicit Dec(std::shared_ptr<LValue>& _lval) : Arithmetic(_lval, num(1)) {
        negate = true;
    }
};

std::shared_ptr<Dec> dec(std::shared_ptr<LValue> _lval) {
    return std::make_shared<Dec>(_lval);
}

class Flagged : public Function { //Fixme : Better name for that.
protected:
    std::shared_ptr<LValue> lval;

    explicit Flagged(std::shared_ptr<LValue>& _lval) : lval(_lval) {}
};

class One : public Flagged {
public:
    explicit One(std::shared_ptr<LValue>& _lval) : Flagged(_lval) {}

    void execute(ComputerMemory &mem) override {
        lval->get_reference(mem) = 1;
    }
};

std::shared_ptr<One> one(std::shared_ptr<LValue> _lval) {
    return std::make_shared<One>(_lval);
}

class Ones : public Flagged {
public:
    explicit Ones(std::shared_ptr<LValue>& _lval) : Flagged(_lval) {}

    void execute(ComputerMemory &mem) override {
        if (mem.is_flag_SF_set()) {
            lval->get_reference(mem) = 1;
        }
    }
};

std::shared_ptr<Ones> ones(std::shared_ptr<LValue> _lval) {
    return std::make_shared<Ones>(_lval);
}

class Onez : public Flagged {
public:
    explicit Onez(std::shared_ptr<LValue>& _lval) : Flagged(_lval) {}

    void execute(ComputerMemory &mem) override {
        if (mem.is_flag_ZF_set()) {
            lval->get_reference(mem) = 1;
        }
    }
};

std::shared_ptr<Onez> onez(std::shared_ptr<LValue> _lval) {
    return std::make_shared<Onez>(_lval);
}

class program {
private:
    std::vector<std::shared_ptr<Function>> vec;
public:
    program(std::initializer_list<std::shared_ptr<Function>> init_list) : vec(init_list) {}

    using iterator = typename std::vector<std::shared_ptr<Function>>::iterator;

    iterator begin() noexcept {
        return vec.begin();
    };

    iterator end() noexcept {
        return vec.end();
    };
};

#endif //OOASM_H
