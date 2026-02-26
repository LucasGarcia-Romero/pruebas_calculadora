#include "PostMethod.h"
#include "System.h"
#include "Json.hpp"

string Login::exec(string params)
{
	return "";
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
