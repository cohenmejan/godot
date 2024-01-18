
#ifndef AUDIO_EFFECT_MODULATION_REVERB_FILTERS_H
#define AUDIO_EFFECT_MODULATION_REVERB_FILTERS_H

struct DelayLine
{
	DelayLine();
	~DelayLine();

	float sample;
	void Process(float input);

	void SetSize(float _time);
	float GetSize() const;

	void SetTime(float _time);
	float GetTime() const;

private:
	int writeIndex;
	float readIndex;

	int size;
	float sizeMs;
	float time;

	float *line;
};

struct AllpassFilter {
	AllpassFilter();

	void Initialize(float _size, float _time, float _amplitude);
	void Initialize(float _time, float _amplitude);

	float amplitude;

	float sample;
	DelayLine line;

	void SetTime(float _time);
	float GetTime() const;

	void Process(float input);

private:
	float time;
};

struct PoleFilter {
	PoleFilter();
	~PoleFilter();

	float sample;
	int cutoff;

	void Process(float input);

private:
	float *a1 = nullptr;
	float *b1 = nullptr;

	int nyquist;
};

struct ResonantFilter
{
	ResonantFilter();
	~ResonantFilter();

	void SetFrequency(int _frequency);
	int GetFrequency() const;

	void SetResonance(float _resonance);
	float GetResonance() const;

	void Initialize(float _frequency, float _resonance);

	float *a1 = nullptr;
	float a2;
	float b1;
	float b2;
	float c1;

	float sample;

	void Process(const float &input);

	void InitializeCoefficients();
	void RecalculateA2Coefficient();

protected:

	int frequency;
	float resonance;

	float n1;
	float n2;
};

#endif // AUDIO_EFFECT_MODULATION_REVERB_FILTERS_H
