#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>
#include <utility>
#include <stdexcept>
using namespace std;

int int_part, frac_part, fixed_max_val, fixed_min_val;
int significand_bits, exponent_bits, bias, exp_min, exp_max;
string format;

int correct_form(int num) {
    while (num > fixed_max_val) {
        num -= 2 * (fixed_max_val + 1);
    }
    while (num < fixed_min_val) {
        num += 2 * (fixed_max_val + 1);
    }
    return num;
}

void fixed_print(int num) {
    num = correct_form(num);
    long long num_long = (long long)num * 1000;
    num_long = num_long >> frac_part;
    if (num_long / 1000 == 0 && num_long < 0) cout << "-";
    cout << num_long / 1000 << "." << (abs(num_long) / 100) % 10 << (abs(num_long) / 10) % 10 << abs(num_long) % 10 << endl;
}

int sum(int num1, int num2) {
    return correct_form(num1 + num2);
}

int subtract(int num1, int num2) {
    return correct_form(num1 - num2);
}

int multiply(int num1, int num2) {
    long long num3 = (long long)num1 * num2;
    long long div = 1ll << frac_part;
    if (num3 >= 0) {
        return correct_form((int)(num3 / div));
    }
    else {
        return correct_form((int)((num3 - div + 1) / div));
    }
}

int divide(int num1, int num2) {
    if (num2 == 0) {
        cout << "error";
        return 0;
    }
    long long num1_long = (long long)num1 * (1ll << frac_part);
    return correct_form(num1_long / (long long)num2);
}

string dec_to_hex(int num) {
    int hex = num % 16; 
    if (hex >= 10) {
        int tmp = (int)'a' + (hex - 10);
        return string(1, char(tmp));
    }
    return to_string(hex);
}

bool is_subnormal(long long num) {
    num = num >> significand_bits;
    num &= (1ll << exponent_bits) - 1;
    return num == 0;
}

int exp(long long num) {
    int exponent = num >> significand_bits;
    exponent &= (1 << exponent_bits) - 1;
    if (exponent <= 0) exponent = 1;
    return exponent - bias;
}

long long man(long long num) {
    long long mantissa = num & (1ll << significand_bits) - 1;
    if (!is_subnormal(num) && exp(num) != exp_max + 1) mantissa += 1ll << significand_bits;
    return mantissa;
}

int sign(long long num) {
    if (num >= (1ll << significand_bits + exponent_bits)) return 1;
    return 0;
}

bool is_nan(long long num) {
    return exp(num) == exp_max + 1 && man(num) != 0;
}

bool is_inf(long long num) {
    return exp(num) == exp_max + 1 && man(num) == 0;
}

bool is_zero(long long num) {
    return exp(num) == exp_min && man(num) == 0 && num < 1ll << significand_bits + exponent_bits + 1;
}

long long inf(int sign) {
    long long inf = (1ll << exponent_bits) - 1;
    inf = inf << significand_bits;
    if (sign == 1) inf += 1ll << significand_bits + exponent_bits;
    return inf;
}

long long nan() {
    long long nan = (1ll << exponent_bits) - 1;
    nan = nan << significand_bits;
    nan++;
    return nan;
}

long long zero(int sign) {
    long long zero = 0;
    if (sign == 1) zero += 1ll << significand_bits + exponent_bits;
    return zero;
}

pair<long long, long long> stabilize_point(int sign, int exp, long long man, int extra_bits) {
    while (extra_bits) {
        if (man % 2 == 1 && sign == 1) man = (man >> 1) + 1;
        else man = man >> 1;
        extra_bits--;
    }
    pair<long long, long long> p;
    p.first = exp;
    p.second = man;
    return p;
}

long long normalize(int sign, int exp, long long man) {
    if (man == 0) {
        return zero(sign);
    }
    while (man >= (1ll << significand_bits + 1)) {
        if (man % 2 == 1 && sign == 1) man = (man >> 1) + 1;
        else man = man >> 1;
        if (exp < exp_max) exp++;
    }
    while (man < (1ll << significand_bits)) {
        exp--;
        man = man << 1;
    }
    man -= 1ll << significand_bits;
    exp += bias;
    if (exp > exp_max + bias) {
        exp = exp_max + bias + 1;
        man = 0;
    }
    long long normal_form = ((long long)(abs(exp)) << significand_bits) + man;
    if (sign == 1) normal_form += 1ll << (significand_bits + exponent_bits);
    if (exp < 0) normal_form += 1ll << significand_bits + exponent_bits + 1;
    else if (exp == 0) normal_form += 1ll << significand_bits + exponent_bits + 2;
    return normal_form;
}

