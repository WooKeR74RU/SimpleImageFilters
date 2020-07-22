#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <ctime>
using namespace std;
#include <SFML/Graphics/Image.hpp>
using namespace sf;

int rand(int min, int max)
{
	return min + rand() % (max - min + 1);
}

Color makeColor(float r, float g, float b)
{
	r = min(r, 255.0f);
	r = max(r, 0.0f);
	g = min(g, 255.0f);
	g = max(g, 0.0f);
	b = min(b, 255.0f);
	b = max(b, 0.0f);
	return Color((Uint8)r, (Uint8)g, (Uint8)b);
}

Color redComponent(Color pixel)
{
	return Color(pixel.r, 0, 0);
}
Color greenComponent(Color pixel)
{
	return Color(0, pixel.g, 0);
}
Color blueComponent(Color pixel)
{
	return Color(0, 0, pixel.b);
}
Color grayscale(Color pixel)
{
	float gray = 0.2126f * pixel.r + 0.7152f * pixel.g + 0.0722f * pixel.b;
	return makeColor(gray, gray, gray);
}
Color redShadeSepia(Color pixel)
{
	float red = 0.393f * pixel.r + 0.769f * pixel.g + 0.189f * pixel.b;
	float green = 0.349f * pixel.r + 0.686f * pixel.g + 0.168f * pixel.b;
	float blue = 0.272f * pixel.r + 0.534f * pixel.g + 0.131f * pixel.b;
	return makeColor(red, green, blue);
}
Color greenShadeSepia(Color pixel)
{
	float red = 0.349f * pixel.r + 0.686f * pixel.g + 0.168f * pixel.b;
	float green = 0.393f * pixel.r + 0.769f * pixel.g + 0.189f * pixel.b;
	float blue = 0.272f * pixel.r + 0.534f * pixel.g + 0.131f * pixel.b;
	return makeColor(red, green, blue);
}
Color blueShadeSepia(Color pixel)
{
	float red = 0.272f * pixel.r + 0.534f * pixel.g + 0.131f * pixel.b;
	float green = 0.349f * pixel.r + 0.686f * pixel.g + 0.168f * pixel.b;
	float blue = 0.393f * pixel.r + 0.769f * pixel.g + 0.189f * pixel.b;
	return makeColor(red, green, blue);
}
Color negative(Color pixel)
{
	return Color(255 - pixel.r, 255 - pixel.g, 255 - pixel.b);
}
Color noise(Color pixel)
{
	const Uint8 INTENSE = 100;
	int red = pixel.r + rand(-(int)INTENSE, INTENSE);
	int green = pixel.g + rand(-(int)INTENSE, INTENSE);
	int blue = pixel.b + rand(-(int)INTENSE, INTENSE);
	return makeColor((float)red, (float)green, (float)blue);
}
Color increaseBrightness(Color pixel)
{
	const float INTENSE = 1.4f;
	float red = pixel.r * INTENSE;
	float green = pixel.g * INTENSE;
	float blue = pixel.b * INTENSE;
	return makeColor(red, green, blue);
}
Color decreaseBrightness(Color pixel)
{
	const float INTENSE = 0.6f;
	float red = pixel.r * INTENSE;
	float green = pixel.g * INTENSE;
	float blue = pixel.b * INTENSE;
	return makeColor(red, green, blue);
}
Color blackAndWhite(Color pixel)
{
	const Uint16 SEPARATOR = 384;
	Uint16 total = pixel.r + pixel.g + pixel.b;
	if (total < SEPARATOR)
		return Color::Black;
	return Color::White;
}
Color expressedRed(Color pixel)
{
	return makeColor(1.5f * pixel.r, (pixel.g + pixel.b) / 2.0f, (pixel.g + pixel.b) / 2.0f);
}
Color expressedGreen(Color pixel)
{
	return makeColor((pixel.r + pixel.b) / 2.0f, 1.5f * pixel.g, (pixel.r + pixel.b) / 2.0f);
}
Color expressedBlue(Color pixel)
{
	return makeColor((pixel.r + pixel.g) / 2.0f, (pixel.r + pixel.g) / 2.0f, 1.5f * pixel.b);
}

Image linearTransform(const Image& image, const function<Color(Color)>& filter)
{
	Image res(image);
	for (size_t i = 0; i < res.getSize().y; i++)
	{
		for (size_t j = 0; j < res.getSize().x; j++)
			res.setPixel(j, i, filter(res.getPixel(j, i)));
	}
	return res;
}

