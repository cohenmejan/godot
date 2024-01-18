#define _USE_MATH_DEFINES

//#include <algorithm>

#include "servers/audio_server.h"
#include "audio_effect_modulation_reverb_filters.h"

//using namespace godot;



// DELAY LINE

DelayLine::DelayLine() {
	sample = 0;
	writeIndex = 0;
	readIndex = 0;

	sizeMs = 0;
	size = 0;
	time = 0;

	line = memnew_arr(float, 1);
	line[0] = 0;
}

DelayLine::~DelayLine() {
	memdelete_arr(line);
}

void DelayLine::SetSize(float _size) {
	sizeMs = _size;
	size = (int)(sizeMs * (AudioServer::get_singleton()->get_mix_rate() / 1000)) + 1;

	memdelete_arr(line);
	line = memnew_arr(float, size + 1);

	for (int i = 0; i < size + 1; i++) {
		line[i] = 0;
	}
}

float DelayLine::GetSize() const {
	return sizeMs;
}

void DelayLine::SetTime(float _time) {
	time = _time;

	if (time > sizeMs) {
		SetSize(time);
	}
	
	float readOffset = Math::abs(time) * (AudioServer::get_singleton()->get_mix_rate() / 1000);

	if (readOffset < 1) {
		readOffset = 1;
	}

	readIndex = writeIndex - readOffset;

	while (readIndex < 0) {
		readIndex += size;
	}
}

float DelayLine::GetTime() const {
	return time;
}

void DelayLine::Process(float input) {
	line[writeIndex] = input;
	writeIndex++;

	int readIndexEnd = (int)readIndex + 1;

	if (readIndexEnd >= size) {
		readIndexEnd -= size;
	}

	sample = Math::lerp(line[(int)readIndex], line[readIndexEnd], readIndex - (int)readIndex);

	readIndex++;

	if (writeIndex >= size) {
		writeIndex -= size;
	}

	if (readIndex >= size) {
		readIndex -= size;
	}
}

// ALLPASS FILTER

AllpassFilter::AllpassFilter() {
	amplitude = 0.5;
	sample = 0;
	time = 0;
}

void AllpassFilter::Initialize(float _size, float _time, float _amplitude) {
	amplitude = _amplitude;
	line.SetSize(_size);
	SetTime(_time);
}

void AllpassFilter::Initialize(float _time, float _amplitude) {
	Initialize(_time + 1, _time, _amplitude);
}

void AllpassFilter::SetTime(float _time) {
	time = _time;
	line.SetTime(_time);
}

float AllpassFilter::GetTime() const {
	return time;
}

void AllpassFilter::Process(float input) {
	float feedSample = input + (line.sample * amplitude * -1);
	sample = feedSample * amplitude + line.sample;
	line.Process(feedSample);
}





// POLE FILTER

PoleFilter::PoleFilter() {
	sample = 0;
	cutoff = 2000;

	nyquist = AudioServer::get_singleton()->get_mix_rate() / 2;

	a1 = memnew_arr(float, nyquist);
	b1 = memnew_arr(float, nyquist);

	float frequency = M_PI / AudioServer::get_singleton()->get_mix_rate();

	for (int i = 0; i < nyquist; ++i) {
		a1[i] = sin(i * frequency);
		b1[i] = 1.0 - a1[i];
	}
}

PoleFilter::~PoleFilter() {
	memdelete_arr(a1);
	memdelete_arr(b1);
}

void PoleFilter::Process(float input) {
	int coefficientIndex = CLAMP(cutoff, 0, nyquist);
	sample = input * a1[coefficientIndex] + sample * b1[coefficientIndex];
}





//	RESONANT FILTER

ResonantFilter::ResonantFilter() {
	a1 = memnew_arr(float, 1);
	a1[0] = 0;

	a2 = 0;
	b1 = 0;
	b2 = 0;
	c1 = 0;

	sample = 0;

	frequency = 0;
	resonance = 0;

	n1 = 0;
	n2 = 0;
}

ResonantFilter::~ResonantFilter() {
	memdelete_arr(a1);
}

void ResonantFilter::Initialize(float _frequency, float _resonance) {
	InitializeCoefficients();
	SetFrequency(_frequency);
	SetResonance(_resonance);
}

void ResonantFilter::SetFrequency(int _frequency) {
	frequency = _frequency;
	RecalculateA2Coefficient();
}

int ResonantFilter::GetFrequency() const {
	return frequency;
}

void ResonantFilter::SetResonance(float _resonance) {
	resonance = _resonance;

	b1 = exp(CLAMP(resonance, 0.0, 0.999999) * 0.125) * 0.882497;
	b2 = b1 * b1;
	RecalculateA2Coefficient();
}

float ResonantFilter::GetResonance() const {
	return resonance;
}

void ResonantFilter::InitializeCoefficients() {
	int nyquist = AudioServer::get_singleton()->get_mix_rate() / 2;

	memdelete_arr(a1);
	a1 = memnew_arr(float, nyquist);

	for (int i = 0; i < nyquist; ++i) {
		a1[i] = cos(i * (M_PI * 2) / AudioServer::get_singleton()->get_mix_rate());
	}
}

void ResonantFilter::RecalculateA2Coefficient() {
	a2 = a1[CLAMP(frequency, 0, (int)(AudioServer::get_singleton()->get_mix_rate() / 2) - 1)] * b1 * -2;
	c1 = a2 + b2 + 1;
}

void ResonantFilter::Process(const float &input) {
	sample = (input * c1) - (a2 * n1 + b2 * n2);

	n2 = n1;
	n1 = sample;
}
