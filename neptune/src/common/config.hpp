/// This code loads a configuration from YAML into a struct to easily change the program's behavior.
#pragma < once>

#include <rfl.hpp>
#include <rfl/yaml.hpp>
#include <yaml-cpp/yaml.h>

#include <exception>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

/// This replicates the functionality of Rust's `Result::expect` function, where the value is
/// unwrapped from std::expected, and if the contained value is an error, the program is terminated
/// and the error forwarded to standard error.
template <typename T, typename E> T expect(std::expected<T, E> &&exp, std::string_view msg) {
    if (!exp.has_value()) {
        std::cerr << msg << ": " << exp.error() << std::endl;
        std::terminate();
    }
    return std::move(exp).value();
}

/// @brief Loads the passed `Config` in a typesafe way from the desired config file
/// @tparam Config The type of the configuration object being loaded
/// @param config_filepath The name of the config file
/// @return Either an instance of the `Config` type or an error message
template <typename Config>
std::expected<Config, std::string> load_config(std::filesystem::path config_filepath) {
    std::ifstream config_file(config_filepath);

    if (!config_file.is_open()) {
        return std::unexpected(std::string("Could not open file 'config.yaml'"));
    }

    std::stringstream buffer;
    buffer << config_file.rdbuf();
    std::string yaml_contents = buffer.str();

    auto result = rfl::yaml::read<Config>(yaml_contents);
    if (!result) {
        return std::unexpected(
            std::string("Failed to parse YAML config: \n" + result.error().what())
        );
    }
    return result.value();
}
