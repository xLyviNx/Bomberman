#pragma once
#include <allegro5/allegro_audio.h>
/**
 * @brief Struktura Dzwieku w stosie
 */
struct SampleStackElement
{
	float timePassed; /**< Miniony czas */
	ALLEGRO_SAMPLE_INSTANCE* myInstance; /**< Sample Instance*/
	struct SampleStackElement* next; /**< nastepny element*/
};



struct SampleStackElement* SampleStack_Push(struct SampleStackElement*, ALLEGRO_SAMPLE_INSTANCE*);
void SampleStack_Loop(struct SampleStackElement**, float);
struct SampleStackElement* SampleStack_Pop(struct SampleStackElement*);
