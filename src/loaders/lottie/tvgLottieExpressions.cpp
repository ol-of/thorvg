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

#include <alloca.h>
#include "tvgLottieModel.h"
#include "tvgLottieExpressions.h"

#ifdef THORVG_LOTTIE_EXPRESSIONS_SUPPORT

/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/

//reserved expressions speicifiers
static const char* EXP_NAME = "name";
static const char* EXP_CONTENT = "content";
static const char* EXP_WIDTH = "width";
static const char* EXP_HEIGHT = "height";

static void _buildLayer(jerry_value_t context, LottieLayer* layer, LottieComposition* comp);


static char* _name(jerry_value_t args)
{
    auto arg0 = jerry_value_to_string(args);
    auto len = jerry_string_length(arg0);
    auto name = (jerry_char_t*)malloc(len * sizeof(jerry_char_t) + 1);
    jerry_string_to_buffer(arg0, JERRY_ENCODING_UTF8, name, len);
    name[len] = '\0';
    jerry_value_free(arg0);
    return (char*) name;
}


static jerry_value_t _mul(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 2) return jerry_undefined();

    auto arg0 = jerry_value_to_number(args[0]);
    auto arg1 = jerry_value_to_number(args[1]);

    auto ret = jerry_value_as_number(arg0) * jerry_value_as_number(arg1);

    jerry_value_free(arg0);
    jerry_value_free(arg1);

    return jerry_number(ret);
}


static jerry_value_t _sum(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 2) return jerry_undefined();

    auto arg0 = jerry_value_to_number(args[0]);
    auto arg1 = jerry_value_to_number(args[1]);

    auto ret = jerry_value_as_number(arg0) + jerry_value_as_number(arg1);

    jerry_value_free(arg0);
    jerry_value_free(arg1);

    return jerry_number(ret);
}


static jerry_value_t _add(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 2) return jerry_undefined();

    auto arg0 = jerry_value_to_number(args[0]);
    auto arg1 = jerry_value_to_number(args[1]);

    auto ret = jerry_value_as_number(arg0) + jerry_value_as_number(arg1);

    jerry_value_free(arg0);
    jerry_value_free(arg1);

    return jerry_number(ret);
}

static jerry_value_t _sub(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 2) return jerry_undefined();

    auto arg0 = jerry_value_to_number(args[0]);
    auto arg1 = jerry_value_to_number(args[1]);

    auto ret = jerry_value_as_number(arg0) - jerry_value_as_number(arg1);

    jerry_value_free(arg0);
    jerry_value_free(arg1);

    return jerry_number(ret);
}


static jerry_value_t _div(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 2) return jerry_undefined();

    auto arg0 = jerry_value_to_number(args[0]);
    auto arg1 = jerry_value_to_number(args[1]);

    auto ret = jerry_value_as_number(arg0) / jerry_value_as_number(arg1);

    jerry_value_free(arg0);
    jerry_value_free(arg1);

    return jerry_number(ret);
}


static jerry_value_t _path(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto name = _name(args[0]);

    //find the a path property(sh) in the shape layer
    auto group = static_cast<LottieGroup*>(jerry_object_get_native_ptr(info->function, nullptr));
    auto path = group->content((char*)name);
    free(name);

    if (!path) return jerry_undefined();

    //TODO: other properties?
    jerry_value_t pathset = jerry_object();
    jerry_object_set_native_ptr(pathset, nullptr, &static_cast<LottiePath*>(path)->pathset);
    jerry_object_set_sz(pathset, "path", pathset);

    return pathset;
}


static jerry_value_t _shape(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto name = _name(args[0]);

    //find a shape layer(group) from the root
    auto layer = static_cast<LottieLayer*>(jerry_object_get_native_ptr(info->function, nullptr));
    auto group = layer->content((char*)name);
    free(name);

    if (!group) return jerry_undefined();

    //TODO: other properties?
    auto property = jerry_function_external(_path);
    jerry_object_set_native_ptr(property, nullptr, group);
    jerry_object_set_sz(property, EXP_CONTENT, property);

    return property;
}


