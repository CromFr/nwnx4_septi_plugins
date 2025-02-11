#pragma once

#include <vector>

class ByteArray{
	std::vector<uint8_t> data;

	ByteArray(size_t reservedLen){
		data.reserve(reservedLen);
	}

	void extend(const std::initializer_list<uint8_t>& moreData){
		const auto oldSize = data.size();
		data.resize(data.size() + moreData.size());
		memcpy(&data[oldSize], moreData.data(), moreData.size());
	}
	void extend(const std::string& value){
		const auto oldSize = data.size();
		data.resize(data.size() + value.size());
		memcpy(&data[oldSize], value.data(), value.size());
	}
	template<typename T>
	void extend(const T& value){
		static_assert(std::is_scalar(value));
		const auto oldSize = data.size();
		data.resize(data.size() + sizeof(value));
		memcpy(&data[oldSize], reinterpret_cast<char*>(&value), sizeof(value));
	}

};
