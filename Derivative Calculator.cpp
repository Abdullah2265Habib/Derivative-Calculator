#include <iostream>
#include <string>
#include <cctype>
#include <cmath>
using namespace std;

struct Expr {
    virtual ~Expr() {}
    virtual Expr* derivative() = 0;
    virtual Expr* clone() = 0;
    virtual void print() = 0;
};

struct Number : Expr {
    double value;
    Number(double v) : value(v) {}
    Expr* derivative() override { return new Number(0); }
    Expr* clone() override { return new Number(value); }
    void print() override { cout << value; }
};

struct Var : Expr {
    Expr* derivative() override { return new Number(1); }
    Expr* clone() override { return new Var(); }
    void print() override { cout << "x"; }
};

struct Add : Expr {
    Expr* a; Expr* b;
    Add(Expr* x, Expr* y) : a(x), b(y) {}
    Expr* derivative() override {
        return new Add(a->derivative(), b->derivative());
    }
    Expr* clone() override { return new Add(a->clone(), b->clone()); }
    void print() override {
        cout << "("; a->print(); cout << " + "; b->print(); cout << ")";
    }
};

struct Sub : Expr {
    Expr* a; Expr* b;
    Sub(Expr* x, Expr* y) : a(x), b(y) {}
    Expr* derivative() override {
        return new Sub(a->derivative(), b->derivative());
    }
    Expr* clone() override { return new Sub(a->clone(), b->clone()); }
    void print() override {
        cout << "("; a->print(); cout << " - "; b->print(); cout << ")";
    }
};

struct Mul : Expr {
    Expr* a; Expr* b;
    Mul(Expr* x, Expr* y) : a(x), b(y) {}
    Expr* derivative() override {
        return new Add(
            new Mul(a->derivative(), b->clone()),
            new Mul(a->clone(), b->derivative())
        );
    }
    Expr* clone() override { return new Mul(a->clone(), b->clone()); }
    void print() override {
        cout << "("; a->print(); cout << " * "; b->print(); cout << ")";
    }
};

struct Div : Expr {
    Expr* a; Expr* b;
    Div(Expr* x, Expr* y) : a(x), b(y) {}
    Expr* derivative() override {
        return new Div(
            new Sub(
                new Mul(a->derivative(), b->clone()),
                new Mul(a->clone(), b->derivative())
            ),
            new Mul(b->clone(), b->clone())
        );
    }
    Expr* clone() override { return new Div(a->clone(), b->clone()); }
    void print() override {
        cout << "("; a->print(); cout << " / "; b->print(); cout << ")";
    }
};

struct Func;

struct Pow : Expr {
    Expr* base; Expr* exp;
    Pow(Expr* b, Expr* e) : base(b), exp(e) {}
    Expr* derivative() override;
    Expr* clone() override { return new Pow(base->clone(), exp->clone()); }
    void print() override {
        cout << "("; base->print(); cout << "^"; exp->print(); cout << ")";
    }
};

struct Func : Expr {
    string name; Expr* arg;
    Func(string n, Expr* a) : name(n), arg(a) {}
    Expr* derivative() override {
        if (name == "sin")
            return new Mul(new Func("cos", arg->clone()), arg->derivative());
        if (name == "cos")
            return new Mul(new Number(-1),
                new Mul(new Func("sin", arg->clone()), arg->derivative()));
        if (name == "tan")
            return new Mul(
                new Div(new Number(1), new Pow(new Func("cos", arg->clone()), new Number(2))),
                arg->derivative()
            );
        if (name == "ln")
            return new Div(arg->derivative(), arg->clone());
        if (name == "exp")
            return new Mul(new Func("exp", arg->clone()), arg->derivative());
        return new Number(0);
    }
    Expr* clone() override { return new Func(name, arg->clone()); }
    void print() override {
        cout << name << "("; arg->print(); cout << ")";
    }
};

Expr* Pow::derivative() {
    Expr* f = base;
    Expr* g = exp;

    return new Mul(
        this->clone(),
        new Add(
            new Mul(g->derivative(), new Func("ln", f->clone())),
            new Mul(g->clone(), new Div(f->derivative(), f->clone()))
        )
    );
}

