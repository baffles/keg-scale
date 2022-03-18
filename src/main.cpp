#include <Arduino.h>

#include "Scale.hpp"

const ScaleHardware ScaleHw {
	.dataPin = 23,
	.clockPin = 22,
	.gain = 128,
};

const ScaleConfiguration ScaleConfig {
	// .tare = -524375,
	.tare = -524375 - 4074,
	.calibrationFactor = -9379.0f,
	
	// calculate rolling averages over the 15s, reading every 200ms
	// .sampleIntervalMs = 200,
	// .sampleCount = 75,

	// 1 second rolling average reading at 10 hz
	.sampleIntervalMs = 100,
	.sampleCount = 10,

	// correct over a 10s period with a 0.4oz threshold
	.correctionPeriod = 100,
	.correctionThreshold = 0.4f / 16,
};

Scale scale(ScaleConfig);

void setup() {
	Serial.begin(9600);
	
	Serial.println("hi!");

	scale.setup(ScaleHw);
}

unsigned long lastUpdate = 0;

// float previousReadings[10];
// int readingIndex = 0;

// float previous10s = 0.0f;

// float error = 0.0f;

void loop() {
	// put your main code here, to run repeatedly:
	scale.loop();

	if ((millis() - lastUpdate) > 1000) {
		lastUpdate = millis();

		float reading = scale.currentReading();
		auto rawReading = (scale.currentRawReading() - scale.configuration().tare) / scale.configuration().calibrationFactor;

		Serial.printf("SCALE: %.1f lbs (%.1f oz)\t\t(raw: %.2f lbs / %.2f oz)\t\tcorrection factor: %.2f oz\n", reading, reading * 16, rawReading, rawReading * 16, scale.currentCorrection() * 16);

		// float corrected = reading - error;

		// auto delta10s = previousReadings[readingIndex] - corrected;
		// previousReadings[readingIndex] = corrected;

		// if (readingIndex >= 10) {
		// 	auto current10s = previousReadings
		// }
		// readingIndex %= 10;

		// if (delta10s > -0.025 && delta10s < 0.025) {
		// 	error += delta10s;
		// }

		// Serial.printf("RAW: %f lbs (%f oz)\n", reading, reading * 16);
		// Serial.printf("CORRECTED: %.1f lbs (%.1f oz)\n", corrected, corrected * 16);
		// Serial.printf("10s delta: %.1f lbs (%.1f oz) | error: %f\n", delta10s, delta10s * 16, error);
	}
}
