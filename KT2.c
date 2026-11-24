#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define UCL              62.90   
#define X_NORMAL_MIN      5.0    
#define X_NORMAL_MAX     18.0    
#define X_RANGE_MIN       0.0   
#define X_RANGE_MAX      50.0  
#define N_MEASUREMENTS   70      
#define STAT_FROM        31      
#define STAT_TO          43     

#define STATE_NORMAL      0     
#define STATE_WARNING     1    
#define STATE_ALARM       2     

/* ==================== Прототипы (перед main) ==================== */
int    validate_X(double x);
double calculate_Q(double x);
int    check_control(double x, double q);
void   print_state(int state);
void   run_tests(void);

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
    int    mode;

#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
    setlocale(LC_ALL, "RUS");
    setlocale(LC_ALL, "");

    printf("==============================================\n");
    printf("  Цифровой двойник технологического процесса\n");
    printf("  Мониторинг содержания воды в реакторе\n");
    printf("  Вариант 55  |  КТ2: расчёт Q и базовый контроль\n");
    printf("==============================================\n");
    printf("Параметр X: содержание воды, %%\n");
    printf("Диапазон X: %.0f ... %.0f\n", X_RANGE_MIN, X_RANGE_MAX);
    printf("Штатный режим: %.0f–%.0f %%\n", X_NORMAL_MIN, X_NORMAL_MAX);
    printf("UCL = %.2f\n", UCL);
    printf("Состояния: НОРМА | ПРЕДУПРЕЖДЕНИЕ | ТРЕВОГА\n\n");

    printf("Выберите режим:\n");
    printf("  1 — полный цикл (%d измерений)\n", N_MEASUREMENTS);
    printf("  2 — тестовые случаи (КТ2: норма / граница / нарушение)\n");
    printf("Ваш выбор: ");
    if (scanf("%d", &mode) != 1) {
        mode = 2;
        while (getchar() != '\n' && !feof(stdin)) {}
    }

    if (mode == 2) {
        run_tests();
        return 0;
    }

    printf("\nКоличество измерений N = %d\n", N_MEASUREMENTS);
    printf("Статистика: среднее Q[%d..%d]\n\n", STAT_FROM, STAT_TO);

    for (i = 1; i <= N_MEASUREMENTS; i++) {
        printf("Введите содержание воды X[%d] (%%): ", i);
        ok = scanf("%lf", &x);

        if (ok != 1) {
            printf("  Ошибка ввода! Пропуск измерения.\n\n");
            while (getchar() != '\n' && !feof(stdin)) {}
            continue;
        }

        if (!validate_X(x)) {
            printf("  X = %.2f вне допустимого диапазона [%.0f; %.0f]. Пропуск.\n\n",
                x, X_RANGE_MIN, X_RANGE_MAX);
            continue;
        }

        q = calculate_Q(x);
        state = check_control(x, q);

        printf("  X = %.2f %%   Q = %.2f\n", x, q);
        print_state(state);
        printf("\n");

        if (i >= STAT_FROM && i <= STAT_TO) {
            sum_stat += q;
            count_stat++;
        }
    }

    average = (count_stat > 0) ? (sum_stat / (double)count_stat) : 0.0;

    printf("========== СТАТИСТИКА Q[%d..%d] ==========\n", STAT_FROM, STAT_TO);
    printf("Среднее значение Q = %.2f\n", average);
    printf("Медиана             = (на следующих этапах)\n");
    printf("==========================================\n");

    return 0;
}

/* ==================== Определения функций ==================== */

int validate_X(double x)
{
    if (x < X_RANGE_MIN || x > X_RANGE_MAX) {
        return 0;
    }
    return 1;
}

double calculate_Q(double x)
{
    double q;
    double under_sqrt;

    if (x < X_NORMAL_MIN) {
        q = 3.0 * (x - X_NORMAL_MIN) + 10.0;
    }
    else if (x <= X_NORMAL_MAX) {
        under_sqrt = x - 11.5 + 6.0;   /* X - 5.5 */
        if (under_sqrt < 0.0) {
            under_sqrt = 0.0;
        }
        q = 2.0 * sqrt(under_sqrt) + 45.0;
    }
    else {
        q = (x - X_NORMAL_MAX) * (x - X_NORMAL_MAX) / 2.0 + 55.0;
    }

    return q;
}

int check_control(double x, double q)
{
    if (q > UCL) {
        return STATE_ALARM;
    }
    if (x < X_NORMAL_MIN || x > X_NORMAL_MAX) {
        return STATE_WARNING;
    }
    return STATE_NORMAL;
}

void print_state(int state)
{
    if (state == STATE_ALARM) {
        printf("  СОСТОЯНИЕ: ТРЕВОГА (Q > UCL)\n");
    }
    else if (state == STATE_WARNING) {
        printf("  СОСТОЯНИЕ: ПРЕДУПРЕЖДЕНИЕ (отклонение режима)\n");
    }
    else {
        printf("  СОСТОЯНИЕ: НОРМА\n");
    }
}

void run_tests(void)
{
    double tests_x[] = { 16.70, 5.00, 18.00, 4.00, 25.00, 40.00 };
    const char* tests_name[] = {
        "норма (контрольный пример)",
        "нижняя граница режима",
        "верхняя граница режима",
        "нарушение режима (X < 5)",
        "нарушение режима (X > 18)",
        "возможная тревога (большой X)"
    };
    double expected_q[] = { 51.69, 45.00, 52.07, 7.00, 79.50, 297.00 };
    int n_tests = 6;
    int i;
    double q;
    int state;

    printf("\n========== ТЕСТЫ КТ2 ==========\n");
    printf("(X, Q программы, Q ручное, состояние)\n\n");

    for (i = 0; i < n_tests; i++) {
        double x = tests_x[i];

        if (!validate_X(x)) {
            printf("%d) %s\n", i + 1, tests_name[i]);
            printf("   X = %.2f — вне диапазона, пропуск\n\n", x);
            continue;
        }

        q = calculate_Q(x);
        state = check_control(x, q);

        printf("%d) %s\n", i + 1, tests_name[i]);
        printf("   X = %.2f %%\n", x);
        printf("   Q (программа) = %.2f\n", q);
        printf("   Q (ручной)    = %.2f\n", expected_q[i]);
        print_state(state);

        if (fabs(q - expected_q[i]) < 0.05) {
            printf("   Проверка: OK (совпадает с ручным расчётом)\n\n");
        }
        else {
            printf("   Проверка: РАСХОЖДЕНИЕ! Проверьте формулу.\n\n");
        }
    }

    printf("================================\n");
    printf("Финиш КТ2: функция Q работает; контроль реализован;\n");
    printf("значения Q и состояния сверены с ручным расчётом.\n");
}