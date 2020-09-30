#define MS_GOAL        10   // to try maintain 1000 / 10ms == 100 frames per second

#define NUM_LEAPERS        4
#define GRAVITY            10
#define SETTLED_THRESHOLD  48
#define WALL_FRICTION      248    // 255 is no friction
#define DRAG               240    // 255 is no wind resistance


CRGBPalette16 leaperPalette = RainbowColors_p;
uint32_t last_millis = 0;

typedef struct {
  int16_t x, y;
  int32_t xd, yd;
  // uint8_t state;
} Leaper;

Leaper leapers[NUM_LEAPERS];

extern "C" {
  void restart_leaper(Leaper * l);
  void move_leaper(Leaper * l);
}

void leaperSetup() {
  for (uint8_t l = 0; l < NUM_LEAPERS; l++) {
    // restart_leaper(&leapers[l]);
    leapers[l].x = random8() * kMatrixWidth;
    leapers[l].y = random8() * kMatrixHeight;
  }
}

/*  */
// x and y are 24.8 fixed point
// Not Ray Wu. ;)  The idea came from Xiaolin Wu.
void wu_pixel(uint32_t x, uint32_t y, CRGB * col) {
    // extract the fractional parts and derive their inverses
    uint8_t xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
    // calculate the intensities for each affected pixel
    #define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
    uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                     WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
    // multiply the intensities by the colour, and saturating-add them to the pixels
    for (uint8_t i = 0; i < 4; i++) {
        uint16_t xy = XY((x >> 8) + (i & 1), (y >> 8) + ((i >> 1) & 1));
        leds[xy].r = qadd8(leds[xy].r, col->r * wu[i] >> 8);
        leds[xy].g = qadd8(leds[xy].g, col->g * wu[i] >> 8);
        leds[xy].b = qadd8(leds[xy].b, col->b * wu[i] >> 8);
    }
}

void leaperLoop() {
  FastLED.delay(1000/60);
  FastLED.clear();
  // fadeToBlackBy(leds, NUM_LEDS, 64);
  for (uint8_t l = 0; l < NUM_LEAPERS; l++) {
    move_leaper(&leapers[l]);
    CRGB rgb = ColorFromPalette(leaperPalette, l * (255 / NUM_LEAPERS), 255, LINEARBLEND);
    wu_pixel(leapers[l].x, leapers[l].y, &rgb);
  }
}

void restart_leaper(Leaper * l) {
  // leap up and to the side with some random component
  l->xd = random8() + 10;
  l->yd = random8() + 128;

  // for variety, sometimes go 50% faster
  if (random8() < 12) {
    l->xd += l->xd >> 1;
    l->yd += l->yd >> 1;
  }

  // leap towards the centre of the screen
  if (l->x > (kMatrixWidth / 2 * 256)) {
    l->xd = -l->xd;
  }
}

void move_leaper(Leaper * l) {
  // add the X & Y velocities to the position
  l->x += l->xd;
  l->y += l->yd;

  // bounce off the floor and ceiling?
  if (l->y < 0 || l->y >= ((kMatrixHeight - 1) << 8)) {
    l->yd = (-l->yd * WALL_FRICTION) >> 8;
    l->xd = ( l->xd * WALL_FRICTION) >> 8;
    l->y += l->yd;
    if (l->y < 0) l->y = 0;
    // settled on the floor?
    if (l->y <= SETTLED_THRESHOLD && abs(l->yd) <= SETTLED_THRESHOLD) {
      restart_leaper(l);
    }
  }

  // bounce off the sides of the screen?
  if (l->x <= 0 || l->x >= (kMatrixWidth - 1) << 8) {
    l->xd = (-l->xd * WALL_FRICTION) >> 8;
    // l->yd = (-l->yd * WALL_FRICTION) >> 8;
    // force back onto the screen, otherwise they eventually sneak away
    if (l->x <= 0) {
      l->x = l->xd;
    } else {
      l->x = ((kMatrixWidth - 1) << 8) - l->xd;
    }
  }

  // gravity
  l->yd -= GRAVITY;

  // viscosity,  done badly
  // uint32_t speed2 = l->xd * l->xd + l->yd * l->yd;
  l->xd = (l->xd * DRAG) >> 8;
  l->yd = (l->yd * DRAG) >> 8;
}

