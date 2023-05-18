#include "soundstack.h"

struct SampleStackElement* SampleStack_Push(struct SampleStackElement* first, ALLEGRO_SAMPLE_INSTANCE* instance)
{
	if (ISON) {
		struct SampleStackElement* new = (struct SampleStackElement*)malloc(sizeof(struct SampleStackElement));
		if (new)
		{
			new->myInstance = instance;
			new->timePassed = 0.0;
			new->next = NULL;
		}
		if (first)
		{
			puts("1");
			struct SampleStackElement* last = first;
			while (last != NULL && last->next != NULL)
			{
				puts("2");
				if (last->next)
				{
					last = last->next;
				}
			}
			puts("4");
			last->next = new;
		}
		else
		{
			puts("5");

			first = new;
		}
	}
	return first;
}
void SampleStack_Loop(struct SampleStackElement** first, float dT)
{
	if (ISON) {
		if (first && *first) {
			(*first)->timePassed += dT;
			//printf("Sound: %lf\n", (*first)->timePassed);
			if ((*first)->timePassed > 0.8)
			{
				printf("SHOULD POP SOUND\n");
				*first = SampleStack_Pop(*first);
			}
		}
	}
}
struct SampleStackElement* SampleStack_Pop(struct SampleStackElement* first)
{
	if (ISON) {
		if (first)
		{
			al_destroy_sample_instance(first->myInstance);
			struct SampleStackElement* next = first->next;
			free(first);
			printf("POPPING SOUND INSTANCE");
			return next;
		}
	}
}