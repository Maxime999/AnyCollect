//
// AnyCollectValues.cc
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

#include <iostream>
#include <iomanip>
#include <sstream>

#include <AnyCollect/Controller.h>


template<typename Clock, typename Duration>
std::ostream &operator<<(std::ostream &stream,
	const std::chrono::time_point<Clock, Duration> &time_point) {
	const time_t time = Clock::to_time_t(time_point);
	struct tm tm;
	localtime_r(&time, &tm);
	return stream << std::put_time(&tm, "%c");
}


void printMetric(const AnyCollect::Metric& m) {
	std::cout << "{  Name: [ ";
	for (const auto& part : m.name())
		std::cout << "'" << part << "' ";
	std::cout << "]    value: '" << m.value() << "'    t: '" << m.timestamp() << "'    unit: '" << m.unit() << "'    tags: [ ";
	for (const auto& [k, v] : m.tags())
		std::cout << "'" << k << "'='" << v << "' ";
	std::cout << "]" << std::endl;
}


struct AnyCollectValues : public AnyCollect::ControllerDelegate {
	size_t iterationCount;

#if PRINT_METRICS
	void contollerCollectedMetrics(const AnyCollect::Controller& , const std::vector<AnyCollect::Metric*>& metrics) override {
		for (const auto& m : metrics)
			printMetric(*m);
		std::cout << std::endl << "----------------" << std::endl << std::endl;
#else
	void contollerCollectedMetrics(const AnyCollect::Controller& , const std::vector<AnyCollect::Metric*>& ) override {
#endif
	}

	bool contollerShouldStopCollectingMetrics(const AnyCollect::Controller& ) override {
		iterationCount--;
		return iterationCount == 0;
	}
};


int main(int argc, char* argv[]) {
	auto samplingInterval = AnyCollect::Controller::defaultSamplingInterval;
	std::string configPath = "";
	AnyCollectValues d;

	if (argc > 1)
		samplingInterval = std::chrono::seconds(std::atol(argv[1]));
	if (argc > 2)
		configPath = argv[2];
	if (argc > 3)
		d.iterationCount = std::atol(argv[3]);
	else
		d.iterationCount = 0;

	AnyCollect::Controller controller(d);
	controller.setSamplingInterval(samplingInterval);
	controller.loadConfigFromFile(configPath);

	controller.collectMetrics();

	return 0;
}
