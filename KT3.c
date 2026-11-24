#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <math.h>
#include <locale.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* ==================== Константы (вариант 55) ==================== */
#define UCL_DEFAULT      62.90
#define X_NORMAL_MIN      5.0
#define X_NORMAL_MAX     18.0
#define X_RANGE_MIN       0.0
#define X_RANGE_MAX      50.0
#define STAT_FROM        31      /* начало окна статистики (1-based) */
#define STAT_TO          43      /* конец окна статистики (1-based) */
#define STAT_SIZE        13      /* 43 - 31 + 1 */
#define MAX_LINE         128
#define DEFAULT_DATA     "data.txt"
#define DEFAULT_LOG      "log.txt"

#define STATE_NORMAL      0
#define STATE_WARNING     1
#define STATE_ALARM       2

#define PROG_INIT         0
#define PROG_WORK         1
#define PROG_END          2

/* ==================== Прототипы ==================== */
int    validate_X(double x);
double calculate_Q(double x);
int    check_control(double x, double q, double ucl);
const char* state_name(int state);
const char* prog_state_name(int prog);
void   print_banner(double ucl);
double calc_average(double sum, int count);
double calc_median(double* window, int count);  /* только окно 13 значений */
void   sort_window(double* a, int n);
int    process_file(const char* data_path, const char* log_path, double ucl);

/* ==================== main ==================== */
int main(void)
{
    char data_path[MAX_LINE] = DEFAULT_DATA;
    char log_path[MAX_LINE] = DEFAULT_LOG;
    double ucl = UCL_DEFAULT;
    int choice;
    int prog = PROG_INIT;

#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
    setlocale(LC_ALL, "RUS");
    setlocale(LC_ALL, "");

    print_banner(ucl);
    printf("Состояние программы: %s\n\n", prog_state_name(prog));

    printf("=== Настройка параметров (КТ3) ===\n");
    printf("1 — использовать значения по умолчанию\n");
    printf("    (файл: %s, лог: %s, UCL = %.2f)\n", data_path, log_path, ucl);
    printf("2 — задать свои параметры\n");
    printf("Ваш выбор: ");
    if (scanf("%d", &choice) != 1) {
        choice = 1;
        while (getchar() != '\n' && !feof(stdin)) {}
    }
    while (getchar() != '\n' && !feof(stdin)) {}

    if (choice == 2) {
        printf("Имя входного файла [%s]: ", DEFAULT_DATA);
        if (fgets(data_path, MAX_LINE, stdin) != NULL) {
            data_path[strcspn(data_path, "\n")] = 0;
            if (data_path[0] == '\0') strcpy(data_path, DEFAULT_DATA);
        }
        printf("Имя файла лога [%s]: ", DEFAULT_LOG);
        if (fgets(log_path, MAX_LINE, stdin) != NULL) {
            log_path[strcspn(log_path, "\n")] = 0;
            if (log_path[0] == '\0') strcpy(log_path, DEFAULT_LOG);
        }
        printf("UCL [%.2f]: ", UCL_DEFAULT);
        if (scanf("%lf", &ucl) != 1) {
            ucl = UCL_DEFAULT;
            while (getchar() != '\n' && !feof(stdin)) {}
        }
    }

    printf("\nПараметры:\n");
    printf("  Входной файл : %s\n", data_path);
    printf("  Файл лога    : %s\n", log_path);
    printf("  UCL          : %.2f\n", ucl);
    printf("  Штатный режим: %.0f–%.0f %%\n", X_NORMAL_MIN, X_NORMAL_MAX);
    printf("  Статистика   : Q[%d..%d]\n\n", STAT_FROM, STAT_TO);

    prog = PROG_WORK;
    printf("Состояние программы: %s\n", prog_state_name(prog));
    printf("----------------------------------------------\n");

    if (process_file(data_path, log_path, ucl) != 0) {
        printf("\nОшибка при обработке файла.\n");
        prog = PROG_END;
        printf("Состояние программы: %s\n", prog_state_name(prog));
        return 1;
    }

    prog = PROG_END;
    printf("----------------------------------------------\n");
    printf("Состояние программы: %s\n", prog_state_name(prog));
    printf("Лог записан в файл: %s\n", log_path);

    return 0;
}

/* ==================== Функции ==================== */

