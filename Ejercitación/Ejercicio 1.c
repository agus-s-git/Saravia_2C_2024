//Declare una constante de 32 bits, con todos los bits en 0 y el bit 6 en 1. Utilice el operador <<.

#include <stdio.h>
#include <stdint.h>

int main() {
    // Declaramos la constante de 32 bits con el bit 6 en 1 usando el operador <<
    const unsigned int BIT = 1 << 6;

    // Imprimimos el valor en formato decimal y hexadecimal
    printf("Valor de la constante: %u (decimal)\n", BIT);
    

    return 0;
}