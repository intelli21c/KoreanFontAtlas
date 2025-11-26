#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <locale>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <json.hpp>
#include "loader.h"

#pragma comment(lib, "freetype.lib")



using json = nlohmann::ordered_json;



extern const wchar_t* koreans;

class Character
{
public:
	int width;
	int height;
	int bearingx;
	int bearingy;
	int advance;
	int coordx;
	int coordy;
	char* bitmap;
	Character(int pwidth, int pheight, int pbearingx, int pbearingy, int padvance, char* glyph = NULL)
	{
		width = pwidth;
		height = pheight;
		bearingx = pbearingx;
		bearingy = pbearingy;
		advance = padvance;
		bitmap = (char*)malloc(sizeof(char) * width * height);
		if (glyph)
		{
			memcpy(bitmap, glyph, sizeof(char) * width * height); // image is 8bit greyscale
		}
	}
};


Character* kchars;

template <typename T> void bitblt(T* src, int srcdimx, int srcdimy, T* dst, int dstdimx, int dstdimy, int offsetx, int offsety) // from, to, where
{
	//size check - is dst larger?

	for (int j = 0; j < srcdimy; j++)
	{
		for (int i = 0; i < srcdimx; i++)
		{
			dst[offsety * dstdimx + offsetx + j * dstdimx + i] = src[j * srcdimx + i];
			//dst[(offsety + j) * dstdimx + (offsetx + i)] = src[j * srcdimx + i]; //gpt ... equivalent with mine
		}
	}
}


FT_Library ft;
FT_Face face;

//int maxw = -999; //obviously...
//int maxh = -999;
int maxw = 0; //why??
int maxh = 0;

int main(int argc, char* argv[])
{
	int charcount = wcslen(koreans);//2350 letters in ksx 1001
	std::locale::global(std::locale("kor"));
	kchars = (Character*)malloc(sizeof(Character) * charcount);
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	}
	if (FT_New_Face(ft, "C:\\Users\\intelli21c\\Sources\\BopenGL1\\BottomUp\\GyeonggiTitleM.ttf", 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}

	FT_Select_Charmap(face, ft_encoding_unicode);
	// set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);
	// loop and generate
	for (int c = 0; c < charcount; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, koreans[c], FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}

		//update max width / height
		//std::cout << face->glyph->bitmap.width << std::endl;
		if ((face->glyph->bitmap.width) > maxw)
			maxw = face->glyph->bitmap.width;
		if (face->glyph->bitmap.rows > maxh)
			maxh = face->glyph->bitmap.rows;

		kchars[c] = Character((int)face->glyph->bitmap.width, (int)face->glyph->bitmap.rows, face->glyph->bitmap_left, face->glyph->bitmap_top, face->glyph->advance.x, (char*)face->glyph->bitmap.buffer);
	}
	//std::cout << "max width / height " << maxw << "," << maxh;
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	//calc padding, compute col row size
	int atlasheight = (4 + maxh + 4) * (charcount / 32 + 1);
	int atlaswidth = (4 + maxw + 4) * 32;

	char* atlas = (char*)calloc(atlaswidth * atlasheight, sizeof(char));
	json metadata = json::object({ {"row_length", 32}, {"grid_width", maxw}, {"grid_height", maxh}, {"padding", 4} });
	json arr = json::array();
	//blit paste each character in grid
	for (int c = 0; c < charcount; c++)
	{
		int x = c % 32;
		int y = c / 32;
		bitblt<char>(kchars[c].bitmap, kchars[c].width, kchars[c].height, atlas, atlaswidth, atlasheight, x * (maxw + 8) + 4, y * (maxh + 8) + 4);
		//also write coord data to somewhere
		kchars[c].coordx = x * (maxw + 8) + 4;
		kchars[c].coordy = y * (maxh + 8) + 4;
		json chardata = json::object({ {"character", koreans[c]}, {"coordx", x * (maxw + 8) + 4}, {"coordy", y * (maxh + 8) + 4}, {"width", kchars[c].width}, {"height", kchars[c].height}, {"advance", kchars[c].advance} ,{"bearingx", kchars[c].bearingx}, {"bearingy", kchars[c].bearingy} });
		arr.push_back(chardata);
	}
	//metadata["data"] = arr;
	metadata.push_back({ "data", arr });
	//export image
	stbi_write_png("C:\\Users\\intelli21c\\Sources\\KoreanFontAtlas\\KoreanFontAtlas\\glyph.png", atlaswidth, atlasheight, 1, atlas, atlaswidth);
	//export json metadata
	std::ofstream of;
	of.open("C:\\Users\\intelli21c\\Sources\\KoreanFontAtlas\\KoreanFontAtlas\\glyph.json");
	if (of.is_open())
	{
		of << metadata;
		of.close();
	}
	else std::cout << metadata.dump(4);

	FontLoader::Character* output;
	FontLoader::loadBitmapMetadata("C:\\Users\\intelli21c\\Sources\\KoreanFontAtlas\\KoreanFontAtlas\\glyph.json", &output);

	return 0;
}