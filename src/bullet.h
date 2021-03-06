#ifndef BULLET_H
#define BULLET_H

#include "sprite.h"

/* Forward declaration so we can have a pointer to a bullet_t in the struct */
typedef struct bullet_t bullet_t;
struct bullet_t {
    int xpos;         /* This bullets xpos */
    int ypos;         /* This bullets ypos */
    int damage;       /* How much damage this bullet does */
    sprite_t *sprite; /* A sprite for this bullet_t */

    /* A function pointer to a movement algorithm for each bullet_t */
    void (*move_bullet)(bullet_t *b);
};

/**
 * Update this bullets x and y position by 1 tick
 */
void bullet_update_position(bullet_t *b);

/**
 * Frees the memory used by an array of bullets
 */
void bullet_free(bullet_t *bullet);

/**
 * Functions for creating various typtes of bullets
 *
 * @param start_x The x position where these bullets are being 'fired' from
 * @param start_y The y position where these bullets are being 'fired' from
 * @param sprite The sprite that these bullets will use for drawning. Note that
 *               the sprite can be used for multiple bullets to save memory,
 *               so each bulle tdoes not own the sprite passed to it, and thus
 *               that sprite must be freed elseware
 */
/*
 * These are your basic bullets, which travel in a straight line one at a time.
 */
bullet_t* bullet_create_player_basic(int start_x, int start_y, sprite_t *sprite);
bullet_t* bullet_create_alien_basic(int start_x, int start_y, sprite_t *sprite);

/* Returns the width and height of a given bullet */
int bullet_get_height(bullet_t *b);
int bullet_get_width(bullet_t *b);

#endif
