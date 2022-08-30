#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace spdlog
{
// Forward declaration
class logger;
} // namespace spdlog

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

enum class SinkType : uint8_t {
	CONSOLE,
	FILE,
	DLT, // Hope, we will use it in the future for remote logs
	SYSLOG // For metrics and remote logs possibility
};

struct SinkConfig {
	SinkType type{SinkType::CONSOLE};
	LogLevel log_level{LogLevel::TRACE};

	std::optional<uint32_t> log_file_max_count{std::nullopt};
	std::optional<size_t> log_file_max_size_mb{std::nullopt};
	std::optional<std::string> log_directory{std::nullopt};
};

struct LoggerConfig {
	std::string name;
	LogLevel log_level{LogLevel::TRACE};
	std::vector<SinkConfig> sinks;
};

class Logger
{
public:
	Logger(const LoggerConfig& config);

	// Should be incapsulated later
	std::shared_ptr<spdlog::logger> register_logger(const std::string& name);
	std::shared_ptr<spdlog::logger> get_logger(const std::string& name) const;

private:
	LoggerConfig config_;
};

} // namespace cognitiv
