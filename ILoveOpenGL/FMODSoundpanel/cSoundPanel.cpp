/*
	Contributers:			Ethan Robertson
							Gian Tullo

	Purpose:				Implementation of the cSoundPanel class
	Date:					September 28, 2021
*/

#include "cSoundPanel.h"
#include <iostream>
#include <sstream>
#include <fstream>

cSoundPanel::cSoundPanel()
{
	result = FMOD::System_Create(&system);
	if (result != FMOD_OK)
	{
		std::cout << "Unable to create FMOD system" << std::endl;
	}
	else
	{

		result = system->init(32, FMOD_INIT_CHANNEL_LOWPASS, NULL);
		if (result != FMOD_OK)
		{
			std::cout << "Unable to initialize FMOD system" << std::endl;
		}
		else
		{

			LoadMusic();
			LoadSounds();
			LoadBg();

			musicPan = 0;
			musicIsPaused = true;
			musicChannel->setPaused(true);

			reverbProperties = new FMOD_REVERB_PROPERTIES(FMOD_PRESET_STONEROOM);
			reverbProperties->WetLevel = -80.0f;
			system->setReverbProperties(0, reverbProperties);

			std::cout << "FMOD sound panel created succesfully!" << std::endl;
		}
	}
}

cSoundPanel* cSoundPanel::GetInstance()
{
	return &cSoundPanel::_instance;
}
cSoundPanel cSoundPanel::_instance;

cSoundPanel::~cSoundPanel()
{
	//Removes the background stream
	bgSound->sound->release();
	delete bgSound;

	//Release all music in the music list.
	for (int i = 0; i < musicList.size(); i++)
	{
		musicList[i].sound->release();
	}

	//Release all sounds in the sound effects list.
	for (int i = 0; i < soundEffectList.size(); i++)
	{
		soundEffectList[i].sound->release();
	}

	delete reverbProperties;

	result = system->close();
	if (result != FMOD_OK)
	{
		std::cout << "ERROR: Unable to close system" << std::endl;
	}

	result = system->release();
	if (result != FMOD_OK)
	{
		std::cout << "ERROR: Unable to release system" << std::endl;
	}
}

bool cSoundPanel::PlayMusic(std::string musicName)
{
	for (int i = 0; i < musicList.size(); i++)
	{
		if (musicList[i].name == musicName)
		{
			bool setUp = musicChannel == nullptr;

			//If a song is currently playing, stop before playing another song.
			if (currentlyPlayingMusic != nullptr)
			{
				musicChannel->stop();
			}

			//Play created sound
			result = system->playSound(musicList[i].sound, 0, false, &musicChannel);
			if (result != FMOD_OK)
			{
				std::cout << "ERROR: Found, but was unable to play music: " << musicList[i].name << std::endl;
				return false;
			}

			if (setUp)
			{
				musicChannel->setReverbProperties(0, 1);
				musicChannel->setLowPassGain(1);
				musicChannel->setVolume(this->GetMusicVolume() - 0.2f);
			}
			
			if (musicIsPaused)
				musicChannel->setPaused(true);

			currentlyPlayingMusic = &musicList[i];

			return true;
		}
	}
	std::cout << "ERROR: Couldn't find song " << musicName << ". It was not found in the music array." << std::endl;
	return false;
}

bool cSoundPanel::SetPauseMusic(bool paused)
{
	if (musicChannel)
	{
		result = musicChannel->setPaused(paused);
		if (result != FMOD_OK)
		{
			std::cout << "ERROR: Could not set paused from music channel" << std::endl;
			return false;
		}

		musicIsPaused = paused;
		return true;
	}

	return false;
}

bool cSoundPanel::StopMusic()
{
	if (musicChannel)
	{
		musicChannel->stop();
		return true;
	}

	return false;
}

float cSoundPanel::GetMusicFrequency()
{
	float frequency;
	musicChannel->getFrequency(&frequency);
	return frequency;
}

void cSoundPanel::SetMusicFrequency(float frequency)
{
	musicChannel->setFrequency(frequency);
}

float cSoundPanel::GetMusicVolume()
{
	float volume;
	musicChannel->getVolume(&volume);
	return volume;
}

void cSoundPanel::SetMusicVolume(float volume)
{
	if(volume > 0.0f)
		musicChannel->setVolume(volume);
}

void cSoundPanel::SetBGVolume(float volume)
{
	if (volume > 0.0f)
		backgroundChannel->setVolume(volume);
}

float cSoundPanel::GetReverb()
{
	return reverbProperties->WetLevel;
}
void cSoundPanel::SetReverb(float wet)
{
	if (wet <= 20.0f && wet >= -80.0f)
	{
		reverbProperties->WetLevel = wet;
	}

	system->setReverbProperties(0, reverbProperties);
}

float cSoundPanel::GetLowpass()
{
	float lowpass;
	musicChannel->getLowPassGain(&lowpass);
	return lowpass;
}
void cSoundPanel::SetLowpass(float wet)
{
	if (wet <= 1.0f && wet >= 0.0f)
	{
		musicChannel->setLowPassGain(wet);
	}
}

