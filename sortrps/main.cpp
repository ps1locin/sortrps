#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <limits>
#include <locale.h>

#include "merge_sort.h"

using std::cin;
using std::cout;
using std::string;
using std::vector;

namespace
{
    void clear_input()
    {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    int read_int(const string& prompt)
    {
        while (true)
        {
            cout << prompt;
            int value;
            if (cin >> value)
                return value;

            cout << "Ошибка: введите целое число.\n";
            clear_input();
        }
    }

    int read_int_in_range(const string& prompt, int min_val, int max_val)
    {
        while (true)
        {
            int v = read_int(prompt);
            if (v >= min_val && v <= max_val)
                return v;

            cout << "Ошибка: число должно быть в диапазоне ["
                << min_val << "; " << max_val << "].\n";
        }
    }

    vector<int> input_from_keyboard()
    {
        int n = read_int_in_range("Введите размер массива (n > 0): ", 1, 1000000);

        vector<int> a;
        a.reserve(n);

        cout << "Введите " << n << " целых чисел:\n";
        for (int i = 0; i < n; ++i)
        {
            a.push_back(read_int("  a[" + std::to_string(i) + "] = "));
        }

        return a;
    }

    vector<int> generate_random_array()
    {
        int n = read_int_in_range("Введите размер массива (n > 0): ", 1, 1000000);
        int lo = read_int("Введите минимальное значение: ");
        int hi = read_int("Введите максимальное значение: ");

        while (hi < lo)
        {
            cout << "Ошибка: максимум должен быть >= минимума.\n";
            lo = read_int("Введите минимальное значение: ");
            hi = read_int("Введите максимальное значение: ");
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(lo, hi);

        vector<int> a(n);
        for (int i = 0; i < n; ++i)
            a[i] = dist(gen);

        return a;
    }

    vector<int> load_from_file()
    {
        cout << "Введите путь к файлу: ";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        string path;
        std::getline(cin, path);

        std::ifstream in(path);
        while (!in.is_open())
        {
            cout << "Ошибка: не удалось открыть файл. Введите путь ещё раз:\n";
            std::getline(cin, path);
            in.open(path);
        }

        vector<int> a;
        int x;
        while (in >> x)
            a.push_back(x);

        if (a.empty())
            cout << "Внимание: файл пустой или не содержит целых чисел.\n";

        return a;
    }

    void print_array(const vector<int>& a, const string& title)
    {
        cout << "\n" << title << " (size=" << a.size() << "):\n";

        if (a.empty())
        {
            cout << "  <empty>\n";
            return;
        }

        for (int v : a)
            cout << v << " ";

        cout << "\n";
    }

    void save_arrays_to_file(const vector<int>& original, const vector<int>& sorted)
    {
        cout << "\nСохранить массив в файл?\n";
        cout << "1) Только исходный\n";
        cout << "2) Только отсортированный\n";
        cout << "3) Оба массива\n";
        cout << "4) Не сохранять\n";

        int choice = read_int_in_range("Ваш выбор: ", 1, 4);

        if (choice == 4)
            return;

        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        cout << "Введите путь для сохранения: ";
        string path;
        std::getline(cin, path);

        std::ofstream out(path);
        if (!out.is_open())
        {
            cout << "Ошибка: не удалось открыть файл для записи.\n";
            return;
        }

        if (choice == 1 || choice == 3)
        {
            out << "Original (" << original.size() << "):\n";
            for (int v : original)
                out << v << " ";
            out << "\n\n";
        }

        if (choice == 2 || choice == 3)
        {
            out << "Sorted (" << sorted.size() << "):\n";
            for (int v : sorted)
                out << v << " ";
            out << "\n";
        }

        cout << "Сохранено в файл: " << path << "\n";
    }
}

int main()
{
    setlocale(LC_ALL, "Russian");

    while (true)
    {
        cout << "\n==============================\n";
        cout << "Программа сортировки (Merge Sort)\n";
        cout << "0) Выход\n";
        cout << "1) Ввод с клавиатуры\n";
        cout << "2) Генерация случайных чисел\n";
        cout << "3) Загрузка массива из файла\n";

        int mode = read_int_in_range("Ваш выбор: ", 0, 3);

        if (mode == 0)
        {
            cout << "Выход из программы...\n";
            break;
        }

        vector<int> arr;

        if (mode == 1) arr = input_from_keyboard();
        if (mode == 2) arr = generate_random_array();
        if (mode == 3) arr = load_from_file();

        if (arr.empty())
        {
            cout << "Массив пустой — сортировать нечего.\n";
            continue;
        }

        vector<int> original = arr;

        print_array(original, "Изначальный массив");

        sorting::merge_sort(arr);

        print_array(arr, "Отсортированный массив");

        save_arrays_to_file(original, arr);

        cout << "\nНажмите Enter для возврата в меню...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cin.get();
    }

    return 0;
}