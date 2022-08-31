#include "logger/Logger.hpp"

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
constexpr auto DEFAULT_FLUSH_TIMEOUT{std::chrono::seconds(10)};

std::vector<spdlog::sink_ptr> create_sinks(const cognitiv::logger::Config& config)
{
	std::vector<spdlog::sink_ptr> sinks;

	for (auto& sink : config.sinks) {
		switch (sink.type) {
			case cognitiv::sink::Type::CONSOLE: {
				sinks.push_back(GLOBAL_CONSOLE_SINK);
			} break;
			case cognitiv::sink::Type::FILE: {
				auto rotating_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
					std::filesystem::path(
						sink.log_directory.value_or(DEFAULT_LOG_FILE_PATH) + '/' + config.name),
					sink.log_file_max_size_mb.value_or(DEFAULT_LOG_FILE_MAX_SIZE_MB) * 1024 * 1024,
					sink.log_file_max_count.value_or(DEFAULT_LOG_FILE_MAX_COUNT));
				sinks.push_back(rotating_file_sink);
			} break;
			case cognitiv::sink::Type::DLT:
			case cognitiv::sink::Type::SYSLOG:
				// Doesn't supported yet
				break;
		}
	}

	return sinks;
}

spdlog::level::level_enum convert(cognitiv::LogLevel level)
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

	return spdlog::level::trace;
}

} // namespace

namespace cognitiv
{

class SpdlogLogger : public logger::ILogger
{
public:
	explicit SpdlogLogger(std::shared_ptr<spdlog::logger> logger)
		: logger_(std::move(logger))
	{
	}

	void log_msg(LogLevel level, const std::string_view& msg) override
	{
		logger_->log(convert(level), msg);
	};

private:
	std::shared_ptr<spdlog::logger> logger_{nullptr};
};

void logger::init()
{
	if (!GLOBAL_CONSOLE_SINK) {
		spdlog::set_pattern(DEFAULT_LOG_PATTERN);
		GLOBAL_CONSOLE_SINK = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
	}
	spdlog::flush_every(DEFAULT_FLUSH_TIMEOUT);
}

std::shared_ptr<logger::ILogger> logger::register_new(const logger::Config& config)
{
	std::vector<spdlog::sink_ptr> sinks = create_sinks(config);

	auto logger = std::make_shared<spdlog::logger>(config.name, begin(sinks), end(sinks));
	logger->set_level(convert(config.log_level));
	logger->set_pattern(DEFAULT_LOG_PATTERN);
	spdlog::register_logger(logger);

	return std::make_shared<SpdlogLogger>(logger);
}

std::shared_ptr<logger::ILogger> logger::get(const std::string& name)
{
	return std::make_shared<SpdlogLogger>(spdlog::get(name));
}

} // namespace cognitiv
