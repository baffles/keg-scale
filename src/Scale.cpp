#include "Scale.hpp"

Scale::Scale(ScaleConfiguration initialConfiguration) :
	scaleInterface(), _configuration(initialConfiguration),
	weightSampleTotalCount(0), weightSampleCount(0), weightSampleIdx(0), weightSamples(NULL), currentWeightSum(0),
	currentCorrectionFactor(0), correctionSampleCount(0), currentCorrectionSum(0), previousCorrectionValue(0.0f),
	lastSampleTime(0) {
}

Scale::Scale() : Scale(ScaleConfiguration {}) {
}

Scale::~Scale() {
	if (weightSampleTotalCount > 0) {
		delete weightSamples;
		weightSampleTotalCount = 0;
	}
}

auto Scale::setup(const ScaleHardware hwInfo) -> void {
	scaleInterface.begin(hwInfo.dataPin, hwInfo.clockPin, hwInfo.gain);
	reconfigure();
}

auto Scale::loop() -> void {
	auto currentTime = millis();

	// if the scale is ready
	if (((currentTime - lastSampleTime) > _configuration.sampleIntervalMs) && scaleInterface.is_ready()) {
		// take a sample
		recordReading(scaleInterface.read());

		// remember the last sample time
		lastSampleTime = currentTime;
	}
}

auto Scale::configuration()       ->       ScaleConfiguration& { return _configuration; }
auto Scale::configuration() const -> const ScaleConfiguration& { return _configuration; }

auto Scale::reconfigure() -> void {
	if (_configuration.sampleCount != weightSampleTotalCount) {
		if (weightSampleTotalCount > 0) {
			delete weightSamples;
		}

		weightSamples = new long[_configuration.sampleCount];
		weightSampleTotalCount = _configuration.sampleCount;
		weightSampleIdx = weightSampleCount = 0;

		lastSampleTime = 0;
	}

	correctionSampleCount = 0;
	currentCorrectionSum = 0;
	previousCorrectionValue = 0.0f;
}

auto Scale::tare() -> void {
	_configuration.tare = currentRawReading();
}

auto Scale::calibrate(float knownWeight) -> void {
	auto currentTaredValue = currentRawReading() - _configuration.tare;
	_configuration.calibrationFactor = currentTaredValue / knownWeight;
}

auto Scale::currentReading() const -> const float {
	return (currentRawReading() - _configuration.tare - currentCorrectionFactor) / _configuration.calibrationFactor;
}

auto Scale::recordReading(const long reading) -> void {
	if (weightSampleCount < weightSampleTotalCount) {
		// when don't have a full set of samples, we simply just keep adding samples
		++weightSampleCount;
		currentWeightSum += reading;
	} else {
		// otherwise, we have to update our sum by also removing the oldest sample
		currentWeightSum += reading - weightSamples[weightSampleIdx];
	}

	// store the sample so we can subtract it when it becomes the oldest
	weightSamples[weightSampleIdx++] = reading;

	// wrap our sample index (circular buffer)
	weightSampleIdx %= weightSampleTotalCount;

	// update our correction sum
	currentCorrectionSum += reading;

	// perform correction, if necessary
	if (++correctionSampleCount >= _configuration.correctionPeriod) {
		long currentCorrectionValue = currentCorrectionSum / correctionSampleCount;
		auto delta = (long)(currentCorrectionValue - previousCorrectionValue);
		auto threshold = abs((long)(_configuration.correctionThreshold * _configuration.calibrationFactor));

		if (delta > -threshold && delta < threshold) {
			currentCorrectionFactor += delta;
		}
		// currentCorrectionValue = -541426 (prev = -541429) | delta = 3 | threshold = -234

		Serial.printf("currentCorrectionValue = %ld (prev = %ld) | delta = %ld | threshold = %ld\n", currentCorrectionValue, previousCorrectionValue, delta, threshold);

		previousCorrectionValue = currentCorrectionValue;
		correctionSampleCount = 0;
		currentCorrectionSum = 0;
	}
}

auto Scale::currentRawReading() const -> const long {
	// calculate the current raw rolling average
	if (weightSampleCount == 0) return 0;

	return currentWeightSum / weightSampleCount;
}

auto Scale::readingAge() const -> const long {
	return millis() - lastSampleTime;
}

auto Scale::currentCorrection() const -> const float {
	return currentCorrectionFactor / _configuration.calibrationFactor;
}
