#pragma once

#include <string>
#include <map>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "fmod.hpp"
using namespace std;

class AudioManager
{
public:
	AudioManager();
	~AudioManager();
	void Update(float elapsed);

	void LoadSFX(const string &path);
	void LoadSong(const string &path);

	void PlaySFX(const string &path, float minVolume, float maxVolume, float minPitch, float maxPitch);
	void PlaySong(const string &path);

	void StopSFXs();
	void StopSongs();

	void SetMasterVolume(float volume);
	void SetSFXsVolume(float volume);
	void SetSongsVolume(float volume);

private:
	typedef std::map<std::string, FMOD::Sound*> SoundMap;
	enum Category{ CATEGORY_SFX, CATEGORY_SONG, CATEGORY_COUNT};

	void Load(Category type, const string &path);

	float RandomBetween(float min, float max);
	float ChangeOctave(float frequency, float variation);
	float ChangeSemitone(float frequency, float variation);



	FMOD::System* pSystem;
	FMOD::ChannelGroup* master;
	FMOD::ChannelGroup* groups[CATEGORY_COUNT];
	SoundMap sounds[CATEGORY_COUNT];
	FMOD_MODE modes[CATEGORY_COUNT];

	FMOD::Channel* currentSong;
	string currentSongPath;
	string nextSongPath;


	enum FadeState{FADE_NONE, FADE_IN, FADE_OUT};
	FadeState fade;
};