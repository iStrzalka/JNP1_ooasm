#ifndef OOASM_H
#define OOASM_H

#include "computer_memory.h"
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <memory>
#include <cstring>

namespace ooasm {
    // Checks whether given input is a valid identifier. Throws an error if it is not.
    void check(const char *input) {
        if (input == nullptr || input[0] == '\0' || std::strlen(input) > 10)
            throw std::invalid_argument("Input should be not empty nor NULL nor of length over 10");
    }

    // Designed as virtual class to return the value to what it is pointing to.
    class RValue {
    public:
        virtual memory_word_t get_value([[maybe_unused]] ComputerMemory &mem) const = 0;

        virtual ~RValue() = default;
    };

    // Designed as virtual class to return the reference to what it is pointing to.
    class LValue {
    public:
        virtual memory_word_t &get_reference(ComputerMemory &mem) const = 0;

        virtual ~LValue() = default;
    };

    // Designed as a class that inherits from RValue to return its value.
    class Num : public RValue {
    private:
        memory_word_t value;
    public:
        explicit Num(memory_word_t val) : value(val) {}

        memory_word_t get_value([[maybe_unused]] ComputerMemory &mem) const override {
            return value;
        };
    };

    // Designed as a class that inherits from RValue to return which cell in memory
    // its Id is pointing to.
    class Lea : public RValue {
    private:
        identifier_t id;
    public:
        explicit Lea(const char *_id) : id(_id) {check(_id);}

        memory_word_t get_value(ComputerMemory &mem) const override {
            return mem.idx(id);
        };
    };

    // Designed as a class that inherits from both RValue and LValue.
    // On get_value() call returns value of cell in memory based on value of RValue.
    // On get_reference() call returns reference to the cell in memory based on value of RValue.
    class Mem : public LValue, public RValue {
    private:
        std::shared_ptr<RValue> rval;
    public:
        explicit Mem(std::shared_ptr<RValue> &x) : rval(std::move(x)) {}

        memory_word_t get_value(ComputerMemory &mem) const override {
            return mem.at(rval->get_value(mem));
        };

        memory_word_t &get_reference(ComputerMemory &mem) const override {
            return mem.at(rval->get_value(mem));
        }
    };

    // Designed as a virtual class that can execute it's functionality on given memory.
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

    // Designed as a class that inherits from Function that adds new identifier to memory
    // and assigns value to it.
    class Data : public Function {
    private:
        identifier_t data_id;
        std::shared_ptr<Num> data_num;
    public:
        Data(const char *input, std::shared_ptr<Num> &_num) : data_id(input), data_num(_num) {
            check(input);
            definition = true;
        }

        void execute(ComputerMemory &mem) override {
            mem.at(mem.add(data_id)) = data_num->get_value(mem);
        }
    };

    // Designed as a class that inherits from Function to overwrite memory cell referenced
    // by LValue by value of RValue.
    class Mov : public Function {
    private:
        std::shared_ptr<LValue> lval;
        std::shared_ptr<RValue> rval;
    public:
        explicit Mov(std::shared_ptr<LValue> &_lval,
                     std::shared_ptr<RValue> &_rval) : lval(_lval), rval(_rval) {}

        void execute(ComputerMemory &mem) override {
            lval->get_reference(mem) = rval->get_value(mem);
        }
    };

    // Designed as a virtual class that inherits from Function to be responsible
    // for arithmetic functions of the program.
    class Arithmetic : public Function {
    protected:
        std::shared_ptr<LValue> lval;
        std::shared_ptr<RValue> rval;
        bool negate = false;

        explicit Arithmetic(std::shared_ptr<LValue> &_lval,
                            std::shared_ptr<RValue> &_rval) : lval(_lval), rval(_rval) {}

        explicit Arithmetic(std::shared_ptr<LValue> &_lval,
                            std::shared_ptr<RValue> &&_rval) : lval(_lval), rval(_rval) {}

        void execute(ComputerMemory &mem) override {
            auto &lref = lval->get_reference(mem);

            if (negate)
                lref -= rval->get_value(mem);
            else
                lref += rval->get_value(mem);

            mem.set_flags(lref);
        }
    };

    // Designed as a class that inherits from Arithmetic to perform addition of value from RValue
    // to LValue's referenced memory cell and storing it in said cell. Sets flags if needed.
    class Add : public Arithmetic {
    public:
        explicit Add(std::shared_ptr<LValue> &_lval,
                     std::shared_ptr<RValue> &_rval) : Arithmetic(_lval, _rval) {}
    };

    // Designed as a class that inherits from Arithmetic to perform subtraction of value from RValue
    // to LValue's referenced memory cell and storing it in said cell. Sets flags if needed.
    class Sub : public Arithmetic {
    public:
        explicit Sub(std::shared_ptr<LValue> &_lval,
                     std::shared_ptr<RValue> &_rval) : Arithmetic(_lval, _rval) {
            negate = true;
        }
    };

