#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <locale.h>
int main() {
    setlocale(LC_ALL, "RUS");

    double X[50], Q[50];
    double sum = 0, average, median, temp;
    int i, j;

    for (i = 0; i < 50; i++) {
        printf("Введите температуру X: ");
        scanf("%lf", &X[i]);

        if (X[i] < 25) {
            Q[i] = 3 * (X[i] - 25) + 10;
        }
        else if (X[i] <= 45) {
            Q[i] = 2 * sqrt(X[i] - 25) + 45;
        }
        else {
            Q[i] = (X[i] - 45) * (X[i] - 45) / 4 + 55;
        }

        printf("Q = %.2f\n", Q[i]);

        if (Q[i] > 64.32) {
            printf("Превышение контрольной границы\n");
        }
        else {
            printf("В пределах контрольной границы\n");
        }

        if (i < 20) {
            sum = sum + Q[i];
        }
    }

    average = sum / 20;

    for (i = 0; i < 20; i++) {
        for (j = i + 1; j < 20; j++) {
            if (Q[i] > Q[j]) {
                temp = Q[i];
                Q[i] = Q[j];
                Q[j] = temp;
            }
        }
    }

    median = (Q[9] + Q[10]) / 2;

    printf("\nСреднее значение = %.2f\n", average);
    printf("Медиана = %.2f\n", median);

    return 0;
}