#pragma once
// ============================================================================
// ВАРИАНТ: Система расчета эффективности использования трудовых ресурсов
// Студент: Черкасова Елизавета Алексеевна, Группа: УИБО-02-25.
//
// Входные параметры:
// 1. volume (объем_продукции): от 0 до 10^12
// 2. staff_count (численность_персонала): от 1 до 10^6
// 3. actual_time (фактическое_время): от 1 до 8760 часов
// 4. normative_time (нормативное_время): от 1 до 8760 часов
//
// Выходные значения:
// 1. Производительность труда (на одного работника)
// 2. Коэффициент использования рабочего времени (%)
// 3. Резервы роста (%)
//
// ============================================================================
#include <sstream> 
#include <iomanip> 
#include <string>
#include <cmath>
#include "../../json.hpp"

namespace TaskVariant {

    // ------------------------------------------------------------------------
    // Структура входных данных. Соответствует полям JSON-запроса.
    // ------------------------------------------------------------------------
    struct Input {
        double volume;            // объем_продукции
        double staff_count;       // численность_персонала
        double actual_time;       // фактическое_время (часы)
        double normative_time;    // нормативное_время (часы)
    };

    // ------------------------------------------------------------------------
    // Структура выходных данных. Соответствует полям JSON-ответа.
    // ------------------------------------------------------------------------
    struct Output {
        double productivity;              // Производительность труда
        double time_utilization_coeff;    // Коэффициент использования рабочего времени (%)
        double growth_reserves;           // Резервы роста (%)

        // Форматированные строки для отображения
        std::string str_productivity;
        std::string str_time_utilization;
        std::string str_growth_reserves;
    };

    // ------------------------------------------------------------------------
    // Проверка входных данных.
    // Возвращает "" если всё ок, иначе — текст ошибки.
    // ------------------------------------------------------------------------
    inline std::string validate(const Input& in) 
    {
        // объем_продукции: от 0 до 10^12
        if (in.volume < 0.0 || in.volume > 1e12)
            return "volume must be between 0 and 10^12";

        // численность_персонала: от 1 до 10^6
        if (in.staff_count < 1.0 || in.staff_count > 1e6)
            return "staff_count must be between 1 and 10^6";

        // фактическое_время: от 1 до 8760 часов
        if (in.actual_time < 1.0 || in.actual_time > 8760.0)
            return "actual_time must be between 1 and 8760";

        // нормативное_время: от 1 до 8760 часов
        if (in.normative_time < 1.0 || in.normative_time > 8760.0)
            return "normative_time must be between 1 and 8760";

        return "";
    }

    // ------------------------------------------------------------------------
    // Основные расчёты.
    // Алгоритм:
    // 1. Расчет производительности труда на одного работника = volume / staff_count
    // 2. Определение коэффициента использования рабочего времени = (actual_time / normative_time) * 100
    // 3. Выявление резервов повышения эффективности = 100 - coefficient
    // 4. Сравнение с нормативными значениями 
    // 5. Форматирование с указанием потенциала улучшений
    // ------------------------------------------------------------------------
    inline Output calculate(const Input& in) {
        Output out{};

        // 1. Производительность труда
        out.productivity = in.volume / in.staff_count;

        // 2. Коэффициент использования рабочего времени (%)
        out.time_utilization_coeff = (in.actual_time / in.normative_time) * 100.0;

        // Норматив использования рабочего времени — 100%.
        // Если коэффициент < 100% — есть резерв роста.
        // Если коэффициент = 100% — норматив выполнен, резервов нет.
        // Если коэффициент > 100% — норматив перевыполнен, резервы принимаются равными 0.

       // 3. Резервы роста (%) — определяются на шаге 4 после сравнения с нормативом
       // 4.Сравнение с нормативом 100% и расчёт резервов
        if (out.time_utilization_coeff >= 100.0)
            out.growth_reserves = 0.0;                             // норматив выполнен/перевыполнен
        else
            out.growth_reserves = 100.0 - out.time_utilization_coeff; // есть резерв роста

        // ШАГ 5: Форматирование с указанием потенциала улучшений
        //std::ostringstream ss; — создаёт строковый поток: в него можно писать как в cout, но результат сохраняется в виде строки (через ss.str()).
        // ss << std::fixed; — включает «фиксированный» формат чисел
        // ss << std::setprecision(2); — задаёт ровно 2 знака после запятой для всех последующих чисел.
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss << "Производительность труда: " << out.productivity;
        out.str_productivity = ss.str();

        ss.str(""); // Очистка потока
        ss << "Коэффициент использования рабочего времени: " << out.time_utilization_coeff << "%";
        out.str_time_utilization = ss.str();

        ss.str(""); // Очистка потока
        ss << "Резервы роста: " << out.growth_reserves << "%";
        out.str_growth_reserves = ss.str();

        return out;
    }

    // ------------------------------------------------------------------------
    // Разбор JSON-тела запроса в Input.
    // Достаём числа по ключам. Простой парсер подходит для плоских JSON.
    // ------------------------------------------------------------------------
    inline Input parse(const std::string& body) {
        Input in{};

        auto get_d = [&](const std::string& key) -> double {
            auto p = body.find("\"" + key + "\"");
            if (p == std::string::npos) return 0.0;
            p = body.find(':', p);
            if (p == std::string::npos) return 0.0;
            try { return std::stod(body.substr(p + 1)); }
            catch (...) { return 0.0; }
        };

        in.volume = get_d("volume");
        in.staff_count = get_d("staff_count");
        in.actual_time = get_d("actual_time");
        in.normative_time = get_d("normative_time");
        return in;
    }

    // ------------------------------------------------------------------------
    // Формирование JSON-ответа из Output.
    // ------------------------------------------------------------------------
    inline std::string to_json(const Output& out) {
        JObj o;
        o.num("productivity", out.productivity);
        o.num("time_utilization_coeff", out.time_utilization_coeff);
        o.num("growth_reserves", out.growth_reserves);
        return o.str();
    }

} // namespace TaskVariant