static jerry_value_t _layer(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto comp = static_cast<LottieComposition*>(jerry_object_get_native_ptr(info->function, nullptr));
    LottieLayer* layer;

    //layer index
    if (jerry_value_is_number(args[0])) {
        auto idx = (uint16_t)jerry_value_as_integer(args[0]);
        layer = comp->layer(idx);
        jerry_value_free(idx);
    //layer name
    } else {
        auto name = _name(args[0]);
        layer = comp->layer((char*)name);
        free(name);
    }

    if (!layer) return jerry_undefined();

    auto obj = jerry_object();
    jerry_object_set_native_ptr(obj, nullptr, layer);
    _buildLayer(obj, layer, comp);

    return obj;
}


static jerry_value_t _nearestKey(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto time = jerry_value_to_number(args[0]);

    auto exp = static_cast<LottieExpression*>(jerry_object_get_native_ptr(info->function, nullptr));
    auto frameNo = exp->comp->frameAtTime(time);
    auto keyidx = exp->property->nearest(frameNo);

    auto index = jerry_number(keyidx);
    jerry_object_set_sz(index, "index", keyidx);
    return index;
}



static jerry_value_t _valueAtTime(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto time = jerry_value_to_number(args[0]);

    auto exp = static_cast<LottieExpression*>(jerry_object_get_native_ptr(info->function, nullptr));
    auto frameNo = exp->comp->frameAtTime(time);

    //TODO:

}


static jerry_value_t _velocityAtTime(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto time = jerry_value_to_number(args[0]);

    auto exp = static_cast<LottieExpression*>(jerry_object_get_native_ptr(info->function, nullptr));
    auto frameNo = exp->comp->frameAtTime(time);

    //TODO:

}


static jerry_value_t _speedAtTime(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto time = jerry_value_to_number(args[0]);

    auto exp = static_cast<LottieExpression*>(jerry_object_get_native_ptr(info->function, nullptr));
    auto frameNo = exp->comp->frameAtTime(time);

    //TODO:

}


static jerry_value_t _key(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto frameNo = jerry_value_as_int32(args[0]);

    auto exp = static_cast<LottieExpression*>(jerry_object_get_native_ptr(info->function, nullptr));

    auto time = jerry_number(exp->comp->timeAtFrame(frameNo));
    jerry_object_set_sz(time, "time", time);
    return time;
}


static void _buildTransform(jerry_value_t context, LottieTransform* transform)
{
    if (!transform) return;

    auto obj = jerry_object();
    jerry_object_set_sz(context, "transform", obj);

    auto anchorPoint = jerry_object();
    jerry_object_set_native_ptr(anchorPoint, nullptr, &transform->anchor);
    jerry_object_set_sz(obj, "position", anchorPoint);
    jerry_value_free(anchorPoint);

    auto position = jerry_object();
    jerry_object_set_native_ptr(position, nullptr, &transform->position);
    jerry_object_set_sz(obj, "position", position);
    jerry_value_free(position);

    auto scale = jerry_object();
    jerry_object_set_native_ptr(scale, nullptr, &transform->scale);
    jerry_object_set_sz(obj, "scale", scale);
    jerry_value_free(scale);

    auto rotation = jerry_object();
    jerry_object_set_native_ptr(rotation, nullptr, &transform->rotation);
    jerry_object_set_sz(obj, "rotation", rotation);
    jerry_value_free(rotation);

    auto opacity = jerry_object();
    jerry_object_set_native_ptr(opacity, nullptr, &transform->opacity);
    jerry_object_set_sz(obj, "rotation", opacity);
    jerry_value_free(opacity);

    jerry_value_free(obj);
}


