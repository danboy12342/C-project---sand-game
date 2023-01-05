#include <stdbool.h>
#include <stdlib.h>

#include "wasm4.h"
#include "menu.h"
#include "play.h"

#define CANVAS_WIDTH 160
#define CANVAS_HEIGHT 120

#define PARTICLE_MATERIAL_BITS          0b1111000000000000
#define PARTICLE_MATERIAL_BIT_OFFSET                    12
#define PARTICLE_COLOR_BITS             0b0000110000000000
#define PARTICLE_COLOR_BIT_OFFSET                       10
#define PARTICLE_AGE_BITS               0b0000001111000000
#define PARTICLE_AGE_BIT_OFFSET                          6
#define PARTICLE_UPDATED_BITS           0b0000000000000001
#define PARTICLE_UPDATED_BIT_OFFSET                      0 

#define MATERIAL_AIR_ID 0
#define MATERIAL_SAND_ID 1
#define MATERIAL_FIRE_ID 3
#define MATERIAL_LAVA_ID 4
#define MATERIAL_GLASS_ID 5
#define MATERIAL_TORCH_ID 6
#define MATERIAL_SPOUT_ID 7
#define MATERIAL_ERASE_ID 13

typedef struct v2 {
  unsigned char x;
  unsigned char y;
} v2;

bool paused = false;
const unsigned char cursor_colors[] = {1,2,3,3,0,3,2,0,0,0,0,0,0,0,0,0};

typedef uint16_t particle_t;

static particle_t particles[CANVAS_HEIGHT][CANVAS_WIDTH]; // y,x

particle_t* get_particle(uint8_t x, uint8_t y) {
  return &particles[y][x];
}

void move_particle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  particle_t *particle_one = get_particle(x1, y1);
  particle_t *particle_two = get_particle(x2, y2);
  particle_t particle_buffer = *particle_one;

  *particle_one = *particle_two;
  *particle_two = particle_buffer;
}

void set_particle_state(particle_t *particle, uint8_t value, int bits, int offset) {
  *particle &= ~(bits); // clear previous value
  *particle |= (value << offset); // set value
}

void set_particle_color(particle_t *particle, uint8_t color) {
  set_particle_state(particle, color, PARTICLE_COLOR_BITS, PARTICLE_COLOR_BIT_OFFSET);
}

void set_particle_updated(particle_t *particle, bool value) {
  set_particle_state(particle, value, PARTICLE_UPDATED_BITS, PARTICLE_UPDATED_BIT_OFFSET);
}

void set_particle_material_id(particle_t *particle, uint8_t material_id) {
  set_particle_state(particle, material_id, PARTICLE_MATERIAL_BITS, PARTICLE_MATERIAL_BIT_OFFSET);
}

void set_particle_age(particle_t *particle, uint8_t age) {
  set_particle_state(particle, age, PARTICLE_AGE_BITS, PARTICLE_AGE_BIT_OFFSET);
}

uint8_t get_particle_state(particle_t *particle, int bits, int offset) {
  return (uint8_t)((*particle & bits) >> offset);
}

uint8_t get_particle_color(particle_t *particle) {
  return get_particle_state(particle, PARTICLE_COLOR_BITS, PARTICLE_COLOR_BIT_OFFSET);
}

bool get_particle_updated(particle_t *particle) {
  return get_particle_state(particle, PARTICLE_UPDATED_BITS, PARTICLE_UPDATED_BIT_OFFSET);
}

uint8_t get_particle_material_id(particle_t *particle) {
  return get_particle_state(particle, PARTICLE_MATERIAL_BITS, PARTICLE_MATERIAL_BIT_OFFSET);
}

uint8_t get_particle_age(particle_t *particle) {
  return get_particle_state(particle, PARTICLE_AGE_BITS, PARTICLE_AGE_BIT_OFFSET);
}

