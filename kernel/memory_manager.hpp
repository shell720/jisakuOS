// メモリ管理クラスと周辺機能

#pragma once

#include <array>
#include <limits>

#include "error.hpp"
#include "memory_map.hpp"

namespace{
    constexpr unsigned long long operator""_KiB(unsigned long long kib){
        return kib*1024;
    }

    constexpr unsigned long long operator""_MiB(unsigned long long mib){
        return mib*1024_KiB;
    }

    constexpr unsigned long long operator""_GiB(unsigned long long gib){
        return gib*1024_MiB;
    }
}

// 物理メモリフレーム1つの大きさ[バイト]
static const auto kBytesPerFrame{4_KiB};

class FrameID{
    public:
        explicit FrameID(size_t id) : id_{id} {}
        size_t ID() const {return id_;}
        void* Frame() const {return reinterpret_cast<void*>(id_ * kBytesPerFrame);}
    
    private:
        size_t id_;
};

static const FrameID kNullFrame{std::numeric_limits<size_t>::max()};

// 1ビットを1フレームに対応させて、ビットマップにより空きフレームを管理する
// 配列alloc_mapの各ビットがフレームに対応  0: 空き、1: 使用中
// alloc_map[n] の mビット目が対応する物理アドレスは次の式： kFrameBytes * (n * kBitsPerMapLine + m)
class BitmapMemoryManager{
    public:
        static const auto kMaxPhysicalMemoryBytes{128_GiB};  // このメモリ管理クラスで扱える最大の物理メモリ量[バイト]
        static const auto kFrameCount{kMaxPhysicalMemoryBytes / kBytesPerFrame}; // ↑物理メモリを扱うために必要なフレーム数

        using MapLineType =  unsigned long; //ページフレームの空き or 使用を表すためのbitの塊
        static const size_t kBitsPerMapLine{8 * sizeof(MapLineType)}; 

        BitmapMemoryManager();

        // 要求されたフレーム数の領域を確保して先頭のフレームIDを返す
        WithError<FrameID> Allocate(size_t num_frames);
        Error Free(FrameID start_frame, size_t num_frames);
        void MarkAllocated(FrameID start_frame, size_t num_frames);

        // このメモリマネージャで扱うメモリ範囲を設定する
        void SetMemoryRange(FrameID range_begin, FrameID range_end);
    
    private:
        std::array<MapLineType, kFrameCount / kBitsPerMapLine> alloc_map_;
        FrameID range_begin_;
        FrameID range_end_;

        bool GetBit(FrameID frame) const;
        void SetBit(FrameID frame, bool allocated);
};

extern BitmapMemoryManager* memory_manager;
void InitializeMemoryManager(const MemoryMap& memory_map);