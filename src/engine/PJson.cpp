#include "PJson.hpp"

namespace PJson{

void jsonReadString(const nlohmann::json& j, const std::string& name, std::string& target){
	if(j.contains(name)){
		target = j[name].get<std::string>();
	}
}

void jsonReadInt(const nlohmann::json& j, const std::string& name, int& target){
	if(j.contains(name)){
		target = j[name].get<int>();
	}
}

void jsonReadDouble(const nlohmann::json& j, const std::string& name, double& target ){
	if(j.contains(name)){
		target = j[name].get<double>();
	}
}

void jsonReadBool(const nlohmann::json& j, const std::string& name, bool& target){
	if(j.contains(name)){
		target = j[name].get<bool>();
	}
}

void jsonReadEnumU8(const nlohmann::json& j, const std::string& name, u8& target){
	if(j.contains(name)){
		int res = j[name].get<int>();
		target= (u8) res;
	}
}

}