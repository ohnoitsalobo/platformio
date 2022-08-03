float _x[half_size];                              // arrays for the 2d coordinates of any led
float _y[half_size];

void XYsetup(){
    for (uint16_t i = 0; i < half_size; i++) {         // precalculate the lookup-tables:
        float angle = (i * 255) / half_sizef;         // on which position on the circle is the led?
        _x[i] = cos8( angle );                         // corrsponding x position in the matrix
        _y[i] = sin8( angle );                         // corrsponding y position in the matrix
    }
}
// moves a noise up and down while slowly shifting to the side
void noise1() {

    uint8_t scale = 255;                               // the "zoom factor" for the noise

    for (uint16_t i = 0; i < half_size; i++) {

        uint16_t shift_x = beatsin8(10);                  // the x position of the noise field swings @ 17 bpm
        uint16_t shift_y = millis() / 100;                // the y position becomes slowly incremented

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint8_t noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        uint8_t index = noise * 3;                        // map led color based on noise data
        uint8_t bri   = noise;

        CRGB color = CHSV( index, 255, bri);
        RIGHT[i] = color;
    }
    
    for (uint16_t i = 0; i < half_size; i++) {

        uint16_t shift_x = beatsin8(11);                  // the x position of the noise field swings @ 17 bpm
        uint16_t shift_y = millis() / 90;                 // the y position becomes slowly incremented

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint8_t noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        uint8_t index = noise * 3;                        // map led color based on noise data
        uint8_t bri   = noise;

        CRGB color = CHSV( index, 255, bri);
        LEFT [i] = color;
    }
}

// just moving along one axis = "lavalamp effect"
void noise2() {

    uint8_t scale = 255;                               // the "zoom factor" for the noise

    for (uint16_t i = 0; i < half_size; i++) {

        uint16_t shift_x = millis() / 10;                 // x as a function of time
        uint16_t shift_y = 0;

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint8_t noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        uint8_t index = noise * 3;                        // map led color based on noise data
        uint8_t bri   = noise;

        CRGB color = CHSV( index, 255, bri);
        RIGHT[i] = color;
    }
    
    for (uint16_t i = 0; i < half_size; i++) {

        uint16_t shift_x = millis() / 10;                 // x as a function of time
        uint16_t shift_y = 0;

        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint8_t noise = inoise16(real_x, real_y, 4223) >> 8;           // get the noise data and scale it down

        uint8_t index = noise * 3;                        // map led color based on noise data
        uint8_t bri   = noise;

        CRGB color = CHSV( index, 255, bri);
        LEFT [i] = color;
    }
}

// no x/y shifting but scrolling along z
void noise3() {

    uint8_t scale = 255;                               // the "zoom factor" for the noise

    for (uint16_t i = 0; i < half_size; i++) {

        uint16_t shift_x = 0;                             // no movement along x and y
        uint16_t shift_y = 0;


        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint32_t real_z = millis() * 20;                  // increment z linear

        uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;           // get the noise data and scale it down

        uint8_t index = noise * 3;                        // map led color based on noise data
        uint8_t bri   = noise;

        CRGB color = CHSV( index, 255, bri);
        RIGHT[i] = color;
    }

    for (uint16_t i = 0; i < half_size; i++) {

        uint16_t shift_x = 0;                             // no movement along x and y
        uint16_t shift_y = 0;


        uint32_t real_x = (_x[i] + shift_x) * scale;       // calculate the coordinates within the noise field
        uint32_t real_y = (_y[i] + shift_y) * scale;       // based on the precalculated positions

        uint32_t real_z = millis() * 21;                  // increment z linear

        uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;           // get the noise data and scale it down

        uint8_t index = noise * 3;                        // map led color based on noise data
        uint8_t bri   = noise;

        CRGB color = CHSV( index, 255, bri);
        LEFT[i] = color;
    }
}