static void _buildLayer(jerry_value_t context, LottieLayer* layer, LottieComposition* comp)
{
    auto width = jerry_number(layer->w);
    jerry_object_set_sz(context, EXP_WIDTH, width);
    jerry_value_free(width);

    auto height = jerry_number(layer->h);
    jerry_object_set_sz(context, EXP_HEIGHT, height);
    jerry_value_free(height);

    auto index = jerry_number(layer->id);
    jerry_object_set_sz(context, "index", index);
    jerry_value_free(index);

    auto parent = jerry_object();
    jerry_object_set_native_ptr(parent, nullptr, layer->parent);
    jerry_object_set_sz(context, "parent", parent);
    jerry_value_free(parent);

    auto hasParent = jerry_boolean(layer->parent ? true : false);
    jerry_object_set_sz(context, "hasParent", hasParent);
    jerry_value_free(hasParent);

    auto inPoint = jerry_number(layer->inFrame);
    jerry_object_set_sz(context, "inPoint", inPoint);
    jerry_value_free(inPoint);

    auto outPoint = jerry_number(layer->outFrame);
    jerry_object_set_sz(context, "outPoint", outPoint);
    jerry_value_free(outPoint);

    auto startTime = jerry_number(comp->timeAtFrame(layer->startFrame));
    jerry_object_set_sz(context, "startTime", startTime);
    jerry_value_free(startTime);

    auto hasVideo = jerry_boolean(false);
    jerry_object_set_sz(context, "hasVideo", hasVideo);
    jerry_value_free(hasVideo);

    auto hasAudio = jerry_boolean(false);
    jerry_object_set_sz(context, "hasAudio", hasAudio);
    jerry_value_free(hasAudio);

    //active, #current in the animation range?

    auto enabled = jerry_boolean(!layer->hidden);
    jerry_object_set_sz(context, "enabled", enabled);
    jerry_value_free(enabled);

    auto audioActive = jerry_boolean(false);
    jerry_object_set_sz(context, "audioActive", audioActive);
    jerry_value_free(audioActive);

    //sampleImage(point, radius = [.5, .5], postEffect=true, t=time)

    _buildTransform(context, layer->transform);

    //audioLevels, #the value of the Audio Levels property of the layer in decibels

    auto timeRemap = jerry_object();
    jerry_object_set_native_ptr(timeRemap, nullptr, &layer->timeRemap);
    jerry_object_set_sz(context, "timeRemap", timeRemap);
    jerry_value_free(timeRemap);

    //marker.key(index)
    //marker.key(name)
    //marker.nearestKey(t)
    //marker.numKeys

    auto name = jerry_string((jerry_char_t*)layer->name, strlen(layer->name), JERRY_ENCODING_UTF8);
    jerry_object_set_sz(context, EXP_NAME, name);
    jerry_value_free(name);
}


static void _buildProperty(jerry_value_t context, LottieExpression* exp)
{
    //TODO: set value for type
    auto value = jerry_object();
    jerry_object_set_sz(context, "value", value);
    jerry_object_set_native_ptr(value, nullptr, exp->property);
    jerry_value_free(value);

    auto valueAtTime = jerry_function_external(_valueAtTime);
    jerry_object_set_sz(context, "valueAtTime", valueAtTime);
    jerry_object_set_native_ptr(valueAtTime, nullptr, exp);
    jerry_value_free(valueAtTime);

    auto velocity = jerry_number(0.0f);
    jerry_object_set_sz(context, "velocity", velocity);
    jerry_value_free(velocity);

    auto velocityAtTime = jerry_function_external(_velocityAtTime);
    jerry_object_set_sz(context, "velocityAtTime", velocityAtTime);
    jerry_object_set_native_ptr(velocityAtTime, nullptr, exp);
    jerry_value_free(velocityAtTime);

    auto speed = jerry_number(0.0f);
    jerry_object_set_sz(context, "speed", speed);
    jerry_value_free(speed);

    auto speedAtTime = jerry_function_external(_speedAtTime);
    jerry_object_set_sz(context, "speedAtTime", speedAtTime);
    jerry_object_set_native_ptr(speedAtTime, nullptr, exp);
    jerry_value_free(speedAtTime);

    //wiggle(freq, amp, octaves=1, amp_mult=.5, t=time)
    //temporalWiggle(freq, amp, octaves=1, amp_mult=.5, t=time)
    //smooth(width=.2, samples=5, t=time)
    //loopIn(type="cycle", numKeyframes=0)
    //loopOut(type="cycle", numKeyframes=0)
    //loopInDuration(type="cycle", duration=0)
    //loopOutDuration(type="cycle", duration=0)
    auto key = jerry_function_external(_key);
    jerry_object_set_sz(context, "key", key);
    jerry_object_set_native_ptr(key, nullptr, exp);
    jerry_value_free(key);

    //key(markerName)

    auto nearestKey = jerry_function_external(_nearestKey);
    jerry_object_set_native_ptr(nearestKey, nullptr, exp);
    jerry_object_set_sz(context, "nearestKey", nearestKey);
    jerry_value_free(nearestKey);

    auto numKeys = jerry_number(exp->property->frameCnt());
    jerry_object_set_sz(context, "numKeys", numKeys);
    jerry_value_free(numKeys);

    //propertyGroup(countUp = 1)
    //propertyIndex
    //name

    //content("name"), #look for the named shape object from a layer
    auto content = jerry_function_external(_shape);
    jerry_object_set_sz(context, EXP_CONTENT, content);
    jerry_object_set_native_ptr(content, nullptr, exp->layer);
    jerry_value_free(content);
}


