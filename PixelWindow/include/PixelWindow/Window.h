#pragma once

#include <GLFW/glfw3.h>

#include <vector>
#include <functional>

namespace pw {

    namespace internal {
        class Glfw {
        private:
        public:
            Glfw();
            ~Glfw();
        };
    }

    struct mpos
    {
        double x;
        double y;
    };

    class Window {
    private:
        const static internal::Glfw glfw;

        GLFWwindow* handle = nullptr;
        void* owner = nullptr;

        std::vector<std::function<void(int, int)>> windowResizeCallbacks;
        std::vector<std::function<void(void)>> windowRefreshCallbacks;
        std::vector<std::function<void(void*, mpos, int, int, int)>> windowMouseCallbacks;
        std::function<bool(void*)> windowCloseCallbackFn;
        std::vector<std::function<void(void*, mpos)>> windowCursorCallbacks;

        static void windowSizeCallback(GLFWwindow* window, int width, int height);
        static void windowRefreshCallback(GLFWwindow* window);
        static void windowMouseCallback(GLFWwindow* window, int button, int action, int modes);
        static int windowCloseCallback(GLFWwindow* window);
        static void windowCursorPosCalback(GLFWwindow* window, double xpos, double ypos);

    public:
        explicit Window(int width, int height, const char* title);
        ~Window();

        Window(const Window& other) = delete;
        Window(Window&& other) noexcept;

        Window& operator=(const Window& other) = delete;
        Window& operator=(Window&& other) noexcept;

        friend void swap(Window& w1, Window& w2) noexcept;

        bool isActive() const noexcept;
        void pollEvents() const noexcept;
        void makeCurrent() const noexcept;
        void swapBuffers() const noexcept;
        void forceClose() noexcept;

        GLFWwindow* _getHandle() const { return handle; }
        void setOwner(void* ownerPtr);
        void hideCursor(bool hide);
        void addResizeCallback(std::function<void(int,int)> callback);
        void addRefreshCallback(std::function<void(void)> callback);
        void addMouseCallback(std::function<void(void*, mpos, int, int, int)> callback);
        void setCloseCallback(std::function<bool(void*)> callback);
        void addCursorPosCallback(std::function<void(void*, mpos)> callback);
    };

}
