// Añade estos includes donde corresponda en tu servidor
#include "temperature_history.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
 
// Helpers
 
// Convierte "YYYY-MM-DD HH:MM:SS" a time_t
time_t parseTimestamp(const std::string& s) {
    struct tm t = {};
    // sscanf es más portable que strptime en MSVC, pero en Linux strptime está ok
    if (sscanf(s.c_str(), "%4d-%2d-%2d %2d:%2d:%2d",
               &t.tm_year, &t.tm_mon, &t.tm_mday,
               &t.tm_hour, &t.tm_min, &t.tm_sec) == 6) {
        t.tm_year -= 1900;
        t.tm_mon  -= 1;
        t.tm_isdst = -1;
        return mktime(&t);
    }
    return -1;
}
 
// URL-decode mínimo (solo %20 y +, suficiente para fechas)
std::string urlDecode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out += ' ';
        } else if (s[i] == '%' && i + 2 < s.size()) {
            int val = 0;
            sscanf(s.substr(i+1, 2).c_str(), "%2x", &val);
            out += static_cast<char>(val);
            i += 2;
        } else {
            out += s[i];
        }
    }
    return out;
}
 
// Extrae el valor de un parámetro de query string: getParam("from=X&to=Y", "from") → "X"
std::string getParam(const std::string& query, const std::string& key) {
    std::string search = key + "=";
    size_t pos = query.find(search);
    if (pos == std::string::npos) return "";
    pos += search.size();
    size_t end = query.find('&', pos);
    std::string raw = (end == std::string::npos) ? query.substr(pos) : query.substr(pos, end - pos);
    return urlDecode(raw);
}
 
// Escapa caracteres especiales JSON en un string
std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else                out += c;
    }
    return out;
}
 
// Handler
 
// Llama a esta función desde tu router donde gestionas las URLs.
// `queryString` es la parte de la URL tras '?'  (p.ej. "from=2025-07-10 08:00:00&to=2025-07-10 20:00:00")
// `dataDir`    es el valor de DATA_DIR (p.ej. "/data")
std::string buildTemperatureHistoryJson(const std::string& queryString,const std::string& dataDir)
{
    // Parsear parámetros
    std::string fromStr = getParam(queryString, "from");
    std::string toStr   = getParam(queryString, "to");
 
    time_t fromTime = parseTimestamp(fromStr);
    time_t toTime   = toStr.empty() ? time(nullptr) : parseTimestamp(toStr);
 
    if (fromTime == -1) {
        return R"({"error":"invalid 'from' parameter"})";
    }
    if (toTime == -1) {
        return R"({"error":"invalid 'to' parameter"})";
    }
 
    // Leer fichero
    std::string tempFile = dataDir + "/cpu_temp.txt";
    std::ifstream file(tempFile);
    if (!file.is_open()) {
        return R"({"error":"cpu_temp.txt not found","entries":[]})";
    }
 
    // Filtrar líneas
    // Formato de cada línea: "2025-07-10 14:32:05 BOARD_TEMP=42.3C"
    std::vector<std::pair<std::string, float>> entries;
 
    std::string line;
    while (std::getline(file, line)) {
        if (line.size() < 20) continue;
 
        // Timestamp: primeros 19 caracteres
        std::string ts = line.substr(0, 19);
        time_t lineTime = parseTimestamp(ts);
        if (lineTime == -1)            continue;
        if (lineTime < fromTime)       continue;
        if (lineTime > toTime)         break;   // el fichero está ordenado cronológicamente
 
        // Temperatura: busca "BOARD_TEMP=" y extrae el float
        size_t pos = line.find("BOARD_TEMP=");
        if (pos == std::string::npos) continue;
        pos += 11; // longitud de "BOARD_TEMP="
        float temp = 0.0f;
        if (sscanf(line.c_str() + pos, "%f", &temp) != 1) continue;
 
        entries.push_back({ts, temp});
    }
 
    // Construir JSON
    std::ostringstream json;
    json << "{\"entries\":[";
    for (size_t i = 0; i < entries.size(); ++i) {
        if (i > 0) json << ",";
        json << "{\"timestamp\":\"" << jsonEscape(entries[i].first) << "\","
             << "\"temp\":"         << entries[i].second            << "}";
    }
    json << "]}";
 
    return json.str();
}