static jerry_value_t _comp(const jerry_call_info_t* info, const jerry_value_t args[], const jerry_length_t argsCnt)
{
    if (argsCnt != 1) return jerry_undefined();

    auto comp = static_cast<LottieComposition*>(jerry_object_get_native_ptr(info->function, nullptr));
    LottieLayer* layer;

    auto arg0 = jerry_value_to_string(args[0]);
    auto len = jerry_string_length(arg0);
    auto name = (jerry_char_t*)alloca(len * sizeof(jerry_char_t) + 1);
    jerry_string_to_buffer(arg0, JERRY_ENCODING_UTF8, name, len);
    name[len] = '\0';

    jerry_value_free(arg0);

    layer = comp->asset((char*)name);

    if (!layer) return jerry_undefined();

    auto obj = jerry_object();
    jerry_object_set_native_ptr(obj, nullptr, layer);
    _buildLayer(obj, layer, comp);

    return obj;
}


void LottieExpressions::buildMath()
{
    auto bm_mul = jerry_function_external(_mul);
    jerry_object_set_sz(global, "$bm_mul", bm_mul);
    jerry_value_free(bm_mul);

    auto bm_sum = jerry_function_external(_sum);
    jerry_object_set_sz(global, "bm_sum", bm_sum);
    jerry_value_free(bm_sum);

    auto bm_add = jerry_function_external(_add);
    jerry_object_set_sz(global, "bm_add", bm_add);
    jerry_value_free(bm_add);

    auto bm_sub = jerry_function_external(_sub);
    jerry_object_set_sz(global, "bm_sub", bm_sub);
    jerry_value_free(bm_sub);

    auto bm_div = jerry_function_external(_div);
    jerry_object_set_sz(global, "bm_div", bm_div);
    jerry_value_free(bm_div);

    auto mul = jerry_function_external(_mul);
    jerry_object_set_sz(global, "$mul", mul);
    jerry_value_free(mul);

    auto sum = jerry_function_external(_sum);
    jerry_object_set_sz(global, "sum", sum);
    jerry_value_free(sum);

    auto add = jerry_function_external(_add);
    jerry_object_set_sz(global, "add", add);
    jerry_value_free(add);

    auto sub = jerry_function_external(_sub);
    jerry_object_set_sz(global, "sub", sub);
    jerry_value_free(sub);

    auto div = jerry_function_external(_div);
    jerry_object_set_sz(global, "div", div);
    jerry_value_free(div);
}


