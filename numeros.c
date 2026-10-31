#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>

int main() {
    // ============ ENTEROS (con signo) ============
    // Decimal
    short s_dec = -32768;
    int i_dec = -2147483648;
    long l_dec = -9223372036854775807L;
    long long ll_dec = -9223372036854775807LL;
    
    // Binario (C99/C11, GNU extension)
    short s_bin = 0b1000000000000000;      // -32768
    int i_bin = 0b10000000000000000000000000000000; // -2147483648
    long l_bin = 0b1000000000000000000000000000000000000000000000000000000000000000L;
    
    
    // Octal
    short s_oct = 0100000;     // 32768 en decimal, pero en short es negativo
    int i_oct = 020000000000;  // 2147483648 -> desborda a negativo
    long l_oct = 01000000000000000000000L;
    
    // Hexadecimal
    short s_hex = 0x8000;
    int i_hex = 0x80000000;
    long l_hex = 0x8000000000000000L;

    // ============ ENTEROS SIN SIGNO ============
    unsigned short us_dec = 65535;
    unsigned int ui_dec = 4294967295U;
    unsigned long ul_dec = 18446744073709551615UL;
    unsigned long long ull_dec = 18446744073709551615ULL;
    
    unsigned short us_bin = 0b1111111111111111;
    unsigned int ui_bin = 0b11111111111111111111111111111111;
    unsigned short us_oct = 0177777;
    unsigned int ui_oct = 037777777777;
    unsigned short us_hex = 0xFFFF;
    unsigned int ui_hex = 0xFFFFFFFF;

    // ============ PUNTO FLOTANTE ============
    float f_dec = -3.402823466e+38F;
    float f_hex = 0x1.fffffep+127F;  // máximo float (hexadecimal)
    float f_inf = 1.0f / 0.0f;
    float f_nan = 0.0f / 0.0f;
    
    double d_dec = -1.7976931348623157e+308;
    double d_hex = 0x1.fffffffffffffp+1023;  // máximo double
    long double ld_dec = -1.7976931348623157E+301L;
    long double ld_hex = 0x1.ffffffffffffffffp+301L;

    // ============ VERIFICACIÓN ============
    printf("=== ENTEROS CON SIGNA ===\n");
    printf("short dec: %hd, bin: %hd, oct: %ho, hex: %hx\n", s_dec, s_bin, s_oct, s_hex);
    printf("int dec: %d, bin: %d, oct: %o, hex: %x\n", i_dec, i_bin, i_oct, i_hex);
    printf("long dec: %ld, bin: %ld, oct: %lo, hex: %lx\n", l_dec, l_bin, l_oct, l_hex);
    printf("long long dec: %lld\n\n", ll_dec);

    printf("=== ENTEROS SIN SIGNO ===\n");
    printf("ushort dec: %hu, bin: %hu, oct: %ho, hex: %hx\n", us_dec, us_bin, us_oct, us_hex);
    printf("uint dec: %u, bin: %u, oct: %o, hex: %x\n", ui_dec, ui_bin, ui_oct, ui_hex);
    printf("ulong dec: %lu\n", ul_dec);
    printf("ull dec: %llu\n\n", ull_dec);

    printf("=== PUNTO FLOTANTE ===\n");
    printf("float (dec): %e\n", f_dec);
    printf("float (hex): %a\n", f_hex);
    printf("float inf: %f, nan: %f\n", f_inf, f_nan);
    printf("double (dec): %e\n", d_dec);
    printf("double (hex): %a\n", d_hex);
    printf("long double (dec): %Le\n", ld_dec);
    printf("long double (hex): %La\n", ld_hex);

    printf("\n=== TAMAÑOS (bytes) ===\n");
    printf("short: %zu, int: %zu, long: %zu, llong: %zu\n", 
           sizeof(short), sizeof(int), sizeof(long), sizeof(long long));
    printf("float: %zu, double: %zu, long double: %zu\n",
           sizeof(float), sizeof(double), sizeof(long double));
    printf("ushort: %zu, uint: %zu, ulong: %zu, ull: %zu\n",
           sizeof(unsigned short), sizeof(unsigned int), 
           sizeof(unsigned long), sizeof(unsigned long long));

    return 0;
}
