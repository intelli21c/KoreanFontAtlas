#pragma once
#ifndef FONTATLASLOADER_H
#define FONTATLASLOADER_H

#include <iostream>
#include <fstream>
#include <json.hpp>


using json = nlohmann::ordered_json;

namespace FontLoader
{
	class Character
	{
	public:
		wchar_t character;
		int width;
		int height;
		int bearingx;
		int bearingy;
		int advance;
		int coordx;
		int coordy;
		Character(wchar_t pcharacter, int pwidth, int pheight, int pbearingx, int pbearingy, int padvance, int pcoordx, int pcoordy)
		{
			character = pcharacter;
			width = pwidth;
			height = pheight;
			bearingx = pbearingx;
			bearingy = pbearingy;
			advance = padvance;
			coordx = pcoordx;
			coordy = pcoordy;
		}
	};



	int loadBitmapMetadata(std::string path, Character** arr)
	{
		json meta;
		std::ifstream file;
		file.open(path, std::ios_base::in);

		if (!file.is_open())
		{
			std::cout << "failed to open file.\n";
			return -1;
		}
		file >> meta;
		int rowl, maxw, maxh, padding;
		//try
		{
			rowl = meta["row_length"].get<int>();
			maxw = meta["grid_width"].get<int>();
			maxh = meta["grid_height"].get<int>();
			padding = meta["padding"].get<int>();
			json data = meta["data"];
			int count = data.size();
			*arr = (Character*)malloc(count * sizeof(Character));
			for (int i = 0; i < count; i++)
			{
				json current = data[i];
				Character c = Character(current["character"], current["width"], current["height"], current["bearingx"], current["bearingy"], current["advance"], current["coordx"], current["coordy"]);
				(*arr)[i] = c;
			}
			return count;
		}
		//catch (const std::exception& e)
		{
		//	std::cout << e.what();
		}
		return -1;

	}
}


#endif // !FONTATLASLOADER_H