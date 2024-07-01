// 重ね合わせ処理

#pragma once

#include <memory>
#include <map>
#include <vector>

#include "graphics.hpp"
#include "window.hpp"
#include "message.hpp"

class Layer{
    public:
        Layer(unsigned int id = 0);
        unsigned int ID() const;

        Layer& SetWindow(const std::shared_ptr<Window>& window);
        std::shared_ptr<Window> GetWindow() const;
        Vector2D<int> GetPosition() const;
        Layer& SetDraggable(bool draggable); // trueでレイヤーがドラッグ移動可能
        bool IsDraggable() const;

        //レイヤーの位置情報を指定された絶対座標へと更新する。※再描画はしない
        Layer& Move(Vector2D<int> pos); 
        //レイヤーの位置情報を指定された相対座標へと更新する。※再描画はしない
        Layer& MoveRelative(Vector2D<int> pos_diff); 

        //writerにWindowの内容を描画
        void DrawTo(FrameBuffer& screen, const Rectangle<int>& area) const; 

    private:
        unsigned int id_;
        Vector2D<int> pos_{};
        std::shared_ptr<Window> window_{};
        bool draggable_{false};
};

class LayerManager { // 複数のレイヤーを管理
    public:
        void SetWriter(FrameBuffer* screen);
        Layer& NewLayer();

        void Draw(const Rectangle<int>& area) const;
        void Draw(unsigned int id) const;
        void Draw(unsigned int id, Rectangle<int> area) const;

        void Move(unsigned int id, Vector2D<int> new_pos);
        void MoveRelative(unsigned int id, Vector2D<int> pos_diff);

        // レイヤーの高さ方向の位置を指定された位置に移動する
        // new_heightに負の高さを指定するとレイヤーは非表示
        // new_heightに現在のレイヤー数以上の数値を指定するとレイヤーは最前面
        void UpDown(unsigned int id, int new_height);
        // レイヤーを非表示
        void Hide(unsigned int id);

        //座標で、ウィンドウを持つ最も上に表示されているレイヤーを探す
        Layer* FindLayerByPosition(Vector2D<int> pos, unsigned int exclude_id) const;
        Layer* FindLayer(unsigned int id);
        int GetHeight(unsigned int id);
        
    private:
        FrameBuffer* screen_{nullptr};
        mutable FrameBuffer back_buffer_{};
        std::vector<std::unique_ptr<Layer>> layers_{};
        std::vector<Layer*> layer_stack_{};
        unsigned int latest_id_{0};
};

extern LayerManager* layer_manager;

class ActiveLayer{
    public:
        ActiveLayer(LayerManager& manager);
        void SetMouseLayer(unsigned int mouse_layer);
        void Activate(unsigned int layer_id);
        unsigned int GetActive() const { return active_layer_;}

    private:
        LayerManager& manager_;
        unsigned int active_layer_{0};
        unsigned int mouse_layer_{0};
};

extern ActiveLayer* active_layer;
extern std::map<unsigned int, uint64_t>* layer_task_map;

void InitializeLayer();
void ProcessLayerMessage(const Message& msg);

constexpr Message MakeLayerMessage(uint64_t task_id, unsigned int layer_id, LayerOperation op, const Rectangle<int>& area){
    Message msg{Message::kLayer, task_id};
    msg.arg.layer.layer_id = layer_id;
    msg.arg.layer.op = op;
    msg.arg.layer.x = area.pos.x;
    msg.arg.layer.y = area.pos.y;
    msg.arg.layer.w = area.size.x;
    msg.arg.layer.h = area.size.y;
    return msg;
}