void LottieExpressions::buildComp()
{
    //layer(index) / layer(name) / layer(otherLayer, reIndex)
    auto layer = jerry_function_external(_layer);
    jerry_object_set_native_ptr(layer, nullptr, comp);
    jerry_object_set_sz(thisComp, "layer", layer);
    jerry_value_free(layer);

    //marker
    //marker.key(index)
    //marker.key(name)
    //marker.nearestKey(t)
    //marker.numKeys

    auto numLayers = jerry_number(comp->root->children.count);
    jerry_object_set_sz(thisComp, "numLayers", numLayers);
    jerry_value_free(numLayers);

    //activeCamera

    auto width = jerry_number(comp->w);
    jerry_object_set_sz(thisComp, EXP_WIDTH, width);
    jerry_value_free(width);

    auto height = jerry_number(comp->h);
    jerry_object_set_sz(thisComp, EXP_HEIGHT, height);
    jerry_value_free(height);

    auto duration = jerry_number(comp->duration());
    jerry_object_set_sz(thisComp, "duration", duration);
    jerry_value_free(duration);

    //ntscDropFrame
    //displayStartTime

    auto frameDuration = jerry_number(comp->frameCnt());
    jerry_object_set_sz(thisComp, "frameDuration", frameDuration);
    jerry_value_free(frameDuration);

    //shutterAngle
    //shutterPhase
    //bgColor
    //pixelAspect

    auto name = jerry_string((jerry_char_t*)comp->name, strlen(comp->name), JERRY_ENCODING_UTF8);
    jerry_object_set_sz(thisComp, EXP_NAME, name);
    jerry_value_free(name);
}


void LottieExpressions::buildGlobal()
{
    global = jerry_current_realm();

    //comp(name)
    auto comp = jerry_function_external(_comp);
    jerry_object_set_native_ptr(comp, nullptr, this->comp);
    jerry_object_set_sz(global, "comp", comp);
    jerry_value_free(comp);

    //footage(name)

    thisComp = jerry_object();
    jerry_object_set_native_ptr(thisComp, nullptr, this->comp);
    jerry_object_set_sz(global, "thisComp", thisComp);

    thisLayer = jerry_object();
    jerry_object_set_sz(global, "thisLayer", thisLayer);

    thisProperty = jerry_object();
    jerry_object_set_sz(global, "thisProperty", thisProperty);

    //time: see update()

    //posterizeTime(framesPerSecond)
    //value
}


jerry_value_t LottieExpressions::evaluate(float frameNo, LottieExpression* exp)
{
    //update global context values
    jerry_object_set_native_ptr(thisLayer, nullptr, exp->layer);
    _buildLayer(thisLayer, exp->layer, comp);

    jerry_object_set_native_ptr(thisProperty, nullptr, exp->property);

    _buildProperty(global, exp);

    if (exp->object->type == LottieObject::Transform) {
        _buildTransform(global, static_cast<LottieTransform*>(exp->object));
    }

    //evalue the code
    auto eval = jerry_eval((jerry_char_t *) exp->code, strlen(exp->code), JERRY_PARSE_NO_OPTS);
    if (jerry_value_is_undefined(eval)) TVGERR("LOTTIE", "Expression error");
    else jerry_value_free(eval);

    return jerry_object_get_sz(global, "$bm_rt");
}


/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/


LottieExpressions::~LottieExpressions()
{
    if (!prepared) return;

    jerry_value_free(thisProperty);
    jerry_value_free(thisLayer);
    jerry_value_free(thisComp);
    jerry_value_free(global);
    jerry_cleanup();
}


void LottieExpressions::prepare(LottieComposition* comp)
{
    if (prepared) return;

    this->comp = comp;

    //FIXME: thread safety?
    jerry_init(JERRY_INIT_EMPTY);

    buildGlobal();
    buildComp();
    buildMath();

    prepared = true;
}


void LottieExpressions::update(float frameNo, LottieComposition* comp)
{
    //time, #current time in seconds
    auto time = jerry_number(comp->timeAtFrame(frameNo));
    jerry_object_set_sz(global, "time", time);
    jerry_value_free(time);
}

#endif //THORVG_LOTTIE_EXPRESSIONS_SUPPORT