long long f_sum(long long num1, long long num2) {
    if (is_nan(num1) || is_nan(num2)) return nan();
    int exp1 = exp(num1), exp2 = exp(num2), exp3;
    long long man1 = man(num1), man2 = man(num2), man3;
    if (exp1 < exp_min) exp1 = exp_min;
    if (exp2 < exp_min) exp2 = exp_min;
    int sign1 = 0, sign2 = 0, sign3;
    if (num1 >= (1ll << (significand_bits + exponent_bits))) sign1 = 1;
    if (num2 >= (1ll << (significand_bits + exponent_bits))) sign2 = 1;
    if (is_inf(num1) && is_inf(num2)) {
        if (sign1 == sign2) return inf(sign1);
        else return nan();
    }
    else if (is_inf(num1)) return inf(sign1);
    else if (is_inf(num2)) return inf(sign2);
    if (exp1 < exp2) {
        swap(num1, num2);
        swap(exp1, exp2);
        swap(man1, man2);
        swap(sign1, sign2);
    }
    exp3 = exp1;
    int diff = exp1 - exp2;
    if (diff > significand_bits + 1) return normalize(sign1, exp1, man1);
    else if (diff != 0) man1 = man1 << diff;
    if (sign1 == sign2) {
        man3 = man1 + man2;
        sign3 = sign1;
    }
    else {
        if (man1 >= man2) {
            sign3 = sign1;
            man3 = man1 - man2;
        }
        else {
            sign3 = sign2;
            man3 = man2 - man1;
        }
    }
    pair<long long, long long> exp_man = stabilize_point(sign3, exp3, man3, diff);
    exp3 = exp_man.first;
    man3 = exp_man.second;
    long long f = normalize(sign3, exp3, man3);
    return f;
}

int f_subtract(long long num1, long long num2) {
    if (is_nan(num1) || is_nan(num2)) return nan();
    if (sign(num2) == 1) num2 -= 1ll << (significand_bits + exponent_bits);
    else num2 += 1ll << (significand_bits + exponent_bits);
    return f_sum(num1, num2);
}

long long f_multiply(long long num1, long long num2) {
    if (is_nan(num1) || is_nan(num2)) return nan();
    int sign_, exponent;
    long long mantissa;
    exponent = exp(num1) + exp(num2);
    mantissa = man(num1) * man(num2);
    sign_ = sign(num1) ^ sign(num2);
    if (is_inf(num1) && is_inf(num2)) {
        return inf(sign_);
    }
    else if (is_inf(num1)) {
        if (is_zero(num2)) return nan();
        else return inf(sign_);
    }
    else if (is_inf(num2)) {
        if (is_zero(num1)) return nan();
        else return inf(sign_);
    }
    else if (is_zero(num1) && is_zero(num2)) {
        return zero(sign_);
    }
    pair<long long, long long> exp_man = stabilize_point(sign_, exponent, mantissa, significand_bits);
    return normalize(sign_, exp_man.first, exp_man.second);
}

long long f_divide(long long num1, long long num2) {
    if (is_nan(num1) || is_nan(num2)) return nan();
    int sign_, exponent;
    sign_ = sign(num1) ^ sign(num2);
    if (is_inf(num1)) {
        if (is_inf(num2)) return nan();
        else return inf(sign_);
    }
    else if (is_zero(num1)) {
        if (is_zero(num2)) return nan();
        else return zero(sign_);
    }
    else {
        if (is_zero(num2)) return inf(sign_);
        else if (is_inf(num2)) return zero(sign_);
    }
    long long mantissa;
    exponent = exp(num1) - exp(num2);
    mantissa = man(num1) * (1ll << significand_bits) / man(num2);
    return normalize(sign_, exponent, mantissa);
}

