//The code to compile into ARM assembly:
// 1. clang -S -emit-llvm -target arm-none-eabi -mcpu=cortex-m3 -mthumb main.c -o main.ll


int main() {
    int a = 2;
    int b = 3;
    int c = a + b * 4;
    return c;
}

