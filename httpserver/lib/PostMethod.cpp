// Expone las rutas privadas, cada una de ellas con su clase + función exec

#include "PostMethod.h"
#include "System.h"
#include "Json.hpp"
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

// funcion para el hasheo de la contraseña por medio de sha256
static std::string sha256(const std::string& input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

string Login::exec(string params)
{
    string user     = getPostParam(params, "user");
    string password = getPostParam(params, "pass");
    string passHash = sha256(password);

    std::ifstream file(System::dataFilesFolder + "/credentials.txt");
    if (!file.is_open())
        return "{\"ok\":false,\"error\":\"no credentials file\"}";

    std::string line;
    while (std::getline(file, line))
    {
        size_t sep = line.find(':');
        if (sep == std::string::npos) continue;
        string fileUser = line.substr(0, sep);
        string fileHash = line.substr(sep + 1);
        if (fileUser == user && fileHash == passHash)
            return "{\"ok\":true}";
    }
    return "{\"ok\":false}";
}

string ListWavFiles::exec(string params)
{
	return string();
}

//--- helper function convert timepoint to usable timestamp
template <typename TP>
time_t to_time_t(TP tp) {
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}

string ListFiles::exec(string params)
{
	
	std::string path = getPostParam(params, "directory");
	replaceSubstrs(path, "/../", "/");//avoid relative paths
	replaceSubstrs(path, "//", "/");//avoid relative paths

	int  parentIdx = (int)path.find_last_of("/", path.length()-2);
	string parentPath = path;
	if (parentIdx != string::npos)
		parentPath = path.substr(0, parentIdx+1);

	std::string realPath = System::dataFilesFolder+"/"+getPostParam(params, "directory");
	
	std::string directories = "{\"files\": [";
	std::map<time_t, std::vector<std::filesystem::directory_entry>, std::greater<time_t>> sort_by_time;
	int countFiles = 0;
	for (const auto& entry : std::filesystem::directory_iterator(realPath))
	{
		auto time = to_time_t(entry.last_write_time());
		sort_by_time[time].push_back(entry);
	}
	//add previous folder
	directories += "\n{\n"
		"\"parent\":\"" + parentPath + "\",\n"
		"\"name\":\"\",\n"
		"\"date\":\"\",\n"
		"\"type\":\"PARENTDIR\"\n"
		"}\n,";

	for (auto const& [time, entryList] : sort_by_time)
	{
		for (auto entry : entryList) {
			std::string href;
			std::string name = (char*)entry.path().filename().u8string().c_str();
			string folderDate = std::string(asctime(std::localtime(&time)));
			folderDate.pop_back();//scape last \n
			if (entry.is_directory())
			{
				href = name + "/";
				directories += "\n{\n"
					"\"parent\":\""+ parentPath +"\",\n"
					"\"name\":\"" +href + "\",\n"
					"\"date\":\"" + folderDate + "\",\n"
					"\"type\":\"DIR\"\n"
					"}\n,";

			}
			if (entry.is_regular_file()) {
				
				href = name ;
				directories += "\n{\n"
					"\"parent\":\"" + parentPath + "\",\n"
					"\"name\":\"" + href + "\",\n"
					"\"date\":\"" + folderDate + "\",\n"
					"\"type\":\"FILE\"\n"
					"}\n,";
			}
			countFiles++;
		}
	}
	directories.pop_back();

	directories += "]}";

	return directories;
}

string RecordData::exec(string params)
{
	json::JSON obj = json::JSON::Load(params);
	std::ofstream outfile;

	outfile.open(obj["fileName"].ToString(), std::ios_base::app);

	for (auto& val : *(obj.Internal.Map))
		outfile << val.second<<"\t";
		//std::cout << val.first << " " <<val.second;
	outfile << "\n";
	outfile.close();
	return "OK";
}

// implementación de la logica para get/post de los parametros de configuración
static std::string escapeJson(const std::string& s)
{
    std::string out;
    for (char c : s) {
        switch (c) {
        case '\"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b";  break;
        case '\f': out += "\\f";  break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:   out += c;      break;
        }
    }
    return out;
}

string GetConfig::exec(string params)
{
    std::map<std::string, std::string> defaults = {
        {"STATION",       "TECHOUTAD_"},
        {"BITRATE",       "16"},
        {"SAMPLE_RATE",   "32000"},
        {"GAIN",          "5.0"},
        {"DURATION",      "60"},
        {"IDRECORDER",    "1"},
        {"SLEEPDURATION", "10"},
        {"GPIO_PIN",      "117"}
    };

    // Sobreescribe con lo que haya en config.txt
    std::ifstream file(System::dataFilesFolder + "/config.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t sep = line.find('=');
            if (sep == std::string::npos) continue;
            std::string key = line.substr(0, sep);
            std::string val = line.substr(sep + 1);
            if (defaults.count(key)) defaults[key] = val;
        }
    }

    std::string json = "{";
    bool first = true;
    for (auto& kv : defaults) {
        if (!first) json += ",";
        json += "\"" + kv.first + "\":\"" + escapeJson(kv.second) + "\"";
        first = false;
    }
    json += "}";
    return json;
}

string SaveConfig::exec(string params)
{
    const std::vector<std::string> keys = {
        "STATION", "BITRATE", "SAMPLE_RATE", "GAIN",
        "DURATION", "IDRECORDER", "SLEEPDURATION", "GPIO_PIN"
    };

    std::ofstream file(System::dataFilesFolder + "/config.txt", std::ios::trunc);
    if (!file.is_open())
        return "{\"ok\":false,\"error\":\"cannot write config\"}";

    for (auto& key : keys) {
        std::string val = getPostParam(params, key);
        if (!val.empty())
            file << key << "=" << val << "\n";
    }
    file.close();

    // Reinicia el contenedor recorder en background
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        system("docker restart bird-recorder");
    }).detach();

    return "{\"ok\":true}";
}