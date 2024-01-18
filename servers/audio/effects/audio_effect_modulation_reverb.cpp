#include "audio_effect_modulation_reverb.h"

AudioEffectModulationReverbInstance::AudioEffectModulationReverbInstance() {
	allpassL[0].Initialize(4.76f, 0.75f);
	allpassL[1].Initialize(3.58f, 0.75f);
	allpassL[2].Initialize(12.73f, 0.625f);
	allpassL[3].Initialize(9.3f, 0.625f);
	allpassL[4].Initialize(100.0f, 0, -0.7f);
	allpassL[5].Initialize(300.0f, 0, 0.5f);

	allpassR[0].Initialize(4.76f, 0.75f);
	allpassR[1].Initialize(3.58f, 0.75f);
	allpassR[2].Initialize(12.73f, 0.625f);
	allpassR[3].Initialize(9.3f, 0.625f);
	allpassR[4].Initialize(100.0f, 0, -0.7f);
	allpassR[5].Initialize(300.0f, 0, 0.5f);

	modulationL.Initialize(1, 0.995f);
	modulationR.Initialize(1, 0.995f);

	combL[0].SetSize(3000);
	combL[1].SetSize(400);
	combL[2].SetSize(500);
	combL[3].SetSize(200);
	combL[4].SetSize(100);
	combL[5].SetSize(300);
	combL[6].SetSize(300);
	combL[7].SetSize(300);

	combR[0].SetSize(3000);
	combR[1].SetSize(400);
	combR[2].SetSize(500);
	combR[3].SetSize(200);
	combR[4].SetSize(100);
	combR[5].SetSize(300);
	combR[6].SetSize(300);
	combR[7].SetSize(300);

	tapL.SetSize(500);
	tapR.SetSize(500);
}

void AudioEffectModulationReverbInstance::process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
	allpassL[4].SetTime(base->sizeTimes[0]);
	allpassL[5].SetTime(base->sizeTimes[1]);
	allpassR[4].SetTime(base->sizeTimes[2]);
	allpassR[5].SetTime(base->sizeTimes[3]);
	tapL.SetTime(base->sizeTimes[4]);
	tapR.SetTime(base->sizeTimes[5]);

	poleL[0].cutoff = (int)base->damping;
	poleL[1].cutoff = (int)base->damping;
	poleR[0].cutoff = (int)base->damping;
	poleR[1].cutoff = (int)base->damping;

	hpL.cutoff = (int)base->highpass;
	hpR.cutoff = (int)base->highpass;

	combL[1].SetTime(base->diffusionTimes[0]);
	combL[2].SetTime(base->diffusionTimes[1]);
	combL[3].SetTime(base->diffusionTimes[2]);
	combL[4].SetTime(base->diffusionTimes[3]);
	combL[5].SetTime(base->diffusionTimes[4]);
	combL[6].SetTime(base->diffusionTimes[5]);
	combL[7].SetTime(base->diffusionTimes[6]);

	combR[1].SetTime(base->diffusionTimes[7]);
	combR[2].SetTime(base->diffusionTimes[8]);
	combR[3].SetTime(base->diffusionTimes[9]);
	combR[4].SetTime(base->diffusionTimes[10]);
	combR[5].SetTime(base->diffusionTimes[11]);
	combR[6].SetTime(base->diffusionTimes[12]);
	combR[7].SetTime(base->diffusionTimes[13]);

	for (int i = 0; i < p_frame_count; i++) {
		poleL[0].Process(p_src_frames[i].l);
		poleR[0].Process(p_src_frames[i].r);

		allpassL[0].Process(poleL[0].sample);
		allpassL[1].Process(allpassL[0].sample);
		allpassL[2].Process(allpassL[1].sample);
		allpassL[3].Process(allpassL[2].sample);

		allpassR[0].Process(poleR[0].sample);
		allpassR[1].Process(allpassR[0].sample);
		allpassR[2].Process(allpassR[1].sample);
		allpassR[3].Process(allpassR[2].sample);

		allpassL[4].Process(allpassL[3].sample + tapL.sample);
		allpassR[4].Process(allpassR[3].sample + tapR.sample);

		modulationL.Process(Math::random(-1.0f, 1.0f));
		combL[0].SetTime(base->size * 9.6f + modulationL.sample * base->modulation);
		combL[0].Process(allpassL[4].sample);

		modulationR.Process(Math::random(-1.0f, 1.0f));
		combR[0].SetTime(base->size * 7.7f + modulationR.sample * base->modulation);
		combR[0].Process(allpassR[4].sample);

		poleL[1].Process(combL[0].sample);
		poleR[1].Process(combR[0].sample);

		hpL.Process(poleL[1].sample);
		hpR.Process(poleR[1].sample);

		allpassL[5].Process((poleL[1].sample - hpL.sample) * base->decay);
		allpassR[5].Process((poleR[1].sample - hpR.sample) * base->decay);

		tapL.Process(allpassL[5].sample);
		tapR.Process(allpassR[5].sample);

		combL[1].Process(allpassL[4].sample);
		combL[2].Process(allpassL[4].sample);
		combL[3].Process(allpassL[5].sample);
		combL[4].Process(tapL.sample);
		combL[5].Process(combR[0].sample);
		combL[6].Process(allpassR[5].sample);
		combL[7].Process(tapR.sample);

		combR[1].Process(allpassR[4].sample);
		combR[2].Process(allpassR[4].sample);
		combR[3].Process(allpassR[5].sample);
		combR[4].Process(tapR.sample);
		combR[5].Process(combL[0].sample);
		combR[6].Process(allpassL[5].sample);
		combR[7].Process(tapL.sample);

		float wetL = (combL[1].sample + combL[2].sample + combL[4].sample + hpL.sample +
				((combL[3].sample + combL[5].sample + combL[6].sample + combL[7].sample) * -1));

		float wetR = (combR[1].sample + combR[2].sample + combR[4].sample + hpR.sample +
				((combR[3].sample + combR[5].sample + combR[6].sample + combR[7].sample) * -1));

		float m = wetL + wetR;
		float s = (wetL - wetR) * base->width;

		AudioFrame wetSample(m + s, m - s);
		wetSample *= base->wet * 0.25f;

		p_dst_frames[i] = (p_src_frames[i] * base->dry) + wetSample;
	}
}

