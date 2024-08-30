#include <inc/lib.h>

int
esPrimo(int n)
{
	if (n <= 1)
		return 0;
	for (int i = 2; i * i <= n; i++) {
		if (n % i == 0)
			return 0;
	}
	return 1;
}

void
calcularPrimos(int inicio, int fin)
{
	for (int i = inicio; i <= fin; i++) {
		if (esPrimo(i)) {
			int dummy = i * i;
		}
	}
	cprintf("Tarea finalizada: %d - %d\n", inicio, fin);
}

void
umain(int argc, char **argv)
{
	calcularPrimos(1, 100000);
	calcularPrimos(100001, 200000);
	calcularPrimos(200001, 300000);
	calcularPrimos(300001, 400000);
	calcularPrimos(400001, 500000);
}