void print_banner(double ucl)
{
    printf("==============================================\n");
    printf("  Цифровой двойник технологического процесса\n");
    printf("  Мониторинг содержания воды в реакторе\n");
    printf("  Вариант 55  |  КТ3: интерфейс и чтение из файла\n");
    printf("==============================================\n");
    printf("Параметр X: содержание воды, %%\n");
    printf("Диапазон X: %.0f ... %.0f\n", X_RANGE_MIN, X_RANGE_MAX);
    printf("Штатный режим: %.0f–%.0f %%\n", X_NORMAL_MIN, X_NORMAL_MAX);
    printf("UCL (по умолчанию) = %.2f\n", ucl);
    printf("Состояния: НОРМА | ПРЕДУПРЕЖДЕНИЕ | ТРЕВОГА\n");
    printf("==============================================\n\n");
}

const char* prog_state_name(int prog)
{
    if (prog == PROG_INIT) return "ИНИЦИАЛИЗАЦИЯ";
    if (prog == PROG_WORK) return "РАБОТА";
    return "КОНЕЦ (ОСТАНОВ)";
}

const char* state_name(int state)
{
    if (state == STATE_ALARM)   return "ТРЕВОГА";
    if (state == STATE_WARNING) return "ПРЕДУПРЕЖДЕНИЕ";
    return "НОРМА";
}

int validate_X(double x)
{
    if (x < X_RANGE_MIN || x > X_RANGE_MAX) return 0;
    return 1;
}

double calculate_Q(double x)
{
    double q, under_sqrt;

    if (x < X_NORMAL_MIN) {
        q = 3.0 * (x - X_NORMAL_MIN) + 10.0;
    }
    else if (x <= X_NORMAL_MAX) {
        under_sqrt = x - 11.5 + 6.0;
        if (under_sqrt < 0.0) under_sqrt = 0.0;
        q = 2.0 * sqrt(under_sqrt) + 45.0;
    }
    else {
        q = (x - X_NORMAL_MAX) * (x - X_NORMAL_MAX) / 2.0 + 55.0;
    }
    return q;
}

int check_control(double x, double q, double ucl)
{
    if (q > ucl) return STATE_ALARM;
    if (x < X_NORMAL_MIN || x > X_NORMAL_MAX) return STATE_WARNING;
    return STATE_NORMAL;
}

/* Среднее без массива: только сумма и счётчик */
double calc_average(double sum, int count)
{
    if (count <= 0) {
        return 0.0;   /* защита от деления на 0 → не будет nan */
    }
    return sum / (double)count;
}

/* Простая сортировка пузырьком для окна из 13 элементов */
void sort_window(double* a, int n)
{
    int i, j;
    double t;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                t = a[j];
                a[j] = a[j + 1];
                a[j + 1] = t;
            }
        }
    }
}

double calc_median(double* window, int count)
{
    double tmp[STAT_SIZE];
    int i;

    if (count <= 0) {
        return 0.0;
    }
    if (count > STAT_SIZE) {
        count = STAT_SIZE;
    }

    for (i = 0; i < count; i++) {
        tmp[i] = window[i];
    }
    sort_window(tmp, count);

    if (count % 2 == 1) {
        return tmp[count / 2];
    }
    return (tmp[count / 2 - 1] + tmp[count / 2]) / 2.0;
}

