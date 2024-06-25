// 重ね合わせ処理

#pragma once

#include <memory>
#include <map>
#include <vector>

#include "graphics.hpp"
#include "window.hpp"

class Layer{
    public:
        Layer(unsigned int id = 0);
        unsigned int ID() const;

        Layer& SetWindow(const std::shared_ptr<Window>& window);
        std::shared_ptr<Window> GetWindow() const;

        //レイヤーの位置情報を指定された絶対座標へと更新する。※再描画はしない
        Layer& Move(Vector2D<int> pos); 
        //レイヤーの位置情報を指定された相対座標へと更新する。※再描画はしない
        Layer& MoveRelative(Vector2D<int> pos_diff); 

        //writerにWindowの内容を描画
        void DrawTo(PixelWriter& writer) const; 

    private:
        unsigned int id_;
        Vector2D<int> pos_;
        std::shared_ptr<Window> window_;
};

class LayerManager { // 複数のレイヤーを管理
    public:
        void SetWriter(PixelWriter* writer);
        Layer& NewLayer();

        void Draw() const;

        void Move(unsigned int id, Vector2D<int> new_position);
        void MoveRelative(unsigned int id, Vector2D<int> pos_diff);

        // レイヤーの高さ方向の位置を指定された位置に移動する
        // new_heightに負の高さを指定するとレイヤーは非表示
        // new_heightに現在のレイヤー数以上の数値を指定するとレイヤーは最前面
        void UpDown(unsigned int id, int new_height);
        // レイヤーを非表示
        void Hide(unsigned int id);
        
    private:
        PixelWriter* writer_{nullptr};
        std::vector<std::unique_ptr<Layer>> layers_{};
        std::vector<Layer*> layer_stack_{};
        unsigned int latest_id_{0};

        Layer* FindLayer(unsigned int id);
};

extern LayerManager* layer_manager;