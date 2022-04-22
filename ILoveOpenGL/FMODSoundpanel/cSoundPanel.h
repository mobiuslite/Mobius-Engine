/*
	Contributers:			Ethan Robertson
							Gian Tullo

	Purpose:				A Class that interfaces with the console client for stream and sample controls
	Date:					September 28, 2021
*/

#pragma once
#include <FMOD/fmod.hpp>
#include <cstddef>
#include <vector>
#include <string>

struct Sound
{
	FMOD::Sound* sound = NULL;
	std::string name = "";
};

class cSoundPanel
{
public:

	// << CONSTRUCTORS >>

	//Sets up the system to be able to play music. Also loads music.
	cSoundPanel();
	~cSoundPanel();

	cSoundPanel(cSoundPanel& other) = delete;
	cSoundPanel& operator=(cSoundPanel& other) = delete;

	//Gets the instance of the scene loader
	static cSoundPanel* GetInstance();

	// << MUSIC CONTROLS >> Returns true if successful.

	//Plays a streamed song, use the same file name found in the music folder
	bool PlayMusic(std::string musicName);
	//Pauses the music channel
	bool SetPauseMusic(bool paused);
	//Stops the music channel.
	bool StopMusic();

	//Gets and Sets the music's frequency (speed)
	float GetMusicFrequency();
	void SetMusicFrequency(float frequency);

	//Gets and Sets the music's volume
	float GetMusicVolume();
	void SetMusicVolume(float volume);
	void SetBGVolume(float volume);

	//Gets and Sets the music's reverb wetness
	float GetReverb();
	void SetReverb(float wet);

	//Gets and Sets the music's lowpass wetness
	float GetLowpass();
	void SetLowpass(float wet);

	//Sets the background (rain) lowpass wetness
	void SetLowpassBG(float wet);

	//Sets the Balance/Pan
	void AddMusicBalance(float balance);
	float GetMusicBalance();

	//Plays a sampled sound
	bool PlaySound(std::string soundName);

	// << GETTERS >>
	
	//Returns the current list of music sounds in the soundpanel
	std::vector<Sound> GetMusicList();

	//Returns the currently playing music track.
	Sound GetCurrentMusic();
	//Returns the length of the song in milliseconds.
	unsigned int GetCurrentMusicLength();

	bool IsMusicPaused();

private:
	//internal values
	FMOD::System* system;
	FMOD::Channel* musicChannel = NULL;
	FMOD::Channel* soundEffectChannel = NULL;
	FMOD::Channel* backgroundChannel = NULL;

	FMOD_RESULT result;

	std::vector<Sound> musicList;
	std::vector<Sound> soundEffectList;

	Sound* currentlyPlayingMusic;
	Sound* bgSound;
	bool musicIsPaused;

	float musicPan;

	//Settings for the system's reverb
	FMOD_REVERB_PROPERTIES* reverbProperties;

	//Loads the music, sound, and background from text files.
	void LoadMusic();
	void LoadSounds();
	void LoadBg();

	static cSoundPanel _instance;
};