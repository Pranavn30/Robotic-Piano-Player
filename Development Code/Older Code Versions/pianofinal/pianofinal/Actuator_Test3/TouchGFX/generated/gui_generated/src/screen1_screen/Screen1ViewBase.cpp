/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <touchgfx/Color.hpp>
#include <images/BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>

Screen1ViewBase::Screen1ViewBase() :
    frameCountInteraction2Interval(0)
{
    __background.setPosition(0, 0, 320, 240);
    __background.setColor(touchgfx::Color::getColorFromRGB(0, 0, 0));
    add(__background);

    image1.setXY(0, 0);
    image1.setBitmap(touchgfx::Bitmap(BITMAP_ALTERNATE_THEME_IMAGES_BACKGROUNDS_320X240_POLY_ID));
    add(image1);

    boxWithBorder1.setPosition(0, 15, 320, 95);
    boxWithBorder1.setColor(touchgfx::Color::getColorFromRGB(255, 255, 255));
    boxWithBorder1.setBorderColor(touchgfx::Color::getColorFromRGB(107, 104, 104));
    boxWithBorder1.setBorderSize(1);
    add(boxWithBorder1);

    textArea1.setPosition(30, 52, 516, 20);
    textArea1.setColor(touchgfx::Color::getColorFromRGB(0, 0, 0));
    textArea1.setLinespacing(0);
    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%s", touchgfx::TypedText(T___SINGLEUSE_3WQK).getText());
    textArea1.setWildcard(textArea1Buffer);
    textArea1.setTypedText(touchgfx::TypedText(T___SINGLEUSE_G9G3));
    add(textArea1);

    button1.setXY(130, 147);
    button1.setBitmaps(touchgfx::Bitmap(BITMAP_ALTERNATE_THEME_IMAGES_WIDGETS_BUTTON_ICON_ROUND_TINY_FILL_ACTION_ID), touchgfx::Bitmap(BITMAP_ALTERNATE_THEME_IMAGES_WIDGETS_BUTTON_ICON_ROUND_TINY_FILL_PRESSED_ID));
    add(button1);

    image2.setXY(135, 152);
    image2.setBitmap(touchgfx::Bitmap(BITMAP_ICON_THEME_IMAGES_AV_PLAY_CIRCLE_OUTLINE_50_50_000000_SVG_ID));
    add(image2);
}

Screen1ViewBase::~Screen1ViewBase()
{

}

void Screen1ViewBase::setupScreen()
{

}

void Screen1ViewBase::handleTickEvent()
{
    //Interaction1
    //When every N tick move textArea1
    //Move textArea1 to x:-550, y:52 with QuadOut easing in 10000 ms (600 Ticks)
    textArea1.clearMoveAnimationEndedAction();
    textArea1.startMoveAnimation(-550, 52, 600, touchgfx::EasingEquations::quadEaseOut, touchgfx::EasingEquations::quadEaseOut);

    frameCountInteraction2Interval++;
    if(frameCountInteraction2Interval == TICK_INTERACTION2_INTERVAL)
    {
        //Interaction2
        //When every N tick move textArea1
        //Move textArea1 to x:270, y:52 with LinearOut easing in 1 ms (1 Ticks)
        textArea1.clearMoveAnimationEndedAction();
        textArea1.startMoveAnimation(270, 52, 1, touchgfx::EasingEquations::linearEaseOut, touchgfx::EasingEquations::linearEaseOut);
        frameCountInteraction2Interval = 0;
    }
}