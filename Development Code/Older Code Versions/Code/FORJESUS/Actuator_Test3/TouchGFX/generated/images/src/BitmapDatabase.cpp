// 4.22.1 0xbaff8df6
// Generated by imageconverter. Please, do not edit!

#include <images/BitmapDatabase.hpp>
#include <touchgfx/Bitmap.hpp>

extern const unsigned char image_alternate_theme_images_backgrounds_320x240_poly[]; // BITMAP_ALTERNATE_THEME_IMAGES_BACKGROUNDS_320X240_POLY_ID = 0, Size: 320x240 pixels
extern const unsigned char image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal[]; // BITMAP_ALTERNATE_THEME_IMAGES_WIDGETS_TOGGLEBUTTON_MEDIUM_ROUNDED_TEXT_OFF_NORMAL_ID = 1, Size: 138x50 pixels
extern const unsigned char image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action[]; // BITMAP_ALTERNATE_THEME_IMAGES_WIDGETS_TOGGLEBUTTON_MEDIUM_ROUNDED_TEXT_ON_ACTION_ID = 2, Size: 138x50 pixels

const touchgfx::Bitmap::BitmapData bitmap_database[] = {
    { image_alternate_theme_images_backgrounds_320x240_poly, 0, 320, 240, 0, 0, 320, ((uint8_t)touchgfx::Bitmap::RGB565) >> 3, 240, ((uint8_t)touchgfx::Bitmap::RGB565) & 0x7 },
    { image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal, 0, 138, 50, 10, 9, 57, ((uint8_t)touchgfx::Bitmap::ARGB8888) >> 3, 32, ((uint8_t)touchgfx::Bitmap::ARGB8888) & 0x7 },
    { image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action, 0, 138, 50, 71, 9, 57, ((uint8_t)touchgfx::Bitmap::ARGB8888) >> 3, 32, ((uint8_t)touchgfx::Bitmap::ARGB8888) & 0x7 }
};

namespace BitmapDatabase
{
const touchgfx::Bitmap::BitmapData* getInstance()
{
    return bitmap_database;
}

uint16_t getInstanceSize()
{
    return (uint16_t)(sizeof(bitmap_database) / sizeof(touchgfx::Bitmap::BitmapData));
}
} // namespace BitmapDatabase