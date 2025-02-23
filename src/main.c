#include <ti/screen.h>
#include <ti/getcsc.h>
#include <stdlib.h>

#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <math.h>
#include <fileioc.h>

#define MAX_ITER 50
#define APPVAR_NAME "FRACTAL2"

static const float Sixteenth = 1.0f / 16.0f;

static void save_screen(void) {
    // 1. Open/create the appvar in write mode.
    ti_var_t file = ti_Open(APPVAR_NAME, "w");
    if (!file) {
        return;
    }

    // 2. Write the entire VRAM to the appvar.
    //    Note that gfx_vram is 16-bit, but we treat it as a raw byte array.
    ti_Write(gfx_vram, LCD_SIZE, 1, file);
    
    ti_SetArchiveStatus(true, file);

    // 3. Close the file.
    ti_Close(file);
}

inline uint8_t get_color(int iter) {
    if (iter == MAX_ITER) return 0; // Black for points inside the Mandelbrot set

    return iter % 255; // Gradient effect
}

void draw_mandelbrot(void) {
    float min_re = -2.0, max_re = 1.0;
    float min_im = -1.2, max_im = 1.2;
    float re_factor = (max_re - min_re) / GFX_LCD_WIDTH;
    float im_factor = (max_im - min_im) / GFX_LCD_HEIGHT;

    for (int y = 0; y < GFX_LCD_HEIGHT; y++) {
        for (int x = 0; x < GFX_LCD_WIDTH; x++) {
            if (os_GetCSC() == sk_Clear) {
                return;
            }
            
            float c_re = min_re + x * re_factor;
            float c_im = max_im - y * im_factor;
            
            // Cardioid check:
            float xshift = c_re - 0.25f;
            float c_im_sqrd = c_im * c_im;
            float r  = xshift * xshift + c_im_sqrd;
            if (r * (r + xshift) <= 0.25f * c_im_sqrd) {
                // The point is in the main cardioid. Color it black and return.
                gfx_SetColor(0);
                gfx_SetPixel(x, y);
                continue;
            }

            // Period-2 bulb check:
            float xplus1 = c_re + 1.0f;
            if (xplus1 * xplus1 + c_im_sqrd <= Sixteenth) {
                // In the period-2 bulb
                gfx_SetColor(0);
                gfx_SetPixel(x, y);
                continue;
            }
            
            float z_re = 0, z_im = 0;
            int iter;

            for (iter = 0; iter < MAX_ITER; iter++) {
                float z_re2 = z_re * z_re, z_im2 = z_im * z_im;
                if (z_re2 + z_im2 > 4) break;
                float new_re = z_re2 - z_im2 + c_re;
                float new_im = 2 * z_re * z_im + c_im;
                z_re = new_re;
                z_im = new_im;
            }

            uint8_t color = get_color(iter);
            gfx_SetColor(color);
            gfx_SetPixel(x, y);
        }
    }
}

int main(void) {
    gfx_Begin();
    gfx_SetDrawScreen();

    {
        draw_mandelbrot();
        save_screen();
        
        while (!os_GetCSC());
        
        gfx_End();
    }
    
    return 0;
}