particle_t* update_spout(uint8_t x, uint8_t y) {
  particle_t *particle = get_particle(x, y);
  set_particle_color(particle, 2);
  if (paused == false) {
    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_AIR_ID) {
      if ((rand() % 100) < 20) {
        particle_t *particle = get_particle(x, y + 1);
        set_particle_color(particle, 2);
      }
    } 
  }


  return particle;
}

particle_t* update_torch(uint8_t x, uint8_t y) {
  particle_t *particle = get_particle(x, y);

  if (get_particle_color(particle) == 0 || paused == false){
    if ((rand() % 100) < 30) {
      set_particle_color(particle, 1);
    } else {
      set_particle_color(particle, 3);
    }

    if (get_particle_material_id(get_particle(x, y - 1)) == MATERIAL_AIR_ID) {
      if ((rand() % 100) < 30) {
        particle_t *particle = get_particle(x, y - 1);
        set_particle_material_id(particle, MATERIAL_FIRE_ID);
        set_particle_color(particle, 3);
        set_particle_age(particle, 0);
      }
    } 

    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_AIR_ID) {
      if ((rand() % 100) < 30) {
        particle_t *particle = get_particle(x, y + 1);
        set_particle_material_id(particle, MATERIAL_FIRE_ID);
        set_particle_color(particle, 3);
        set_particle_age(particle, 0);
      }
    } 

    if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_AIR_ID) {
      if ((rand() % 100) < 30) {
        particle_t *particle = get_particle(x - 1, y);
        set_particle_material_id(particle, MATERIAL_FIRE_ID);
        set_particle_color(particle, 3);
        set_particle_age(particle, 0);
      }
    } 

    if (get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_AIR_ID) {
      if ((rand() % 100) < 30) {
        particle_t *particle = get_particle(x + 1, y);
        set_particle_material_id(particle, MATERIAL_FIRE_ID);
        set_particle_color(particle, 3);
        set_particle_age(particle, 0);
      }
    }
  }

  return particle;
}


particle_t* update_lava(uint8_t x, uint8_t y) {
  particle_t *particle = get_particle(x, y);
  v2 new_position = {.x = x, .y = y};

  if (get_particle_color(particle) == 0 || paused == false){
    if ((rand() % 100) < 30) {
      set_particle_color(particle, 1);
    } else {
      set_particle_color(particle, 3);
    }

    if (get_particle_age(particle) == 15) {
      if ((rand() % 100) < 5) {
        set_particle_material_id(particle, MATERIAL_AIR_ID);
        set_particle_color(particle, 0);
        set_particle_age(particle, 0);
        return particle;
      }
    }
  }


  if (get_particle_updated(particle) == false && y < CANVAS_HEIGHT && paused == false) {
    // Ignite the sand
    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_SAND_ID) {
      if ((rand() % 100) < 70) {
        particle_t *sand_particle = get_particle(x, y + 1);
        set_particle_material_id(sand_particle, MATERIAL_FIRE_ID);
        set_particle_color(sand_particle, 3);
      }
    }
    if (get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_SAND_ID) {
      if ((rand() % 100) < 20) {
        particle_t *sand_particle = get_particle(x + 1, y);
        set_particle_material_id(sand_particle, MATERIAL_FIRE_ID);
        set_particle_color(sand_particle, 3);
      }
    }
    if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_SAND_ID) {
      if ((rand() % 100) < 20) {
        particle_t *sand_particle = get_particle(x - 1, y);
        set_particle_material_id(sand_particle, MATERIAL_FIRE_ID);
        set_particle_color(sand_particle, 3);
      }
    }
    
    // Melt glass into sand
    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_GLASS_ID) {
      if ((rand() % 100) < 3) {
        particle_t *glass_particle = get_particle(x, y + 1);
        set_particle_material_id(glass_particle, MATERIAL_SAND_ID);
        set_particle_color(glass_particle, 1);
      }
    }
    if (get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_GLASS_ID) {
      if ((rand() % 100) < 2) {
        particle_t *glass_particle = get_particle(x + 1, y);
        set_particle_material_id(glass_particle, MATERIAL_SAND_ID);
        set_particle_color(glass_particle, 1);
      }
    }
    if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_GLASS_ID) {
      if ((rand() % 100) < 2) {
        particle_t *glass_particle = get_particle(x - 1, y);
        set_particle_material_id(glass_particle, MATERIAL_SAND_ID);
        set_particle_color(glass_particle, 1);
      }
    }

    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_AIR_ID && y < CANVAS_HEIGHT - 1) {
      if ((rand() % 100) > 50) {
        new_position.y = y + 1;
      }
      // If there is air on both sides, randomly choose a direction to move
    } else if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_AIR_ID && 
        get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_AIR_ID) {
      if ((rand() % 100) > 75) {
        if ((rand() % 100) > 50) {
          new_position.x = x - 1;
        }
      } else if ((rand() % 100) < 25) {
        if ((rand() % 100) > 50) {
          new_position.x = x + 1;
        }
      }
    } else if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_AIR_ID) {
      if ((rand() % 100) > 50) {
        new_position.x = x - 1;
      }
    } else if (get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_AIR_ID && x < CANVAS_WIDTH - 1) {
      if ((rand() % 100) > 50) {
        new_position.x = x + 1;
      }
    }
    move_particle(x, y, new_position.x, new_position.y);
  }

  particle_t *particle_at_new_position = get_particle(new_position.x, new_position.y);
  set_particle_age(particle_at_new_position, get_particle_age(particle_at_new_position) + 1);
  return particle_at_new_position;
}

