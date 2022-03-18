#ifndef Scale_hpp
#define Scale_hpp

#include <HX711.h>

//TODO: share with CO2 scale codebase

struct ScaleHardware {
	/** The data and clock pins that are wired to the HX711. */
	const int dataPin, clockPin;

	/** The gain value to use (32 for channel B, 64 or 128 for channel A). */
	const byte gain;
};

struct ScaleConfiguration {
	/** The raw tare (zero) value for the scale. */
	long tare = 0;

	/** The calibration factor for raw to units. */
	float calibrationFactor = 0;

	/** The interval (in milliseconds) at which to collect samples from the scale. */
	unsigned int sampleIntervalMs = 500;

	/** The number of samples (intervals) to calculate the rolling average weight over. */
	unsigned int sampleCount = 120;

	/** The error correction period (a multiple of sampleIntervalMs). */
	unsigned int correctionPeriod = 20;

	/** The correction threshold (in calibrated units). */
	float correctionThreshold = 0.025f; // maybe "limit" is a better name it's the limit of how much drift we can have, or the threshold of how much weight change we want to measure
};

/** Scale interface that remains as asynchronous as possible while continually taking readings from
  * the scale and maintaining a rolling average of the weight on the scale.
  * 
  * This implementation uses the HX711 library, but it only leverages it for getting the raw scale
  * data. The handling of offset, scaling, and taring is handled by us.
  */
class Scale {
public:

	Scale();
	Scale(ScaleConfiguration initialConfiguration);
	~Scale();

	/** Core initialization of the hardware interface; to be called on boot. */
	auto setup(const ScaleHardware hwInfo) -> void;

	/** The looping logic that periodically collects samples. */
	auto loop() -> void;

	auto configuration()       ->       ScaleConfiguration&;
	auto configuration() const -> const ScaleConfiguration&;

	/** Re-apply the current configuration. This is to be called any time the configuration of the
	  * scale is changed (after the initial `setup` call).
	  */
	auto reconfigure() -> void;

	/** Tares the scale, such that the current reading will be adjusted to zero. The current
	  * rolling average value is used as the tare, so be sure that the scale has been empty for the
	  * full sampling period before calling this.
	  */
	auto tare() -> void;

	/** Calibrates the scale to a known current weight. Like with `tare`, the current rolling
	  * average value is used for the calibration calculation, so be sure that the scale has been
	  * sitting with the known weight on it for the full sampling period before calling this.
	  */
	auto calibrate(float knownWeight) -> void;

	/** Returns the current rolling average scale reading for the configured sampling interval. */
	auto currentReading() const -> const float;

	/** Returns the age of the last sample, in milliseconds. */
	auto readingAge() const -> const long;

	auto currentRawReading() const -> const long;
	auto currentCorrection() const -> const float;

private:

	HX711 scaleInterface;
	ScaleConfiguration _configuration;

	unsigned int weightSampleTotalCount, weightSampleCount, weightSampleIdx;
	long *weightSamples;
	long long currentWeightSum;

	long currentCorrectionFactor;
	unsigned int correctionSampleCount;
	long long currentCorrectionSum;
	long previousCorrectionValue;

	auto recordReading(const long reading) -> void;
	// auto currentRawReading() const -> const long;

	long lastSampleTime;

};

#endif