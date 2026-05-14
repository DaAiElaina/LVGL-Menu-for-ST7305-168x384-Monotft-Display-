
#ifndef ST7305_BITMAP_HELPER_H
#define ST7305_BITMAP_HELPER_H

#include <Arduino.h>
#include "ST7305_2p9_BW_DisplayDriver.h"

// Draw a 1-bit bitmap to the ST7305 framebuffer.
// bitmap: row-major, MSB-first in each byte (like Adafruit_GFX drawBitmap)
// x,y: top-left on screen
// w,h: bitmap width/height in pixels
// color: ST7305_COLOR_BLACK or ST7305_COLOR_WHITE
static inline void st7305_drawBitmap1bit(ST7305_2p9_BW_DisplayDriver &display,
                                         int16_t x, int16_t y,
                                         const uint8_t *bitmap,
                                         int16_t w, int16_t h,
                                         uint16_t color,
                                         bool drawBackground = false,
                                         uint16_t bg = ST7305_COLOR_WHITE)
{
  int bytesPerRow = (w + 7) / 8;
  
  for (int16_t yy = 0; yy < h; yy++) {
    int16_t sy = y + yy;
    // 边界检查：超出屏幕高度就跳过整行
    if (sy < 0 || sy >= 384) continue;
    
    const uint8_t *row = bitmap + yy * bytesPerRow;
    for (int16_t xx = 0; xx < w; xx++) {
      int16_t sx = x + xx;
      // 边界检查：超出屏幕宽度就跳过
      if (sx < 0 || sx >= 168) continue;
      
      if ((xx & 7) == 0) {
        uint8_t current = row[xx >> 3];
        // 预计算8个像素
        for (int b = 0; b < 8 && xx + b < w; b++) {
          bool bitSet = (current & (0x80 >> b)) != 0;
          int16_t sx2 = sx + b;
          if (sx2 < 168) {
            if (bitSet) {
              display.writePoint(sx2, sy, color);
            } else if (drawBackground) {
              display.writePoint(sx2, sy, bg);
            }
          }
        }
        xx += 7;  // 跳过已处理的像素
      }
    }
  }
}

#endif // ST7305_BITMAP_HELPER_H
