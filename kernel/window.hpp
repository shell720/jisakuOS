// 表示領域を表すwindowクラス

#pragma once

#include <vector>
#include <optional>
#include "graphics.hpp"
#include "frame_buffer.hpp"

// Windowクラスはグラフィックの表示領域を表す
// タイトルやメニューがあるウィンドウだけでなく、マウスカーソルの表示領域なども含む
class Window{
    public:
        // WindowWriterはWindowと関連付けられたPixelWriterを提供する
        class WindowWriter : public PixelWriter{
            public:
                WindowWriter(Window& window) : window_{window} {}
                virtual void Write(Vector2D<int> pos, const PixelColor& c) override {
                    window_.Write(pos, c);
                }
                virtual int Width() const override {return window_.Width(); }
                virtual int Height() const override {return window_.Height(); }

            private:
                Window& window_;
        };


        Window(int width, int height, PixelFormat shadow_format);
        ~Window() = default;
        Window(const Window& rhs) = delete;
        Window& operator=(const Window& rhs) = delete;


        // 与えられたFrameBufferにこのウィンドウの表示領域を描画する。
        // @param dst 描画先
        // @param position  writerの左上を基準とした描画位置
        void DrawTo(FrameBuffer& dst, Vector2D<int> position);
        void SetTransparentColor(std::optional<PixelColor> c);
        // このインスタンスに紐付いたWindowWriterを取得
        WindowWriter* Writer();

        const PixelColor& At(Vector2D<int> pos) const;
        void Write(Vector2D<int> pos, PixelColor c);

        int Width() const;
        int Height() const;

        void Move(Vector2D<int> dst_pos, const Rectangle<int>& src);

    private:
        int width_, height_;
        std::vector<std::vector<PixelColor>> data_{};
        WindowWriter writer_{*this};
        std::optional<PixelColor> transparent_color_{std::nullopt};

        FrameBuffer shadow_buffer_{};
};