particle_t* update_fire(uint8_t x, uint8_t y) {
  particle_t *particle = get_particle(x, y);
  v2 new_position = {.x = x, .y = y};
  set_particle_color(particle, 3);

  if (paused == false) {
    if (get_particle_age(particle) < 2) {
      set_particle_color(particle, 1);
    } else {
      if (get_particle_age(particle) < 10 && (rand() % 100) < 30) {
        set_particle_color(particle, 1);
      } else if ((rand() % 100) < 20) {
        set_particle_color(particle, 0);
      } else {
        set_particle_color(particle,3);
      }
    }

    if (get_particle_age(particle) == 15) {
      set_particle_material_id(particle, MATERIAL_AIR_ID);
      set_particle_color(particle, 0);
      set_particle_age(particle, 0);
      return particle;
    }
  }

  if (get_particle_updated(particle) == false && y < CANVAS_HEIGHT - 1 && paused == false) {
    if (get_particle_material_id(get_particle(x, y - 1)) == MATERIAL_SAND_ID) {
      particle_t *sand_particle = get_particle(x, y - 1);
      set_particle_material_id(sand_particle, MATERIAL_FIRE_ID);
      set_particle_color(sand_particle, 3);
    }

    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_SAND_ID) {
      if ((rand() % 100) < 40) {
        particle_t *sand_particle = get_particle(x, y + 1);
        set_particle_material_id(sand_particle, MATERIAL_FIRE_ID);
        set_particle_color(sand_particle, 3);
      }
    }

    if (get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_SAND_ID) {
      if ((rand() % 100) < 20) {
        particle_t *sand_particle = get_particle(x + 1, y);
        set_particle_material_id(sand_particle, MATERIAL_FIRE_ID);
        set_particle_color(sand_particle, 3);
      }
    }

    if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_SAND_ID) {
      if ((rand() % 100) < 20) {
        particle_t *sand_particle = get_particle(x - 1, y);
        set_particle_material_id(sand_particle, MATERIAL_FIRE_ID);
        set_particle_color(sand_particle, 3);
      }
    }

    if (get_particle_material_id(get_particle(x, y - 1)) == MATERIAL_AIR_ID) {
      new_position.y = y - 1;
    } else if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_AIR_ID && 
        get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_AIR_ID) {
      new_position.x = rand() % 100 < 50 ? x - 1 : x + 1;
    } else if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_AIR_ID) {
      new_position.x = x - 1;
    } else if (get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_AIR_ID) {
      new_position.x = x + 1;
    }
    move_particle(x, y, new_position.x, new_position.y);
  }

  particle_t *particle_at_new_position = get_particle(new_position.x, new_position.y);
  set_particle_age(particle_at_new_position, get_particle_age(particle_at_new_position) + 1);
  return particle_at_new_position;
}


  if (get_particle_updated(particle) == false && y < CANVAS_HEIGHT && paused == false) {
    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_AIR_ID &&
          ) {
      new_position.y = y + 1;
    } else if (get_particle_material_id(get_particle(x - 1, y + 1)) == MATERIAL_AIR_ID && 
        get_particle_material_id(get_particle(x + 1, y + 1)) == MATERIAL_AIR_ID &&
        y < CANVAS_HEIGHT - 1
        ) {
      // set_particle_color(particle, 1);
      if (((rand() % 100) > 50) && x > 0) {
        new_position.x = x - 1;
      } else if (x < CANVAS_WIDTH - 1) {
        new_position.x = x + 1;
      }
      new_position.y = y + 1;
    } else if (get_particle_material_id(get_particle(x - 1, y + 1)) == MATERIAL_AIR_ID &&
        x > 0 &&
        y < CANVAS_HEIGHT - 1
       ) {
      new_position.x = x - 1;
      new_position.y = y + 1;
    } else if (get_particle_material_id(get_particle(x + 1, y + 1)) == MATERIAL_AIR_ID &&
        x < CANVAS_WIDTH - 1 &&
        y < CANVAS_HEIGHT - 1
        ) {
      new_position.x = x + 1;
      new_position.y = y + 1;
    } else if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_AIR_ID && 
        get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_AIR_ID) {
      if (((rand() % 100) > 50) && x > 0) {
        new_position.x = x - 1;
      } else if (x < CANVAS_WIDTH - 1) {
        new_position.x = x + 1;
      }
    } else if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_AIR_ID &&
        x > 0
        ) {
      new_position.x = x - 1;
    } else if (get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_AIR_ID && 
        x < CANVAS_WIDTH - 1
        ) {
      new_position.x = x + 1;
    }
    move_particle(x, y, new_position.x, new_position.y);

    // Extinguish fire
    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_FIRE_ID) {
      if ((rand() % 100) < 80) {
        particle_t *fire_particle = get_particle(x, y + 1);
        set_particle_material_id(particle, MATERIAL_AIR_ID);
      }
    } else if (get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_FIRE_ID) {
      if ((rand() % 100) < 80) {
        particle_t *fire_particle = get_particle(x - 1, y);
        set_particle_material_id(particle, MATERIAL_AIR_ID);
      }
    } else if (get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_FIRE_ID) {
      if ((rand() % 100) < 80) {
        particle_t *fire_particle = get_particle(x + 1, y);
        set_particle_material_id(particle, MATERIAL_AIR_ID);
      }
    }
  }

  particle_t *particle_at_new_position = get_particle(new_position.x, new_position.y);
  return particle_at_new_position;
}

