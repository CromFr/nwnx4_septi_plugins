#pragma once

#include <vector>
#include <memory>
#include <string_view>
#include <optional>
#include <type_traits>

// Write data to a byte array
class ByteWriter{
public:
	std::vector<uint8_t> data;

	ByteWriter(size_t reservedLen){
		data.reserve(reservedLen);
	}

	void write(const std::initializer_list<uint8_t>& moreData){
		const auto oldSize = data.size();
		data.resize(data.size() + moreData.size());
		memcpy(&data[oldSize], std::data(moreData), moreData.size());
	}
	void write(const std::string& value){
		const auto oldSize = data.size();
		data.resize(data.size() + value.size());
		memcpy(&data[oldSize], value.data(), value.size());
	}
	void write(const std::string_view& value){
		const auto oldSize = data.size();
		data.resize(data.size() + value.size());
		memcpy(&data[oldSize], value.data(), value.size());
	}
	template<typename T>
	void write(const T& value){
		static_assert(std::is_scalar<T>());
		const auto oldSize = data.size();
		data.resize(data.size() + sizeof(value));
		memcpy(&data[oldSize], reinterpret_cast<const uint8_t*>(&value), sizeof(value));
	}

};

// Safely extract data from a byte array
class ByteReader{
public:
	std::span<const uint8_t> data;

	std::optional<std::string_view> read_string(size_t size){
		if(data.size() < size)
			return std::nullopt;

		const auto ptr = data.data();
		data = data.subspan(size);
		return std::string_view{(const char*)ptr, size};
	}

	template<typename T>
	const T* read(){
		static_assert(std::is_arithmetic_v<T> == true);
		if(data.size() < sizeof(T))
			return nullptr;

		const auto ptr = data.data();
		data = data.subspan(sizeof(T));
		return reinterpret_cast<const T*>(ptr);
	}

};