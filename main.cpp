//#include <string>
//#include <iostream>
//#include "pa_win_ds.h"
//#include "pa_win_wasapi.h"
//#include "pa_win_waveformat.h"
//
//using namespace std;
//
//int main()
//{
//	Pa_Initialize();
//
//
//
//	//Pa_Terminate();
//
//
//
//	int x;
//	cin >> x;
//	return 0;
//}

/** @file paex_sine.c
@ingroup examples_src
@brief Play a sine wave for several seconds.
@author Ross Bencina <rossb@audiomulch.com>
@author Phil Burk <philburk@softsynth.com>
*/
/*
* $Id$
*
* This program uses the PortAudio Portable Audio Library.
* For more information see: http://www.portaudio.com/
* Copyright (c) 1999-2000 Ross Bencina and Phil Burk
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
* The text above constitutes the entire PortAudio license; however,
* the PortAudio community also makes the following non-binding requests:
*
* Any person wishing to distribute modifications to the Software is
* requested to send the modifications to the original developer so that
* they can be incorporated into the canonical version. It is also
* requested that these non-binding requests be included along with the
* license above.
*/
#include <stdio.h>
#include <math.h>
#include "portaudio.h"
#include <SFML\Graphics.hpp>

using namespace std;
using namespace sf;

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (256)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (200)//(1024)//(200)
typedef struct
{
	float sine[TABLE_SIZE];
	int left_phase;
	int right_phase;
	char message[20];
}
paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/

#define MIDDLE_C (261.626)
#define A4 (440.0)

static float xxx = .01f;
static int xx = 1;
static float hmm = 1.0f;
static unsigned long n = 0;
static float currFreq = A4;

static int patestCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	paTestData *data = (paTestData*)userData;
	float *out = (float*)outputBuffer;
	unsigned long i;

	(void)timeInfo; /* Prevent unused variable warnings. */
	(void)statusFlags;
	(void)inputBuffer;

	float leftTest, rightTest;
	float xTest;

	for (i = 0; i<framesPerBuffer; i++)
	{

		//data.sine[i] = (float)sin(((double)i / (double)TABLE_SIZE) * M_PI * 2.);
		//float v = sin(261.626 * 2 * M_PI * ((float)n / (float)TABLE_SIZE));
		float v = sin(currFreq * 2 * M_PI * ((float)n / (float)SAMPLE_RATE));
		*out++ = v;
		*out++ = v;

		leftTest = data->sine[data->left_phase];
		rightTest = data->sine[data->right_phase];

		leftTest *= hmm;
		rightTest *= hmm;

		//*out++ = leftTest;
		//*out++ = rightTest;

		data->left_phase += xx;
		if (data->left_phase >= TABLE_SIZE) data->left_phase -= TABLE_SIZE;
		data->right_phase += xx;
		if (data->right_phase >= TABLE_SIZE) data->right_phase -= TABLE_SIZE;

		++n;
	}

	return paContinue;
}

/*
* This routine is called by portaudio when playback is done.
*/
static void StreamFinished(void* userData)
{
	paTestData *data = (paTestData *)userData;
	printf("Stream Completed: %s\n", data->message);
}

void end(PaError &err)
{
	Pa_Terminate();
	fprintf(stderr, "An error occurred while using the portaudio stream\n");
	fprintf(stderr, "Error number: %d\n", err);
	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
}

/*******************************************************************/
int main(void)
{
	PaStreamParameters outputParameters;
	PaStream *stream;
	PaError err;
	paTestData data;
	int i;

	printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

	/* initialise sinusoidal wavetable */
	for (i = 0; i<TABLE_SIZE; i++)
	{
		data.sine[i] = (float)sin(((double)i / (double)TABLE_SIZE) * M_PI * 2.);
	}
	data.left_phase = data.right_phase = 0;

	err = Pa_Initialize();
	if (err != paNoError)
	{
		end(err);
		return err;
	}

	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device == paNoDevice) {
		fprintf(stderr, "Error: No default output device.\n");
		{
			end(err);
			return err;
		}
	}
	outputParameters.channelCount = 2;       /* stereo output */
	outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		patestCallback,
		&data);
	if (err != paNoError)
	{
		end(err);
		return err;
	}

	sprintf(data.message, "No Message");
	err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
	if (err != paNoError)
	{
		end(err);
		return err;
	}

	err = Pa_StartStream(stream);
	if (err != paNoError)
	{
		end(err);
		return err;
	}

	printf("Play for %d seconds.\n", NUM_SECONDS);
	//Pa_Sleep(NUM_SECONDS * 1000);

	sf::RenderWindow window(sf::VideoMode(600, 600), "SFML works!");
	sf::CircleShape shape(100.f);

	sf::Font font;
	font.loadFromFile("Kinetic_Font_01.ttf"); //each panel loads the font separately, this is bad
	shape.setFillColor(sf::Color::Green);

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(50);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		Vector2i pos = sf::Mouse::getPosition();
		float test = max( 0, pos.x);
		test = min(test, 1920.f);

		float minValue = .1;
		float total = test * .00001 + minValue;
		xxx = total;

		

		float test2 = pos.y;
		float total2 = pos.y * .001;

		int testxx = round(pos.x / 25);

		currFreq = A4 + pos.x;

		testxx = max(0, testxx);

		xx = testxx;

		text.setString("currFreq: " + to_string(currFreq));

		/*hmm = pos.y / 1080.f;
		if (hmm < 1.f / 1080.f)
			hmm = 1.f / 1080.f;*/
		hmm = .36;//4.f;

		window.clear();
		window.draw(shape);
		window.draw(text);
		window.display();
	}

	err = Pa_StopStream(stream);

	
	if (err != paNoError)
	{
		end(err);
		return err;
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError)
	{
		end(err);
		return err;
	}

	Pa_Terminate();
	printf("Test finished.\n");

	return err;
}
