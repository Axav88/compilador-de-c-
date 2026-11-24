int calcular(int a, int b) {
    int resultado = a + b;
    if (resultado > 10) {
        resultado = resultado * 2;
    } else {
        resultado = resultado - 1;
    }

    while (resultado < 50) {
        resultado = resultado + 5;
    }

    return resultado;
}

int main() {
    int x = 5;
    int y = 7;
    int res = calcular(x, y);
    return 0;
}
