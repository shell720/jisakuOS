// PCIバス制御のプログラム

#pragma once

#include <cstdint>
#include <array>

#include "error.hpp"

namespace pci {
    //configアドレス
    const uint16_t kConfigAddress = 0x0cf8; //CONFIG_ADDRESSレジスタのIOポートアドレス
    const uint16_t kConfigData=0x0cfc; //CONFIG_DATAレジスタのIOポートアドレス

    struct ClassCode{
        uint8_t base, sub, interface;

        bool Match(uint8_t b) {return b==base;}
        bool Match(uint8_t b, uint8_t s) {return Match(b)&& s==sub;}
        bool Match(uint8_t b, uint8_t s, uint8_t i) {return Match(b, s)&& i==interface;}
    };

    // PCIデバイスを操作するための基礎データを格納
    struct Device {
        uint8_t bus, device, function, header_type;
        ClassCode class_code;
    };

    //データの読み書き
    void WriteAddress(uint32_t address); //CONFIG_ADDRESSに整数を書き込む
    void WriteData(uint32_t value); //CONFIG_DATAに整数を書き込む
    uint32_t ReadData(); //CONFIG_DATAから整数を読み込む

    uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);
    uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);
    uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function);
    ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function); //31-24: ベースクラス2, 23-16:サブクラス
    uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function); //23-16:サブオーディネイトパス番号
    
    inline uint16_t ReadVendorId(const Device& dev){
        return ReadVendorId(dev.bus, dev.device, dev.function);
    }

    uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr);

    void WriteConfReg(const Device& dev, uint8_t reg_addr, uint32_t value);

    bool IsSingleFunctionDevice(uint8_t header_type);

    
    inline std::array<Device, 32> devices; //ScanAllBus()により発見されたPCIデバイスの一覧
    inline int num_devices; //deviceの有効な要素の数

    //PCIデバイスをバス0から再帰的に全て探索して、devicesの先頭から書き込む
    // 発見したデバイスの数を num_deviceに設定する
    Error ScanAllBus(); 

    constexpr uint8_t CalcBarAddress(unsigned int bar_index){
        return 0x10 + 4*bar_index;
    }

    WithError<uint64_t> ReadBar(Device& device, unsigned int bar_index);

    //PCIケーパビリティレジスタの共通ヘッダ
    union CapabilityHeader {
        uint32_t data;
        struct {
            uint32_t cap_id: 8;
            uint32_t next_ptr: 8;
            uint32_t cap: 16;
        } __attribute__((packed)) bits;
    } __attribute__((packed));
    const uint8_t kCapabilityMSI = 0x05;
    const uint8_t kCapabilityMSIX = 0x11;

    // PCIデバイスのPCIケーパビリティを読み込む
    // @param dev ケーパビリティを読み込むPCIデバイス
    // @param addr ケーパビリティレジスタのコンフィグレーション空間アドレス
    CapabilityHeader ReadCapabilityHeader(const Device& dev, uint8_t addr);

    //色々なMSIケーパビリティ構造があるため、全てに対応できるように定義
    struct MSICapability{
        union{
            uint32_t data;
            struct {
                uint32_t cap_id: 8;
                uint32_t next_ptr: 8;
                uint32_t msi_enable: 1;
                uint32_t multi_msg_capable: 3;
                uint32_t multi_msg_enable: 3;
                uint32_t addr_64_capable: 1;
                uint32_t per_vector_mask_capable : 1;
                uint32_t: 7;
            } __attribute__((packed)) bits;
        } __attribute__((packed)) header;

        uint32_t msg_addr;
        uint32_t msg_upper_addr;
        uint32_t msg_data;
        uint32_t mask_bits;
        uint32_t pending_bits;
    } __attribute__((packed));

    //@param 
    // dev：対象のPCIデバイス
    // msg_addr: 割り込み発生時にメッセージを書き込む先のアドレス
    // msg_data: 割り込み発生時に書き込むメッセージの値
    Error ConfigureMSI(const Device& dev, uint32_t msg_addr, uint32_t msg_data, unsigned int num_vector_exponent);

    enum class MSITriggerMode{
        kEdge = 0,
        kLevel = 1
    };

    enum class MSIDeliveryMode {
        kFixed = 0b000,
        kLowestPriority = 0b001,
        kSMI = 0b010,
        kNMI = 0b100,
        kINIT = 0b101,
        kExtINT = 0b111,
    };

    Error ConfigureMSIFixedDestination(const Device& dev, uint8_t apic_id, MSITriggerMode trigger_mode,
        MSIDeliveryMode delivery_mode, uint8_t vector, unsigned int num_vector_exponent);
}

void InitializePCI();