#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <locale.h>
#define UCL 64.32
#define X_MIN 10
#define X_MAX 70
#define N 50
#define N_STAT 20
double calculate_q(double x);
int check_control(double q);
double calculate_mean(double q[]);
double calculate_median(double q[]);

int main() {
    setlocale(LC_ALL, "RUS");

    double x, q;
    double q_values[N];

    printf("Программа мониторинга температуры реактора\n");
    printf("Введите 50 значений температуры сырья.\n\n");

    for (int i = 0; i < N; i++) {

        printf("Измерение %d из %d\n", i + 1, N);
        printf("Введите температуру X (от 10 до 70): ");
        scanf("%lf", &x);

        if (x < X_MIN || x > X_MAX) {
            printf("Ошибка! Температура вне диапазона.\n");
            i--;
            continue;
        }

        q = calculate_q(x);
        q_values[i] = q;

        printf("Рассчитанное Q: %.2f\n", q);

        if (x < 25 || x > 45) {
            printf("Технологическое отклонение!\n");
        }
        else {
            printf("Штатный технологический режим.\n");
        }

        if (check_control(q)) {
            printf("Авария! Превышен предел UCL.\n");
        }
        else {
            printf("Контроль UCL: норма.\n");
        }

        printf("------------------------------\n");
    }

    printf("\nСтатистика первых 20 измерений:\n");

    printf("Среднее значение Q: %.2f\n",
        calculate_mean(q_values));

    printf("Медиана Q: %.2f\n",
        calculate_median(q_values));

    return 0;
}
double calculate_q(double x) {
    double q;

    if (x < 25) {
        q = 3 * (x - 25) + 10;
    }
    else if (x <= 45) {
        q = 2 * sqrt(x - 25) + 45;
    }
    else {
        q = (x - 45) * (x - 45) / 4 + 55;
    }

    return q;
}
int check_control(double q) {
    if (q > UCL) {
        return 1;
    }
    else {
        return 0;
    }
}
double calculate_mean(double q[]) {
    double sum = 0;

    for (int i = 0; i < N_STAT; i++) {
        sum = sum + q[i];
    }

    return sum / N_STAT;
}
double calculate_median(double q[]) {
    double a[N_STAT];
    for (int i = 0; i < N_STAT; i++) {
        a[i] = q[i];
    }
    for (int i = 0; i < N_STAT - 1; i++) {
        for (int j = 0; j < N_STAT - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                double temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
    return (a[9] + a[10]) / 2;
}