#include "soundstack.h"
/**
 * @brief Dodanie dzwieku do stosu.
 *
 * @param first Stos (wskaznik)
 * @param instance Allegro Sample Instance (wskaznik)
 */
struct SampleStackElement* SampleStack_Push(struct SampleStackElement* first, ALLEGRO_SAMPLE_INSTANCE* instance)
{
	struct SampleStackElement* new = (struct SampleStackElement*)malloc(sizeof(struct SampleStackElement));
	if (new)
	{
		new->myInstance = instance;
		new->timePassed = 0.0;
		new->next = NULL;
	}
	if (first)
	{
		struct SampleStackElement* last = first;
		while (last != NULL && last->next != NULL)
		{
			if (last->next)
			{
				last = last->next;
			}
		}
		last->next = new;
	}
	else
	{
		first = new;
	}

	return first;
}
/**
 * @brief Wykrywanie kiedy wyrzucic element ze stosu (swego rodzaju PEEK z odczytaniem czasu)
 *
 * @param first Stos (podwojny wskaznik)
 * @param dT deltaTime (czas od ostatniej klatki)
 */
void SampleStack_Loop(struct SampleStackElement** first, float dT)
{
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
/**
 * @brief Zdjecie elementu ze stosu
 *
 * @param first Stos (wskaznik)
 * @return Wskaznik na stos
 */
struct SampleStackElement* SampleStack_Pop(struct SampleStackElement* first)
{
	if (first)
	{
		al_destroy_sample_instance(first->myInstance);
		struct SampleStackElement* next = first->next;
		free(first);
		printf("POPPING SOUND INSTANCE");
		return next;
	}

}