    // Designed as a class that inherits from Arithmetic to perform incrementing
    // LValue's referenced memory cell and storing it in said cell. Sets flags if needed.
    class Inc : public Arithmetic {
    public:
        explicit Inc(std::shared_ptr<LValue> &_lval) : Arithmetic(_lval, std::make_shared<ooasm::Num>(1)) {}
    };

    // Designed as a class that inherits from Arithmetic to perform decrementing
    // LValue's referenced memory cell and storing it in said cell. Sets flags if needed.
    class Dec : public Arithmetic {
    public:
        explicit Dec(std::shared_ptr<LValue> &_lval) : Arithmetic(_lval, std::make_shared<ooasm::Num>(1)) {
            negate = true;
        }
    };

    // Designed as a virtual class that inherits from Function to be responsible
    // for assigning ones to memory cell referenced by LValue.
    class Flagged : public Function {
    protected:
        std::shared_ptr<LValue> lval;

        explicit Flagged(std::shared_ptr<LValue> &_lval) : lval(_lval) {}
    };

    // Designed as a virtual class that inherits from Flagged to be responsible
    // for assigning one to LValue of the program regardless of the state of both flags.
    class One : public Flagged {
    public:
        explicit One(std::shared_ptr<LValue> &_lval) : Flagged(_lval) {}

        void execute(ComputerMemory &mem) override {
            lval->get_reference(mem) = 1;
        }
    };

    // Designed as a virtual class that inherits from Flagged to be responsible
    // for assigning one to LValue of the program only when flag SF is set.
    class Ones : public Flagged {
    public:
        explicit Ones(std::shared_ptr<LValue> &_lval) : Flagged(_lval) {}

        void execute(ComputerMemory &mem) override {
            if (mem.is_flag_SF_set())
                lval->get_reference(mem) = 1;
        }
    };

    // Designed as a virtual class that inherits from Flagged to be responsible
    // for assigning one to LValue of the program only when flag ZF is set.
    class Onez : public Flagged {
    public:
        explicit Onez(std::shared_ptr<LValue> &_lval) : Flagged(_lval) {}

        void execute(ComputerMemory &mem) override {
            if (mem.is_flag_ZF_set())
                lval->get_reference(mem) = 1;
        }
    };
}

// Actual elements of OOASM language
inline std::shared_ptr<ooasm::Num> num(int64_t val) {
    return std::make_shared<ooasm::Num>(val);
}

inline std::shared_ptr<ooasm::Lea> lea(const char *_id) {
    return std::make_shared<ooasm::Lea>(_id);
}

inline std::shared_ptr<ooasm::Mem> mem(std::shared_ptr<ooasm::RValue> x) {
    return std::make_shared<ooasm::Mem>(x);
}

inline std::shared_ptr<ooasm::Data> data(const char *input, std::shared_ptr<ooasm::Num> _num) {
    return std::make_shared<ooasm::Data>(input, _num);
}

inline std::shared_ptr<ooasm::Mov> mov(std::shared_ptr<ooasm::LValue> _lval, std::shared_ptr<ooasm::RValue> _rval) {
    return std::make_shared<ooasm::Mov>(_lval, _rval);
}

inline std::shared_ptr<ooasm::Add> add(std::shared_ptr<ooasm::LValue> _lval, std::shared_ptr<ooasm::RValue> _rval) {
    return std::make_shared<ooasm::Add>(_lval, _rval);
}

inline std::shared_ptr<ooasm::Sub> sub(std::shared_ptr<ooasm::LValue> _lval, std::shared_ptr<ooasm::RValue> _rval) {
    return std::make_shared<ooasm::Sub>(_lval, _rval);
}

inline std::shared_ptr<ooasm::Inc> inc(std::shared_ptr<ooasm::LValue> _lval) {
    return std::make_shared<ooasm::Inc>(_lval);
}

inline std::shared_ptr<ooasm::Dec> dec(std::shared_ptr<ooasm::LValue> _lval) {
    return std::make_shared<ooasm::Dec>(_lval);
}

inline std::shared_ptr<ooasm::One> one(std::shared_ptr<ooasm::LValue> _lval) {
    return std::make_shared<ooasm::One>(_lval);
}

inline std::shared_ptr<ooasm::Ones> ones(std::shared_ptr<ooasm::LValue> _lval) {
    return std::make_shared<ooasm::Ones>(_lval);
}

inline std::shared_ptr<ooasm::Onez> onez(std::shared_ptr<ooasm::LValue> _lval) {
    return std::make_shared<ooasm::Onez>(_lval);
}

class program {
private:
    std::vector<std::shared_ptr<ooasm::Function>> vec;
public:
    program(std::initializer_list<std::shared_ptr<ooasm::Function>> init_list) : vec(init_list) {}

    using iterator = typename std::vector<std::shared_ptr<ooasm::Function>>::iterator;

    iterator begin() noexcept {
        return vec.begin();
    };

    iterator end() noexcept {
        return vec.end();
    };
};

#endif //OOASM_H