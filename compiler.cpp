// g++ -o compiler compiler.cpp
// then:
// arm-none-eabi-as -mcpu=cortex-m3 -mthumb test.s -o test.o
// arm-none-eabi-ld -T linker.ld -o test.elf test.o
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <bits/stdc++.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <bits/stdc++.h>
using namespace std;


// ================= Helper functions =================

// Trim functions
static inline void ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(),
        [](unsigned char ch){ return !isspace(ch); }));
}
static inline void rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(),
        [](unsigned char ch){ return !isspace(ch); }).base(), s.end());
}
static inline string trim(const string &s) {
    string result = s;
    ltrim(result);
    rtrim(result);
    return result;
}

// Trim trailing comma
string trim_comma(const string &s) {
    if (!s.empty() && s.back() == ',') return s.substr(0,s.size()-1);
    return s;
}

// Remove leading % from SSA names
string sanitize_name(const string &s) {
    if (!s.empty() && s[0] == '%') return s.substr(1);
    return s;
}

// ================= Register mapping =================

unordered_map<string,int> ssa_to_reg;
int next_reg = 0;

int get_reg(const string &name) {
    string clean_name = sanitize_name(name);
    if (ssa_to_reg.count(clean_name)) return ssa_to_reg[clean_name];
    int r = next_reg++;
    ssa_to_reg[clean_name] = r;
    return r;
}

// ================= Assembly emitter =================

struct Emitter {
    vector<string> lines;
    void emit(const string &s) { lines.push_back(s); }
    void write(const string &fname) {
        ofstream f(fname);
        for (auto &l: lines) f << l << "\n";
    }
} emi;

// ================= Parser functions =================

void parse_store(const string &line) {
    istringstream iss(line);
    string tok, type, val, ptr, var;
    iss >> tok >> type >> val >> ptr >> var;

    val = trim_comma(val);
    var = sanitize_name(trim_comma(var));

    int tgt_reg = get_reg(var);

    // Check if val is a number or SSA variable
    if (!val.empty() && isdigit(val[0])) {
        emi.emit("    MOV r" + to_string(tgt_reg) + ", #" + val);
    } else {
        // val is another SSA variable
        int src_reg = get_reg(val);
        emi.emit("    MOV r" + to_string(tgt_reg) + ", r" + to_string(src_reg));
    }
}


void parse_load(const string &line) {
    istringstream iss(line);
    string target, eq, tok, type, ptr, var;
    iss >> target >> eq >> tok >> type >> ptr >> var;
    var = sanitize_name(trim_comma(var));
    int src_reg = get_reg(var);
    int tgt_reg = get_reg(target);
    emi.emit("    MOV r" + to_string(tgt_reg) + ", r" + to_string(src_reg));
}

void parse_arith(const string &line) {
    // Example: "%8 = add nsw i32 %5, %7"
    size_t eq_pos = line.find('=');
    string target = trim(line.substr(0, eq_pos));
    target = sanitize_name(target);

    string rhs = trim(line.substr(eq_pos+1));
    // Remove optional flags like 'nsw' or 'nuw'
    rhs = regex_replace(rhs, regex("\\b(nsw|nuw)\\b"), "");
    istringstream iss(rhs);
    string op, type, lhs, rhs_op;
    iss >> op >> type >> lhs >> rhs_op;
    lhs = sanitize_name(trim_comma(lhs));
    rhs_op = sanitize_name(trim_comma(rhs_op));

    int lhs_reg = get_reg(lhs);
    int rhs_reg = get_reg(rhs_op);
    int tgt_reg = get_reg(target);

    if (op == "add")
        emi.emit("    ADD r" + to_string(tgt_reg) + ", r" + to_string(lhs_reg) + ", r" + to_string(rhs_reg));
    else if (op == "mul")
        emi.emit("    MUL r" + to_string(tgt_reg) + ", r" + to_string(lhs_reg) + ", r" + to_string(rhs_reg));
}

void parse_ret(const string &line) {
    istringstream iss(line);
    string tok, type, val;
    iss >> tok >> type >> val;
    val = sanitize_name(val);
    int reg = get_reg(val);
    emi.emit("    MOV r2, r" + to_string(reg)); // r2 holds return value
}

void parse_line(const string &line) {
    string l = trim(line);
    if (l.empty() || l.back() == ':') return; // skip empty lines/labels
    if (l.find("store") != string::npos)
        parse_store(l);
    else if (l.find("= load") != string::npos)
        parse_load(l);
    else if (l.find("add") != string::npos || l.find("mul") != string::npos)
        parse_arith(l);
    else if (l.find("ret") != string::npos)
        parse_ret(l);
}

// ================= Main =================

int main() {
    ifstream in("main.ll");
    if (!in) { cerr << "Cannot open main.ll\n"; return 1; }

    // Vector table & Reset_Handler header
    emi.emit("    .syntax unified");
    emi.emit("    .cpu cortex-m3");
    emi.emit("    .thumb");
    emi.emit("");
    emi.emit("    .section .isr_vector, \"a\", %progbits");
    emi.emit("    .word 0x20001000");
    emi.emit("    .word Reset_Handler");
    emi.emit("");
    emi.emit("    .text");
    emi.emit("    .thumb_func");
    emi.emit("    .global Reset_Handler");
    emi.emit("Reset_Handler:");

    string line;
    while (getline(in,line)) parse_line(line);

    // Infinite loop
    emi.emit("1:");
    emi.emit("    B 1b");

    emi.write("test.s");
    cerr << "Emitted test.s\n";
}