void cSoundPanel::SetLowpassBG(float wet)
{
	backgroundChannel->setLowPassGain(wet);
}

void cSoundPanel::AddMusicBalance(float pan)
{
	if (musicPan < 1.0f && musicPan > -1.0f || (musicPan < 0.0f && pan > 0.0f) || (musicPan > 0.0f && pan < 0.0f))
	{
		musicChannel->setPan(musicPan + pan);
		musicPan += pan;
	}
}

float cSoundPanel::GetMusicBalance()
{
	return musicPan;
}

bool cSoundPanel::PlaySound(std::string soundName)
{
	for (int i = 0; i < soundEffectList.size(); i++)
	{
		if (soundEffectList[i].name == soundName)
		{
			//Play created sound
			result = system->playSound(soundEffectList[i].sound, 0, false, &soundEffectChannel);
			if (result != FMOD_OK)
			{
				std::cout << "ERROR: Found, but was unable to play sound: " << soundEffectList[i].name << std::endl;
				return false;
			}

			soundEffectChannel->setReverbProperties(0, 0);
			soundEffectChannel->setLowPassGain(1);

			return true;
		}
	}
	std::cout << "ERROR: Couldn't find sound " << soundName << ". It was not found in the sound effect array." << std::endl;
	return false;
}

std::vector<Sound> cSoundPanel::GetMusicList() {
	return musicList;
}

Sound cSoundPanel::GetCurrentMusic()
{
	if (currentlyPlayingMusic == nullptr)
	{
		Sound nullSound = Sound();
		return nullSound;
	}

	return *currentlyPlayingMusic;
}

unsigned int cSoundPanel::GetCurrentMusicLength()
{
	if (currentlyPlayingMusic != nullptr)
	{
		unsigned int musicLength;
		currentlyPlayingMusic->sound->getLength(&musicLength, FMOD_TIMEUNIT_MS);

		return musicLength;
	}
	else
	{
		return 0;
	}
}

bool cSoundPanel::IsMusicPaused()
{
	return musicIsPaused;
}

void cSoundPanel::LoadMusic()
{
	//Read the text file one line at a time and save the line into my vector
	std::ifstream inputFile("assets/audio/musiclist.txt");

	if (inputFile.is_open())
	{

		std::string musicName;
		while (std::getline(inputFile, musicName) && musicName != "")
		{
			std::string musicFile = std::string("assets/audio/music/" + musicName);
			//Creates a stream 
			FMOD::Sound* music;
			result = system->createStream(musicFile.c_str(), FMOD_LOOP_NORMAL, 0, &music);
			if (result != FMOD_OK)
			{
				std::cout << "unable to create music: " << musicName;
			}

			Sound tempMusic;
			tempMusic.sound = music;
			tempMusic.name = musicName;

			//store all the music in memory
			musicList.push_back(tempMusic);
		}
	}
	else
	{
		std::cout << "ERROR: Unable to open music file" << std::endl;
	}
}

void cSoundPanel::LoadSounds()
{
	//Read the text file one line at a time and save the line into my vector
	std::ifstream inputFile("assets/audio/soundeffectslist.txt");

	if (inputFile.is_open())
	{

		std::string soundName;
		while (std::getline(inputFile, soundName) && soundName != "")
		{
			std::string soundFile = std::string("assets/audio/soundeffects/" + soundName);
			//Creates a sample 
			FMOD::Sound* soundEffect;
			result = system->createSound(soundFile.c_str(), FMOD_CREATESAMPLE, 0, &soundEffect);
			if (result != FMOD_OK)
			{
				std::cout << "unable to create sound effect: " << soundName;
			}

			Sound tempMusic;
			tempMusic.sound = soundEffect;
			tempMusic.name = soundName;

			//store all the samples in memory
			soundEffectList.push_back(tempMusic);
		}
	}
	else
	{
		std::cout << "ERROR: Unable to open sound file" << std::endl;
	}
}

void cSoundPanel::LoadBg()
{

	//Read the text file one line at a time and save the line into my vector
	std::ifstream inputFile("assets/audio/bglist.txt");

	if (inputFile.is_open())
	{

		std::string bgName;
		std::getline(inputFile, bgName);

		std::string soundFile = std::string("assets/audio/bg/" + bgName);

		//Creates a sample 
		FMOD::Sound* backGroundStream;
		result = system->createStream(soundFile.c_str(), FMOD_LOOP_NORMAL, 0, &backGroundStream);
		if (result != FMOD_OK)
		{
			std::cout << "unable to create background sound: " << bgName;
		}

		Sound* tempMusic = new Sound();
		tempMusic->sound = backGroundStream;
		tempMusic->name = bgName;

		this->bgSound = tempMusic;

		result = system->playSound(backGroundStream, 0, false, &backgroundChannel);
		if (result != FMOD_OK)
		{
			std::cout << "ERROR: Found, but was unable to play background sounds: " << bgName << std::endl;
		}

		backgroundChannel->setReverbProperties(0, 0);
		backgroundChannel->setLowPassGain(1);
		backgroundChannel->setVolume(1.1f);
	}
	else
	{
		std::cout << "ERROR: Unable to open background file" << std::endl;
	}
}
