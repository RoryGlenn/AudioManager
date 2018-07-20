#include "AudioManager.h"

AudioManager::AudioManager()	:	currentSong(0), fade(FADE_NONE)
{
	// Initialize the system
	FMOD::System_Create(&pSystem);
	pSystem->init(128, FMOD_INIT_NORMAL, 0);

	// Create channels groups for each category
	pSystem->getMasterChannelGroup(&master);
	for (int i = 0; i < CATEGORY_COUNT; ++i)
	{
		pSystem->createChannelGroup(0, &groups[i]);
		master->addGroup(groups[i]);
	}

	// Set up modes for each category
	modes[CATEGORY_SFX] = FMOD_DEFAULT;
	modes[CATEGORY_SONG] = FMOD_DEFAULT | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL;

	// Seed random number generator for SFX's
	srand(time(0));
}

AudioManager::~AudioManager()
{
	// Release sounds in each category
	SoundMap::iterator iter;
	for (int i = 0; i < CATEGORY_COUNT; ++i)
	{
		for (iter = sounds[i].begin(); iter != sounds[i].end(); ++iter)
		{
			iter->second->release();
			sounds[i].clear();
		}
		// Release the system
		pSystem->release();
	}

}

void AudioManager::LoadSFX(const string &path) {
	Load(CATEGORY_SFX, path);
}

void AudioManager::LoadSong(const string &path) {
	Load(CATEGORY_SONG, path);
}

void AudioManager::Load(Category type, const string &path) {
	if (sounds[type].find(path) != sounds[type].end()) {
		return;
	}

	FMOD::Sound *pSound;
	pSystem->createSound(path.c_str(), modes[type], 0, &pSound);
	sounds[type].insert(make_pair(path, pSound));
}

void AudioManager::PlaySFX(const string &path, float minVolume, float maxVolume, float minPitch, float maxPitch) {

	// Try to find sound effect and return if not found
	SoundMap::iterator sound = sounds[CATEGORY_SFX].find(path);
	if (sound == sounds[CATEGORY_SFX].end()) return;

	// Calculate random volume and pitch in selected range
	float volume = RandomBetween(minVolume, maxVolume);
	float pitch = RandomBetween(minPitch, maxPitch);

	// Play the sound effect with these initial values
	FMOD::Channel *pChannel;
	pSystem->playSound(sound->second, 0, false, &pChannel);
	pChannel->setChannelGroup(groups[CATEGORY_SFX]);
	pChannel->setVolume(volume);
	float frequency;
	pChannel->getFrequency(&frequency);
	pChannel->setFrequency(ChangeSemitone(frequency, pitch));
	pChannel->setPaused(false);
} 

void AudioManager::StopSFXs() {
	groups[CATEGORY_SFX]->stop();
}

void AudioManager::PlaySong(const string &path) {
	// Ignore if this song is already playing
	if (currentSongPath == path) return;

	// If a song is playing stop them and set this as the next song
	if (currentSong != 0) {
		StopSongs();
		nextSongPath = path;
		return;
	}

	// Find the song in the corresponding sound map
	SoundMap::iterator sound = sounds[CATEGORY_SONG].find(path);
	if (sound == sounds[CATEGORY_SONG].end()) return;

	// Start playing song with volume set to 0 and fade in
	currentSongPath = path;
	pSystem->playSound(sound->second, FMOD_DEFAULT, true, &currentSong);
	currentSong->setChannelGroup(groups[CATEGORY_SONG]);
	currentSong->setPaused(false);
	fade = FADE_IN;
}

void AudioManager::StopSongs() {
	if (currentSong != 0)
	{
		fade = FADE_OUT;
		nextSongPath.clear();
	}
}

void AudioManager::Update(float elapsed) {
	const float fadeTime = 1.0f;	// in seconds
	if (currentSong != 0 && fade == FADE_IN)
	{
		float volume;
		currentSong->getVolume(&volume);
		float nextVolume = volume + elapsed / fadeTime;

		if (nextVolume >= 1.0f) {
			currentSong->setVolume(1.0f);
			fade = FADE_NONE;
		}
		else {
			currentSong->setVolume(nextVolume);
		}
	}
	else if (currentSong != 0 && fade == FADE_OUT) {
		float volume;
		currentSong->getVolume(&volume);
		float nextVolume = volume - elapsed / fadeTime;
		if (nextVolume <= 0.0f) {
			currentSong->stop();
			currentSong = 0;
			currentSongPath.clear();
			fade = FADE_NONE;
		}
		else {
			currentSong->setVolume(nextVolume);
		}
	}
	else if (currentSong == 0 && !nextSongPath.empty()) {
		PlaySong(nextSongPath);
		nextSongPath.clear();
	}
	pSystem->update();
}

void AudioManager::SetMasterVolume(float volume) {
	master->setVolume(volume);
}

void AudioManager::SetSFXsVolume(float volume) {
	groups[CATEGORY_SFX]->setVolume(volume);
}

void AudioManager::SetSongsVolume(float volume) {
	groups[CATEGORY_SONG]->setVolume(volume);
}

float ChangeOctave(float frequency, float variation) {
	static float octave_ratio = 2.0f;
	return frequency * pow(octave_ratio, variation);
}

float ChangeSemitone(float frequency, float variation) {
	static float semitone_ratio = pow(2.0f, 1.0f / 12.0f);
	return frequency * pow(semitone_ratio, variation);
}

float RandomBetween(float min, float max) {
	if (min == max)
	{
		return min;
	}
	float n = (float)rand() / (float)RAND_MAX;

	return min + n * (max - min);
}