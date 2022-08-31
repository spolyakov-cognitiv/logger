#pragma once

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
};

void init();
std::shared_ptr<ILogger> register_new(const Config& config);
std::shared_ptr<ILogger> get(const std::string& name);

} // namespace logger

} // namespace cognitiv