Image matrixTransform(const Image& image, const vector<vector<float>>& matrix)
{
	const size_t imageH = image.getSize().y;
	const size_t imageW = image.getSize().x;
	const size_t kernelSize = matrix.size();
	const size_t padding = kernelSize / 2;
	Image complement;
	const size_t complementH = imageH + 2 * padding;
	const size_t complementW = imageW + 2 * padding;
	complement.create(complementW, complementH);
	for (size_t y = 0; y < imageH; y++)
	{
		for (size_t x = 0; x < imageW; x++)
			complement.setPixel(x + padding, y + padding, image.getPixel(x, y));
	}
	for (size_t y = padding; y < complementH - padding; y++)
	{
		for (size_t x = 0; x < padding; x++)
		{
			complement.setPixel(x, y, complement.getPixel(padding, y));
			complement.setPixel(complementW - 1 - x, y, complement.getPixel(complementW - 1 - padding, y));
		}
	}
	for (size_t x = 0; x < complementW; x++)
	{
		for (size_t y = 0; y < padding; y++)
		{
			complement.setPixel(x, y, complement.getPixel(x, padding));
			complement.setPixel(x, complementH - 1 - y, complement.getPixel(x, complementH - 1 - padding));
		}
	}
	Image result;
	result.create(imageW, imageH);
	for (size_t y = 0; y < imageH; y++)
	{
		for (size_t x = 0; x < imageW; x++)
		{
			float red = 0;
			float green = 0;
			float blue = 0;
			for (size_t i = 0; i < kernelSize; i++)
			{
				for (size_t j = 0; j < kernelSize; j++)
				{
					red += complement.getPixel(x + j, y + i).r * matrix[i][j];
					green += complement.getPixel(x + j, y + i).g * matrix[i][j];
					blue += complement.getPixel(x + j, y + i).b * matrix[i][j];
				}
			}
			result.setPixel(x, y, makeColor(red, green, blue));
		}
	}
	return result;
}

int main()
{
	srand((size_t)time(nullptr));

	vector<vector<float>> blur(5);
	blur[0] = { 0.000789f, 0.006581f, 0.013347f, 0.006581f, 0.000789f };
	blur[1] = { 0.006581f, 0.054901f, 0.111345f, 0.054901f, 0.006581f };
	blur[2] = { 0.013347f, 0.111345f, 0.225824f, 0.111345f, 0.013347f };
	blur[3] = { 0.006581f, 0.054901f, 0.111345f, 0.054901f, 0.006581f };
	blur[4] = { 0.000789f, 0.006581f, 0.013347f, 0.006581f, 0.000789f };

	vector<vector<float>> sharpen(3);
	sharpen[0] = { -1, -1, -1 };
	sharpen[1] = { -1, 9, -1 };
	sharpen[2] = { -1, -1, -1 };

	vector<vector<float>> outlineBlur(3);
	outlineBlur[0] = { 0.1f, 0.2f, 0.3f };
	outlineBlur[1] = { 0.8f, -2.6f, 0.4f };
	outlineBlur[2] = { 0.7f, 0.6f, 0.5f };

	vector<function<Color(Color)>> filters;
	filters.push_back(redComponent);
	filters.push_back(greenComponent);
	filters.push_back(blueComponent);
	filters.push_back(grayscale);
	filters.push_back(redShadeSepia);
	filters.push_back(greenShadeSepia);
	filters.push_back(blueShadeSepia);
	filters.push_back(negative);
	filters.push_back(noise);
	filters.push_back(increaseBrightness);
	filters.push_back(decreaseBrightness);
	filters.push_back(blackAndWhite);
	filters.push_back(expressedRed);
	filters.push_back(expressedGreen);
	filters.push_back(expressedBlue);

	vector<vector<vector<float>>> matrices;
	matrices.push_back(blur);
	matrices.push_back(sharpen);
	matrices.push_back(outlineBlur);

	Image image;
	if (!image.loadFromFile("pict.jpg"))
		throw;

	Image result;
	result.create(max(filters.size(), matrices.size()) * image.getSize().x, 3 * image.getSize().y);
	result.copy(image, 0, 0);
	for (size_t i = 0; i < filters.size(); i++)
	{
		Image tile = linearTransform(image, filters[i]);
		result.copy(tile, i * image.getSize().x, image.getSize().y);
	}
	for (size_t i = 0; i < matrices.size(); i++)
	{
		Image tile = matrixTransform(image, matrices[i]);
		result.copy(tile, i * image.getSize().x, 2 * image.getSize().y);
	}

	if (!result.saveToFile("result.jpg"))
		throw;

	return 0;
}