particle_t* update_sand(uint8_t x, uint8_t y) {
  particle_t *particle = get_particle(x, y);
  v2 new_position = {.x = x, .y = y};
  set_particle_color(particle, 1);

  if (get_particle_updated(particle) == false && y < CANVAS_HEIGHT - 1 && paused == false) {
    if (get_particle_material_id(get_particle(x, y + 1)) == MATERIAL_AIR_ID) {
      new_position.y = y + 1;
    } else if (get_particle_material_id(get_particle(x - 1, y + 1)) == MATERIAL_AIR_ID &&
        get_particle_material_id(get_particle(x - 1, y)) == MATERIAL_AIR_ID) {
      new_position.x = x - 1;
      new_position.y = y + 1;
    } else if (get_particle_material_id(get_particle(x + 1, y + 1)) == MATERIAL_AIR_ID && 
        get_particle_material_id(get_particle(x + 1, y)) == MATERIAL_AIR_ID) {
      new_position.x = x + 1;
      new_position.y = y + 1;
    }
    move_particle(x, y, new_position.x, new_position.y);
  }

  particle_t *particle_at_new_position = get_particle(new_position.x, new_position.y);
  return particle_at_new_position;
}

void pixel(int x, int y) {
    // The byte index into the framebuffer that contains (x, y)
    int idx = (y*160 + x) >> 2;

    // Calculate the bits within the byte that corresponds to our position
    int shift = (x & 0b11) << 1;
    int mask = 0b11 << shift;
    // Use the first DRAW_COLOR as the pixel color
    int palette_color = *DRAW_COLORS & 0b1111;
    if (palette_color == 0) {
        // Transparent
        return;
    }
    int color = (palette_color - 1) & 0b11;  

    // Write to the framebuffer
    FRAMEBUFFER[idx] = (color << shift) | (FRAMEBUFFER[idx] & ~mask);
}

