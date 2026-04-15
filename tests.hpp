#pragma once

#include <string>
#include <vector>

namespace ChickenTests {

    // Hlavní funkce, která spustí všechny testy
    // Vrací true, pokud všechny testy prošly
    bool run_all_tests();

    // Jednotlivé testy
    bool test_config();
    bool test_assets_existence();
    bool test_math_splines();
    bool test_math_vectors();
    bool test_file_permissions();
    bool test_shader_syntax_basic();
    bool test_window_config();
    
    // Pomocná funkce pro hlášení výsledků
    void log_info(const std::string& message);
    void log_error(const std::string& message);

}
