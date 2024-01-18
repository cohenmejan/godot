
#ifndef AUDIO_EFFECT_MODULATION_REVERB_H
#define AUDIO_EFFECT_MODULATION_REVERB_H

#include "servers/audio/audio_effect.h"
#include "audio_effect_modulation_reverb_filters.h"

class AudioEffectModulationReverbInstance : public AudioEffectInstance {
    GDCLASS(AudioEffectModulationReverbInstance, AudioEffectInstance)
    friend class AudioEffectModulationReverb;
	Ref<AudioEffectModulationReverb> base;

	PoleFilter poleL[2];
	PoleFilter poleR[2];

	PoleFilter hpL;
	PoleFilter hpR;

	AllpassFilter allpassL[6];
	AllpassFilter allpassR[6];

	ResonantFilter modulationL;
	ResonantFilter modulationR;

	DelayLine combL[8];
	DelayLine combR[8];

	DelayLine tapL;
	DelayLine tapR;

public:
	virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;

	AudioEffectModulationReverbInstance();
};

class AudioEffectModulationReverb : public AudioEffect {
    GDCLASS(AudioEffectModulationReverb, AudioEffect)
    friend class AudioEffectModulationReverbInstance;

	float dry;
	float wet;
	float decay;
	float size;
	float damping;
	float highpass;
	float diffusion;
	float modulation;
	float width;

protected:
	static void _bind_methods();

	float sizeTimes[6];
	float diffusionTimes[14];

public:
	Ref<AudioEffectInstance> instantiate() override;

	void SetDry(float _size);
	float GetDry() const;

	void SetWet(float _size);
	float GetWet() const;

	void SetDecay(float _size);
	float GetDecay() const;

	void SetSize(float _size);
	float GetSize() const;

	void SetDamping(float _size);
	float GetDamping() const;

	void SetHighpass(float _size);
	float GetHighpass() const;

	void SetDiffusion(float _size);
	float GetDiffusion() const;

	void SetModulation(float _size);
	float GetModulation() const;

	void SetWidth(float _width);
	float GetWidth() const;

	void RecalculateDiffusion();

    AudioEffectModulationReverb();
};

#endif // AUDIO_EFFECT_MODULATION_REVERB_H
