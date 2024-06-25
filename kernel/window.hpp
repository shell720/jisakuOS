// 表示領域を表すwindowクラス

#pragma once

#include <vector>
#include <optional>
#include "graphics.hpp"

// Windowクラスはグラフィックの表示領域を表す
// タイトルやメニューがあるウィンドウだけでなく、マウスカーソルの表示領域なども含む
class Window{
    public:
        // WindowWriterはWindowと関連付けられたPixelWriterを提供する
        class WindowWriter : public PixelWriter{
            public:
                WindowWriter(Window& window) : window_{window} {}
                virtual void Write(int x, int y, const PixelColor& c) override {
                    window_.At(x, y) = c;
                }
                virtual int Width() const override {return window_.Width(); }
                virtual int Height() const override {return window_.Height(); }

            private:
                Window& window_;
        };


        Window(int width, int height);
        ~Window() = default;
        Window(const Window& rhs) = delete;
        Window& operator=(const Window& rhs) = delete;


        // 与えられたPixelWriterにこのウィンドウの表示領域を描画する。
        // @param writer  描画先
        // @param position  writerの左上を基準とした描画位置
        void DrawTo(PixelWriter& writer, Vector2D<int> position);
        void SetTransparentColor(std::optional<PixelColor> c);
        // このインスタンスに紐付いたWindowWriterを取得
        WindowWriter* Writer();

        PixelColor& At(int x, int y);
        const PixelColor& At(int x, int y) const;

        int Width() const;
        int Height() const;

    private:
        int width_, height_;
        std::vector<std::vector<PixelColor>> data_{};
        WindowWriter writer_{*this};
        std::optional<PixelColor> transparent_color_{std::nullopt};
};