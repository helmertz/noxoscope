//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef Constants_H
#define Constants_H

#include <glm/vec3.hpp>

constexpr auto PROGRAM_NAME = "Noxoscope";

constexpr auto UNIFORM_MODEL_MATRIX = "modelMatrix";
constexpr auto UNIFORM_VIEW_MATRIX = "viewMatrix";
constexpr auto UNIFORM_PROJECTION_MATRIX = "projMatrix";
constexpr auto UNIFORM_INVERSE_PROJECTION_MATRIX = "invProj";
constexpr auto UNIFORM_TEXTURE_DIFFUSE = "textureDiffuse";
constexpr auto UNIFORM_TEXTURE_SPECULAR = "textureSpecular";
constexpr auto UNIFORM_TEXTURE_NORMAL = "textureNormal";
constexpr auto UNIFORM_COLOR_DIFFUSE = "colorDiffuse";
constexpr auto UNIFORM_HAS_DIFFUSE_TEXTURE = "hasDiffuseTexture";
constexpr auto UNIFORM_HAS_SPECULAR_TEXTURE = "hasSpecularTexture";
constexpr auto UNIFORM_HAS_NORMAL_TEXTURE = "hasNormalTexture";
constexpr auto UNIFORM_SPECULAR = "specular";
constexpr auto UNIFORM_REFLECTIVENESS = "reflectiveness";
constexpr auto UNIFORM_SAMPLES = "samples";
constexpr auto UNIFORM_WIDTH = "width";
constexpr auto UNIFORM_HEIGHT = "height";

constexpr int DEFAULT_WIDTH = 1280;
constexpr int DEFAULT_HEIGHT = 720;

constexpr const size_t MAX_MODELS = 512;
constexpr const size_t MAX_ENTITIES = 512;
constexpr const size_t MAX_LIGHTS = 512;

extern const glm::vec3 NULL_VECTOR;
extern const glm::vec3 UNIT_X;
extern const glm::vec3 UNIT_Y;
extern const glm::vec3 UNIT_Z;

