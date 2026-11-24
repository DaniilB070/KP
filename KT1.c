#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#endif

/* ==================== Константы (вариант 55) ==================== */
#define UCL              62.90   
#define X_NORMAL_MIN      5.0    
#define X_NORMAL_MAX     18.0   
#define X_RANGE_MIN       0.0    
#define X_RANGE_MAX      50.0   
#define N_MEASUREMENTS   70      
#define STAT_FROM        31     
#define STAT_TO          43  

#define STATE_NORMAL      0
#define STATE_ALARM       1

/* ==================== Прототипы (перед main) ==================== */
double calculate_Q(double x);
int    check_control(double q);
double calc_average(double sum, int count);

/* ==================== main ==================== */
int main(void)
{
    double x = 0.0;
    double q = 0.0;
    int    state = 0;
    double sum_stat = 0.0;
    int    count_stat = 0;
    double average = 0.0;
    int    i;
    int    ok;

#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
    setlocale(LC_ALL, "RUS");
    setlocale(LC_ALL, "");

    /* Приветствие с назначением программы */
    printf("==============================================\n");
    printf("  Цифровой двойник технологического процесса\n");
    printf("  Мониторинг содержания воды в реакторе\n");
    printf("  Вариант 55\n");
    printf("==============================================\n");
    printf("Параметр X: содержание воды, %%\n");
    printf("Диапазон X: %.0f ... %.0f\n", X_RANGE_MIN, X_RANGE_MAX);
    printf("Штатный режим: %.0f–%.0f %%\n", X_NORMAL_MIN, X_NORMAL_MAX);
    printf("UCL = %.2f\n", UCL);
    printf("Количество измерений N = %d\n", N_MEASUREMENTS);
    printf("Статистика: Q[%d..%d] (среднее; медиана — заглушка КТ1)\n\n",
        STAT_FROM, STAT_TO);

    /* Цикл по N измерениям БЕЗ массивов */
    for (i = 1; i <= N_MEASUREMENTS; i++) {
        printf("Введите содержание воды X[%d] (%%): ", i);
        ok = scanf("%lf", &x);

        if (ok != 1) {
            printf("  Ошибка ввода! Берём X = 10.0\n");
            x = 10.0;
            while (getchar() != '\n' && !feof(stdin)) {}
        }

        q = calculate_Q(x);
        state = check_control(q);

        printf("  X = %.2f %%   Q = %.2f\n", x, q);

        if (x < X_NORMAL_MIN || x > X_NORMAL_MAX) {
            printf("  Технологическое отклонение (вне 5–18 %%)\n");
        }
        else {
            printf("  Штатный технологический режим\n");
        }

        if (state == STATE_ALARM) {
            printf("  СОСТОЯНИЕ: АВАРИЯ (Q > UCL)\n\n");
        }
        else {
            printf("  СОСТОЯНИЕ: НОРМА\n\n");
        }

        if (i >= STAT_FROM && i <= STAT_TO) {
            sum_stat += q;
            count_stat++;
        }
    }

    average = calc_average(sum_stat, count_stat);

    printf("========== СТАТИСТИКА Q[%d..%d] ==========\n", STAT_FROM, STAT_TO);
    printf("Среднее значение Q = %.2f\n", average);
    printf("Медиана             = (для медианы нужно хранить окно из 13 значений);\n");
    printf("==========================================\n");

    return 0;
}

/* ==================== Определения функций ==================== */
double calculate_Q(double x)
{
    double q;

    if (x < X_NORMAL_MIN) {
        q = 3.0 * (x - X_NORMAL_MIN) + 10.0;
    }
    else if (x <= X_NORMAL_MAX) {
        q = 2.0 * sqrt(x - 11.5 + 6.0) + 45.0;
    }
    else {
        q = (x - X_NORMAL_MAX) * (x - X_NORMAL_MAX) / 2.0 + 55.0;
    }

    return q;
}

int check_control(double q)
{
    if (q > UCL) {
        return STATE_ALARM;
    }
    return STATE_NORMAL;
}

double calc_average(double sum, int count)
{
    if (count <= 0) {
        return 0.0;
    }
    return sum / (double)count;
}