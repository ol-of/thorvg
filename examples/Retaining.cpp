/*
 * Copyright (c) 2020 - 2025 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Example.h"

/************************************************************************/
/* ThorVG Drawing Contents                                              */
/************************************************************************/

struct UserExample : tvgexam::Example
{
    uint32_t last = 0;

    bool content(tvg::Canvas* canvas, uint32_t w, uint32_t h) override
    {
        //Prepare Round Rectangle
        auto shape1 = tvg::Shape::gen();
        shape1->appendRect(0, 0, 480, 480, 50, 50);  //x, y, w, h, rx, ry
        shape1->fill(0, 255, 0);                     //r, g, b
        canvas->push(shape1);

        //Prepare Round Rectangle2
        auto shape2 = tvg::Shape::gen();
        shape2->appendRect(140, 140, 480, 480, 50, 50);  //x, y, w, h, rx, ry
        shape2->fill(255, 255, 0);                       //r, g, b
        canvas->push(shape2);

        //Prepare Round Rectangle3
        auto shape3 = tvg::Shape::gen();
        shape3->appendRect(280, 280, 480, 480, 50, 50);  //x, y, w, h, rx, ry
        shape3->fill(0, 255, 255);                       //r, g, b
        canvas->push(shape3);

        //Prepare Scene
        auto scene = tvg::Scene::gen();

        auto shape4 = tvg::Shape::gen();
        shape4->appendCircle(520, 520, 140, 140);
        shape4->fill(255, 0, 0);
        shape4->strokeWidth(5);
        shape4->strokeFill(255, 255, 255);
        scene->push(shape4);

        auto shape5 = tvg::Shape::gen();
        shape5->appendCircle(630, 630, 190, 190);
        shape5->fill(255, 0, 255);
        shape5->strokeWidth(5);
        shape5->strokeFill(255, 255, 255);
        scene->push(shape5);

        canvas->push(scene);

        return true;
    }

    bool update(tvg::Canvas* canvas, uint32_t elapsed) override
    {
        //update per every 250ms
        //reorder with a circular list
        if (elapsed - last > 250) {
            //Acquire the first paint from the root scene
            auto paint = *canvas->paints().begin();

            //Prevent deleting from canvas->remove()
            paint->ref();

            //Re-push the front paint to the end of the root scene
            tvgexam::verify(canvas->remove(paint));
            tvgexam::verify(canvas->push(paint));

            //Make it pair ref() - unref()
            paint->unref();

            last = elapsed;
        }

        return true;
    }
};


/************************************************************************/
/* Entry Point                                                          */
/************************************************************************/

int main(int argc, char **argv)
{
    return tvgexam::main(new UserExample, argc, argv, true, 960, 960);
}