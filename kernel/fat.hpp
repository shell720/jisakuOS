// FATファイルシステムを操作する

#pragma once

#include <cstdint>
#include <cstddef>

#include "error.hpp"
#include "file.hpp"

namespace fat{
    struct BPB{
        uint8_t jump_boot[3];
        char oem_name[8];
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sector_count;
        uint8_t num_fats;
        uint16_t root_entry_count;
        uint16_t total_sectors_16;
        uint8_t media;
        uint16_t fat_size_16;
        uint16_t sectors_per_track;
        uint16_t num_heads;
        uint32_t hidden_sectors;
        uint32_t total_sectors_32;
        uint32_t fat_size_32;
        uint16_t ext_flags;
        uint16_t fs_version;
        uint32_t root_cluster;
        uint16_t fs_info;
        uint16_t backup_boot_sector;
        uint8_t reserved[12];
        uint8_t drive_number;
        uint8_t reserved1;
        uint8_t boot_signature;
        uint32_t volume_id;
        char volume_label[11];
        char fs_type[8];
    } __attribute__((packed));

    enum class Attribute : uint8_t{
        kReadOnly = 0x01,
        kHidden = 0x02,
        kSystem = 0x04,
        kVolumeID = 0x08,
        kDirectory = 0x10,
        kArchive = 0x20,
        kLongName = 0x0f,
    };

    struct DirectoryEntry{
        unsigned char name[11];
        Attribute attr;
        uint8_t ntres;
        uint8_t create_time_tenth;
        uint16_t create_time;
        uint16_t create_date;
        uint16_t last_access_date;
        uint16_t first_cluster_high;
        uint16_t write_time;
        uint16_t write_date;
        uint16_t first_cluster_low;
        uint32_t file_size;

        uint32_t FirstCluster() const{
            return first_cluster_low | (static_cast<uint32_t>(first_cluster_high)<<16);
        }
    }__attribute__((packed));

    extern BPB* boot_volume_image;
    extern unsigned long bytes_per_cluster;
    void Initialize(void* volume_image);

    // 指定されたクラスタの先頭セクタが置いてあるメモリアドレスを返す
    // @param cluster  クラスタ番号（2 始まり）
    // @return クラスタの先頭セクタが置いてあるメモリ領域のアドレス
    uintptr_t GetClusterAddr(unsigned long cluster);

    // 指定されたクラスタの先頭セクタが置いてあるメモリ領域を返す
    // @param cluster  クラスタ番号（2 始まり）
    // @return クラスタの先頭セクタが置いてあるメモリ領域へのポインタ
    template <class T>
    T* GetSectorByCluster(unsigned long cluster){
        return reinterpret_cast<T*>(GetClusterAddr(cluster));
    }

    // ディレクトリエントリの短名を基本名と拡張子名に分割して取得する。
    // パディングされた空白文字（0x20）は取り除かれ，ヌル終端される。
    // @param entry  ファイル名を得る対象のディレクトリエントリ
    // @param base  拡張子を除いたファイル名（9 バイト以上の配列）
    // @param ext  拡張子（4 バイト以上の配列）
    void ReadName(const DirectoryEntry& entry, char* base, char* ext);

    void FormatName(const DirectoryEntry& entry, char* dest);

    static const unsigned long kEndOfClusterchain = 0x0fffffflu;

    // 指定されたクラスタの次のクラスタ番号を返す。
    // @param cluster  クラスタ番号
    // @return 次のクラスタ番号（無い場合は kEndOfClusterchain）
    unsigned long NextCluster(unsigned long cluster);

    // @param name  8+3形式のファイル名（大文字小文字は区別しない）
    // @param directory_cluster  ディレクトリの開始クラスタ（省略するとルートディレクトリから検索する）
    // @return ファイルまたはディレクトリを表すエントリ、と末尾スラッシュを示すフラグの組。見つからなければ nullptr。
    std::pair<DirectoryEntry*, bool>
    FindFile(const char* path, unsigned long directory_cluster=0);

    bool NameIsEqual(const DirectoryEntry& entry, const char* name);

    // ファイルの内容をバッファにコピーする
    // @param buf ファイル内容の格納先
    // @param len バッファの大きさ（バイト単位）
    // @return 読み込んだバイト数
    size_t LoadFile(void* buf, size_t len, DirectoryEntry& entry);

    bool IsEndOfClusterchain(unsigned long cluster);

    uint32_t* GetFAT();

    unsigned long ExtendCluster(unsigned long eoc_cluster, size_t n);

    DirectoryEntry* AllocateEntry(unsigned long dir_cluster);

    void SetFileName(DirectoryEntry& entry, const char* name);

    WithError<DirectoryEntry*> CreateFile(const char* path);

    unsigned long AllocateClusterChain(size_t n);

    class FileDescriptor: public ::FileDescriptor{
        public:
            explicit FileDescriptor(DirectoryEntry& fat_entry);
            size_t Read(void* buf, size_t len) override;
            size_t Write(const void* buf, size_t len) override;
            size_t Size() const override {return fat_entry_.file_size;}
            size_t Load(void* buf, size_t len, size_t offset) override;

        private:
            DirectoryEntry& fat_entry_;
            size_t rd_off_ = 0;
            unsigned long rd_cluster_ = 0;
            size_t rd_cluster_off_ = 0;
            size_t wr_off_ = 0;
            unsigned long wr_cluster_ = 0;
            size_t wr_cluster_off_ = 0;
    };
}