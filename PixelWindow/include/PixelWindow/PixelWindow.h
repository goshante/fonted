#pragma once

#include "Window.h"

namespace pw {

	class PixelWindow : public Window {
	private:
		int width = 0;
		int height = 0;
		bool resized = false;
		int* pixelBuffer = nullptr;

		// Geometry
		GLuint vertVbo = 0;
		GLuint texVbo = 0;
		GLuint vao = 0;

		// Shaders
		GLuint shader = 0;

		// Texture
		GLuint textureId = 0;

		// Pbos
		GLuint pbos[2]{ 0 };
		int pboIndex = 0;

		void resizeBuffers() noexcept;

	public:
		PixelWindow(int width, int height, const char* title);

		PixelWindow(const PixelWindow& other) = delete;
		PixelWindow(PixelWindow&& other) noexcept;

		PixelWindow& operator=(const PixelWindow& other) = delete;
		PixelWindow& operator=(PixelWindow&& other) noexcept;

		~PixelWindow();

		friend void swap(PixelWindow& w1, PixelWindow& w2) noexcept;

		void setBackgroundColor(int color);
		void setPixel(int x, int y, int color);
		void beginFrame();
		void endFrame();

		int getWidth() const noexcept;
		int getHeight() const noexcept;
	};

}
