#pragma once

#include <vector>
#include <memory>

class ByteArray{
public:
	std::vector<uint8_t> data;

	ByteArray(size_t reservedLen){
		data.reserve(reservedLen);
	}

	void extend(const std::initializer_list<uint8_t>& moreData){
		const auto oldSize = data.size();
		data.resize(data.size() + moreData.size());
		memcpy(&data[oldSize], std::data(moreData), moreData.size());
	}
	void extend(const std::string& value){
		const auto oldSize = data.size();
		data.resize(data.size() + value.size());
		memcpy(&data[oldSize], value.data(), value.size());
	}
	void extend(const std::string_view& value){
		const auto oldSize = data.size();
		data.resize(data.size() + value.size());
		memcpy(&data[oldSize], value.data(), value.size());
	}
	template<typename T>
	void extend(const T& value){
		static_assert(std::is_scalar<T>());
		const auto oldSize = data.size();
		data.resize(data.size() + sizeof(value));
		memcpy(&data[oldSize], reinterpret_cast<const uint8_t*>(&value), sizeof(value));
	}

};