void clear_particles() {
  for (int x = 0; x < CANVAS_WIDTH; x++) {
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
      *get_particle(x, y) = 0;
    }
  }
}

void start(void) {
  clear_particles();
  
  // From https://lospec.com/palette-list/coldfire-gb
  PALETTE[0] = 0x46425e;
  PALETTE[1] = 0xf6c6a8;
  PALETTE[2] = 0x5b768d;
  PALETTE[3] = 0xd17c7c;

  // Hide gamepad overlay
  *SYSTEM_FLAGS = 0x2;
}

uint8_t previous_gamepad;
uint8_t previous_mouse;
void update(void) {
  uint8_t gamepad = *GAMEPAD1;
  uint8_t mouse = *MOUSE_BUTTONS;
  uint8_t gamepad_this_frame = gamepad & (gamepad ^ previous_gamepad);
  uint8_t mouse_this_frame = mouse & (mouse ^ previous_mouse);
  previous_gamepad = gamepad;
  previous_mouse = mouse;

  static int pen_size = 1;
  static int primary_material_id = 1;
  static int secondary_material_id = 2;

  for (int x = CANVAS_WIDTH; x >= 0; x--) {
    for (int y = CANVAS_HEIGHT - 1; y >= 0; y--) {
      set_particle_updated(get_particle(x, y), false);
    }
  }

  for (int x = 0; x <= CANVAS_WIDTH; x++) {
    for (int y = CANVAS_HEIGHT - 1; y >= 0; y--) {
      particle_t *particle = get_particle(x, y);

      switch (get_particle_material_id(particle)) {
        case MATERIAL_AIR_ID:
          set_particle_color(particle, 0);
          break;
        case MATERIAL_SAND_ID:
          particle = update_sand(x, y);
          break;
        case MATERIAL_FIRE_ID:
          particle = update_fire(x, y);
          break;
        case MATERIAL_LAVA_ID:
          particle = update_lava(x, y);
          break;
        case MATERIAL_GLASS_ID:
          set_particle_color(particle, 0);
          break;
        case MATERIAL_TORCH_ID:
          particle = update_torch(x, y);
          break;
        case MATERIAL_SPOUT_ID:
          particle = update_spout(x, y);
          break;
        default:
          break;
      }

      set_particle_updated(particle, true);
      *DRAW_COLORS = get_particle_color(particle) + 1;
      pixel(x, y);
    }
  }
  
  // Attempt to draw material if within canvas
  if (*MOUSE_X <= CANVAS_WIDTH && 
      *MOUSE_X >= 0 && 
      *MOUSE_Y <= CANVAS_HEIGHT && 
      *MOUSE_Y >= 0 && 
      *MOUSE_BUTTONS
  ){
    int *selected_id = NULL;

    if (*MOUSE_BUTTONS & MOUSE_LEFT) {
      selected_id = &primary_material_id;
    } else if (*MOUSE_BUTTONS & MOUSE_RIGHT) {
      selected_id = &secondary_material_id;
    } 

    if (*selected_id){
      for (int i = 1; i <= pen_size; i++){
        for (int j = 1; j <= pen_size; j++){
          int x = 1 + *MOUSE_X + (pen_size / 2) - i;
          int y = 1 + *MOUSE_Y + (pen_size / 2) - j;
          // Hacky fix to prevent out-of-bounds memory access
          particle_t *particle = get_particle(x, y >= 0 ? y : 0);

          if (*selected_id == MATERIAL_ERASE_ID){
            set_particle_material_id(particle, MATERIAL_AIR_ID); 
          } else if (get_particle_material_id(particle) == MATERIAL_AIR_ID) {
            if (pen_size > 1 && *selected_id != MATERIAL_GLASS_ID){
              // Don't scatter the particles if the game is paused
              if(paused || ((rand() % 100) < 40)) {
                set_particle_material_id(particle, *selected_id); 
              }
            } else {
              set_particle_material_id(particle, *selected_id); 
            }
          }
        }
      }
    }
  }
  
  // Draw cursor
  *DRAW_COLORS = cursor_colors[primary_material_id - 1] + 1;
  rect(1 + *MOUSE_X + (pen_size / 2) - pen_size, 1 + *MOUSE_Y + (pen_size / 2) - pen_size, pen_size, pen_size);

  // Draw menu sprite
  *DRAW_COLORS = 0x4321;
  blit(menu, 0, CANVAS_HEIGHT, MENU_WIDTH, MENU_HEIGHT, MENU_FLAGS);
  // Draw play sprite
  if (paused) {
    blit(play, 120, CANVAS_HEIGHT + 19, PLAY_WIDTH, PLAY_HEIGHT, PLAY_FLAGS);
  }
  
  // Attempt to pick new material type
  if (*MOUSE_X < 160 && *MOUSE_X >= 0 && *MOUSE_Y > CANVAS_HEIGHT && mouse_this_frame){
    int col = *MOUSE_X / 40;
    int row = (*MOUSE_Y - CANVAS_HEIGHT) / 10;
    static int *selected_id = NULL;

    if (*MOUSE_BUTTONS & MOUSE_LEFT) {
      selected_id = &primary_material_id;
    }
    if (*MOUSE_BUTTONS & MOUSE_RIGHT) {
      selected_id = &secondary_material_id;
    }
    
    int menu_item = (col * 4) + row + 1;
    if (menu_item <= 7 || menu_item == 13) {
      // Select an element/material
      *selected_id = menu_item;
    } else if (menu_item == 14) {
      // Reset canvas
      clear_particles();
    } else if (menu_item == 15) {
      // Toggle paused/play state
      paused = paused ? false : true;
    } else if (menu_item == 16) {
      // Cycle through pen sizes
      if (++pen_size > 4) {
        pen_size = 1;
      }
    }
  }
  
  // Draw primary material dot
  *DRAW_COLORS = 4;
  int row_index = ((primary_material_id % 4) == 0 ? 4 : primary_material_id / 4);
  int col_index = ((primary_material_id % 4) == 0 ? 4 : primary_material_id % 4);
  pixel((row_index * 40) + 2, (col_index * 9) + CANVAS_HEIGHT - 3);
  
  // Draw secondary material dot
  *DRAW_COLORS = 3;
  row_index = ((secondary_material_id % 4) == 0 ? 4 : secondary_material_id / 4);
  col_index = ((secondary_material_id % 4) == 0 ? 4 : secondary_material_id % 4);
  pixel((row_index * 40) + 2, (col_index * 9) + CANVAS_HEIGHT - 2);
  
  // Draw pen size indicator
  *DRAW_COLORS = 3;
  rect(152 + (pen_size / 2) - pen_size, CANVAS_HEIGHT + 34 + (pen_size / 2) - pen_size, pen_size, pen_size);
}