Expr* simplify(Expr* e) {
    //addition
    if (Add* add = dynamic_cast<Add*>(e)) {
        Expr* a = simplify(add->a);
        Expr* b = simplify(add->b);

        Number* n1 = dynamic_cast<Number*>(a);
        if (n1 && n1->value == 0) return b;

        Number* n2 = dynamic_cast<Number*>(b);
        if (n2 && n2->value == 0) return a;

        return new Add(a, b);
    }

    //subtraction
    if (Sub* sub = dynamic_cast<Sub*>(e)) {
        Expr* a = simplify(sub->a);
        Expr* b = simplify(sub->b);

        Number* n2 = dynamic_cast<Number*>(b);
        if (n2 && n2->value == 0) return a;

        return new Sub(a, b);
    }

    //multiplication
    if (Mul* mul = dynamic_cast<Mul*>(e)) {
        Expr* a = simplify(mul->a);
        Expr* b = simplify(mul->b);

        Number* n1 = dynamic_cast<Number*>(a);
        if (n1 && n1->value == 0) return new Number(0);

        Number* n2 = dynamic_cast<Number*>(b);
        if (n2 && n2->value == 0) return new Number(0);

        n1 = dynamic_cast<Number*>(a);
        if (n1 && n1->value == 1) return b;

        n2 = dynamic_cast<Number*>(b);
        if (n2 && n2->value == 1) return a;

        return new Mul(a, b);
    }

    //division
    if (Div* div = dynamic_cast<Div*>(e)) {
        Expr* a = simplify(div->a);
        Expr* b = simplify(div->b);

        Number* n1 = dynamic_cast<Number*>(a);
        if (n1 && n1->value == 0) return new Number(0);

        Number* n2 = dynamic_cast<Number*>(b);
        if (n2 && n2->value == 1) return a;

        return new Div(a, b);
    }

    //power
    if (Pow* p = dynamic_cast<Pow*>(e)) {
        Expr* a = simplify(p->base);
        Expr* b = simplify(p->exp);

        Number* n = dynamic_cast<Number*>(b);
        if (n && n->value == 1) return a;
        if (n && n->value == 0) return new Number(1);

        return new Pow(a, b);
    }

    //function
    if (Func* f = dynamic_cast<Func*>(e)) {
        f->arg = simplify(f->arg);
        return f;
    }

    return e;
}

string s;
int pos = 0;
void skip() { while (pos < s.size() && isspace(s[pos])) pos++; }
bool match(char c) { skip(); if (pos < s.size() && s[pos] == c) { pos++; return true; } return false; }
Expr* parseExpression();

Expr* parseNumber() {
    skip();
    double num = 0;
    bool decimal = false;
    double frac = 1.0;

    while (pos < s.size() && (isdigit(s[pos]) || s[pos] == '.')) {
        if (s[pos] == '.')
            decimal = true;
        else {
            num = num * 10 + (s[pos] - '0');
            if (decimal) frac *= 0.1;
        }
        pos++;
    }
    return new Number(num * frac);
}

Expr* parseFactor() {
    skip();
    if (pos < s.size() && isdigit(s[pos])) return parseNumber();
    if (pos < s.size() && s[pos] == 'x') { pos++; return new Var(); }

    if (pos < s.size() && isalpha(s[pos])) {
        string name;
        while (pos < s.size() && isalpha(s[pos])) name += s[pos++];
        match('(');
        Expr* arg = parseExpression();
        match(')');
        return new Func(name, arg);
    }

    if (match('(')) {
        Expr* e = parseExpression();
        match(')');
        return e;
    }

    return nullptr;
}

Expr* parsePower() {
    Expr* left = parseFactor();
    skip();
    if (match('^')) return new Pow(left, parsePower());
    return left;
}

Expr* parseTerm() {
    Expr* left = parsePower();
    skip();
    while (true) {
        if (match('*'))
            left = new Mul(left, parsePower());
        else if (match('/'))
            left = new Div(left, parsePower());
        else
            break;
    }
    return left;
}

Expr* parseExpression() {
    Expr* left = parseTerm();
    skip();
    while (true) {
        if (match('+'))
            left = new Add(left, parseTerm());
        else if (match('-'))
            left = new Sub(left, parseTerm());
        else break;
    }
    return left;
}

int main() {
    cout << "Enter function f(x): ";
    getline(cin, s);
    pos = 0;

    Expr* expr = parseExpression();
    cout << "f(x) = ";
    expr->print();
    cout << "\n";

    Expr* deriv = simplify(expr->derivative());
    cout << "f'(x) = ";
    deriv->print();
    cout << "\n";

    return 0;
}
