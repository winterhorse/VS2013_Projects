#ifndef DISPLAY_HPP_
#define DISPLAY_HPP_

#include <iostream>
#include <string>
#include <gc_vdk.h>
#include <GLES3/gl3.h>

const int WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720;

typedef struct _MouseOffset
{
	/* coordinate. */
	double        x;
	double        y;
}MouseOffset;

class Display {
public:
	Display() {}
	Display(int width, int height, const std::string& title) {
		// EGL configuration - we use 24-bpp render target and a 16-bit Z buffer.
		EGLint configAttribs[] =
		{
			EGL_SAMPLES, 0,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, EGL_DONT_CARE,
			EGL_DEPTH_SIZE, 2,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_NONE,
		};

		EGLint attribListContext[] =
		{
			// Needs to be set for es2.0 as default client version is es1.1.
			EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE
		};

		int posX = -1;
		int posY = -1;
		int samples = 0;

		// Set multi-sampling.
		configAttribs[1] = samples;

		vdkEGL eglTemp = { 0 };

		if (!vdkSetupEGL(posX, posY, width, height, configAttribs, NULL, attribListContext, &eglTemp)) {
			return;
		}

		this->egl = eglTemp;	//必须赋值才能执行 vdkSetupEGL

		// Set window title and show the window.
		vdkSetWindowTitle(egl.window, title.c_str());
		vdkShowWindow(egl.window);

		m_isCloseed = false;
	}

	~Display() {
		vdkFinishEGL(&egl);
	}

	//事件处理
	void update() {
		// flush all commands.
		glFlush();

		// swap display with drawn surface.
		vdkSwapEGL(&egl);

		vdkEvent event;
		if (vdkGetEvent(egl.window, &event))
		{
			if ((event.type == VDK_KEYBOARD))
			{
				if (0 == event.data.keyboard.pressed)	// key released
				{
					//std::cout << "Key: " << event.data.keyboard.key;
					//printf(" code: 0x%x", event.data.keyboard.key);
					//printf(" scancode: 0x%x\n", event.data.keyboard.scancode);

					switch (event.data.keyboard.scancode)
					{
					case VDK_ESCAPE:
						// Use ESCAPE to quit.
						m_isCloseed = true;
						break;

					case VDK_F1:
						key = 5;
						break;

					case VDK_UP:
						key = 4;
						break;

					case VDK_DOWN:
						key = 3;
						break;

					case VDK_LEFT:
						key = 2;
						break;

					case VDK_RIGHT:
						key = 1;
						break;

					case VDK_W:
						key = 00;
						break;
					case VDK_S:
						key = 11;
						break;
					case VDK_A:
						key = 22;
						break;
					case VDK_D:
						key = 33;
						break;
					case VDK_Z:
						key = 44;
						break;
					case VDK_X:
						key = 55;
						break;
					case VDK_R:
						key = 66;
						break;
					case VDK_F:
						key = 77;
						break;
					case VDK_T:
						key = 88;
						break;
					case VDK_G:
						key = 99;
						break;

					default:
						if (VDK_UNKNOWN == event.data.keyboard.scancode)
						{
							std::cout << "key code is unknown " << std::endl;
						}
						break;
					}
				}
			}
			else if (event.type == VDK_POINTER)
			{
				if (pressed_left || pressed_right || pressed_middle)
				{
					offset.x = event.data.pointer.x - current_key_x;
					offset.y = event.data.pointer.y - current_key_y;
				}

			}
			else if (event.type == VDK_BUTTON)
			{
				if (event.data.button.left) // mouse left button is pressed 
				{
					pressed_left = true;

					current_key_x = event.data.button.x;
					current_key_y = event.data.button.y;
				}
				else // mouse left button is released
				{
					pressed_left = false;
				}

				if (event.data.button.right) // mouse right button is pressed 
				{
					pressed_right = true;

					current_key_x = event.data.button.x;
					current_key_y = event.data.button.y;
				}
				else // mouse right button is released
				{
					pressed_right = false;
				}


				if (event.data.button.middle)
				{
					pressed_middle = true;

					current_key_x = event.data.button.x;
					current_key_y = event.data.button.y;
				}
				else
				{
					pressed_middle = false;
				}
			}
		}
	}

	int getKeyState()
	{
		int temp = key;
		key = -1;

		return temp;
	}

	bool isMouseLeftPressed()
	{
		return pressed_left;
	}

	bool isMouseRightPressed()
	{
		return pressed_right;
	}

	bool isMouseMiddlePressed()
	{
		return pressed_middle;
	}

	//关闭判断
	bool isClosed()
	{
		return m_isCloseed;
	}

	void clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!
	}

private:
	bool m_isCloseed;
	vdkEGL egl;
	int key;

	bool pressed_left = false;
	bool pressed_right = false;
	bool pressed_middle = false;

	int current_key_x = 0;
	int current_key_y = 0;

public:
	MouseOffset offset;
};

#endif