int process_file(const char* data_path, const char* log_path, double ucl)
{
    FILE* fin, * flog;
    double x, q;
    int state;
    int n = 0, n_ok = 0, n_skip = 0;
    int n_norm = 0, n_warn = 0, n_alarm = 0;
    char line[MAX_LINE];

    /* Статистика Q[31..43] без большого массива */
    double sum_stat = 0.0;
    int    count_stat = 0;
    double window[STAT_SIZE];  /* только 13 значений для медианы */
    double average, median;

    fin = fopen(data_path, "r");
    if (fin == NULL) {
        printf("Не удалось открыть файл данных: %s\n", data_path);
        return -1;
    }

    flog = fopen(log_path, "w");
    if (flog == NULL) {
        printf("Не удалось открыть файл лога: %s\n", log_path);
        fclose(fin);
        return -1;
    }

    fprintf(flog, "Лог мониторинга (вариант 55, КТ3)\n");
    fprintf(flog, "UCL = %.2f | Штатный режим: %.0f–%.0f %%\n",
        ucl, X_NORMAL_MIN, X_NORMAL_MAX);
    fprintf(flog, "----------------------------------------------\n");
    fprintf(flog, "%-6s %-10s %-10s %-16s %s\n",
        "N", "X", "Q", "Состояние", "Причина");
    fprintf(flog, "----------------------------------------------\n");

    printf("%-6s %-10s %-10s %-16s %s\n",
        "N", "X", "Q", "Состояние", "Причина");
    printf("----------------------------------------------\n");

    while (fgets(line, MAX_LINE, fin) != NULL) {
        if (sscanf(line, "%lf", &x) != 1) continue;
        n++;

        if (!validate_X(x)) {
            n_skip++;
            printf("%-6d %-10.2f %-10s %-16s %s\n",
                n, x, "—", "ПРОПУСК", "X вне диапазона 0..50");
            fprintf(flog, "%-6d %-10.2f %-10s %-16s %s\n",
                n, x, "—", "ПРОПУСК", "X вне диапазона 0..50");
            continue;
        }

        q = calculate_Q(x);
        state = check_control(x, q, ucl);
        n_ok++;

        /* Накопление статистики для Q[31..43] */
        if (n >= STAT_FROM && n <= STAT_TO && count_stat < STAT_SIZE) {
            sum_stat += q;
            window[count_stat] = q;
            count_stat++;
        }

        if (state == STATE_ALARM) {
            n_alarm++;
            printf("%-6d %-10.2f %-10.2f %-16s %s\n",
                n, x, q, state_name(state), "Q > UCL");
            fprintf(flog, "%-6d %-10.2f %-10.2f %-16s %s\n",
                n, x, q, state_name(state), "Q > UCL");
        }
        else if (state == STATE_WARNING) {
            n_warn++;
            printf("%-6d %-10.2f %-10.2f %-16s %s\n",
                n, x, q, state_name(state), "X вне 5–18 %");
            fprintf(flog, "%-6d %-10.2f %-10.2f %-16s %s\n",
                n, x, q, state_name(state), "X вне 5–18 %");
        }
        else {
            n_norm++;
            printf("%-6d %-10.2f %-10.2f %-16s %s\n",
                n, x, q, state_name(state), "в норме");
            fprintf(flog, "%-6d %-10.2f %-10.2f %-16s %s\n",
                n, x, q, state_name(state), "в норме");
        }
    }

    average = calc_average(sum_stat, count_stat);
    median = calc_median(window, count_stat);

    printf("----------------------------------------------\n");
    printf("Итого: прочитано %d | обработано %d | пропущено %d\n", n, n_ok, n_skip);
    printf("  НОРМА: %d | ПРЕДУПРЕЖДЕНИЕ: %d | ТРЕВОГА: %d\n", n_norm, n_warn, n_alarm);

    printf("\n========== СТАТИСТИКА Q[%d..%d] ==========\n", STAT_FROM, STAT_TO);
    if (count_stat == 0) {
        printf("Нет данных в окне (в файле меньше %d измерений).\n", STAT_TO);
    }
    else {
        printf("Число значений в окне: %d\n", count_stat);
        printf("Среднее значение Q = %.2f\n", average);
        printf("Медиана Q          = %.2f\n", median);
    }
    printf("==========================================\n");

    fprintf(flog, "----------------------------------------------\n");
    fprintf(flog, "Итого: прочитано %d | обработано %d | пропущено %d\n", n, n_ok, n_skip);
    fprintf(flog, "  НОРМА: %d | ПРЕДУПРЕЖДЕНИЕ: %d | ТРЕВОГА: %d\n", n_norm, n_warn, n_alarm);
    fprintf(flog, "\n========== СТАТИСТИКА Q[%d..%d] ==========\n", STAT_FROM, STAT_TO);
    if (count_stat == 0) {
        fprintf(flog, "Нет данных в окне.\n");
    }
    else {
        fprintf(flog, "Число значений в окне: %d\n", count_stat);
        fprintf(flog, "Среднее значение Q = %.2f\n", average);
        fprintf(flog, "Медиана Q          = %.2f\n", median);
    }
    fprintf(flog, "==========================================\n");

    fclose(fin);
    fclose(flog);
    return 0;
}