AudioEffectModulationReverb::AudioEffectModulationReverb() {
	SetDry(1.0f);
	SetWet(0.5f);
	SetSize(2.0f);
	SetDecay(0.75f);
	SetDamping(5000.0f);
	SetHighpass(50.0f);
	SetDiffusion(0.5f);
	SetModulation(25.0f);
	SetWidth(0.75f);
}

Ref<AudioEffectInstance> AudioEffectModulationReverb::instantiate() {
	Ref<AudioEffectModulationReverbInstance> ins;
	ins.instantiate();
	ins->base = Ref<AudioEffectModulationReverb>(this);
	return ins;
}

void AudioEffectModulationReverb::SetDry(float _dry) {
	dry = _dry;
}

float AudioEffectModulationReverb::GetDry() const {
	return dry;
}

void AudioEffectModulationReverb::SetWet(float _wet) {
	wet = _wet;
}

float AudioEffectModulationReverb::GetWet() const {
	return wet;
}

void AudioEffectModulationReverb::SetDecay(float _decay) {
	decay = _decay;
}

float AudioEffectModulationReverb::GetDecay() const {
	return decay;
}

void AudioEffectModulationReverb::SetDamping(float _damping) {
	damping = _damping;
}

float AudioEffectModulationReverb::GetDamping() const {
	return damping;
}

void AudioEffectModulationReverb::SetHighpass(float _highpass) {
	highpass = _highpass;
}

float AudioEffectModulationReverb::GetHighpass() const {
	return highpass;
}

void AudioEffectModulationReverb::SetSize(float _size) {
	size = _size;
	RecalculateDiffusion();

	sizeTimes[0] = size * 22.58f;
	sizeTimes[1] = size * 60.48f;
	sizeTimes[2] = size * 20.51f;
	sizeTimes[3] = size * 64.24f;
	sizeTimes[4] = size * 25.0f;
	sizeTimes[5] = size * 27.31f;
}

float AudioEffectModulationReverb::GetSize() const {
	return size;
}

void AudioEffectModulationReverb::SetDiffusion(float _diffusion) {
	diffusion = _diffusion;
	RecalculateDiffusion();
}

float AudioEffectModulationReverb::GetDiffusion() const {
	return diffusion;
}

void AudioEffectModulationReverb::SetModulation(float _modulation) {
	modulation = _modulation;
}

float AudioEffectModulationReverb::GetModulation() const {
	return modulation;
}

void AudioEffectModulationReverb::SetWidth(float _width) {
	width = _width;
}

