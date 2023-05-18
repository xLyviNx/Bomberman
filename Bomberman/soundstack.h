#pragma once
#include <allegro5/allegro_audio.h>
#define ISON false
struct SampleStackElement
{
	float timePassed;
	ALLEGRO_SAMPLE_INSTANCE* myInstance;
	struct SampleStackElement* next;
};



struct SampleStackElement* SampleStack_Push(struct SampleStackElement*, ALLEGRO_SAMPLE_INSTANCE*);
void SampleStack_Loop(struct SampleStackElement**, float);
struct SampleStackElement* SampleStack_Pop(struct SampleStackElement*);
