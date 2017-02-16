/*
 * OpenAL example
 *
 * Copyright(C) Florian Fainelli <f.fainelli@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <cmath>
#include <chrono>
#include <string>

#include <AL/al.h>
#include <AL/alc.h>

#ifdef LIBAUDIO
#include <audio/wave.h>
#define BACKEND	"libaudio"
#else
#include <AL/alut.h>
#define BACKEND "alut"
#endif

static void list_audio_devices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	fprintf(stdout, "Devices list:\n");
	fprintf(stdout, "----------\n");
	while (device && *device != '\0' && next && *next != '\0') {
		fprintf(stdout, "%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	fprintf(stdout, "----------\n");
}

#define TEST_ERROR(_msg)		\
	error = alGetError();		\
	if (error != AL_NO_ERROR) {	\
		fprintf(stderr, _msg "\n");	\
		return -1;		\
	}

static inline ALenum to_al_format(short channels, short samples)
{
	bool stereo = (channels > 1);

	switch (samples) {
	case 16:
		if (stereo)
			return AL_FORMAT_STEREO16;
		else
			return AL_FORMAT_MONO16;
	case 8:
		if (stereo)
			return AL_FORMAT_STEREO8;
		else
			return AL_FORMAT_MONO8;
	default:
		return -1;
	}
}

int main(int argc, char **argv)
{
    bool show_debug = false;
    int duration_input = 19;
    const float radius = 1.5f;
    const std::string filename = "x1.wav";
    ALbyte * al_filename = (ALbyte*)filename.c_str();

    if (argc >= 3) {
        al_filename = (ALbyte*)argv[1];
        duration_input = strtol(argv[2], NULL, 10);

        if (argc >= 4 && strcmp(argv[3], "debug") == 0) {
            show_debug = true;
        }
    }

    printf("File: %s, Duration: %d\n\n", (char*)al_filename, duration_input);

	ALboolean enumeration;
	const ALCchar *devices;
	const ALCchar *defaultDeviceName = NULL;
	int ret;
#ifdef LIBAUDIO
	WaveInfo *wave;
#endif
	char *bufferData;
	ALCdevice *device;
	ALvoid *data;
	ALCcontext *context;
	ALsizei size, freq;
	ALenum format;
	ALuint buffer, source;
	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    ALfloat position[] = { 0.0f, 0.0f, radius };
	ALboolean loop = AL_FALSE;
	ALCenum error;
	ALint source_state;

	fprintf(stdout, "Using " BACKEND " as audio backend\n");

	enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if (enumeration == AL_FALSE)
		fprintf(stderr, "enumeration extension not available\n");

	list_audio_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	if (!defaultDeviceName)
		defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

	device = alcOpenDevice(defaultDeviceName);
	if (!device) {
		fprintf(stderr, "unable to open default device\n");
		return -1;
	}

	fprintf(stdout, "Device: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));

	alGetError();

	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context)) {
		fprintf(stderr, "failed to make default context\n");
		return -1;
	}
	TEST_ERROR("make default context");

	/* set orientation */
	alListener3f(AL_POSITION, 0, 0, 1.0f);
	TEST_ERROR("listener position");
    alListener3f(AL_VELOCITY, 0, 0, 0);
	TEST_ERROR("listener velocity");
	alListenerfv(AL_ORIENTATION, listenerOri);
	TEST_ERROR("listener orientation");

	alGenSources((ALuint)1, &source);
	TEST_ERROR("source generation");

	alSourcef(source, AL_PITCH, 1);
	TEST_ERROR("source pitch");
	alSourcef(source, AL_GAIN, 1);
	TEST_ERROR("source gain");
	alSourcefv(source, AL_POSITION, position);
	TEST_ERROR("source position");
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	TEST_ERROR("source velocity");
	alSourcei(source, AL_LOOPING, AL_TRUE);
	TEST_ERROR("source looping");
    alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);

	alGenBuffers(1, &buffer);
	TEST_ERROR("buffer generation");

#ifdef LIBAUDIO
	/* load data */
	wave = WaveOpenFileForReading(al_filename);
	if (!wave) {
		fprintf(stderr, "failed to read wave file\n");
		return -1;
	}

	ret = WaveSeekFile(0, wave);
	if (ret) {
		fprintf(stderr, "failed to seek wave file\n");
		return -1;
	}

	bufferData = malloc(wave->dataSize);
	if (!bufferData) {
		perror("malloc");
		return -1;
	}

	ret = WaveReadFile(bufferData, wave->dataSize, wave);
	if (ret != wave->dataSize) {
		fprintf(stderr, "short read: %d, want: %d\n", ret, wave->dataSize);
		return -1;
	}

	alBufferData(buffer, to_al_format(wave->channels, wave->bitsPerSample),
			bufferData, wave->dataSize, wave->sampleRate);
	TEST_ERROR("failed to load buffer data");
#else
#ifndef __APPLE__
	alutLoadWAVFile(al_filename, &format, &data, &size, &freq, &loop);
#else
	alutLoadWAVFile(al_filename, &format, &data, &size, &freq);
#endif
	TEST_ERROR("loading wav file");

	alBufferData(buffer, format, data, size, freq);
	TEST_ERROR("buffer copy");
#endif

	alSourcei(source, AL_BUFFER, buffer);
	TEST_ERROR("buffer binding");

	alSourcePlay(source);
	TEST_ERROR("source playing");

	alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	TEST_ERROR("source state get");

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    float elapsed_ms = 0;
    int duration_ms = duration_input * 1000;

    while (elapsed_ms < duration_ms) {
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
        if (source_state != AL_PLAYING) alSourcePlay(source);

        const float degree_pos = 360 * (elapsed_ms / duration_ms);
        const float rad_pos = 2 * 3.1415926 * (elapsed_ms / duration_ms);
        const float new_loc_z = radius * cos(rad_pos);
        const float new_loc_x = radius * sin(rad_pos);

        position[0] = new_loc_x;
        position[2] = new_loc_z;

        alSourcefv(source, AL_POSITION, position);

        if (show_debug) printf("X: %0.8f, Z: %0.8f, Radians: %0.8f, Degrees: %0.8f\n", position[0], position[2], rad_pos, degree_pos);

        elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    }

	/* exit context */
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);

	return 0;
}
