#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <locale.h>
double read_temperature() {
	double x;
	printf("Введите значение температуры сырья X (°C): ");
	scanf("%lf", &x);
	return x;
}

double calculate_Q(double x) {
	double q = 0.0;

	if (x < 25) {
		q = 3 * (x - 25) + 10;
	}
	else if (x <= 45) {
		q = 2 * sqrt(x - 25) + 45;
	}
	else {
		q = ((x - 45) * (x - 45)) / 4.0 + 55;
	}

	return q;
}

void check_control(double q) {
	double ucl = 64.32;
	if (q > ucl) {
		printf("Аварийное состояние: превышен предел UCL!\n");
	}
	else {
		printf("Штатный режим: параметры в норме.\n");
	}
}

void print_state(double x, double q) {
	printf("----------------------------------------\n");
	printf("Входное X: %.2f °C\n", x);
	printf("Рассчитанное Q: %.2f\n", q);
	check_control(q);
	printf("----------------------------------------\n");
}

void print_statistics_stub() {
	printf("[Статистика] Блок расчета медианы и среднего значения по окну Q[1..20] будет реализован в КТ4.\n");
}

int main() {
	setlocale(LC_ALL, "RUS");
	double x = read_temperature();
	double q = calculate_Q(x);
	print_state(x, q);
	print_statistics_stub();

	return 0;
}