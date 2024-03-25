/*
 * Copyright (c) 2024 the ThorVG project. All rights reserved.

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

#ifndef _TVG_LOTTIE_EXPRESSIONS_H_
#define _TVG_LOTTIE_EXPRESSIONS_H_

#include "tvgCommon.h"

struct LottieComposition;
struct LottieLayer;
struct LottieObject;
struct LottieProperty;
struct LottieTransform;
struct RGB24;

#ifdef THORVG_LOTTIE_EXPRESSIONS_SUPPORT

#include "jerryscript.h"


struct LottieExpression
{
    char* code;
    LottieComposition* comp;
    LottieLayer* layer;
    LottieObject* object;
    LottieProperty* property;

    bool refernce = false;

    static LottieExpression* gen(char* code, LottieComposition* comp, LottieLayer* layer, LottieObject* object, LottieProperty* property)
    {
        auto inst = new LottieExpression;
        inst->code = code;
        inst->comp = comp;
        inst->layer = layer;
        inst->object = object;
        inst->property = property;

        return inst;
    }

    ~LottieExpression()
    {
        free(code);
    }
};


struct LottieExpressions
{
public:
    ~LottieExpressions();

    template<typename Property, typename NumType>
    bool result(float frameNo, NumType& out, LottieExpression* exp)
    {
        auto bm_rt = evaluate(frameNo, exp);

        if (jerry_value_is_number(bm_rt)) {
            //FIXME:
            //out = { (NumType) jerry_value_as_number(bm_rt) };
            auto tmp = (NumType) jerry_value_as_number(bm_rt);
        } else {
            auto prop = static_cast<Property*>(jerry_object_get_native_ptr(bm_rt, nullptr));
            if (prop) out = (*prop)(frameNo);
            else TVGERR("LOTTIE", "Failed dispatching Numberic Property!");
        }
        jerry_value_free(bm_rt);
        return true;
    }

    template<typename Property>
    bool result(float frameNo, Point& out, LottieExpression* exp)
    {
        auto bm_rt = evaluate(frameNo, exp);
        auto prop = static_cast<Property*>(jerry_object_get_native_ptr(bm_rt, nullptr));

        if (prop) out = (*prop)(frameNo);
        else TVGERR("LOTTIE", "Failed dispatching Object Property!");

        jerry_value_free(bm_rt);
        return true;
    }

    template<typename Property>
    bool result(float frameNo, RGB24& out, LottieExpression* exp)
    {
        auto bm_rt = evaluate(frameNo, exp);
        auto color = static_cast<Property*>(jerry_object_get_native_ptr(bm_rt, nullptr));

        if (color) out = (*color)(frameNo);
        else TVGERR("LOTTIE", "Failed dispatching Color!");

        jerry_value_free(bm_rt);
        return true;
    }

    template<typename Property>
    bool result(float frameNo, Fill* fill, LottieExpression* exp)
    {
        auto bm_rt = evaluate(frameNo, exp);
        auto colorStop = static_cast<Property*>(jerry_object_get_native_ptr(bm_rt, nullptr));

        if (colorStop) (*colorStop)(frameNo, fill, *this);
        else TVGERR("LOTTIE", "Failed dispatching ColorStop!");

        jerry_value_free(bm_rt);
        return true;
    }

    template<typename Property>
    bool result(float frameNo, Array<PathCommand>& cmds, Array<Point>& pts, LottieExpression* exp)
    {
        auto bm_rt = evaluate(frameNo, exp);
        auto pathSet = static_cast<Property*>(jerry_object_get_native_ptr(bm_rt, nullptr));

        if (pathSet) (*pathSet)(frameNo, cmds, pts, *this);
        else TVGERR("LOTTIE", "Failed dispatching PathSet!");

        jerry_value_free(bm_rt);
        return true;
    }

    void prepare(LottieComposition* comp);
    void update(float frameNo, LottieComposition* comp);

private:
    jerry_value_t evaluate(float frameNo, LottieExpression* exp);

    void buildGlobal();
    void buildComp();
    void buildMath();

    //global object, attributes, methods
    jerry_value_t global;
    jerry_value_t thisComp;
    jerry_value_t thisLayer;
    jerry_value_t thisProperty;

    LottieComposition* comp;
    bool prepared = false;
};

#else

struct LottieExpression
{
    static LottieExpression* gen(TVG_UNUSED char* code, TVG_UNUSED LottieLayer* layer, TVG_UNUSED LottieObject* object, TVG_UNUSED LottieProperty* property)
    {
        return nullptr;
    }
};


struct LottieExpressions
{
    LottieExpressions() {}

    template<typename Property, typename NumType> bool dispatch(TVG_UNUSED float, TVG_UNUSED NumType&, TVG_UNUSED LottieExpression*) { return false; }
    template<typename Property> bool dispatch(TVG_UNUSED float, TVG_UNUSED Point&, LottieExpression*) { return false; }
    template<typename Property> bool dispatch(TVG_UNUSED float, TVG_UNUSED RGB24&, TVG_UNUSED LottieExpression*) { return false; }
    template<typename Property> bool dispatch(TVG_UNUSED float, TVG_UNUSED Fill*, TVG_UNUSED LottieExpression*) { return false; }
    template<typename Property> bool dispatch(TVG_UNUSED float, TVG_UNUSED Array<PathCommand>&, TVG_UNUSED Array<Point>&, TVG_UNUSED LottieExpression*) { return false; }
    void prepare(TVG_UNUSED LottieComposition*) {}
    void update(TVG_UNUSED float, TVG_UNUSED LottieComposition*) {}
};

#endif //THORVG_LOTTIE_EXPRESSIONS_SUPPORT

#endif //_TVG_LOTTIE_EXPRESSIONS_H_