#include "Logger.hpp"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <filesystem>

namespace
{
spdlog::sink_ptr GLOBAL_CONSOLE_SINK{nullptr};

constexpr auto* DEFAULT_LOG_PATTERN = "[%Y-%m-%d %H:%M:%S.%e %z] [%n] [tid %t] [%^%l%$] %v";
constexpr auto* DEFAULT_LOG_FILE_PATH{"/var/log/bidrouter/"};
constexpr size_t DEFAULT_LOG_FILE_MAX_SIZE_MB{1024U * 1024 * 1024};
constexpr uint32_t DEFAULT_LOG_FILE_MAX_COUNT{3U};

std::vector<spdlog::sink_ptr> create_sinks(const cognitiv::LoggerConfig& config)
{
	std::vector<spdlog::sink_ptr> sinks;

	for (auto& sink : config.sinks) {
		switch (sink.type) {
			case cognitiv::SinkType::CONSOLE: {
				sinks.push_back(GLOBAL_CONSOLE_SINK);
			} break;
			case cognitiv::SinkType::FILE: {
				auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
					std::filesystem::path(
						sink.log_directory.value_or(DEFAULT_LOG_FILE_PATH) + '/' + config.name),
					sink.log_file_max_size_mb.value_or(DEFAULT_LOG_FILE_MAX_SIZE_MB) * 1024 * 1024,
					sink.log_file_max_count.value_or(DEFAULT_LOG_FILE_MAX_COUNT));
				sinks.push_back(rotating_file_sink);
			} break;
			case cognitiv::SinkType::DLT:
			case cognitiv::SinkType::SYSLOG:
				// Doesn't supported yet
				break;
		}
	}

	return sinks;
}

spdlog::level::level_enum to_spdlog_log_level(cognitiv::LogLevel level)
{
	switch (level) {
		case cognitiv::LogLevel::TRACE:
			return spdlog::level::trace;
		case cognitiv::LogLevel::DEBUG:
			return spdlog::level::debug;
		case cognitiv::LogLevel::INFO:
			return spdlog::level::info;
		case cognitiv::LogLevel::WARNING:
			return spdlog::level::warn;
		case cognitiv::LogLevel::ERROR:
			return spdlog::level::err;
		case cognitiv::LogLevel::CRITICAL:
			return spdlog::level::critical;
		case cognitiv::LogLevel::OFF:
			return spdlog::level::off;
		case cognitiv::LogLevel::COUNT:
			return spdlog::level::n_levels;
	}
}

} // namespace

namespace cognitiv
{

Logger::Logger(const LoggerConfig& config)
	: config_(std::move(config))
{
	if (!GLOBAL_CONSOLE_SINK) {
		spdlog::set_pattern(DEFAULT_LOG_PATTERN);
		GLOBAL_CONSOLE_SINK = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
	}
	spdlog::flush_every(std::chrono::seconds(10));

	std::vector<spdlog::sink_ptr> sinks = create_sinks(config_);

	auto service_logger = std::make_shared<spdlog::logger>(config_.name, begin(sinks), end(sinks));
	service_logger->set_level(to_spdlog_log_level(config_.log_level));
	service_logger->set_pattern(DEFAULT_LOG_PATTERN);
	spdlog::register_logger(service_logger);
}

std::shared_ptr<spdlog::logger> Logger::register_logger(const std::string& name)
{
	std::vector<spdlog::sink_ptr> sinks = create_sinks(config_);

	auto logger = std::make_shared<spdlog::logger>(name, begin(sinks), end(sinks));
	logger->set_pattern(DEFAULT_LOG_PATTERN);
	spdlog::register_logger(logger);

	return logger;
}

std::shared_ptr<spdlog::logger> Logger::get_logger(const std::string& name) const
{
	return spdlog::get(name);
}

} // namespace cognitiv
