#include "glad/gl.h"
#include "glad/wgl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#include "loader.h"

#pragma comment(lib, "freetype.lib")

#include <iostream>
#include <map>
#include <string>
#include <cmath>

struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
	bool in_atlas;			//in pre-generated atlas?
	glm::ivec2 TexCoord;	//coordinate in atlas
};

std::map<wchar_t, Character> Characters;

Shader* shader;
unsigned int VBO, VAO;

bool ready = false;

float c = 0;
UINT fontatlas;
glm::ivec2 atlas_size;
glm::mat4 ortho;

void resize(int x, int y)
{
	if (!ready) return;
	ortho = glm::ortho(0.0f, (float)x, 0.0f, (float)y);
	glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(ortho));
}

int mouseaccx = 0, mouseaccy = 0;
void mousein(int* dx, int* dy)
{
	mouseaccx += *dx;
	mouseaccy += *dy;
}

void kbdin(int key, bool type)
{
	if (type) // key down
	{

	}
}

void update(int interval) //millisecs, keep up and update from begg. of last update to now.
{

}

void timedupdate(int interval)
{
	//std::cout << "timed update\n";
	c++;
}

FT_Library ft;
FT_Face face;

void prepare()
{
	ortho = glm::ortho(0.0f, static_cast<float>(1024), 0.0f, static_cast<float>(768));
	shader = new Shader("C:\\Users\\intelli21c\\Sources\\FontJob\\FontJob\\text.vs", "C:\\Users\\intelli21c\\Sources\\FontJob\\FontJob\\text.fs");
	shader->use();
	glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(ortho));
	// FreeType
	// --------
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	}

	// load font as face
	if (FT_New_Face(ft, "C:\\Users\\intelli21c\\Sources\\BopenGL1\\BottomUp\\GyeonggiTitleM.ttf", 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}

	FT_Select_Charmap(face, ft_encoding_unicode);
	// set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);

	// destroy FreeType once we're finished
	//FT_Done_Face(face);
	//FT_Done_FreeType(ft);
	// configure VAO/VBO for texture quads
	// -----------------------------------
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	int width, height, nrChannels;
	unsigned char* data = stbi_load("C:\\Users\\intelli21c\\Sources\\FontJob\\FontJob\\glyph.png", &width, &height, &nrChannels, 0);
	atlas_size = glm::ivec2(width, height);

	if (data == NULL) puts("Texture loading failed.");
	glActiveTexture(GL_TEXTURE0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &fontatlas);
	glBindTexture(GL_TEXTURE_2D, fontatlas); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, 0);

	FontLoader::Character* fcharacters;
	int charcnt = FontLoader::loadBitmapMetadata("C:\\Users\\intelli21c\\Sources\\FontJob\\FontJob\\glyph.json", &fcharacters);
	for (int i = 0; i < charcnt; i++)
	{
		FontLoader::Character current = fcharacters[i];
		// now store character for later use
		Character character = {
			fontatlas,
			glm::ivec2(current.width, current.height),
			glm::ivec2(current.bearingx, current.bearingy),
			current.advance,
			true,
			glm::ivec2(current.coordx, current.coordy)
		};
		Characters.insert(std::pair<wchar_t, Character>(current.character, character));
	}
	free(fcharacters);


	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ready = true;
}

void RenderText(Shader& shader, std::wstring text, float x, float y, float scale, glm::vec3 color)
{
	// activate corresponding render state	
	shader.use();

	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glBindVertexArray(VAO);

	// iterate through all characters
	std::wstring::const_iterator c;
	bool generated = false;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch;
		auto it = Characters.find(*c);
		if (it == Characters.end()) {
			std::wcout << L"Generated " << *c << "\n";
			glActiveTexture(GL_TEXTURE1);
			// Load character glyph 
			if (FT_Load_Char(face, *c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			}
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			ch = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<unsigned int>(face->glyph->advance.x),
				false
			};

			Characters.insert({ *c, ch });
			//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}
		ch = Characters[*c];
		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4];
		if (ch.in_atlas)
		{
			float ul, ur, vt, vb;
			ul = (float)ch.TexCoord.x / (float)atlas_size.x;
			ur = (float)(ch.TexCoord.x + ch.Size.x) / (float)atlas_size.x;
			vt = (float)ch.TexCoord.y / (float)atlas_size.y;
			vb = (float)(ch.TexCoord.y + ch.Size.y) / (float)atlas_size.y;
			//std::cout << ul << ur << vt << vb << "\n";

			float verticesa[6][4] = {
				{ xpos, ypos + h,	    ul, vt },
				{ xpos,     ypos,       ul, vb },
				{ xpos + w, ypos,       ur, vb },

				{ xpos,     ypos + h,   ul, vt },
				{ xpos + w, ypos,       ur, vb },
				{ xpos + w, ypos + h,   ur, vt }
			};
			memcpy(vertices, verticesa, sizeof(float) * 6 * 4);
			glActiveTexture(GL_TEXTURE0);
			shader.setInt("text", 0);
		}
		else
		{
			float verticesa[6][4] = {
				{ xpos, ypos + h, 0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};
			memcpy(vertices, verticesa, sizeof(float) * 6 * 4);
			glActiveTexture(GL_TEXTURE1);
			shader.setInt("text", 1);
		}

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void render()
{
	if (!ready) return;

	//glUniform1f(glGetUniformLocation(shader->ID, "time"), c);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader->use();
	char outtxt[65] = { 0 };
	//sprintf_s(outtxt, sizeof(outtxt), u8"가나다 time: %d", int(c / 60));
	RenderText(*shader, L"가나다라마바사 뱦뺴뷁궳읒증", 25.0f, 25.0f, 1.0f, glm::vec3(0.0, 0.0f, 0.0f));
	//glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}