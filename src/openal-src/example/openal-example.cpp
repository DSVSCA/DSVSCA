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

#include <AL/alut.h>

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

    printf("File: %s, Duration: %d seconds\n\n", (char*)al_filename, duration_input);

	ALboolean enumeration;
	const ALCchar *devices;
	const ALCchar *defaultDeviceName = NULL;
	int ret;
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

	/* set orientation */
	alListener3f(AL_POSITION, 0, 0, 1.0f);
    alListener3f(AL_VELOCITY, 0, 0, 0);
	alListenerfv(AL_ORIENTATION, listenerOri);

	alGenSources((ALuint)1, &source);

	alSourcef(source, AL_PITCH, 1);
	alSourcef(source, AL_GAIN, 1);
	alSourcefv(source, AL_POSITION, position);
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	alSourcei(source, AL_LOOPING, AL_TRUE);
    alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);

	alGenBuffers(1, &buffer);

#ifndef __APPLE__
	alutLoadWAVFile(al_filename, &format, &data, &size, &freq, &loop);
#else
	alutLoadWAVFile(al_filename, &format, &data, &size, &freq);
#endif

	alBufferData(buffer, format, data, size, freq);

	alSourcei(source, AL_BUFFER, buffer);

	alSourcePlay(source);

	alGetSourcei(source, AL_SOURCE_STATE, &source_state);

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
