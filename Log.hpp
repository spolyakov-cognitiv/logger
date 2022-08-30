#pragma once

#include <string>

#include <iostream>
#include <mutex>
#include <thread>

#define LOG_ENUM(EnumClass, ...)                                                                   \
	inline std::string stringify(EnumClass enum_class, const char* name, EnumClass value)          \
	{                                                                                              \
		return (enum_class != value) ? "UNKNOWN" : "\"" + std::string(name) + "\"";                \
	}                                                                                              \
																								   \
	template<class... Args>                                                                        \
	inline std::string stringify(                                                                  \
		EnumClass enum_class, const char* name, EnumClass value, Args... args)                     \
	{                                                                                              \
		return (enum_class != value) ? stringify(enum_class, args...) :                            \
									   "\"" + std::string(name) + "\"";                            \
	}                                                                                              \
																								   \
	inline std::ostream& operator<<(std::ostream& out, EnumClass enum_class)                       \
	{                                                                                              \
		out << stringify(enum_class, __VA_ARGS__);                                                 \
		return out;                                                                                \
	}

#define LOG_E(x) #x, x

namespace cognitiv
{
enum class LogLevel : uint16_t {
	FATAL = 0U,
	ERROR,
	WARNING,
	INFO,
	DEBUG,
	TRACE,

	COUNT //< Maximum value, used for range check
};

LOG_ENUM(
	LogLevel,
	LOG_E(LogLevel::FATAL),
	LOG_E(LogLevel::ERROR),
	LOG_E(LogLevel::WARNING),
	LOG_E(LogLevel::INFO),
	LOG_E(LogLevel::DEBUG),
	LOG_E(LogLevel::TRACE));

class Log
{
public:
	void set_max_log_level(LogLevel level) { m_max_level = level; }

	template<class... Args>
	void log_msg(LogLevel level, const char* func, const Args&... args)
	{
		if (level <= m_max_level) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_ostream << "\""
					  << "Thread:" << std::this_thread::get_id() << "\""
					  << " ";
			m_ostream << level << " ";
			m_ostream << "\"" << func << "\""
					  << " ";
			append(m_ostream, args...);
		}
	}

private:
	template<class T, class... Args>
	inline void append(std::ostream& ostream, const T& value, const Args&... args)
	{
		ostream << value;
		append(ostream, args...);
	}

	template<class T>
	inline void append(std::ostream& ostream, const T& value)
	{
		ostream << value << std::endl;
	}

	std::mutex m_mutex{};
	std::ostream& m_ostream{std::cout};
	LogLevel m_max_level{LogLevel::DEBUG};
};

Log& logger()
{
	static Log log;
	return log;
}

} // namespace cognitiv

#define LOG_MSG(...) cognitiv::logger().log_msg(__VA_ARGS__)

#define LOG_FATAL_MSG(...)                                                                         \
	LOG_MSG(cognitiv::LogLevel::FATAL, static_cast<const char*>(__PRETTY_FUNCTION__), __VA_ARGS__)
#define LOG_ERROR_MSG(...)                                                                         \
	LOG_MSG(cognitiv::LogLevel::ERROR, static_cast<const char*>(__PRETTY_FUNCTION__), __VA_ARGS__)
#define LOG_WARNING_MSG(...)                                                                       \
	LOG_MSG(cognitiv::LogLevel::WARNING, static_cast<const char*>(__PRETTY_FUNCTION__), __VA_ARGS__)
#define LOG_INFO_MSG(...)                                                                          \
	LOG_MSG(cognitiv::LogLevel::INFO, static_cast<const char*>(__PRETTY_FUNCTION__), __VA_ARGS__)
#define LOG_DEBUG_MSG(...)                                                                         \
	LOG_MSG(cognitiv::LogLevel::DEBUG, static_cast<const char*>(__PRETTY_FUNCTION__), __VA_ARGS__)
#define LOG_TRACE_MSG(...)                                                                         \
	LOG_MSG(cognitiv::LogLevel::TRACE, static_cast<const char*>(__PRETTY_FUNCTION__), __VA_ARGS__)

#define REQUIRED_RETVAL(condition, msg, return_value)                                              \
	if (!(condition)) {                                                                            \
		LOG_ERROR_MSG(msg);                                                                        \
		return (return_value);                                                                     \
	}

#define REQUIRED_VOID(condition, msg) REQUIRED_RETVAL((condition), (msg), void())

// https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments/11763277#11763277
#define GET_MACRO(_1, _2, _3, MACRO, ...) MACRO
#define REQUIRED(...) GET_MACRO(__VA_ARGS__, REQUIRED_RETVAL, REQUIRED_VOID)(__VA_ARGS__)

#define LOG_IT(x) #x, x

template<class T>
inline void merge_key_value_pairs(std::ostream& out, const char* key, const T& value)
{
	out << R"(")" << key << R"(":)";
	out << value;
	out << "";
}

template<class T, class... Args>
inline void merge_key_value_pairs(
	std::ostream& out, const char* key, const T& value, const Args&... args)
{
	out << R"(")" << key << R"(":)";
	out << value;
	out << ",";
	merge_key_value_pairs(out, args...);
}

template<class... Args>
inline void serialization(std::ostream& out, const Args&... args)
{
	out << "{";
	merge_key_value_pairs(out, args...);
	out << "}";
}

#define LOG_CLASS(Class, ...)                                                                      \
	inline void serialize_me(std::ostream& out) const { serialization(out, __VA_ARGS__); }         \
	friend inline std::ostream& operator<<(std::ostream& stream, const Class& obj)                 \
	{                                                                                              \
		obj.serialize_me(stream);                                                                  \
		return stream;                                                                             \
	}