void float_print(long long num) {
    int exponent = exp(num), mantissa = man(num), sign = 0;
    if (num >= (1ll << significand_bits + exponent_bits) && num < 1ll << significand_bits + exponent_bits + 1) sign = 1;
    mantissa &= (1ll << significand_bits) - 1;
    string m;
    if (format == "h") {
        mantissa = mantissa << 2;
        m = dec_to_hex(mantissa >> 8) + dec_to_hex(mantissa >> 4 & 15) + dec_to_hex(mantissa);
    }
    else {
        mantissa = mantissa << 1;
        m = dec_to_hex(mantissa >> 20) + dec_to_hex(mantissa >> 16 & 15) + dec_to_hex(mantissa >> 12 & 15) +
            dec_to_hex(mantissa >> 8 & 15) + dec_to_hex(mantissa >> 4 & 15) + dec_to_hex(mantissa);
    }
    // Detects zero
    if (is_zero(num)) {
        if (sign == 1) cout << "-";
        cout << "0x0." << m << "p+0" << endl;
    }
    // Detects NaN or infinity
    else if (exponent == exp_max + 1) {
        if (mantissa == 0) {
            if (sign == 1) cout << "-";
            cout << "inf" << endl;
        }
        else {
            cout << "nan" << endl;
        }
    }
    else {
        int exp_sign = 0;
        if (num >= 1ll << significand_bits + exponent_bits + 1 && num < 1ll << significand_bits + exponent_bits + 2) exp_sign = 1;
        if (sign == 1) cout << "-";
        cout << "0x1." << m << "p";
        if (exp_sign == 1) {
            cout << "-" << abs(exponent + bias) + bias << endl;
        }
        else if (num >= 1ll << significand_bits + exponent_bits + 2) {
            cout << exp_min - 1 << endl;
        }
        else {
            if (exponent >= 0) cout << "+";
            cout << exponent << endl;;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc <= 3) {
        cerr << "Error: not enough arguments passed" << endl;
        return 1;
    }
    format = argv[1];
    string rounding = argv[2];
    if (rounding != "3") {
        cerr << "Error: incorrect type of rounding" << endl;
        return 1;
    }
    if (format.size() > 1) {
        string::size_type sz;
        try {
            int_part = stoi(format, &sz), frac_part = stoi(format.substr(sz + 1));
        }
        catch (const invalid_argument& e) {
            cerr << "Error: " << e.what() << endl;
            return 1;
        }
        catch (...) {
            cerr << "Unexpected error" << endl;
            return 1;
        }
        if (int_part < 1 || int_part + frac_part > 32) {
            cerr << "Error: invalid format" << endl;
            return 1;
        }
        if (to_string(int_part) + "." + to_string(frac_part) != format) {
            cerr << "Error: invalid format" << endl;
            return 1;
        }
        fixed_max_val = (1 << (int_part + frac_part - 1)) -  1;
        fixed_min_val = -(fixed_max_val + 1);
        if (argc < 5) {
            int num;
            try {
                num = stoi(argv[3], nullptr, 16);
            }
            catch (const invalid_argument& e) {
                cerr << "Error: " << e.what() << endl;
                return 1;
            }
            fixed_print(num);
        }
        else {
            string operation = argv[4];
            long long num1, num2;
            try {
                num1 = stoi(argv[3], nullptr, 16), num2 = stoi(argv[5], nullptr, 16);
            }
            catch (const invalid_argument& e) {
                cerr << "Error: " << e.what() << endl;
                return 1;
            }
            catch (...) {
                cerr << "Unexpected error" << endl;
                return 1;
            }
            num1 = correct_form(num1), num2 = correct_form(num2);
            if (operation == "+") {
                fixed_print(sum(num1, num2));
            }
            else if (operation == "-") {
                fixed_print(subtract(num1, num2));
            }
            else if (operation == "*") {
                fixed_print(multiply(num1, num2));
            }
            else if (operation == "/") {
                int res;
                try {
                    res = divide(num1, num2);
                }
                catch (const invalid_argument& e) {
                    cerr << e.what() << endl;
                    return 1;
                }
                fixed_print(res);
            }
            else {
                cerr << "Error: invalid operation" << endl;
                return 1;
            }
        }
    }
    else {
        if (format == "h") {
            significand_bits = 10;
            exponent_bits = 5;
            bias = 15;
            exp_min = -14;
            exp_max = 15;
        }
        else if (format == "f") {
            significand_bits = 23;
            exponent_bits = 8;
            bias = 127;
            exp_min = -126;
            exp_max = 127;
        }
        else {
            cerr << "Invalid format" << endl;
            return 1;
        }
        if (argc < 5) {
            long long num;
            try {
                num = stoll(argv[3], nullptr, 16);
            }
            catch (const invalid_argument& e) {
                cerr << "Error: " << e.what() << endl;
                return 1;
            }
            catch (...) {
                cerr << "Unexpected error" << endl;
                return 1;
            }
            if (!is_nan(num) && !is_inf(num)) num = normalize(sign(num), exp(num), man(num));
            float_print(num);
        }
        else {
            long long num1, num2;
            try {
                num1 = stoll(argv[3], nullptr, 16), num2 = stoll(argv[5], nullptr, 16);
            }
            catch (const invalid_argument& e) {
                cerr << "Error: " << e.what() << endl;
                return 1;
            }
            catch (...) {
                cerr << "Unexpected error" << endl;
                return 1;
            }
            string operation = argv[4];
            if (operation == "+") {
                float_print(f_sum(num1, num2));
            }
            else if (operation == "-") {
                float_print(f_subtract(num1, num2));
            }
            else if (operation == "*") {
                float_print(f_multiply(num1, num2));
            }
            else if (operation == "/") {
                float_print(f_divide(num1, num2));
            }
            else {
                cerr << "Error: invalid operation" << endl;
                return 1;
            }
        }
    }
}