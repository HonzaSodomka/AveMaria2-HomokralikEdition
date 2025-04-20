#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <string>

class AudioManager {
private:
    ALCdevice* device;
    ALCcontext* context;

    ALuint backgroundMusicSource;
    ALuint backgroundMusicBuffer;

    ALuint footstepSource;
    ALuint footstepBuffer;

    bool footstepPlaying;

    static AudioManager* instance;

    AudioManager();

public:
    ~AudioManager();

    static AudioManager* getInstance();

    bool initialize();
    bool loadWAVSound(const std::string& filename, ALuint& buffer);

    void playBackgroundMusic(const std::string& filename);
    void startFootsteps(const std::string& filename);
    void stopFootsteps();
};