#pragma once

/*  This file is part of 2600.emu.

	2600.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	2600.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with 2600.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <stella/emucore/Sound.hxx>
#include <OSystem.hxx>

class AudioQueue;
class Resampler;
class EmulationTiming;
class EmuAudio;

class SoundEmuEx : public Sound
{
public:
	SoundEmuEx(OSystem& osystem);
	SoundEmuEx() = delete;
	SoundEmuEx(const SoundEmuEx&) = delete;
	SoundEmuEx(SoundEmuEx&&) = delete;
	SoundEmuEx& operator=(const SoundEmuEx&) = delete;
	SoundEmuEx& operator=(SoundEmuEx&&) = delete;
	void open(shared_ptr<AudioQueue> audioQueue, EmulationTiming* emulationTiming);
	void close() final;
	void setFrameTime(OSystem &osystem, double frameTime, unsigned int soundRate);
	void setResampleQuality(AudioSettings::ResamplingQuality quality);
	void processAudio(OSystem &osystem, EmuAudio *audio);
	void setEnabled(bool enable) final;
	bool mute(bool state) final;
	bool toggleMute() final;
	void setVolume(uInt32 percent) final;
	void adjustVolume(Int8 direction) final;
	string about() const final;

private:
	shared_ptr<AudioQueue> audioQueue{};
	unique_ptr<Resampler> myResampler{};
	EmulationTiming *emulationTiming{};
	Int16 *currentFragment{};
	double frameTime{};
	double configuredVideoFrameRate{};
	uint32_t soundRate{};
	AudioSettings::ResamplingQuality resampleQuality{AudioSettings::DEFAULT_RESAMPLING_QUALITY};

	void configForVideoFrameRate(double rate);
};
