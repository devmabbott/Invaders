#include <assert.h>
#include <stdio.h>

#include "game.h"
#include "errors.h"
#include "linked_list.h"
#include "bullet.h"
#include "sprite.h"
#include "alien.h"

/* How many times per second we want to update the game state */
#define TICKS_PER_SECOND 60

/* Milliseconds between when each game state update should occur */
#define SKIP_TICKS 1000 / TICKS_PER_SECOND

/* The maximum number of frames that can be skipped */
#define  MAX_FRAMESKIP 10

/* Global, so that the signal handlers can interact with it */
static game_t *g;

static void game_sdl_init(game_t *g) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        app_error("SDL failed to init");

    g->surface = SDL_SetVideoMode(GAME_WIDTH, GAME_HEIGHT,
        32, SDL_HWSURFACE | SDL_DOUBLEBUF);

    if(!g->surface)
        app_error("SDL surface didn't init properly");
}

static void game_stop(game_t *g) {
    g->running = 0;
}

static void game_handle_input(game_t *g) {
    while(SDL_PollEvent(&g->event)) {
        if(g->event.type == SDL_QUIT)
            game_stop(g);

        else if(g->event.type == SDL_KEYDOWN) {
            if(g->event.key.keysym.sym == SDLK_UP)
                g->state->player_move_up = 1;
            if(g->event.key.keysym.sym == SDLK_DOWN)
                g->state->player_move_down = 1;
            if(g->event.key.keysym.sym == SDLK_LEFT)
                g->state->player_move_left = 1;
            if(g->event.key.keysym.sym == SDLK_RIGHT)
                g->state->player_move_right = 1;
            if(g->event.key.keysym.sym == SDLK_SPACE)
                g->state->player_fire = 1;
        }

        else if(g->event.type == SDL_KEYUP) {
            if(g->event.key.keysym.sym == SDLK_UP)
                g->state->player_move_up = 0;
            if(g->event.key.keysym.sym == SDLK_DOWN)
                g->state->player_move_down = 0;
            if(g->event.key.keysym.sym == SDLK_LEFT)
                g->state->player_move_left = 0;
            if(g->event.key.keysym.sym == SDLK_RIGHT)
                g->state->player_move_right = 0;
            if(g->event.key.keysym.sym == SDLK_SPACE)
                g->state->player_fire = 0;
        }
    }
}

static void game_draw_player(game_t *g) {
    SDL_Rect DestR;

    DestR.x = g->state->player_ship->xpos;
    DestR.y = g->state->player_ship->ypos;

    SDL_BlitSurface(g->state->player_ship->sprite->pic, NULL, g->surface, &DestR);
}

static void game_draw_bullets(game_t *g) {
    SDL_Rect DestR;
    ll_node_t *node;
    bullet_t *bullet;

    node = ll_get_first_node(g->state->player_bullets);
    while(node) {
        bullet = (bullet_t *)ll_get_item(node);

        DestR.x = bullet->xpos;
        DestR.y = bullet->ypos;
        SDL_BlitSurface(sprite_get_basic_bullet()->pic, NULL, g->surface, &DestR);

        node = ll_next_node(g->state->player_bullets, node);
    }

    node = ll_get_first_node(g->state->alien_bullets);
    while(node) {
        bullet = (bullet_t *)ll_get_item(node);

        DestR.x = bullet->xpos;
        DestR.y = bullet->ypos;
        SDL_BlitSurface(sprite_get_basic_bullet()->pic, NULL, g->surface, &DestR);

        node = ll_next_node(g->state->alien_bullets, node);
    }
}

static void game_draw_aliens(game_t *g) {
    SDL_Rect DestR;
    ll_node_t *node;
    alien_t *alien;

    node = ll_get_first_node(g->state->level->aliens);
    while(node) {
        alien = (alien_t *)ll_get_item(node);

        DestR.x = alien->ship->xpos;
        DestR.y = alien->ship->ypos;
        SDL_BlitSurface(alien->ship->sprite->pic, NULL, g->surface, &DestR);

        node = ll_next_node(g->state->level->aliens, node);
    }
}

static void game_update_display(game_t *g, float interpolation) {
    /* Prepare the background for redrawing */
    SDL_FillRect(g->surface, NULL, 0x000000);

    /* Add the player to the new surface */
    game_draw_player(g);

    /* Add the bullets to tne new surface */
    game_draw_bullets(g);

    /* Draw the aliens to the new survace */
    game_draw_aliens(g);

    /* Draw the new surface */
    SDL_Flip(g->surface);
}

static void game_update(game_t *g) {
    game_handle_input(g);
    game_state_update(g->state);
}

game_t* game_init() {
    game_t *g;

    g = malloc(sizeof(game_t));
    if(!g)
        system_error("malloc error on game init");

    g->state = game_state_init();
    game_sdl_init(g);

    return g;
}

void game_free(game_t *g) {
    assert(g);

    game_state_free(g->state);
    SDL_FreeSurface(g->surface);
    SDL_Quit();
    free(g);
}

/* This will update teh display as fast as possible, but only
 * update the underlying game state on a fixed time-stamp system */
void game_run(game_t *g) {
    /* The next time that the game state should be updated */
    unsigned int next_game_tick;

    /* The number of frames that have been skipped to try and keep the game
     * state updating 60 times a second */
    unsigned int frame_skip;

    assert(g);

    /* Game loop */
    g->running = 1;
    next_game_tick = SDL_GetTicks();
    while(g->running) {
        frame_skip = 0;

        while(SDL_GetTicks() > next_game_tick && frame_skip <= MAX_FRAMESKIP) {
            game_update(g);
            next_game_tick += SKIP_TICKS;
            frame_skip++;
        }

        /* The current time inbetween game state updates used to guess where
         * objects should be drawn (interpolation) */
        game_update_display(g, (float) (SDL_GetTicks() + SKIP_TICKS
                            - next_game_tick) / (float) (SKIP_TICKS));
    }
}

void sigint_handler(int sig) {
    game_stop(g);
}

handler_t* sigaction_wrapper(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if(sigaction(signum, &action, &old_action) < 0)
        system_error("Signal error");
    return (old_action.sa_handler);
}


int main(int argc, char *argv[]) {
    /* Signal handlers */
    sigaction_wrapper(SIGINT, sigint_handler);

    /* Start the game */
    g = game_init();
    game_run(g);
    game_free(g);

    return 0;
}

