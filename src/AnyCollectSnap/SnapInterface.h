//
// SnapInterface.h
//
// Created on October 25th 2018
//
// Copyright 2018 CFM (www.cfm.fr)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include <AnyCollect/Controller.h>
#include <snap/plugin.h>

using namespace std::literals;


namespace AnyCollect {
	class SnapInterface final : public Plugin::StreamCollectorInterface, public AnyCollect::ControllerDelegate {
		public:
			static constexpr int appVersion = 1;															//!< Snap plugin version
			static constexpr std::string_view appName = "anycollect"sv;										//!< Snap plugin name

		protected:
			static constexpr std::array appPrefix = {"cfm"sv, "anycollect"sv};								//!< Snap metric name prefix
			static constexpr std::string_view configKeyConfigFile = "ConfigFile"sv;							//!< Snap plugin configuration key
			static constexpr std::string_view configKeySamplingInterval = "SamplingInterval"sv;				//!< Snap plugin configuration key
			static constexpr std::string_view configKeySendAllMetrics = "SendAllMetrics"sv;					//!< Snap plugin configuration key
			static constexpr std::string_view configKeyMaxCollectDuration = "MaxCollectDuration"sv;			//!< Snap plugin configuration key
			static constexpr std::string_view configKeyMaxMetricsBuffer = "MaxMetricsBuffer"sv;				//!< Snap plugin configuration key
			static constexpr std::array configKeysInt = {configKeySamplingInterval, configKeyMaxCollectDuration, configKeyMaxMetricsBuffer};		//!< Array of integer-valued configuration keys
			static constexpr std::array configKeysBool = {configKeySendAllMetrics};							//!< Array of boolean-valued configuration keys
			static constexpr bool defaultSendAllMetrics = false;											//!< Snap plugin configuration default value
			static constexpr std::chrono::seconds defaultMaxCollectDuration = 0s;							//!< Snap plugin configuration default value
			static constexpr size_t defaultMaxMetricsBuffer = 0;											//!< Snap plugin configuration default value
			static constexpr std::array<int, configKeysInt.size()> configValuesInt = {AnyCollect::Controller::defaultSamplingInterval.count(), SnapInterface::defaultMaxCollectDuration.count(), SnapInterface::defaultMaxMetricsBuffer};		//!< Array of integer-valued configuration default values
			static constexpr std::array<int, configKeysBool.size()> configValuesBool = {SnapInterface::defaultSendAllMetrics};		//!< Array of boolean-valued configuration default values
			static std::hash<std::string> hasher;															//!< Object used to compute keys (string hashes)

			AnyCollect::Controller controller_;																//!< Controller used to collect statistics
			std::map<size_t, Plugin::Metric> metrics_;														//!< Map associating keys to their metric
			std::set<size_t> requestedMetrics_;																//!< Array of keys of requested metrics
			std::set<size_t> unwantedMetrics_;																//!< Array of keys of non requested metrics
			bool sendAllMetrics_;																			//!< Whether to send all metrics, regardless of which are requested
			std::vector<Plugin::Metric*> metricsToSend_;													//!< Array of pointers to Snap metrics to be sent


			/**
			 * @brief Inserts plugin name prefix into Snap namespace
			 *
			 * @param ns Snap namespace to modify
			 */
			void insertAppPrefixToNamespace(std::vector<std::string>& ns);

			/**
			 * @brief Configures the plugin and the controller  accordingly, unspecified values are reset to default
			 *
			 * @param cfg configuration options to use
			 */
			void setConfig(const Plugin::Config& cfg);

			/**
			 * @brief Formats a metric name to be compatible with Snap requirements
			 *
			 * @param name name to format
			 */
			void formatName(std::vector<std::string>& name);

			/**
			 * @brief Computes a key based solely on a metric's name
			 *
			 * @param m the metric to compute the name key
			 * @return the computed name key
			 */
			size_t computeNameKey(const Metric& m);

			/**
			 * @brief Computes a key based solely on a metric's name
			 *
			 * @param m the Snap metric to compute the name key
			 * @return the computed name key
			 */
			size_t computeNameKey(const Plugin::Metric& m);

			/**
			 * @brief Converts a metric object into a Snap metric object
			 *
			 * @param metric the metric to convert
			 * @return the converted metric
			 */
			Plugin::Metric convertToSnapMetric(const Metric& metric);

		public:
			/**
			 * @brief Construct a new Snap Interface object
			 */
			SnapInterface();


			/**
			 * @brief Returns the plugin configuration keys and default values to Snap
			 *
			 * @return The plugin default configuration
			 */
	        const Plugin::ConfigPolicy get_config_policy() override final;

			/**
			 * @brief Returns all metrics that can be fetched by the plugin to Snap
			 *
			 * @param cfg incomming Snap configuration values
			 * @return metrics that can be fetched
			 */
     	 	std::vector<Plugin::Metric> get_metric_types(Plugin::Config cfg) override final;

			/**
			 * @brief Gets the user's requested metrics from Snap
			 *
			 * @param metsIn the user's requested metrics array
			 */
			void get_metrics_in(std::vector<Plugin::Metric> &metsIn) override final;

			/**
			 * @brief Stream metrics to Snap
			 */
			void stream_metrics() override final;


			void contollerCollectedMetrics(const Controller& controller, const std::vector<const Metric*>& metrics) override final;
			bool contollerShouldStopCollectingMetrics(const Controller& controller) override final;

	};
}


/**
 * @brief AnyCollect's main function
 */
int main(int argc, char* argv[]);
