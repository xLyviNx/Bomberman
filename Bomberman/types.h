#pragma once
#include "algif5-master/src/algif.h"

/**
 * @brief Struktura posiadajaca dwie wspolrzedne X i Y.
 */
struct Vector2
{
    double x;  /**< Wspolrzedna x. */
    double y; /**< Wspolrzedna y. */
};
/**
 * @brief Struktura posiadajaca dane o pozycji, pozycji w siatce oraz skali.
 */
struct Transform
{
    struct Vector2 position; /**< Pozycja. */
    struct Vector2 gridPosition;/**< Pozycja w siatce. */
    struct Vector2 scale; /**< Skala. */
};
/**
 * @brief Struktura gracza
 */
struct Character
{
    struct Transform Transform; /**< Informacje o pozycji oraz skali. */
    float Speed; /**< Predkosc gracza. */
    ALGIF_ANIMATION* IdleAnim; /**< Animacja. */
    ALGIF_ANIMATION* RightWalkAnim; /**< Animacja. */
    ALGIF_ANIMATION* LeftWalkAnim; /**< Animacja. */
    ALGIF_ANIMATION* UpWalkAnim; /**< Animacja. */
    ALGIF_ANIMATION* DownWalkAnim; /**< Animacja. */
    unsigned short int bombRange; /**< Zasieg bomb. */
    unsigned short int maxBombs; /**< Maksymalna ilosc bomb. */
    bool enabled; /**< Zmienna mowiaca o tym, czy gracz jest aktywny. */
    unsigned short walking; /**< Kierunek poruszania sie gracza. */
};
bool is_on_block(struct dstr_block*, float, float, bool*, struct Character*, struct BombList*, bool);
