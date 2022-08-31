#pragma once

#include <fmt/core.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

//#include <experimental/source_location> EXIST gcc-10

namespace cognitiv
{
enum class LogLevel : uint8_t {
	TRACE = 0U,
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	CRITICAL,
	OFF,

	COUNT //< Maximum value, used for range check
};

namespace sink
{
enum class Type : uint8_t {
	CONSOLE,
	FILE,
	DLT, // Hope, we will use it in the future for remote logs
	SYSLOG // For metrics and remote logs possibility
};

struct Config {
	Type type{Type::CONSOLE};
	LogLevel log_level{LogLevel::TRACE};

	// TODO rethink
	std::optional<uint32_t> log_file_max_count{std::nullopt};
	std::optional<size_t> log_file_max_size_mb{std::nullopt};
	std::optional<std::string> log_directory{std::nullopt};
};
} // namespace sink

namespace logger
{
struct Config {
	std::string name;
	LogLevel log_level{LogLevel::TRACE};
	std::vector<sink::Config> sinks;
};

class ILogger
{
public:
	virtual ~ILogger() = default;

	// Add source location and extend formatter
	virtual void log_msg(LogLevel level, const std::string_view& msg) = 0;
	virtual void flush() = 0;

	// Deprecated section. Backward compatibility
	virtual void critical(const std::string_view& msg) { log_msg(LogLevel::CRITICAL, msg); };

	virtual void error(const std::string_view& msg) { log_msg(LogLevel::ERROR, msg); };

	virtual void warn(const std::string_view& msg) { log_msg(LogLevel::WARNING, msg); };

	virtual void warning(const std::string_view& msg) { log_msg(LogLevel::WARNING, msg); };

	virtual void info(const std::string_view& msg) { log_msg(LogLevel::INFO, msg); };

	virtual void debug(const std::string_view& msg) { log_msg(LogLevel::DEBUG, msg); };

	template<typename... Args>
	void critical(Args&&... args)
	{
		critical(fmt::format(std::forward<Args>(args)...));
	}

	template<typename... Args>
	void error(Args&&... args)
	{
		error(fmt::format(std::forward<Args>(args)...));
	}

	template<typename... Args>
	void warn(Args&&... args)
	{
		warn(fmt::format(std::forward<Args>(args)...));
	}

	template<typename... Args>
	void info(Args&&... args)
	{
		info(fmt::format(std::forward<Args>(args)...));
	}

	template<typename... Args>
	void debug(Args&&... args)
	{
		debug(fmt::format(std::forward<Args>(args)...));
	}
};

void init();
std::shared_ptr<ILogger> register_new(const Config& config);

// DEPRECATED. Backward compatibility
std::shared_ptr<ILogger> register_new(const std::string& name);
std::shared_ptr<ILogger> get(const std::string& name);

} // namespace logger

} // namespace cognitiv
