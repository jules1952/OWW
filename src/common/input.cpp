#include "input.hpp"

#include <charconv>
#include <cmath>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>

namespace pmedian {
namespace {

// Śledzi wiersz zgodnie z formatem v1, żeby brak pola nie przesunął odczytu.
class TextInput {
public:
    explicit TextInput(const std::filesystem::path& path)
        : path_(path.u8string()), stream_(path, std::ios::binary) {
        if (!stream_) {
            fail("nie można otworzyć pliku");
        }
    }

    std::istringstream row() {
        ++line_;
        std::string text;
        if (!std::getline(stream_, text)) {
            fail(stream_.bad() ? "błąd odczytu" : "brak wymaganego wiersza");
        }
        std::istringstream fields(text);
        fields.imbue(std::locale::classic());
        return fields;
    }

    void require_end() {
        // Dopuszczamy końcowe białe znaki, ale wykrywamy dodatkowe rekordy.
        std::string text;
        while (std::getline(stream_, text)) {
            ++line_;
            if (text.find_first_not_of(" \t\r\v\f") != std::string::npos) {
                fail("nadmiarowe dane");
            }
        }
        if (stream_.bad()) {
            fail("błąd odczytu");
        }
    }

    [[noreturn]] void fail(const char* message) const {
        const auto location = line_ == 0 ? path_ : path_ + ":" + std::to_string(line_);
        throw std::runtime_error(location + ": " + message);
    }

private:
    std::string path_;
    std::ifstream stream_;
    std::size_t line_ = 0;
};

template <typename Number>
bool parse_number(const std::string& token, Number& value) {
    // Odczyt jest niezależny od ustawień regionalnych i obejmuje całe pole.
    const char* end = token.data() + token.size();
    const auto result = std::from_chars(token.data(), end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

bool read_index(std::istream& row, std::size_t& value) {
    std::string token;
    return (row >> token) && parse_number(token, value);
}

bool read_point(std::istream& row, Point& point) {
    std::string x;
    std::string y;
    return (row >> x >> y) && parse_number(x, point.x) && parse_number(y, point.y)
        && std::isfinite(point.x) && std::isfinite(point.y);
}

bool read_weight(std::istream& row, double& weight) {
    std::string token;
    return (row >> token) && token.find_first_not_of("0123456789") == std::string::npos
        && parse_number(token, weight) && std::isfinite(weight) && weight > 0;
}

bool row_finished(std::istream& row) {
    row >> std::ws;
    return row.eof();
}

}  // namespace

Instance read_instance(const std::filesystem::path& path) {
    TextInput input(path);
    auto header = input.row();
    std::size_t n;
    std::size_t m;
    if (!read_index(header, n) || !read_index(header, m) || !row_finished(header)
        || n == 0 || m == 0) {
        input.fail("oczekiwano dwóch dodatnich liczb całkowitych n i m");
    }

    // Rozszerzamy wektor dopiero po odczycie rekordu. Rozmiar z nagłówka może być błędny.
    Instance instance;
    for (std::size_t i = 0; i < n; ++i) {
        auto row = input.row();
        DemandPoint demand;
        if (!read_point(row, demand.position) || !read_weight(row, demand.weight)
            || !row_finished(row)) {
            input.fail("oczekiwano x y w (skończone współrzędne i dodatnia waga całkowita)");
        }
        instance.demand.push_back(demand);
    }
    for (std::size_t i = 0; i < m; ++i) {
        auto row = input.row();
        Point candidate;
        if (!read_point(row, candidate) || !row_finished(row)) {
            input.fail("oczekiwano dwóch skończonych współrzędnych x y");
        }
        instance.candidates.push_back(candidate);
    }
    input.require_end();
    return instance;
}

StationSet read_start(const std::filesystem::path& path, const Instance& instance) {
    TextInput input(path);
    auto header = input.row();
    const std::size_t m = instance.candidates.size();
    std::size_t p;
    if (!read_index(header, p) || !row_finished(header) || p == 0 || p > m) {
        input.fail("p musi być liczbą całkowitą z zakresu [1, m]");
    }

    auto row = input.row();
    StationSet selected;
    for (std::size_t i = 0; i < p; ++i) {
        std::size_t index;
        if (!read_index(row, index) || index >= m) {
            input.fail("oczekiwano p indeksów kandydatów z zakresu [0, m)");
        }
        // Sprawdzenie kolejności wykrywa też powtórzone indeksy.
        if (!selected.empty() && index <= selected.back()) {
            input.fail("indeksy kandydatów muszą być różne i posortowane rosnąco");
        }
        selected.push_back(index);
    }
    if (!row_finished(row)) {
        input.fail("oczekiwano dokładnie p indeksów kandydatów");
    }
    input.require_end();
    return selected;
}

}  // namespace pmedian