float AudioEffectModulationReverb::GetWidth() const {
	return width;
}

void AudioEffectModulationReverb::RecalculateDiffusion() {
	float diffusionSize = diffusion * size;

	diffusionTimes[0] = diffusionSize * 31.0f;
	diffusionTimes[1] = diffusionSize * 40.0f;
	diffusionTimes[2] = diffusionSize * 50.0f;
	diffusionTimes[3] = diffusionSize * 25.0f;
	diffusionTimes[4] = diffusionSize * 69.0f;
	diffusionTimes[5] = diffusionSize * 80.0f;
	diffusionTimes[6] = diffusionSize * 75.0f;
	diffusionTimes[7] = diffusionSize * 37.0f;
	diffusionTimes[8] = diffusionSize * 35.0f;
	diffusionTimes[9] = diffusionSize * 55.0f;
	diffusionTimes[10] = diffusionSize * 23.0f;
	diffusionTimes[11] = diffusionSize * 72.0f;
	diffusionTimes[12] = diffusionSize * 77.0f;
	diffusionTimes[13] = diffusionSize * 82.0f;
}

void AudioEffectModulationReverb::_bind_methods() {
	ClassDB::bind_method(D_METHOD("SetDry", "dry"), &AudioEffectModulationReverb::SetDry);
	ClassDB::bind_method(D_METHOD("GetDry"), &AudioEffectModulationReverb::GetDry);

	ClassDB::bind_method(D_METHOD("SetWet", "wet"), &AudioEffectModulationReverb::SetWet);
	ClassDB::bind_method(D_METHOD("GetWet"), &AudioEffectModulationReverb::GetWet);

	ClassDB::bind_method(D_METHOD("SetDecay", "decay"), &AudioEffectModulationReverb::SetDecay);
	ClassDB::bind_method(D_METHOD("GetDecay"), &AudioEffectModulationReverb::GetDecay);

	ClassDB::bind_method(D_METHOD("SetSize", "size"), &AudioEffectModulationReverb::SetSize);
	ClassDB::bind_method(D_METHOD("GetSize"), &AudioEffectModulationReverb::GetSize);

	ClassDB::bind_method(D_METHOD("SetDamping", "damping"), &AudioEffectModulationReverb::SetDamping);
	ClassDB::bind_method(D_METHOD("GetDamping"), &AudioEffectModulationReverb::GetDamping);

	ClassDB::bind_method(D_METHOD("SetHighpass", "highpass"), &AudioEffectModulationReverb::SetHighpass);
	ClassDB::bind_method(D_METHOD("GetHighpass"), &AudioEffectModulationReverb::GetHighpass);

	ClassDB::bind_method(D_METHOD("SetDiffusion", "diffusion"), &AudioEffectModulationReverb::SetDiffusion);
	ClassDB::bind_method(D_METHOD("GetDiffusion"), &AudioEffectModulationReverb::GetDiffusion);

	ClassDB::bind_method(D_METHOD("SetModulation", "modulation"), &AudioEffectModulationReverb::SetModulation);
	ClassDB::bind_method(D_METHOD("GetModulation"), &AudioEffectModulationReverb::GetModulation);

	ClassDB::bind_method(D_METHOD("SetWidth", "width"), &AudioEffectModulationReverb::SetWidth);
	ClassDB::bind_method(D_METHOD("GetWidth"), &AudioEffectModulationReverb::GetWidth);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dry", PROPERTY_HINT_RANGE, "0,1,0.001"), "SetDry", "GetDry");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "wet", PROPERTY_HINT_RANGE, "0,1,0.001"), "SetWet", "GetWet");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "decay", PROPERTY_HINT_RANGE, "0,1,0.001"), "SetDecay", "GetDecay");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "size", PROPERTY_HINT_RANGE, "0,10,0.001"), "SetSize", "GetSize");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "damping", PROPERTY_HINT_RANGE, "1,20000,0.001"), "SetDamping", "GetDamping");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "highpass", PROPERTY_HINT_RANGE, "1,20000,0.001"), "SetHighpass", "GetHighpass");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "diffusion", PROPERTY_HINT_RANGE, "0,1,0.001"), "SetDiffusion", "GetDiffusion");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "modulation", PROPERTY_HINT_RANGE, "0,200,0.001"), "SetModulation", "GetModulation");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width", PROPERTY_HINT_RANGE, "0,1,0.001"), "SetWidth", "GetWidth");
}