// X11 Colors
extern const glm::vec3 ALICE_BLUE;
extern const glm::vec3 ANTIQUE_WHITE;
extern const glm::vec3 AQUA;
extern const glm::vec3 AQUAMARINE;
extern const glm::vec3 AZURE;
extern const glm::vec3 BEIGE;
extern const glm::vec3 BISQUE;
extern const glm::vec3 BLACK;
extern const glm::vec3 BLANCHED_ALMOND;
extern const glm::vec3 BLUE;
extern const glm::vec3 BLUE_VIOLET;
extern const glm::vec3 BROWN;
extern const glm::vec3 BURLYWOOD;
extern const glm::vec3 CADET_BLUE;
extern const glm::vec3 CHARTREUSE;
extern const glm::vec3 CHOCOLATE;
extern const glm::vec3 CORAL;
extern const glm::vec3 CORNFLOWER;
extern const glm::vec3 CORNSILK;
extern const glm::vec3 CRIMSON;
extern const glm::vec3 CYAN;
extern const glm::vec3 DARK_BLUE;
extern const glm::vec3 DARK_CYAN;
extern const glm::vec3 DARK_GOLDENROD;
extern const glm::vec3 DARK_GRAY;
extern const glm::vec3 DARK_GREEN;
extern const glm::vec3 DARK_KHAKI;
extern const glm::vec3 DARK_MAGENTA;
extern const glm::vec3 DARK_OLIVE_GREEN;
extern const glm::vec3 DARK_ORANGE;
extern const glm::vec3 DARK_ORCHID;
extern const glm::vec3 DARK_RED;
extern const glm::vec3 DARK_SALMON;
extern const glm::vec3 DARK_SEA_GREEN;
extern const glm::vec3 DARK_SLATE_BLUE;
extern const glm::vec3 DARK_SLATE_GRAY;
extern const glm::vec3 DARK_TURQUOISE;
extern const glm::vec3 DARK_VIOLET;
extern const glm::vec3 DEEP_PINK;
extern const glm::vec3 DEEP_SKY_BLUE;
extern const glm::vec3 DIM_GRAY;
extern const glm::vec3 DODGER_BLUE;
extern const glm::vec3 FIREBRICK;
extern const glm::vec3 FLORAL_WHITE;
extern const glm::vec3 FOREST_GREEN;
extern const glm::vec3 FUCHSIA;
extern const glm::vec3 GAINSBORO;
extern const glm::vec3 GHOST_WHITE;
extern const glm::vec3 GOLD;
extern const glm::vec3 GOLDENROD;
extern const glm::vec3 GRAY;
extern const glm::vec3 WEB_GRAY;
extern const glm::vec3 GREEN;
extern const glm::vec3 WEB_GREEN;
extern const glm::vec3 GREEN_YELLOW;
extern const glm::vec3 HONEYDEW;
extern const glm::vec3 HOT_PINK;
extern const glm::vec3 INDIAN_RED;
extern const glm::vec3 INDIGO;
extern const glm::vec3 IVORY;
extern const glm::vec3 KHAKI;
extern const glm::vec3 LAVENDER;
extern const glm::vec3 LAVENDER_BLUSH;
extern const glm::vec3 LAWN_GREEN;
extern const glm::vec3 LEMON_CHIFFON;
extern const glm::vec3 LIGHT_BLUE;
extern const glm::vec3 LIGHT_CORAL;
extern const glm::vec3 LIGHT_CYAN;
extern const glm::vec3 LIGHT_GOLDENROD;
extern const glm::vec3 LIGHT_GRAY;
extern const glm::vec3 LIGHT_GREEN;
extern const glm::vec3 LIGHT_PINK;
extern const glm::vec3 LIGHT_SALMON;
extern const glm::vec3 LIGHT_SEA_GREEN;
extern const glm::vec3 LIGHT_SKY_BLUE;
extern const glm::vec3 LIGHT_SLATE_GRAY;
extern const glm::vec3 LIGHT_STEEL_BLUE;
extern const glm::vec3 LIGHT_YELLOW;
extern const glm::vec3 LIME;
extern const glm::vec3 LIME_GREEN;
extern const glm::vec3 LINEN;
extern const glm::vec3 MAGENTA;
extern const glm::vec3 MAROON;
extern const glm::vec3 WEB_MAROON;
extern const glm::vec3 MEDIUM_AQUAMARINE;
extern const glm::vec3 MEDIUM_BLUE;
extern const glm::vec3 MEDIUM_ORCHID;
extern const glm::vec3 MEDIUM_PURPLE;
extern const glm::vec3 MEDIUM_SEA_GREEN;
extern const glm::vec3 MEDIUM_SLATE_BLUE;
extern const glm::vec3 MEDIUM_SPRING_GREEN;
extern const glm::vec3 MEDIUM_TURQUOISE;
extern const glm::vec3 MEDIUM_VIOLET_RED;
extern const glm::vec3 MIDNIGHT_BLUE;
extern const glm::vec3 MINT_CREAM;
extern const glm::vec3 MISTY_ROSE;
extern const glm::vec3 MOCCASIN;
extern const glm::vec3 NAVAJO_WHITE;
extern const glm::vec3 NAVY_BLUE;
extern const glm::vec3 OLD_LACE;
extern const glm::vec3 OLIVE;
extern const glm::vec3 OLIVE_DRAB;
extern const glm::vec3 ORANGE;
extern const glm::vec3 ORANGE_RED;
extern const glm::vec3 ORCHID;
extern const glm::vec3 PALE_GOLDENROD;
extern const glm::vec3 PALE_GREEN;
extern const glm::vec3 PALE_TURQUOISE;
extern const glm::vec3 PALE_VIOLET_RED;
extern const glm::vec3 PAPAYA_WHIP;
extern const glm::vec3 PEACH_PUFF;
extern const glm::vec3 PERU;
extern const glm::vec3 PINK;
extern const glm::vec3 PLUM;
extern const glm::vec3 POWDER_BLUE;
extern const glm::vec3 PURPLE;
extern const glm::vec3 WEB_PURPLE;
extern const glm::vec3 REBECCA_PURPLE;
extern const glm::vec3 RED;
extern const glm::vec3 ROSY_BROWN;
extern const glm::vec3 ROYAL_BLUE;
extern const glm::vec3 SADDLE_BROWN;
extern const glm::vec3 SALMON;
extern const glm::vec3 SANDY_BROWN;
extern const glm::vec3 SEA_GREEN;
extern const glm::vec3 SEASHELL;
extern const glm::vec3 SIENNA;
extern const glm::vec3 SILVER;
extern const glm::vec3 SKY_BLUE;
extern const glm::vec3 SLATE_BLUE;
extern const glm::vec3 SLATE_GRAY;
extern const glm::vec3 SNOW;
extern const glm::vec3 SPRING_GREEN;
extern const glm::vec3 STEEL_BLUE;
extern const glm::vec3 TAN;
extern const glm::vec3 TEAL;
extern const glm::vec3 THISTLE;
extern const glm::vec3 TOMATO;
extern const glm::vec3 TURQUOISE;
extern const glm::vec3 VIOLET;
extern const glm::vec3 WHEAT;
extern const glm::vec3 WHITE;
extern const glm::vec3 WHITE_SMOKE;
extern const glm::vec3 YELLOW;
extern const glm::vec3 YELLOW_GREEN;

#endif // Constants_H
