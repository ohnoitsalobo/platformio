
class FireWork{
    private:
        float _pos = 0;
        float _vel = 0;
        float _acc = 0;
        CHSV _color;
        CRGBPalette16 _palette;
        
    public:
        FireWork(){
            init(NUM_LEDS/2);
        };
        
        void init(int position){
            _palette = RainbowColors_p;
            _color = CHSV(random8(100)+gHue, 255-random8(50), 255);
            // _pos = NUM_LEDS/2 + random16(0, (NUM_LEDS/2-1)*3.0f)/3.0f;
            _pos = position;
            _vel = (500 - random16(0, 1000))*0.002f;
            _acc = (1000-random16(10, 35))*0.001;
        }
        
        void draw(int _segment){
            DrawPixels(_segment, _pos, 1.5, _color);
            _color.h += 1;
            _color.v *= 0.98;
            _pos += _vel;
            _vel *= _acc;
            if(_pos > NUM_LEDS)
                _vel *= -1;
            else if (_pos < 0)
                _vel *= -1;
        }
        
        CRGB ColorFraction(CRGB colorIn, float fraction){
            fraction = min(1.0f, fraction);
            return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
        }

        void DrawPixels(uint8_t segment, float fPos, float count, CRGB color){
            // Calculate how much the first pixel will hold
            float availFirstPixel = 1.0f - (fPos - (long)(fPos));
            float amtFirstPixel = min(availFirstPixel, count);
            float remaining = min(count, FastLED.size()-fPos);
            int iPos = fPos;

            // Blend (add) in the color of the first partial pixel

            if (remaining > 0.0f){
                led_array[segment][iPos++] += ColorFraction(color, amtFirstPixel);
                remaining -= amtFirstPixel;
            }

            // Now draw any full pixels in the middle

            while (remaining > 1.0f){
                led_array[segment][iPos++] += color;
                remaining--;
            }

            // Draw tail pixel, up to a single full pixel

            if (remaining > 0.0f){
                led_array[segment][iPos] += ColorFraction(color, remaining);
            }
        }
};

